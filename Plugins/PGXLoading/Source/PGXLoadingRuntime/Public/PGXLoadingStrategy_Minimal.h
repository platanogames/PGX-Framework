// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXLoadingStrategyBase.h"
#include "PGXLoadingStrategy_Minimal.generated.h"

/**
 * EN: Minimal loading strategy — black screen with no external assets.
 *     Always available as fallback. Zero async loading required.
 *     If the widget has optional elements (progress bar, status text), they are configured.
 *
 * ES: Estrategia minima de carga — pantalla negra sin assets externos.
 *     Siempre disponible como fallback. Cero carga async requerida.
 */
UCLASS()
class PGXLOADINGRUNTIME_API UPGXLoadingStrategy_Minimal : public UPGXLoadingStrategyBase
{
	GENERATED_BODY()

public:
	void InitializeStrategy(UPGXLoadingProfile* Profile) override;
	void Activate(UPGXLoadingWidget* Widget) override;
	void Deactivate() override;
	bool IsReady() const override;
	FText GetCurrentTip() const override;

private:
	int32 CurrentTipIndex = 0;
	double LastTipTime = 0.0;
};
