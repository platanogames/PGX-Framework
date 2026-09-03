// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FPGXHarnessSimulation;
class IConsoleObject;

// EN: Log category for PGX Sim Harness module
// ES: Categoria de log para el modulo PGX Sim Harness
DECLARE_LOG_CATEGORY_EXTERN(LogPGXSimHarness, Log, All);

/**
 * EN: PGXSimHarnessEditor module. Demo mode and visual verification harness for PGX Framework.
 *     Provides pre-configured DataAssets with educational values and rich data injection for panel testing.
 *
 * ES: Modulo PGXSimHarnessEditor. Modo demo y harness de verificacion visual para PGX Framework.
 *     Provee DataAssets pre-configurados con valores educativos e inyeccion de datos ricos para testing de paneles.
 */
class FPGXSimHarnessEditorModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;

	/** EN: Stable tab id used by self-registration and automation tests. / ES: Id estable del tab. */
	static FName GetSimHarnessTabId();

private:
	void RegisterHarnessConsoleCommands();
	void UnregisterHarnessConsoleCommands();
	void StartLiveSimulation();
	void StopLiveSimulation();
	void LogLiveSimulationStatus() const;
	void ExportLiveSimulationReport() const;

	TUniquePtr<FPGXHarnessSimulation> LiveSimulation;
	TArray<IConsoleObject*> RegisteredConsoleCommands;
};
