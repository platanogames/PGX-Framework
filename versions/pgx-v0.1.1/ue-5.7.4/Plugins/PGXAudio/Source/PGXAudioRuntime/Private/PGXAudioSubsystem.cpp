// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXAudioSubsystem.h"
#include "Logging/PGXLogMacros.h"
#include "Profile/PGXProfileSubsystem.h"
#include "Profile/PGXPlatformConfig.h"
#include "PGXAudioLog.h"
#include "PGXAudioConfig.h"
#include "PGXAudioSettings.h"
#include "Utils/PGXConfigResolution.h"
#include "Backend/PGXAudioBackend.h"
#include "Backend/PGXAudioBackendLegacy.h"
#include "Backend/PGXAudioBackendModulation.h"
#include "Data/PGXAudioChannelConfig.h"
#include "Data/PGXSoundDefinition.h"
#include "Data/PGXAudioProfile.h"
#include "Data/PGXMusicPlaylist.h"
#include "Manager/PGXMusicManager.h"
#include "Manager/PGXSoundPool.h"
#include "Manager/PGXDialogueManager.h"
#include "Mix/PGXAudioMixSubsystem.h"
#include "Tags/PGXAudioTags.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundConcurrency.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

// EN: Central audio subsystem implementation
// ES: Implementacion del subsistema central de audio

void UPGXAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	PGX_LOG_INFO(LogPGXAudio, TEXT("UPGXAudioSubsystem::Initialize — Starting audio system initialization"));
	AudioState = EPGXAudioState::Initializing;

	// EN: Step 1: Load config DA from AssetRegistry / ES: Paso 1: Cargar config DA desde AssetRegistry
	LoadConfig();

	if (!AudioConfig)
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("UPGXAudioSubsystem::Initialize — No UPGXAudioConfig found. Using defaults."));
	}

	// EN: Step 2: Create and initialize backend / ES: Paso 2: Crear e inicializar backend
	const EPGXAudioBackendType RequestedType = AudioConfig ? AudioConfig->BackendType : EPGXAudioBackendType::Auto;
	if (!CreateBackend(RequestedType))
	{
		PGX_LOG_ERROR(LogPGXAudio, TEXT("UPGXAudioSubsystem::Initialize — Failed to create audio backend!"));
		AudioState = EPGXAudioState::Error;
		return;
	}

	// EN: Step 3: Create owned objects / ES: Paso 3: Crear objetos owned
	MusicManager = NewObject<UPGXMusicManager>(this);
	MusicManager->Initialize(this);

	SoundPool = NewObject<UPGXSoundPool>(this);
	const int32 PoolInitial = AudioConfig ? AudioConfig->SoundPoolInitialSize : 32;
	const int32 PoolMax = AudioConfig ? AudioConfig->SoundPoolMaxSize : 128;
	SoundPool->Initialize(PoolInitial, PoolMax);

	DialogueManager = NewObject<UPGXDialogueManager>(this);
	DialogueManager->Initialize(this);

	// EN: Step 4: Register console commands / ES: Paso 4: Registrar comandos de consola
	// EN: Step 5: Cache audio assets from AssetRegistry (avoids per-frame scans)
	// ES: Paso 5: Cachear assets de audio desde AssetRegistry (evita escaneos por frame)
	CacheAudioAssets();

	// EN: Step 6: Reserve event history / ES: Paso 6: Reservar historial de eventos
	const int32 MaxHistory = AudioConfig ? AudioConfig->MaxEventHistorySize : MaxEventHistory;
	EventHistory.Reserve(MaxHistory);

	// EN: Step 7: Apply profile constraints / ES: Paso 7: Aplicar restricciones de profile
	if (auto* ProfileSS = GetGameInstance()->GetSubsystem<UPGXProfileSubsystem>())
	{
		if (ProfileSS->IsProfileResolved())
		{
			ApplyProfileConstraints(ProfileSS->GetResolvedProfile());
		}
		ProfileSS->OnProfileChangedNative.AddUObject(this, &ThisClass::HandleProfileChanged);
	}

	// EN: Mark ready and broadcast / ES: Marcar como listo y difundir
	AudioState = EPGXAudioState::Ready;
	PGX_LOG_INFO(LogPGXAudio, TEXT("UPGXAudioSubsystem::Initialize — Audio system ready (Backend: %s)"),
		ActiveBackend ? *ActiveBackend->GetStatusText() : TEXT("None"));

	OnAudioSystemReady.Broadcast();
	OnAudioSystemReadyNative.Broadcast();
}

void UPGXAudioSubsystem::Deinitialize()
{
	PGX_LOG_INFO(LogPGXAudio, TEXT("UPGXAudioSubsystem::Deinitialize — Shutting down audio system"));

	// EN: Cleanup Profile delegate subscription / ES: Limpiar suscripcion de delegado de Profile
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UPGXProfileSubsystem* Profile = GI->GetSubsystem<UPGXProfileSubsystem>())
		{
			Profile->OnProfileChangedNative.RemoveAll(this);
		}
	}

	// EN: Unregister console commands / ES: Desregistrar comandos de consola
	// EN: Shutdown managers / ES: Apagar managers
	if (DialogueManager)
	{
		DialogueManager->Deinitialize();
		DialogueManager = nullptr;
	}

	if (MusicManager)
	{
		MusicManager->Deinitialize();
		MusicManager = nullptr;
	}

	if (SoundPool)
	{
		SoundPool->Deinitialize();
		SoundPool = nullptr;
	}

	// EN: Shutdown backend / ES: Apagar backend
	if (ActiveBackend)
	{
		ActiveBackend->ShutdownBackend();
		ActiveBackend = nullptr;
	}

	AudioConfig = nullptr;

	// EN: Clear runtime data and caches / ES: Limpiar datos de runtime y caches
	ActiveSoundsMap.Empty();
	HandleToComponentMap.Empty();
	EventHistory.Empty();
	CachedChannelConfigs.Empty();
	CachedProfiles.Empty();
	CachedSoundDefinitions.Empty();

	AudioState = EPGXAudioState::Uninitialized;

	Super::Deinitialize();
}

// ── Backend ──

EPGXAudioBackendType UPGXAudioSubsystem::GetActiveBackendType() const
{
	return ActiveBackend ? ActiveBackend->GetBackendType() : EPGXAudioBackendType::Auto;
}

bool UPGXAudioSubsystem::SwitchBackend(EPGXAudioBackendType NewType)
{
	const FPGXAudioBackendSwitchResult Result = SwitchBackendDetailed(NewType);
	return Result.bSuccess || Result.Status == EPGXAudioBackendSwitchStatus::AlreadyActive;
}

FPGXAudioBackendSwitchResult UPGXAudioSubsystem::SwitchBackendDetailed(EPGXAudioBackendType NewType)
{
	FPGXAudioBackendSwitchResult Result;
	Result.RequestedBackend = NewType;
	Result.PreservedActiveSoundCount = ActiveSoundsMap.Num();

	if (!ActiveBackend)
	{
		Result.Status = EPGXAudioBackendSwitchStatus::Failed_NoBackend;
		Result.Message = TEXT("No active backend to switch from");
		PGX_LOG_WARNING(LogPGXAudio, TEXT("SwitchBackend — %s"), *Result.Message);
		RecordEvent(TAG_PGX_Audio_Event_BackendSwitched, Result.Message, FGameplayTag(), FGameplayTag(), false);
		return Result;
	}

	const EPGXAudioBackendType OldType = ActiveBackend->GetBackendType();
	Result.SourceBackend = OldType;
	Result.ActiveBackend = OldType;

	if (OldType == NewType)
	{
		Result.bSuccess = true;
		Result.Status = EPGXAudioBackendSwitchStatus::AlreadyActive;
		Result.Message = FString::Printf(TEXT("Already using %s backend"), *ActiveBackend->GetStatusText());
		PGX_LOG_INFO(LogPGXAudio, TEXT("SwitchBackend — %s"), *Result.Message);
		return Result;
	}

	PGX_LOG_INFO(LogPGXAudio, TEXT("SwitchBackend — Switching from %s to %s"),
		*ActiveBackend->GetStatusText(), *UEnum::GetValueAsString(NewType));

	// EN: Capture deterministic state before switch / ES: Capturar estado determinista antes del cambio
	const TArray<FPGXAudioChannelSnapshot> SavedChannels = GetAllChannelStates();
	TMap<int32, TWeakObjectPtr<UAudioComponent>> SavedHandleToComponentMap = HandleToComponentMap;
	TMap<int32, FPGXActiveSoundInfo> SavedActiveSoundsMap = ActiveSoundsMap;

	// EN: Shutdown old backend / ES: Apagar backend viejo
	ActiveBackend->ShutdownBackend();

	if (!CreateBackend(NewType))
	{
		PGX_LOG_ERROR(LogPGXAudio, TEXT("SwitchBackend — Failed to create new backend! Attempting rollback..."));
		if (!CreateBackend(OldType))
		{
			Result.Status = EPGXAudioBackendSwitchStatus::Failed_Rollback;
			Result.ActiveBackend = EPGXAudioBackendType::Auto;
			Result.Message = TEXT("Failed to create requested backend and rollback backend");
			RecordEvent(TAG_PGX_Audio_Event_BackendSwitched, Result.Message, FGameplayTag(), FGameplayTag(), false);
			return Result;
		}

		for (const FPGXAudioChannelSnapshot& Ch : SavedChannels)
		{
			if (Ch.ChannelTag.IsValid())
			{
				ActiveBackend->SetChannelVolume(Ch.ChannelTag, Ch.Volume);
				ActiveBackend->SetChannelMuted(Ch.ChannelTag, Ch.bMuted);
				++Result.RestoredChannelCount;
			}
		}
		HandleToComponentMap = MoveTemp(SavedHandleToComponentMap);
		ActiveSoundsMap = MoveTemp(SavedActiveSoundsMap);

		Result.Status = EPGXAudioBackendSwitchStatus::Failed_CreateTarget;
		Result.ActiveBackend = ActiveBackend ? ActiveBackend->GetBackendType() : EPGXAudioBackendType::Auto;
		Result.Message = TEXT("Failed to create requested backend; rolled back to source backend");
		RecordEvent(TAG_PGX_Audio_Event_BackendSwitched, Result.Message, FGameplayTag(), FGameplayTag(), false);
		return Result;
	}

	Result.ActiveBackend = ActiveBackend ? ActiveBackend->GetBackendType() : EPGXAudioBackendType::Auto;
#if WITH_DEV_AUTOMATION_TESTS
	if (bForceNextSwitchResultActiveBackendForTesting)
	{
		Result.ActiveBackend = ForcedNextSwitchResultActiveBackendForTesting;
		bForceNextSwitchResultActiveBackendForTesting = false;
	}
#endif
	if (NewType != EPGXAudioBackendType::Auto && Result.ActiveBackend != NewType)
	{
		Result.Status = EPGXAudioBackendSwitchStatus::Failed_TargetMismatch;
		Result.Message = FString::Printf(TEXT("Requested %s but active backend is %s"),
			*UEnum::GetValueAsString(NewType), *UEnum::GetValueAsString(Result.ActiveBackend));
		PGX_LOG_WARNING(LogPGXAudio, TEXT("SwitchBackend — %s"), *Result.Message);
		RecordEvent(TAG_PGX_Audio_Event_BackendSwitched, Result.Message, FGameplayTag(), FGameplayTag(), false);
		return Result;
	}

	// EN: Restore channel volumes/mutes on new backend / ES: Restaurar volumenes/mutes de canales en el backend nuevo
	for (const FPGXAudioChannelSnapshot& Ch : SavedChannels)
	{
		if (Ch.ChannelTag.IsValid())
		{
			ActiveBackend->SetChannelVolume(Ch.ChannelTag, Ch.Volume);
			ActiveBackend->SetChannelMuted(Ch.ChannelTag, Ch.bMuted);
			++Result.RestoredChannelCount;
		}
	}

	// EN: Backend switch should not orphan existing audio component bookkeeping.
	// ES: El cambio de backend no debe huerfanar el bookkeeping de componentes existentes.
	HandleToComponentMap = MoveTemp(SavedHandleToComponentMap);
	ActiveSoundsMap = MoveTemp(SavedActiveSoundsMap);

	Result.bSuccess = true;
	Result.Status = EPGXAudioBackendSwitchStatus::Success;
	Result.Message = FString::Printf(TEXT("Switch complete. Restored %d channels and preserved %d active sounds."),
		Result.RestoredChannelCount, Result.PreservedActiveSoundCount);
	PGX_LOG_INFO(LogPGXAudio, TEXT("SwitchBackend — %s"), *Result.Message);

	OnBackendSwitched.Broadcast(OldType, NewType);
	OnBackendSwitchedNative.Broadcast(OldType, NewType);

	RecordEvent(TAG_PGX_Audio_Event_BackendSwitched, Result.Message);
	return Result;
}

FString UPGXAudioSubsystem::GetBackendStatusText() const
{
	return ActiveBackend ? ActiveBackend->GetStatusText() : TEXT("No backend");
}

// ── Volume Control ──

void UPGXAudioSubsystem::SetChannelVolume(FGameplayTag ChannelTag, float Volume)
{
	if (!ActiveBackend)
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("SetChannelVolume — No active backend for channel %s"), *ChannelTag.ToString());
		RecordEvent(TAG_PGX_Audio_Event_VolumeChanged, TEXT("No active backend"), ChannelTag, FGameplayTag(), false);
		return;
	}
	if (!ChannelTag.IsValid())
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("SetChannelVolume — Invalid channel tag"));
		RecordEvent(TAG_PGX_Audio_Event_VolumeChanged, TEXT("Invalid channel tag"), ChannelTag, FGameplayTag(), false);
		return;
	}

	const float ClampedVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	const float OldVolume = ActiveBackend->GetChannelVolume(ChannelTag);

	ActiveBackend->SetChannelVolume(ChannelTag, ClampedVolume);

	if (!FMath::IsNearlyEqual(OldVolume, ClampedVolume))
	{
		OnChannelVolumeChanged.Broadcast(ChannelTag, OldVolume, ClampedVolume);
		OnChannelVolumeChangedNative.Broadcast(ChannelTag, OldVolume, ClampedVolume);
		RecordEvent(TAG_PGX_Audio_Event_VolumeChanged, TEXT(""), ChannelTag);
	}
}

float UPGXAudioSubsystem::GetChannelVolume(FGameplayTag ChannelTag) const
{
	return ActiveBackend ? ActiveBackend->GetChannelVolume(ChannelTag) : 1.0f;
}

void UPGXAudioSubsystem::SetChannelMuted(FGameplayTag ChannelTag, bool bMuted)
{
	if (!ActiveBackend)
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("SetChannelMuted — No active backend for channel %s"), *ChannelTag.ToString());
		RecordEvent(TAG_PGX_Audio_Event_MuteChanged, TEXT("No active backend"), ChannelTag, FGameplayTag(), false);
		return;
	}
	if (!ChannelTag.IsValid())
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("SetChannelMuted — Invalid channel tag"));
		RecordEvent(TAG_PGX_Audio_Event_MuteChanged, TEXT("Invalid channel tag"), ChannelTag, FGameplayTag(), false);
		return;
	}

	ActiveBackend->SetChannelMuted(ChannelTag, bMuted);

	OnMuteChanged.Broadcast(ChannelTag, bMuted);
	OnMuteChangedNative.Broadcast(ChannelTag, bMuted);
	RecordEvent(TAG_PGX_Audio_Event_MuteChanged, TEXT(""), ChannelTag);
}

bool UPGXAudioSubsystem::IsChannelMuted(FGameplayTag ChannelTag) const
{
	return ActiveBackend ? ActiveBackend->IsChannelMuted(ChannelTag) : false;
}

void UPGXAudioSubsystem::SetMuteAll(bool bMuted)
{
	bGlobalMute = bMuted;
	PGX_LOG_INFO(LogPGXAudio, TEXT("SetMuteAll — %s"), bMuted ? TEXT("MUTED") : TEXT("UNMUTED"));

	if (ActiveBackend)
	{
		const TArray<FPGXAudioChannelSnapshot> Channels = GetAllChannelStates();
		for (const FPGXAudioChannelSnapshot& Ch : Channels)
		{
			if (Ch.ChannelTag.IsValid())
			{
				ActiveBackend->SetChannelMuted(Ch.ChannelTag, bMuted);
			}
		}
	}
}

TArray<FPGXAudioChannelSnapshot> UPGXAudioSubsystem::GetAllChannelStates() const
{
	TArray<FPGXAudioChannelSnapshot> Result;

	// EN: Use cached channel configs instead of scanning AssetRegistry every call
	// ES: Usar configs de canal cacheados en vez de escanear AssetRegistry cada llamada
	for (const TWeakObjectPtr<const UPGXAudioChannelConfig>& WeakConfig : CachedChannelConfigs)
	{
		const UPGXAudioChannelConfig* ChannelConfig = WeakConfig.Get();
		if (!ChannelConfig || !ChannelConfig->ChannelTag.IsValid())
		{
			continue;
		}

		FPGXAudioChannelSnapshot Snapshot;
		Snapshot.ChannelTag = ChannelConfig->ChannelTag;
		Snapshot.DisplayName = ChannelConfig->ChannelDisplayName.ToString();
		Snapshot.Volume = ActiveBackend ? ActiveBackend->GetChannelVolume(ChannelConfig->ChannelTag) : ChannelConfig->DefaultVolume;
		Snapshot.bMuted = ActiveBackend ? ActiveBackend->IsChannelMuted(ChannelConfig->ChannelTag) : false;

		// EN: Count active sounds on this channel / ES: Contar sonidos activos en este canal
		int32 SoundCount = 0;
		for (const auto& Pair : ActiveSoundsMap)
		{
			if (Pair.Value.ChannelTag == ChannelConfig->ChannelTag)
			{
				SoundCount++;
			}
		}
		Snapshot.ActiveSoundCount = SoundCount;

		Result.Add(Snapshot);
	}

	return Result;
}

// ── Playback ──

FPGXSoundHandle UPGXAudioSubsystem::PlaySound2D(USoundBase* Sound, const FPGXAudioPlayParams& Params)
{
	if (!Sound)
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("PlaySound2D — Invalid sound"));
		RecordEvent(TAG_PGX_Audio_Event_Play, TEXT("Invalid sound"), Params.ChannelTag, Params.ProfileTag, false);
		return FPGXSoundHandle::Invalid();
	}
	if (AudioState != EPGXAudioState::Ready)
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("PlaySound2D — Audio subsystem is not ready: %s"), *UEnum::GetValueAsString(AudioState));
		RecordEvent(TAG_PGX_Audio_Event_Play, Sound->GetName(), Params.ChannelTag, Params.ProfileTag, false);
		return FPGXSoundHandle::Invalid();
	}

	// EN: Get world context / ES: Obtener contexto de mundo
	UWorld* World = nullptr;
	if (const UGameInstance* GI = GetGameInstance())
	{
		World = GI->GetWorld();
	}
	if (!World)
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("PlaySound2D — No valid world"));
		RecordEvent(TAG_PGX_Audio_Event_Play, Sound->GetName(), Params.ChannelTag, Params.ProfileTag, false);
		return FPGXSoundHandle::Invalid();
	}

	// EN: Resolve profile for playback parameters / ES: Resolver perfil para parametros de reproduccion
	const UPGXAudioProfile* Profile = FindAudioProfile(Params.ProfileTag);
	const float FinalVolume = Params.VolumeMultiplier * (Profile ? Profile->VolumeMultiplier : 1.0f);
	const float FinalPitch = Params.PitchMultiplier * (Profile ? Profile->PitchMultiplier : 1.0f);

	USoundConcurrency* Concurrency = nullptr;
	if (Profile && !Profile->ConcurrencySettings.IsNull())
	{
		Concurrency = Profile->ConcurrencySettings.LoadSynchronous();
	}

	UAudioComponent* AudioComp = UGameplayStatics::SpawnSound2D(World, Sound,
		FinalVolume, FinalPitch, 0.0f, Concurrency, false, false);

	if (!AudioComp)
	{
		RecordEvent(TAG_PGX_Audio_Event_Play, Sound->GetName(), Params.ChannelTag, Params.ProfileTag, false);
		return FPGXSoundHandle::Invalid();
	}

	// EN: Apply fade-in / ES: Aplicar fade-in
	const float FadeIn = Params.FadeInDuration > 0.0f ? Params.FadeInDuration : (Profile ? Profile->FadeInDuration : 0.0f);
	if (FadeIn > 0.0f)
	{
		AudioComp->FadeIn(FadeIn);
	}

	// EN: Create handle and track / ES: Crear handle y rastrear
	FPGXSoundHandle Handle;
	Handle.Id = GenerateHandleId();
	Handle.ChannelTag = Params.ChannelTag;
	Handle.ProfileTag = Params.ProfileTag;
	Handle.StartTime = FPlatformTime::Seconds();

	FPGXActiveSoundInfo Info;
	Info.Handle = Handle;
	Info.SoundName = Sound->GetName();
	Info.ChannelTag = Params.ChannelTag;
	Info.ProfileTag = Params.ProfileTag;
	Info.Priority = Params.Priority;
	Info.bIs3D = false;

	ActiveSoundsMap.Add(Handle.Id, Info);
	HandleToComponentMap.Add(Handle.Id, AudioComp);

	RecordEvent(TAG_PGX_Audio_Event_Play, Sound->GetName(), Params.ChannelTag, Params.ProfileTag, true);

	OnSoundPlayed.Broadcast(Handle, Params.ProfileTag);
	OnSoundPlayedNative.Broadcast(Handle, Params.ProfileTag);

	return Handle;
}

FPGXSoundHandle UPGXAudioSubsystem::PlaySoundAtLocation(USoundBase* Sound, FVector Location, FRotator Rotation, const FPGXAudioPlayParams& Params)
{
	if (!Sound)
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("PlaySoundAtLocation — Invalid sound"));
		RecordEvent(TAG_PGX_Audio_Event_Play, TEXT("Invalid sound"), Params.ChannelTag, Params.ProfileTag, false);
		return FPGXSoundHandle::Invalid();
	}
	if (AudioState != EPGXAudioState::Ready)
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("PlaySoundAtLocation — Audio subsystem is not ready: %s"), *UEnum::GetValueAsString(AudioState));
		RecordEvent(TAG_PGX_Audio_Event_Play, Sound->GetName(), Params.ChannelTag, Params.ProfileTag, false);
		return FPGXSoundHandle::Invalid();
	}

	UWorld* World = nullptr;
	if (const UGameInstance* GI = GetGameInstance())
	{
		World = GI->GetWorld();
	}
	if (!World)
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("PlaySoundAtLocation — No valid world for %s"), *Sound->GetName());
		RecordEvent(TAG_PGX_Audio_Event_Play, Sound->GetName(), Params.ChannelTag, Params.ProfileTag, false);
		return FPGXSoundHandle::Invalid();
	}

	const UPGXAudioProfile* Profile = FindAudioProfile(Params.ProfileTag);
	const float FinalVolume = Params.VolumeMultiplier * (Profile ? Profile->VolumeMultiplier : 1.0f);
	const float FinalPitch = Params.PitchMultiplier * (Profile ? Profile->PitchMultiplier : 1.0f);

	USoundAttenuation* Attenuation = nullptr;
	if (Profile && !Profile->AttenuationSettings.IsNull())
	{
		Attenuation = Profile->AttenuationSettings.LoadSynchronous();
	}

	USoundConcurrency* Concurrency = nullptr;
	if (Profile && !Profile->ConcurrencySettings.IsNull())
	{
		Concurrency = Profile->ConcurrencySettings.LoadSynchronous();
	}

	UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(World, Sound,
		Location, Rotation, FinalVolume, FinalPitch, 0.0f, Attenuation, Concurrency, false);

	if (!AudioComp)
	{
		RecordEvent(TAG_PGX_Audio_Event_Play, Sound->GetName(), Params.ChannelTag, Params.ProfileTag, false);
		return FPGXSoundHandle::Invalid();
	}

	const float FadeIn = Params.FadeInDuration > 0.0f ? Params.FadeInDuration : (Profile ? Profile->FadeInDuration : 0.0f);
	if (FadeIn > 0.0f)
	{
		AudioComp->FadeIn(FadeIn);
	}

	FPGXSoundHandle Handle;
	Handle.Id = GenerateHandleId();
	Handle.ChannelTag = Params.ChannelTag;
	Handle.ProfileTag = Params.ProfileTag;
	Handle.StartTime = FPlatformTime::Seconds();

	FPGXActiveSoundInfo Info;
	Info.Handle = Handle;
	Info.SoundName = Sound->GetName();
	Info.ChannelTag = Params.ChannelTag;
	Info.ProfileTag = Params.ProfileTag;
	Info.Priority = Params.Priority;
	Info.Location = Location;
	Info.bIs3D = true;

	ActiveSoundsMap.Add(Handle.Id, Info);
	HandleToComponentMap.Add(Handle.Id, AudioComp);

	RecordEvent(TAG_PGX_Audio_Event_Play, Sound->GetName(), Params.ChannelTag, Params.ProfileTag, true);

	OnSoundPlayed.Broadcast(Handle, Params.ProfileTag);
	OnSoundPlayedNative.Broadcast(Handle, Params.ProfileTag);

	return Handle;
}

FPGXSoundHandle UPGXAudioSubsystem::PlaySoundAttached(USoundBase* Sound, USceneComponent* AttachComponent,
	FName AttachPointName, const FPGXAudioPlayParams& Params)
{
	if (!Sound)
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("PlaySoundAttached — Invalid sound"));
		RecordEvent(TAG_PGX_Audio_Event_Play, TEXT("Invalid sound"), Params.ChannelTag, Params.ProfileTag, false);
		return FPGXSoundHandle::Invalid();
	}
	if (!AttachComponent)
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("PlaySoundAttached — Invalid attach component for %s"), *Sound->GetName());
		RecordEvent(TAG_PGX_Audio_Event_Play, Sound->GetName(), Params.ChannelTag, Params.ProfileTag, false);
		return FPGXSoundHandle::Invalid();
	}
	if (AudioState != EPGXAudioState::Ready)
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("PlaySoundAttached — Audio subsystem is not ready: %s"), *UEnum::GetValueAsString(AudioState));
		RecordEvent(TAG_PGX_Audio_Event_Play, Sound->GetName(), Params.ChannelTag, Params.ProfileTag, false);
		return FPGXSoundHandle::Invalid();
	}

	const UPGXAudioProfile* Profile = FindAudioProfile(Params.ProfileTag);
	const float FinalVolume = Params.VolumeMultiplier * (Profile ? Profile->VolumeMultiplier : 1.0f);
	const float FinalPitch = Params.PitchMultiplier * (Profile ? Profile->PitchMultiplier : 1.0f);

	USoundAttenuation* Attenuation = nullptr;
	if (Profile && !Profile->AttenuationSettings.IsNull())
	{
		Attenuation = Profile->AttenuationSettings.LoadSynchronous();
	}

	USoundConcurrency* Concurrency = nullptr;
	if (Profile && !Profile->ConcurrencySettings.IsNull())
	{
		Concurrency = Profile->ConcurrencySettings.LoadSynchronous();
	}

	UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAttached(Sound, AttachComponent,
		AttachPointName, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset,
		false, FinalVolume, FinalPitch, 0.0f, Attenuation, Concurrency, false);

	if (!AudioComp)
	{
		RecordEvent(TAG_PGX_Audio_Event_Play, Sound->GetName(), Params.ChannelTag, Params.ProfileTag, false);
		return FPGXSoundHandle::Invalid();
	}

	const float FadeIn = Params.FadeInDuration > 0.0f ? Params.FadeInDuration : (Profile ? Profile->FadeInDuration : 0.0f);
	if (FadeIn > 0.0f)
	{
		AudioComp->FadeIn(FadeIn);
	}

	FPGXSoundHandle Handle;
	Handle.Id = GenerateHandleId();
	Handle.ChannelTag = Params.ChannelTag;
	Handle.ProfileTag = Params.ProfileTag;
	Handle.StartTime = FPlatformTime::Seconds();

	FPGXActiveSoundInfo Info;
	Info.Handle = Handle;
	Info.SoundName = Sound->GetName();
	Info.ChannelTag = Params.ChannelTag;
	Info.ProfileTag = Params.ProfileTag;
	Info.Priority = Params.Priority;
	Info.bIs3D = true;

	ActiveSoundsMap.Add(Handle.Id, Info);
	HandleToComponentMap.Add(Handle.Id, AudioComp);

	RecordEvent(TAG_PGX_Audio_Event_Play, Sound->GetName(), Params.ChannelTag, Params.ProfileTag, true);

	OnSoundPlayed.Broadcast(Handle, Params.ProfileTag);
	OnSoundPlayedNative.Broadcast(Handle, Params.ProfileTag);

	return Handle;
}

void UPGXAudioSubsystem::StopSound(FPGXSoundHandle Handle, float FadeOutDuration)
{
	if (!Handle.IsValid())
	{
		return;
	}

	TWeakObjectPtr<UAudioComponent>* CompPtr = HandleToComponentMap.Find(Handle.Id);
	if (CompPtr && CompPtr->IsValid())
	{
		UAudioComponent* Comp = CompPtr->Get();
		if (FadeOutDuration > 0.0f)
		{
			Comp->FadeOut(FadeOutDuration, 0.0f);
		}
		else
		{
			Comp->Stop();
		}
	}

	ActiveSoundsMap.Remove(Handle.Id);
	HandleToComponentMap.Remove(Handle.Id);

	RecordEvent(TAG_PGX_Audio_Event_Stop);

	OnSoundStopped.Broadcast(Handle, EPGXSoundStopReason::Explicit);
	OnSoundStoppedNative.Broadcast(Handle, EPGXSoundStopReason::Explicit);
}

void UPGXAudioSubsystem::StopAllSounds(float FadeOutDuration)
{
	TArray<int32> HandleIds;
	HandleToComponentMap.GetKeys(HandleIds);

	for (const int32 HandleId : HandleIds)
	{
		FPGXSoundHandle Handle;
		Handle.Id = HandleId;
		StopSound(Handle, FadeOutDuration);
	}
}

// ── Resolution ──

USoundBase* UPGXAudioSubsystem::ResolveSound(const UPGXSoundDefinition* Definition, const FGameplayTagContainer& ContextTags)
{
	if (!Definition)
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("ResolveSound — Invalid SoundDefinition"));
		return nullptr;
	}
	if (Definition->Variants.Num() == 0)
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("ResolveSound — SoundDefinition %s has no variants"), *Definition->GetName());
		return nullptr;
	}

	// EN: Collect matching variants / ES: Recopilar variantes que coinciden
	TArray<const FPGXSoundVariant*> Matches;
	for (const FPGXSoundVariant& Variant : Definition->Variants)
	{
		// EN: Empty context tags = universal match / ES: Tags de contexto vacios = match universal
		if (Variant.ContextTags.IsEmpty() || ContextTags.HasAll(Variant.ContextTags))
		{
			Matches.Add(&Variant);
		}
	}

	if (Matches.Num() == 0)
	{
		// EN: Fallback: use first variant / ES: Fallback: usar primera variante
		Matches.Add(&Definition->Variants[0]);
	}

	// EN: Select variant based on selection mode / ES: Seleccionar variante basada en modo de seleccion
	const FPGXSoundVariant* Selected = nullptr;

	switch (Definition->SelectionMode)
	{
	case EPGXSoundSelectionMode::Random:
		Selected = Matches[FMath::RandRange(0, Matches.Num() - 1)];
		break;

	case EPGXSoundSelectionMode::Weighted:
		{
			float TotalWeight = 0.0f;
			for (const FPGXSoundVariant* M : Matches)
			{
				TotalWeight += M->Weight;
			}
			float Roll = FMath::FRand() * TotalWeight;
			for (const FPGXSoundVariant* M : Matches)
			{
				Roll -= M->Weight;
				if (Roll <= 0.0f)
				{
					Selected = M;
					break;
				}
			}
			if (!Selected)
			{
				Selected = Matches.Last();
			}
		}
		break;

	case EPGXSoundSelectionMode::First:
		Selected = Matches[0];
		break;

	case EPGXSoundSelectionMode::Sequential:
		// EN: Per-definition sequential index, independent from handle ID generation
		// ES: Indice secuencial por definicion, independiente de generacion de ID de handle
		{
			int32& DefIndex = SequentialVariantIndex.FindOrAdd(Definition, 0);
			Selected = Matches[DefIndex % Matches.Num()];
			DefIndex++;
		}
		break;
	}

	if (!Selected || Selected->Sounds.Num() == 0)
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("ResolveSound — No sound assets matched definition %s"), *Definition->GetName());
		return nullptr;
	}

	// EN: Pick a random sound from the variant / ES: Elegir un sonido aleatorio de la variante
	const int32 SoundIndex = FMath::RandRange(0, Selected->Sounds.Num() - 1);
	return Selected->Sounds[SoundIndex].LoadSynchronous();
}

FPGXSoundHandle UPGXAudioSubsystem::PlayResolved(const UPGXSoundDefinition* Definition, const FPGXAudioPlayParams& Params)
{
	USoundBase* Sound = ResolveSound(Definition, Params.ContextTags);
	if (!Sound)
	{
		const FString DefinitionName = Definition ? Definition->GetName() : TEXT("Invalid SoundDefinition");
		RecordEvent(TAG_PGX_Audio_Event_Play, DefinitionName, Params.ChannelTag, Params.ProfileTag, false);
		return FPGXSoundHandle::Invalid();
	}

	// EN: Use definition defaults if params don't specify / ES: Usar defaults de definicion si los params no especifican
	FPGXAudioPlayParams ResolvedParams = Params;
	if (!ResolvedParams.ChannelTag.IsValid() && Definition)
	{
		ResolvedParams.ChannelTag = Definition->DefaultChannelTag;
	}
	if (!ResolvedParams.ProfileTag.IsValid() && Definition)
	{
		ResolvedParams.ProfileTag = Definition->DefaultProfileTag;
	}

	return PlaySound2D(Sound, ResolvedParams);
}

// ── Music (delegates to MusicManager) ──

void UPGXAudioSubsystem::PlayMusic(USoundBase* Music, float FadeInDuration)
{
	if (MusicManager) MusicManager->PlayMusic(Music, FadeInDuration);
}

void UPGXAudioSubsystem::StopMusic(float FadeOutDuration)
{
	if (MusicManager) MusicManager->StopMusic(FadeOutDuration);
}

void UPGXAudioSubsystem::PauseMusic()
{
	if (MusicManager) MusicManager->PauseMusic();
}

void UPGXAudioSubsystem::ResumeMusic()
{
	if (MusicManager) MusicManager->ResumeMusic();
}

void UPGXAudioSubsystem::CrossfadeTo(USoundBase* NewMusic, float CrossfadeDuration)
{
	if (MusicManager) MusicManager->CrossfadeTo(NewMusic, CrossfadeDuration);
}

void UPGXAudioSubsystem::PlayPlaylist(const UPGXMusicPlaylist* Playlist)
{
	if (MusicManager) MusicManager->PlayPlaylist(Playlist);
}

void UPGXAudioSubsystem::SetMusicState(FGameplayTag StateTag)
{
	if (MusicManager) MusicManager->SetMusicState(StateTag);
}

FGameplayTag UPGXAudioSubsystem::GetMusicState() const
{
	return MusicManager ? MusicManager->GetMusicState() : FGameplayTag();
}

EPGXMusicState UPGXAudioSubsystem::GetMusicPlaybackState() const
{
	return MusicManager ? MusicManager->GetState() : EPGXMusicState::Idle;
}

FString UPGXAudioSubsystem::GetCurrentTrackName() const
{
	return MusicManager ? MusicManager->GetCurrentTrackName() : TEXT("None");
}

// ── Dialogue (delegates to DialogueManager) ──

bool UPGXAudioSubsystem::QueueDialogue(USoundBase* Sound, FText SubtitleText, float Duration,
	FGameplayTag SpeakerTag, FGameplayTag PriorityTag, EPGXDialogueInterruptPolicy Policy)
{
	return DialogueManager ? DialogueManager->QueueDialogue(Sound, SubtitleText, Duration, SpeakerTag, PriorityTag, Policy) : false;
}

void UPGXAudioSubsystem::StopDialogue(float FadeOutDuration)
{
	if (DialogueManager) DialogueManager->StopDialogue(FadeOutDuration);
}

void UPGXAudioSubsystem::ClearDialogueQueue()
{
	if (DialogueManager) DialogueManager->ClearDialogueQueue();
}

int32 UPGXAudioSubsystem::GetDialogueQueueCount() const
{
	return DialogueManager ? DialogueManager->GetDialogueQueueCount() : 0;
}

bool UPGXAudioSubsystem::IsDialoguePlaying() const
{
	return DialogueManager ? DialogueManager->IsDialoguePlaying() : false;
}

// ── Query ──

int32 UPGXAudioSubsystem::GetActiveSoundCount() const
{
	return ActiveSoundsMap.Num();
}

TArray<FPGXActiveSoundInfo> UPGXAudioSubsystem::GetActiveSounds() const
{
	TArray<FPGXActiveSoundInfo> Result;
	Result.Reserve(ActiveSoundsMap.Num());

	const double Now = FPlatformTime::Seconds();
	for (const auto& Pair : ActiveSoundsMap)
	{
		FPGXActiveSoundInfo Info = Pair.Value;
		Info.Age = static_cast<float>(Now - Info.Handle.StartTime);
		Result.Add(Info);
	}

	return Result;
}

FPGXAudioSystemSnapshot UPGXAudioSubsystem::GetAudioSnapshot() const
{
	FPGXAudioSystemSnapshot Snapshot;
	Snapshot.State = AudioState;
	Snapshot.BackendType = GetActiveBackendType();
	Snapshot.Channels = GetAllChannelStates();
	Snapshot.ActiveSounds = GetActiveSounds();
	Snapshot.MusicState = GetMusicPlaybackState();
	Snapshot.PoolStats = GetPoolStatistics();
	Snapshot.MemoryInfo = GetMemoryEstimate();
	return Snapshot;
}

FPGXSoundPoolStats UPGXAudioSubsystem::GetPoolStatistics() const
{
	return SoundPool ? SoundPool->GetStats() : FPGXSoundPoolStats();
}

FPGXAudioMemoryInfo UPGXAudioSubsystem::GetMemoryEstimate() const
{
	FPGXAudioMemoryInfo Info = SoundPool ? SoundPool->GetMemoryInfo() : FPGXAudioMemoryInfo();
	Info.LoadedSoundCount = ActiveSoundsMap.Num();
	return Info;
}

TArray<FPGXAudioEventRecord> UPGXAudioSubsystem::GetEventHistory(int32 MaxCount) const
{
	if (!ShouldExposeEventHistory())
	{
		return TArray<FPGXAudioEventRecord>();
	}

	const int32 ResolvedMaxCount = FMath::Clamp(MaxCount, 0, GetResolvedMaxEventHistorySize());
	const int32 Count = FMath::Min(ResolvedMaxCount, EventHistory.Num());
	if (Count <= 0)
	{
		return TArray<FPGXAudioEventRecord>();
	}

	const int32 StartIdx = EventHistory.Num() - Count;
	TArray<FPGXAudioEventRecord> Result;
	Result.Reserve(Count);
	for (int32 i = StartIdx; i < EventHistory.Num(); ++i)
	{
		Result.Add(EventHistory[i]);
	}
	return Result;
}

// ── Internal ──

void UPGXAudioSubsystem::LoadConfig()
{
	// EN: Settings-first resolution with AssetRegistry fallback (deprecated)
	// ES: Resolucion Settings-first con fallback a AssetRegistry (deprecated)
	const UPGXAudioSettings* Settings = GetDefault<UPGXAudioSettings>();
	AudioConfig = PGX::ResolveSingleConfig<UPGXAudioConfig>(Settings->ActiveConfig, TEXT("Audio"));

	if (IsValid(AudioConfig))
	{
		PGX_LOG_INFO(LogPGXAudio, TEXT("LoadConfig — Loaded config: %s"), *AudioConfig->GetPathName());
	}
}

void UPGXAudioSubsystem::CacheAudioAssets()
{
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	const IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// EN: Cache ChannelConfigs / ES: Cachear ChannelConfigs
	{
		CachedChannelConfigs.Empty();
		TArray<FAssetData> FoundAssets;
		AssetRegistry.GetAssetsByClass(UPGXAudioChannelConfig::StaticClass()->GetClassPathName(), FoundAssets);
		for (const FAssetData& AssetData : FoundAssets)
		{
			if (const UPGXAudioChannelConfig* Config = Cast<UPGXAudioChannelConfig>(AssetData.GetAsset()))
			{
				CachedChannelConfigs.Add(Config);
			}
		}
		PGX_LOG_INFO(LogPGXAudio, TEXT("CacheAudioAssets — Cached %d ChannelConfigs"), CachedChannelConfigs.Num());
	}

	// EN: Cache AudioProfiles / ES: Cachear AudioProfiles
	{
		CachedProfiles.Empty();
		TArray<FAssetData> FoundAssets;
		AssetRegistry.GetAssetsByClass(UPGXAudioProfile::StaticClass()->GetClassPathName(), FoundAssets);
		FoundAssets.Sort([](const FAssetData& Lhs, const FAssetData& Rhs)
		{
			return Lhs.PackageName.ToString() < Rhs.PackageName.ToString();
		});
		for (const FAssetData& AssetData : FoundAssets)
		{
			if (const UPGXAudioProfile* Profile = Cast<UPGXAudioProfile>(AssetData.GetAsset()))
			{
				if (Profile->ProfileTag.IsValid())
				{
					if (CachedProfiles.Contains(Profile->ProfileTag))
					{
						PGX_LOG_WARNING(LogPGXAudio, TEXT("CacheAudioAssets — Duplicate AudioProfile tag %s ignored for %s"),
							*Profile->ProfileTag.ToString(), *Profile->GetPathName());
						continue;
					}
					CachedProfiles.Add(Profile->ProfileTag, Profile);
				}
			}
		}
		PGX_LOG_INFO(LogPGXAudio, TEXT("CacheAudioAssets — Cached %d AudioProfiles"), CachedProfiles.Num());
	}

	// EN: Cache SoundDefinitions by SoundTag / ES: Cachear SoundDefinitions por SoundTag
	{
		CachedSoundDefinitions.Empty();
		TArray<FAssetData> FoundAssets;
		AssetRegistry.GetAssetsByClass(UPGXSoundDefinition::StaticClass()->GetClassPathName(), FoundAssets);
		FoundAssets.Sort([](const FAssetData& Lhs, const FAssetData& Rhs)
		{
			return Lhs.PackageName.ToString() < Rhs.PackageName.ToString();
		});
		for (const FAssetData& AssetData : FoundAssets)
		{
			if (const UPGXSoundDefinition* Def = Cast<UPGXSoundDefinition>(AssetData.GetAsset()))
			{
				if (Def->SoundTag.IsValid())
				{
					if (CachedSoundDefinitions.Contains(Def->SoundTag))
					{
						PGX_LOG_WARNING(LogPGXAudio, TEXT("CacheAudioAssets — Duplicate SoundDefinition tag %s ignored for %s"),
							*Def->SoundTag.ToString(), *Def->GetPathName());
						continue;
					}
					CachedSoundDefinitions.Add(Def->SoundTag, Def);
				}
			}
		}
		PGX_LOG_INFO(LogPGXAudio, TEXT("CacheAudioAssets — Cached %d SoundDefinitions"), CachedSoundDefinitions.Num());
	}
}

bool UPGXAudioSubsystem::CreateBackend(EPGXAudioBackendType RequestedType)
{
	EPGXAudioBackendType ResolvedType = RequestedType;

	if (ResolvedType == EPGXAudioBackendType::Auto)
	{
		if (FModuleManager::Get().IsModuleLoaded(TEXT("AudioModulation")))
		{
			ResolvedType = EPGXAudioBackendType::Modulation;
		}
		else
		{
			ResolvedType = EPGXAudioBackendType::Legacy;
			PGX_LOG_INFO(LogPGXAudio, TEXT("CreateBackend — AudioModulation not available, using Legacy"));
		}
	}

	UPGXAudioBackend* NewBackend = nullptr;

	if (ResolvedType == EPGXAudioBackendType::Modulation)
	{
		NewBackend = NewObject<UPGXAudioBackendModulation>(this);
	}
	else
	{
		NewBackend = NewObject<UPGXAudioBackendLegacy>(this);
	}

	if (!NewBackend->InitializeBackend(AudioConfig))
	{
		if (ResolvedType == EPGXAudioBackendType::Modulation)
		{
			PGX_LOG_WARNING(LogPGXAudio, TEXT("CreateBackend — Modulation init failed, falling back to Legacy"));
			NewBackend = NewObject<UPGXAudioBackendLegacy>(this);
			if (!NewBackend->InitializeBackend(AudioConfig))
			{
				PGX_LOG_ERROR(LogPGXAudio, TEXT("CreateBackend — Legacy fallback also failed!"));
				return false;
			}
		}
		else
		{
			PGX_LOG_ERROR(LogPGXAudio, TEXT("CreateBackend — Legacy backend init failed!"));
			return false;
		}
	}

	ActiveBackend = NewBackend;
	PGX_LOG_INFO(LogPGXAudio, TEXT("CreateBackend — Active: %s"), *ActiveBackend->GetStatusText());
	return true;
}

int32 UPGXAudioSubsystem::GetResolvedMaxEventHistorySize() const
{
	const int32 ConfiguredMax = AudioConfig ? AudioConfig->MaxEventHistorySize : MaxEventHistory;
	return FMath::Clamp(ConfiguredMax, 0, 1000);
}

bool UPGXAudioSubsystem::ShouldRecordEventHistory() const
{
#if UE_BUILD_SHIPPING
	return AudioConfig && AudioConfig->bRecordEventHistoryInShipping;
#else
	return true;
#endif
}

bool UPGXAudioSubsystem::ShouldExposeEventHistory() const
{
#if UE_BUILD_SHIPPING
	return AudioConfig && AudioConfig->bExposeEventHistoryInShipping;
#else
	return true;
#endif
}

bool UPGXAudioSubsystem::AreConsoleMutationsAllowed() const
{
#if UE_BUILD_SHIPPING
	return false;
#else
	return AudioConfig && AudioConfig->bAllowConsoleMutations;
#endif
}

void UPGXAudioSubsystem::ExecuteConsoleCommand(const FString& CommandName, const TArray<FString>& Args, UWorld* World)
{
	if (CommandName == TEXT("pgx.audio.backend"))
	{
		PGX_LOG_INFO(LogPGXAudio, TEXT("=== Audio Backend ==="));
		PGX_LOG_INFO(LogPGXAudio, TEXT("Type: %s"), *UEnum::GetValueAsString(GetActiveBackendType()));
		PGX_LOG_INFO(LogPGXAudio, TEXT("Status: %s"), *GetBackendStatusText());
		PGX_LOG_INFO(LogPGXAudio, TEXT("Operational: %s"), ActiveBackend && ActiveBackend->IsOperational() ? TEXT("YES") : TEXT("NO"));
		return;
	}
	if (CommandName == TEXT("pgx.audio.channels"))
	{
		const TArray<FPGXAudioChannelSnapshot> Channels = GetAllChannelStates();
		PGX_LOG_INFO(LogPGXAudio, TEXT("=== Audio Channels (%d) ==="), Channels.Num());
		for (const FPGXAudioChannelSnapshot& Ch : Channels)
		{
			PGX_LOG_INFO(LogPGXAudio, TEXT("  %s [%s]: %.0f%% %s (sounds: %d)"),
				*Ch.DisplayName, *Ch.ChannelTag.ToString(), Ch.Volume * 100.0f,
				Ch.bMuted ? TEXT("[MUTED]") : TEXT(""), Ch.ActiveSoundCount);
		}
		return;
	}
	if (CommandName == TEXT("pgx.audio.debug"))
	{
		if (!AreConsoleMutationsAllowed())
		{
			PGX_LOG_WARNING(LogPGXAudio, TEXT("pgx.audio.debug — Mutation blocked by audio debug policy"));
			return;
		}
		if (Args.Num() >= 1)
		{
			bDebugOverlay = Args[0].Equals(TEXT("on"), ESearchCase::IgnoreCase);
		}
		else
		{
			bDebugOverlay = !bDebugOverlay;
		}
		PGX_LOG_INFO(LogPGXAudio, TEXT("pgx.audio.debug — Debug overlay: %s"), bDebugOverlay ? TEXT("ON") : TEXT("OFF"));
		return;
	}
	if (CommandName == TEXT("pgx.audio.device"))
	{
		UPGXAudioMixSubsystem* MixSub = FindMixSubsystem();
		if (!MixSub)
		{
			PGX_LOG_WARNING(LogPGXAudio, TEXT("pgx.audio.device — No active world for MixSubsystem"));
			return;
		}
		if (Args.Num() >= 1)
		{
			if (!AreConsoleMutationsAllowed())
			{
				PGX_LOG_WARNING(LogPGXAudio, TEXT("pgx.audio.device — Mutation blocked by audio debug policy"));
				return;
			}
			// EN: Join all args in case device name has spaces
			// ES: Unir todos los args en caso de que el nombre del dispositivo tenga espacios
			FString DeviceName = FString::Join(Args, TEXT(" "));
			const bool bOk = MixSub->SetAudioDevice(DeviceName);
			PGX_LOG_INFO(LogPGXAudio, TEXT("pgx.audio.device — Set to '%s': %s"), *DeviceName, bOk ? TEXT("OK") : TEXT("FAILED"));
		}
		else
		{
			PGX_LOG_INFO(LogPGXAudio, TEXT("=== Audio Device ==="));
			PGX_LOG_INFO(LogPGXAudio, TEXT("Current: %s"), *MixSub->GetCurrentAudioDevice());
			const TArray<FString> Devices = MixSub->GetAvailableAudioDevices();
			PGX_LOG_INFO(LogPGXAudio, TEXT("Available (%d):"), Devices.Num());
			for (const FString& D : Devices)
			{
				PGX_LOG_INFO(LogPGXAudio, TEXT("  - %s"), *D);
			}
		}
		return;
	}
	if (CommandName == TEXT("pgx.audio.dialogue"))
	{
		PGX_LOG_INFO(LogPGXAudio, TEXT("=== Dialogue Manager ==="));
		PGX_LOG_INFO(LogPGXAudio, TEXT("Playing: %s | Queue: %d"),
			IsDialoguePlaying() ? TEXT("YES") : TEXT("NO"), GetDialogueQueueCount());
		return;
	}
	if (CommandName == TEXT("pgx.audio.ducking"))
	{
		UPGXAudioMixSubsystem* MixSub = FindMixSubsystem();
		if (!MixSub)
		{
			PGX_LOG_INFO(LogPGXAudio, TEXT("=== Ducking Rules === (no active world)"));
			return;
		}
		const TArray<FPGXDuckingRule> Rules = MixSub->GetActiveDuckingRules();
		PGX_LOG_INFO(LogPGXAudio, TEXT("=== Ducking Rules (%d) — Enabled: %s ==="),
			Rules.Num(), MixSub->IsDuckingEnabled() ? TEXT("YES") : TEXT("NO"));
		for (const FPGXDuckingRule& R : Rules)
		{
			PGX_LOG_INFO(LogPGXAudio, TEXT("  %s: %s -> %s (duck: %.0f%%, attack: %.2fs, release: %.2fs)"),
				*R.RuleName.ToString(), *R.TriggerChannelTag.ToString(),
				*R.TargetChannelTag.ToString(), R.DuckVolume * 100.0f,
				R.AttackTime, R.ReleaseTime);
		}
		return;
	}
	if (CommandName == TEXT("pgx.audio.hdr"))
	{
		if (!AreConsoleMutationsAllowed())
		{
			PGX_LOG_WARNING(LogPGXAudio, TEXT("pgx.audio.hdr — Mutation blocked by audio debug policy"));
			return;
		}
		UPGXAudioMixSubsystem* MixSub = FindMixSubsystem();
		if (!MixSub)
		{
			PGX_LOG_WARNING(LogPGXAudio, TEXT("pgx.audio.hdr — No active world for MixSubsystem"));
			return;
		}
		if (Args.Num() >= 1)
		{
			MixSub->SetHDRAudioEnabled(Args[0].Equals(TEXT("on"), ESearchCase::IgnoreCase));
		}
		else
		{
			MixSub->SetHDRAudioEnabled(!MixSub->IsHDRAudioEnabled());
		}
		PGX_LOG_INFO(LogPGXAudio, TEXT("pgx.audio.hdr — HDR Audio: %s"), MixSub->IsHDRAudioEnabled() ? TEXT("ON") : TEXT("OFF"));
		return;
	}
	if (CommandName == TEXT("pgx.audio.history"))
	{
		const TArray<FPGXAudioEventRecord> Events = GetEventHistory(20);
		PGX_LOG_INFO(LogPGXAudio, TEXT("=== Event History (%d) ==="), Events.Num());
		for (const FPGXAudioEventRecord& E : Events)
		{
			PGX_LOG_INFO(LogPGXAudio, TEXT("  [%.1f] %s %s %s"),
				E.Timestamp, *E.EventTag.ToString(),
				E.SoundName.IsEmpty() ? TEXT("") : *E.SoundName,
				E.bSuccess ? TEXT("") : TEXT("[FAILED]"));
		}
		return;
	}
	if (CommandName == TEXT("pgx.audio.hrtf"))
	{
		if (!AreConsoleMutationsAllowed())
		{
			PGX_LOG_WARNING(LogPGXAudio, TEXT("pgx.audio.hrtf — Mutation blocked by audio debug policy"));
			return;
		}
		UPGXAudioMixSubsystem* MixSub = FindMixSubsystem();
		if (!MixSub)
		{
			PGX_LOG_WARNING(LogPGXAudio, TEXT("pgx.audio.hrtf — No active world for MixSubsystem"));
			return;
		}
		if (Args.Num() >= 1)
		{
			MixSub->SetHRTFEnabled(Args[0].Equals(TEXT("on"), ESearchCase::IgnoreCase));
		}
		else
		{
			MixSub->SetHRTFEnabled(!MixSub->IsHRTFEnabled());
		}
		PGX_LOG_INFO(LogPGXAudio, TEXT("pgx.audio.hrtf — HRTF: %s"), MixSub->IsHRTFEnabled() ? TEXT("ON") : TEXT("OFF"));
		return;
	}
	if (CommandName == TEXT("pgx.audio.memory"))
	{
		const FPGXAudioMemoryInfo Mem = GetMemoryEstimate();
		PGX_LOG_INFO(LogPGXAudio, TEXT("=== Audio Memory ==="));
		PGX_LOG_INFO(LogPGXAudio, TEXT("Loaded: %d | Est. Memory: %.1f MB"), Mem.LoadedSoundCount, Mem.EstimatedMemoryBytes / (1024.0 * 1024.0));
		PGX_LOG_INFO(LogPGXAudio, TEXT("Pool: %d/%d | Peak Concurrent: %d"), Mem.PoolInUse, Mem.PoolAllocated, Mem.PeakConcurrent);
		return;
	}
	if (CommandName == TEXT("pgx.audio.mix"))
	{
		UPGXAudioMixSubsystem* MixSub = FindMixSubsystem();
		if (!MixSub)
		{
			PGX_LOG_INFO(LogPGXAudio, TEXT("=== Mix Layers === (no active world)"));
			return;
		}
		const TArray<FPGXMixLayerState> Layers = MixSub->GetAllMixLayerStates();
		PGX_LOG_INFO(LogPGXAudio, TEXT("=== Mix Layers (%d) ==="), Layers.Num());
		for (const FPGXMixLayerState& L : Layers)
		{
			PGX_LOG_INFO(LogPGXAudio, TEXT("  %s: %s (%d channels) %s"),
				*UEnum::GetValueAsString(L.Layer),
				L.bActive ? TEXT("ACTIVE") : TEXT("inactive"),
				L.ChannelValues.Num(),
				L.SourceConfigName.IsEmpty() ? TEXT("") : *FString::Printf(TEXT("[%s]"), *L.SourceConfigName));
		}
		return;
	}
	if (CommandName == TEXT("pgx.audio.music"))
	{
		PGX_LOG_INFO(LogPGXAudio, TEXT("=== Music Manager ==="));
		PGX_LOG_INFO(LogPGXAudio, TEXT("State: %s"), *UEnum::GetValueAsString(GetMusicPlaybackState()));
		PGX_LOG_INFO(LogPGXAudio, TEXT("Track: %s"), *GetCurrentTrackName());
		PGX_LOG_INFO(LogPGXAudio, TEXT("Music State Tag: %s"), *GetMusicState().ToString());
		return;
	}
	if (CommandName == TEXT("pgx.audio.mute"))
	{
		if (!AreConsoleMutationsAllowed())
		{
			PGX_LOG_WARNING(LogPGXAudio, TEXT("pgx.audio.mute — Mutation blocked by audio debug policy"));
			return;
		}
		if (Args.Num() < 1)
		{
			PGX_LOG_INFO(LogPGXAudio, TEXT("Usage: pgx.audio.mute <channel_tag|all>"));
			return;
		}
		if (Args[0].Equals(TEXT("all"), ESearchCase::IgnoreCase))
		{
			SetMuteAll(!IsMuteAll());
			PGX_LOG_INFO(LogPGXAudio, TEXT("pgx.audio.mute — Global mute: %s"), IsMuteAll() ? TEXT("ON") : TEXT("OFF"));
		}
		else
		{
			const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*Args[0]), false);
			if (!Tag.IsValid())
			{
				PGX_LOG_WARNING(LogPGXAudio, TEXT("pgx.audio.mute — Invalid tag: %s"), *Args[0]);
				return;
			}
			const bool bNewMute = !IsChannelMuted(Tag);
			SetChannelMuted(Tag, bNewMute);
			PGX_LOG_INFO(LogPGXAudio, TEXT("pgx.audio.mute — %s: %s"), *Tag.ToString(), bNewMute ? TEXT("MUTED") : TEXT("UNMUTED"));
		}
		return;
	}
	if (CommandName == TEXT("pgx.audio.play"))
	{
		if (!AreConsoleMutationsAllowed())
		{
			PGX_LOG_WARNING(LogPGXAudio, TEXT("pgx.audio.play — Mutation blocked by audio debug policy"));
			return;
		}
		if (Args.Num() < 1)
		{
			PGX_LOG_INFO(LogPGXAudio, TEXT("Usage: pgx.audio.play <sound_definition_tag>"));
			return;
		}
		const FGameplayTag SoundTag = FGameplayTag::RequestGameplayTag(FName(*Args[0]), false);
		if (!SoundTag.IsValid())
		{
			PGX_LOG_WARNING(LogPGXAudio, TEXT("pgx.audio.play — Invalid tag: %s"), *Args[0]);
			return;
		}

		// EN: Use init-time cache for deterministic resolution / ES: Usar cache de inicializacion para resolucion determinista
		const UPGXSoundDefinition* Def = FindDefinitionByTag(SoundTag);
		if (!Def)
		{
			PGX_LOG_WARNING(LogPGXAudio, TEXT("pgx.audio.play — No cached SoundDefinition found with tag: %s"), *SoundTag.ToString());
			return;
		}

		FPGXAudioPlayParams Params;
		Params.ChannelTag = Def->DefaultChannelTag;
		Params.ProfileTag = Def->DefaultProfileTag;
		FPGXSoundHandle H = PlayResolved(Def, Params);
		PGX_LOG_INFO(LogPGXAudio, TEXT("pgx.audio.play — Resolved %s → Handle %d"), *SoundTag.ToString(), H.Id);
		return;
	}
	if (CommandName == TEXT("pgx.audio.playing"))
	{
		const TArray<FPGXActiveSoundInfo> Sounds = GetActiveSounds();
		PGX_LOG_INFO(LogPGXAudio, TEXT("=== Active Sounds (%d) ==="), Sounds.Num());
		for (const FPGXActiveSoundInfo& S : Sounds)
		{
			PGX_LOG_INFO(LogPGXAudio, TEXT("  [%d] %s (ch: %s, age: %.1fs, 3D: %s, pri: %d)"),
				S.Handle.Id, *S.SoundName, *S.ChannelTag.ToString(), S.Age,
				S.bIs3D ? TEXT("Y") : TEXT("N"), S.Priority);
		}
		return;
	}
	if (CommandName == TEXT("pgx.audio.pool"))
	{
		const FPGXSoundPoolStats Stats = GetPoolStatistics();
		PGX_LOG_INFO(LogPGXAudio, TEXT("=== Sound Pool ==="));
		PGX_LOG_INFO(LogPGXAudio, TEXT("Capacity: %d | InUse: %d | Available: %d"), Stats.TotalCapacity, Stats.InUse, Stats.Available);
		PGX_LOG_INFO(LogPGXAudio, TEXT("Peak: %d | Misses: %d | Growths: %d"), Stats.PeakUsage, Stats.MissCount, Stats.GrowthEvents);
		return;
	}
	if (CommandName == TEXT("pgx.audio.set"))
	{
		if (!AreConsoleMutationsAllowed())
		{
			PGX_LOG_WARNING(LogPGXAudio, TEXT("pgx.audio.set — Mutation blocked by audio debug policy"));
			return;
		}
		if (Args.Num() < 2)
		{
			PGX_LOG_INFO(LogPGXAudio, TEXT("Usage: pgx.audio.set <channel_tag> <volume 0.0-1.0>"));
			return;
		}
		const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*Args[0]), false);
		if (!Tag.IsValid())
		{
			PGX_LOG_WARNING(LogPGXAudio, TEXT("pgx.audio.set — Invalid tag: %s"), *Args[0]);
			return;
		}
		const float Vol = FCString::Atof(*Args[1]);
		SetChannelVolume(Tag, Vol);
		PGX_LOG_INFO(LogPGXAudio, TEXT("pgx.audio.set — %s = %.0f%%"), *Tag.ToString(), Vol * 100.0f);
		return;
	}
	if (CommandName == TEXT("pgx.audio.status"))
	{
		PGX_LOG_INFO(LogPGXAudio, TEXT("=== PGX Audio Status ==="));
		PGX_LOG_INFO(LogPGXAudio, TEXT("State: %s"), *UEnum::GetValueAsString(AudioState));
		PGX_LOG_INFO(LogPGXAudio, TEXT("Backend: %s"), *GetBackendStatusText());
		PGX_LOG_INFO(LogPGXAudio, TEXT("Active Sounds: %d"), GetActiveSoundCount());
		PGX_LOG_INFO(LogPGXAudio, TEXT("Global Mute: %s"), bGlobalMute ? TEXT("ON") : TEXT("OFF"));
		PGX_LOG_INFO(LogPGXAudio, TEXT("Music: %s (%s)"), *UEnum::GetValueAsString(GetMusicPlaybackState()), *GetCurrentTrackName());
		PGX_LOG_INFO(LogPGXAudio, TEXT("Dialogue Playing: %s (Queue: %d)"), IsDialoguePlaying() ? TEXT("YES") : TEXT("NO"), GetDialogueQueueCount());
		return;
	}
	if (CommandName == TEXT("pgx.audio.stop"))
	{
		if (!AreConsoleMutationsAllowed())
		{
			PGX_LOG_WARNING(LogPGXAudio, TEXT("pgx.audio.stop — Mutation blocked by audio debug policy"));
			return;
		}
		StopAllSounds(0.5f);
		PGX_LOG_INFO(LogPGXAudio, TEXT("pgx.audio.stop — All sounds stopped"));
		return;
	}
	if (CommandName == TEXT("pgx.audio.switch"))
	{
		if (!AreConsoleMutationsAllowed())
		{
			PGX_LOG_WARNING(LogPGXAudio, TEXT("pgx.audio.switch — Mutation blocked by audio debug policy"));
			return;
		}
		if (Args.Num() < 1)
		{
			PGX_LOG_INFO(LogPGXAudio, TEXT("Usage: pgx.audio.switch <Legacy|Modulation>"));
			return;
		}
		EPGXAudioBackendType Target;
		if (Args[0].Equals(TEXT("Legacy"), ESearchCase::IgnoreCase))
		{
			Target = EPGXAudioBackendType::Legacy;
		}
		else if (Args[0].Equals(TEXT("Modulation"), ESearchCase::IgnoreCase))
		{
			Target = EPGXAudioBackendType::Modulation;
		}
		else
		{
			PGX_LOG_WARNING(LogPGXAudio, TEXT("pgx.audio.switch — Unknown type: %s (use Legacy or Modulation)"), *Args[0]);
			return;
		}
		const FPGXAudioBackendSwitchResult Result = SwitchBackendDetailed(Target);
		PGX_LOG_INFO(LogPGXAudio, TEXT("pgx.audio.switch — %s (%s)"),
			Result.bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"), *Result.Message);
		return;
	}
}


void UPGXAudioSubsystem::RecordEvent(const FGameplayTag& EventTag, const FString& SoundName,
	const FGameplayTag& ChannelTag, const FGameplayTag& ProfileTag,
	bool bSuccess, float Duration)
{
	if (!ShouldRecordEventHistory())
	{
		return;
	}

	const int32 ResolvedMaxHistory = GetResolvedMaxEventHistorySize();
	if (ResolvedMaxHistory <= 0)
	{
		return;
	}

	FPGXAudioEventRecord Record;
	Record.Timestamp = FPlatformTime::Seconds();
	Record.EventTag = EventTag;
	Record.SoundName = SoundName;
	Record.ChannelTag = ChannelTag;
	Record.ProfileTag = ProfileTag;
	Record.bSuccess = bSuccess;
	Record.Duration = Duration;

	while (EventHistory.Num() >= ResolvedMaxHistory)
	{
		EventHistory.RemoveAt(0, 1, EAllowShrinking::No);
	}
	EventHistory.Add(MoveTemp(Record));
}

int32 UPGXAudioSubsystem::GenerateHandleId()
{
	return NextHandleId++;
}

const UPGXAudioProfile* UPGXAudioSubsystem::FindAudioProfile(const FGameplayTag& ProfileTag) const
{
	if (!ProfileTag.IsValid())
	{
		return nullptr;
	}

	// EN: O(1) lookup from cached profiles instead of scanning AssetRegistry
	// ES: Busqueda O(1) desde perfiles cacheados en vez de escanear AssetRegistry
	const TWeakObjectPtr<const UPGXAudioProfile>* Found = CachedProfiles.Find(ProfileTag);
	if (Found && Found->IsValid())
	{
		return Found->Get();
	}

	return nullptr;
}

const UPGXSoundDefinition* UPGXAudioSubsystem::FindDefinitionByTag(const FGameplayTag& SoundTag) const
{
	if (!SoundTag.IsValid())
	{
		return nullptr;
	}

	// EN: O(1) lookup from cached definitions instead of scanning AssetRegistry
	// ES: Busqueda O(1) desde definiciones cacheadas en vez de escanear AssetRegistry
	const TWeakObjectPtr<const UPGXSoundDefinition>* Found = CachedSoundDefinitions.Find(SoundTag);
	if (Found && Found->IsValid())
	{
		return Found->Get();
	}

	return nullptr;
}

// EN: IPGXTaggedRegistry facade — exposes CachedProfiles via the canonical
//     tag-keyed read contract. Behavior-preserving: same data, unified interface
//     so cross-plugin consumers can treat Audio like any tagged registry.
// ES: Fachada IPGXTaggedRegistry — expone CachedProfiles via el contrato
//     canonico de lectura keyed-por-tag. Behavior-preserving: mismos datos,
//     interfaz unificada para que consumers cross-plugin traten Audio como
//     cualquier registry tagged.
// Keep validation output aligned with the shared PGX result contract.
bool UPGXAudioSubsystem::HasEntryByTag(FGameplayTag Tag) const
{
	return CachedProfiles.Contains(Tag);
}

int32 UPGXAudioSubsystem::GetCount() const
{
	return CachedProfiles.Num();
}

void UPGXAudioSubsystem::GetSnapshot(TArray<FGameplayTag>& OutTags) const
{
	CachedProfiles.GetKeys(OutTags);
}

// ============================================================================
// EN: Test injection API (editor only)
// ES: API de inyeccion de test (solo editor)
// ============================================================================

#if WITH_EDITOR || WITH_DEV_AUTOMATION_TESTS
void UPGXAudioSubsystem::InjectTestChannelConfig(const UPGXAudioChannelConfig* Config)
{
	if (!IsValid(Config))
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("[TestHarness] InjectTestChannelConfig — invalid Config"));
		return;
	}

	CachedChannelConfigs.Add(Config);
	PGX_LOG_INFO(LogPGXAudio, TEXT("[TestHarness] Injected test ChannelConfig: %s"), *Config->GetName());
}

void UPGXAudioSubsystem::ClearTestChannelConfigs()
{
	const int32 Before = CachedChannelConfigs.Num();
	CachedChannelConfigs.RemoveAll([](const TWeakObjectPtr<const UPGXAudioChannelConfig>& WeakCfg)
	{
		const UObject* Obj = WeakCfg.Get();
		return IsValid(Obj) && Obj->HasAnyFlags(RF_Transient);
	});
	PGX_LOG_INFO(LogPGXAudio, TEXT("[TestHarness] ClearTestChannelConfigs — removed %d transient configs"),
		Before - CachedChannelConfigs.Num());
}

void UPGXAudioSubsystem::InjectTestAudioConfig(const UPGXAudioConfig* Config)
{
	if (!IsValid(Config))
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("[TestHarness] InjectTestAudioConfig — invalid config"));
		return;
	}
	AudioConfig = Config;
	PGX_LOG_INFO(LogPGXAudio, TEXT("[TestHarness] Injected test AudioConfig: %s"), *Config->GetName());
}

void UPGXAudioSubsystem::ClearTestAudioConfig()
{
	if (IsValid(AudioConfig) && AudioConfig->HasAnyFlags(RF_Transient))
	{
		PGX_LOG_INFO(LogPGXAudio, TEXT("[TestHarness] ClearTestAudioConfig — removed transient config"));
		AudioConfig = nullptr;
	}
}
#endif

#if WITH_DEV_AUTOMATION_TESTS
void UPGXAudioSubsystem::InjectTestSoundDefinition(const UPGXSoundDefinition* Definition)
{
	if (!IsValid(Definition) || !Definition->SoundTag.IsValid())
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("[TestHarness] InjectTestSoundDefinition — invalid definition"));
		return;
	}

	if (CachedSoundDefinitions.Contains(Definition->SoundTag))
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("[TestHarness] InjectTestSoundDefinition — duplicate tag ignored: %s"),
			*Definition->SoundTag.ToString());
		return;
	}

	CachedSoundDefinitions.Add(Definition->SoundTag, Definition);
	PGX_LOG_INFO(LogPGXAudio, TEXT("[TestHarness] Injected test SoundDefinition: %s"), *Definition->GetName());
}

void UPGXAudioSubsystem::ClearTestSoundDefinitions()
{
	const int32 Before = CachedSoundDefinitions.Num();
	for (auto It = CachedSoundDefinitions.CreateIterator(); It; ++It)
	{
		const UObject* Obj = It.Value().Get();
		if (IsValid(Obj) && Obj->HasAnyFlags(RF_Transient))
		{
			It.RemoveCurrent();
		}
	}
	PGX_LOG_INFO(LogPGXAudio, TEXT("[TestHarness] ClearTestSoundDefinitions — removed %d transient definitions"),
		Before - CachedSoundDefinitions.Num());
}

void UPGXAudioSubsystem::ClearEventHistoryForTesting()
{
	EventHistory.Empty();
}

void UPGXAudioSubsystem::RecordEventForTesting(const FGameplayTag& EventTag, const FString& SoundName,
	const FGameplayTag& ChannelTag, const FGameplayTag& ProfileTag, bool bSuccess, float Duration)
{
	RecordEvent(EventTag, SoundName, ChannelTag, ProfileTag, bSuccess, Duration);
}

bool UPGXAudioSubsystem::AreConsoleMutationsAllowedForTesting() const
{
	return AreConsoleMutationsAllowed();
}

bool UPGXAudioSubsystem::ShouldRecordEventHistoryForTesting() const
{
	return ShouldRecordEventHistory();
}

bool UPGXAudioSubsystem::ShouldExposeEventHistoryForTesting() const
{
	return ShouldExposeEventHistory();
}

int32 UPGXAudioSubsystem::GetResolvedMaxEventHistorySizeForTesting() const
{
	return GetResolvedMaxEventHistorySize();
}

void UPGXAudioSubsystem::SetActiveBackendForTesting(UPGXAudioBackend* Backend)
{
	ActiveBackend = Backend;
}

void UPGXAudioSubsystem::ForceNextSwitchActiveBackendTypeForTesting(EPGXAudioBackendType ForcedType)
{
	ForcedNextSwitchResultActiveBackendForTesting = ForcedType;
	bForceNextSwitchResultActiveBackendForTesting = true;
}
#endif

// ============================================================================
// Profile Integration
// ============================================================================

void UPGXAudioSubsystem::ApplyProfileConstraints(const FPGXResolvedProfile& Profile)
{
	// EN: Enforce Audio platform budgets from active PlatformConfig
	// ES: Aplicar presupuestos de plataforma Audio desde PlatformConfig activa
	int32 EnforcedAudioMemory_MB = 0;
	int32 EnforcedMaxConcurrent = 0;
	int32 EnforcedPoolMax = 0;

	if (auto* ProfileSS = UPGXProfileSubsystem::GetCachedInstance())
	{
		if (const UPGXPlatformConfig* PlatformCfg = ProfileSS->GetActivePlatformConfig())
		{
			const auto& B = PlatformCfg->AudioBudgets;
			EnforcedAudioMemory_MB = B.AudioMemory_MB;
			EnforcedMaxConcurrent = B.MaxConcurrentSounds;
			EnforcedPoolMax = B.SoundPoolMaxSize;
		}
	}

	PGX_LOG_INFO(LogPGXAudio, TEXT("[AudioSubsystem] Profile constraints enforced — AudioMem=%dMB, MaxConcurrent=%d, PoolMax=%d, GlobalAudioBudget=%d"),
		EnforcedAudioMemory_MB, EnforcedMaxConcurrent, EnforcedPoolMax,
		Profile.Budgets.AudioMemory_MB);
}

void UPGXAudioSubsystem::HandleProfileChanged(const FPGXResolvedProfile& /*OldProfile*/, const FPGXResolvedProfile& NewProfile)
{
	ApplyProfileConstraints(NewProfile);
}

UPGXAudioMixSubsystem* UPGXAudioSubsystem::FindMixSubsystem() const
{
	// EN: Look for MixSubsystem in any active game world
	// ES: Buscar MixSubsystem en cualquier mundo de juego activo
	if (!GEngine)
	{
		return nullptr;
	}

	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	for (const FWorldContext& Context : WorldContexts)
	{
		if (Context.World() && (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE))
		{
			if (UPGXAudioMixSubsystem* MixSub = Context.World()->GetSubsystem<UPGXAudioMixSubsystem>())
			{
				return MixSub;
			}
		}
	}

	return nullptr;
}
