// Copyright PGX Framework. All Rights Reserved.

#include "PGXAbilityEditor.h"
#include "Panel/FPGXAbilityPanelTabSpawner.h"

#define LOCTEXT_NAMESPACE "FPGXAbilityEditorModule"

void FPGXAbilityEditorModule::StartupModule()
{
	// EN: Register the observability NomadTab spawner at editor startup.
	// ES: Registrar el spawner del NomadTab de observabilidad en startup del editor.
	FPGXAbilityPanelTabSpawner::Register();
}

void FPGXAbilityEditorModule::ShutdownModule()
{
	// EN: Symmetric unregister on shutdown.
	// ES: Unregister simetrico en shutdown.
	FPGXAbilityPanelTabSpawner::Unregister();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXAbilityEditorModule, PGXAbilityEditor)
