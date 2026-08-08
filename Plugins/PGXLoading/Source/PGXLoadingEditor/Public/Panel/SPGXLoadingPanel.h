// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * EN: PGX Loading and LevelFlow observability panel. Presents loading configuration,
 *     runtime status, level transitions, and active profiles in the shared editor shell.
 * ES: Panel de observabilidad de PGX Loading y LevelFlow. Presenta configuracion,
 *     estado runtime, transiciones de nivel y perfiles activos en el editor compartido.
 */
class PGXLOADINGEDITOR_API SPGXLoadingPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXLoadingPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};
