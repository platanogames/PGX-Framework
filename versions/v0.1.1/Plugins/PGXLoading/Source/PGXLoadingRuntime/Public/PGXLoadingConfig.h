// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Data/PGXConfigDataAsset.h"
#include "Trace/PGXTraceTypes.h"
#include "PGXLoadingTypes.h"
#include "PGXLoadingConfig.generated.h"

/**
 * EN: Global configuration for the Loading Screen system.
 *     One per project — auto-discovered via AssetRegistry.
 *     Defines default behavior, viewport settings, safety timeouts, and integration toggles.
 *
 * ES: Configuracion global para el sistema de Pantalla de Carga.
 *     Uno por proyecto — auto-descubierto via AssetRegistry.
 *     Define comportamiento por defecto, ajustes de viewport, timeouts de seguridad, y toggles de integracion.
 */
UCLASS(BlueprintType)
class PGXLOADINGRUNTIME_API UPGXLoadingConfig : public UPGXConfigDataAsset
{
	GENERATED_BODY()

public:
	// ======================================================================
	// EN: Default Behavior / ES: Comportamiento por defecto
	// ======================================================================

	/** EN: Default close policy / ES: Politica de cierre por defecto */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|Defaults")
	EPGXLoadingClosePolicy DefaultClosePolicy = EPGXLoadingClosePolicy::Automatic;

	/** EN: Default reentry policy / ES: Politica de reentrada por defecto */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|Defaults", meta = (AdvancedDisplay))
	EPGXLoadingReentryPolicy ReentryPolicy = EPGXLoadingReentryPolicy::Restart;

	/** EN: Default fade config / ES: Configuracion de fade por defecto */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|Defaults")
	FPGXFadeConfig DefaultFadeConfig;

	/** EN: Min display time (prevents flash on fast SSDs) / ES: Tiempo minimo de muestra */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|Defaults",
		meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float DefaultMinDisplayTime = 1.0f;

	// ======================================================================
	// EN: Viewport / ES: Viewport
	// ======================================================================

	/** EN: ZOrder for loading overlay (above normal UI) / ES: ZOrder del overlay */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|Viewport",
		meta = (AdvancedDisplay, ClampMin = "100", ClampMax = "10000"))
	int32 OverlayZOrder = 1000;

	/** EN: ZOrder reserved for critical errors (above loading) / ES: ZOrder para errores criticos */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|Viewport",
		meta = (AdvancedDisplay, ClampMin = "1001", ClampMax = "20000"))
	int32 CriticalErrorZOrder = 5000;

	// ======================================================================
	// EN: Safety / ES: Seguridad
	// ======================================================================

	/** EN: Max time in Preparing state before fallback / ES: Tiempo max en Preparing */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|Safety",
		meta = (AdvancedDisplay, ClampMin = "1.0", ClampMax = "30.0"))
	float PreparingTimeout = 5.0f;

	/** EN: Max time in WaitingClose (watchdog) / ES: Tiempo max en WaitingClose */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|Safety",
		meta = (AdvancedDisplay, ClampMin = "5.0", ClampMax = "120.0"))
	float WaitingCloseTimeout = 20.0f;

	/** EN: Frames to wait after PostLoadMap before allowing close / ES: Frames post-load */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|Safety",
		meta = (AdvancedDisplay, ClampMin = "0", ClampMax = "10"))
	int32 PostLoadFrameDelay = 2;

	// ======================================================================
	// EN: PSO Integration / ES: Integracion PSO
	// ======================================================================

	/** EN: Wait for PSO warm-up before close (global default) / ES: Esperar PSO */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|PSO")
	bool bWaitForPSOByDefault = true;

	/** EN: Max time to wait for PSO before forced close / ES: Timeout PSO */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|PSO",
		meta = (AdvancedDisplay, ClampMin = "1.0", ClampMax = "60.0", EditCondition = "bWaitForPSOByDefault"))
	float PSOWaitTimeout = 15.0f;

	/** EN: Weight of PSO progress in combined progress bar / ES: Peso PSO en barra combinada */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|PSO",
		meta = (AdvancedDisplay, ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bWaitForPSOByDefault"))
	float PSOProgressWeight = 0.3f;

	// ======================================================================
	// EN: LevelFlow Integration / ES: Integracion LevelFlow
	// ======================================================================

	/** EN: Auto-activate loading screen on LevelFlow transitions / ES: Auto-activar en transiciones */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|LevelFlow", meta = (AdvancedDisplay))
	bool bAutoActivateOnLevelFlow = true;

	/** EN: Default context tag for LevelFlow-triggered loading / ES: Tag por defecto para LevelFlow */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|LevelFlow",
		meta = (AdvancedDisplay, Categories = "PGX.Loading.Context", EditCondition = "bAutoActivateOnLevelFlow"))
	FGameplayTag LevelFlowDefaultContext;

	// ======================================================================
	// EN: History / ES: Historial
	// ======================================================================

	/** EN: Max loading records to keep / ES: Maximo de registros a guardar */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|History",
		meta = (AdvancedDisplay, ClampMin = "10", ClampMax = "200"))
	int32 MaxHistoryDepth = 50;

	// ======================================================================
	// EN: Traceability (Infrastructure v0.4.0) / ES: Trazabilidad
	// ======================================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|Trace", meta = (AdvancedDisplay))
	FPGXTraceConfig TraceConfig;
};
