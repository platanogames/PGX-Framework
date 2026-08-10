// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * EN: Step-by-step wizard for creating new Object DataAsset categories.
 *     Generates C++ code: DataAsset subclass, struct, interface, factory, and DataTable row.
 * ES: Wizard paso a paso para crear nuevas categorias de Object DataAssets.
 *     Genera codigo C++: subclase DataAsset, struct, interface, factory y fila de DataTable.
 */
class PGXEDITORTOOLSEDITOR_API SPGXNewCategoryWizard : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXNewCategoryWizard) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};
