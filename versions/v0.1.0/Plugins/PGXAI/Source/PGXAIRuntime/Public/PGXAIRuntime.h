// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Logging/PGXLogMacros.h"

PGX_DECLARE_LOG_CATEGORY(PGXAIRUNTIME_API, LogPGXAI);

/**
 * EN: PGXAIRuntime module. High-level AI framework with controllers, behavior tree support, and squad coordination.
 * ES: Modulo PGXAIRuntime. Framework de AI de alto nivel con controladores, soporte de behavior tree y coordinacion de escuadrones.
 */
class FPGXAIRuntimeModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};
