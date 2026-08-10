// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "PGXVersionControlTypes.h"
#include "PGXSourceControlSubsystem.generated.h"

class FPGXChangelistStore;
class FPGXCommitValidator;
class FPGXCommitTagger;
struct IConsoleCommand;

/**
 * EN: Central PGX Version Control overlay subsystem (UEditorSubsystem).
 *     Runs on top of UE's active Source Control provider (typically Git).
 *     Provides: changelists, auto-tagging, commit templates, pre-commit validation.
 *     Does NOT replace or wrap the provider — uses hooks and monitoring.
 *
 * ES: Subsistema overlay central de PGX Version Control (UEditorSubsystem).
 *     Corre encima del provider de Source Control activo de UE (tipicamente Git).
 *     Provee: changelists, auto-tagging, templates de commit, validacion pre-commit.
 *     NO reemplaza ni wrappea el provider — usa hooks y monitoreo.
 */
UCLASS()
class PGXVERSIONCONTROLEDITOR_API UPGXSourceControlSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	UPGXSourceControlSubsystem();
	UPGXSourceControlSubsystem(FVTableHelper& Helper);
	~UPGXSourceControlSubsystem() override;

	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface

	// ─── Provider Status ───

	/** EN: Is the active provider Git? / ES: El provider activo es Git? */
	bool IsGitProviderActive() const;

	/** EN: Get active provider display name / ES: Obtener nombre del provider activo */
	FString GetProviderName() const;

	/** EN: Get current branch (parsed from status text) / ES: Obtener branch actual */
	FString GetCurrentBranch() const;

	/** EN: Is the provider connected and enabled? / ES: El provider esta conectado y habilitado? */
	bool IsProviderConnected() const;

	// ─── Changelists ───

	const TArray<FPGXChangelist>& GetChangelists() const;
	const FPGXChangelist* FindChangelist(const FGuid& InGuid) const;
	FGuid CreateChangelist(const FString& InName, const FString& InDescription);
	bool DeleteChangelist(const FGuid& InGuid);
	bool RenameChangelist(const FGuid& InGuid, const FString& InNewName);
	bool MoveFileToChangelist(const FString& InFilePath, const FGuid& InTargetGuid);
	FPGXVersionControlOperationResult GetLastOperationResult() const;

	/** EN: Get changelists-changed delegate for UI binding / ES: Obtener delegate de changelists-changed para UI */
	FOnPGXChangelistsChanged& GetOnChangelistsChanged();

	// ─── Workflow ───

	/** EN: Run all enabled validation rules on pending files / ES: Ejecutar reglas de validacion */
	TArray<FPGXValidationIssue> ValidatePending() const;

	/** EN: Detect PGX systems from file paths / ES: Detectar sistemas PGX de rutas */
	TArray<FString> GetAutoDetectedSystemTags() const;

	/** EN: Generate commit template for a changelist / ES: Generar template de commit para una changelist */
	FString GetCommitTemplate(const FGuid& InChangelistGuid) const;

	/** EN: Get pending file count across all changelists / ES: Obtener conteo de archivos pendientes */
	int32 GetPendingFileCount() const;

private:
	void RegisterConsoleCommands();
	void UnregisterConsoleCommands();

	void OnProviderChanged(class ISourceControlProvider& OldProvider, class ISourceControlProvider& NewProvider);

	/** EN: Collect file paths from all changelists / ES: Recolectar rutas de todas las changelists */
	TArray<FString> CollectAllFilePaths() const;

	// ─── Owned components ───
	TUniquePtr<FPGXChangelistStore> ChangelistStore;
	TUniquePtr<FPGXCommitValidator> CommitValidator;
	TUniquePtr<FPGXCommitTagger> CommitTagger;

	// ─── S1: Delegate handles ───
	FDelegateHandle ProviderChangedHandle;

	// ─── Console commands ───
	TArray<struct IConsoleCommand*> ConsoleCommands;
};
