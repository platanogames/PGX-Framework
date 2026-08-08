// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "PGXPSOTypes.generated.h"

// ============================================================================
// EN: Enums
// ES: Enums
// ============================================================================

/**
 * EN: Vertex factory type for PSO precaching. Determines which VF the entry
 *     will use when calling UMaterialInterface::PrecachePSOs().
 * ES: Tipo de vertex factory para precaching de PSO. Determina que VF usara
 *     la entrada al llamar UMaterialInterface::PrecachePSOs().
 */
UENUM(BlueprintType)
enum class EPGXVertexFactoryType : uint8
{
	StaticMesh       UMETA(DisplayName = "Static Mesh (FLocalVertexFactory)"),
	SkeletalMesh     UMETA(DisplayName = "Skeletal Mesh (FGPUSkinVertexFactory)"),
	InstancedMesh    UMETA(DisplayName = "Instanced Static Mesh (FInstancedStaticMeshVertexFactory)"),
	SplineMesh       UMETA(DisplayName = "Spline Mesh (FSplineMeshVertexFactory)"),
	Landscape        UMETA(DisplayName = "Landscape (FLandscapeVertexFactory)"),
	NiagaraSprite    UMETA(DisplayName = "Niagara Sprite (FNiagaraSpriteVertexFactory)"),
	NiagaraMesh      UMETA(DisplayName = "Niagara Mesh (FNiagaraMeshVertexFactory)"),
	Custom           UMETA(DisplayName = "Custom (specify FName)")
};

/**
 * EN: When to trigger PSO warm-up for a config.
 * ES: Cuando activar el warm-up de PSO para una config.
 */
UENUM(BlueprintType)
enum class EPGXPSOActivationMode : uint8
{
	OnSubsystemInit   UMETA(DisplayName = "On Subsystem Init (game start)"),
	OnLevelLoad       UMETA(DisplayName = "On Level Load (streaming)"),
	OnGameFlowTag     UMETA(DisplayName = "On GameFlow Tag (e.g. Loading)"),
	OnExplicitCall    UMETA(DisplayName = "On Explicit Call (Blueprint/C++)")
};

/**
 * EN: Current state of the warm-up pipeline.
 * ES: Estado actual del pipeline de warm-up.
 */
UENUM(BlueprintType)
enum class EPGXPSOWarmUpState : uint8
{
	Idle,       // EN: No warm-up active / ES: Sin warm-up activo
	Loading,    // EN: Loading material assets / ES: Cargando material assets
	Compiling,  // EN: PSOs being compiled async / ES: PSOs compilandose async
	Complete,   // EN: All PSOs compiled / ES: Todos los PSOs compilados
	Failed,     // EN: Warm-up failed (partial or total) / ES: Warm-up fallido
	Paused      // EN: Paused by user/system / ES: Pausado por usuario/sistema
};

/**
 * EN: Hint for which mesh draw pass the PSO belongs to.
 * ES: Indicacion de a que mesh draw pass pertenece el PSO.
 */
UENUM(BlueprintType)
enum class EPGXPSOMeshPassHint : uint8
{
	Auto            UMETA(DisplayName = "Auto (from flags)"),
	BasePass        UMETA(DisplayName = "Base Pass"),
	ShadowDepth     UMETA(DisplayName = "Shadow Depth"),
	DepthOnly       UMETA(DisplayName = "Depth Only"),
	CustomDepth     UMETA(DisplayName = "Custom Depth"),
	Velocity        UMETA(DisplayName = "Velocity"),
	LightFunction   UMETA(DisplayName = "Light Function")
};

/**
 * EN: Type of PSO entry. Material entries use PrecachePSOs(); RawCache
 *     entries delegate to FShaderPipelineCache for compute/post-process.
 * ES: Tipo de entrada PSO. Entradas Material usan PrecachePSOs(); entradas
 *     RawCache delegan a FShaderPipelineCache para compute/post-process.
 */
UENUM(BlueprintType)
enum class EPGXPSOEntryType : uint8
{
	Material    UMETA(DisplayName = "Material + Vertex Factory (standard)"),
	RawCache    UMETA(DisplayName = "Raw PSO Cache (delegate to FShaderPipelineCache)")
};

/**
 * EN: When to persist the PSO cache to disk.
 * ES: Cuando persistir el cache de PSO a disco.
 */
UENUM(BlueprintType)
enum class EPGXPSOSavePolicy : uint8
{
	OnCompleteOnly     UMETA(DisplayName = "Save only when warm-up completes"),
	Periodic           UMETA(DisplayName = "Save periodically during warm-up"),
	OnLevelTransition  UMETA(DisplayName = "Save on level transition"),
	Manual             UMETA(DisplayName = "Manual only (via SaveCacheToDisk())")
};

/**
 * EN: How to handle a new warm-up request when one is already active.
 * ES: Como manejar una nueva solicitud de warm-up cuando ya hay una activa.
 */
UENUM(BlueprintType)
enum class EPGXPSOConcurrencyPolicy : uint8
{
	RejectNew         UMETA(DisplayName = "Reject new request (log warning)"),
	CancelAndRestart  UMETA(DisplayName = "Cancel current, start new"),
	MergeAndContinue  UMETA(DisplayName = "Merge new entries into active warm-up (Recommended)")
};

/**
 * EN: How the subsystem discovers UPGXPSOWarmUpConfig DataAssets at initialization.
 * ES: Como el subsistema descubre DataAssets UPGXPSOWarmUpConfig al inicializar.
 */
UENUM(BlueprintType)
enum class EPGXPSODiscoveryMode : uint8
{
	AssetRegistryScan  UMETA(DisplayName = "AssetRegistry Scan (default)"),
	PrimaryAssetType   UMETA(DisplayName = "Primary Asset Type (Asset Manager)")
};

// ============================================================================
// EN: Structs
// ES: Structs
// ============================================================================

/**
 * EN: Hashable deduplication key for PSO entries. Two entries with the same
 *     key will only be precached once.
 * ES: Clave hasheable de deduplicacion para entradas PSO. Dos entradas con
 *     la misma clave solo se precachean una vez.
 */
USTRUCT()
struct PGXPSORUNTIME_API FPGXPSOKey
{
	GENERATED_BODY()

	/** EN: Soft object path of the material / ES: Ruta del soft object del material */
	FName MaterialPath;

	/** EN: Vertex factory type name / ES: Nombre del tipo de vertex factory */
	FName VertexFactoryName;

	/** EN: Packed render parameters (shadow, depth, mobility, etc.) / ES: Parametros de render empaquetados */
	uint64 PackedParams = 0;

	/** EN: Mesh pass hint / ES: Indicacion de mesh pass */
	EPGXPSOMeshPassHint PassHint = EPGXPSOMeshPassHint::Auto;

	bool operator==(const FPGXPSOKey& Other) const
	{
		return MaterialPath == Other.MaterialPath
			&& VertexFactoryName == Other.VertexFactoryName
			&& PackedParams == Other.PackedParams
			&& PassHint == Other.PassHint;
	}

	friend uint32 GetTypeHash(const FPGXPSOKey& Key)
	{
		uint32 Hash = GetTypeHash(Key.MaterialPath);
		Hash = HashCombine(Hash, GetTypeHash(Key.VertexFactoryName));
		Hash = HashCombine(Hash, GetTypeHash(Key.PackedParams));
		Hash = HashCombine(Hash, GetTypeHash(static_cast<uint8>(Key.PassHint)));
		return Hash;
	}
};

/**
 * EN: A single PSO catalog entry. Describes one material + vertex factory
 *     combination to precache, with render parameters and context.
 * ES: Una entrada individual de catalogo PSO. Describe una combinacion
 *     material + vertex factory a precachear, con parametros de render y contexto.
 */
USTRUCT(BlueprintType)
struct PGXPSORUNTIME_API FPGXPSOEntry
{
	GENERATED_BODY()

	// ── What to precache / Que precachear ──

	/** EN: Material to precache PSOs for / ES: Material para el cual precachear PSOs */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|PSO")
	TSoftObjectPtr<UMaterialInterface> Material;

	/** EN: Vertex factory type / ES: Tipo de vertex factory */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|PSO")
	EPGXVertexFactoryType VertexFactory = EPGXVertexFactoryType::StaticMesh;

	/** EN: Custom VF name (only when VertexFactory == Custom) / ES: Nombre VF personalizado (solo cuando VertexFactory == Custom) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|PSO",
		meta = (EditCondition = "VertexFactory == EPGXVertexFactoryType::Custom"))
	FName CustomVertexFactoryName;

	// ── Context / Contexto ──

	/** EN: Context tag for filtering (e.g. PGX.PSO.Context.Global) / ES: Tag de contexto para filtrado */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|PSO", meta = (Categories = "PGX.PSO.Context"))
	FGameplayTag ContextTag;

	/** EN: Human-readable label for identification / ES: Etiqueta legible para identificacion */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|PSO")
	FString Label;

	// ── Render parameters / Parametros de render ──

	/** EN: Render in main (base) pass / ES: Renderizar en el pase principal (base) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|PSO|Render")
	bool bRenderInMainPass = true;

	/** EN: Render in depth pass / ES: Renderizar en el pase de profundidad */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|PSO|Render")
	bool bRenderInDepthPass = true;

	/** EN: Cast shadow / ES: Proyectar sombra */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|PSO|Render")
	bool bCastShadow = true;

	/** EN: Is a skinned mesh (skeletal) / ES: Es un mesh con skinning (skeletal) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|PSO|Render")
	bool bSkinnedMesh = false;

	/** EN: Component mobility type / ES: Tipo de movilidad del componente */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|PSO|Render")
	TEnumAsByte<EComponentMobility::Type> Mobility = EComponentMobility::Static;

	/** EN: Render in custom depth pass / ES: Renderizar en pase de profundidad personalizado */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|PSO|Render")
	bool bRenderCustomDepth = false;

	/** EN: Is a Nanite mesh (VF verification required) / ES: Es un mesh Nanite (requiere verificacion de VF) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|PSO|Render")
	bool bNaniteMesh = false;

	// ── Pass hint / Indicacion de pase ──

	/** EN: Which mesh draw pass this entry belongs to / ES: A que mesh draw pass pertenece esta entrada */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|PSO|Pass")
	EPGXPSOMeshPassHint PassHint = EPGXPSOMeshPassHint::Auto;

	// ── Entry type / Tipo de entrada ──

	/** EN: Material entry or raw cache entry / ES: Entrada de material o de cache raw */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|PSO")
	EPGXPSOEntryType EntryType = EPGXPSOEntryType::Material;

	// ── Memory lifecycle / Ciclo de vida de memoria ──

	/** EN: Keep material loaded after precache (prevents GC) / ES: Mantener material cargado tras precache (previene GC) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|PSO|Memory")
	bool bKeepLoadedAfterPrecache = false;

	// ── Helpers ──

	/** EN: Build a deduplication key from this entry / ES: Construir una clave de deduplicacion desde esta entrada */
	FPGXPSOKey MakeKey() const;

	/** EN: Resolve the vertex factory FName for PrecachePSOs() / ES: Resolver el FName de vertex factory para PrecachePSOs() */
	FName GetResolvedVertexFactoryName() const;
};

/**
 * EN: Entry recorded during an Active Audit session (editor-only).
 *     Captures runtime data about a material's PSO compilation.
 * ES: Entrada grabada durante una sesion de Active Audit (solo editor).
 *     Captura datos de runtime sobre la compilacion PSO de un material.
 */
USTRUCT(BlueprintType)
struct PGXPSORUNTIME_API FPGXPSORecordedEntry
{
	GENERATED_BODY()

	/** EN: Full asset path of the material / ES: Ruta completa del asset del material */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|PSO")
	FString MaterialPath;

	/** EN: Vertex factory type name used / ES: Nombre del tipo de vertex factory usado */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|PSO")
	FName VertexFactoryName;

	/** EN: Mesh pass name / ES: Nombre del mesh pass */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|PSO")
	FString MeshPassName;

	/** EN: When this entry was recorded / ES: Cuando se grabo esta entrada */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|PSO")
	FDateTime Timestamp;

	/** EN: Time spent compiling this PSO (ms) / ES: Tiempo de compilacion de este PSO (ms) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|PSO")
	float CompilationTimeMs = 0.0f;

	/** EN: Whether this PSO caused a frame hitch / ES: Si este PSO causo un hitch de frame */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|PSO")
	bool bCausedHitch = false;

	/** EN: Frame number when recorded / ES: Numero de frame cuando se grabo */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|PSO")
	int64 FrameNumber = 0;

	/** EN: Result string from PrecachePSOs() / ES: Cadena resultado de PrecachePSOs() */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|PSO")
	FString PrecacheResult;
};

/**
 * EN: Progress snapshot for the current warm-up operation.
 *     Broadcast via delegates and used by PSO Inspector.
 * ES: Instantanea de progreso para la operacion de warm-up actual.
 *     Emitida via delegados y usada por el PSO Inspector.
 */
USTRUCT(BlueprintType)
struct PGXPSORUNTIME_API FPGXPSOWarmUpProgress
{
	GENERATED_BODY()

	/** EN: Total entries to process / ES: Total de entradas a procesar */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|PSO")
	int32 TotalEntries = 0;

	/** EN: Entries completed successfully / ES: Entradas completadas exitosamente */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|PSO")
	int32 CompletedEntries = 0;

	/** EN: Entries that failed / ES: Entradas que fallaron */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|PSO")
	int32 FailedEntries = 0;

	/** EN: Completion percentage (0.0 - 1.0) / ES: Porcentaje de completitud (0.0 - 1.0) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|PSO")
	float PercentComplete = 0.0f;

	/** EN: Elapsed time since warm-up started (seconds) / ES: Tiempo transcurrido desde inicio de warm-up (segundos) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|PSO")
	float ElapsedTimeSeconds = 0.0f;

	/** EN: Current state of the pipeline / ES: Estado actual del pipeline */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|PSO")
	EPGXPSOWarmUpState State = EPGXPSOWarmUpState::Idle;

	// ── Metrics / Metricas ──

	/** EN: Total PrecachePSOs() requests submitted / ES: Total de solicitudes PrecachePSOs() enviadas */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|PSO")
	int32 TotalPrecacheRequests = 0;

	/** EN: Peak number of materials loaded simultaneously / ES: Pico de materiales cargados simultaneamente */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|PSO")
	int32 PeakLoadedMaterials = 0;

	/** EN: Number of entries skipped due to deduplication / ES: Entradas omitidas por deduplicacion */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|PSO")
	int32 DeduplicatedEntries = 0;
};

// ============================================================================
// EN: Dynamic Multicast Delegates (Blueprint-compatible, need .generated.h)
// ES: Delegados Dynamic Multicast (compatibles con Blueprint, necesitan .generated.h)
// ============================================================================

/** EN: Fired when warm-up begins / ES: Se dispara cuando comienza el warm-up */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPGXPSOWarmUpBegin);

/** EN: Fired periodically with progress data / ES: Se dispara periodicamente con datos de progreso */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnPGXPSOWarmUpProgress,
	int32, Completed,
	int32, Total,
	float, Percent);

/** EN: Fired when warm-up completes successfully / ES: Se dispara cuando el warm-up completa exitosamente */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPGXPSOWarmUpComplete);

/** EN: Fired when warm-up fails / ES: Se dispara cuando el warm-up falla */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnPGXPSOWarmUpFailed,
	const FString&, ErrorMessage);

/** EN: Fired when a single entry finishes compiling / ES: Se dispara cuando una entrada individual termina de compilar */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnPGXPSOEntryCompiled,
	const FPGXPSOEntry&, Entry);

/** EN: Fired when a new entry is recorded during Active Audit / ES: Se dispara cuando se graba una nueva entrada durante Active Audit */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnPGXPSORecordingUpdate,
	const FPGXPSORecordedEntry&, RecordedEntry);

// ============================================================================
// EN: Helper functions
// ES: Funciones auxiliares
// ============================================================================

namespace PGXPSOUtils
{
	/**
	 * EN: Resolve an EPGXVertexFactoryType enum to the UE vertex factory FName.
	 * ES: Resolver un enum EPGXVertexFactoryType al FName de vertex factory de UE.
	 */
	PGXPSORUNTIME_API FName ResolveVertexFactoryType(EPGXVertexFactoryType Type, FName CustomName = NAME_None);
}

// ============================================================================
// DataTable Row Structs — Config Resolution via Project Settings
// ============================================================================

class UPGXPSOWarmUpConfig;

/**
 * EN: DataTable row for PSO config resolution via Project Settings.
 *     Maps a config GameplayTag to a UPGXPSOWarmUpConfig DA.
 * ES: Fila de DataTable para resolucion de config PSO via Project Settings.
 *     Mapea un GameplayTag de config a un DA UPGXPSOWarmUpConfig.
 */
USTRUCT(BlueprintType)
struct PGXPSORUNTIME_API FPGXPSOConfigRow : public FTableRowBase
{
	GENERATED_BODY()

	/** EN: Config tag for this PSO warm-up config / ES: Tag de config para este config de warm-up PSO */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PSO")
	FGameplayTag ConfigTag;

	/** EN: PSO warm-up config DA / ES: DA de config de warm-up PSO */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PSO")
	TSoftObjectPtr<UPGXPSOWarmUpConfig> ConfigRef;

	/** EN: Optional description / ES: Descripcion opcional */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PSO")
	FText Description;
};
