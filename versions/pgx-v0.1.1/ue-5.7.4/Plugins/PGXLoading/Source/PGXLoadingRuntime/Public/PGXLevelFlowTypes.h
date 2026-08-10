// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "PGXLevelFlowTypes.generated.h"

// ============================================================================
// EN: Types for PGX LevelFlow System
//     Data-driven level transition management with deterministic timing.
//
// ES: Tipos para el Sistema LevelFlow de PGX
//     Gestion de transiciones de nivel data-driven con temporalizacion determinista.
// ============================================================================

// ── Enums ───────────────────────────────────────────────────────────────────

/**
 * EN: Current state of the level transition pipeline.
 * ES: Estado actual del pipeline de transicion de nivel.
 */
UENUM(BlueprintType)
enum class EPGXLevelFlowState : uint8
{
	/** EN: No transition active / ES: Sin transicion activa */
	Idle,
	/** EN: Resolving tag, validating, broadcasting start / ES: Resolviendo tag, validando, broadcasting inicio */
	Preparing,
	/** EN: Async asset load in progress / ES: Carga async del asset en progreso */
	Loading,
	/** EN: OpenLevel called, waiting for new level / ES: OpenLevel llamado, esperando nuevo nivel */
	Transitioning,
	/** EN: New level loaded, waiting for readiness conditions / ES: Nuevo nivel cargado, esperando condiciones de readiness */
	PostLoad,
	/** EN: Transition finished successfully / ES: Transicion terminada exitosamente */
	Complete,
	/** EN: Transition failed / ES: Transicion fallida */
	Failed
};

/**
 * EN: How to load the level asset.
 * ES: Como cargar el asset del nivel.
 */
UENUM(BlueprintType)
enum class EPGXLoadStrategy : uint8
{
	/** EN: Async load + OpenLevel (standard) / ES: Carga async + OpenLevel (estandar) */
	AsyncLoad,
	/** EN: Synchronous load (blocking) / ES: Carga sincrona (bloqueante) */
	SyncLoad,
	/** EN: ULevelStreaming — no persistent level change / ES: ULevelStreaming — sin cambio de nivel persistente */
	StreamSubLevel
};

/**
 * EN: Visual transition between levels.
 * ES: Transicion visual entre niveles.
 */
UENUM(BlueprintType)
enum class EPGXTransitionMode : uint8
{
	/** EN: No visual transition — cut / ES: Sin transicion visual — corte */
	Instant,
	/** EN: Fade to black, load, fade from black / ES: Fade a negro, carga, fade desde negro */
	FadeOut_FadeIn,
	/** EN: Project handles transition via delegates / ES: El proyecto maneja la transicion via delegates */
	Custom
};

/**
 * EN: Result code for transition requests.
 * ES: Codigo de resultado para solicitudes de transicion.
 */
UENUM(BlueprintType)
enum class EPGXLevelFlowResultCode : uint8
{
	Success,
	TagNotFound,
	AlreadyInLevel,
	TransitionInProgress,
	LoadFailed,
	Cancelled,
	Timeout
};

// ── Structs ─────────────────────────────────────────────────────────────────

/**
 * EN: Deterministic transition timing. Replaces fixed "LazyTime" with
 *     condition-based readiness + timeout safety net.
 *     MinTime ensures visual smoothness. MaxTime ensures we always proceed.
 *     bWaitForShaderCompilation checks UE native API (no PGXPSO dependency).
 *
 * ES: Temporalizacion determinista de transiciones. Reemplaza "LazyTime" fijo
 *     con readiness basado en condicion + timeout de seguridad.
 */
USTRUCT(BlueprintType)
struct PGXLOADINGRUNTIME_API FPGXTransitionTiming
{
	GENERATED_BODY()

	/** EN: Always wait at least this (visual smoothness for fades) / ES: Siempre esperar al menos esto (suavidad visual) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|LevelFlow|Timing", meta = (ClampMin = "0.0"))
	float MinTime = 0.5f;

	/** EN: Never wait more than this (safety net) / ES: Nunca esperar mas que esto (red de seguridad) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|LevelFlow|Timing", meta = (ClampMin = "0.1"))
	float MaxTime = 10.0f;

	/** EN: Hold until FShaderPipelineCache::NumPrecompilesRemaining() == 0 / ES: Mantener hasta que NumPrecompilesRemaining() == 0 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|LevelFlow|Timing")
	bool bWaitForShaderCompilation = true;
};

/**
 * EN: A sub-level entry within a level profile.
 * ES: Una entrada de sub-nivel dentro de un perfil de nivel.
 */
USTRUCT(BlueprintType)
struct PGXLOADINGRUNTIME_API FPGXSubLevelEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|LevelFlow|SubLevel")
	FGameplayTag SubLevelTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|LevelFlow|SubLevel")
	TSoftObjectPtr<UWorld> SubLevelReference;

	/** EN: Auto-load when parent level loads / ES: Auto-cargar cuando carga el nivel padre */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|LevelFlow|SubLevel")
	bool bAutoLoadOnEntry = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|LevelFlow|SubLevel", meta = (ClampMin = "0"))
	int32 LoadPriority = 0;
};

/**
 * EN: Full description of a loadable level. The core data unit stored in UPGXLevelProfile.
 * ES: Descripcion completa de un nivel cargable. La unidad de datos core almacenada en UPGXLevelProfile.
 */
USTRUCT(BlueprintType)
struct PGXLOADINGRUNTIME_API FPGXLevelEntry
{
	GENERATED_BODY()

	// ── Identity / Identidad ──

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|LevelFlow|Level")
	FGameplayTag LevelTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|LevelFlow|Level")
	FText DisplayName;

	// ── Asset Reference / Referencia al Asset ──

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|LevelFlow|Level")
	TSoftObjectPtr<UWorld> LevelReference;

	/** EN: Path string for OpenLevel (auto-derived from LevelReference if empty) / ES: Path string para OpenLevel (auto-derivado de LevelReference si vacio) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|LevelFlow|Level")
	FString LevelPath;

	// ── Load Behavior / Comportamiento de Carga ──

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|LevelFlow|Loading")
	EPGXLoadStrategy LoadStrategy = EPGXLoadStrategy::AsyncLoad;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|LevelFlow|Loading")
	EPGXTransitionMode TransitionMode = EPGXTransitionMode::FadeOut_FadeIn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|LevelFlow|Loading")
	FPGXTransitionTiming Timing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|LevelFlow|Loading")
	bool bShowLoadingScreen = true;

	// ── GameFlow Integration / Integracion GameFlow ──

	/** EN: GameFlow tag to set on Global channel when entering this level (empty = no change) / ES: Tag de GameFlow a setear en canal Global al entrar a este nivel (vacio = sin cambio) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|LevelFlow|GameFlow",
		meta = (Categories = "PGX.GameFlow.Phase"))
	FGameplayTag GameFlowTagOnEnter;

	// ── Sub-Levels / Sub-Niveles ──

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|LevelFlow|SubLevels")
	TArray<FPGXSubLevelEntry> SubLevels;

	// ── Custom / Personalizado ──

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|LevelFlow|Custom")
	TMap<FName, FString> CustomParams;
};

/**
 * EN: Result returned by transition requests.
 * ES: Resultado retornado por solicitudes de transicion.
 */
USTRUCT(BlueprintType)
struct PGXLOADINGRUNTIME_API FPGXLevelFlowResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|LevelFlow")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|LevelFlow")
	FString Description;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|LevelFlow")
	EPGXLevelFlowResultCode Code = EPGXLevelFlowResultCode::Success;

	static FPGXLevelFlowResult MakeSuccess(const FString& Desc = TEXT(""))
	{
		FPGXLevelFlowResult R;
		R.bSuccess = true;
		R.Code = EPGXLevelFlowResultCode::Success;
		R.Description = Desc;
		return R;
	}

	static FPGXLevelFlowResult MakeFail(EPGXLevelFlowResultCode InCode, const FString& Desc)
	{
		FPGXLevelFlowResult R;
		R.bSuccess = false;
		R.Code = InCode;
		R.Description = Desc;
		return R;
	}
};

/**
 * EN: Record of a level transition for history and diagnostics.
 * ES: Registro de una transicion de nivel para historial y diagnosticos.
 */
USTRUCT(BlueprintType)
struct PGXLOADINGRUNTIME_API FPGXLevelTransitionRecord
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|LevelFlow")
	FGameplayTag FromLevelTag;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|LevelFlow")
	FGameplayTag ToLevelTag;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|LevelFlow")
	FDateTime Timestamp;

	/** EN: Time spent in async loading phase (seconds) / ES: Tiempo en fase de carga async (segundos) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|LevelFlow")
	float LoadDurationSeconds = 0.0f;

	/** EN: Time spent in PostLoad timing wait (seconds) / ES: Tiempo en espera de timing PostLoad (segundos) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|LevelFlow")
	float WaitDurationSeconds = 0.0f;

	/** EN: Shader compilations pending when level loaded / ES: Compilaciones de shader pendientes al cargar el nivel */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|LevelFlow")
	int32 ShaderCompilationsOnEntry = 0;

	/** EN: Whether MaxTime was reached before conditions were met / ES: Si MaxTime fue alcanzado antes de cumplir condiciones */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|LevelFlow")
	bool bTimedOut = false;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|LevelFlow")
	EPGXLevelFlowResultCode ResultCode = EPGXLevelFlowResultCode::Success;
};

// ============================================================================
// DataTable Row Structs — Config Resolution via Project Settings
// ============================================================================

class UPGXLevelProfile;

/**
 * EN: DataTable row for level catalog resolution via Project Settings.
 *     Maps a catalog GameplayTag to a UPGXLevelProfile DA.
 * ES: Fila de DataTable para resolucion de catalogo de niveles via Project Settings.
 *     Mapea un GameplayTag de catalogo a un DA UPGXLevelProfile.
 */
USTRUCT(BlueprintType)
struct PGXLOADINGRUNTIME_API FPGXLevelCatalogRow : public FTableRowBase
{
	GENERATED_BODY()

	/** EN: Catalog tag for this level profile / ES: Tag de catalogo para este profile de niveles */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LevelFlow")
	FGameplayTag CatalogTag;

	/** EN: Level profile DA / ES: DA de profile de niveles */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LevelFlow")
	TSoftObjectPtr<UPGXLevelProfile> ProfileRef;

	/** EN: Optional description / ES: Descripcion opcional */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LevelFlow")
	FText Description;
};
