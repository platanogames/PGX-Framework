// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXSettings.h"
#include "Observability/PGXObservable.h"
#include "Registry/PGXRegistryTypes.h"
#include "PGXRegistrySettings.generated.h"

/**
 * EN: Project-wide settings for the PGX Data Registry v2.0.
 *     Accessible via Project Settings > PGX > Registry.
 *     Controls scan roots, validation budgets, tag policy, and conflict defaults.
 *
 * ES: Configuracion global del proyecto para PGX Data Registry v2.0.
 *     Accesible via Project Settings > PGX > Registry.
 *     Controla raices de escaneo, presupuestos de validacion, politica de tags y defaults de conflicto.
 */
UCLASS(config = PGX, defaultconfig, meta = (DisplayName = "PGX Registry"))
class PGXCORERUNTIME_API UPGXRegistrySettings : public UPGXSettings, public IPGXObservable
{
	GENERATED_BODY()

public:
	//~ Begin UDeveloperSettings Interface
	FName GetSectionName() const override { return FName(TEXT("Registry")); }
	FText GetSectionText() const override;
	FText GetSectionDescription() const override;
	//~ End UDeveloperSettings Interface

	//~ Begin IPGXObservable (delegates to PGXCoreObservability helpers)
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;
	//~ End IPGXObservable

	// ─── Discovery / Descubrimiento ───

	/**
	 * EN: Root content paths to scan for UPGXRegistryDefinition DAs.
	 *     If empty, scans entire project content.
	 * ES: Rutas raiz de contenido para escanear DAs UPGXRegistryDefinition.
	 *     Si esta vacio, escanea todo el contenido del proyecto.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Discovery",
		meta = (ContentDir, DisplayName = "Definition Scan Roots"))
	TArray<FDirectoryPath> ScanRoots;

	/**
	 * EN: If true, automatically ingest all discovered definitions at subsystem init.
	 *     If false, definitions must be ingested manually via API or console command.
	 * ES: Si es true, ingestar automaticamente todas las definiciones descubiertas al init del subsistema.
	 *     Si es false, las definiciones deben ingestarse manualmente via API o comando de consola.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Discovery",
		meta = (DisplayName = "Auto-Ingest On Init"))
	bool bAutoIngestOnInit = true;

	// ─── Tag Policy / Politica de Tags ───

	/**
	 * EN: Approved GameplayTag roots for registry items.
	 *     Tags outside these roots trigger RDT012 warning.
	 *     If empty, no root restriction is enforced.
	 * ES: Raices aprobadas de GameplayTag para items del registro.
	 *     Tags fuera de estas raices disparan advertencia RDT012.
	 *     Si esta vacio, no se aplica restriccion de raiz.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Tags",
		meta = (DisplayName = "Allowed Tag Roots"))
	TArray<FGameplayTag> AllowedTagRoots;

	// ─── Policy / Politica ───

	/**
	 * EN: Default conflict policy for databases without explicit override in their definition.
	 * ES: Politica de conflicto por defecto para BDs sin override explicito en su definicion.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Policy",
		meta = (DisplayName = "Default Conflict Policy"))
	EPGXRegistryConflictPolicy DefaultConflictPolicy = EPGXRegistryConflictPolicy::FailOnConflict;

	/**
	 * EN: Strict mode — treat validation warnings as errors.
	 *     Useful for CI pipelines and pre-release builds.
	 * ES: Modo estricto — tratar warnings de validacion como errores.
	 *     Util para pipelines CI y builds pre-release.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Policy",
		meta = (DisplayName = "Strict Mode"))
	bool bStrictMode = false;

	// ─── Budgets / Presupuestos ───

	/**
	 * EN: Max entries per category before budget warning (RDT040).
	 * ES: Max entradas por categoria antes de advertencia de presupuesto (RDT040).
	 */
	UPROPERTY(config, EditAnywhere, Category = "Budgets",
		meta = (ClampMin = "100", ClampMax = "100000", DisplayName = "Category Budget"))
	int32 CategoryBudget = 1000;

	/**
	 * EN: Max total entries per table before budget warning (RDT041).
	 * ES: Max entradas totales por tabla antes de advertencia de presupuesto (RDT041).
	 */
	UPROPERTY(config, EditAnywhere, Category = "Budgets",
		meta = (ClampMin = "100", ClampMax = "1000000", DisplayName = "Table Budget"))
	int32 TableBudget = 10000;

	// ─── Debug / Depuracion ───

	/**
	 * EN: Enable verbose logging during DataTable ingestion.
	 * ES: Habilitar logging detallado durante ingesta de DataTable.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Debug",
		meta = (DisplayName = "Verbose Ingest Logging"))
	bool bVerboseIngestLogging = false;
};
