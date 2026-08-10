// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXLoadingEditor.h"
#include "Panel/FPGXLoadingPanelTabSpawner.h"

#define LOCTEXT_NAMESPACE "FPGXLoadingEditorModule"

void FPGXLoadingEditorModule::StartupModule()
{
	FPGXLoadingPanelTabSpawner::Register();
}

void FPGXLoadingEditorModule::ShutdownModule()
{
	FPGXLoadingPanelTabSpawner::Unregister();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXLoadingEditorModule, PGXLoadingEditor)
