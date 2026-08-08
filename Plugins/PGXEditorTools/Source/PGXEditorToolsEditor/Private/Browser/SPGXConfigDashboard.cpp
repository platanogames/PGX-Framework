// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Browser/SPGXConfigDashboard.h"
#include "PGXEditorToolsEditor.h"
#include "Logging/PGXLogMacros.h"
#include "Data/PGXConfigDataAsset.h"

// EN: Settings classes for slot scanning / ES: Clases Settings para escaneo de slots
#include "Messages/PGXMessageSettings.h"
#include "EventHandler/PGXEventHandlerSettings.h"
#include "Logging/PGXLogSettings.h"
#include "Profile/PGXProfileSettings.h"
#include "Registry/PGXRegistrySettings.h"
#include "PGXLoadingSettings.h"
#include "PGXLevelFlowSettings.h"
#include "PGXMGOSSettings.h"
#include "PGXGameFlowSettings.h"
#include "PGXSaveSettings.h"
#include "PGXAudioSettings.h"
#include "PGXPSOSettings.h"

#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "Utils/PGXEditorUtils.h"
#include "Style/PGXVisualTokens.h"
#include "Widgets/SPGXPremiumShell.h"
#include "Widgets/SPGXSectionDivider.h"
#include "Widgets/SPGXStatusBadge.h"
#include "Style/PGXEditorStyle.h"
#include "Widgets/SPGXEmptyStateV2.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "ISettingsModule.h"
#include "Editor.h"

#define LOCTEXT_NAMESPACE "PGXConfigDashboard"

// EN: Config Dashboard accent color (Green) — via PGX Visual Tokens / ES: Color de acento del Config Dashboard (Verde) — via PGX Visual Tokens
static const FLinearColor GConfigColor = PGX::System::Config;

// ============================================================================
// EN: Construct / Destruct
// ES: Construccion / Destruccion
// ============================================================================

void SPGXConfigDashboard::Construct(const FArguments& /*InArgs*/)
{
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[ConfigDashboard] Construct"));

	BindAssetRegistryDelegates();

	ChildSlot
	[
		SNew(SPGXPremiumShell)
		.SystemColor(PGX::System::Config)
		.Title(LOCTEXT("PanelTitle", "CONFIG DASHBOARD"))
		.Subtitle(LOCTEXT("PanelSubtitle", "Config Resolution Status"))
		.Icon(FPGXEditorStyle::Get().GetBrush("PGXEditor.Icon.ConfigDashboard"))
		.bShowFooter(true)
		.StatusText(this, &SPGXConfigDashboard::GetFooterStatusText)
		.StatusColor(this, &SPGXConfigDashboard::GetFooterStatusColor)
		.TitleRightContent()
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("ToggleView", "Toggle View"))
				.OnClicked(this, &SPGXConfigDashboard::OnToggleViewClicked)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("Refresh", "Refresh"))
				.OnClicked(this, &SPGXConfigDashboard::OnRefreshClicked)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("ValidateAll", "Validate All"))
				.OnClicked(this, &SPGXConfigDashboard::OnValidateAllClicked)
			]
		]
		.Content()
		[
			SNew(SScrollBox)

			+ SScrollBox::Slot()
			.Padding(8.0f)
			[
				SAssignNew(ContentBox, SVerticalBox)
			]
		]
	];

	ScanSettingsSlots();
	ScanConfigAssets();
	RebuildView();
}

SPGXConfigDashboard::~SPGXConfigDashboard()
{
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[ConfigDashboard] Destructor — cleaning up"));
	UnbindAssetRegistryDelegates();
}

// ============================================================================
// EN: Footer Attribute Getters / ES: Getters de Atributos del Footer
// ============================================================================

FText SPGXConfigDashboard::GetFooterStatusText() const
{
	if (CurrentView == EDashboardView::SettingsSlots)
	{
		int32 Configured = 0;
		for (const FSettingsSlotEntry& Slot : SettingsSlots)
		{
			if (Slot.bConfigSet) { Configured++; }
		}
		return FText::Format(
			LOCTEXT("SlotFooter", "Settings Slots: {0}/{1} configured"),
			FText::AsNumber(Configured),
			FText::AsNumber(SettingsSlots.Num()));
	}

	if (ErrorCount > 0)
	{
		return FText::Format(
			LOCTEXT("StatusFmtErr", "Total: {0} configs | {1} warnings | {2} errors"),
			FText::AsNumber(ConfigEntries.Num()),
			FText::AsNumber(WarningCount),
			FText::AsNumber(ErrorCount));
	}
	if (WarningCount > 0)
	{
		return FText::Format(
			LOCTEXT("StatusFmtWarn", "Total: {0} configs | {1} warnings"),
			FText::AsNumber(ConfigEntries.Num()),
			FText::AsNumber(WarningCount));
	}
	return FText::Format(
		LOCTEXT("StatusFmt", "Total: {0} configs"),
		FText::AsNumber(ConfigEntries.Num()));
}

FLinearColor SPGXConfigDashboard::GetFooterStatusColor() const
{
	if (CurrentView == EDashboardView::SettingsSlots)
	{
		for (const FSettingsSlotEntry& Slot : SettingsSlots)
		{
			if (!Slot.bConfigSet) { return PGX::Semantic::Warn; }
		}
		return PGX::Semantic::Good;
	}

	if (ErrorCount > 0) { return PGX::Semantic::Error; }
	if (WarningCount > 0) { return PGX::Semantic::Warn; }
	return PGX::Text::Muted;
}

// ============================================================================
// EN: View Switching / ES: Cambio de Vista
// ============================================================================

void SPGXConfigDashboard::RebuildView()
{
	if (!ContentBox.IsValid()) { return; }
	ContentBox->ClearChildren();

	if (CurrentView == EDashboardView::SettingsSlots)
	{
		BuildSettingsSlotsView();
	}
	else
	{
		BuildAllDAsView();
	}
}

// ============================================================================
// EN: Settings Slots View (Primary) / ES: Vista de Slots de Settings (Primaria)
// ============================================================================

void SPGXConfigDashboard::BuildSettingsSlotsView()
{
	// EN: Section header / ES: Encabezado de seccion
	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 8.0f)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("SlotsHeader", "Project Settings > PGX — Config Slots"))
		.Font(PGX::Font::PanelTitle())
		.ColorAndOpacity(PGX::Text::Primary)
	];

	if (SettingsSlots.Num() == 0)
	{
		ContentBox->AddSlot()
		.AutoHeight()
		.Padding(4.0f)
		[
			SNew(SPGXEmptyStateV2)
			.Message(LOCTEXT("NoSlots", "No Settings slots scanned"))
			.Hint(LOCTEXT("NoSlotsHint", "Click Refresh"))
		];
		return;
	}

	for (const FSettingsSlotEntry& Slot : SettingsSlots)
	{
		const FLinearColor ConfigBadgeColor = Slot.bConfigSet ? PGX::Semantic::Good : PGX::Semantic::Warn;
		const FText ConfigStatusText = Slot.bConfigSet
			? FText::FromString(Slot.ConfigDAName)
			: LOCTEXT("AutoDiscovery", "Auto-discovery (deprecated)");

		ContentBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 2.0f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
			.Padding(FMargin(8.0f, 6.0f))
			[
				SNew(SHorizontalBox)

				// EN: System color indicator / ES: Indicador de color del sistema
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(SBox)
					.WidthOverride(4.0f)
					.HeightOverride(24.0f)
					[
						SNew(SBorder)
						.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
						.BorderBackgroundColor(Slot.SystemColor)
					]
				]

				// EN: System name / ES: Nombre del sistema
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 12.0f, 0.0f)
				[
					SNew(SBox)
					.WidthOverride(120.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(Slot.SystemName))
						.Font(PGX::Font::SubHeader())
						.ColorAndOpacity(PGX::Text::Primary)
					]
				]

				// EN: Config slot status / ES: Estado del slot de config
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 4.0f, 0.0f)
				[
					SNew(SPGXStatusBadge)
					.Shape(EPGXBadgeShape::Dot)
					.Color(ConfigBadgeColor)
				]

				+ SHorizontalBox::Slot()
				.FillWidth(0.5f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(ConfigStatusText)
					.ColorAndOpacity(Slot.bConfigSet ? PGX::Text::Secondary : PGX::Semantic::Warn)
				]

				// EN: DataTable slot status (if applicable) / ES: Estado del slot de DataTable
				+ SHorizontalBox::Slot()
				.FillWidth(0.3f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(Slot.bTableSet
						? FText::Format(LOCTEXT("TableSetFmt", "Table: {0} ({1} rows)"),
							FText::FromString(Slot.TableDAName), FText::AsNumber(Slot.TableRowCount))
						: FText::GetEmpty())
					.ColorAndOpacity(PGX::Text::Muted)
				]

				// EN: Open Settings button / ES: Boton Abrir Settings
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("OpenSettings", "Settings"))
					.ContentPadding(FMargin(4.0f, 1.0f))
					.OnClicked_Lambda([]()
					{
						FModuleManager::LoadModuleChecked<ISettingsModule>("Settings")
							.ShowViewer("Project", "PGX", "");
						return FReply::Handled();
					})
				]
			]
		];
	}
}

// ============================================================================
// EN: All DAs View (Secondary) / ES: Vista All DAs (Secundaria)
// ============================================================================

void SPGXConfigDashboard::BuildAllDAsView()
{
	// EN: Section header / ES: Encabezado de seccion
	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 8.0f)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("AllDAsHeader", "All Config DataAssets"))
		.Font(PGX::Font::PanelTitle())
		.ColorAndOpacity(PGX::Text::Primary)
	];

	if (ConfigEntries.Num() == 0)
	{
		ContentBox->AddSlot()
		.AutoHeight()
		.Padding(4.0f)
		[
			SNew(SPGXEmptyStateV2)
			.Message(LOCTEXT("NoConfigs", "No Config DataAssets found in project"))
			.Hint(LOCTEXT("NoConfigsHint", "Create a Config DA from Content Browser > PGX"))
		];
		return;
	}

	// EN: Sort by SystemGroup then ClassName / ES: Ordenar por SystemGroup luego ClassName
	ConfigEntries.Sort([](const FConfigEntry& A, const FConfigEntry& B)
	{
		if (A.SystemGroup != B.SystemGroup) return A.SystemGroup < B.SystemGroup;
		return A.ClassName < B.ClassName;
	});

	MarkReferencedDAs();

	// EN: Group by system / ES: Agrupar por sistema
	FString CurrentGroup;
	for (const FConfigEntry& Entry : ConfigEntries)
	{
		if (Entry.SystemGroup != CurrentGroup)
		{
			CurrentGroup = Entry.SystemGroup;
			const FLinearColor GroupColor = GetSystemGroupColor(CurrentGroup);

			ContentBox->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 8.0f, 0.0f, 2.0f)
			[
				SNew(SPGXSectionDivider)
				.Title(FText::FromString(CurrentGroup))
				.AccentColor(GroupColor)
			];
		}

		// EN: Validation + reference indicator / ES: Indicador de validacion + referencia
		FLinearColor IndicatorColor;
		switch (Entry.Validation)
		{
		case EValidationResult::OK:      IndicatorColor = PGX::Semantic::Good; break;
		case EValidationResult::Warning: IndicatorColor = PGX::Semantic::Warn; break;
		case EValidationResult::Error:   IndicatorColor = PGX::Semantic::Error; break;
		default:                          IndicatorColor = PGX::Semantic::Neutral; break;
		}

		FAssetData CapturedAssetData = Entry.AssetData;
		const FString CapturedValidMsg = Entry.ValidationMessage;
		const bool bReferenced = Entry.bReferencedBySettings;

		ContentBox->AddSlot()
		.AutoHeight()
		.Padding(16.0f, 2.0f, 4.0f, 2.0f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
			.Padding(FMargin(8.0f, 4.0f))
			[
				SNew(SHorizontalBox)

				// EN: Validation indicator / ES: Indicador de validacion
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 6.0f, 0.0f)
				[
					SNew(SBox)
					.ToolTipText(FText::FromString(CapturedValidMsg.IsEmpty() ? TEXT("Not validated") : CapturedValidMsg))
					[
						SNew(SPGXStatusBadge)
						.Shape(EPGXBadgeShape::Dot)
						.Color(IndicatorColor)
					]
				]

				// EN: Class name / ES: Nombre de clase
				+ SHorizontalBox::Slot()
				.FillWidth(0.25f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Entry.ClassName))
					.Font(PGX::Font::Body())
				]

				// EN: Asset name / ES: Nombre de asset
				+ SHorizontalBox::Slot()
				.FillWidth(0.3f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Entry.AssetName))
					.ColorAndOpacity(PGX::Text::Secondary)
				]

				// EN: Reference badge / ES: Badge de referencia
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4.0f, 0.0f)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
					.Padding(FMargin(6.0f, 2.0f))
					[
						SNew(STextBlock)
						.Text(bReferenced
							? LOCTEXT("BadgeConfigured", "CONFIGURED")
							: LOCTEXT("BadgeUnreferenced", "UNREFERENCED"))
						.Font(PGX::Font::CaptionBold())
						.ColorAndOpacity(bReferenced ? PGX::Semantic::Good : PGX::Semantic::Warn)
					]
				]

				// EN: Browse button / ES: Boton Browse
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("Browse", "Browse"))
					.ContentPadding(FMargin(4.0f, 1.0f))
					.OnClicked_Lambda([CapturedAssetData]()
					{
						if (CapturedAssetData.IsValid())
						{
							TArray<FAssetData> Assets;
							Assets.Add(CapturedAssetData);
							FContentBrowserModule& CBModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
							CBModule.Get().SyncBrowserToAssets(Assets);
						}
						return FReply::Handled();
					})
				]

				// EN: Edit button / ES: Boton de edicion
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("Edit", "Edit"))
					.ContentPadding(FMargin(4.0f, 1.0f))
					.OnClicked_Lambda([CapturedAssetData]()
					{
						UObject* Asset = CapturedAssetData.GetAsset();
						if (Asset && GEditor)
						{
							GEditor->EditObject(Asset);
						}
						return FReply::Handled();
					})
				]
			]
		];
	}
}

// ============================================================================
// EN: Actions / ES: Acciones
// ============================================================================

FReply SPGXConfigDashboard::OnRefreshClicked()
{
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[ConfigDashboard] Manual refresh"));
	ScanSettingsSlots();
	ScanConfigAssets();
	RebuildView();
	return FReply::Handled();
}

FReply SPGXConfigDashboard::OnValidateAllClicked()
{
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[ConfigDashboard] Validate All clicked"));
	WarningCount = 0;
	ErrorCount = 0;

	for (FConfigEntry& Entry : ConfigEntries)
	{
		ValidateEntry(Entry);
		if (Entry.Validation == EValidationResult::Warning) WarningCount++;
		if (Entry.Validation == EValidationResult::Error) ErrorCount++;
	}

	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[ConfigDashboard] Validation complete: %d configs, %d warnings, %d errors"),
		ConfigEntries.Num(), WarningCount, ErrorCount);

	RebuildView();
	return FReply::Handled();
}

FReply SPGXConfigDashboard::OnToggleViewClicked()
{
	CurrentView = (CurrentView == EDashboardView::SettingsSlots)
		? EDashboardView::AllDAs
		: EDashboardView::SettingsSlots;

	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[ConfigDashboard] Toggled to %s view"),
		CurrentView == EDashboardView::SettingsSlots ? TEXT("Settings Slots") : TEXT("All DAs"));

	RebuildView();
	return FReply::Handled();
}

void SPGXConfigDashboard::ValidateEntry(FConfigEntry& Entry)
{
	UObject* Asset = Entry.AssetData.GetAsset();
	if (!Asset)
	{
		Entry.Validation = EValidationResult::Error;
		Entry.ValidationMessage = TEXT("Asset failed to load");
		return;
	}

	UPGXConfigDataAsset* ConfigDA = Cast<UPGXConfigDataAsset>(Asset);
	if (ConfigDA)
	{
		if (ConfigDA->ConfigDisplayName.IsEmpty())
		{
			Entry.Validation = EValidationResult::Warning;
			Entry.ValidationMessage = TEXT("ConfigDisplayName is empty");
			return;
		}
	}

	Entry.Validation = EValidationResult::OK;
	Entry.ValidationMessage = TEXT("OK");
}

// ============================================================================
// EN: Settings Slot Scanning / ES: Escaneo de Slots de Settings
// ============================================================================

void SPGXConfigDashboard::ScanSettingsSlots()
{
	SettingsSlots.Empty();

	// EN: Scan each system's Settings class for config slot status
	// ES: Escanear la clase Settings de cada sistema para estado de slot de config

	// ── Profile (gold standard — always has Settings) ──
	{
		const auto* S = GetDefault<UPGXProfileSettings>();
		FSettingsSlotEntry Slot;
		Slot.SystemName = TEXT("Profile");
		Slot.SystemColor = PGX::System::Profile;
		Slot.bConfigSet = !S->ActiveProfile.IsNull();
		if (Slot.bConfigSet) { Slot.ConfigDAName = S->ActiveProfile.GetAssetName(); }
		SettingsSlots.Add(MoveTemp(Slot));
	}

	// ── Message ──
	{
		const auto* S = GetDefault<UPGXMessageSettings>();
		FSettingsSlotEntry Slot;
		Slot.SystemName = TEXT("Message");
		Slot.SystemColor = PGX::System::Message;
		Slot.bConfigSet = !S->ActiveConfig.IsNull();
		if (Slot.bConfigSet) { Slot.ConfigDAName = S->ActiveConfig.GetAssetName(); }
		SettingsSlots.Add(MoveTemp(Slot));
	}

	// ── EventHandler ──
	{
		const auto* S = GetDefault<UPGXEventHandlerSettings>();
		FSettingsSlotEntry Slot;
		Slot.SystemName = TEXT("EventHandler");
		Slot.SystemColor = PGX::System::EventHandler;
		Slot.bConfigSet = !S->ActiveConfig.IsNull();
		if (Slot.bConfigSet) { Slot.ConfigDAName = S->ActiveConfig.GetAssetName(); }
		SettingsSlots.Add(MoveTemp(Slot));
	}

	// ── Log ──
	{
		const auto* S = GetDefault<UPGXLogSettings>();
		FSettingsSlotEntry Slot;
		Slot.SystemName = TEXT("Log");
		Slot.SystemColor = PGX::System::Log;
		Slot.bConfigSet = !S->ActiveConfig.IsNull();
		if (Slot.bConfigSet) { Slot.ConfigDAName = S->ActiveConfig.GetAssetName(); }
		SettingsSlots.Add(MoveTemp(Slot));
	}

	// ── GameFlow ──
	{
		const auto* S = GetDefault<UPGXGameFlowSettings>();
		FSettingsSlotEntry Slot;
		Slot.SystemName = TEXT("GameFlow");
		Slot.SystemColor = PGX::System::GameFlow;
		Slot.bConfigSet = !S->ActiveConfig.IsNull();
		if (Slot.bConfigSet) { Slot.ConfigDAName = S->ActiveConfig.GetAssetName(); }
		Slot.bTableSet = !S->FlowRulesTable.IsNull();
		if (Slot.bTableSet) { Slot.TableDAName = S->FlowRulesTable.GetAssetName(); }
		SettingsSlots.Add(MoveTemp(Slot));
	}

	// ── Save ──
	{
		const auto* S = GetDefault<UPGXSaveSettings>();
		FSettingsSlotEntry Slot;
		Slot.SystemName = TEXT("Save");
		Slot.SystemColor = PGX::System::Save;
		Slot.bTableSet = !S->SaveContextTable.IsNull();
		if (Slot.bTableSet) { Slot.TableDAName = S->SaveContextTable.GetAssetName(); Slot.bConfigSet = true; Slot.ConfigDAName = TEXT("via DataTable"); }
		SettingsSlots.Add(MoveTemp(Slot));
	}

	// ── Loading ──
	{
		const auto* S = GetDefault<UPGXLoadingSettings>();
		FSettingsSlotEntry Slot;
		Slot.SystemName = TEXT("Loading");
		Slot.SystemColor = PGX::System::Loading;
		Slot.bConfigSet = !S->ActiveConfig.IsNull();
		if (Slot.bConfigSet) { Slot.ConfigDAName = S->ActiveConfig.GetAssetName(); }
		Slot.bTableSet = !S->LoadingProfileTable.IsNull();
		if (Slot.bTableSet) { Slot.TableDAName = S->LoadingProfileTable.GetAssetName(); }
		SettingsSlots.Add(MoveTemp(Slot));
	}

	// ── LevelFlow ──
	{
		const auto* S = GetDefault<UPGXLevelFlowSettings>();
		FSettingsSlotEntry Slot;
		Slot.SystemName = TEXT("LevelFlow");
		Slot.SystemColor = PGX::System::LevelFlow;
		Slot.bConfigSet = !S->ActiveConfig.IsNull();
		if (Slot.bConfigSet) { Slot.ConfigDAName = S->ActiveConfig.GetAssetName(); }
		Slot.bTableSet = !S->LevelCatalogTable.IsNull();
		if (Slot.bTableSet) { Slot.TableDAName = S->LevelCatalogTable.GetAssetName(); }
		SettingsSlots.Add(MoveTemp(Slot));
	}

	// ── PSO ──
	{
		const auto* S = GetDefault<UPGXPSOSettings>();
		FSettingsSlotEntry Slot;
		Slot.SystemName = TEXT("PSO");
		Slot.SystemColor = PGX::System::PSO;
		Slot.bTableSet = !S->PSOConfigTable.IsNull();
		if (Slot.bTableSet) { Slot.TableDAName = S->PSOConfigTable.GetAssetName(); Slot.bConfigSet = true; Slot.ConfigDAName = TEXT("via DataTable"); }
		SettingsSlots.Add(MoveTemp(Slot));
	}

	// ── MGOS ──
	{
		const auto* S = GetDefault<UPGXMGOSSettings>();
		FSettingsSlotEntry Slot;
		Slot.SystemName = TEXT("MGOS");
		Slot.SystemColor = PGX::System::MGOS;
		Slot.bConfigSet = !S->ActiveConfig.IsNull();
		if (Slot.bConfigSet) { Slot.ConfigDAName = S->ActiveConfig.GetAssetName(); }
		SettingsSlots.Add(MoveTemp(Slot));
	}

	// ── Audio ──
	{
		const auto* S = GetDefault<UPGXAudioSettings>();
		FSettingsSlotEntry Slot;
		Slot.SystemName = TEXT("Audio");
		Slot.SystemColor = PGX::System::Audio;
		Slot.bConfigSet = !S->ActiveConfig.IsNull();
		if (Slot.bConfigSet) { Slot.ConfigDAName = S->ActiveConfig.GetAssetName(); }
		Slot.bTableSet = !S->ChannelTable.IsNull() || !S->ProfileTable.IsNull();
		if (Slot.bTableSet) { Slot.TableDAName = TEXT("Channel/Profile"); }
		SettingsSlots.Add(MoveTemp(Slot));
	}

	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[ConfigDashboard] Scanned %d settings slots"), SettingsSlots.Num());
}

// ============================================================================
// EN: Mark Referenced DAs / ES: Marcar DAs Referenciados
// ============================================================================

void SPGXConfigDashboard::MarkReferencedDAs()
{
	// EN: Collect all asset names referenced by Settings
	// ES: Recolectar todos los nombres de asset referenciados por Settings
	TSet<FString> ReferencedNames;

	auto AddIfSet = [&ReferencedNames](const auto& SoftRef)
	{
		if (!SoftRef.IsNull()) { ReferencedNames.Add(SoftRef.GetAssetName()); }
	};

	AddIfSet(GetDefault<UPGXProfileSettings>()->ActiveProfile);
	AddIfSet(GetDefault<UPGXMessageSettings>()->ActiveConfig);
	AddIfSet(GetDefault<UPGXEventHandlerSettings>()->ActiveConfig);
	AddIfSet(GetDefault<UPGXLogSettings>()->ActiveConfig);
	AddIfSet(GetDefault<UPGXGameFlowSettings>()->ActiveConfig);
	AddIfSet(GetDefault<UPGXLoadingSettings>()->ActiveConfig);
	AddIfSet(GetDefault<UPGXLevelFlowSettings>()->ActiveConfig);
	AddIfSet(GetDefault<UPGXMGOSSettings>()->ActiveConfig);
	AddIfSet(GetDefault<UPGXAudioSettings>()->ActiveConfig);

	for (FConfigEntry& Entry : ConfigEntries)
	{
		Entry.bReferencedBySettings = ReferencedNames.Contains(Entry.AssetName);
	}
}

// ============================================================================
// EN: Auto-Refresh via AssetRegistry / ES: Auto-Refresco via AssetRegistry
// ============================================================================

void SPGXConfigDashboard::BindAssetRegistryDelegates()
{
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
	OnAssetAddedHandle = AssetRegistry.OnAssetAdded().AddSP(SharedThis(this), &SPGXConfigDashboard::OnAssetAdded);
	OnAssetRemovedHandle = AssetRegistry.OnAssetRemoved().AddSP(SharedThis(this), &SPGXConfigDashboard::OnAssetRemoved);
	OnAssetRenamedHandle = AssetRegistry.OnAssetRenamed().AddSP(SharedThis(this), &SPGXConfigDashboard::OnAssetRenamed);
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[ConfigDashboard] Bound 3 AssetRegistry delegates"));
}

void SPGXConfigDashboard::UnbindAssetRegistryDelegates()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetRegistry"))
	{
		IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
		AssetRegistry.OnAssetAdded().Remove(OnAssetAddedHandle);
		AssetRegistry.OnAssetRemoved().Remove(OnAssetRemovedHandle);
		AssetRegistry.OnAssetRenamed().Remove(OnAssetRenamedHandle);
	}
	OnAssetAddedHandle.Reset();
	OnAssetRemovedHandle.Reset();
	OnAssetRenamedHandle.Reset();
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[ConfigDashboard] Unbound AssetRegistry delegates"));
}

void SPGXConfigDashboard::OnAssetAdded(const FAssetData& AssetData)
{
	if (IsConfigAsset(AssetData))
	{
		PGX_LOG_INFO(LogPGXEditorTools, TEXT("[ConfigDashboard] Config asset added: %s — auto-refreshing"), *AssetData.AssetName.ToString());
		ScanSettingsSlots();
		ScanConfigAssets();
		RebuildView();
	}
}

void SPGXConfigDashboard::OnAssetRemoved(const FAssetData& AssetData)
{
	if (IsConfigAsset(AssetData))
	{
		PGX_LOG_INFO(LogPGXEditorTools, TEXT("[ConfigDashboard] Config asset removed: %s — auto-refreshing"), *AssetData.AssetName.ToString());
		ScanSettingsSlots();
		ScanConfigAssets();
		RebuildView();
	}
}

void SPGXConfigDashboard::OnAssetRenamed(const FAssetData& AssetData, const FString& /*OldPath*/)
{
	if (IsConfigAsset(AssetData))
	{
		PGX_LOG_INFO(LogPGXEditorTools, TEXT("[ConfigDashboard] Config asset renamed: %s — auto-refreshing"), *AssetData.AssetName.ToString());
		ScanSettingsSlots();
		ScanConfigAssets();
		RebuildView();
	}
}

bool SPGXConfigDashboard::IsConfigAsset(const FAssetData& AssetData) const
{
	const FTopLevelAssetPath ClassPath = AssetData.AssetClassPath;
	const FString ClassName = ClassPath.GetAssetName().ToString();
	return ClassName.Contains(TEXT("PGXConfig")) || ClassName.Contains(TEXT("Config"));
}

// ============================================================================
// EN: Data Scan / ES: Escaneo de Datos
// ============================================================================

void SPGXConfigDashboard::ScanConfigAssets()
{
	ConfigEntries.Empty();
	WarningCount = 0;
	ErrorCount = 0;

	FAssetRegistryModule& RegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = RegistryModule.Get();

	TArray<FAssetData> AssetList;
	AssetRegistry.GetAssetsByClass(UPGXConfigDataAsset::StaticClass()->GetClassPathName(), AssetList, true);

	for (const FAssetData& AssetData : AssetList)
	{
		FConfigEntry Entry;
		Entry.AssetName = AssetData.AssetName.ToString();

		FString ClassPath = AssetData.AssetClassPath.ToString();
		int32 DotIndex;
		if (ClassPath.FindLastChar(TEXT('.'), DotIndex))
		{
			Entry.ClassName = ClassPath.Mid(DotIndex + 1);
		}
		else
		{
			Entry.ClassName = ClassPath;
		}

		Entry.SystemGroup = InferSystemGroup(Entry.ClassName);
		Entry.AssetData = AssetData;

		ConfigEntries.Add(MoveTemp(Entry));
	}

	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[ConfigDashboard] Scanned %d config assets"), ConfigEntries.Num());
}

FString SPGXConfigDashboard::InferSystemGroup(const FString& ClassName) const
{
	if (ClassName.Contains(TEXT("Audio")) || ClassName.Contains(TEXT("Channel")) ||
		ClassName.Contains(TEXT("Ducking")) || ClassName.Contains(TEXT("Music")) ||
		ClassName.Contains(TEXT("Sound")))
		return TEXT("Audio");

	if (ClassName.Contains(TEXT("GameMode")) || ClassName.Contains(TEXT("Construction")) ||
		ClassName.Contains(TEXT("PlayerC")) || ClassName.Contains(TEXT("HUD")) ||
		ClassName.Contains(TEXT("GameState")) || ClassName.Contains(TEXT("PlayerState")) ||
		ClassName.Contains(TEXT("Pawn")))
		return TEXT("Construction");

	if (ClassName.Contains(TEXT("Platform")))  return TEXT("Profile");
	if (ClassName.Contains(TEXT("Profile")))   return TEXT("Profile");
	if (ClassName.Contains(TEXT("Save")))      return TEXT("Save");
	if (ClassName.Contains(TEXT("GameFlow")))  return TEXT("GameFlow");
	if (ClassName.Contains(TEXT("PSO")))       return TEXT("PSO");
	if (ClassName.Contains(TEXT("Loading")))   return TEXT("Loading");
	if (ClassName.Contains(TEXT("LevelFlow")) || ClassName.Contains(TEXT("Level")))
		return TEXT("LevelFlow");
	if (ClassName.Contains(TEXT("Log")))       return TEXT("Log");
	if (ClassName.Contains(TEXT("MGOS")) || ClassName.Contains(TEXT("GCObserver")))
		return TEXT("MGOS");
	if (ClassName.Contains(TEXT("Registry")))  return TEXT("Data Registry");
	if (ClassName.Contains(TEXT("Message")))   return TEXT("Message");
	if (ClassName.Contains(TEXT("EventHandler")))
		return TEXT("EventHandler");

	return TEXT("General");
}

FLinearColor SPGXConfigDashboard::GetSystemGroupColor(const FString& Group)
{
	if (Group == TEXT("Audio"))          return PGX::System::Audio;
	if (Group == TEXT("Construction"))   return PGX::System::Construction;
	if (Group == TEXT("Profile"))        return PGX::System::Profile;
	if (Group == TEXT("Save"))           return PGX::System::Save;
	if (Group == TEXT("GameFlow"))       return PGX::System::GameFlow;
	if (Group == TEXT("PSO"))            return PGX::System::PSO;
	if (Group == TEXT("Loading"))        return PGX::System::Loading;
	if (Group == TEXT("LevelFlow"))      return PGX::System::LevelFlow;
	if (Group == TEXT("Log"))            return PGX::System::Log;
	if (Group == TEXT("MGOS"))           return PGX::System::MGOS;
	if (Group == TEXT("Data Registry"))  return PGX::System::DataRegistry;
	if (Group == TEXT("Message"))        return PGX::System::Message;
	if (Group == TEXT("EventHandler"))   return PGX::System::EventHandler;

	return PGX::Surface::Elevated;
}

#undef LOCTEXT_NAMESPACE
