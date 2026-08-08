// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXAudioRuntime.h"
#include "Logging/PGXLogMacros.h"
#include "PGXAudioLog.h"

#include "PGXAudioConfig.h"
#include "Data/PGXAudioChannelConfig.h"
#include "Data/PGXAudioDuckingConfig.h"
#include "Data/PGXAudioProfile.h"
#include "Data/PGXLevelAudioConfig.h"
#include "Data/PGXMusicPlaylist.h"
#include "Data/PGXSoundDefinition.h"
#include "Observability/PGXObservabilityRegistry.h"

#define LOCTEXT_NAMESPACE "FPGXAudioRuntimeModule"

// EN: Define the log category for PGX Audio / ES: Definir la categoria de log para PGX Audio
DEFINE_LOG_CATEGORY(LogPGXAudio);

void FPGXAudioRuntimeModule::StartupModule()
{
	// EN: PGXAudioRuntime module started. Audio subsystem will initialize via GameInstance.
	// ES: Modulo PGXAudioRuntime iniciado. El subsistema de audio se inicializara via GameInstance.
	PGX_LOG_INFO(LogPGXAudio, TEXT("PGXAudioRuntime: Module started"));

	// EN: Runtime observability — manual fallback registration of all 7 PGXAudio
	//     observable DA classes with FPGXObservabilityRegistry. Mirror
	//     PGXEnvironment 8.3.B + PGXAI / PGXUI / PGXInput 8.3.C reference.
	// ES: Observabilidad runtime — registro manual fallback de las 7 clases DA
	//     observables PGXAudio con FPGXObservabilityRegistry. Mirror referencia
	//     PGXEnvironment 8.3.B + PGXAI / PGXUI / PGXInput 8.3.C.
	FPGXObservabilityRegistry::Register(UPGXAudioConfig::StaticClass());
	FPGXObservabilityRegistry::Register(UPGXAudioChannelConfig::StaticClass());
	FPGXObservabilityRegistry::Register(UPGXAudioDuckingConfig::StaticClass());
	FPGXObservabilityRegistry::Register(UPGXAudioProfile::StaticClass());
	FPGXObservabilityRegistry::Register(UPGXLevelAudioConfig::StaticClass());
	FPGXObservabilityRegistry::Register(UPGXMusicPlaylist::StaticClass());
	FPGXObservabilityRegistry::Register(UPGXSoundDefinition::StaticClass());
}

void FPGXAudioRuntimeModule::ShutdownModule()
{
	// EN: Cleanup audio module resources before unloading
	// ES: Limpiar recursos del modulo de audio antes de descargar
	PGX_LOG_INFO(LogPGXAudio, TEXT("PGXAudioRuntime: Module shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXAudioRuntimeModule, PGXAudioRuntime)
