// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXSourceControlSubsystem.h"
#include "Logging/PGXLogMacros.h"
#include "PGXChangelistStore.h"
#include "PGXCommitValidator.h"
#include "PGXCommitTagger.h"
#include "PGXVersionControlEditor.h"
#include "PGXVersionControlSettings.h"

#include "ISourceControlModule.h"
#include "ISourceControlProvider.h"
#include "HAL/IConsoleManager.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "PGXSourceControlSubsystem"

namespace
{
FString PGXExtractBranchNameFromStatusText(const FString& InStatusStr)
{
	static const TArray<FString> BranchMarkers = {
		TEXT("Branch:"),
		TEXT("branch:"),
		TEXT("On branch "),
		TEXT("on branch ")
	};

	for (const FString& Marker : BranchMarkers)
	{
		const int32 BranchIdx = InStatusStr.Find(Marker, ESearchCase::CaseSensitive);
		if (BranchIdx == INDEX_NONE)
		{
			continue;
		}

		FString BranchPart = InStatusStr.Mid(BranchIdx + Marker.Len()).TrimStartAndEnd();
		if (BranchPart.IsEmpty())
		{
			continue;
		}

		int32 EndIdx = BranchPart.Len();
		const TCHAR Delimiters[] = { TCHAR('\r'), TCHAR('\n'), TCHAR(','), TCHAR(')'), TCHAR('(') };
		for (const TCHAR Delimiter : Delimiters)
		{
			int32 CandidateIdx = INDEX_NONE;
			if (BranchPart.FindChar(Delimiter, CandidateIdx))
			{
				EndIdx = FMath::Min(EndIdx, CandidateIdx);
			}
		}

		BranchPart = BranchPart.Left(EndIdx).TrimStartAndEnd();
		if (!BranchPart.IsEmpty())
		{
			return BranchPart;
		}
	}

	return TEXT("Unknown");
}
} // namespace

// ============================================================================
// EN: Constructor / Destructor (TUniquePtr requires explicit destructor in .cpp)
// ES: Constructor / Destructor (TUniquePtr requiere destructor explícito en .cpp)
// ============================================================================

UPGXSourceControlSubsystem::UPGXSourceControlSubsystem()
{
	// EN: Default constructor / ES: Constructor por defecto
}

UPGXSourceControlSubsystem::UPGXSourceControlSubsystem(FVTableHelper& Helper)
	: Super(Helper)
{
	// EN: VTable helper constructor required by generated code.
	// ES: Constructor helper de VTable requerido por codigo generado.
}

UPGXSourceControlSubsystem::~UPGXSourceControlSubsystem()
{
	// EN: Explicit destructor needed for TUniquePtr with forward-declared types
	// ES: Destructor explícito necesario para TUniquePtr con tipos forward-declared
}

// ============================================================================
// EN: Lifecycle / ES: Ciclo de vida
// ============================================================================

void UPGXSourceControlSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// EN: Create owned components / ES: Crear componentes propios
	ChangelistStore = MakeUnique<FPGXChangelistStore>();
	ChangelistStore->Load();

	CommitValidator = MakeUnique<FPGXCommitValidator>();
	CommitTagger = MakeUnique<FPGXCommitTagger>();

	// EN: S1: Register provider-changed delegate with handle / ES: S1: Registrar delegate de cambio de provider con handle
	ISourceControlModule* SCModule = FModuleManager::Get().IsModuleLoaded("SourceControl") ? &ISourceControlModule::Get() : nullptr;
	if (SCModule)
	{
		ProviderChangedHandle = SCModule->RegisterProviderChanged(
			FSourceControlProviderChanged::FDelegate::CreateUObject(this, &UPGXSourceControlSubsystem::OnProviderChanged));
	}

	RegisterConsoleCommands();

	PGX_LOG_INFO(LogPGXVersionControl,
		TEXT("PGXSourceControlSubsystem: Initialized — Provider=%s, Changelists=%d"),
		*GetProviderName(), ChangelistStore->GetChangelists().Num());
}

void UPGXSourceControlSubsystem::Deinitialize()
{
	UnregisterConsoleCommands();

	// EN: S1: Unbind provider-changed delegate / ES: S1: Desvincular delegate de cambio de provider
	ISourceControlModule* SCModule = FModuleManager::Get().IsModuleLoaded("SourceControl") ? &ISourceControlModule::Get() : nullptr;
	if (SCModule && ProviderChangedHandle.IsValid())
	{
		SCModule->UnregisterProviderChanged(ProviderChangedHandle);
		ProviderChangedHandle.Reset();
	}

	// EN: Persist changelists before shutdown / ES: Persistir changelists antes de shutdown
	if (ChangelistStore.IsValid())
	{
		ChangelistStore->Save();
	}

	// EN: Release owned components / ES: Liberar componentes propios
	ChangelistStore.Reset();
	CommitValidator.Reset();
	CommitTagger.Reset();

	PGX_LOG_INFO(LogPGXVersionControl, TEXT("PGXSourceControlSubsystem: Deinitialized"));

	Super::Deinitialize();
}

// ============================================================================
// EN: Provider Status / ES: Estado del Provider
// ============================================================================

bool UPGXSourceControlSubsystem::IsGitProviderActive() const
{
	ISourceControlModule* SCModule = FModuleManager::Get().IsModuleLoaded("SourceControl") ? &ISourceControlModule::Get() : nullptr;
	if (!SCModule) return false;

	ISourceControlProvider& Provider = SCModule->GetProvider();
	return Provider.GetName().ToString().Contains(TEXT("Git"));
}

FString UPGXSourceControlSubsystem::GetProviderName() const
{
	ISourceControlModule* SCModule = FModuleManager::Get().IsModuleLoaded("SourceControl") ? &ISourceControlModule::Get() : nullptr;
	if (!SCModule) return TEXT("None");

	return SCModule->GetProvider().GetName().ToString();
}

FString UPGXSourceControlSubsystem::GetCurrentBranch() const
{
	ISourceControlModule* SCModule = FModuleManager::Get().IsModuleLoaded("SourceControl") ? &ISourceControlModule::Get() : nullptr;
	if (!SCModule) return TEXT("Unknown");

	ISourceControlProvider& Provider = SCModule->GetProvider();
	FText StatusText = Provider.GetStatusText();
	const FString StatusStr = StatusText.ToString();
	return PGXExtractBranchNameFromStatusText(StatusStr);
}

bool UPGXSourceControlSubsystem::IsProviderConnected() const
{
	ISourceControlModule* SCModule = FModuleManager::Get().IsModuleLoaded("SourceControl") ? &ISourceControlModule::Get() : nullptr;
	if (!SCModule) return false;

	return SCModule->GetProvider().IsEnabled();
}

// ============================================================================
// EN: Changelists (delegates to store) / ES: Changelists (delega al store)
// ============================================================================

const TArray<FPGXChangelist>& UPGXSourceControlSubsystem::GetChangelists() const
{
	static const TArray<FPGXChangelist> Empty;
	return ChangelistStore.IsValid() ? ChangelistStore->GetChangelists() : Empty;
}

const FPGXChangelist* UPGXSourceControlSubsystem::FindChangelist(const FGuid& InGuid) const
{
	return ChangelistStore.IsValid() ? ChangelistStore->FindChangelist(InGuid) : nullptr;
}

FGuid UPGXSourceControlSubsystem::CreateChangelist(const FString& InName, const FString& InDescription)
{
	if (!ChangelistStore.IsValid()) return FGuid();
	const FGuid NewGuid = ChangelistStore->CreateChangelist(InName, InDescription);
	ChangelistStore->Save();
	return NewGuid;
}

bool UPGXSourceControlSubsystem::DeleteChangelist(const FGuid& InGuid)
{
	if (!ChangelistStore.IsValid()) return false;
	const bool bResult = ChangelistStore->DeleteChangelist(InGuid);
	if (bResult) ChangelistStore->Save();
	return bResult;
}

bool UPGXSourceControlSubsystem::RenameChangelist(const FGuid& InGuid, const FString& InNewName)
{
	if (!ChangelistStore.IsValid()) return false;
	const bool bResult = ChangelistStore->RenameChangelist(InGuid, InNewName);
	if (bResult) ChangelistStore->Save();
	return bResult;
}

bool UPGXSourceControlSubsystem::MoveFileToChangelist(const FString& InFilePath, const FGuid& InTargetGuid)
{
	if (!ChangelistStore.IsValid()) return false;
	const bool bResult = ChangelistStore->MoveFileToChangelist(InFilePath, InTargetGuid);
	if (bResult) ChangelistStore->Save();
	return bResult;
}

FPGXVersionControlOperationResult UPGXSourceControlSubsystem::GetLastOperationResult() const
{
	if (!ChangelistStore.IsValid())
	{
		FPGXVersionControlOperationResult Result;
		Result.Status = EPGXVersionControlOperationStatus::StoreUnavailable;
		Result.Message = TEXT("Changelist store unavailable");
		return Result;
	}
	return ChangelistStore->GetLastOperationResult();
}

FOnPGXChangelistsChanged& UPGXSourceControlSubsystem::GetOnChangelistsChanged()
{
	return ChangelistStore->OnChangelistsChanged;
}

// ============================================================================
// EN: Workflow / ES: Workflow
// ============================================================================

TArray<FPGXValidationIssue> UPGXSourceControlSubsystem::ValidatePending() const
{
	if (!CommitValidator.IsValid()) return {};
	return CommitValidator->Validate(CollectAllFilePaths());
}

TArray<FString> UPGXSourceControlSubsystem::GetAutoDetectedSystemTags() const
{
	if (!CommitTagger.IsValid()) return {};
	return CommitTagger->DetectSystems(CollectAllFilePaths());
}

FString UPGXSourceControlSubsystem::GetCommitTemplate(const FGuid& InChangelistGuid) const
{
	if (!CommitTagger.IsValid() || !ChangelistStore.IsValid()) return FString();

	const FPGXChangelist* CL = ChangelistStore->FindChangelist(InChangelistGuid);
	const FString CLName = CL ? CL->DisplayName : TEXT("");
	const TArray<FString> Tags = GetAutoDetectedSystemTags();

	return CommitTagger->GetCommitTemplate(CLName, Tags);
}

int32 UPGXSourceControlSubsystem::GetPendingFileCount() const
{
	return CollectAllFilePaths().Num();
}

TArray<FString> UPGXSourceControlSubsystem::CollectAllFilePaths() const
{
	TArray<FString> AllPaths;
	if (!ChangelistStore.IsValid()) return AllPaths;

	for (const FPGXChangelist& CL : ChangelistStore->GetChangelists())
	{
		AllPaths.Append(CL.FilePaths);
	}
	return AllPaths;
}

// ============================================================================
// EN: Delegate handlers / ES: Handlers de delegates
// ============================================================================

void UPGXSourceControlSubsystem::OnProviderChanged(ISourceControlProvider& OldProvider, ISourceControlProvider& NewProvider)
{
	// EN: Save changelists, update state, broadcast / ES: Guardar changelists, actualizar estado, broadcast
	if (ChangelistStore.IsValid())
	{
		ChangelistStore->Save();
		ChangelistStore->OnChangelistsChanged.Broadcast();
	}

	PGX_LOG_INFO(LogPGXVersionControl, TEXT("PGXSourceControlSubsystem: Provider changed from %s to %s"),
		*OldProvider.GetName().ToString(), *NewProvider.GetName().ToString());
}

// ============================================================================
// EN: Console Commands / ES: Comandos de Consola
// ============================================================================

void UPGXSourceControlSubsystem::RegisterConsoleCommands()
{
	ConsoleCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.vc.status"),
		TEXT("Show PGX Version Control status: provider, branch, pending changes"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			PGX_LOG_INFO(LogPGXVersionControl, TEXT("=== PGX Version Control Status ==="));
			PGX_LOG_INFO(LogPGXVersionControl, TEXT("  Provider: %s"), *GetProviderName());
			PGX_LOG_INFO(LogPGXVersionControl, TEXT("  Branch: %s"), *GetCurrentBranch());
			PGX_LOG_INFO(LogPGXVersionControl, TEXT("  Connected: %s"), IsProviderConnected() ? TEXT("Yes") : TEXT("No"));
			PGX_LOG_INFO(LogPGXVersionControl, TEXT("  Git Active: %s"), IsGitProviderActive() ? TEXT("Yes") : TEXT("No"));
			PGX_LOG_INFO(LogPGXVersionControl, TEXT("  Changelists: %d"), GetChangelists().Num());
			PGX_LOG_INFO(LogPGXVersionControl, TEXT("  Pending Files: %d"), GetPendingFileCount());
		}),
		ECVF_Default
	));

	ConsoleCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.vc.validate"),
		TEXT("Run PGX pre-commit validation on all pending files"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			const TArray<FPGXValidationIssue> Issues = ValidatePending();
			PGX_LOG_INFO(LogPGXVersionControl, TEXT("=== PGX Validation Results (%d issues) ==="), Issues.Num());
			for (const FPGXValidationIssue& Issue : Issues)
			{
				const TCHAR* SevStr = Issue.Severity == EPGXValidationSeverity::Error ? TEXT("ERROR")
					: Issue.Severity == EPGXValidationSeverity::Warning ? TEXT("WARN") : TEXT("INFO");
				PGX_LOG_INFO(LogPGXVersionControl, TEXT("  [%s] %s: %s (%s)"),
					SevStr, *Issue.RuleId, *Issue.Message, *FPaths::GetCleanFilename(Issue.FilePath));
			}
			if (Issues.IsEmpty())
			{
				PGX_LOG_INFO(LogPGXVersionControl, TEXT("  All checks passed."));
			}
		}),
		ECVF_Default
	));

	ConsoleCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.vc.changelists"),
		TEXT("List active PGX changelists with file counts"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			PGX_LOG_INFO(LogPGXVersionControl, TEXT("=== PGX Changelists ==="));
			for (const FPGXChangelist& CL : GetChangelists())
			{
				PGX_LOG_INFO(LogPGXVersionControl, TEXT("  %s%s — %d files"),
					*CL.DisplayName, CL.bIsDefault ? TEXT(" [Default]") : TEXT(""), CL.FilePaths.Num());
			}
		}),
		ECVF_Default
	));

	ConsoleCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.vc.tag"),
		TEXT("Show auto-detected PGX system tags from pending files"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			const TArray<FString> Tags = GetAutoDetectedSystemTags();
			if (Tags.IsEmpty())
			{
				PGX_LOG_INFO(LogPGXVersionControl, TEXT("PGX VC: No system tags detected."));
				return;
			}
			FString TagsStr = FString::Join(Tags, TEXT(", "));
			PGX_LOG_INFO(LogPGXVersionControl, TEXT("PGX VC: Detected systems — %s"), *TagsStr);

			if (CommitTagger.IsValid())
			{
				PGX_LOG_INFO(LogPGXVersionControl, TEXT("PGX VC: Prefix — %s"), *CommitTagger->BuildCommitPrefix(Tags));
			}
		}),
		ECVF_Default
	));

	ConsoleCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.vc.template"),
		TEXT("Show commit message template for default changelist"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			if (!ChangelistStore.IsValid()) return;

			const FGuid DefaultGuid = ChangelistStore->GetDefaultChangelistGuid();
			const FString Template = GetCommitTemplate(DefaultGuid);
			PGX_LOG_INFO(LogPGXVersionControl, TEXT("PGX VC: Commit template:\n%s"), *Template);
		}),
		ECVF_Default
	));
}

void UPGXSourceControlSubsystem::UnregisterConsoleCommands()
{
	for (struct IConsoleCommand* Cmd : ConsoleCommands)
	{
		IConsoleManager::Get().UnregisterConsoleObject(Cmd);
	}
	ConsoleCommands.Empty();
}

#undef LOCTEXT_NAMESPACE
