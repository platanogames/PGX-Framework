// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

/**
 * EN: Custom Slate style set for PGX Editor icons.
 *     Registers SVG icons from PGXCore/Resources/Icons/ as vector Slate brushes.
 *     Icons are referenced as "PGXEditor.Icon.Name" in FSlateIcon constructors.
 *
 * ES: Conjunto de estilos Slate personalizados para iconos del Editor PGX.
 *     Registra iconos SVG de PGXCore/Resources/Icons/ como brushes vectoriales Slate.
 *     Los iconos se referencian como "PGXEditor.Icon.Name" en constructores FSlateIcon.
 */
class PGXCOREEDITOR_API FPGXEditorStyle
{
public:
	static void Initialize();
	static void Shutdown();

	static const FName& GetStyleSetName();
	static const ISlateStyle& Get();

private:
	static TSharedPtr<FSlateStyleSet> StyleInstance;
};
