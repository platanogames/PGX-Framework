// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Tutorial overlay widget — arrow indicator + callout + navigation + action feedback.
// ES: Widget overlay de tutorial — indicador flecha + callout + navegacion + feedback de acciones.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class FPGXTutorialRunner;
class FSlateWindowElementList;
class SEditableTextBox;

/**
 * EN: Full-window overlay that draws an animated arrow pointing at the target tab,
 *     positions a navigation callout with step content, shows action feedback,
 *     and supports base path configuration for Constructor tutorials.
 * ES: Overlay de ventana completa que dibuja una flecha animada apuntando al tab objetivo,
 *     posiciona un callout de navegacion con contenido del paso, muestra feedback de acciones,
 *     y soporta configuracion de ruta base para tutoriales Constructor.
 */
class SPGXTutorialOverlay : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXTutorialOverlay) {}
		SLATE_ARGUMENT(FPGXTutorialRunner*, Runner)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	// -- SWidget overrides --
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

private:
	/** EN: Rebuild callout content for the current step / ES: Reconstruir contenido del callout */
	void RebuildCallout();

	/** EN: Find the target tab widget and cache its geometry / ES: Encontrar el widget del tab objetivo */
	void UpdateTargetGeometry();

	/** EN: Position the callout relative to the target / ES: Posicionar el callout relativo al target */
	void PositionCallout(const FGeometry& AllottedGeometry);

	FPGXTutorialRunner* Runner = nullptr;

	// EN: Cached geometry of the target tab in absolute coords
	FGeometry CachedTargetGeometry;
	bool bHasValidTarget = false;

	// EN: Arrow indicator state
	// ES: Estado del indicador de flecha
	enum class EArrowDirection : uint8 { None, Left, Right, Up, Down };
	EArrowDirection ArrowDir = EArrowDirection::None;
	FVector2D ArrowBasePosition = FVector2D::ZeroVector;
	float ArrowTime = 0.0f;
	static constexpr float ArrowChevronSize = 14.0f;
	static constexpr float ArrowBounceAmplitude = 6.0f;
	static constexpr float ArrowBounceSpeed = 4.0f;
	static constexpr float ArrowLineThickness = 4.0f;

	// EN: Callout widgets
	TSharedPtr<SWidget> CalloutWidget;
	TSharedPtr<class SCanvas> CalloutCanvas;

	// EN: Callout cached position
	FVector2D CalloutPosition = FVector2D::ZeroVector;
	static constexpr float CalloutWidth = 480.0f;
	static constexpr float CalloutHeight = 600.0f;

	// EN: Frame delay for newly opened tabs (need layout before capturing geometry)
	// ES: Delay de frames para tabs recien abiertos (necesitan layout antes de capturar geometria)
	bool bWaitingForLayout = false;
	int32 LayoutWaitFrames = 0;

	// EN: Pending base path for ConfigBasePath action (editable text box)
	// ES: Ruta base pendiente para accion ConfigBasePath (campo de texto editable)
	FString PendingBasePath;
};
