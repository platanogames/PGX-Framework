// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * EN: Status bar widget showing PGX framework version and plugin count.
 *     Minimal display — version from PGXVersion namespace.
 * ES: Widget de status bar mostrando version del framework PGX y conteo de plugins.
 *     Display minimo — version desde namespace PGXVersion.
 */
class PGXCOREEDITOR_API SPGXStatusBarWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXStatusBarWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	// EN: Register widget with the editor status bar / ES: Registrar widget con la status bar del editor
	static void Register();

	// EN: Unregister widget / ES: Desregistrar widget
	static void Unregister();
};
