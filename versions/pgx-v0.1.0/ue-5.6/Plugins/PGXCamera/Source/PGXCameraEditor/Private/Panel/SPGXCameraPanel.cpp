// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Panel/SPGXCameraPanel.h"

#include "PGXCameraConfig.h"
#include "PGXCameraSubsystem.h"
#include "Style/PGXVisualTokens.h"
#include "Widgets/SPGXPremiumShell.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Editor.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Brushes/SlateColorBrush.h"
#include "UObject/UnrealType.h"

#define LOCTEXT_NAMESPACE "PGXCamera"

void SPGXCameraPanel::Construct(const FArguments& InArgs)
{
	// EN: Widget-owned brushes — leak prevention per the panel ownership policy.
	SurfaceRaisedBrush = MakeShared<FSlateColorBrush>(PGX::Surface::Raised);

	ChildSlot
	[
		SNew(SPGXPremiumShell)
		.SystemColor(PGX::System::Camera)
		.Title(LOCTEXT("PanelTitle", "PGX Camera"))
		.Subtitle(LOCTEXT("PanelSubtitle", "Config assets + reflected schema preview"))
		.bShowFooter(false)
		.TitleRightContent()
		[
			SNew(SButton)
			.OnClicked(this, &SPGXCameraPanel::OnRefreshClicked)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("RefreshButton", "Refresh"))
				.Font(PGX::Font::Body())
			]
		]
		[
			SNew(SScrollBox)

			// Section A: Config DA inventory + schema introspection
			+ SScrollBox::Slot()
			.Padding(FMargin(0.0f, 0.0f, 0.0f, PGX::Spacing::SM))
			[
				SNew(SBorder)
				.BorderImage(SurfaceRaisedBrush.Get())
				.Padding(FMargin(PGX::Spacing::MD))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("ConfigHeader", "UPGXCameraConfig assets + reflected Fields"))
						.Font(PGX::Font::SectionHeader())
						.ColorAndOpacity(FSlateColor(PGX::Text::Primary))
					]
					+ SVerticalBox::Slot().AutoHeight()
					.Padding(FMargin(0.0f, PGX::Spacing::XS, 0.0f, 0.0f))
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(0.0f, 0.0f, PGX::Spacing::MD, 0.0f))
						[
							SAssignNew(ConfigCountText, STextBlock)
							.Font(PGX::Font::KPIValue())
							.ColorAndOpacity(FSlateColor(PGX::System::Camera))
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Bottom)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("ConfigCountLabel", " assets"))
							.Font(PGX::Font::KPILabel())
							.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
						]
					]
					+ SVerticalBox::Slot().AutoHeight()
					.Padding(FMargin(0.0f, PGX::Spacing::XS, 0.0f, 0.0f))
					[
						SAssignNew(FieldsCountText, STextBlock)
						.Font(PGX::Font::Body())
						.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
					]
				]
			]

			// Section B: Subsystem live state — selected mode exposed by UPGXCameraSubsystem.
			+ SScrollBox::Slot()
			.Padding(FMargin(0.0f, 0.0f, 0.0f, PGX::Spacing::SM))
			[
				SNew(SBorder)
				.BorderImage(SurfaceRaisedBrush.Get())
				.Padding(FMargin(PGX::Spacing::MD))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("SubsystemHeader", "Subsystem live state"))
						.Font(PGX::Font::SectionHeader())
						.ColorAndOpacity(FSlateColor(PGX::Text::Primary))
					]
					+ SVerticalBox::Slot().AutoHeight()
					.Padding(FMargin(0.0f, PGX::Spacing::XS, 0.0f, 0.0f))
					[
						SAssignNew(SubsystemStateText, STextBlock)
						.Text(LOCTEXT("SubsystemStatePending", "Camera subsystem state pending refresh."))
						.Font(PGX::Font::Body())
						.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
						.AutoWrapText(true)
					]
				]
			]

			// Section C: preview envelope demo + integration status
			+ SScrollBox::Slot()
			.Padding(FMargin(0.0f, 0.0f, 0.0f, PGX::Spacing::SM))
			[
				SNew(SBorder)
				.BorderImage(SurfaceRaisedBrush.Get())
				.Padding(FMargin(PGX::Spacing::MD))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("EnvelopeHeader", "preview envelope demo"))
						.Font(PGX::Font::SectionHeader())
						.ColorAndOpacity(FSlateColor(PGX::Text::Primary))
					]
					+ SVerticalBox::Slot().AutoHeight()
					.Padding(FMargin(0.0f, PGX::Spacing::XS, 0.0f, 0.0f))
					[
						SNew(SButton)
						.OnClicked(this, &SPGXCameraPanel::OnShowEnvelopeClicked)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("EnvelopeButton", "Show schema descriptor"))
							.Font(PGX::Font::Body())
						]
					]
					+ SVerticalBox::Slot().AutoHeight()
					.Padding(FMargin(0.0f, PGX::Spacing::SM, 0.0f, 0.0f))
					[
						SAssignNew(EnvelopeText, STextBlock)
						.Font(PGX::Font::Mono())
						.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
						.AutoWrapText(true)
					]
				]
			]
		]
	];

	RefreshState();
}

FReply SPGXCameraPanel::OnRefreshClicked()
{
	RefreshState();
	return FReply::Handled();
}

void SPGXCameraPanel::RefreshState()
{
	// EN: AssetRegistry inventory of UPGXCameraConfig + reflected UPROPERTY count
	// ES: Inventario AssetRegistry de UPGXCameraConfig + conteo UPROPERTY reflejado
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<FAssetData> Found;
	AssetRegistryModule.Get().GetAssetsByClass(UPGXCameraConfig::StaticClass()->GetClassPathName(), Found, /*bSearchSubClasses=*/true);

	int32 FieldsCount = 0;
	for (TFieldIterator<FProperty> PropertyIt(UPGXCameraConfig::StaticClass()); PropertyIt; ++PropertyIt)
	{
		++FieldsCount;
	}

	if (ConfigCountText.IsValid())
	{
		ConfigCountText->SetText(FText::AsNumber(Found.Num()));
	}
	if (FieldsCountText.IsValid())
	{
		FieldsCountText->SetText(FText::Format(
			LOCTEXT("FieldsCountFmt", "Schema reflected Fields: {0} (UPROPERTY count of UPGXCameraConfig static class)"),
			FText::AsNumber(FieldsCount)));
	}

	if (SubsystemStateText.IsValid())
	{
		UWorld* EditorWorld = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
		const UPGXCameraSubsystem* CameraSubsystem = EditorWorld ? EditorWorld->GetSubsystem<UPGXCameraSubsystem>() : nullptr;
		const FName ActiveModeName = CameraSubsystem ? CameraSubsystem->GetActiveCameraModeName() : NAME_None;

		SubsystemStateText->SetText(FText::Format(
			LOCTEXT("SubsystemStateFmt", "UPGXCameraSubsystem API available. Active mode: {0}. Note: this surface reports selected mode state; camera blend/transition execution remains a future runtime layer."),
			FText::FromName(ActiveModeName)));
	}
}

FReply SPGXCameraPanel::OnShowEnvelopeClicked()
{
	// EN: REFLECTION-BASED schema descriptor preview (NOT canonical preview envelope).
	//     This panel does NOT call IPGXObservable::ToJson() — UPGXCameraConfig has not adopted
	//     the interface yet (split to a separate observable integration). The output below is
	//     a human-readable schema preview built from TFieldIterator (UPROPERTY name + CPPType);
	//     it is *intentionally NOT valid JSON* and is labelled as such to avoid misuse as canonical
	//     envelope. This panel exposes reflection metadata only.
	// ES: Vista previa schema basada en reflection (NO envelope preview canonico). Panel NO llama
	//     IPGXObservable::ToJson() — UPGXCameraConfig pendiente adopcion (separate observable integration).
	//     Output abajo es preview legible de schema desde TFieldIterator — *NO es JSON valido*.
	FString Summary = TEXT("[reflection-based schema preview — NOT valid JSON, NOT canonical preview envelope]\n\n");
	Summary += TEXT("Schema source: UPGXCameraConfig (reflection only; IPGXObservable is unavailable)\n");
	Summary += TEXT("Displayed version: 1.0   Plugin: PGXCameraRuntime\n\n");
	Summary += TEXT("Reflected UPROPERTY fields (name -> CPP type):\n");
	for (TFieldIterator<FProperty> PropertyIt(UPGXCameraConfig::StaticClass()); PropertyIt; ++PropertyIt)
	{
		FProperty* Property = *PropertyIt;
		if (!Property) { continue; }
		Summary += FString::Printf(TEXT("  - %s : %s\n"), *Property->GetName(), *Property->GetCPPType());
	}
	Summary += TEXT("\n[When IPGXObservable integration is available, this preview is replaced by Config->ToJson() canonical envelope output.]");

	if (EnvelopeText.IsValid())
	{
		EnvelopeText->SetText(FText::FromString(Summary));
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
