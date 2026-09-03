// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class STextBlock;

/**
 * EN: PGX Inventory observability panel (preview).
 *     - Section A: UPGXItemDefinition assets — observable IPGXObservable adopted
 *     - Section B: UPGXInventoryComponent derived classes loaded (live state runtime-derived)
 *     - Section C: Subsystem GAP banner + Schema demo via ItemDefinition GetSchemaDescriptor
 *
 * ES: Panel observabilidad PGX Inventory (preview). ItemDef observable preview.
 */
class SPGXInventoryPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXInventoryPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FReply OnRefreshClicked();
	FReply OnShowSchemaClicked();
	void RefreshState();

	TSharedPtr<STextBlock> ItemDefCountText;
	TSharedPtr<STextBlock> ComponentCountText;
	TSharedPtr<STextBlock> SchemaText;

	// EN: Widget-owned brushes (leak-prevention per the panel ownership policy).
	TSharedPtr<struct FSlateBrush> SurfaceBaseBrush;
	TSharedPtr<struct FSlateBrush> SurfaceRaisedBrush;
	TSharedPtr<struct FSlateBrush> AccentBrush;
};
