// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/** EN: Log category for PGX Registry Editor module / ES: Categoria de log para modulo PGX Registry Editor */
DECLARE_LOG_CATEGORY_EXTERN(LogPGXRegistryEditor, Log, All);

/**
 * EN: PGX Registry Editor module.
 *     Provides validation toolchain, diagnostics tab, report export,
 *     and safe autofix for the Data Registry v2.0 DataTable workflow.
 *     Editor-only — not loaded in runtime/shipping builds.
 *
 * ES: Modulo editor del PGX Registry.
 *     Proporciona cadena de validacion, tab de diagnosticos, exportacion
 *     de reportes y autofix seguro para el flujo DataTable del Data Registry v2.0.
 *     Solo editor — no se carga en builds runtime/shipping.
 */
class FPGXRegistryEditorModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};
