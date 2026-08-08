// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Observability/PGXObservable.h"
#include "PGXVersionControlTypes.h"
#include "PGXVersionControlSettings.generated.h"

/**
 * EN: Per-user settings for PGX Version Control overlay.
 *     Configurable via Project Settings > PGX > Version Control.
 *     Stored in EditorPerProjectUserSettings (not committed to source control).
 *
 * ES: Configuracion por usuario para el overlay PGX Version Control.
 *     Configurable via Project Settings > PGX > Version Control.
 *     Almacenado en EditorPerProjectUserSettings (no se commitea).
 */
UCLASS(config = EditorPerProjectUserSettings, defaultconfig, meta = (DisplayName = "PGX Version Control"))
class PGXVERSIONCONTROLEDITOR_API UPGXVersionControlSettings : public UDeveloperSettings, public IPGXObservable
{
	GENERATED_BODY()

public:
	UPGXVersionControlSettings();

	// EN: UDeveloperSettings interface / ES: Interfaz UDeveloperSettings
	FName GetCategoryName() const override { return FName(TEXT("PGX")); }
	FName GetSectionName() const override { return FName(TEXT("VersionControl")); }
	FText GetSectionText() const override;
	FText GetSectionDescription() const override;

	//~ Begin IPGXObservable
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;
	//~ End IPGXObservable

	// ─── Workflow ───

	/** EN: Auto-detect PGX systems from changed file paths and prefix commits [Operational]
	 *  ES: Auto-detectar sistemas PGX de rutas de archivos cambiados y prefijar commits [Operational] */
	UPROPERTY(config, EditAnywhere, Category = "Workflow",
		meta = (DisplayName = "Enable Auto-Tagging"))
	bool bEnableAutoTagging = true;

	/** EN: Commit message template. Supports {PREFIX}, {NAME}, {DESC} [Operational]
	 *  ES: Template de mensaje de commit. Soporta {PREFIX}, {NAME}, {DESC} [Operational] */
	UPROPERTY(config, EditAnywhere, Category = "Workflow",
		meta = (DisplayName = "Commit Message Template", MultiLine = true))
	FString CommitMessageTemplate = TEXT("{PREFIX} {NAME}\n\n{DESC}");

	// ─── Validation ───

	/** EN: Run PGX validation rules before commit [Operational]
	 *  ES: Ejecutar reglas de validacion PGX antes del commit [Operational] */
	UPROPERTY(config, EditAnywhere, Category = "Validation",
		meta = (DisplayName = "Enable Pre-Submit Validation"))
	bool bEnablePreSubmitValidation = true;

	/** EN: [Partial] Shallow scan — check #include vs Build.cs deps
	 *  ES: [Partial] Scan superficial — verificar #include vs deps en Build.cs */
	UPROPERTY(config, EditAnywhere, Category = "Validation",
		meta = (DisplayName = "Validate Build.cs Dependencies",
		EditCondition = "bEnablePreSubmitValidation"))
	bool bValidateBuildCsDeps = true;

	/** EN: [Partial] Same-file scan — check AddUObject/AddRaw without RemoveAll
	 *  ES: [Partial] Scan mismo archivo — verificar AddUObject/AddRaw sin RemoveAll */
	UPROPERTY(config, EditAnywhere, Category = "Validation",
		meta = (DisplayName = "Validate Delegate Symmetry",
		EditCondition = "bEnablePreSubmitValidation"))
	bool bValidateDelegateSymmetry = true;

	/** EN: Check for LogTemp usage in PGX modules [Operational]
	 *  ES: Verificar uso de LogTemp en modulos PGX [Operational] */
	UPROPERTY(config, EditAnywhere, Category = "Validation",
		meta = (DisplayName = "Validate Log Categories",
		EditCondition = "bEnablePreSubmitValidation"))
	bool bValidateLogCategories = true;

	/** EN: How validation results affect the commit workflow
	 *  ES: Como los resultados de validacion afectan el workflow de commit */
	UPROPERTY(config, EditAnywhere, Category = "Validation",
		meta = (DisplayName = "Validation Mode"))
	EPGXValidationMode ValidationMode = EPGXValidationMode::WarningOnly;
};
