// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

// EN: Log category for PGX Scaffold module
// ES: Categoria de log para el modulo PGX Scaffold
DECLARE_LOG_CATEGORY_EXTERN(LogPGXScaffold, Log, All);

/**
 * EN: PGXScaffoldEditor module. Automated project scaffolding with templates,
 *     dry-run validation, transactional execution, and audit trail.
 *
 * ES: Modulo PGXScaffoldEditor. Scaffolding automatizado de proyecto con templates,
 *     validacion dry-run, ejecucion transaccional, y audit trail.
 */
class FPGXScaffoldEditorModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};
