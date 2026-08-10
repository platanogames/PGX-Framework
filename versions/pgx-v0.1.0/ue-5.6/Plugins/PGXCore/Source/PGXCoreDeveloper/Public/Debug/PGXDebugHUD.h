// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PGXDebugHUD.generated.h"

/**
 * EN: Debug HUD for PGX runtime inspection.
 *     Displays system panels with real-time information.
 *     Stripped from Shipping builds.
 *
 * ES: HUD de depuracion para inspeccion runtime de PGX.
 *     Muestra paneles de sistema con informacion en tiempo real.
 *     Se elimina de los builds de Shipping.
 */
UCLASS()
class APGXDebugHUD : public AHUD
{
	GENERATED_BODY()

public:
	/** EN: Draw the debug HUD overlay / ES: Dibujar el overlay del HUD de depuracion */
	void DrawHUD() override;
};
