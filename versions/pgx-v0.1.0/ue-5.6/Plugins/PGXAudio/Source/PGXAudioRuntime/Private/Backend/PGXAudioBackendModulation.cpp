// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Backend/PGXAudioBackendModulation.h"
#include "PGXAudioLog.h"
#include "Logging/PGXLogMacros.h"
#include "PGXAudioConfig.h"
#include "Data/PGXAudioChannelConfig.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "SoundControlBus.h"
#include "SoundControlBusMix.h"
#include "AudioModulationStatics.h"

// EN: Modulation backend implementation using ControlBus for volume control.
// ES: Implementacion de backend Modulation usando ControlBus para control de volumen.

bool UPGXAudioBackendModulation::InitializeBackend(const UPGXAudioConfig* /*Config*/)
{
	PGX_LOG_INFO(LogPGXAudio, TEXT("BackendModulation::Initialize — Setting up ControlBus/ControlBusMix backend"));

	// EN: Verify AudioModulation plugin is available / ES: Verificar que el plugin AudioModulation esta disponible
	if (!FModuleManager::Get().IsModuleLoaded(TEXT("AudioModulation")))
	{
		PGX_LOG_ERROR(LogPGXAudio, TEXT("BackendModulation::Initialize — AudioModulation plugin not loaded! Falling back to Legacy."));
		return false;
	}

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

		// EN: Load and cache the ControlBus reference / ES: Cargar y cachear la referencia al ControlBus
		USoundControlBus* Bus = ChannelConfig->ControlBusRef.LoadSynchronous();
		if (Bus)
		{
			State.ControlBus = Bus;
			PGX_LOG_INFO(LogPGXAudio, TEXT("  Channel registered: %s (bus=%s, vol=%.2f)"),
				*ChannelConfig->ChannelTag.ToString(), *Bus->GetName(), ChannelConfig->DefaultVolume);
		}
		else
		{
			PGX_LOG_WARNING(LogPGXAudio, TEXT("  Channel %s has no ControlBus reference — volume control unavailable"),
				*ChannelConfig->ChannelTag.ToString());
		}

		ChannelStates.Add(ChannelConfig->ChannelTag, State);
	}

	PGX_LOG_INFO(LogPGXAudio, TEXT("BackendModulation::Initialize — %d channels registered"), ChannelStates.Num());

	bInitialized = true;
	return true;
}

void UPGXAudioBackendModulation::ShutdownBackend()
{
	PGX_LOG_INFO(LogPGXAudio, TEXT("BackendModulation::Shutdown — Cleaning up Modulation backend"));
	ChannelStates.Empty();
	bInitialized = false;
}

void UPGXAudioBackendModulation::SetChannelVolume(const FGameplayTag& ChannelTag, float Volume)
{
	FChannelState* State = ChannelStates.Find(ChannelTag);
	if (!State)
	{
		FChannelState NewState;
		NewState.Volume = Volume;
		ChannelStates.Add(ChannelTag, NewState);
		State = ChannelStates.Find(ChannelTag);
	}

	State->Volume = FMath::Clamp(Volume, 0.0f, 1.0f);

	if (!State->bMuted)
	{
		ApplyVolumeToControlBus(ChannelTag, State->Volume);
	}
}

float UPGXAudioBackendModulation::GetChannelVolume(const FGameplayTag& ChannelTag) const
{
	const FChannelState* State = ChannelStates.Find(ChannelTag);
	return State ? State->Volume : 1.0f;
}

void UPGXAudioBackendModulation::SetChannelMuted(const FGameplayTag& ChannelTag, bool bMuted)
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

	const float EffectiveVolume = bMuted ? 0.0f : State->Volume;
	ApplyVolumeToControlBus(ChannelTag, EffectiveVolume);
}

bool UPGXAudioBackendModulation::IsChannelMuted(const FGameplayTag& ChannelTag) const
{
	const FChannelState* State = ChannelStates.Find(ChannelTag);
	return State ? State->bMuted : false;
}

void UPGXAudioBackendModulation::ApplyMixLayer(EPGXMixLayer Layer, const TMap<FGameplayTag, float>& ChannelValues)
{
	PGX_LOG_VERBOSE(LogPGXAudio, TEXT("BackendModulation::ApplyMixLayer — Layer %d with %d channel values"),
		static_cast<int32>(Layer), ChannelValues.Num());

	for (const auto& Pair : ChannelValues)
	{
		ApplyVolumeToControlBus(Pair.Key, Pair.Value);
	}
}

void UPGXAudioBackendModulation::RemoveMixLayer(EPGXMixLayer Layer)
{
	PGX_LOG_VERBOSE(LogPGXAudio, TEXT("BackendModulation::RemoveMixLayer — Layer %d"), static_cast<int32>(Layer));

	for (const auto& Pair : ChannelStates)
	{
		const float EffectiveVolume = Pair.Value.bMuted ? 0.0f : Pair.Value.Volume;
		ApplyVolumeToControlBus(Pair.Key, EffectiveVolume);
	}
}

FString UPGXAudioBackendModulation::GetStatusText() const
{
	int32 BusCount = 0;
	for (const auto& Pair : ChannelStates)
	{
		if (Pair.Value.ControlBus.IsValid())
		{
			++BusCount;
		}
	}
	return FString::Printf(TEXT("Modulation (ControlBus) — %d channels, %d buses"), ChannelStates.Num(), BusCount);
}

void UPGXAudioBackendModulation::ApplyVolumeToControlBus(const FGameplayTag& ChannelTag, float EffectiveVolume)
{
	const FChannelState* State = ChannelStates.Find(ChannelTag);
	if (!State || !State->ControlBus.IsValid())
	{
		return;
	}

	USoundControlBus* Bus = State->ControlBus.Get();
	if (!Bus)
	{
		return;
	}

	// EN: Use UAudioModulationStatics::SetGlobalBusMixValue() to set bus value.
	//     This creates/updates a persistent global mix for the bus.
	//     Requires a WorldContextObject — use the bus itself as context.
	//     FadeTime = 0.0f for immediate change.
	// ES: Usar UAudioModulationStatics::SetGlobalBusMixValue() para establecer valor del bus.
	//     Esto crea/actualiza un mix global persistente para el bus.
	//     Requiere un WorldContextObject — usar el propio bus como contexto.
	//     FadeTime = 0.0f para cambio inmediato.
	UAudioModulationStatics::SetGlobalBusMixValue(Bus, Bus, EffectiveVolume, 0.0f);

	PGX_LOG_VERBOSE(LogPGXAudio, TEXT("  ControlBus '%s' value → %.2f"),
		*Bus->GetName(), EffectiveVolume);
}
