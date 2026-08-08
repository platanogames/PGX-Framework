// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PGXDocsSettings.generated.h"

/**
 * EN: Lightweight editor settings for PGX Documentation Viewer.
 *     Accessible via Project Settings > PGX > Docs.
 *     For rich data-driven configuration, use UPGXDocsConfig (Config DataAsset).
 *
 * ES: Settings ligeros de editor para el Visor de Documentacion PGX.
 *     Accesible via Project Settings > PGX > Docs.
 *     Para configuracion rica data-driven, usar UPGXDocsConfig (Config DataAsset).
 */
UCLASS(config = EditorPerProjectUserSettings, defaultconfig, meta = (DisplayName = "PGX Docs"))
class PGXDOCS_API UPGXDocsSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	FName GetCategoryName() const override { return TEXT("PGX"); }
	FName GetSectionName() const override { return TEXT("Docs"); }

	// ============================================================
	// General
	// ============================================================

	/** EN: Auto-reload docs when files change on disk / ES: Auto-recargar docs cuando cambian archivos en disco */
	UPROPERTY(config, EditAnywhere, Category = "General",
		meta = (ToolTip = "Auto-reload docs when files change on disk"))
	bool bEnableLiveReload = true;

	// ============================================================
	// Cache
	// ============================================================

	/** EN: Path for search index cache (relative to project) / ES: Ruta del cache de indice de busqueda (relativa al proyecto) */
	UPROPERTY(config, EditAnywhere, Category = "Cache",
		meta = (ToolTip = "Path for search index cache (relative to project Saved/)"))
	FString IndexCachePath = TEXT("Saved/PGXDocs/index.json");

	// ============================================================
	// Helpers
	// ============================================================

	/** EN: Get singleton instance / ES: Obtener instancia singleton */
	static const UPGXDocsSettings* Get()
	{
		return GetDefault<UPGXDocsSettings>();
	}

	static UPGXDocsSettings* GetMutable()
	{
		return GetMutableDefault<UPGXDocsSettings>();
	}
};
