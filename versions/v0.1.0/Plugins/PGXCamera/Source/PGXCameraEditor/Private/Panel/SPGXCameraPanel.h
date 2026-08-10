// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class STextBlock;

/**
 * EN: PGX Camera observability panel (Config DataAsset inspector and schema preview).
 *     - Section A: UPGXCameraConfig asset count + reflected Fields count (schema via IPGXObservable)
 *     - Section B: Subsystem live state — selected mode read via UPGXCameraSubsystem API
 *     - Section C: JSON envelope demo button — invokes ToJson() of first Config asset and shows raw text
 *
 * ES: Panel observabilidad PGX Camera (inspector de Config DataAsset y vista previa del schema).
 */
class SPGXCameraPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXCameraPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FReply OnRefreshClicked();
	FReply OnShowEnvelopeClicked();
	void RefreshState();

	TSharedPtr<STextBlock> ConfigCountText;
	TSharedPtr<STextBlock> FieldsCountText;
	TSharedPtr<STextBlock> SubsystemStateText;
	TSharedPtr<STextBlock> EnvelopeText;

	// EN: Widget-owned brushes (leak-prevention per the panel ownership policy).
	TSharedPtr<struct FSlateBrush> SurfaceRaisedBrush;
};
