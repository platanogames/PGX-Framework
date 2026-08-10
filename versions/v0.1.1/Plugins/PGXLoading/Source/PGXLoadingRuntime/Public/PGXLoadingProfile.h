// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Data/PGXConfigDataAsset.h"
#include "Data/PGXContractTypes.h"
#include "GameplayTagContainer.h"
#include "PGXLoadingTypes.h"
#include "PGXLoadingProfile.generated.h"

class UPGXLoadingStrategyBase;
class UPGXLoadingWidget;
class UTexture2D;
class UMaterialInterface;
class UMediaSource;
class UCurveFloat;

/**
 * EN: Per-context visual profile DataAsset for the Loading Screen system.
 *     Each profile defines the visual behavior for one or more loading contexts.
 *     Multiple profiles can coexist (e.g., DA_Loading_LevelTransition, DA_Loading_NetworkWait).
 *     All are auto-discovered via AssetRegistry scan and merged into the context map.
 *     Same discovery pattern as PGXSave, PGXGameFlow, PGXPSO, and PGXLevelFlow.
 *
 * ES: DataAsset de perfil visual por-contexto para el sistema de Pantalla de Carga.
 *     Cada perfil define el comportamiento visual para uno o mas contextos de carga.
 *     Multiples perfiles pueden coexistir (e.g., DA_Loading_LevelTransition, DA_Loading_NetworkWait).
 *     Todos se auto-descubren via AssetRegistry scan y se fusionan en el mapa de contextos.
 *     Mismo patron de descubrimiento que PGXSave, PGXGameFlow, PGXPSO, y PGXLevelFlow.
 */
UCLASS(BlueprintType)
class PGXLOADINGRUNTIME_API UPGXLoadingProfile : public UPGXConfigDataAsset
{
	GENERATED_BODY()

public:
	UPGXLoadingProfile();

	// ======================================================================
	// EN: Context Mapping / ES: Mapeo de contextos
	// ======================================================================

	/**
	 * EN: Context tags this profile handles. When RequestLoading(Tag) is called,
	 *     the subsystem looks up this tag in the merged context map.
	 *     A profile can handle multiple context tags.
	 * ES: Tags de contexto que este perfil maneja. Cuando se llama RequestLoading(Tag),
	 *     el subsistema busca este tag en el mapa de contextos fusionado.
	 *     Un perfil puede manejar multiples tags de contexto.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|Context")
	TArray<FGameplayTag> ContextTags;

	// ======================================================================
	// EN: Visual Type / ES: Tipo visual
	// ======================================================================

	/** EN: Visual strategy type for this profile / ES: Tipo de estrategia visual */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|Visual")
	EPGXLoadingVisualType DefaultVisualType = EPGXLoadingVisualType::StaticImage;

	// ======================================================================
	// EN: Assets (all soft references — lazy loaded)
	// ES: Assets (todas referencias soft — carga lazy)
	// ======================================================================

	/** EN: Background images for StaticImage/Slideshow types / ES: Imagenes de fondo */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|Assets")
	TArray<TSoftObjectPtr<UTexture2D>> BackgroundImages;

	/** EN: Background materials for MaterialAnimated type / ES: Materiales de fondo */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|Assets", meta = (AdvancedDisplay))
	TArray<TSoftObjectPtr<UMaterialInterface>> BackgroundMaterials;

	/** EN: Video source for Video type / ES: Fuente de video para tipo Video */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|Assets",
		meta = (AdvancedDisplay, EditCondition = "DefaultVisualType == EPGXLoadingVisualType::Video"))
	TSoftObjectPtr<UMediaSource> VideoSource;

	/** EN: Custom strategy class for Custom type / ES: Clase de estrategia custom */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|Assets",
		meta = (AdvancedDisplay, EditCondition = "DefaultVisualType == EPGXLoadingVisualType::Custom"))
	TSubclassOf<UPGXLoadingStrategyBase> CustomStrategyClass;

	/** EN: Widget class override (null = use default from config) / ES: Override de clase widget */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|Assets", meta = (AdvancedDisplay))
	TSubclassOf<UPGXLoadingWidget> WidgetClassOverride;

	// ======================================================================
	// EN: Tips / ES: Consejos
	// ======================================================================

	/** EN: Tip texts to display during loading / ES: Textos de consejo durante carga */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|Tips")
	TArray<FText> Tips;

	/** EN: Time between tip rotations / ES: Tiempo entre rotaciones de consejos */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|Tips",
		meta = (AdvancedDisplay, ClampMin = "2.0", ClampMax = "30.0"))
	float TipRotationInterval = 5.0f;

	// ======================================================================
	// EN: Behavior Overrides / ES: Overrides de comportamiento
	// ======================================================================

	/** EN: Close policy override for this context / ES: Override de politica de cierre */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|Behavior", meta = (AdvancedDisplay))
	EPGXLoadingClosePolicy ClosePolicyOverride = EPGXLoadingClosePolicy::Automatic;

	/** EN: Allow user to skip loading (if close conditions permit) / ES: Permitir skip */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|Behavior", meta = (AdvancedDisplay))
	bool bAllowSkip = false;

	/** EN: Show progress bar in the widget / ES: Mostrar barra de progreso */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|Behavior", meta = (AdvancedDisplay))
	bool bShowProgressBar = true;

	/** EN: Wait for PSO warm-up before closing / ES: Esperar warm-up PSO antes de cerrar */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|Behavior", meta = (AdvancedDisplay))
	bool bWaitForPSO = true;

	/** EN: Minimum display time for this profile (overrides config default) / ES: Tiempo minimo */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|Behavior",
		meta = (AdvancedDisplay, ClampMin = "0.0", ClampMax = "10.0"))
	float MinDisplayTime = 1.0f;

	/** EN: Fade configuration override / ES: Override de configuracion de fade */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|Behavior", meta = (AdvancedDisplay))
	FPGXFadeConfig FadeConfig;

	// ======================================================================
	// EN: Slideshow Config / ES: Config de slideshow
	// ======================================================================

	/** EN: Time between slideshow images / ES: Tiempo entre imagenes del slideshow */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|Slideshow",
		meta = (AdvancedDisplay, EditCondition = "DefaultVisualType == EPGXLoadingVisualType::Slideshow",
				ClampMin = "1.0", ClampMax = "30.0"))
	float SlideshowInterval = 5.0f;

	/** EN: Crossfade duration between slideshow images / ES: Duracion crossfade entre imagenes */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Loading|Slideshow",
		meta = (AdvancedDisplay, EditCondition = "DefaultVisualType == EPGXLoadingVisualType::Slideshow",
				ClampMin = "0.1", ClampMax = "3.0"))
	float SlideshowCrossfadeDuration = 0.5f;

	// ======================================================================
	// EN: Widget Contract — Expected named slots
	// ES: Contrato de Widget — Slots con nombre esperados
	// ======================================================================

	/**
	 * EN: Read-only contract showing which named widgets the loading Widget Blueprint
	 *     should contain. The developer sees this in the Details panel and knows exactly
	 *     which names to use in their UMG Widget — zero documentation needed.
	 * ES: Contrato read-only que muestra que widgets con nombre debe contener el Widget Blueprint
	 *     de loading. El desarrollador lo ve en el panel Details y sabe exactamente
	 *     que nombres usar en su Widget UMG — zero documentacion necesaria.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PGX|Contract")
	FPGXWidgetContract WidgetContract;
};
