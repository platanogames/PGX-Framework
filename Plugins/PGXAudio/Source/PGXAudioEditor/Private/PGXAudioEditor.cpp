// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXAudioEditor.h"
#include "Logging/PGXLogMacros.h"
#include "PGXAudioLog.h"
#include "Inspector/PGXAudioInspector.h"

#define LOCTEXT_NAMESPACE "FPGXAudioEditorModule"

void FPGXAudioEditorModule::StartupModule()
{
	// EN: Register the Audio Inspector NomadTab
	// ES: Registrar el NomadTab del Audio Inspector
	SPGXAudioInspector::RegisterTabSpawner();

	PGX_LOG_INFO(LogPGXAudio, TEXT("PGXAudioEditor: Module started (Inspector registered)"));
}

void FPGXAudioEditorModule::ShutdownModule()
{
	// EN: Unregister the Audio Inspector NomadTab
	// ES: Desregistrar el NomadTab del Audio Inspector
	SPGXAudioInspector::UnregisterTabSpawner();

	PGX_LOG_INFO(LogPGXAudio, TEXT("PGXAudioEditor: Module shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXAudioEditorModule, PGXAudioEditor)
