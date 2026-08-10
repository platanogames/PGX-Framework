// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXCommitValidator.h"
#include "Logging/PGXLogMacros.h"
#include "PGXVersionControlSettings.h"
#include "PGXVersionControlEditor.h"
#include "Misc/FileHelper.h"
#include "Internationalization/Regex.h"

namespace
{
int32 PGXCalculateLineNumber(const FString& InContent, const int32 InIndex)
{
	int32 LineNum = 1;
	const int32 ClampedIndex = FMath::Clamp(InIndex, 0, InContent.Len());
	for (int32 Index = 0; Index < ClampedIndex; ++Index)
	{
		if (InContent[Index] == TEXT('\n'))
		{
			++LineNum;
		}
	}
	return LineNum;
}

bool PGXHasImmediateUPropertyAnnotation(const FString& InContent, const int32 InMatchIndex)
{
	const int32 LineStart = InContent.Find(TEXT("\n"), ESearchCase::CaseSensitive, ESearchDir::FromEnd, InMatchIndex);
	if (LineStart == INDEX_NONE)
	{
		return false;
	}

	int32 PreviousLineEnd = LineStart - 1;
	while (PreviousLineEnd >= 0 && (InContent[PreviousLineEnd] == TEXT('\r') || InContent[PreviousLineEnd] == TEXT('\n') || FChar::IsWhitespace(InContent[PreviousLineEnd])))
	{
		--PreviousLineEnd;
	}
	if (PreviousLineEnd < 0)
	{
		return false;
	}

	const int32 PreviousLineStart = InContent.Find(TEXT("\n"), ESearchCase::CaseSensitive, ESearchDir::FromEnd, PreviousLineEnd);
	const int32 SliceStart = PreviousLineStart == INDEX_NONE ? 0 : PreviousLineStart + 1;
	const FString PreviousLine = InContent.Mid(SliceStart, PreviousLineEnd - SliceStart + 1).TrimStartAndEnd();
	return PreviousLine.Contains(TEXT("UPROPERTY")) || PreviousLine.Contains(TEXT("TObjectPtr"));
}
} // namespace

bool FPGXCommitValidator::IsPGXSourceFile(const FString& InFilePath) const
{
	// EN: Only validate files inside PGX plugin directories / ES: Solo validar archivos dentro de directorios de plugins PGX
	return InFilePath.Contains(TEXT("PGX")) && (InFilePath.EndsWith(TEXT(".h")) || InFilePath.EndsWith(TEXT(".cpp")));
}

TArray<FPGXValidationIssue> FPGXCommitValidator::Validate(const TArray<FString>& InFilePaths) const
{
	TArray<FPGXValidationIssue> Issues;
	const UPGXVersionControlSettings* Settings = GetDefault<UPGXVersionControlSettings>();

	if (!Settings->bEnablePreSubmitValidation)
	{
		return Issues;
	}

	for (const FString& FilePath : InFilePaths)
	{
		if (!IsPGXSourceFile(FilePath)) continue;

		FString Content;
		if (!FFileHelper::LoadFileToString(Content, *FilePath))
		{
			PGX_LOG_WARNING(LogPGXVersionControl, TEXT("CommitValidator: Cannot read file %s — skipping."), *FilePath);
			continue;
		}

		if (Settings->bValidateLogCategories)
		{
			ValidateLogCategories(FilePath, Content, Issues);
		}

		if (Settings->bValidateDelegateSymmetry)
		{
			ValidateDelegateSymmetry(FilePath, Content, Issues);
		}

		if (Settings->bValidateBuildCsDeps)
		{
			ValidateBuildCsDeps(FilePath, Content, Issues);
		}

		// EN: Always-on rules (no individual toggle) / ES: Reglas siempre activas (sin toggle individual)
		ValidateNamingConventions(FilePath, Content, Issues);
		ValidateUPropertyAnnotation(FilePath, Content, Issues);
	}

	return Issues;
}

void FPGXCommitValidator::ValidateLogCategories(const FString& InFilePath, const FString& InContent, TArray<FPGXValidationIssue>& OutIssues) const
{
	// EN: [Operational] Search for LogTemp usage in PGX modules
	// ES: [Operational] Buscar uso de LogTemp en modulos PGX
	int32 SearchFrom = 0;
	while (true)
	{
		const int32 Idx = InContent.Find(TEXT("LogTemp"), ESearchCase::CaseSensitive, ESearchDir::FromStart, SearchFrom);
		if (Idx == INDEX_NONE) break;

		const int32 LineNum = PGXCalculateLineNumber(InContent, Idx);

		FPGXValidationIssue Issue;
		Issue.Severity = EPGXValidationSeverity::Warning;
		Issue.Confidence = EPGXValidationConfidence::Operational;
		Issue.RuleId = TEXT("Log.TempUsed");
		Issue.Message = FString::Printf(TEXT("LogTemp found at line %d — use LogPGX{System} instead (S7)"), LineNum);
		Issue.FilePath = InFilePath;
		Issue.LineNumber = LineNum;
		OutIssues.Add(MoveTemp(Issue));

		SearchFrom = Idx + 7;
	}
}

void FPGXCommitValidator::ValidateDelegateSymmetry(const FString& InFilePath, const FString& InContent, TArray<FPGXValidationIssue>& OutIssues) const
{
	// EN: [Partial] Same-file scan — look for bind calls without corresponding unbind
	// ES: [Partial] Scan mismo archivo — buscar calls de bind sin unbind correspondiente
	static const TArray<FString> BindPatterns = {
		TEXT("AddUObject"), TEXT("AddRaw"), TEXT("AddLambda"), TEXT("AddSP")
	};

	bool bHasBindCalls = false;
	for (const FString& Pattern : BindPatterns)
	{
		if (InContent.Contains(Pattern))
		{
			bHasBindCalls = true;
			break;
		}
	}

	if (bHasBindCalls && !InContent.Contains(TEXT("RemoveAll")) && !InContent.Contains(TEXT("Remove(")))
	{
		FPGXValidationIssue Issue;
		Issue.Severity = EPGXValidationSeverity::Warning;
		Issue.Confidence = EPGXValidationConfidence::HeuristicPartial;
		Issue.RuleId = TEXT("Delegate.NoUnbind");
		Issue.Message = TEXT("[Partial] Delegate bind found without RemoveAll/Remove in same file (S1)");
		Issue.FilePath = InFilePath;
		OutIssues.Add(MoveTemp(Issue));
	}
}

void FPGXCommitValidator::ValidateBuildCsDeps(const FString& InFilePath, const FString& InContent, TArray<FPGXValidationIssue>& OutIssues) const
{
	// EN: [Partial] Shallow scan — only check .cpp files that include PGX headers
	// ES: [Partial] Scan superficial — solo verificar .cpp que incluyen headers PGX
	if (!InFilePath.EndsWith(TEXT(".cpp"))) return;

	// EN: Look for #include of PGX modules that might need Build.cs deps
	// ES: Buscar #include de modulos PGX que podrian necesitar deps en Build.cs
	const FRegexPattern IncludePattern(TEXT("#include\\s+\"(PGX\\w+)(?:Runtime|Editor)?/"));
	FRegexMatcher Matcher(IncludePattern, InContent);

	TSet<FString> IncludedModules;
	while (Matcher.FindNext())
	{
		IncludedModules.Add(Matcher.GetCaptureGroup(1));
	}

	if (IncludedModules.Num() > 3)
	{
		FPGXValidationIssue Issue;
		Issue.Severity = EPGXValidationSeverity::Info;
		Issue.Confidence = EPGXValidationConfidence::HeuristicPartial;
		Issue.RuleId = TEXT("BuildCs.MissingDep");
		Issue.Message = FString::Printf(TEXT("[Partial] File includes %d PGX modules — verify Build.cs has all deps (S3)"), IncludedModules.Num());
		Issue.FilePath = InFilePath;
		OutIssues.Add(MoveTemp(Issue));
	}
}

void FPGXCommitValidator::ValidateNamingConventions(const FString& InFilePath, const FString& InContent, TArray<FPGXValidationIssue>& OutIssues) const
{
	// EN: [Partial] Check UCLASS/USTRUCT declarations for PGX prefix
	// ES: [Partial] Verificar declaraciones UCLASS/USTRUCT para prefijo PGX
	if (!InFilePath.EndsWith(TEXT(".h"))) return;

	// EN: Look for class declarations that should have PGX prefix
	// ES: Buscar declaraciones de clase que deberian tener prefijo PGX
	const FRegexPattern ClassPattern(TEXT("class\\s+\\w+_API\\s+(U|A|F|S)(\\w+)\\s*:"));
	FRegexMatcher Matcher(ClassPattern, InContent);

	while (Matcher.FindNext())
	{
		const FString Prefix = Matcher.GetCaptureGroup(1);
		const FString Name = Matcher.GetCaptureGroup(2);

		// EN: Skip if already has PGX prefix / ES: Saltar si ya tiene prefijo PGX
		if (Name.StartsWith(TEXT("PGX"))) continue;

		// EN: Only flag if file is in PGX directory / ES: Solo marcar si archivo esta en directorio PGX
		if (InFilePath.Contains(TEXT("PGX")))
		{
			FPGXValidationIssue Issue;
			Issue.Severity = EPGXValidationSeverity::Info;
			Issue.Confidence = EPGXValidationConfidence::HeuristicPartial;
			Issue.RuleId = TEXT("Naming.BadPrefix");
			Issue.Message = FString::Printf(TEXT("[Partial] Class %s%s missing PGX prefix (convention: %sPGX%s)"), *Prefix, *Name, *Prefix, *Name);
			Issue.FilePath = InFilePath;
			OutIssues.Add(MoveTemp(Issue));
		}
	}
}

void FPGXCommitValidator::ValidateUPropertyAnnotation(const FString& InFilePath, const FString& InContent, TArray<FPGXValidationIssue>& OutIssues) const
{
	// EN: [Partial] Heuristic — look for UObject* members without preceding UPROPERTY
	// ES: [Partial] Heuristica — buscar miembros UObject* sin UPROPERTY precedente
	if (!InFilePath.EndsWith(TEXT(".h"))) return;

	const FRegexPattern RawPtrPattern(TEXT("(?:^|\\n)\\s+(?:U|A)\\w+\\*\\s+\\w+;"));
	FRegexMatcher Matcher(RawPtrPattern, InContent);

	while (Matcher.FindNext())
	{
		const FString Match = Matcher.GetCaptureGroup(0);

		const int32 MatchBeginning = Matcher.GetMatchBeginning();
		if (MatchBeginning == INDEX_NONE)
		{
			continue;
		}

		int32 DeclarationOffset = 0;
		while (DeclarationOffset < Match.Len() && FChar::IsWhitespace(Match[DeclarationOffset]))
		{
			++DeclarationOffset;
		}
		const int32 DeclarationIdx = MatchBeginning + DeclarationOffset;

		if (!PGXHasImmediateUPropertyAnnotation(InContent, DeclarationIdx))
		{
			const int32 LineNum = PGXCalculateLineNumber(InContent, DeclarationIdx);
			FPGXValidationIssue Issue;
			Issue.Severity = EPGXValidationSeverity::Warning;
			Issue.Confidence = EPGXValidationConfidence::HeuristicPartial;
			Issue.RuleId = TEXT("UProperty.MissingTag");
			Issue.Message = FString::Printf(TEXT("[Partial] Possible raw UObject* without immediate UPROPERTY/TObjectPtr annotation at line %d: %s (S0)"), LineNum, *Match.TrimStartAndEnd());
			Issue.FilePath = InFilePath;
			Issue.LineNumber = LineNum;
			OutIssues.Add(MoveTemp(Issue));
			break; // EN: One per file is enough / ES: Uno por archivo es suficiente
		}
	}
}
