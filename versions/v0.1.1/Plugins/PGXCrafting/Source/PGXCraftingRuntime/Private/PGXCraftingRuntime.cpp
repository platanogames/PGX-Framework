// Copyright PGX Framework. All Rights Reserved.

#include "PGXCraftingRuntime.h"
#include "Observability/PGXObservabilityRegistry.h"
#include "PGXCraftingTypes.h"
#include "Logging/PGXLogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogPGXCrafting, Log, All);

#define LOCTEXT_NAMESPACE "FPGXCraftingRuntimeModule"

void FPGXCraftingRuntimeModule::StartupModule()
{
	PGX_LOG_INFO(LogPGXCrafting, TEXT("PGXCraftingRuntime: Module started."));

	// EN: Manual observability registration for the plugin-owned DataAsset class.
	// ES: Registro manual de observabilidad para el DataAsset owned por el plugin.
	FPGXObservabilityRegistry::Register(UPGXRecipeDefinition::StaticClass());
}

void FPGXCraftingRuntimeModule::ShutdownModule()
{
	PGX_LOG_INFO(LogPGXCrafting, TEXT("PGXCraftingRuntime: Module shut down."));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXCraftingRuntimeModule, PGXCraftingRuntime)