// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "Observability/PGXValidationResult.h"
#include "PGXAudioTypes.generated.h"

// ============================================================================
// EN: Core types for the PGX Audio system v1.1.
//     Defines enums (backend, state machines, results, policies, layers),
//     structs (handle, play params, variants, ducking rules, snapshots).
//     All extensible hierarchies use FGameplayTag. Enums only for fixed states.
//
// ES: Tipos centrales para el sistema PGX Audio v1.1.
//     Define enums (backend, maquinas de estado, resultados, politicas, capas),
//     structs (handle, params de reproduccion, variantes, reglas de ducking, snapshots).
//     Todas las jerarquias extensibles usan FGameplayTag. Enums solo para estados fijos.
// ============================================================================

// ── Backend Type ──

/** EN: Audio backend implementation type / ES: Tipo de implementacion del backend de audio */
UENUM(BlueprintType)
enum class EPGXAudioBackendType : uint8
{
	/** EN: SoundMix + SoundClass (UE legacy) / ES: SoundMix + SoundClass (UE legacy) */
	Legacy      = 0  UMETA(DisplayName = "Legacy"),
	/** EN: ControlBus + ControlBusMix (AudioModulation) / ES: ControlBus + ControlBusMix (AudioModulation) */
	Modulation  = 1  UMETA(DisplayName = "Modulation"),
	/** EN: Auto-detect best available / ES: Auto-detectar el mejor disponible */
	Auto        = 2  UMETA(DisplayName = "Auto")
};

// ── Backend Switch Result ──

/** EN: Detailed result for deterministic backend hot-switches / ES: Resultado detallado para hot-switch determinista de backend */
UENUM(BlueprintType)
enum class EPGXAudioBackendSwitchStatus : uint8
{
	Success               = 0  UMETA(DisplayName = "Success"),
	AlreadyActive         = 1  UMETA(DisplayName = "Already Active"),
	Failed_NoBackend      = 2  UMETA(DisplayName = "Failed: No Backend"),
	Failed_CreateTarget   = 3  UMETA(DisplayName = "Failed: Create Target"),
	Failed_TargetMismatch = 4  UMETA(DisplayName = "Failed: Target Mismatch"),
	Failed_Rollback       = 5  UMETA(DisplayName = "Failed: Rollback")
};

/** EN: State-transfer summary for backend switch attempts / ES: Resumen de transferencia de estado para intentos de cambio de backend */
USTRUCT(BlueprintType)
struct PGXAUDIORUNTIME_API FPGXAudioBackendSwitchResult
{
	GENERATED_BODY()

	/** EN: Whether the requested switch completed / ES: Si el cambio solicitado se completo */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio")
	bool bSuccess = false;

	/** EN: Detailed status / ES: Estado detallado */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio")
	EPGXAudioBackendSwitchStatus Status = EPGXAudioBackendSwitchStatus::Failed_NoBackend;

	/** EN: Backend before the switch / ES: Backend antes del cambio */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio")
	EPGXAudioBackendType SourceBackend = EPGXAudioBackendType::Auto;

	/** EN: Requested backend / ES: Backend solicitado */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio")
	EPGXAudioBackendType RequestedBackend = EPGXAudioBackendType::Auto;

	/** EN: Active backend after the attempt / ES: Backend activo despues del intento */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio")
	EPGXAudioBackendType ActiveBackend = EPGXAudioBackendType::Auto;

	/** EN: Channel states restored after the switch / ES: Estados de canal restaurados tras el cambio */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio")
	int32 RestoredChannelCount = 0;

	/** EN: Active sounds tracked during the switch / ES: Sonidos activos rastreados durante el cambio */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio")
	int32 PreservedActiveSoundCount = 0;

	/** EN: Human-readable outcome / ES: Resultado legible */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio")
	FString Message;

	/**
	 * EN: Convert to the canonical FPGXValidationResult for cross-cutting
	 *     concerns. Maps bSuccess -> bValid, EPGXAudioBackendSwitchStatus
	 *     (via integer Code) -> FName. Domain fields (SourceBackend,
	 *     RequestedBackend, ActiveBackend, channel/sound counts) are NOT
	 *     carried — they stay in this local struct.
	 *
	 *     Converts the operation result to the shared FPGXValidationResult format.
	 *
	 * ES: Convertir al FPGXValidationResult canonico. Mapea bSuccess -> bValid,
	 *     EPGXAudioBackendSwitchStatus (vía Code int) -> FName. Los campos de
	 *     dominio (SourceBackend, RequestedBackend, ActiveBackend, channel/sound
	 *     counts) NO se llevan — se quedan en la struct local.
	 */
	FPGXValidationResult ToValidationResult() const
	{
		if (bSuccess)
		{
			return FPGXValidationResult::MakeValid();
		}
		// EN: BackendSwitchStatus is the typed Code; carry as int32-bridged FName
		//     using FPGXValidationResult::MakeFromCode for the path.
		// ES: BackendSwitchStatus es el Code tipado; llevamos como int32-bridged
		//     FName usando FPGXValidationResult::MakeFromCode.
		return FPGXValidationResult::MakeFromCode(
			static_cast<int32>(Status),
			Message);
	}
};

// ── System State ──

/** EN: Audio subsystem operational state / ES: Estado operativo del subsistema de audio */
UENUM(BlueprintType)
enum class EPGXAudioState : uint8
{
	Uninitialized  = 0  UMETA(DisplayName = "Uninitialized"),
	Initializing   = 1  UMETA(DisplayName = "Initializing"),
	Ready          = 2  UMETA(DisplayName = "Ready"),
	Error          = 3  UMETA(DisplayName = "Error"),
	Suspended      = 4  UMETA(DisplayName = "Suspended")
};

// ── Music State Machine ──

/** EN: Music manager playback state / ES: Estado de reproduccion del music manager */
UENUM(BlueprintType)
enum class EPGXMusicState : uint8
{
	Idle          = 0  UMETA(DisplayName = "Idle"),
	Playing       = 1  UMETA(DisplayName = "Playing"),
	CrossFading   = 2  UMETA(DisplayName = "CrossFading"),
	Paused        = 3  UMETA(DisplayName = "Paused"),
	Stopping      = 4  UMETA(DisplayName = "Stopping")
};

// ── Sound Play Result ──

/** EN: Result of attempting to play a sound / ES: Resultado de intentar reproducir un sonido */
UENUM(BlueprintType)
enum class EPGXSoundPlayResult : uint8
{
	Success               = 0  UMETA(DisplayName = "Success"),
	Failed_InvalidSound   = 1  UMETA(DisplayName = "Failed: Invalid Sound"),
	Failed_NotLoaded      = 2  UMETA(DisplayName = "Failed: Not Loaded"),
	Failed_Concurrency    = 3  UMETA(DisplayName = "Failed: Concurrency Limit"),
	Failed_Budget         = 4  UMETA(DisplayName = "Failed: Budget Exceeded"),
	Failed_BackendError   = 5  UMETA(DisplayName = "Failed: Backend Error")
};

// ── Sound Stop Reason ──

/** EN: Reason a sound was stopped / ES: Razon por la que se detuvo un sonido */
UENUM(BlueprintType)
enum class EPGXSoundStopReason : uint8
{
	Explicit        = 0  UMETA(DisplayName = "Explicit"),
	FadeComplete    = 1  UMETA(DisplayName = "Fade Complete"),
	Concurrency     = 2  UMETA(DisplayName = "Concurrency"),
	Budget          = 3  UMETA(DisplayName = "Budget"),
	PoolReclaim     = 4  UMETA(DisplayName = "Pool Reclaim"),
	SystemShutdown  = 5  UMETA(DisplayName = "System Shutdown")
};

// ── Sound Selection Mode ──

/** EN: How to pick a variant from multiple matches / ES: Como elegir variante de multiples coincidencias */
UENUM(BlueprintType)
enum class EPGXSoundSelectionMode : uint8
{
	Random      = 0  UMETA(DisplayName = "Random"),
	Sequential  = 1  UMETA(DisplayName = "Sequential"),
	Weighted    = 2  UMETA(DisplayName = "Weighted"),
	First       = 3  UMETA(DisplayName = "First Match")
};

// ── Concurrency Policy ──

/** EN: What to do when concurrency limit is reached / ES: Que hacer cuando se alcanza el limite de concurrencia */
UENUM(BlueprintType)
enum class EPGXAudioConcurrencyPolicy : uint8
{
	StopOldest          = 0  UMETA(DisplayName = "Stop Oldest"),
	RejectNew           = 1  UMETA(DisplayName = "Reject New"),
	StopLowestPriority  = 2  UMETA(DisplayName = "Stop Lowest Priority")
};

// ── Mix Layer ──

/** EN: 5-layer mix model priority order / ES: Modelo de mezcla de 5 capas en orden de prioridad */
UENUM(BlueprintType)
enum class EPGXMixLayer : uint8
{
	DefaultBase     = 0  UMETA(DisplayName = "Default Base"),
	Level           = 1  UMETA(DisplayName = "Level"),
	Ducking         = 2  UMETA(DisplayName = "Ducking"),
	LoadingScreen   = 3  UMETA(DisplayName = "Loading Screen"),
	User            = 4  UMETA(DisplayName = "User"),
	MAX             = 5  UMETA(Hidden)
};

inline constexpr int32 PGX_MIX_LAYER_COUNT = static_cast<int32>(EPGXMixLayer::MAX);

// ── Dialogue Interruption Policy ──

/** EN: How a dialogue request interacts with current playback / ES: Como interactua una peticion de dialogo con la reproduccion actual */
UENUM(BlueprintType)
enum class EPGXDialogueInterruptPolicy : uint8
{
	/** EN: Interrupt current dialogue immediately / ES: Interrumpir dialogo actual inmediatamente */
	CanInterrupt    = 0  UMETA(DisplayName = "Can Interrupt"),
	/** EN: Queue behind current dialogue / ES: Encolar detras del dialogo actual */
	QueueBehind     = 1  UMETA(DisplayName = "Queue Behind"),
	/** EN: Drop if something is already playing / ES: Descartar si algo ya esta reproduciendose */
	DropIfPlaying   = 2  UMETA(DisplayName = "Drop If Playing")
};

// ============================================================================
// Structs
// ============================================================================

// ── Sound Handle ──

/**
 * EN: Opaque handle to an active sound instance.
 *     Used to stop, query, or track individual sounds.
 * ES: Handle opaco a una instancia de sonido activa.
 *     Usado para detener, consultar o rastrear sonidos individuales.
 */
USTRUCT(BlueprintType)
struct PGXAUDIORUNTIME_API FPGXSoundHandle
{
	GENERATED_BODY()

	/** EN: Unique ID (0 = invalid) / ES: ID unico (0 = invalido) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio")
	int32 Id = 0;

	/** EN: Channel this sound belongs to / ES: Canal al que pertenece este sonido */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio")
	FGameplayTag ChannelTag;

	/** EN: Audio profile applied to this sound / ES: Perfil de audio aplicado a este sonido */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio")
	FGameplayTag ProfileTag;

	/** EN: World time when playback started / ES: Tiempo de mundo cuando empezo la reproduccion */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio")
	double StartTime = 0.0;

	/** EN: Check if this handle refers to a valid sound / ES: Comprobar si este handle refiere a un sonido valido */
	FORCEINLINE bool IsValid() const { return Id != 0; }

	/** EN: Factory: create invalid handle / ES: Fabrica: crear handle invalido */
	static FPGXSoundHandle Invalid() { return FPGXSoundHandle(); }

	bool operator==(const FPGXSoundHandle& Other) const { return Id == Other.Id; }
	bool operator!=(const FPGXSoundHandle& Other) const { return Id != Other.Id; }

	friend uint32 GetTypeHash(const FPGXSoundHandle& Handle) { return ::GetTypeHash(static_cast<uint32>(Handle.Id)); }
};

// ── Play Parameters ──

/**
 * EN: Parameters for a PlaySound request (per-call unique data).
 *     Reusable/configurable data goes in UPGXAudioProfile DataAsset instead.
 * ES: Parametros para una peticion PlaySound (datos unicos por llamada).
 *     Datos reutilizables/configurables van en el DataAsset UPGXAudioProfile.
 */
USTRUCT(BlueprintType)
struct PGXAUDIORUNTIME_API FPGXAudioPlayParams
{
	GENERATED_BODY()

	/** EN: Volume override (1.0 = use profile default) / ES: Override de volumen (1.0 = usar default del perfil) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Audio", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float VolumeMultiplier = 1.0f;

	/** EN: Pitch override (1.0 = use profile default) / ES: Override de pitch (1.0 = usar default del perfil) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Audio", meta = (ClampMin = "0.1", ClampMax = "4.0"))
	float PitchMultiplier = 1.0f;

	/** EN: Channel tag (required) / ES: Tag de canal (requerido) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Audio", meta = (Categories = "PGX.Audio.Channel"))
	FGameplayTag ChannelTag;

	/** EN: Audio profile tag (optional, default profile if empty) / ES: Tag de perfil de audio (opcional, perfil default si vacio) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Audio", meta = (Categories = "PGX.Audio.Profile"))
	FGameplayTag ProfileTag;

	/** EN: Context tags for sound variant resolution / ES: Tags de contexto para resolucion de variantes de sonido */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Audio")
	FGameplayTagContainer ContextTags;

	/** EN: Fade-in duration in seconds (0 = instant) / ES: Duracion del fade-in en segundos (0 = instantaneo) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Audio", meta = (ClampMin = "0.0"))
	float FadeInDuration = 0.0f;

	/** EN: Sound priority (higher = harder to reclaim) / ES: Prioridad del sonido (mayor = mas dificil de reclamar) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Audio", meta = (ClampMin = "0"))
	int32 Priority = 0;
};

// ── Sound Variant ──

/**
 * EN: A single variant within a UPGXSoundDefinition.
 *     Matches against context tags for resolution.
 * ES: Una variante individual dentro de un UPGXSoundDefinition.
 *     Se compara contra tags de contexto para resolucion.
 */
USTRUCT(BlueprintType)
struct PGXAUDIORUNTIME_API FPGXSoundVariant
{
	GENERATED_BODY()

	/** EN: Context tags this variant requires (all must match) / ES: Tags de contexto que esta variante requiere (todos deben coincidir) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio")
	FGameplayTagContainer ContextTags;

	/** EN: Sound assets for this variant (soft refs, async load) / ES: Assets de sonido para esta variante (refs suaves, carga async) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio")
	TArray<TSoftObjectPtr<USoundBase>> Sounds;

	/** EN: Weight for weighted selection (default 1.0) / ES: Peso para seleccion ponderada (default 1.0) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio", meta = (ClampMin = "0.01"))
	float Weight = 1.0f;

	/** EN: Volume multiplier for this variant / ES: Multiplicador de volumen para esta variante */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float VolumeMultiplier = 1.0f;

	/** EN: Pitch range min/max (random within range) / ES: Rango de pitch min/max (aleatorio dentro del rango) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio")
	FVector2D PitchRange = FVector2D(1.0, 1.0);
};

// ── Ducking Rule ──

/**
 * EN: A single ducking rule: when TriggerChannel plays, TargetChannel is attenuated.
 * ES: Una regla de ducking: cuando TriggerChannel reproduce, TargetChannel se atenua.
 */
USTRUCT(BlueprintType)
struct PGXAUDIORUNTIME_API FPGXDuckingRule
{
	GENERATED_BODY()

	/** EN: Name for debugging/inspector / ES: Nombre para debugging/inspector */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Ducking")
	FName RuleName;

	/** EN: Channel that triggers the duck / ES: Canal que activa el duck */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Ducking", meta = (Categories = "PGX.Audio.Channel"))
	FGameplayTag TriggerChannelTag;

	/** EN: Channel being ducked / ES: Canal que se atenua */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Ducking", meta = (Categories = "PGX.Audio.Channel"))
	FGameplayTag TargetChannelTag;

	/** EN: Target volume during duck (0.0 = mute, 1.0 = no duck) / ES: Volumen objetivo durante duck (0.0 = mute, 1.0 = sin duck) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Ducking", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DuckVolume = 0.3f;

	/** EN: Time to reach ducked volume (seconds) / ES: Tiempo para alcanzar volumen de duck (segundos) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Ducking", meta = (ClampMin = "0.0"))
	float AttackTime = 0.2f;

	/** EN: Time to restore original volume (seconds) / ES: Tiempo para restaurar volumen original (segundos) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Ducking", meta = (ClampMin = "0.0"))
	float ReleaseTime = 0.5f;

	/** EN: Volume threshold on trigger channel to activate (0 = always) / ES: Umbral de volumen en canal trigger para activar (0 = siempre) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Ducking", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TriggerThreshold = 0.0f;

	/** EN: Optional context tag for conditional ducking / ES: Tag de contexto opcional para ducking condicional */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Ducking")
	FGameplayTag ContextTag;

	/** EN: Priority for rule conflicts (higher wins) / ES: Prioridad para conflictos de reglas (mayor gana) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Ducking", meta = (ClampMin = "0"))
	int32 Priority = 0;
};

// ── Music Track ──

/**
 * EN: A single track entry in a music playlist.
 * ES: Una entrada de pista en un playlist de musica.
 */
USTRUCT(BlueprintType)
struct PGXAUDIORUNTIME_API FPGXMusicTrack
{
	GENERATED_BODY()

	/** EN: Music asset (soft ref, async load) / ES: Asset de musica (ref suave, carga async) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Music")
	TSoftObjectPtr<USoundBase> Music;

	/** EN: Crossfade duration when transitioning to this track / ES: Duracion del crossfade al transicionar a esta pista */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Music", meta = (ClampMin = "0.0"))
	float CrossfadeDuration = 2.0f;

	/** EN: Minimum play time before allowing transition / ES: Tiempo minimo de reproduccion antes de permitir transicion */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Music", meta = (ClampMin = "0.0"))
	float MinPlayDuration = 0.0f;

	/** EN: Whether this track can be interrupted before finishing / ES: Si esta pista puede ser interrumpida antes de terminar */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Music")
	bool bAllowInterrupt = true;
};

// ── Mix Layer State ──

/**
 * EN: Runtime state of one mix layer.
 * ES: Estado en tiempo de ejecucion de una capa de mezcla.
 */
USTRUCT(BlueprintType)
struct PGXAUDIORUNTIME_API FPGXMixLayerState
{
	GENERATED_BODY()

	/** EN: Which layer this represents / ES: Que capa representa */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Mix")
	EPGXMixLayer Layer = EPGXMixLayer::DefaultBase;

	/** EN: Whether this layer is currently active / ES: Si esta capa esta activa actualmente */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Mix")
	bool bActive = false;

	/** EN: Per-channel volume values in this layer / ES: Valores de volumen por canal en esta capa */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Mix")
	TMap<FGameplayTag, float> ChannelValues;

	/** EN: Source config name (for inspector/debug) / ES: Nombre de config fuente (para inspector/debug) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Mix")
	FString SourceConfigName;
};

// ── Ambient Zone Config ──

/**
 * EN: Configuration for a spatial ambient audio zone.
 * ES: Configuracion para una zona de audio ambiental espacial.
 */
USTRUCT(BlueprintType)
struct PGXAUDIORUNTIME_API FPGXAmbientZoneConfig
{
	GENERATED_BODY()

	/** EN: Tag identifying this zone / ES: Tag que identifica esta zona */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Zone")
	FGameplayTag ZoneTag;

	/** EN: Ambient sounds for this zone / ES: Sonidos ambientales para esta zona */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Zone")
	TArray<TSoftObjectPtr<USoundBase>> AmbientSounds;

	/** EN: Volume multiplier for zone sounds / ES: Multiplicador de volumen para sonidos de zona */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Zone", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float VolumeMultiplier = 1.0f;

	/** EN: Blend radius for transitioning into zone (cm) / ES: Radio de mezcla para transicionar a zona (cm) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Zone", meta = (ClampMin = "0.0"))
	float BlendRadius = 500.0f;

	/** EN: Reverb settings override for this zone / ES: Override de settings de reverb para esta zona */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Zone")
	TSoftObjectPtr<UReverbEffect> ReverbOverride;
};

// ============================================================================
// Query / Snapshot Structs (for Inspector + Console + API)
// ============================================================================

// ── Channel Snapshot ──

/** EN: Snapshot of a single audio channel state / ES: Snapshot del estado de un canal de audio */
USTRUCT(BlueprintType)
struct PGXAUDIORUNTIME_API FPGXAudioChannelSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	FGameplayTag ChannelTag;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	float Volume = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	bool bMuted = false;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	int32 ActiveSoundCount = 0;
};

// ── Active Sound Info ──

/** EN: Info about a currently playing sound / ES: Info sobre un sonido reproduciendose actualmente */
USTRUCT(BlueprintType)
struct PGXAUDIORUNTIME_API FPGXActiveSoundInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	FPGXSoundHandle Handle;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	FString SoundName;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	FGameplayTag ChannelTag;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	FGameplayTag ProfileTag;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	float Age = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	int32 Priority = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	bool bIs3D = false;
};

// ── Sound Pool Stats ──

/** EN: Statistics for the sound instance pool / ES: Estadisticas del pool de instancias de sonido */
USTRUCT(BlueprintType)
struct PGXAUDIORUNTIME_API FPGXSoundPoolStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	int32 TotalCapacity = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	int32 InUse = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	int32 Available = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	int32 PeakUsage = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	int32 MissCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	int32 GrowthEvents = 0;
};

// ── Audio Memory Info ──

/** EN: Memory usage breakdown for audio system / ES: Desglose de uso de memoria del sistema de audio */
USTRUCT(BlueprintType)
struct PGXAUDIORUNTIME_API FPGXAudioMemoryInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	int32 LoadedSoundCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	int64 EstimatedMemoryBytes = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	int32 PoolAllocated = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	int32 PoolInUse = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	int32 PeakConcurrent = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	TArray<FString> TopConsumers;
};

// ── Audio Event Record ──

/** EN: Record of an audio system event (for history log) / ES: Registro de un evento del sistema de audio (para log de historial) */
USTRUCT(BlueprintType)
struct PGXAUDIORUNTIME_API FPGXAudioEventRecord
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	double Timestamp = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	FGameplayTag EventTag;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	FString SoundName;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	FGameplayTag ChannelTag;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	FGameplayTag ProfileTag;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	bool bSuccess = true;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	float Duration = 0.0f;
};

// ── Full System Snapshot ──

/**
 * EN: Complete snapshot of the audio system state (for inspector + console).
 * ES: Snapshot completo del estado del sistema de audio (para inspector + consola).
 */
USTRUCT(BlueprintType)
struct PGXAUDIORUNTIME_API FPGXAudioSystemSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	EPGXAudioState State = EPGXAudioState::Uninitialized;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	EPGXAudioBackendType BackendType = EPGXAudioBackendType::Auto;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	TArray<FPGXAudioChannelSnapshot> Channels;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	TArray<FPGXActiveSoundInfo> ActiveSounds;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	EPGXMusicState MusicState = EPGXMusicState::Idle;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	FPGXSoundPoolStats PoolStats;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	FPGXAudioMemoryInfo MemoryInfo;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Audio|Query")
	TArray<FPGXMixLayerState> MixLayers;
};

// ============================================================================
// DataTable Row Structs — Config Resolution via Project Settings
// ============================================================================

class UPGXAudioChannelConfig;
class UPGXAudioProfile;

/**
 * EN: DataTable row for audio channel config resolution via Project Settings.
 *     Maps a channel GameplayTag to a UPGXAudioChannelConfig DA.
 * ES: Fila de DataTable para resolucion de config de canal de audio via Project Settings.
 *     Mapea un GameplayTag de canal a un DA UPGXAudioChannelConfig.
 */
USTRUCT(BlueprintType)
struct PGXAUDIORUNTIME_API FPGXAudioChannelRow : public FTableRowBase
{
	GENERATED_BODY()

	/** EN: Channel tag / ES: Tag de canal */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	FGameplayTag ChannelTag;

	/** EN: Audio channel config DA / ES: DA de config de canal de audio */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TSoftObjectPtr<UPGXAudioChannelConfig> ConfigRef;

	/** EN: Optional description / ES: Descripcion opcional */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	FText Description;
};

/**
 * EN: DataTable row for audio profile resolution via Project Settings.
 *     Maps a profile GameplayTag to a UPGXAudioProfile DA.
 * ES: Fila de DataTable para resolucion de perfil de audio via Project Settings.
 *     Mapea un GameplayTag de perfil a un DA UPGXAudioProfile.
 */
USTRUCT(BlueprintType)
struct PGXAUDIORUNTIME_API FPGXAudioProfileRow : public FTableRowBase
{
	GENERATED_BODY()

	/** EN: Profile tag / ES: Tag de perfil */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	FGameplayTag ProfileTag;

	/** EN: Audio profile DA / ES: DA de perfil de audio */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TSoftObjectPtr<UPGXAudioProfile> ProfileRef;

	/** EN: Optional description / ES: Descripcion opcional */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	FText Description;
};
