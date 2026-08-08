// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXSettings.h"
#include "PGXAudioTypes.h"
#include "PGXAudioSettings.generated.h"

class UPGXAudioConfig;
class UDataTable;

/**
 * EN: Audio settings exposed in Project Settings > PGX > Audio.
 *     Configures default backend preference, debug options, and developer toggles.
 *     Runtime user preferences (volumes, mute) are persisted via Save system instead.
 *
 * ES: Settings de audio expuestos en Project Settings > PGX > Audio.
 *     Configura preferencia de backend por defecto, opciones de debug y toggles de desarrollo.
 *     Preferencias de usuario en runtime (volumenes, mute) se persisten via sistema Save.
 */
UCLASS(config=PGX, defaultconfig, meta = (DisplayName = "PGX Audio"))
class PGXAUDIORUNTIME_API UPGXAudioSettings : public UPGXSettings
{
	GENERATED_BODY()

public:
	//~ Begin UDeveloperSettings Interface
	FName GetSectionName() const override { return TEXT("Audio"); }
	//~ End UDeveloperSettings Interface

	// ── Config Resolution ──

	/**
	 * EN: The active Audio config DA. If empty, falls back to AssetRegistry discovery (deprecated).
	 * ES: El DA de config de Audio activo. Si esta vacio, hace fallback a discovery de AssetRegistry (deprecated).
	 */
	UPROPERTY(config, EditAnywhere, Category = "Config",
		meta = (DisplayName = "Active Config"))
	TSoftObjectPtr<UPGXAudioConfig> ActiveConfig;

	/**
	 * EN: DataTable with audio channel rows (FPGXAudioChannelRow).
	 *     Each row maps a channel GameplayTag to a UPGXAudioChannelConfig DA.
	 *     If empty, falls back to AssetRegistry scan (deprecated).
	 * ES: DataTable con filas de canal de audio (FPGXAudioChannelRow).
	 *     Cada fila mapea un GameplayTag de canal a un DA UPGXAudioChannelConfig.
	 *     Si esta vacio, hace fallback a escaneo de AssetRegistry (deprecated).
	 */
	UPROPERTY(config, EditAnywhere, Category = "Config|Channels",
		meta = (DisplayName = "Channel Table"))
	TSoftObjectPtr<UDataTable> ChannelTable;

	/**
	 * EN: DataTable with audio profile rows (FPGXAudioProfileRow).
	 *     Each row maps a profile GameplayTag to a UPGXAudioProfile DA.
	 *     If empty, falls back to AssetRegistry scan (deprecated).
	 * ES: DataTable con filas de perfil de audio (FPGXAudioProfileRow).
	 *     Cada fila mapea un GameplayTag de perfil a un DA UPGXAudioProfile.
	 *     Si esta vacio, hace fallback a escaneo de AssetRegistry (deprecated).
	 */
	UPROPERTY(config, EditAnywhere, Category = "Config|Profiles",
		meta = (DisplayName = "Profile Table"))
	TSoftObjectPtr<UDataTable> ProfileTable;

	// ── Backend ──

	/** EN: Preferred backend type for this project / ES: Tipo de backend preferido para este proyecto */
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Backend")
	EPGXAudioBackendType PreferredBackend = EPGXAudioBackendType::Auto;

	// ── Debug ──

	/** EN: Enable audio debug overlay in PIE / ES: Habilitar overlay de debug de audio en PIE */
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Debug")
	bool bEnableDebugOverlay = false;

	/** EN: Enable verbose audio logging / ES: Habilitar logging verbose de audio */
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Debug")
	bool bVerboseLogging = false;

	/** EN: Show sound play/stop events in output log / ES: Mostrar eventos play/stop de sonido en output log */
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Debug")
	bool bLogSoundEvents = false;

	// ── Spatial ──

	/** EN: Default HRTF (binaural spatialization) enabled for new projects / ES: HRTF (espacializacion binaural) habilitado por defecto para proyectos nuevos */
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Spatial")
	bool bDefaultHRTFEnabled = false;

	/** EN: Default HDR audio enabled / ES: Audio HDR habilitado por defecto */
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Spatial")
	bool bDefaultHDRAudioEnabled = false;
};
