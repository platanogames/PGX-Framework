// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Details/FPGXDataAssetCustomization.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Notifications/PGXEditorNotification.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "PGXDataAssetDetails"

TSharedRef<IDetailCustomization> FPGXDataAssetCustomization::MakeInstance()
{
	return MakeShareable(new FPGXDataAssetCustomization);
}

void FPGXDataAssetCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& PGXCategory = DetailBuilder.EditCategory(
		"PGX",
		LOCTEXT("PGXCategory", "PGX Framework"),
		ECategoryPriority::Important
	);

	PGXCategory.AddCustomRow(LOCTEXT("PGXHeader", "PGX Header"))
	.WholeRowContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(4.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("PGXDataAsset", "PGX DataAsset"))
			.Font(FAppStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNullWidget::NullWidget
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(4.0f, 2.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Validate", "Validate"))
			.OnClicked_Lambda([]()
			{
				UPGXEditorNotification::ShowInfo(LOCTEXT("ValidateInDev", "PGX: Asset Validation - In Development"));
				return FReply::Handled();
			})
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(4.0f, 2.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("ResetDefaults", "Reset Defaults"))
			.OnClicked_Lambda([]()
			{
				UPGXEditorNotification::ShowInfo(LOCTEXT("ResetInDev", "PGX: Reset Defaults - In Development"));
				return FReply::Handled();
			})
		]
	];
}

#undef LOCTEXT_NAMESPACE
