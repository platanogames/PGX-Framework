// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXDocValidator.h"
#include "PGXDocRegistry.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Internationalization/Regex.h"

FDocValidationReport FDocValidator::ValidateAll(const FDocRegistry& Registry) const
{
	FDocValidationReport Report;

	TArray<FString> AllDocIds = Registry.GetAllDocIds();
	Report.TotalDocs = AllDocIds.Num();

	for (const FString& DocId : AllDocIds)
	{
		const FString AbsPath = Registry.ResolveDocId(DocId);
		if (AbsPath.IsEmpty())
		{
			continue;
		}

		FString Content;
		if (!FFileHelper::LoadFileToString(Content, *AbsPath))
		{
			FDocValidationEntry Entry;
			Entry.DocId = DocId;
			Entry.Message = FString::Printf(TEXT("Cannot read file: %s"), *AbsPath);
			Entry.Type = TEXT("ReadError");
			Report.Errors.Add(Entry);
			Report.DocsWithErrors++;
			continue;
		}

		TArray<FDocValidationEntry> DocEntries = ValidateDocument(DocId, Content, Registry);

		bool bHasErrors = false;
		for (const FDocValidationEntry& Entry : DocEntries)
		{
			if (Entry.Type.Contains(TEXT("Error")) || Entry.Type == TEXT("BrokenLink") || Entry.Type == TEXT("MissingImage"))
			{
				Report.Errors.Add(Entry);
				bHasErrors = true;
			}
			else
			{
				Report.Warnings.Add(Entry);
			}
		}

		if (bHasErrors)
		{
			Report.DocsWithErrors++;
		}
	}

	return Report;
}

TArray<FDocValidationEntry> FDocValidator::ValidateDocument(const FString& DocId, const FString& Content, const FDocRegistry& Registry) const
{
	TArray<FDocValidationEntry> Results;

	const FString DocPath = Registry.ResolveDocId(DocId);

	Results.Append(ValidateCrossRefs(DocId, Content, Registry));
	Results.Append(ValidateImages(DocId, Content, DocPath));
	Results.Append(ValidateLinks(DocId, Content, Registry));

	return Results;
}

TArray<FDocValidationEntry> FDocValidator::ValidateCrossRefs(const FString& DocId, const FString& Content, const FDocRegistry& Registry) const
{
	TArray<FDocValidationEntry> Results;

	// EN: Find [[...]] patterns / ES: Encontrar patrones [[...]]
	FRegexPattern Pattern(TEXT("\\[\\[([^\\]]+)\\]\\]"));
	FRegexMatcher Matcher(Pattern, Content);

	while (Matcher.FindNext())
	{
		const FString CrossRef = Matcher.GetCaptureGroup(1);

		// EN: Parse type:value / ES: Parsear tipo:valor
		FString RefType, RefValue;
		if (CrossRef.Split(TEXT(":"), &RefType, &RefValue))
		{
			RefType = RefType.TrimStartAndEnd();
			RefValue = RefValue.TrimStartAndEnd();

			if (RefType.Equals(TEXT("Doc"), ESearchCase::IgnoreCase))
			{
				// EN: Check if target doc exists / ES: Verificar si el doc destino existe
				const FString TargetPath = Registry.ResolveDocId(RefValue);
				if (TargetPath.IsEmpty())
				{
					FDocValidationEntry Entry;
					Entry.DocId = DocId;
					Entry.Message = FString::Printf(TEXT("Broken doc crossref: [[Doc:%s]]"), *RefValue);
					Entry.Type = TEXT("BrokenLink");
					Results.Add(Entry);
				}
			}
			// EN: Class and BP refs are validated at resolve time, not here
			// ES: Refs de Class y BP se validan en tiempo de resolucion, no aqui
		}
		else
		{
			// EN: No type prefix — try as doc ref / ES: Sin prefijo de tipo — intentar como ref de doc
			const FString TargetPath = Registry.ResolveDocId(CrossRef);
			if (TargetPath.IsEmpty())
			{
				FDocValidationEntry Entry;
				Entry.DocId = DocId;
				Entry.Message = FString::Printf(TEXT("Unresolved crossref: [[%s]]"), *CrossRef);
				Entry.Type = TEXT("UnresolvedRef");
				Results.Add(Entry);
			}
		}
	}

	return Results;
}

TArray<FDocValidationEntry> FDocValidator::ValidateImages(const FString& DocId, const FString& Content, const FString& DocPath) const
{
	TArray<FDocValidationEntry> Results;

	if (DocPath.IsEmpty())
	{
		return Results;
	}

	const FString DocDir = FPaths::GetPath(DocPath);

	// EN: Find ![alt](path) patterns / ES: Encontrar patrones ![alt](path)
	FRegexPattern Pattern(TEXT("!\\[[^\\]]*\\]\\(([^)]+)\\)"));
	FRegexMatcher Matcher(Pattern, Content);

	while (Matcher.FindNext())
	{
		const FString ImagePath = Matcher.GetCaptureGroup(1);

		// EN: Skip URLs / ES: Saltar URLs
		if (ImagePath.StartsWith(TEXT("http://")) || ImagePath.StartsWith(TEXT("https://")))
		{
			continue;
		}

		// EN: Resolve relative path / ES: Resolver ruta relativa
		const FString AbsImagePath = FPaths::ConvertRelativePathToFull(FPaths::Combine(DocDir, ImagePath));
		if (!FPaths::FileExists(AbsImagePath))
		{
			FDocValidationEntry Entry;
			Entry.DocId = DocId;
			Entry.Message = FString::Printf(TEXT("Missing image: %s"), *ImagePath);
			Entry.Type = TEXT("MissingImage");
			Results.Add(Entry);
		}
	}

	return Results;
}

TArray<FDocValidationEntry> FDocValidator::ValidateLinks(const FString& DocId, const FString& Content, const FDocRegistry& Registry) const
{
	TArray<FDocValidationEntry> Results;

	// EN: Find [text](path.md) patterns (internal markdown links)
	// ES: Encontrar patrones [texto](path.md) (links markdown internos)
	FRegexPattern Pattern(TEXT("(?<!!)\\[([^\\]]+)\\]\\(([^)]+\\.md[^)]*)\\)"));
	FRegexMatcher Matcher(Pattern, Content);

	while (Matcher.FindNext())
	{
		const FString LinkPath = Matcher.GetCaptureGroup(2);

		// EN: Skip URLs / ES: Saltar URLs
		if (LinkPath.StartsWith(TEXT("http://")) || LinkPath.StartsWith(TEXT("https://")))
		{
			continue;
		}

		// EN: Try to resolve as a file path relative to the doc's directory
		// ES: Intentar resolver como ruta de archivo relativa al directorio del doc
		const FString DocPath = Registry.ResolveDocId(DocId);
		if (!DocPath.IsEmpty())
		{
			const FString DocDir = FPaths::GetPath(DocPath);
			const FString AbsLinkPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(DocDir, LinkPath));
			if (!FPaths::FileExists(AbsLinkPath))
			{
				FDocValidationEntry Entry;
				Entry.DocId = DocId;
				Entry.Message = FString::Printf(TEXT("Broken markdown link: [%s](%s)"), *Matcher.GetCaptureGroup(1), *LinkPath);
				Entry.Type = TEXT("BrokenLink");
				Results.Add(Entry);
			}
		}
	}

	return Results;
}
