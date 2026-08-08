// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "PGXLoadingTypes.generated.h"

class UPGXLoadingProfile;

// ============================================================================
// EN: Enums
// ES: Enums
// ============================================================================

/**
 * EN: Loading screen visual state machine.
 *     6 states covering the full lifecycle of a loading screen.
 * ES: Maquina de estados visual de la pantalla de carga.
 *     6 estados cubriendo el ciclo de vida completo de una pantalla de carga.
 */
UENUM(BlueprintType)
enum class EPGXLoadingScreenState : uint8
{
	Idle,           // EN: No loading screen active / ES: Sin pantalla de carga activa
	Preparing,      // EN: Profile resolved, assets loading / ES: Perfil resuelto, assets cargando
	FadingIn,       // EN: Overlay appearing (fade from gameplay) / ES: Overlay apareciendo
	Active,         // EN: Overlay fully visible / ES: Overlay completamente visible
	WaitingClose,   // EN: Evaluating close conditions / ES: Evaluando condiciones de cierre
	FadingOut       // EN: Overlay disappearing (fade to gameplay) / ES: Overlay desapareciendo
};

/**
 * EN: Visual type determines which Strategy handles the loading screen.
 * ES: El tipo visual determina que Strategy maneja la pantalla de carga.
 */
UENUM(BlueprintType)
enum class EPGXLoadingVisualType : uint8
{
	Minimal,            // EN: Black screen + spinner (always available fallback)
	StaticImage,        // EN: Single background image
	Slideshow,          // EN: Rotating background images with crossfade
	MaterialAnimated,   // EN: Animated material full-screen
	Video,              // EN: Media player video
	Custom              // EN: User-defined strategy subclass
};

/**
 * EN: Close condition policy — when can the loading screen close?
 * ES: Politica de condicion de cierre — cuando puede cerrarse la pantalla?
 */
UENUM(BlueprintType)
enum class EPGXLoadingClosePolicy : uint8
{
	Automatic,      // EN: Close when all conditions met / ES: Cerrar cuando se cumplan condiciones
	ManualOnly,     // EN: Only close via ForceClose() / ES: Solo cerrar via ForceClose()
	AutoWithSkip    // EN: Auto close + user can skip (if conditions allow)
};

/**
 * EN: Reentry policy — what happens if RequestLoading while already active.
 * ES: Politica de reentrada — que pasa si se pide loading estando activo.
 */
UENUM(BlueprintType)
enum class EPGXLoadingReentryPolicy : uint8
{
	Ignore,         // EN: Reject new request / ES: Rechazar nueva peticion
	Restart,        // EN: Cancel current, start new / ES: Cancelar actual, iniciar nuevo
	Queue           // EN: Queue for after current completes / ES: Encolar para despues
};

/**
 * EN: Result code for loading operations.
 * ES: Codigo de resultado para operaciones de carga.
 */
UENUM(BlueprintType)
enum class EPGXLoadingResultCode : uint8
{
	Success,                // EN: Operation completed successfully
	ProfileNotFound,        // EN: No profile for given context tag
	InvalidProfile,         // EN: Profile failed validation
	AlreadyActive,          // EN: Loading screen already active (and policy = Ignore)
	ForceClosed,            // EN: Closed via ForceClose()
	TimedOut,               // EN: Watchdog timeout triggered
	AssetLoadFailed,        // EN: Required assets failed to load
	Cancelled,              // EN: Cancelled by new request (policy = Restart)
	Unsupported             // EN: Feature is declared but not yet implemented. AsyncLoader, StreamingManager, and Queue reentry use this explicit capability result. ES: Funcionalidad declarada pero no implementada (API placeholder).
};

// ============================================================================
// EN: Structs
// ES: Structs
// ============================================================================

/**
 * EN: Fade configuration for transitions.
 * ES: Configuracion de fade para transiciones.
 */
USTRUCT(BlueprintType)
struct PGXLOADINGRUNTIME_API FPGXFadeConfig
{
	GENERATED_BODY()

	/** EN: Fade in duration (gameplay to loading) / ES: Duracion fade in */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|Fade",
		meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float FadeInDuration = 0.3f;

	/** EN: Fade out duration (loading to gameplay) / ES: Duracion fade out */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|Fade",
		meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float FadeOutDuration = 0.5f;

	/** EN: Use curve asset for fade animation / ES: Usar curva para animacion */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|Fade")
	TSoftObjectPtr<UCurveFloat> FadeCurve;
};

/**
 * EN: Context mapping entry — maps a GameplayTag to a Loading Profile.
 * ES: Entrada de mapeo de contexto — mapea un GameplayTag a un Loading Profile.
 */
USTRUCT(BlueprintType)
struct PGXLOADINGRUNTIME_API FPGXLoadingContextEntry
{
	GENERATED_BODY()

	/** EN: Context tag that triggers this profile / ES: Tag de contexto que activa este perfil */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading", meta = (Categories = "PGX.Loading.Context"))
	FGameplayTag ContextTag;

	/** EN: Loading profile to use (soft reference) / ES: Perfil de carga a usar (referencia soft) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading")
	TSoftObjectPtr<UPGXLoadingProfile> Profile;
};

/**
 * EN: Progress information broadcast to widgets and listeners.
 * ES: Informacion de progreso enviada a widgets y listeners.
 */
USTRUCT(BlueprintType)
struct PGXLOADINGRUNTIME_API FPGXLoadingProgress
{
	GENERATED_BODY()

	/** EN: Overall progress [0.0 - 1.0] / ES: Progreso general [0.0 - 1.0] */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Loading")
	float TotalProgress = 0.0f;

	/** EN: Asset loading progress / ES: Progreso de carga de assets */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Loading")
	float AssetProgress = 0.0f;

	/** EN: PSO warm-up progress (if applicable) / ES: Progreso PSO (si aplica) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Loading")
	float PSOProgress = 0.0f;

	/** EN: Status message for display / ES: Mensaje de estado para mostrar */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Loading")
	FText StatusMessage;

	/** EN: Time elapsed since loading started / ES: Tiempo transcurrido */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Loading")
	float ElapsedTime = 0.0f;
};

/**
 * EN: Input state snapshot for restore after loading.
 * ES: Snapshot del estado de input para restaurar despues de carga.
 */
USTRUCT()
struct PGXLOADINGRUNTIME_API FPGXInputStateSnapshot
{
	GENERATED_BODY()

	bool bCursorVisible = false;
	bool bCursorLocked = false;
	// EN: Input mode cannot be queried directly in UE, so we track it manually
	// ES: El InputMode no se puede consultar directamente en UE, lo rastreamos manualmente
	bool bWasGameOnly = true;
	bool bWasUIOnly = false;
	bool bWasGameAndUI = false;
};

/**
 * EN: Record of a completed loading screen event (for history/metrics).
 * ES: Registro de un evento de pantalla de carga completado (historial/metricas).
 */
USTRUCT(BlueprintType)
struct PGXLOADINGRUNTIME_API FPGXLoadingRecord
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Loading")
	FGameplayTag ContextTag;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Loading")
	EPGXLoadingVisualType VisualType = EPGXLoadingVisualType::Minimal;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Loading")
	FDateTime Timestamp;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Loading")
	float TotalDuration = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Loading")
	float PreparingDuration = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Loading")
	float ActiveDuration = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Loading")
	float WaitingDuration = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Loading")
	float FadeDuration = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Loading")
	EPGXLoadingResultCode ResultCode = EPGXLoadingResultCode::Success;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Loading")
	bool bPSOWaited = false;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Loading")
	bool bTimedOut = false;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Loading")
	bool bUserSkipped = false;
};

/**
 * EN: Operation result (same pattern as FPGXLevelFlowResult).
 * ES: Resultado de operacion (mismo patron que FPGXLevelFlowResult).
 */
USTRUCT(BlueprintType)
struct PGXLOADINGRUNTIME_API FPGXLoadingResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Loading")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Loading")
	FString Description;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Loading")
	EPGXLoadingResultCode Code = EPGXLoadingResultCode::Success;

	static FPGXLoadingResult MakeSuccess(const FString& Desc = TEXT(""))
	{
		FPGXLoadingResult R;
		R.bSuccess = true;
		R.Description = Desc;
		R.Code = EPGXLoadingResultCode::Success;
		return R;
	}

	static FPGXLoadingResult MakeFail(EPGXLoadingResultCode InCode, const FString& Desc)
	{
		FPGXLoadingResult R;
		R.bSuccess = false;
		R.Description = Desc;
		R.Code = InCode;
		return R;
	}
};

// ============================================================================
// EN: Dynamic Multicast Delegates (Blueprint-compatible)
// ES: Delegados Dynamic Multicast (compatibles con Blueprint)
// ============================================================================

/** EN: Fired when a loading screen is activated / ES: Disparado cuando se activa una pantalla de carga */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPGXLoadingStarted, FGameplayTag, ContextTag);

/** EN: Fired periodically with progress data / ES: Disparado periodicamente con datos de progreso */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPGXLoadingProgressUpdated, float, Progress, FText, StatusMessage);

/** EN: Fired when a loading screen fully closes / ES: Disparado cuando la pantalla de carga se cierra completamente */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPGXLoadingCompleted, FPGXLoadingRecord, Record);

/** EN: Fired on error or forced close / ES: Disparado en error o cierre forzado */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPGXLoadingFailed, EPGXLoadingResultCode, Code, FString, Reason);

/** EN: Fired on every state machine transition / ES: Disparado en cada transicion de la maquina de estados */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPGXLoadingStateChanged, EPGXLoadingScreenState, NewState, EPGXLoadingScreenState, OldState);

// ============================================================================
// DataTable Row Structs — Config Resolution via Project Settings
// ============================================================================

/**
 * EN: DataTable row for loading profile resolution via Project Settings.
 *     Maps a context GameplayTag to a UPGXLoadingProfile DA.
 * ES: Fila de DataTable para resolucion de profile de loading via Project Settings.
 *     Mapea un GameplayTag de contexto a un DA UPGXLoadingProfile.
 */
USTRUCT(BlueprintType)
struct PGXLOADINGRUNTIME_API FPGXLoadingProfileRow : public FTableRowBase
{
	GENERATED_BODY()

	/** EN: Context tag for this loading profile / ES: Tag de contexto para este profile de loading */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loading")
	FGameplayTag ContextTag;

	/** EN: Loading profile DA / ES: DA de profile de loading */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loading")
	TSoftObjectPtr<UPGXLoadingProfile> ProfileRef;

	/** EN: Optional description / ES: Descripcion opcional */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loading")
	FText Description;
};
