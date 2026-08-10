// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "UI/SPGXScaffoldProgressWindow.h"
#include "Style/PGXVisualTokens.h"

#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "PGXScaffoldProgress"

void SPGXScaffoldProgressWindow::Construct(const FArguments& InArgs)
{
	TotalStepsCount = InArgs._TotalSteps;

	ChildSlot
	[
		SNew(SBox).Padding(PGX::Spacing::XL)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, PGX::Spacing::MD)
			[
				SNew(STextBlock)
					.Text(LOCTEXT("ProgressTitle", "Executing scaffold plan..."))
					.Font(PGX::Font::SectionHeader())
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, PGX::Spacing::MD)
			[
				SAssignNew(ProgressBar, SProgressBar)
					.Percent_Lambda([this]() -> TOptional<float>
					{
						if (TotalStepsCount <= 0) { return 0.0f; }
						return static_cast<float>(CurrentStepValue) / static_cast<float>(TotalStepsCount);
					})
			]

			+ SVerticalBox::Slot().AutoHeight()
			[
				SAssignNew(StepText, STextBlock)
					.Text_Lambda([this]() -> FText
					{
						return FText::Format(LOCTEXT("StepProgress", "Step {0} / {1}"),
							FText::AsNumber(CurrentStepValue), FText::AsNumber(TotalStepsCount));
					})
					.Font(PGX::Font::Body())
					.ColorAndOpacity(PGX::Text::Secondary)
			]
		]
	];
}

void SPGXScaffoldProgressWindow::UpdateProgress(int32 CurrentStep, int32 TotalSteps)
{
	CurrentStepValue = CurrentStep;
	TotalStepsCount = TotalSteps;
}

#undef LOCTEXT_NAMESPACE
