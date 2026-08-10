// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * EN: Modal progress window shown during scaffold execution.
 *     Displays current step, progress bar, and status message.
 *
 * ES: Ventana modal de progreso mostrada durante la ejecucion de scaffold.
 *     Muestra paso actual, barra de progreso, y mensaje de estado.
 */
class SPGXScaffoldProgressWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXScaffoldProgressWindow)
		: _TotalSteps(0)
	{}
		SLATE_ARGUMENT(int32, TotalSteps)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** EN: Update progress from executor callback / ES: Actualizar progreso desde callback del ejecutor */
	void UpdateProgress(int32 CurrentStep, int32 TotalSteps);

private:
	int32 TotalStepsCount = 0;
	int32 CurrentStepValue = 0;
	TSharedPtr<class STextBlock> StepText;
	TSharedPtr<class SProgressBar> ProgressBar;
};
