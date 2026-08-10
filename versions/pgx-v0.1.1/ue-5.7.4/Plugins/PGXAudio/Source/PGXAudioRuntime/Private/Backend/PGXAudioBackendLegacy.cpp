// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Backend/PGXAudioBackendLegacy.h"
#include "PGXAudioLog.h"
#include "Logging/PGXLogMacros.h"
#include "PGXAudioConfig.h"
#include "Data/PGXAudioChannelConfig.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Sound/SoundClass.h"
#include "AudioDevice.h"
#include "Engine/World.h"

// EN: Legacy backend implementation using SoundMix/SoundClass for volume control.
// ES: Implementacion de backend Legacy usando SoundMix/SoundClass para control de volumen.

bool UPGXAudioBackendLegacy::InitializeBackend(const UPGXAudioConfig* /*Config*/)
{
	PGX_LOG_INFO(LogPGXAudio, TEXT("BackendLegacy::Initialize — Setting up SoundMix/SoundClass backend"));

	// EN: Discover all UPGXAudioChannelConfig DAs from AssetRegistry
	// ES: Descubrir todos los DAs UPGXAudioChannelConfig desde AssetRegistry
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	const IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> FoundChannelConfigs;
	AssetRegistry.GetAssetsByClass(UPGXAudioChannelConfig::StaticClass()->GetClassPathName(), FoundChannelConfigs);

	for (const FAssetData& AssetData : FoundChannelConfigs)
	{
		UPGXAudioChannelConfig* ChannelConfig = Cast<UPGXAudioChannelConfig>(AssetData.GetAsset());
		if (!ChannelConfig || !ChannelConfig->ChannelTag.IsValid())
		{
			continue;
		}

		FChannelState State;
		State.Volume = ChannelConfig->DefaultVolume;
		State.bMuted = false;
		State.Config = ChannelConfig;
		ChannelStates.Add(ChannelConfig->ChannelTag, State);

		PGX_LOG_INFO(LogPGXAudio, TEXT("  Channel registered: %s (vol=%.2f)"),
			*ChannelConfig->ChannelTag.ToString(), ChannelConfig->DefaultVolume);
	}

	PGX_LOG_INFO(LogPGXAudio, TEXT("BackendLegacy::Initialize — %d channels registered"), ChannelStates.Num());

	bInitialized = true;
	return true;
}

void UPGXAudioBackendLegacy::ShutdownBackend()
{
	PGX_LOG_INFO(LogPGXAudio, TEXT("BackendLegacy::Shutdown — Cleaning up Legacy backend"));
	ChannelStates.Empty();
	bInitialized = false;
}

void UPGXAudioBackendLegacy::SetChannelVolume(const FGameplayTag& ChannelTag, float Volume)
{
	FChannelState* State = ChannelStates.Find(ChannelTag);
	if (!State)
	{
		// EN: Auto-register channel with no config DA / ES: Auto-registrar canal sin config DA
		FChannelState NewState;
		NewState.Volume = Volume;
		ChannelStates.Add(ChannelTag, NewState);
		State = ChannelStates.Find(ChannelTag);
	}

	State->Volume = FMath::Clamp(Volume, 0.0f, 1.0f);

	if (!State->bMuted)
	{
		ApplyVolumeToSoundClass(ChannelTag, State->Volume);
	}
}

float UPGXAudioBackendLegacy::GetChannelVolume(const FGameplayTag& ChannelTag) const
{
	const FChannelState* State = ChannelStates.Find(ChannelTag);
	return State ? State->Volume : 1.0f;
}

void UPGXAudioBackendLegacy::SetChannelMuted(const FGameplayTag& ChannelTag, bool bMuted)
{
	FChannelState* State = ChannelStates.Find(ChannelTag);
	if (!State)
	{
		FChannelState NewState;
		NewState.bMuted = bMuted;
		ChannelStates.Add(ChannelTag, NewState);
		State = ChannelStates.Find(ChannelTag);
	}

	State->bMuted = bMuted;

	// EN: Apply 0 volume when muted, restore when unmuted / ES: Aplicar volumen 0 al mutear, restaurar al desmutear
	const float EffectiveVolume = bMuted ? 0.0f : State->Volume;
	ApplyVolumeToSoundClass(ChannelTag, EffectiveVolume);
}

bool UPGXAudioBackendLegacy::IsChannelMuted(const FGameplayTag& ChannelTag) const
{
	const FChannelState* State = ChannelStates.Find(ChannelTag);
	return State ? State->bMuted : false;
}

void UPGXAudioBackendLegacy::ApplyMixLayer(EPGXMixLayer Layer, const TMap<FGameplayTag, float>& ChannelValues)
{
	// EN: Legacy backend applies mix layers by adjusting SoundClass volumes
	// ES: Backend Legacy aplica capas de mezcla ajustando volumenes de SoundClass
	PGX_LOG_VERBOSE(LogPGXAudio, TEXT("BackendLegacy::ApplyMixLayer — Layer %d with %d channel values"),
		static_cast<int32>(Layer), ChannelValues.Num());

	for (const auto& Pair : ChannelValues)
	{
		ApplyVolumeToSoundClass(Pair.Key, Pair.Value);
	}
}

void UPGXAudioBackendLegacy::RemoveMixLayer(EPGXMixLayer Layer)
{
	// EN: Restore channel volumes from stored state when layer is removed
	// ES: Restaurar volumenes de canal desde estado almacenado cuando se quita la capa
	PGX_LOG_VERBOSE(LogPGXAudio, TEXT("BackendLegacy::RemoveMixLayer — Layer %d"), static_cast<int32>(Layer));

	for (const auto& Pair : ChannelStates)
	{
		const float EffectiveVolume = Pair.Value.bMuted ? 0.0f : Pair.Value.Volume;
		ApplyVolumeToSoundClass(Pair.Key, EffectiveVolume);
	}
}

FString UPGXAudioBackendLegacy::GetStatusText() const
{
	return FString::Printf(TEXT("Legacy (SoundMix/SoundClass) — %d channels"), ChannelStates.Num());
}

void UPGXAudioBackendLegacy::ApplyVolumeToSoundClass(const FGameplayTag& ChannelTag, float EffectiveVolume)
{
	const FChannelState* State = ChannelStates.Find(ChannelTag);
	if (!State || !State->Config.IsValid())
	{
		return;
	}

	USoundClass* SoundClass = State->Config->SoundClassRef.LoadSynchronous();
	if (!SoundClass)
	{
		PGX_LOG_VERBOSE(LogPGXAudio, TEXT("  ApplyVolumeToSoundClass — No SoundClass for channel %s"),
			*ChannelTag.ToString());
		return;
	}

	// EN: Modify SoundClass volume property directly. This affects all sounds using this SoundClass.
	// ES: Modificar la propiedad de volumen del SoundClass directamente. Afecta todos los sonidos que usan este SoundClass.
	SoundClass->Properties.Volume = EffectiveVolume;

	PGX_LOG_VERBOSE(LogPGXAudio, TEXT("  SoundClass '%s' volume → %.2f"),
		*SoundClass->GetName(), EffectiveVolume);
}
