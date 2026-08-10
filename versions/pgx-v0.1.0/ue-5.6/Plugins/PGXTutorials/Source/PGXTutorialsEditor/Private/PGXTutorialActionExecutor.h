// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Tutorial action executor — creates folders, assets, navigates Content Browser.
// ES: Ejecutor de acciones de tutorial — crea carpetas, assets, navega Content Browser.
#pragma once

#include "CoreMinimal.h"

struct FPGXTutorialStep;
struct FPGXTutorialActionResult;

/**
 * EN: Static executor for tutorial step actions.
 *     Creates folders, DataAssets, opens assets, navigates Content Browser.
 *     Tracks created items for optional cleanup at tutorial end.
 * ES: Ejecutor estatico para acciones de pasos de tutorial.
 *     Crea carpetas, DataAssets, abre assets, navega Content Browser.
 *     Rastrea items creados para limpieza opcional al final del tutorial.
 */
class FPGXTutorialActionExecutor
{
public:
	/** EN: Execute the action of a step. Returns result with success/feedback. */
	static FPGXTutorialActionResult Execute(const FPGXTutorialStep& Step, const FString& BasePath);

	/** EN: Clean up all assets created during the current tutorial / ES: Limpiar assets creados */
	static FPGXTutorialActionResult CleanupTutorialAssets();

	/** EN: Were any assets/folders created during this tutorial? / ES: Se crearon assets en este tutorial? */
	static bool HasCreatedAssets();

	/** EN: Reset tracking (called when starting a new tutorial) / ES: Resetear tracking */
	static void ResetTracking();

	/** EN: Get count of created assets / ES: Obtener conteo de assets creados */
	static int32 GetCreatedAssetCount();

	/** EN: Get count of created folders / ES: Obtener conteo de carpetas creadas */
	static int32 GetCreatedFolderCount();

public:
	/** EN: The current tutorial's BasePath. Public for path-whitelist helper access. */
	static FString CurrentTutorialRoot;

private:
	static FPGXTutorialActionResult CreateFolder(const FString& FullPath);
	static FPGXTutorialActionResult CreateDataAsset(const FString& FolderPath, const FString& AssetClassName, const FString& AssetName);
	static FPGXTutorialActionResult OpenAsset(const FString& FolderPath, const FString& AssetName);
	static FPGXTutorialActionResult NavigateContentBrowser(const FString& FullPath);

	/** EN: Tracking arrays for cleanup / ES: Arrays de tracking para limpieza */
	static TArray<FString> CreatedAssetPaths;
	static TArray<FString> CreatedFolderPaths;
};
