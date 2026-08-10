// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"

class FPGXScaffoldTemplateRegistry;

/**
 * EN: Registers the 4 built-in scaffold templates into the registry.
 *     Called once during module startup.
 *
 * ES: Registra los 4 templates de scaffold built-in en el registro.
 *     Se llama una vez durante el startup del modulo.
 */
class PGXSCAFFOLDEDITOR_API FPGXBuiltInTemplates
{
public:
	static void RegisterAll(FPGXScaffoldTemplateRegistry& Registry);

private:
	static void RegisterGameQuickstart(FPGXScaffoldTemplateRegistry& Registry);
	static void RegisterSystemDataSetup(FPGXScaffoldTemplateRegistry& Registry);
	static void RegisterBlueprintArchitecture(FPGXScaffoldTemplateRegistry& Registry);
	static void RegisterContentPack(FPGXScaffoldTemplateRegistry& Registry);
};
