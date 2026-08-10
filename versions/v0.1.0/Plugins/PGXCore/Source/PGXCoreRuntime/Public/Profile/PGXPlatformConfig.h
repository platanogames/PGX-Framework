// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Data/PGXConfigDataAsset.h"
#include "Profile/PGXProfileTypes.h"
#include "Profile/PGXPlatformBudgets.h"
#include "PGXPlatformConfig.generated.h"

// ============================================================================
// EN: Platform Config DataAsset — per-platform configuration.
//     One DA per target platform. Contains global budgets, feature matrix,
//     platform traits, and per-system budget overrides.
//     Referenced by UPGXProjectProfileConfig::PlatformConfigs map.
//
// ES: Config DataAsset de Plataforma — configuracion por plataforma.
//     Un DA por plataforma objetivo. Contiene presupuestos globales, matriz de features,
//     traits de plataforma, y overrides de presupuesto por sistema.
//     Referenciado por UPGXProjectProfileConfig::PlatformConfigs map.
// ============================================================================

UCLASS(BlueprintType, Blueprintable)
class PGXCORERUNTIME_API UPGXPlatformConfig : public UPGXConfigDataAsset
{
	GENERATED_BODY()

public:
	// ========================================================================
	// Identity
	// ========================================================================

	/** EN: Target platform this config applies to / ES: Plataforma objetivo a la que aplica esta config */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Platform|Identity")
	EPGXTargetPlatform TargetPlatform = EPGXTargetPlatform::PC;

	/** EN: Human-readable display name / ES: Nombre legible para mostrar */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Platform|Identity")
	FText DisplayName;

	// ========================================================================
	// Platform Traits
	// ========================================================================

	/** EN: Hardware/software characteristics of this platform / ES: Caracteristicas de hardware/software de esta plataforma */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Platform|Traits")
	FPGXPlatformTraits Traits;

	// ========================================================================
	// Global Budgets & Features (existing types from Profile v1.0)
	// ========================================================================

	/** EN: Global resource budgets (RAM, VRAM, Disk, etc.) / ES: Presupuestos globales de recursos (RAM, VRAM, Disco, etc.) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Platform|Global")
	FPGXProfileBudgets GlobalBudgets;

	/** EN: Feature availability matrix (Nanite, Lumen, etc.) / ES: Matriz de disponibilidad de features (Nanite, Lumen, etc.) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Platform|Global")
	FPGXProfileFeatureMatrix Features;

	// ========================================================================
	// Per-System Platform Budgets
	// ========================================================================

	/** EN: Audio system budgets / ES: Presupuestos del sistema de Audio */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Platform|Systems", meta = (AdvancedDisplay))
	FPGXAudioPlatformBudgets AudioBudgets;

	/** EN: Save system budgets / ES: Presupuestos del sistema de Save */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Platform|Systems", meta = (AdvancedDisplay))
	FPGXSavePlatformBudgets SaveBudgets;

	/** EN: PSO system budgets / ES: Presupuestos del sistema PSO */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Platform|Systems", meta = (AdvancedDisplay))
	FPGXPSOPlatformBudgets PSOBudgets;

	/** EN: Loading system budgets / ES: Presupuestos del sistema de Loading */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Platform|Systems", meta = (AdvancedDisplay))
	FPGXLoadingPlatformBudgets LoadingBudgets;

	/** EN: LevelFlow system budgets / ES: Presupuestos del sistema LevelFlow */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Platform|Systems", meta = (AdvancedDisplay))
	FPGXLevelFlowPlatformBudgets LevelFlowBudgets;

	/** EN: Log system budgets / ES: Presupuestos del sistema de Log */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Platform|Systems", meta = (AdvancedDisplay))
	FPGXLogPlatformBudgets LogBudgets;

	/** EN: GameFlow system budgets / ES: Presupuestos del sistema GameFlow */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Platform|Systems", meta = (AdvancedDisplay))
	FPGXGameFlowPlatformBudgets GameFlowBudgets;

	/** EN: MGOS (GC Observer) system budgets / ES: Presupuestos del sistema MGOS (Observador GC) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Platform|Systems", meta = (AdvancedDisplay))
	FPGXMGOSPlatformBudgets MGOSBudgets;

	/** EN: Message system budgets / ES: Presupuestos del sistema de Message */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Platform|Systems", meta = (AdvancedDisplay))
	FPGXMessagePlatformBudgets MessageBudgets;

	/** EN: EventHandler system budgets / ES: Presupuestos del sistema EventHandler */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Platform|Systems", meta = (AdvancedDisplay))
	FPGXEventHandlerPlatformBudgets EventHandlerBudgets;

	// ========================================================================
	// Threshold Settings
	// ========================================================================

	/** EN: Percentage at which budget warnings trigger (0-100) / ES: Porcentaje en el que se activan warnings de presupuesto (0-100) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Platform|Thresholds", meta = (AdvancedDisplay, ClampMin = "0", ClampMax = "100"))
	int32 WarningThresholdPercent = 75;

	/** EN: Percentage at which budget errors trigger (0-100) / ES: Porcentaje en el que se activan errores de presupuesto (0-100) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Platform|Thresholds", meta = (AdvancedDisplay, ClampMin = "0", ClampMax = "100"))
	int32 ErrorThresholdPercent = 90;
};
