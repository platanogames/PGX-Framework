// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "PGXAudioTypes.h"
#include "PGXAudioTestUtility.generated.h"

class UPGXAudioSubsystem;

/**
 * EN: Test utility for the PGX Audio system.
 *     Provides Blueprint-callable test/debug helpers for validating
 *     audio configuration, playback, and system health.
 *     Designed for PIE testing and automation.
 *
 * ES: Utilidad de test para el sistema PGX Audio.
 *     Provee helpers de test/debug llamables desde Blueprint para validar
 *     configuracion de audio, reproduccion y salud del sistema.
 *     Disenado para testing PIE y automatizacion.
 */
UCLASS()
class PGXAUDIORUNTIME_API UPGXAudioTestUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * EN: Validate that the audio system initialized correctly.
	 * ES: Validar que el sistema de audio se inicializo correctamente.
	 * @return true if state == Ready, backend operational, music/pool/dialogue managers valid.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Test", meta = (WorldContext = "WorldContextObject"))
	static bool ValidateSystemHealth(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/**
	 * EN: Validate all audio channel configurations (ChannelConfig DAs).
	 * ES: Validar todas las configuraciones de canal de audio (ChannelConfig DAs).
	 * @return true if all channels have valid tags, display names, and backend refs.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Test", meta = (WorldContext = "WorldContextObject"))
	static bool ValidateChannelConfigs(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/**
	 * EN: Validate all SoundDefinition DAs for common errors.
	 * ES: Validar todos los SoundDefinition DAs para errores comunes.
	 * @return true if all definitions have valid tags, at least one variant, and sound refs.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Test", meta = (WorldContext = "WorldContextObject"))
	static bool ValidateSoundDefinitions(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/**
	 * EN: Test sound resolution pipeline (resolve but don't play).
	 * ES: Testear pipeline de resolucion de sonido (resolver pero no reproducir).
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Test", meta = (WorldContext = "WorldContextObject"))
	static bool TestSoundResolution(const UObject* WorldContextObject, FGameplayTag SoundTag,
		const FGameplayTagContainer& ContextTags, FString& OutResolvedName);

	/**
	 * EN: Test backend switch (switch to target, verify, switch back).
	 * ES: Testear cambio de backend (cambiar al objetivo, verificar, volver).
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Test", meta = (WorldContext = "WorldContextObject"))
	static bool TestBackendSwitch(const UObject* WorldContextObject, EPGXAudioBackendType TargetType, TArray<FString>& OutIssues);

	/**
	 * EN: Validate all MusicPlaylist DAs for common errors.
	 * ES: Validar todos los MusicPlaylist DAs para errores comunes.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Test", meta = (WorldContext = "WorldContextObject"))
	static bool ValidateMusicPlaylists(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/**
	 * EN: Run all validation tests and return aggregate result.
	 * ES: Ejecutar todas las pruebas de validacion y retornar resultado agregado.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunAllTests(const UObject* WorldContextObject, TArray<FString>& OutIssues);
};
