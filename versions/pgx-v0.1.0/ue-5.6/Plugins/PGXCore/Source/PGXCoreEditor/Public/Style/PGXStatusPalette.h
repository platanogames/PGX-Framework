// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Canonical status palette helper for editor inspectors.
// ES: Helper canonico de paleta de estado para inspectores editor.
#pragma once

#include "CoreMinimal.h"
#include "Style/PGXVisualTokens.h"

namespace PGX
{
	enum class EStatusPalette : uint8
	{
		Idle,
		Preparing,
		Loading,
		Active,
		FadingIn,
		Waiting,
		FadingOut,
		Transitioning,
		PostLoad,
		Complete,
		Failed,
		Paused,
		Unknown
	};

	inline FLinearColor GetStatusPaletteColor(const EStatusPalette Status)
	{
		switch (Status)
		{
		case EStatusPalette::Idle:          return StatePalette::Idle;
		case EStatusPalette::Preparing:     return StatePalette::Preparing;
		case EStatusPalette::Loading:       return StatePalette::Loading;
		case EStatusPalette::Active:        return StatePalette::Active;
		case EStatusPalette::FadingIn:      return StatePalette::FadingIn;
		case EStatusPalette::Waiting:       return StatePalette::Waiting;
		case EStatusPalette::FadingOut:     return StatePalette::PostLoad;
		case EStatusPalette::Transitioning: return StatePalette::Transitioning;
		case EStatusPalette::PostLoad:      return StatePalette::PostLoad;
		case EStatusPalette::Complete:      return StatePalette::Complete;
		case EStatusPalette::Failed:        return StatePalette::Failed;
		case EStatusPalette::Paused:        return StatePalette::Paused;
		default:                            return StatePalette::Unknown;
		}
	}
}
