// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

// EN: Log category for PGX Editor Tools module
// ES: Categoria de log para el modulo PGX Editor Tools
DECLARE_LOG_CATEGORY_EXTERN(LogPGXEditorTools, Log, All);

/**
 * EN: PGXEditorToolsEditor module. Editor productivity tools for asset auditing, level validation, and dashboard.
 * ES: Modulo PGXEditorToolsEditor. Herramientas de productividad del editor para auditoria de assets, validacion de niveles y dashboard.
 */
class FPGXEditorToolsEditorModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};
