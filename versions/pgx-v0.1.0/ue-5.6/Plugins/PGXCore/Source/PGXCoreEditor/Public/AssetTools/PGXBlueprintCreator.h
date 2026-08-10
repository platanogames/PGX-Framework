// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"

struct FPGXCreatableAssetEntry;

/**
 * EN: Utility class for creating Blueprints and DataAssets from class path strings.
 *     Resolves classes at runtime (no compile-time dependencies on L2 modules).
 *     Shows standard UE save dialog, creates the asset, and opens it in editor.
 *
 * ES: Clase utilitaria para crear Blueprints y DataAssets desde strings de class path.
 *     Resuelve clases en runtime (sin dependencias de compilacion con modulos L2).
 *     Muestra dialogo de guardado estandar UE, crea el asset y lo abre en editor.
 */
class PGXCOREEDITOR_API FPGXBlueprintCreator
{
public:
	// EN: Create a Blueprint derived from the class at ClassPath / ES: Crear un Blueprint derivado de la clase en ClassPath
	static void CreateBlueprintFromEntry(const FPGXCreatableAssetEntry& Entry);

	// EN: Create a DataAsset instance of the class at ClassPath / ES: Crear una instancia de DataAsset de la clase en ClassPath
	static void CreateDataAssetFromEntry(const FPGXCreatableAssetEntry& Entry);

private:
	// EN: Resolve a UClass from a path string, with graceful error handling
	// ES: Resolver un UClass desde un string de path, con manejo graceful de errores
	static UClass* ResolveClass(const FString& ClassPath, const FString& DisplayName);
};
