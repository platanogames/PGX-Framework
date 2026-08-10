// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"

/**
 * EN: Describes a single creatable asset entry (Blueprint or DataAsset).
 * ES: Describe una entrada de asset creable (Blueprint o DataAsset).
 */
struct FPGXCreatableAssetEntry
{
	FString DisplayName;
	FString ClassPath;       // e.g. "/Script/PGXCoreRuntime.PGXActorBase"
	FString SuggestedName;   // e.g. "BP_PGXActor"
	FString Category;        // e.g. "Core", "AI", "Animation"
	FString IconStyleName;   // e.g. "ClassIcon.Actor"
	bool bIsBlueprint;       // true = Blueprint, false = DataAsset
	FColor TypeActionColor;  // EN: Color for Asset Browser type action / ES: Color para type action en Asset Browser (DataAssets only)
};

/**
 * EN: Central registry of all PGX assets that can be created from menus.
 *     Single source of truth for Toolbar and Content Browser.
 *     Uses class path strings to avoid compile-time dependencies on L2 modules.
 *
 * ES: Registro central de todos los assets PGX creables desde menus.
 *     Fuente unica de verdad para Toolbar y Content Browser.
 *     Usa strings de class path para evitar dependencias de compilacion con modulos L2.
 */
class PGXCOREEDITOR_API FPGXAssetCreationRegistry
{
public:
	// EN: Get all Blueprint entries / ES: Obtener todas las entradas de Blueprint
	static const TArray<FPGXCreatableAssetEntry>& GetBlueprintEntries();

	// EN: Get all DataAsset entries / ES: Obtener todas las entradas de DataAsset
	static const TArray<FPGXCreatableAssetEntry>& GetDataAssetEntries();

	// EN: Get unique Blueprint categories in order / ES: Obtener categorias de Blueprint unicas en orden
	static TArray<FString> GetBlueprintCategories();

	// EN: Get Blueprint entries filtered by category / ES: Obtener entradas de Blueprint filtradas por categoria
	static TArray<FPGXCreatableAssetEntry> GetBlueprintEntriesForCategory(const FString& Category);

	// EN: Get unique DataAsset categories in order / ES: Obtener categorias de DataAsset unicas en orden
	static TArray<FString> GetDataAssetCategories();

	// EN: Get DataAsset entries filtered by category / ES: Obtener entradas de DataAsset filtradas por categoria
	static TArray<FPGXCreatableAssetEntry> GetDataAssetEntriesForCategory(const FString& Category);

	// EN: Get all unique system categories (union of BP + DA) / ES: Obtener todas las categorias de sistema unicas
	static TArray<FString> GetAllSystemCategories();
};
