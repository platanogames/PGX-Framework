// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXViewportOverlayManager.generated.h"

class UPGXLoadingWidget;
class UUserWidget;

/**
 * EN: Manages the persistent viewport overlay that survives level transitions.
 *     Handles widget creation, viewport injection via GameViewportClient,
 *     opacity control for fade, and cleanup.
 *
 *     Key guarantee: The overlay is injected into the GameViewport (not the World),
 *     so it persists across OpenLevel calls.
 *
 * ES: Gestiona el overlay persistente del viewport que sobrevive transiciones de nivel.
 *     Maneja creacion de widget, inyeccion en viewport via GameViewportClient,
 *     control de opacidad para fade, y limpieza.
 *
 *     Garantia clave: El overlay se inyecta en el GameViewport (no en el World),
 *     por lo que persiste entre llamadas a OpenLevel.
 */
UCLASS()
class PGXLOADINGRUNTIME_API UPGXViewportOverlayManager : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * EN: Create and inject a minimal loading widget into the GameViewportClient.
	 *     If a widget class is provided, creates that class; otherwise creates a basic black overlay.
	 *     Safe to call multiple times — only creates one overlay.
	 * ES: Crear e inyectar un widget de carga minimo en el GameViewportClient.
	 */
	void ShowOverlay(int32 ZOrder, TSubclassOf<UUserWidget> WidgetClass = nullptr);

	/** EN: Remove widget from viewport and release reference / ES: Remover widget del viewport */
	void HideOverlay();

	/** EN: Check if overlay is currently injected / ES: Verificar si el overlay esta activo */
	bool IsOverlayActive() const { return bOverlayActive; }

	/** EN: Get active widget (null if not active) / ES: Obtener widget activo */
	UUserWidget* GetActiveWidget() const { return ActiveWidget; }

	/** EN: Set opacity for fade control [0.0 - 1.0] / ES: Establecer opacidad */
	void SetOverlayOpacity(float Alpha);

private:
	UPROPERTY()
	TObjectPtr<UUserWidget> ActiveWidget;

	/** EN: Slate widget content handle for viewport removal / ES: Handle de contenido Slate para remocion */
	TSharedPtr<SWidget> OverlaySlateWidget;

	bool bOverlayActive = false;
};
