// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "GameFlowInspector/SPGXGameFlowChannelRow.h"
#include "PGXGameFlowSubsystem.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Styling/AppStyle.h"
#include "Style/PGXVisualTokens.h"

// EN: GameFlow Channel Row - Expandable row for a single FSM channel
// ES: GameFlow Channel Row - Fila expandible para un canal FSM individual

#define LOCTEXT_NAMESPACE "PGXGameFlowInspector"

void SPGXGameFlowChannelRow::Construct(const FArguments& InArgs)
{
	Summary = InArgs._Summary;
	Subsystem = InArgs._Subsystem;
	ChannelColor = InArgs._ChannelColor;

	if (!Summary.IsValid())
	{
		ChildSlot
		[
			SNew(STextBlock).Text(LOCTEXT("NoChannel", "---"))
		];
		return;
	}

	const FString CurrentStr = Summary->CurrentTag.IsValid()
		? Summary->CurrentTag.ToString()
		: TEXT("None");
	const FString LastStr = Summary->LastTag.IsValid()
		? Summary->LastTag.ToString()
		: TEXT("None");

	ChildSlot
	[
		SAssignNew(RootBox, SVerticalBox)

		// EN: Main collapsed row / ES: Fila colapsada principal
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SButton)
			.ButtonStyle(FAppStyle::Get(), "NoBorder")
			.OnClicked_Lambda([this]() -> FReply
			{
				bExpanded = !bExpanded;
				if (bExpanded && !ExpandedContent.IsValid())
				{
					ExpandedContent = BuildExpandedPanel();
					RootBox->AddSlot()
						.AutoHeight()
						[
							ExpandedContent.ToSharedRef()
						];
				}
				else if (ExpandedContent.IsValid())
				{
					ExpandedContent->SetVisibility(bExpanded ? EVisibility::Visible : EVisibility::Collapsed);
				}
				return FReply::Handled();
			})
			[
				SNew(SHorizontalBox)

				// EN: Color indicator (8px strip) / ES: Indicador de color (tira de 8px)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Fill)
				.Padding(0.0f, 0.0f, 4.0f, 0.0f)
				[
					SNew(SBorder)
					.BorderBackgroundColor(ChannelColor)
					.Padding(FMargin(0.0f))
					[
						SNew(SBox)
						.WidthOverride(8.0f)
					]
				]

				// EN: Channel name (bold, 100px) / ES: Nombre de canal (bold, 100px)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(100.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(Summary->ChannelName))
						.Font(PGX::Font::SubHeader())
						.ColorAndOpacity(FSlateColor(ChannelColor))
					]
				]

				// EN: Current tag (fill) / ES: Tag actual (fill)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				.Padding(4.0f, 0.0f)
				[
					SAssignNew(CurrentTagText, STextBlock)
					.Text(FText::FromString(FString::Printf(TEXT("Current: %s"), *CurrentStr)))
					.ColorAndOpacity(FSlateColor(Summary->CurrentTag.IsValid()
						? PGX::Semantic::Good
						: PGX::Text::Muted)) // EN: Good for active, Muted for "None" / ES: Good para activo, Muted para "None"
					.Font(PGX::Font::Mono())
				]

				// EN: Last tag (gray) / ES: Tag anterior (gris)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4.0f, 0.0f)
				[
					SNew(SBox)
					.WidthOverride(180.0f)
					[
						SAssignNew(LastTagText, STextBlock)
						.Text(FText::FromString(FString::Printf(TEXT("Last: %s"), *LastStr)))
						.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
						.Font(PGX::Font::Mono())
					]
				]

				// EN: History count / ES: Conteo de historial
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4.0f, 0.0f)
				[
					SNew(SBox)
					.WidthOverride(50.0f)
					[
						SAssignNew(HistoryCountText, STextBlock)
						.Text(FText::Format(LOCTEXT("HistCountFmt", "[{0}]"), FText::AsNumber(Summary->HistoryCount)))
						.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
						.Font(PGX::Font::Mono())
					]
				]

				// EN: Revert indicator / ES: Indicador de revert
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(2.0f, 0.0f)
				[
					SAssignNew(RevertIndicator, STextBlock)
					.Text(Summary->bCanRevert ? LOCTEXT("RevertYes", "REV") : LOCTEXT("RevertNo", ""))
					.ColorAndOpacity(FSlateColor(PGX::Semantic::Good))
					.Font(PGX::Font::CaptionBold())
				]
			]
		]
	];
}

void SPGXGameFlowChannelRow::UpdateSummary(const FPGXFlowChannelSummary& NewSummary)
{
	if (!Summary.IsValid())
	{
		return;
	}

	*Summary = NewSummary;

	const FString CurrentStr = Summary->CurrentTag.IsValid()
		? Summary->CurrentTag.ToString()
		: TEXT("None");
	const FString LastStr = Summary->LastTag.IsValid()
		? Summary->LastTag.ToString()
		: TEXT("None");

	if (CurrentTagText.IsValid())
	{
		CurrentTagText->SetText(FText::FromString(FString::Printf(TEXT("Current: %s"), *CurrentStr)));
		CurrentTagText->SetColorAndOpacity(FSlateColor(Summary->CurrentTag.IsValid()
			? PGX::Semantic::Good
			: PGX::Text::Muted));
	}

	if (LastTagText.IsValid())
	{
		LastTagText->SetText(FText::FromString(FString::Printf(TEXT("Last: %s"), *LastStr)));
	}

	if (HistoryCountText.IsValid())
	{
		HistoryCountText->SetText(FText::Format(LOCTEXT("HistCountFmt", "[{0}]"), FText::AsNumber(Summary->HistoryCount)));
	}

	if (RevertIndicator.IsValid())
	{
		RevertIndicator->SetText(Summary->bCanRevert ? LOCTEXT("RevertYes", "REV") : LOCTEXT("RevertNo", ""));
	}
}

TSharedRef<SWidget> SPGXGameFlowChannelRow::BuildExpandedPanel() const
{
	TSharedRef<SVerticalBox> Panel = SNew(SVerticalBox);

	UPGXGameFlowSubsystem* Sub = Subsystem.Get();
	if (!Sub || !Summary.IsValid())
	{
		Panel->AddSlot()
			.AutoHeight()
			.Padding(20.0f, 2.0f, 0.0f, 4.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("NoSubsystem", "Subsystem not available"))
				.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
			];
		return Panel;
	}

	// EN: Active rule section / ES: Seccion de regla activa
	FPGXFlowRule OutRule;
	const bool bHasRule = Sub->GetAllowedTransitionByCurrentFlowTag(Summary->Channel, OutRule);

	Panel->AddSlot()
		.AutoHeight()
		.Padding(20.0f, 4.0f, 0.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("RuleHeader", "Active Rule:"))
			.Font(PGX::Font::Badge())
			.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.8f, 0.4f)))
		];

	if (bHasRule)
	{
		// EN: Rule name / ES: Nombre de regla
		Panel->AddSlot()
			.AutoHeight()
			.Padding(32.0f, 1.0f, 0.0f, 1.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("Name: %s"), *OutRule.RuleName.ToString())))
				.Font(PGX::Font::Mono())
				.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
			];

		// EN: Description / ES: Descripcion
		if (!OutRule.Description.IsEmpty())
		{
			Panel->AddSlot()
				.AutoHeight()
				.Padding(32.0f, 1.0f, 0.0f, 1.0f)
				[
					SNew(STextBlock)
					.Text(FText::Format(LOCTEXT("RuleDescFmt", "Description: {0}"), OutRule.Description))
					.Font(PGX::Font::Mono())
					.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
					.AutoWrapText(true)
				];
		}

		// EN: Allow revert + Error code / ES: Permitir revert + Codigo de error
		Panel->AddSlot()
			.AutoHeight()
			.Padding(32.0f, 1.0f, 0.0f, 1.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("AllowRevert: %s | ErrorCode: %d"),
					OutRule.bAllowRevert ? TEXT("Yes") : TEXT("No"), OutRule.ErrorCode)))
				.Font(PGX::Font::Mono())
				.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
			];

		// EN: Allowed destinations / ES: Destinos permitidos
		if (OutRule.AllowedDestinations.Num() > 0)
		{
			FString AllowedStr;
			for (const FGameplayTag& DestTag : OutRule.AllowedDestinations)
			{
				if (!AllowedStr.IsEmpty()) AllowedStr += TEXT(", ");
				AllowedStr += DestTag.ToString();
			}

			Panel->AddSlot()
				.AutoHeight()
				.Padding(32.0f, 1.0f, 0.0f, 1.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(FString::Printf(TEXT("Allowed (%d): %s"),
						OutRule.AllowedDestinations.Num(), *AllowedStr)))
					.Font(PGX::Font::Mono())
					.ColorAndOpacity(FSlateColor(PGX::Semantic::Good))
					.AutoWrapText(true)
				];
		}
		else
		{
			Panel->AddSlot()
				.AutoHeight()
				.Padding(32.0f, 1.0f, 0.0f, 1.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("AllowedAll", "Allowed: (all — no whitelist)"))
					.Font(PGX::Font::Mono())
					.ColorAndOpacity(FSlateColor(PGX::Semantic::Good))
				];
		}

		// EN: Disallowed destinations / ES: Destinos prohibidos
		if (OutRule.DisallowedTagQueries.Num() > 0)
		{
			FString DisallowedStr;
			for (const FGameplayTag& VetoTag : OutRule.DisallowedTagQueries)
			{
				if (!DisallowedStr.IsEmpty()) DisallowedStr += TEXT(", ");
				DisallowedStr += VetoTag.ToString();
			}

			Panel->AddSlot()
				.AutoHeight()
				.Padding(32.0f, 1.0f, 0.0f, 1.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(FString::Printf(TEXT("Disallowed (%d): %s"),
						OutRule.DisallowedTagQueries.Num(), *DisallowedStr)))
					.Font(PGX::Font::Mono())
					.ColorAndOpacity(FSlateColor(PGX::Semantic::Error))
					.AutoWrapText(true)
				];
		}
	}
	else
	{
		Panel->AddSlot()
			.AutoHeight()
			.Padding(32.0f, 1.0f, 0.0f, 1.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("NoRule", "(no rule for current state)"))
				.Font(PGX::Font::Mono())
				.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
			];
	}

	// EN: Recent history (last 5) / ES: Historial reciente (ultimos 5)
	TArray<FPGXFlowHistoryEntry> History = Sub->GetChannelHistory(Summary->Channel);

	Panel->AddSlot()
		.AutoHeight()
		.Padding(20.0f, 6.0f, 0.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(FText::Format(LOCTEXT("HistHeader", "Recent History ({0} total):"), FText::AsNumber(History.Num())))
			.Font(PGX::Font::Badge())
			.ColorAndOpacity(FSlateColor(PGX::Semantic::Info))
		];

	const int32 ShowCount = FMath::Min(History.Num(), 5);
	for (int32 i = History.Num() - ShowCount; i < History.Num(); ++i)
	{
		const FPGXFlowHistoryEntry& Entry = History[i];
		const FString TimeStr = Entry.Timestamp.ToString(TEXT("%H:%M:%S.%s"));
		const FString TagStr = Entry.FlowTag.IsValid() ? Entry.FlowTag.ToString() : TEXT("None");

		Panel->AddSlot()
			.AutoHeight()
			.Padding(32.0f, 1.0f, 0.0f, 1.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("[%s] %s"), *TimeStr, *TagStr)))
				.Font(PGX::Font::Mono())
				.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
			];
	}

	// EN: Bottom padding / ES: Padding inferior
	Panel->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 4.0f)
		[
			SNew(SBox).HeightOverride(1.0f)
		];

	return Panel;
}

FLinearColor SPGXGameFlowChannelRow::GetChannelColor(EPGXFlowChannel Channel)
{
	switch (Channel)
	{
	case EPGXFlowChannel::Global:     return FLinearColor(1.0f, 0.84f, 0.0f);   // Gold
	case EPGXFlowChannel::UI:         return FLinearColor(0.53f, 0.81f, 0.98f);  // SkyBlue
	case EPGXFlowChannel::Characters: return FLinearColor(0.3f, 0.9f, 0.3f);     // Green
	case EPGXFlowChannel::AI:         return FLinearColor(1.0f, 0.45f, 0.2f);    // RedOrange
	case EPGXFlowChannel::Cameras:    return FLinearColor(0.7f, 0.4f, 0.9f);     // Purple
	case EPGXFlowChannel::Systems:    return FLinearColor(0.7f, 0.7f, 0.7f);     // Gray
	case EPGXFlowChannel::LevelLogic: return FLinearColor(0.0f, 0.8f, 0.7f);     // Teal
	case EPGXFlowChannel::Actors:     return FLinearColor(1.0f, 0.65f, 0.0f);    // Orange
	default:                          return FLinearColor::White;
	}
}

#undef LOCTEXT_NAMESPACE
