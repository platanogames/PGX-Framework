// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Logging/LogMacros.h"

/**
 * Runtime module for authored environment variables, zone registration,
 * threshold evaluation and bounded variable modifiers.
 *
 * Passive time drift, propagation, persistence and message-bus integration
 * are outside the Development Preview scope.
 */
class FPGXEnvironmentRuntimeModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};

/** Log category shared by the PGXEnvironment runtime module. */
PGXENVIRONMENTRUNTIME_API DECLARE_LOG_CATEGORY_EXTERN(LogPGXEnvironment, Log, All);
