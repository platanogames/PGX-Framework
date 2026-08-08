// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Button helper functions — creates SButton with Premium tier styling.
//     Three tiers: Primary (filled accent), Secondary (outlined), Ghost (text-only).
// ES: Funciones helper de botones — crea SButton con estilo Premium por tier.
//     Tres tiers: Primary (relleno acento), Secondary (contorno), Ghost (solo texto).
#pragma once

#include "CoreMinimal.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Style/PGXEditorStyle.h"
#include "Style/PGXVisualTokens.h"

namespace PGXButton
{
	// EN: Button tier — determines visual style
	// ES: Tier de boton — determina estilo visual
	enum class ETier : uint8
	{
		Primary,    // Filled, accent border — main actions
		Secondary,  // Outlined, Glass border — secondary actions
		Ghost       // No bg, no border — tertiary/inline actions
	};

	// EN: Get the FButtonStyle name for a tier
	// ES: Obtener el nombre del FButtonStyle para un tier
	inline FName GetStyleName(ETier Tier)
	{
		switch (Tier)
		{
		case ETier::Primary:   return FName(TEXT("PGXEditor.Button.Primary"));
		case ETier::Secondary: return FName(TEXT("PGXEditor.Button.Secondary"));
		case ETier::Ghost:     return FName(TEXT("PGXEditor.Button.Ghost"));
		default:               return FName(TEXT("PGXEditor.Button.Secondary"));
		}
	}

	// EN: Create a button with Premium styling
	// ES: Crear un boton con estilo Premium
	inline TSharedRef<SButton> Make(
		ETier Tier,
		const FText& Label,
		const FOnClicked& OnClicked,
		const FLinearColor& TextColor = PGX::Text::Primary)
	{
		const FName StyleName = GetStyleName(Tier);
		const FButtonStyle* Style = &FPGXEditorStyle::Get().GetWidgetStyle<FButtonStyle>(StyleName);

		TSharedRef<SButton> Button = SNew(SButton)
			.ButtonStyle(Style)
			.OnClicked(OnClicked)
			.ContentPadding(FMargin(PGX::Spacing::LG, PGX::Spacing::SM))
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(PGX::Font::Body())
				.ColorAndOpacity(FSlateColor(TextColor))
			];

		return Button;
	}
}
