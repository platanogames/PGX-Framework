// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPGXVersionControl, Log, All);

/**
 * EN: PGXVersionControlEditor module. Git integration with Perforce-like changelist grouping for organized commits.
 * ES: Modulo PGXVersionControlEditor. Integracion con Git con agrupacion de changelists estilo Perforce para commits organizados.
 */
class FPGXVersionControlEditorModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};
