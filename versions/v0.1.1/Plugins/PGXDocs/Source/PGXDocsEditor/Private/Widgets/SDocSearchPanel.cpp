// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Widgets/SDocSearchPanel.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Style/PGXVisualTokens.h"

#define LOCTEXT_NAMESPACE "PGXDocs"

void SDocSearchPanel::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBox)
		.MaxDesiredHeight(300.0f)
		[
			SAssignNew(ResultsList, SListView<TSharedPtr<FDocSearchResult>>)
			.ListItemsSource(&ResultItems)
			.OnGenerateRow(this, &SDocSearchPanel::OnGenerateResultRow)
			.SelectionMode(ESelectionMode::Single)
		]
	];
}

void SDocSearchPanel::SetResults(const TArray<FDocSearchResult>& Results)
{
	ResultItems.Empty();
	for (const FDocSearchResult& Result : Results)
	{
		ResultItems.Add(MakeShared<FDocSearchResult>(Result));
	}

	if (ResultsList.IsValid())
	{
		ResultsList->RequestListRefresh();
	}
}

void SDocSearchPanel::ClearResults()
{
	ResultItems.Empty();
	if (ResultsList.IsValid())
	{
		ResultsList->RequestListRefresh();
	}
}

TSharedRef<ITableRow> SDocSearchPanel::OnGenerateResultRow(TSharedPtr<FDocSearchResult> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FDocSearchResult>>, OwnerTable)
		.Padding(FMargin(4.0f, 2.0f))
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Item->Title))
					.Font(PGX::Font::SubHeader())
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(STextBlock)
					.Text(FText::FromString(FString::Printf(TEXT("[%d]"), Item->Score)))
					.Font(PGX::Font::BodySmall())
					.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->MatchContext))
				.Font(PGX::Font::BodySmall())
				.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
				.AutoWrapText(true)
			]
		];
}

#undef LOCTEXT_NAMESPACE
