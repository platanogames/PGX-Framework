// Copyright PGX Framework. All Rights Reserved.

#include "PGXAIEditor.h"
#include "Inspector/SPGXAIInspector.h"

void FPGXAIEditorModule::StartupModule()
{
	SPGXAIInspector::RegisterTabSpawner();
}

void FPGXAIEditorModule::ShutdownModule()
{
	SPGXAIInspector::UnregisterTabSpawner();
}

IMPLEMENT_MODULE(FPGXAIEditorModule, PGXAIEditor)
