// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "TestDashboard/FPGXTestHarness.h"

// EN: Subsystem headers for injection API / ES: Headers de subsistemas para API de inyeccion
#include "PGXAudioSubsystem.h"
#include "Logging/PGXLogMacros.h"
#include "PGXAudioConfig.h"
#include "Data/PGXAudioChannelConfig.h"
#include "PGXPSOSubsystem.h"
#include "PGXPSOWarmUpConfig.h"
#include "PGXSaveSubsystem.h"
#include "PGXSaveConfig.h"
#include "PGXSaveTypes.h"
#include "PGXSaveGame.h"
#include "Construction/PGXConstructionSettings.h"
#include "Messages/PGXMessageSubsystem.h"
#include "Messages/PGXMessageConfig.h"
#include "EventHandler/PGXEventHandlerSubsystem.h"
#include "EventHandler/PGXEventHandlerConfig.h"

#include "GameplayTagContainer.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"

// ============================================================================
// EN: Test gameplay tag helpers — created on-demand via RequestGameplayTag
//     (cannot use UE_DEFINE_GAMEPLAY_TAG_STATIC in Editor modules)
// ES: Helpers de tags de test — creados on-demand via RequestGameplayTag
//     (no se puede usar UE_DEFINE_GAMEPLAY_TAG_STATIC en modulos Editor)
// ============================================================================

namespace PGXTestTags
{
	static FGameplayTag AudioSFX()    { return FGameplayTag::RequestGameplayTag(FName(TEXT("PGX.Test.Audio.Channel.SFX"))); }
	static FGameplayTag AudioMusic()  { return FGameplayTag::RequestGameplayTag(FName(TEXT("PGX.Test.Audio.Channel.Music"))); }
	static FGameplayTag SaveContext() { return FGameplayTag::RequestGameplayTag(FName(TEXT("PGX.Test.Save.Context"))); }
	static FGameplayTag SaveDomain()  { return FGameplayTag::RequestGameplayTag(FName(TEXT("PGX.Test.Save.Domain.Main"))); }
}

DEFINE_LOG_CATEGORY_STATIC(LogPGXTestHarness, Log, All);

// ============================================================================
// EN: Setup / Teardown
// ES: Setup / Teardown
// ============================================================================

void FPGXTestHarness::Setup(UWorld* PIEWorld)
{
	if (bIsActive)
	{
		PGX_LOG_WARNING(LogPGXTestHarness, TEXT("Setup called while already active — call Teardown first"));
		return;
	}

	if (!PIEWorld)
	{
		PGX_LOG_WARNING(LogPGXTestHarness, TEXT("Setup — null PIEWorld"));
		return;
	}

	PGX_LOG_INFO(LogPGXTestHarness, TEXT("========== TestHarness Setup START =========="));

	SetupConstruction();
	SetupAudio(PIEWorld);
	SetupPSO(PIEWorld);
	SetupSave(PIEWorld);
	SetupMessage(PIEWorld);
	SetupEventHandler(PIEWorld);

	bIsActive = true;
	PGX_LOG_INFO(LogPGXTestHarness, TEXT("========== TestHarness Setup END — %d objects created =========="),
		CreatedObjects.Num());
}

void FPGXTestHarness::Teardown()
{
	if (!bIsActive)
	{
		return;
	}

	PGX_LOG_INFO(LogPGXTestHarness, TEXT("========== TestHarness Teardown START =========="));

	// EN: Find PIE world from engine contexts — subsystem cleanup requires a valid world
	// ES: Buscar PIE world desde contextos del engine — limpieza de subsistemas requiere world valido
	UWorld* PIEWorld = nullptr;
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::PIE && Context.World())
			{
				PIEWorld = Context.World();
				break;
			}
		}
	}

	if (PIEWorld)
	{
		// EN: Normal path — clear injected data from subsystem caches
		// ES: Ruta normal — limpiar datos inyectados de caches de subsistemas
		TeardownMessage(PIEWorld);
		TeardownEventHandler(PIEWorld);
		TeardownAudio(PIEWorld);
		TeardownPSO(PIEWorld);
		TeardownSave(PIEWorld);
		TeardownConstruction();
	}
	else
	{
		// EN: PIE already ended — subsystems are gone, but strong refs below will still be released
		// ES: PIE ya termino — subsistemas ya no existen, pero strong refs debajo seran liberadas
		PGX_LOG_INFO(LogPGXTestHarness, TEXT("Teardown — no PIE world (subsystems already destroyed), releasing object refs only"));
		TeardownConstruction();
	}

	// EN: Release all strong refs — transient objects become GC candidates
	// ES: Liberar todas las strong refs — objetos transient se vuelven candidatos de GC
	CreatedObjects.Empty();
	bIsActive = false;

	PGX_LOG_INFO(LogPGXTestHarness, TEXT("========== TestHarness Teardown END =========="));
}

// ============================================================================
// EN: Audio — 1x UPGXAudioConfig + 2x UPGXAudioChannelConfig (SFX + Music)
// ES: Audio — 1x UPGXAudioConfig + 2x UPGXAudioChannelConfig (SFX + Music)
// ============================================================================

void FPGXTestHarness::SetupAudio(UWorld* World)
{
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UPGXAudioSubsystem* AudioSub = GI ? GI->GetSubsystem<UPGXAudioSubsystem>() : nullptr;
	if (!AudioSub)
	{
		PGX_LOG_WARNING(LogPGXTestHarness, TEXT("SetupAudio — AudioSubsystem not found, skipping"));
		return;
	}

	// EN: Audio system config DA (defaults are valid) / ES: Config DA del sistema de audio (defaults son validos)
	auto* AudioCfg = NewObject<UPGXAudioConfig>(GetTransientPackage(), NAME_None, RF_Transient);
	CreatedObjects.Add(TStrongObjectPtr<UObject>(AudioCfg));
	AudioSub->InjectTestAudioConfig(AudioCfg);

	// EN: SFX channel config / ES: Config de canal SFX
	auto* SfxConfig = NewObject<UPGXAudioChannelConfig>(GetTransientPackage(), NAME_None, RF_Transient);
	SfxConfig->ChannelTag = PGXTestTags::AudioSFX();
	SfxConfig->ChannelDisplayName = FText::FromString(TEXT("Test SFX"));
	SfxConfig->DefaultVolume = 0.8f;
	SfxConfig->MaxConcurrent = 8;
	CreatedObjects.Add(TStrongObjectPtr<UObject>(SfxConfig));
	AudioSub->InjectTestChannelConfig(SfxConfig);

	// EN: Music channel config / ES: Config de canal Music
	auto* MusicConfig = NewObject<UPGXAudioChannelConfig>(GetTransientPackage(), NAME_None, RF_Transient);
	MusicConfig->ChannelTag = PGXTestTags::AudioMusic();
	MusicConfig->ChannelDisplayName = FText::FromString(TEXT("Test Music"));
	MusicConfig->DefaultVolume = 0.6f;
	MusicConfig->MaxConcurrent = 1;
	CreatedObjects.Add(TStrongObjectPtr<UObject>(MusicConfig));
	AudioSub->InjectTestChannelConfig(MusicConfig);

	PGX_LOG_INFO(LogPGXTestHarness, TEXT("SetupAudio — injected 1 AudioConfig + 2 ChannelConfigs"));
}

void FPGXTestHarness::TeardownAudio(UWorld* World)
{
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UPGXAudioSubsystem* AudioSub = GI ? GI->GetSubsystem<UPGXAudioSubsystem>() : nullptr;
	if (AudioSub)
	{
		AudioSub->ClearTestChannelConfigs();
		AudioSub->ClearTestAudioConfig();
	}
}

// ============================================================================
// EN: PSO — 1x UPGXPSOWarmUpConfig (manual activation, empty entries)
// ES: PSO — 1x UPGXPSOWarmUpConfig (activacion manual, entries vacios)
// ============================================================================

void FPGXTestHarness::SetupPSO(UWorld* World)
{
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UPGXPSOSubsystem* PSOSub = GI ? GI->GetSubsystem<UPGXPSOSubsystem>() : nullptr;
	if (!PSOSub)
	{
		PGX_LOG_WARNING(LogPGXTestHarness, TEXT("SetupPSO — PSOSubsystem not found, skipping"));
		return;
	}

	auto* PSOConfig = NewObject<UPGXPSOWarmUpConfig>(GetTransientPackage(), NAME_None, RF_Transient);
	PSOConfig->ActivationMode = EPGXPSOActivationMode::OnExplicitCall;
	PSOConfig->bSaveCacheAfterWarmUp = false;
	// EN: Entries left empty — test only validates discovery, not compilation
	// ES: Entries dejados vacios — test solo valida descubrimiento, no compilacion
	CreatedObjects.Add(TStrongObjectPtr<UObject>(PSOConfig));
	PSOSub->InjectTestConfig(PSOConfig);

	PGX_LOG_INFO(LogPGXTestHarness, TEXT("SetupPSO — injected 1 test PSOWarmUpConfig"));
}

void FPGXTestHarness::TeardownPSO(UWorld* World)
{
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UPGXPSOSubsystem* PSOSub = GI ? GI->GetSubsystem<UPGXPSOSubsystem>() : nullptr;
	if (PSOSub)
	{
		PSOSub->ClearTestConfigs();
	}
}

// ============================================================================
// EN: Save — 1x UPGXSaveConfig with 1 context + 1 domain
// ES: Save — 1x UPGXSaveConfig con 1 contexto + 1 dominio
// ============================================================================

void FPGXTestHarness::SetupSave(UWorld* World)
{
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UPGXSaveSubsystem* SaveSub = GI ? GI->GetSubsystem<UPGXSaveSubsystem>() : nullptr;
	if (!SaveSub)
	{
		PGX_LOG_WARNING(LogPGXTestHarness, TEXT("SetupSave — SaveSubsystem not found, skipping"));
		return;
	}

	auto* SaveConfig = NewObject<UPGXSaveConfig>(GetTransientPackage(), NAME_None, RF_Transient);
	SaveConfig->ContextTag = PGXTestTags::SaveContext();
	SaveConfig->ContextDisplayName = FText::FromString(TEXT("Test Context"));

	// EN: Add one domain with base UPGXSaveGame class / ES: Agregar un dominio con clase base UPGXSaveGame
	FPGXSaveDomainEntry Domain;
	Domain.DomainTag = PGXTestTags::SaveDomain();
	Domain.DisplayName = FText::FromString(TEXT("Test Main Domain"));
	Domain.SaveGameClass = UPGXSaveGame::StaticClass();
	Domain.bRequired = true;
	SaveConfig->SaveDomains.Add(Domain);

	CreatedObjects.Add(TStrongObjectPtr<UObject>(SaveConfig));
	SaveSub->InjectTestConfig(SaveConfig);

	PGX_LOG_INFO(LogPGXTestHarness, TEXT("SetupSave — injected 1 test SaveConfig (1 context, 1 domain)"));
}

void FPGXTestHarness::TeardownSave(UWorld* World)
{
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UPGXSaveSubsystem* SaveSub = GI ? GI->GetSubsystem<UPGXSaveSubsystem>() : nullptr;
	if (SaveSub)
	{
		SaveSub->ClearTestConfigs();
	}
}

// ============================================================================
// EN: Construction — Normalize class sources to ensure settings consistency
//     Saves original sources, sets all to Default (no custom classes needed), restores on teardown
// ES: Construction — Normalizar class sources para asegurar consistencia de settings
//     Guarda sources originales, pone todos en Default (no necesita clases custom), restaura en teardown
// ============================================================================

void FPGXTestHarness::SetupConstruction()
{
	UPGXConstructionSettings* Settings = GetMutableDefault<UPGXConstructionSettings>();
	if (!Settings)
	{
		PGX_LOG_WARNING(LogPGXTestHarness, TEXT("SetupConstruction — ConstructionSettings not found, skipping"));
		return;
	}

	// EN: Save original class sources / ES: Guardar class sources originales
	SavedSources.GameMode = Settings->GameModeClassSource;
	SavedSources.PlayerController = Settings->PlayerControllerClassSource;
	SavedSources.GameState = Settings->GameStateClassSource;
	SavedSources.PlayerState = Settings->PlayerStateClassSource;
	SavedSources.Character = Settings->CharacterClassSource;
	SavedSources.Pawn = Settings->PawnClassSource;
	SavedSources.HUD = Settings->HUDClassSource;
	SavedSources.bSaved = true;

	// EN: Set all to Default — ensures consistency without needing custom classes
	// ES: Poner todos en Default — asegura consistencia sin necesitar clases custom
	Settings->GameModeClassSource = EPGXClassSourceMode::Default;
	Settings->PlayerControllerClassSource = EPGXClassSourceMode::Default;
	Settings->GameStateClassSource = EPGXClassSourceMode::Default;
	Settings->PlayerStateClassSource = EPGXClassSourceMode::Default;
	Settings->CharacterClassSource = EPGXClassSourceMode::Default;
	Settings->PawnClassSource = EPGXClassSourceMode::Default;
	Settings->HUDClassSource = EPGXClassSourceMode::Default;

	PGX_LOG_INFO(LogPGXTestHarness, TEXT("SetupConstruction — normalized 7 class sources to Default"));
}

void FPGXTestHarness::TeardownConstruction()
{
	if (!SavedSources.bSaved)
	{
		return;
	}

	UPGXConstructionSettings* Settings = GetMutableDefault<UPGXConstructionSettings>();
	if (!Settings)
	{
		return;
	}

	// EN: Restore original class sources / ES: Restaurar class sources originales
	Settings->GameModeClassSource = SavedSources.GameMode;
	Settings->PlayerControllerClassSource = SavedSources.PlayerController;
	Settings->GameStateClassSource = SavedSources.GameState;
	Settings->PlayerStateClassSource = SavedSources.PlayerState;
	Settings->CharacterClassSource = SavedSources.Character;
	Settings->PawnClassSource = SavedSources.Pawn;
	Settings->HUDClassSource = SavedSources.HUD;
	SavedSources.bSaved = false;

	PGX_LOG_INFO(LogPGXTestHarness, TEXT("TeardownConstruction — restored 7 class sources to original values"));
}

// ============================================================================
// EN: Message — 1x UPGXMessageConfig (test values)
// ES: Message — 1x UPGXMessageConfig (valores de test)
// ============================================================================

void FPGXTestHarness::SetupMessage(UWorld* World)
{
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UPGXMessageSubsystem* MsgSub = GI ? GI->GetSubsystem<UPGXMessageSubsystem>() : nullptr;
	if (!MsgSub)
	{
		PGX_LOG_WARNING(LogPGXTestHarness, TEXT("SetupMessage — MessageSubsystem not found, skipping"));
		return;
	}

	auto* MsgCfg = NewObject<UPGXMessageConfig>(GetTransientPackage(), NAME_None, RF_Transient);
	MsgCfg->MaxMessageHistory = 50;
	MsgCfg->bLogBroadcasts = true;
	MsgCfg->bEnablePartialMatching = true;
	CreatedObjects.Add(TStrongObjectPtr<UObject>(MsgCfg));
	MsgSub->InjectTestConfig(MsgCfg);

	PGX_LOG_INFO(LogPGXTestHarness, TEXT("SetupMessage — injected 1 test MessageConfig (MaxHistory=50)"));
}

void FPGXTestHarness::TeardownMessage(UWorld* World)
{
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UPGXMessageSubsystem* MsgSub = GI ? GI->GetSubsystem<UPGXMessageSubsystem>() : nullptr;
	if (MsgSub)
	{
		MsgSub->ClearTestConfigs();
	}
}

// ============================================================================
// EN: EventHandler — 1x UPGXEventHandlerConfig (test values, no tables)
// ES: EventHandler — 1x UPGXEventHandlerConfig (valores de test, sin tablas)
// ============================================================================

void FPGXTestHarness::SetupEventHandler(UWorld* World)
{
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UPGXEventHandlerSubsystem* EHSub = GI ? GI->GetSubsystem<UPGXEventHandlerSubsystem>() : nullptr;
	if (!EHSub)
	{
		PGX_LOG_WARNING(LogPGXTestHarness, TEXT("SetupEventHandler — EventHandlerSubsystem not found, skipping"));
		return;
	}

	auto* EHCfg = NewObject<UPGXEventHandlerConfig>(GetTransientPackage(), NAME_None, RF_Transient);
	EHCfg->MaxCachedHandlers = 64;
	EHCfg->MaxExecutionDepth = 4;
	EHCfg->BlackboxBufferSize = 128;
	EHCfg->bLogExecutions = true;
	CreatedObjects.Add(TStrongObjectPtr<UObject>(EHCfg));
	EHSub->InjectTestConfig(EHCfg);

	PGX_LOG_INFO(LogPGXTestHarness, TEXT("SetupEventHandler — injected 1 test EventHandlerConfig (MaxCache=64, MaxDepth=4)"));
}

void FPGXTestHarness::TeardownEventHandler(UWorld* World)
{
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UPGXEventHandlerSubsystem* EHSub = GI ? GI->GetSubsystem<UPGXEventHandlerSubsystem>() : nullptr;
	if (EHSub)
	{
		EHSub->ClearTestConfigs();
	}
}
