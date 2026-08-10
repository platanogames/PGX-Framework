// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Base class for PGX IM widgets embeddable in NomadTabs.
// ES: Clase base para widgets IM de PGX embebibles en NomadTabs.
#pragma once

#include "CoreMinimal.h"
#include "SlateIMWidgetBase.h"

/**
 * EN: Base class for immediate-mode graph/debug widgets.
 *     Derives from FSlateIMExposedBase for embedding in existing Slate hierarchies.
 *     Subclass and override DrawContent() to render your IM UI.
 *
 * ES: Clase base para widgets de grafico/debug en modo inmediato.
 *     Deriva de FSlateIMExposedBase para embedder en jerarquias Slate existentes.
 *     Subclasear y overridear DrawContent() para renderizar tu UI IM.
 *
 * Usage / Uso:
 *   MyGraph = MakeUnique<FMyIMWidget>();
 *   ChildSlot[ MyGraph->GetExposedWidget() ];
 */
class PGXCOREEDITOR_API FPGXSlateIMWidget : public FSlateIMExposedBase
{
public:
	explicit FPGXSlateIMWidget(const FStringView& Name)
		: FSlateIMExposedBase(Name)
	{
	}

	virtual ~FPGXSlateIMWidget() = default;

protected:
	void DrawContent(float DeltaTime) override = 0;
};
