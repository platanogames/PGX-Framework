// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Details/FPGXActorCustomization.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Notifications/PGXEditorNotification.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "PGXActorDetails"

TSharedRef<IDetailCustomization> FPGXActorCustomization::MakeInstance()
{
	return MakeShareable(new FPGXActorCustomization);
}

void FPGXActorCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& PGXCategory = DetailBuilder.EditCategory(
		"PGX Data",
		LOCTEXT("PGXData", "PGX Data"),
		ECategoryPriority::Important
	);

	PGXCategory.AddCustomRow(LOCTEXT("PGXDataRow", "PGX Data"))
	.WholeRowContent()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("PGXActorInfo", "PGX Actor - Data binding available when Data Registry is active"))
			.Font(FAppStyle::GetFontStyle("SmallFont"))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f, 2.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("OpenDataAsset", "Open DataAsset"))
				.OnClicked_Lambda([]()
				{
					UPGXEditorNotification::ShowInfo(LOCTEXT("DataBindInDev", "PGX: Data Binding - In Development"));
					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("ReloadData", "Reload Data"))
				.OnClicked_Lambda([]()
				{
					UPGXEditorNotification::ShowInfo(LOCTEXT("ReloadInDev", "PGX: Data Reload - In Development"));
					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("Preview", "Preview"))
				.OnClicked_Lambda([]()
				{
					UPGXEditorNotification::ShowInfo(LOCTEXT("PreviewInDev", "PGX: Data Preview - In Development"));
					return FReply::Handled();
				})
			]
		]
	];
}

#undef LOCTEXT_NAMESPACE
