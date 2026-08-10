// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

class UWorld;

#include "CoreMinimal.h"
#include "Base/PGXGameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Interfaces/PGXTaggedRegistry.h"
#include "PGXAudioTypes.h"
#include "PGXAudioDelegates.h"
#include "PGXAudioSubsystem.generated.h"

// Forward declarations
class UPGXAudioConfig;
class UPGXAudioBackend;
class UPGXMusicManager;
class UPGXSoundPool;
class UPGXDialogueManager;
class UPGXSoundDefinition;
class UPGXAudioProfile;
class UPGXAudioChannelConfig;
class UPGXMusicPlaylist;
class UAudioComponent;
class USceneComponent;

/**
 * EN: Central audio subsystem (GameInstance scope — global, persistent).
 *     Manages backend selection, channel volumes, sound playback,
 *     music manager, sound pool, and event history.
 *     Works in tandem with UPGXAudioMixSubsystem (WorldSubsystem, per-level).
 *
 * ES: Subsistema central de audio (scope GameInstance — global, persistente).
 *     Gestiona seleccion de backend, volumenes de canal, reproduccion de sonido,
 *     music manager, sound pool e historial de eventos.
 *     Trabaja en conjunto con UPGXAudioMixSubsystem (WorldSubsystem, por nivel).
 */
UCLASS(BlueprintType)
class PGXAUDIORUNTIME_API UPGXAudioSubsystem : public UPGXGameInstanceSubsystem, public IPGXTaggedRegistry
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface

	// ── State ──

	/** EN: Get the current system state / ES: Obtener el estado actual del sistema */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Audio")
	EPGXAudioState GetAudioState() const { return AudioState; }

	/** EN: Check if the system is ready for use / ES: Comprobar si el sistema esta listo para usar */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Audio")
	bool IsInitialized() const { return AudioState == EPGXAudioState::Ready; }

	// ── Backend ──

	/** EN: Get the currently active backend type / ES: Obtener el tipo de backend activo actualmente */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Audio|Backend")
	EPGXAudioBackendType GetActiveBackendType() const;

	/**
	 * EN: Hot-switch the audio backend. Fades all sounds, swaps, restores channels.
	 * ES: Hot-switch del backend de audio. Fade de todos los sonidos, intercambio, restaurar canales.
	 * @param NewType The target backend type.
	 * @return true if switch succeeded.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Backend")
	bool SwitchBackend(EPGXAudioBackendType NewType);

	/** EN: Hot-switch backend with typed state-transfer diagnostics / ES: Cambiar backend con diagnostico tipado de transferencia de estado */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Backend")
	FPGXAudioBackendSwitchResult SwitchBackendDetailed(EPGXAudioBackendType NewType);

	/** EN: Get human-readable backend status text / ES: Obtener texto de estado del backend legible */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Audio|Backend")
	FString GetBackendStatusText() const;

	// ── Volume Control ──

	/** EN: Set volume for a channel (0.0-1.0) / ES: Establecer volumen de un canal (0.0-1.0) */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Volume")
	void SetChannelVolume(FGameplayTag ChannelTag, float Volume);

	/** EN: Get current volume of a channel / ES: Obtener volumen actual de un canal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Audio|Volume")
	float GetChannelVolume(FGameplayTag ChannelTag) const;

	/** EN: Set mute state for a channel / ES: Establecer estado de mute para un canal */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Volume")
	void SetChannelMuted(FGameplayTag ChannelTag, bool bMuted);

	/** EN: Check if a channel is muted / ES: Comprobar si un canal esta muteado */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Audio|Volume")
	bool IsChannelMuted(FGameplayTag ChannelTag) const;

	/** EN: Mute/unmute all channels / ES: Mutear/desmutear todos los canales */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Volume")
	void SetMuteAll(bool bMuted);

	/** EN: Check if global mute is active / ES: Comprobar si el mute global esta activo */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Audio|Volume")
	bool IsMuteAll() const { return bGlobalMute; }

	/** EN: Get snapshot of all channel states / ES: Obtener snapshot de todos los estados de canal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Audio|Volume")
	TArray<FPGXAudioChannelSnapshot> GetAllChannelStates() const;

	// ── Query ──

	/** EN: Get count of currently active sounds / ES: Obtener conteo de sonidos activos actualmente */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Audio|Query")
	int32 GetActiveSoundCount() const;

	/** EN: Get info about all active sounds / ES: Obtener info sobre todos los sonidos activos */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Audio|Query")
	TArray<FPGXActiveSoundInfo> GetActiveSounds() const;

	/** EN: Get complete system snapshot / ES: Obtener snapshot completo del sistema */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Audio|Query")
	FPGXAudioSystemSnapshot GetAudioSnapshot() const;

	/** EN: Get pool statistics / ES: Obtener estadisticas del pool */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Audio|Query")
	FPGXSoundPoolStats GetPoolStatistics() const;

	/** EN: Get memory usage estimate / ES: Obtener estimacion de uso de memoria */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Audio|Query")
	FPGXAudioMemoryInfo GetMemoryEstimate() const;

	/** EN: Get event history (last N events) / ES: Obtener historial de eventos (ultimos N eventos) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Audio|Query")
	TArray<FPGXAudioEventRecord> GetEventHistory(int32 MaxCount = 50) const;

	// ── Playback ──

	/** EN: Play a sound in 2D (no spatialization) / ES: Reproducir un sonido en 2D (sin espacializacion) */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Playback")
	FPGXSoundHandle PlaySound2D(USoundBase* Sound, const FPGXAudioPlayParams& Params);

	/** EN: Play a sound at a world location / ES: Reproducir un sonido en una ubicacion del mundo */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Playback")
	FPGXSoundHandle PlaySoundAtLocation(USoundBase* Sound, FVector Location, FRotator Rotation, const FPGXAudioPlayParams& Params);

	/** EN: Play a sound attached to a component / ES: Reproducir un sonido adjunto a un componente */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Playback")
	FPGXSoundHandle PlaySoundAttached(USoundBase* Sound, USceneComponent* AttachComponent, FName AttachPointName, const FPGXAudioPlayParams& Params);

	/** EN: Stop a specific sound by handle / ES: Detener un sonido especifico por handle */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Playback")
	void StopSound(FPGXSoundHandle Handle, float FadeOutDuration = 0.0f);

	/** EN: Stop all currently playing sounds / ES: Detener todos los sonidos reproduciendose actualmente */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Playback")
	void StopAllSounds(float FadeOutDuration = 0.0f);

	// ── Resolution ──

	/**
	 * EN: Resolve a SoundDefinition to a concrete USoundBase via context-tag matching.
	 * ES: Resolver un SoundDefinition a un USoundBase concreto via matching de tags de contexto.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Resolution")
	USoundBase* ResolveSound(const UPGXSoundDefinition* Definition, const FGameplayTagContainer& ContextTags);

	/**
	 * EN: Resolve and immediately play a sound from a SoundDefinition.
	 * ES: Resolver e inmediatamente reproducir un sonido desde un SoundDefinition.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Resolution")
	FPGXSoundHandle PlayResolved(const UPGXSoundDefinition* Definition, const FPGXAudioPlayParams& Params);

	/**
	 * EN: Find SoundDefinition DA by SoundTag (uses cache, O(1) lookup).
	 * ES: Encontrar SoundDefinition DA por SoundTag (usa cache, lookup O(1)).
	 */
	const UPGXSoundDefinition* FindDefinitionByTag(const FGameplayTag& SoundTag) const;

	// ── Music ──

	/** EN: Play a music track / ES: Reproducir una pista musical */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Music")
	void PlayMusic(USoundBase* Music, float FadeInDuration = 0.0f);

	/** EN: Stop current music / ES: Detener musica actual */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Music")
	void StopMusic(float FadeOutDuration = 2.0f);

	/** EN: Pause current music / ES: Pausar musica actual */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Music")
	void PauseMusic();

	/** EN: Resume paused music / ES: Reanudar musica pausada */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Music")
	void ResumeMusic();

	/** EN: Crossfade to a new music track / ES: Crossfade a una nueva pista musical */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Music")
	void CrossfadeTo(USoundBase* NewMusic, float CrossfadeDuration = 2.0f);

	/** EN: Play a music playlist / ES: Reproducir un playlist de musica */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Music")
	void PlayPlaylist(const UPGXMusicPlaylist* Playlist);

	/** EN: Set music state tag / ES: Establecer tag de estado musical */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Music")
	void SetMusicState(FGameplayTag StateTag);

	/** EN: Get current music state tag / ES: Obtener tag de estado musical actual */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Audio|Music")
	FGameplayTag GetMusicState() const;

	/** EN: Get current music playback state / ES: Obtener estado de reproduccion musical actual */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Audio|Music")
	EPGXMusicState GetMusicPlaybackState() const;

	/** EN: Get current track name / ES: Obtener nombre de pista actual */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Audio|Music")
	FString GetCurrentTrackName() const;

	// ── Dialogue ──

	/** EN: Queue a dialogue for playback / ES: Encolar un dialogo para reproduccion */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Dialogue")
	bool QueueDialogue(USoundBase* Sound, FText SubtitleText, float Duration,
		FGameplayTag SpeakerTag, FGameplayTag PriorityTag,
		EPGXDialogueInterruptPolicy Policy = EPGXDialogueInterruptPolicy::QueueBehind);

	/** EN: Stop current dialogue / ES: Detener dialogo actual */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Dialogue")
	void StopDialogue(float FadeOutDuration = 0.0f);

	/** EN: Clear dialogue queue / ES: Limpiar cola de dialogo */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Dialogue")
	void ClearDialogueQueue();

	/** EN: Get dialogue queue count / ES: Obtener cantidad en cola de dialogo */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Audio|Dialogue")
	int32 GetDialogueQueueCount() const;

	/** EN: Check if dialogue is playing / ES: Comprobar si un dialogo esta reproduciendose */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Audio|Dialogue")
	bool IsDialoguePlaying() const;

	// ── Config ──

	/** EN: Get the loaded audio config DA / ES: Obtener el config DA de audio cargado */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Audio")
	const UPGXAudioConfig* GetAudioConfig() const { return AudioConfig; }

	// ── Delegates (Blueprint) ──

	UPROPERTY(BlueprintAssignable, Category = "PGX|Audio|Delegates")
	FOnPGXAudioSystemReady OnAudioSystemReady;

	UPROPERTY(BlueprintAssignable, Category = "PGX|Audio|Delegates")
	FOnPGXSoundPlayed OnSoundPlayed;

	UPROPERTY(BlueprintAssignable, Category = "PGX|Audio|Delegates")
	FOnPGXSoundStopped OnSoundStopped;

	UPROPERTY(BlueprintAssignable, Category = "PGX|Audio|Delegates")
	FOnPGXChannelVolumeChanged OnChannelVolumeChanged;

	UPROPERTY(BlueprintAssignable, Category = "PGX|Audio|Delegates")
	FOnPGXMuteChanged OnMuteChanged;

	UPROPERTY(BlueprintAssignable, Category = "PGX|Audio|Delegates")
	FOnPGXMusicStateChanged OnMusicStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "PGX|Audio|Delegates")
	FOnPGXBackendSwitched OnBackendSwitched;

	UPROPERTY(BlueprintAssignable, Category = "PGX|Audio|Delegates")
	FOnPGXPoolExhausted OnPoolExhausted;

	UPROPERTY(BlueprintAssignable, Category = "PGX|Audio|Delegates")
	FOnPGXMixLayerChanged OnMixLayerChanged;

	UPROPERTY(BlueprintAssignable, Category = "PGX|Audio|Delegates")
	FOnPGXDuckingChanged OnDuckingChanged;

	UPROPERTY(BlueprintAssignable, Category = "PGX|Audio|Delegates")
	FOnPGXDialogueSubtitle OnDialogueSubtitle;

	UPROPERTY(BlueprintAssignable, Category = "PGX|Audio|Delegates")
	FOnPGXMixSubsystemReady OnMixSubsystemReady;

	// ── Delegates (Native C++) ──

	FOnPGXAudioSystemReadyNative OnAudioSystemReadyNative;
	FOnPGXSoundPlayedNative OnSoundPlayedNative;
	FOnPGXSoundStoppedNative OnSoundStoppedNative;
	FOnPGXChannelVolumeChangedNative OnChannelVolumeChangedNative;
	FOnPGXMuteChangedNative OnMuteChangedNative;
	FOnPGXMusicStateChangedNative OnMusicStateChangedNative;
	FOnPGXBackendSwitchedNative OnBackendSwitchedNative;
	FOnPGXPoolExhaustedNative OnPoolExhaustedNative;
	FOnPGXMixLayerChangedNative OnMixLayerChangedNative;
	FOnPGXDuckingChangedNative OnDuckingChangedNative;
	FOnPGXDialogueSubtitleNative OnDialogueSubtitleNative;
	FOnPGXMixSubsystemReadyNative OnMixSubsystemReadyNative;

	// ── Internal Accessors (for MusicManager, SoundPool, etc.) ──

	/** EN: Get the active backend (can be null before init) / ES: Obtener el backend activo (puede ser null antes de init) */
	UPGXAudioBackend* GetActiveBackend() const { return ActiveBackend; }

	/** EN: Get the music manager / ES: Obtener el music manager */
	UPGXMusicManager* GetMusicManager() const { return MusicManager; }

	/** EN: Get the sound pool / ES: Obtener el sound pool */
	UPGXSoundPool* GetSoundPool() const { return SoundPool; }

	/** EN: Get the dialogue manager / ES: Obtener el dialogue manager */
	UPGXDialogueManager* GetDialogueManager() const { return DialogueManager; }

	/** EN: Get cached channel configs (populated at init + test injection) / ES: Obtener configs de canal cacheados (poblados en init + inyeccion de test) */
	const TArray<TWeakObjectPtr<const UPGXAudioChannelConfig>>& GetCachedChannelConfigs() const { return CachedChannelConfigs; }

#if WITH_EDITOR || WITH_DEV_AUTOMATION_TESTS
	/** EN: Inject a test channel config into the cache (editor/test only) / ES: Inyectar un config de canal de test en el cache (solo editor/test) */
	void InjectTestChannelConfig(const UPGXAudioChannelConfig* Config);

	/** EN: Remove all transient (test-injected) channel configs / ES: Remover todos los configs de canal transient (inyectados por test) */
	void ClearTestChannelConfigs();

	/** EN: Inject a test audio config DA (editor/test only) / ES: Inyectar un config DA de audio de test (solo editor/test) */
	void InjectTestAudioConfig(const UPGXAudioConfig* Config);

	/** EN: Remove injected test audio config / ES: Remover config de audio de test inyectado */
	void ClearTestAudioConfig();
#endif

	// ========================================================================
	// EN: IPGXTaggedRegistry adoption — CachedProfiles facade
	// ES: Adopcion IPGXTaggedRegistry — fachada de CachedProfiles
	// ========================================================================

	/** EN: True if CachedProfiles contains the tag / ES: True si CachedProfiles contiene el tag. */
	bool HasEntryByTag(FGameplayTag Tag) const override;

	/** EN: Number of cached audio profiles / ES: Numero de perfiles de audio cacheados. */
	int32 GetCount() const override;

	/** EN: Snapshot of cached audio-profile tags / ES: Snapshot de tags de perfil de audio cacheados. */
	void GetSnapshot(TArray<FGameplayTag>& OutTags) const override;

#if WITH_DEV_AUTOMATION_TESTS
	/** EN: Inject a test SoundDefinition into the deterministic lookup cache / ES: Inyectar SoundDefinition de test en cache determinista */
	void InjectTestSoundDefinition(const UPGXSoundDefinition* Definition);

	/** EN: Remove transient SoundDefinitions from the deterministic lookup cache / ES: Remover SoundDefinitions transient del cache determinista */
	void ClearTestSoundDefinitions();

	/** EN: Clear event history for automation isolation / ES: Limpiar historial de eventos para aislamiento de automatizacion */
	void ClearEventHistoryForTesting();

	/** EN: Record an event through production policy for automation / ES: Registrar evento via politica productiva para automatizacion */
	void RecordEventForTesting(const FGameplayTag& EventTag, const FString& SoundName = TEXT(""),
		const FGameplayTag& ChannelTag = FGameplayTag(), const FGameplayTag& ProfileTag = FGameplayTag(),
		bool bSuccess = true, float Duration = 0.0f);

	/** EN: Expose production console mutation policy to automation / ES: Exponer politica productiva de mutacion por consola a automatizacion */
	bool AreConsoleMutationsAllowedForTesting() const;

	/** EN: Expose production event record policy to automation / ES: Exponer politica productiva de registro de eventos a automatizacion */
	bool ShouldRecordEventHistoryForTesting() const;

	/** EN: Expose production event query policy to automation / ES: Exponer politica productiva de consulta de eventos a automatizacion */
	bool ShouldExposeEventHistoryForTesting() const;

	/** EN: Expose resolved event history capacity to automation / ES: Exponer capacidad resuelta de historial a automatizacion */
	int32 GetResolvedMaxEventHistorySizeForTesting() const;

	/** EN: Replace active backend for branch-isolated automation / ES: Reemplazar backend activo para automatizacion aislada por rama */
	void SetActiveBackendForTesting(UPGXAudioBackend* Backend);

	/** EN: Force the next detailed switch result type to exercise target-mismatch branch / ES: Forzar tipo del siguiente switch detallado para probar mismatch */
	void ForceNextSwitchActiveBackendTypeForTesting(EPGXAudioBackendType ForcedType);
#endif

protected:
	// ── Internal ──

	/** EN: Load config DA from AssetRegistry / ES: Cargar config DA desde AssetRegistry */
	void LoadConfig();

	/** EN: Create and initialize the appropriate backend / ES: Crear e inicializar el backend apropiado */
	bool CreateBackend(EPGXAudioBackendType RequestedType);

	/** EN: Register console commands / ES: Registrar comandos de consola */
	/** EN: Unregister console commands / ES: Desregistrar comandos de consola */
	/** EN: Record an event to the history log / ES: Registrar un evento en el log de historial */
	void RecordEvent(const FGameplayTag& EventTag, const FString& SoundName = TEXT(""),
		const FGameplayTag& ChannelTag = FGameplayTag(), const FGameplayTag& ProfileTag = FGameplayTag(),
		bool bSuccess = true, float Duration = 0.0f);

	/** EN: Generate next unique sound handle ID / ES: Generar siguiente ID unico de sound handle */
	int32 GenerateHandleId();

	/** EN: Resolve config-driven event history capacity / ES: Resolver capacidad de historial desde config */
	int32 GetResolvedMaxEventHistorySize() const;

	/** EN: Whether event history should be recorded in this build / ES: Si debe registrarse historial en este build */
	bool ShouldRecordEventHistory() const;

	/** EN: Whether event history should be exposed by queries in this build / ES: Si las consultas pueden exponer historial en este build */
	bool ShouldExposeEventHistory() const;

	/** EN: Whether mutating console commands are allowed / ES: Si se permiten comandos de consola mutadores */
	bool AreConsoleMutationsAllowed() const;

private:
	// ── State ──

	/** EN: Current system state / ES: Estado actual del sistema */
	EPGXAudioState AudioState = EPGXAudioState::Uninitialized;

	/** EN: Global mute flag / ES: Flag de mute global */
	bool bGlobalMute = false;

	/** EN: Next handle ID counter / ES: Contador de siguiente ID de handle */
	int32 NextHandleId = 1;

	/** EN: Per-definition sequential variant index (decoupled from HandleId) / ES: Indice secuencial de variante por definicion (desacoplado de HandleId) */
	TMap<const UPGXSoundDefinition*, int32> SequentialVariantIndex;

	// ── Owned Objects ──

	/** EN: Loaded audio config DA / ES: Config DA de audio cargado */
	UPROPERTY()
	TObjectPtr<const UPGXAudioConfig> AudioConfig = nullptr;

	/** EN: Active backend implementation / ES: Implementacion de backend activa */
	UPROPERTY()
	TObjectPtr<UPGXAudioBackend> ActiveBackend = nullptr;

	/** EN: Music management system / ES: Sistema de gestion musical */
	UPROPERTY()
	TObjectPtr<UPGXMusicManager> MusicManager = nullptr;

	/** EN: Sound instance pool / ES: Pool de instancias de sonido */
	UPROPERTY()
	TObjectPtr<UPGXSoundPool> SoundPool = nullptr;

	/** EN: Dialogue management system / ES: Sistema de gestion de dialogo */
	UPROPERTY()
	TObjectPtr<UPGXDialogueManager> DialogueManager = nullptr;

	// ── Runtime Data ──

	/** EN: Active sounds tracked by handle / ES: Sonidos activos rastreados por handle */
	TMap<int32, FPGXActiveSoundInfo> ActiveSoundsMap;

	/** EN: Event history ring buffer / ES: Buffer circular de historial de eventos */
	TArray<FPGXAudioEventRecord> EventHistory;

	/** EN: Max event history entries / ES: Maximo de entradas de historial de eventos */
	static constexpr int32 MaxEventHistory = 200;

	/** EN: Console command handles for cleanup / ES: Handles de comandos de consola para limpieza */
	/** EN: Map from handle ID to audio component for playback tracking / ES: Mapa de ID de handle a componente de audio para rastreo de reproduccion */
	TMap<int32, TWeakObjectPtr<UAudioComponent>> HandleToComponentMap;

	// ── Asset Discovery Cache (populated at init, avoids per-frame AssetRegistry scans) ──

	/** EN: Cached channel configs discovered at init / ES: Configs de canal cacheados al inicializar */
	TArray<TWeakObjectPtr<const UPGXAudioChannelConfig>> CachedChannelConfigs;

	/** EN: Cached audio profiles by tag / ES: Perfiles de audio cacheados por tag */
	TMap<FGameplayTag, TWeakObjectPtr<const UPGXAudioProfile>> CachedProfiles;

	/** EN: Cached sound definitions by SoundTag / ES: Definiciones de sonido cacheadas por SoundTag */
	TMap<FGameplayTag, TWeakObjectPtr<const UPGXSoundDefinition>> CachedSoundDefinitions;

#if WITH_DEV_AUTOMATION_TESTS
	/** EN: Test-only override for the next SwitchBackendDetailed active-type check / ES: Override solo test para el siguiente check de tipo activo */
	bool bForceNextSwitchResultActiveBackendForTesting = false;

	/** EN: Forced active backend type consumed by the next SwitchBackendDetailed call / ES: Tipo activo forzado consumido por el siguiente SwitchBackendDetailed */
	EPGXAudioBackendType ForcedNextSwitchResultActiveBackendForTesting = EPGXAudioBackendType::Auto;
#endif

	/** EN: Populate audio asset caches from AssetRegistry / ES: Poblar caches de assets de audio desde AssetRegistry */
	void CacheAudioAssets();

	/** EN: Find audio profile DA by tag (uses cache) / ES: Encontrar DA de perfil de audio por tag (usa cache) */
	const UPGXAudioProfile* FindAudioProfile(const FGameplayTag& ProfileTag) const;

	/** EN: Find a MixSubsystem from any active game world (for console commands) / ES: Encontrar un MixSubsystem de cualquier mundo de juego activo (para comandos de consola) */
	class UPGXAudioMixSubsystem* FindMixSubsystem() const;

	/** EN: Debug overlay flag / ES: Flag de overlay de debug */
	bool bDebugOverlay = false;

	// ── Profile Integration ──

	void ApplyProfileConstraints(const struct FPGXResolvedProfile& Profile);
	void HandleProfileChanged(const struct FPGXResolvedProfile& OldProfile, const struct FPGXResolvedProfile& NewProfile);

private:
	friend class FPGXAudioRuntimeModule;
	void ExecuteConsoleCommand(const FString& CommandName, const TArray<FString>& Args, UWorld* World);
};
