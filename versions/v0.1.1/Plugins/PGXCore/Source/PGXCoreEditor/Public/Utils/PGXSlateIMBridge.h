// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Thin wrapper over SlateIM integrating PGX visual tokens.
// ES: Wrapper ligero sobre SlateIM integrando tokens visuales PGX.
#pragma once

#include "CoreMinimal.h"
#include "Style/PGXVisualTokens.h"

/**
 * EN: Convenience functions wrapping SlateIM calls with PGX token defaults.
 *     Each function wraps 3-5 SlateIM calls into 1 PGX call.
 *     Use inside FSlateIMExposedBase::DrawContent() or SlateIM window contexts.
 *
 * ES: Funciones de conveniencia wrapping SlateIM con tokens PGX por defecto.
 *     Cada funcion wrappea 3-5 llamadas SlateIM en 1 llamada PGX.
 *     Usar dentro de FSlateIMExposedBase::DrawContent() o contextos de ventana SlateIM.
 */
namespace PGXSlateIM
{
	// === Quick Graph ===
	// EN: Wraps BeginGraph/GraphLine/EndGraph with PGX defaults
	PGXCOREEDITOR_API void QuickGraph(
		const TArrayView<double>& Values,
		const FLinearColor& Color = PGX::Semantic::Info,
		float Thickness = 1.5f,
		double MinY = 0.0,
		double MaxY = 100.0);

	// EN: Multi-line graph (up to 4 lines in one graph)
	struct PGXCOREEDITOR_API FGraphLine
	{
		TArrayView<double> Values;
		FLinearColor Color = PGX::Semantic::Info;
		float Thickness = 1.5f;
	};
	PGXCOREEDITOR_API void MultiGraph(
		const TArray<FGraphLine>& Lines,
		double MinY = 0.0,
		double MaxY = 100.0);

	// === Themed Text ===
	PGXCOREEDITOR_API void Title(const FStringView& InText);
	PGXCOREEDITOR_API void Section(const FStringView& InText);
	PGXCOREEDITOR_API void Label(const FStringView& InText);
	PGXCOREEDITOR_API void Muted(const FStringView& InText);
	PGXCOREEDITOR_API void Status(const FStringView& InText, const FLinearColor& Color);

	// === Themed Separator ===
	PGXCOREEDITOR_API void Separator();
}
