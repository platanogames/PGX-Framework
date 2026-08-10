// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "Modules/ModuleManager.h"
#include "AssetTypeCategories.h"

// EN: Log category for PGXCoreEditor module / ES: Categoria de log para el modulo PGXCoreEditor
DECLARE_LOG_CATEGORY_EXTERN(LogPGXCoreEditor, Log, All);

class IAssetTypeActions;

/**
 * EN: PGXCoreEditor module interface.
 *     Provides editor extensions, custom detail panels, asset factories,
 *     and editor utilities for the PGX framework.
 *     This module is only loaded in Editor builds.
 *
 * ES: Interfaz del modulo PGXCoreEditor.
 *     Proporciona extensiones de editor, paneles de detalle personalizados,
 *     fabricas de assets y utilidades de editor para el framework PGX.
 *     Este modulo solo se carga en builds de Editor.
 */
class FPGXCoreEditorModule : public IModuleInterface
{
public:
	/** EN: Called when the module is loaded into memory / ES: Se llama cuando el modulo se carga en memoria */
	void StartupModule() override;

	/** EN: Called when the module is unloaded from memory / ES: Se llama cuando el modulo se descarga de memoria */
	void ShutdownModule() override;

	/** EN: Get the PGX asset category for Content Browser / ES: Obtener la categoria de assets PGX */
	EAssetTypeCategories::Type GetPGXAssetCategory() const { return PGXAssetCategory; }

private:
	/** EN: Registered asset type actions for cleanup / ES: Acciones de tipo de asset registradas para limpieza */
	TArray<TSharedPtr<IAssetTypeActions>> RegisteredAssetTypeActions;

	/** EN: Custom asset category bit for PGX / ES: Bit de categoria personalizada para PGX */
	EAssetTypeCategories::Type PGXAssetCategory = EAssetTypeCategories::Misc;
};
