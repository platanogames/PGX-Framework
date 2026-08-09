// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

/**
 * EN: Editor module for PGX Ability plugin — registers the observability panel NomadTab
 *     + spawner lifecycle. editor panel implementation (expand when
 *     UPGXAbilitySubsystem exposes runtime accessors).
 * ES: Modulo Editor del plugin PGX Ability — registra el NomadTab del panel observabilidad
 *     + ciclo de vida del spawner. implementacion del panel editor (ampliar cuando
 *     UPGXAbilitySubsystem exponga accessors runtime).
 */
class FPGXAbilityEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
