// Copyright PGX Framework. All Rights Reserved.

#include "PGXInputEditor.h"
#include "Inspector/SPGXInputInspector.h"

void FPGXInputEditorModule::StartupModule()
{
	SPGXInputInspector::RegisterTabSpawner();
}

void FPGXInputEditorModule::ShutdownModule()
{
	SPGXInputInspector::UnregisterTabSpawner();
}

IMPLEMENT_MODULE(FPGXInputEditorModule, PGXInputEditor)
