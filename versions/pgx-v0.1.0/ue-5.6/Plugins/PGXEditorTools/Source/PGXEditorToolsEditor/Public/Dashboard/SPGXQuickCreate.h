// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * EN: Quick Create panel in the PGX Hub.
 *     Provides buttons for creating Config DAs, Object DAs, Blueprints, and DataTables.
 * ES: Panel de Creacion Rapida en el PGX Hub.
 *     Proporciona botones para crear Config DAs, Object DAs, Blueprints y DataTables.
 */
class PGXEDITORTOOLSEDITOR_API SPGXQuickCreate : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXQuickCreate) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};
