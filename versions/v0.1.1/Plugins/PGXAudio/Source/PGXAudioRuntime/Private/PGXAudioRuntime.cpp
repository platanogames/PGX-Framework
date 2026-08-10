// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXAudioRuntime.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "PGXAudioSubsystem.h"
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


template <typename TSubsystem>
void FPGXAudioRuntimeModule::RegisterSubsystemConsoleCommand(const TCHAR* Name, const TCHAR* Help, uint32 Flags)
{
	IConsoleCommand* Command = IConsoleManager::Get().RegisterConsoleCommand(
		Name, Help,
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([Name](const TArray<FString>& Args, UWorld* World)
		{
			UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
			TSubsystem* Subsystem = GameInstance ? GameInstance->GetSubsystem<TSubsystem>() : nullptr;
			if (Subsystem)
			{
				Subsystem->ExecuteConsoleCommand(FString(Name), Args, World);
			}
		}),
		Flags);
	if (Command)
	{
		RegisteredConsoleCommands.Add(Command);
	}
}

void FPGXAudioRuntimeModule::RegisterConsoleCommands()
{
	if (!RegisteredConsoleCommands.IsEmpty())
	{
		return;
	}
	RegisterSubsystemConsoleCommand<UPGXAudioSubsystem>(TEXT("pgx.audio.backend"), TEXT("Show backend details"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXAudioSubsystem>(TEXT("pgx.audio.channels"), TEXT("Show channel volumes and mute states"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXAudioSubsystem>(TEXT("pgx.audio.debug"), TEXT("Toggle debug overlay: pgx.audio.debug on|off"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXAudioSubsystem>(TEXT("pgx.audio.device"), TEXT("Query/set audio device: pgx.audio.device [device_name]"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXAudioSubsystem>(TEXT("pgx.audio.dialogue"), TEXT("Show dialogue queue"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXAudioSubsystem>(TEXT("pgx.audio.ducking"), TEXT("Show active ducking rules"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXAudioSubsystem>(TEXT("pgx.audio.hdr"), TEXT("Toggle HDR audio: pgx.audio.hdr on|off"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXAudioSubsystem>(TEXT("pgx.audio.history"), TEXT("Show last 20 audio events"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXAudioSubsystem>(TEXT("pgx.audio.hrtf"), TEXT("Toggle HRTF: pgx.audio.hrtf on|off"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXAudioSubsystem>(TEXT("pgx.audio.memory"), TEXT("Show audio memory usage"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXAudioSubsystem>(TEXT("pgx.audio.mix"), TEXT("Show 5-layer mix state"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXAudioSubsystem>(TEXT("pgx.audio.music"), TEXT("Show music manager state"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXAudioSubsystem>(TEXT("pgx.audio.mute"), TEXT("Toggle mute: pgx.audio.mute PGX.Audio.Channel.Music | pgx.audio.mute all"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXAudioSubsystem>(TEXT("pgx.audio.play"), TEXT("Test-play a resolved sound by SoundDefinition tag"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXAudioSubsystem>(TEXT("pgx.audio.playing"), TEXT("Show active sound list"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXAudioSubsystem>(TEXT("pgx.audio.pool"), TEXT("Show sound pool statistics"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXAudioSubsystem>(TEXT("pgx.audio.set"), TEXT("Set channel volume: pgx.audio.set PGX.Audio.Channel.Music 0.5"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXAudioSubsystem>(TEXT("pgx.audio.status"), TEXT("Show PGX Audio system status"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXAudioSubsystem>(TEXT("pgx.audio.stop"), TEXT("Stop all sounds"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXAudioSubsystem>(TEXT("pgx.audio.switch"), TEXT("Switch audio backend: pgx.audio.switch Legacy|Modulation"), ECVF_Default);
}

void FPGXAudioRuntimeModule::UnregisterConsoleCommands()
{
	IConsoleManager& Manager = IConsoleManager::Get();
	for (IConsoleCommand* Command : RegisteredConsoleCommands)
	{
		if (Command)
		{
			Manager.UnregisterConsoleObject(Command);
		}
	}
	RegisteredConsoleCommands.Reset();
}

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

	RegisterConsoleCommands();
}

void FPGXAudioRuntimeModule::ShutdownModule()
{
	UnregisterConsoleCommands();
	// EN: Cleanup audio module resources before unloading
	// ES: Limpiar recursos del modulo de audio antes de descargar
	PGX_LOG_INFO(LogPGXAudio, TEXT("PGXAudioRuntime: Module shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXAudioRuntimeModule, PGXAudioRuntime)
