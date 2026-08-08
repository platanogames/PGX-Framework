// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "PGXLoadingTypes.h"
#include "PGXLoadingWidget.generated.h"

class UImage;
class UProgressBar;
class UTextBlock;
class UButton;

/**
 * EN: Base UUserWidget for loading screens. Designers subclass this in Blueprint for custom visuals.
 *     All logic (state, timing, flow) stays in the Subsystem — the widget is purely visual.
 *     Optional BindWidgets allow strategies to set background images, progress bars, tips, etc.
 *     If a bound widget is missing, strategies degrade gracefully.
 *
 * ES: UUserWidget base para pantallas de carga. Los disenadores la subclasean en Blueprint.
 *     Toda la logica (estado, timing, flujo) se mantiene en el Subsistema — el widget es puramente visual.
 *     BindWidgets opcionales permiten que las estrategias configuren imagenes, barras, consejos, etc.
 */
UCLASS(Blueprintable)
class PGXLOADINGRUNTIME_API UPGXLoadingWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ======================================================================
	// EN: Events called by Subsystem / Strategy (implement in Blueprint subclass)
	// ES: Eventos llamados por Subsistema / Strategy (implementar en subclase Blueprint)
	// ======================================================================

	/** EN: Loading screen activated with context and visual type / ES: Pantalla activada */
	UFUNCTION(BlueprintImplementableEvent, Category = "PGX|Loading")
	void OnLoadingActivated(FGameplayTag ContextTag, EPGXLoadingVisualType VisualType);

	/** EN: Progress updated / ES: Progreso actualizado */
	UFUNCTION(BlueprintImplementableEvent, Category = "PGX|Loading")
	void OnProgressChanged(float Progress, const FText& StatusMessage);

	/** EN: Tip text changed / ES: Texto de consejo cambiado */
	UFUNCTION(BlueprintImplementableEvent, Category = "PGX|Loading")
	void OnTipChanged(const FText& TipText);

	/** EN: Fade in started / ES: Fade in iniciado */
	UFUNCTION(BlueprintImplementableEvent, Category = "PGX|Loading")
	void OnFadeInStarted(float Duration);

	/** EN: Fade out started / ES: Fade out iniciado */
	UFUNCTION(BlueprintImplementableEvent, Category = "PGX|Loading")
	void OnFadeOutStarted(float Duration);

	/** EN: Loading screen deactivated / ES: Pantalla desactivada */
	UFUNCTION(BlueprintImplementableEvent, Category = "PGX|Loading")
	void OnLoadingDeactivated();

	// ======================================================================
	// EN: Optional bound widgets (subclass can bind in UMG designer)
	// ES: Widgets opcionales (subclase puede enlazar en el disenador UMG)
	// ======================================================================

	/** EN: Background image widget / ES: Widget de imagen de fondo */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Loading", meta = (BindWidgetOptional))
	TObjectPtr<UImage> IMG_Background;

	/** EN: Progress bar widget / ES: Widget de barra de progreso */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Loading", meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> PB_Progress;

	/** EN: Tip text widget / ES: Widget de texto de consejo */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Loading", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TXT_Tip;

	/** EN: Status text widget / ES: Widget de texto de estado */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Loading", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TXT_Status;

	/** EN: Skip button widget / ES: Widget de boton skip */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Loading", meta = (BindWidgetOptional))
	TObjectPtr<UButton> BTN_Skip;

	/** EN: Continue button widget (ManualClose mode) / ES: Widget de boton continuar (modo ManualClose) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Loading", meta = (BindWidgetOptional))
	TObjectPtr<UButton> BTN_Continue;

	// ======================================================================
	// EN: C++ API for Subsystem / Strategy
	// ES: API C++ para Subsistema / Strategy
	// ======================================================================

	/** EN: Set background image texture / ES: Establecer textura de imagen de fondo */
	UFUNCTION(BlueprintCallable, Category = "PGX|Loading")
	void SetBackgroundImage(UTexture2D* Texture);

	/** EN: Set background material / ES: Establecer material de fondo */
	UFUNCTION(BlueprintCallable, Category = "PGX|Loading")
	void SetBackgroundMaterial(UMaterialInterface* Material);

	/** EN: Set progress bar value [0.0 - 1.0] / ES: Establecer valor de barra de progreso */
	UFUNCTION(BlueprintCallable, Category = "PGX|Loading")
	void SetProgressValue(float Progress);

	/** EN: Set tip text / ES: Establecer texto de consejo */
	UFUNCTION(BlueprintCallable, Category = "PGX|Loading")
	void SetTipText(const FText& TipText);

	/** EN: Set status text / ES: Establecer texto de estado */
	UFUNCTION(BlueprintCallable, Category = "PGX|Loading")
	void SetStatusText(const FText& StatusText);

	/** EN: Set skip button visibility / ES: Establecer visibilidad del boton skip */
	UFUNCTION(BlueprintCallable, Category = "PGX|Loading")
	void SetSkipButtonVisible(bool bVisible);

	/** EN: Set progress bar visibility / ES: Establecer visibilidad de barra de progreso */
	UFUNCTION(BlueprintCallable, Category = "PGX|Loading")
	void SetProgressBarVisible(bool bVisible);

	/**
	 * EN: Bind skip button click to a callback. Called by Subsystem during activation.
	 * ES: Enlazar click del boton skip a un callback. Llamado por Subsistema durante activacion.
	 */
	void BindSkipButton(const FSimpleDelegate& Callback);

	/** EN: Set continue button visibility / ES: Establecer visibilidad del boton continuar */
	UFUNCTION(BlueprintCallable, Category = "PGX|Loading")
	void SetContinueButtonVisible(bool bVisible);

	/**
	 * EN: Bind continue button click to a callback. Called by Subsystem for ManualClose policy.
	 * ES: Enlazar click del boton continuar a un callback. Para politica ManualClose.
	 */
	void BindContinueButton(const FSimpleDelegate& Callback);

private:
	// EN: UFUNCTION handlers for dynamic delegate binding (OnClicked is dynamic, can't use AddLambda)
	// ES: Handlers UFUNCTION para binding de delegados dinamicos (OnClicked es dinamico, no permite AddLambda)
	UFUNCTION()
	void HandleSkipClicked();

	UFUNCTION()
	void HandleContinueClicked();

	FSimpleDelegate SkipCallback;
	FSimpleDelegate ContinueCallback;
};
