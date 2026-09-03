// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXCameraEditor.h"
#include "Panel/FPGXCameraPanelTabSpawner.h"

#define LOCTEXT_NAMESPACE "PGXCamera"

IMPLEMENT_MODULE(FPGXCameraEditorModule, PGXCameraEditor)

void FPGXCameraEditorModule::StartupModule()
{
	FPGXCameraPanelTabSpawner::Register();
}

void FPGXCameraEditorModule::ShutdownModule()
{
	FPGXCameraPanelTabSpawner::Unregister();
}

#undef LOCTEXT_NAMESPACE
