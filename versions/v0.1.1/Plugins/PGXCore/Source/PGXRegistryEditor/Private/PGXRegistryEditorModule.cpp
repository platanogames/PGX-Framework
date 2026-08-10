// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXRegistryEditorModule.h"
#include "Inspector/FPGXRegistryValidationTabSpawner.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogPGXRegistryEditor);

#define LOCTEXT_NAMESPACE "PGXRegistryEditor"

void FPGXRegistryEditorModule::StartupModule()
{
	FPGXRegistryValidationTabSpawner::Register();
}

void FPGXRegistryEditorModule::ShutdownModule()
{
	FPGXRegistryValidationTabSpawner::Unregister();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXRegistryEditorModule, PGXRegistryEditor)
