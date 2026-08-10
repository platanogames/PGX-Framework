// Copyright PGX Framework. All Rights Reserved.

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
