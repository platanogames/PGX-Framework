// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXVersionControlEditor.h"
#include "Logging/PGXLogMacros.h"
#include "Inspector/FPGXVersionControlInspectorTabSpawner.h"

#define LOCTEXT_NAMESPACE "FPGXVersionControlEditorModule"

DEFINE_LOG_CATEGORY(LogPGXVersionControl);

void FPGXVersionControlEditorModule::StartupModule()
{
	// EN: Register NomadTab spawner (before SystemObserver in PGXEditorTools)
	// ES: Registrar spawner NomadTab (antes de SystemObserver en PGXEditorTools)
	FPGXVersionControlInspectorTabSpawner::Register();

	PGX_LOG_INFO(LogPGXVersionControl, TEXT("PGXVersionControlEditor: Module started — Inspector tab registered."));
}

void FPGXVersionControlEditorModule::ShutdownModule()
{
	// EN: Unregister NomadTab spawner / ES: Desregistrar spawner NomadTab
	FPGXVersionControlInspectorTabSpawner::Unregister();

	PGX_LOG_INFO(LogPGXVersionControl, TEXT("PGXVersionControlEditor: Module shut down."));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXVersionControlEditorModule, PGXVersionControlEditor)
