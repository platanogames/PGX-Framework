// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Mix/PGXAudioMixSubsystem.h"
#include "PGXAudioLog.h"
#include "Logging/PGXLogMacros.h"
#include "PGXAudioSubsystem.h"
#include "PGXAudioConfig.h"
#include "Backend/PGXAudioBackend.h"
#include "Data/PGXAudioDuckingConfig.h"
#include "Data/PGXLevelAudioConfig.h"
#include "Tags/PGXAudioTags.h"
// EN: AudioMixerBlueprintLibrary used for device swap (async API)
// ES: AudioMixerBlueprintLibrary usado para swap de dispositivo (API async)
#include "AudioMixerBlueprintLibrary.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

// EN: Per-level mix subsystem implementation (Lyra-validated WorldSubsystem pattern)
// ES: Implementacion del subsistema de mezcla por nivel (patron WorldSubsystem validado por Lyra)

void UPGXAudioMixSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	PGX_LOG_INFO(LogPGXAudio, TEXT("UPGXAudioMixSubsystem::Initialize — Setting up per-level mix management"));

	// EN: Initialize mix layer array / ES: Inicializar array de capas de mezcla
	MixLayers.SetNum(PGX_MIX_LAYER_COUNT);
	for (int32 i = 0; i < PGX_MIX_LAYER_COUNT; ++i)
	{
		MixLayers[i].Layer = static_cast<EPGXMixLayer>(i);
		MixLayers[i].bActive = (i == 0); // EN: DefaultBase always active / ES: DefaultBase siempre activa
	}

	// EN: Load initial HDR/HRTF settings from config / ES: Cargar settings iniciales de HDR/HRTF desde config
	UPGXAudioSubsystem* AudioSub = GetAudioSubsystem();
	if (AudioSub)
	{
		const UPGXAudioConfig* Config = AudioSub->GetAudioConfig();
		if (Config)
		{
			bHDRAudioEnabled = Config->bDefaultHDRAudioEnabled;
			bHRTFEnabled = Config->bDefaultHRTFEnabled;
		}
	}

	// EN: Apply HRTF CVar if needed / ES: Aplicar CVar de HRTF si es necesario
	if (bHRTFEnabled)
	{
		SetHRTFEnabled(true);
	}

	// EN: Load global ducking config / ES: Cargar config de ducking global
	LoadDuckingConfig();

	// EN: Notify main subsystem that mix is ready / ES: Notificar al subsistema principal que el mix esta listo
	if (AudioSub)
	{
		AudioSub->OnMixSubsystemReady.Broadcast();
		AudioSub->OnMixSubsystemReadyNative.Broadcast();
	}

	PGX_LOG_INFO(LogPGXAudio, TEXT("UPGXAudioMixSubsystem::Initialize — Mix subsystem ready (HDR=%s, HRTF=%s)"),
		bHDRAudioEnabled ? TEXT("ON") : TEXT("OFF"),
		bHRTFEnabled ? TEXT("ON") : TEXT("OFF"));
}

void UPGXAudioMixSubsystem::Deinitialize()
{
	PGX_LOG_INFO(LogPGXAudio, TEXT("UPGXAudioMixSubsystem::Deinitialize — Cleaning up mix subsystem"));

	// EN: Deactivate all non-default layers / ES: Desactivar todas las capas no-default
	for (int32 i = 1; i < MixLayers.Num(); ++i)
	{
		if (MixLayers[i].bActive)
		{
			DeactivateMixLayer(static_cast<EPGXMixLayer>(i));
		}
	}

	MixLayers.Empty();
	ActiveLevelConfig = nullptr;
	ActiveDuckingConfig = nullptr;

	Super::Deinitialize();
}

// ── Mix Layer Management ──

void UPGXAudioMixSubsystem::ActivateMixLayer(EPGXMixLayer Layer, const TMap<FGameplayTag, float>& ChannelValues, const FString& SourceName)
{
	const int32 LayerIdx = static_cast<int32>(Layer);
	if (!MixLayers.IsValidIndex(LayerIdx))
	{
		return;
	}

	FPGXMixLayerState& State = MixLayers[LayerIdx];
	State.bActive = true;
	State.ChannelValues = ChannelValues;
	State.SourceConfigName = SourceName;

	PGX_LOG_INFO(LogPGXAudio, TEXT("ActivateMixLayer — Layer %d (%s) with %d channel values"),
		LayerIdx, *SourceName, ChannelValues.Num());

	// EN: Apply through backend / ES: Aplicar a traves del backend
	UPGXAudioBackend* Backend = GetActiveBackend();
	if (Backend)
	{
		Backend->ApplyMixLayer(Layer, ChannelValues);
	}

	// EN: Broadcast delegate / ES: Difundir delegado
	UPGXAudioSubsystem* AudioSub = GetAudioSubsystem();
	if (AudioSub)
	{
		AudioSub->OnMixLayerChanged.Broadcast(Layer, true);
		AudioSub->OnMixLayerChangedNative.Broadcast(Layer, true);
	}
}

void UPGXAudioMixSubsystem::DeactivateMixLayer(EPGXMixLayer Layer)
{
	const int32 LayerIdx = static_cast<int32>(Layer);
	if (!MixLayers.IsValidIndex(LayerIdx))
	{
		return;
	}

	// EN: Don't deactivate DefaultBase / ES: No desactivar DefaultBase
	if (Layer == EPGXMixLayer::DefaultBase)
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("DeactivateMixLayer — Cannot deactivate DefaultBase layer"));
		return;
	}

	FPGXMixLayerState& State = MixLayers[LayerIdx];
	if (!State.bActive)
	{
		return;
	}

	PGX_LOG_INFO(LogPGXAudio, TEXT("DeactivateMixLayer — Layer %d"), LayerIdx);

	// EN: Remove layer from backend / ES: Quitar capa del backend
	UPGXAudioBackend* Backend = GetActiveBackend();
	if (Backend)
	{
		Backend->RemoveMixLayer(Layer);
	}

	State.bActive = false;
	State.ChannelValues.Empty();
	State.SourceConfigName.Empty();

	// EN: Recalculate effective volumes / ES: Recalcular volumenes efectivos
	RecalculateEffectiveVolumes();

	UPGXAudioSubsystem* AudioSub = GetAudioSubsystem();
	if (AudioSub)
	{
		AudioSub->OnMixLayerChanged.Broadcast(Layer, false);
		AudioSub->OnMixLayerChangedNative.Broadcast(Layer, false);
	}
}

FPGXMixLayerState UPGXAudioMixSubsystem::GetMixLayerState(EPGXMixLayer Layer) const
{
	const int32 LayerIdx = static_cast<int32>(Layer);
	if (MixLayers.IsValidIndex(LayerIdx))
	{
		return MixLayers[LayerIdx];
	}
	return FPGXMixLayerState();
}

TArray<FPGXMixLayerState> UPGXAudioMixSubsystem::GetAllMixLayerStates() const
{
	return MixLayers;
}

// ── Level Audio Config ──

void UPGXAudioMixSubsystem::SetLevelAudioConfig(UPGXLevelAudioConfig* InConfig)
{
	// EN: Remove previous level layer / ES: Quitar capa de nivel previa
	if (ActiveLevelConfig)
	{
		DeactivateMixLayer(EPGXMixLayer::Level);
	}

	ActiveLevelConfig = InConfig;

	if (!InConfig)
	{
		PGX_LOG_INFO(LogPGXAudio, TEXT("SetLevelAudioConfig — Cleared level config"));
		return;
	}

	PGX_LOG_INFO(LogPGXAudio, TEXT("SetLevelAudioConfig — Applied: %s"), *InConfig->GetName());

	// EN: Apply Level mix layer with channel overrides / ES: Aplicar capa de mezcla Level con overrides de canal
	if (InConfig->ChannelVolumeOverrides.Num() > 0)
	{
		ActivateMixLayer(EPGXMixLayer::Level, InConfig->ChannelVolumeOverrides, InConfig->GetName());
	}

	// EN: Load level-specific ducking config / ES: Cargar config de ducking especifica del nivel
	if (!InConfig->LevelDuckingConfig.IsNull())
	{
		UPGXAudioDuckingConfig* LevelDucking = InConfig->LevelDuckingConfig.LoadSynchronous();
		if (LevelDucking)
		{
			ActiveDuckingConfig = LevelDucking;
			PGX_LOG_INFO(LogPGXAudio, TEXT("  Level ducking config: %s (%d rules)"),
				*LevelDucking->GetName(), LevelDucking->Rules.Num());
		}
	}

	// EN: Apply HDR/HRTF overrides if specified / ES: Aplicar overrides de HDR/HRTF si se especifican
	if (InConfig->HDRAudioOverride >= 0)
	{
		SetHDRAudioEnabled(InConfig->HDRAudioOverride != 0);
	}
	if (InConfig->HRTFOverride >= 0)
	{
		SetHRTFEnabled(InConfig->HRTFOverride != 0);
	}
}

// ── Ducking ──

TArray<FPGXDuckingRule> UPGXAudioMixSubsystem::GetActiveDuckingRules() const
{
	if (ActiveDuckingConfig)
	{
		return ActiveDuckingConfig->Rules;
	}
	return TArray<FPGXDuckingRule>();
}

void UPGXAudioMixSubsystem::SetDuckingEnabled(bool bEnabled)
{
	bDuckingEnabled = bEnabled;
	PGX_LOG_INFO(LogPGXAudio, TEXT("SetDuckingEnabled — %s"), bEnabled ? TEXT("ON") : TEXT("OFF"));
}

// ── HDR / LDR ──

void UPGXAudioMixSubsystem::SetHDRAudioEnabled(bool bEnabled)
{
	if (bHDRAudioEnabled == bEnabled)
	{
		return;
	}

	bHDRAudioEnabled = bEnabled;
	PGX_LOG_INFO(LogPGXAudio, TEXT("SetHDRAudioEnabled — %s"), bEnabled ? TEXT("HDR") : TEXT("LDR"));

	// EN: Submix effect chain swap would happen here via audio device.
	//     Requires the AudioConfig to have valid HDR/LDR chain references.
	// ES: El swap de cadena de efectos submix sucederia aqui via audio device.
	//     Requiere que el AudioConfig tenga referencias validas de cadenas HDR/LDR.
}

// ── HRTF ──

void UPGXAudioMixSubsystem::SetHRTFEnabled(bool bEnabled)
{
	if (bHRTFEnabled == bEnabled)
	{
		return;
	}

	bHRTFEnabled = bEnabled;

	// EN: Toggle HRTF via CVar. 0 = HRTF enabled, 1 = HRTF disabled (inverted logic).
	// ES: Toggle HRTF via CVar. 0 = HRTF habilitado, 1 = HRTF deshabilitado (logica invertida).
	IConsoleVariable* HRTFCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("au.DisableBinauralSpatialization"));
	if (HRTFCVar)
	{
		HRTFCVar->Set(bEnabled ? 0 : 1);
		PGX_LOG_INFO(LogPGXAudio, TEXT("SetHRTFEnabled — %s (CVar au.DisableBinauralSpatialization = %d)"),
			bEnabled ? TEXT("ON") : TEXT("OFF"), bEnabled ? 0 : 1);
	}
	else
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("SetHRTFEnabled — CVar 'au.DisableBinauralSpatialization' not found"));
	}
}

// ── Audio Device ──

TArray<FString> UPGXAudioMixSubsystem::GetAvailableAudioDevices() const
{
	// EN: Audio device enumeration is asynchronous in UE, so this returns the cached list.
	// ES: La enumeracion de dispositivos de audio es asincrona en UE, por lo que se
	//     devuelve la lista cacheada.
	return CachedAudioDevices;
}

FString UPGXAudioMixSubsystem::GetCurrentAudioDevice() const
{
	return CurrentAudioDevice.IsEmpty() ? TEXT("Default") : CurrentAudioDevice;
}

bool UPGXAudioMixSubsystem::SetAudioDevice(const FString& DeviceName)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	PGX_LOG_INFO(LogPGXAudio, TEXT("SetAudioDevice — Requesting switch to: %s"), *DeviceName);

	// EN: Store pending name, bind confirmation callback, only update on success
	// ES: Guardar nombre pendiente, vincular callback de confirmacion, solo actualizar en exito
	PendingDeviceName = DeviceName;
	FOnCompletedDeviceSwap CompletionDelegate;
	CompletionDelegate.BindDynamic(this, &UPGXAudioMixSubsystem::OnDeviceSwapCompleted);
	UAudioMixerBlueprintLibrary::SwapAudioOutputDevice(World, DeviceName, CompletionDelegate);

	return true;
}

void UPGXAudioMixSubsystem::OnDeviceSwapCompleted(const FSwapAudioOutputResult& SwapResult)
{
	if (SwapResult.Result == ESwapAudioOutputDeviceResultState::Success)
	{
		CurrentAudioDevice = PendingDeviceName;
		PGX_LOG_INFO(LogPGXAudio, TEXT("SetAudioDevice — Switch confirmed: %s"), *CurrentAudioDevice);
	}
	else
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("SetAudioDevice — Switch FAILED for: %s (keeping: %s)"),
			*PendingDeviceName, *CurrentAudioDevice);
	}
	PendingDeviceName.Empty();
}

// ── Internal ──

void UPGXAudioMixSubsystem::LoadDuckingConfig()
{
	// EN: Try to load global ducking config from AudioConfig DA
	// ES: Intentar cargar config de ducking global desde AudioConfig DA
	UPGXAudioSubsystem* AudioSub = GetAudioSubsystem();
	if (!AudioSub)
	{
		return;
	}

	const UPGXAudioConfig* Config = AudioSub->GetAudioConfig();
	if (!Config || Config->GlobalDuckingConfig.IsNull())
	{
		return;
	}

	ActiveDuckingConfig = Config->GlobalDuckingConfig.LoadSynchronous();
	if (ActiveDuckingConfig)
	{
		PGX_LOG_INFO(LogPGXAudio, TEXT("LoadDuckingConfig — Global ducking loaded: %s (%d rules)"),
			*ActiveDuckingConfig->GetName(), ActiveDuckingConfig->Rules.Num());
	}
}

void UPGXAudioMixSubsystem::RecalculateEffectiveVolumes()
{
	// EN: Walk all active layers in priority order and compute final per-channel volumes.
	//     Higher priority layers override lower ones.
	// ES: Recorrer todas las capas activas en orden de prioridad y calcular volumenes finales por canal.
	//     Capas de mayor prioridad sobreescriben las de menor.
	UPGXAudioBackend* Backend = GetActiveBackend();
	if (!Backend)
	{
		return;
	}

	// EN: Collect all unique channel tags across layers / ES: Recopilar todos los tags de canal unicos entre capas
	TMap<FGameplayTag, float> EffectiveValues;

	for (int32 i = 0; i < MixLayers.Num(); ++i)
	{
		if (!MixLayers[i].bActive)
		{
			continue;
		}

		for (const auto& Pair : MixLayers[i].ChannelValues)
		{
			// EN: Higher layers override lower ones (last write wins)
			// ES: Capas superiores sobreescriben inferiores (ultima escritura gana)
			EffectiveValues.Add(Pair.Key, Pair.Value);
		}
	}

	// EN: Apply effective values through backend / ES: Aplicar valores efectivos a traves del backend
	for (const auto& Pair : EffectiveValues)
	{
		Backend->SetChannelVolume(Pair.Key, Pair.Value);
	}
}

UPGXAudioSubsystem* UPGXAudioMixSubsystem::GetAudioSubsystem() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		return nullptr;
	}

	return GI->GetSubsystem<UPGXAudioSubsystem>();
}

UPGXAudioBackend* UPGXAudioMixSubsystem::GetActiveBackend() const
{
	UPGXAudioSubsystem* AudioSub = GetAudioSubsystem();
	return AudioSub ? AudioSub->GetActiveBackend() : nullptr;
}
