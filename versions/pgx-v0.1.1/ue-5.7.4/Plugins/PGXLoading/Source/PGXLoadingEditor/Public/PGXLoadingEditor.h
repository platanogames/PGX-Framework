// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

/**
 * EN: Editor module for PGX Loading + LevelFlow — registers the observability panel NomadTab
 *     + spawner lifecycle. editor panel implementation around PGXLoadingSubsystem
 *     + PGXLevelFlowSubsystem state.
 * ES: Modulo Editor del plugin PGX Loading + LevelFlow — registra el NomadTab del panel
 *     observabilidad + ciclo de vida del spawner. implementacion del panel de editor.
 */
class FPGXLoadingEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
