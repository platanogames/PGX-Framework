// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "LoadingInspector/SPGXLoadingInspectorTab.h"
#include "PGXLoadingSubsystem.h"
#include "Logging/PGXLogMacros.h"
#include "PGXLoadingTypes.h"
#include "PGXLoadingProfile.h"
#include "Profile/PGXProfileSubsystem.h"
#include "Profile/PGXPlatformConfig.h"
#include "GameplayTagContainer.h"

#include "Widgets/SPGXPremiumShell.h"
#include "Widgets/SPGXSectionDivider.h"
#include "Widgets/SPGXKPIChip.h"
#include "Style/PGXEditorStyle.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "Editor.h"
#include "Engine/GameInstance.h"
#include "Styling/AppStyle.h"
#include "Utils/PGXEditorUtils.h"
#include "Style/PGXVisualTokens.h"
#include "Style/PGXStatusPalette.h"
#include "Utils/SPGXTelemetryGraph.h"
#include "AssetRegistry/AssetRegistryModule.h"

#define LOCTEXT_NAMESPACE "PGXLoadingInspector"
DEFINE_LOG_CATEGORY_STATIC(LogPGXLoadingInspector, Log, All);

// EN: Loading system color (Magenta/Pink) / ES: Color del sistema Loading (Magenta/Rosa)
static const FLinearColor GLoadingColor = PGX::System::Loading;

// ============================================================================
// Construct
// ============================================================================

void SPGXLoadingInspectorTab::Construct(const FArguments& /*InArgs*/)
{
	PGX_LOG_INFO(LogPGXLoadingInspector, TEXT("[Loading] Construct"));

	// EN: Initialize StatusBarWidget before shell (referenced in footer)
	// ES: Inicializar StatusBarWidget antes del shell (referenciado en footer)
	SAssignNew(StatusBarWidget, STextBlock)
		.Text(LOCTEXT("StatusIdle", "Loading Inspector — Waiting for PIE"))
		.Font(PGX::Font::BodySmall())
		.ColorAndOpacity(FSlateColor(PGX::Text::Muted));

	ChildSlot
	[
		SNew(SPGXPremiumShell)
		.SystemColor(PGX::System::Loading)
		.Title(LOCTEXT("HeaderTitle", "PGX LOADING INSPECTOR"))
		.Icon(FPGXEditorStyle::Get().GetBrush("PGXEditor.Icon.Loading"))
		.TitleRightContent()
		[
			SNew(SHorizontalBox)

			// EN: KPI chip: State / ES: Chip KPI: Estado
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0, 0, 4, 0)
			[
				SNew(SPGXKPIChip)
				.Label(LOCTEXT("KPIStateLabel", "State"))
				.AccentColor(PGX::System::Loading)
				.ValueWidget()
				[
					SAssignNew(KPIStateChip, STextBlock)
					.Text(LOCTEXT("KPIStateInit", "Idle"))
					.Font(PGX::Font::Mono())
					.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
				]
			]

			// EN: KPI chip: Profiles / ES: Chip KPI: Perfiles
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0, 0, 4, 0)
			[
				SNew(SPGXKPIChip)
				.Label(LOCTEXT("KPIProfilesLabel", "Profiles"))
				.AccentColor(PGX::System::Loading)
				.ValueWidget()
				[
					SAssignNew(KPIProfilesChip, STextBlock)
					.Text(LOCTEXT("KPIProfilesInit", "0"))
					.Font(PGX::Font::Mono())
					.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
				]
			]

			// EN: KPI chip: History / ES: Chip KPI: Historial
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0, 0, 8, 0)
			[
				SNew(SPGXKPIChip)
				.Label(LOCTEXT("KPIHistoryLabel", "History"))
				.AccentColor(PGX::System::Loading)
				.ValueWidget()
				[
					SAssignNew(KPIHistoryChip, STextBlock)
					.Text(LOCTEXT("KPIHistoryInit", "0"))
					.Font(PGX::Font::Mono())
					.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
				]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("Refresh", "Refresh"))
				.ToolTipText(LOCTEXT("RefreshTooltip", "Refresh all panels from subsystem data"))
				.OnClicked_Lambda([this]()
				{
					PGX_LOG_INFO(LogPGXLoadingInspector, TEXT("[Loading] Manual refresh"));
					RefreshAll();
					RefreshKPIChips();
					return FReply::Handled();
				})
			]
		]
		.FooterLeftContent()
		[
			StatusBarWidget.ToSharedRef()
		]
		.Content()
		[
			SNew(SVerticalBox)

			// Scrollable panels
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SScrollBox)

				// Panel 1: Current Status
				+ SScrollBox::Slot()
				.Padding(8)
				[
					BuildCurrentStatusPanel()
				]

				// EN: Loading Telemetry Graphs / ES: Graficos de Telemetria de Carga
				+ SScrollBox::Slot()
				.Padding(8)
				[
					BuildTelemetrySection()
				]

				// Panel 2: Profile Catalog
				+ SScrollBox::Slot()
				.Padding(8)
				[
					BuildProfileCatalogPanel()
				]

				// Panel 3: Loading History
				+ SScrollBox::Slot()
				.Padding(8)
				[
					BuildLoadingHistoryPanel()
				]

				// Panel 4: Debug Controls
				+ SScrollBox::Slot()
				.Padding(8)
				[
					BuildDebugControlsPanel()
				]
			]

			// EN: Platform Budgets / ES: Presupuestos de Plataforma
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4.0f, 4.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SPGXSectionDivider)
					.Title(LOCTEXT("PlatformBudgets", "PLATFORM BUDGETS"))
					.AccentColor(PGX::System::Loading)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(4.0f, 4.0f, 4.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text_Lambda([]() -> FText
					{
						if (UPGXProfileSubsystem* ProfileSS = UPGXProfileSubsystem::GetCachedInstance())
						{
							if (const UPGXPlatformConfig* Cfg = ProfileSS->GetActivePlatformConfig())
							{
								const auto& B = Cfg->LoadingBudgets;
								return FText::Format(
									LOCTEXT("LoadingBudgetsFmt", "Concurrent Async Loads: {0} | Min Loading Screen: {1}s"),
									B.MaxConcurrentAsyncLoads > 0 ? FText::AsNumber(B.MaxConcurrentAsyncLoads) : LOCTEXT("Unlim1", "Unlimited"),
									B.MinLoadingScreenDuration > 0 ? FText::AsNumber(B.MinLoadingScreenDuration) : LOCTEXT("Unlim2", "None"));
							}
						}
						return LOCTEXT("NoPlatformCfg", "No platform config active");
					})
					.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
				]
			]
		]
	];

	BindPIEDelegates();

	// EN: If PIE is already running when panel opens, bind immediately
	// ES: Si PIE ya esta corriendo cuando se abre el panel, enlazar inmediatamente
	if (GEditor && GEditor->PlayWorld)
	{
		PGX_LOG_INFO(LogPGXLoadingInspector, TEXT("[Loading] PIE already running at Construct — binding"));
		OnPIEStarted(false);
	}

	// EN: Scan profiles even without PIE (AssetRegistry works in editor)
	// ES: Escanear perfiles incluso sin PIE (AssetRegistry funciona en editor)
	RefreshProfileCatalog();
}

SPGXLoadingInspectorTab::~SPGXLoadingInspectorTab()
{
	PGX_LOG_INFO(LogPGXLoadingInspector, TEXT("[Loading] Destructor — cleaning up"));
	// EN: Unbind PIE delegates to prevent dangling callbacks / ES: Desvincular delegados PIE para prevenir callbacks colgados
	if (PIEStartedHandle.IsValid())
	{
		FEditorDelegates::PostPIEStarted.Remove(PIEStartedHandle);
	}
	if (PIEEndedHandle.IsValid())
	{
		FEditorDelegates::EndPIE.Remove(PIEEndedHandle);
	}

	UnbindFromSubsystem();
}

// ============================================================================
// Toolbar
// ============================================================================

// EN: BuildToolbar() — Removed: integrated into SPGXPanelHeader in Construct()
// ES: BuildToolbar() — Eliminado: integrado en SPGXPanelHeader en Construct()

// ============================================================================
// Panel 1: Current Status
// ============================================================================

TSharedRef<SWidget> SPGXLoadingInspectorTab::BuildCurrentStatusPanel()
{
	return SNew(SVerticalBox)

		// EN: Section header / ES: Cabecera de seccion
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 4)
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("StatusHeader", "CURRENT STATUS"))
			.AccentColor(PGX::System::Loading)
		]

		// State
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 4)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 8, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("StateLabel", "State:"))
				.Font(PGX::Font::SubHeader())
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SAssignNew(StateTextWidget, STextBlock)
				.Text(LOCTEXT("StateIdle", "Idle"))
				.ColorAndOpacity(FSlateColor(GetStateColor(EPGXLoadingScreenState::Idle)))
			]
		]

		// Context Tag
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 8, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ContextLabel", "Context:"))
				.Font(PGX::Font::SubHeader())
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SAssignNew(ContextWidget, STextBlock)
				.Text(LOCTEXT("NoContext", "(none)"))
			]
		]

		// Elapsed Time
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 8, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ElapsedLabel", "Elapsed:"))
				.Font(PGX::Font::SubHeader())
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SAssignNew(ElapsedWidget, STextBlock)
				.Text(LOCTEXT("Dash1", "—"))
			]
		]

		// Progress (Total / Asset)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 8, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ProgressLabel", "Progress:"))
				.Font(PGX::Font::SubHeader())
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SAssignNew(ProgressWidget, STextBlock)
				.Text(LOCTEXT("Dash2", "—"))
			]
		]

		// EN: Progress bar / ES: Barra de progreso
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2)
		[
			SNew(SBox)
			.HeightOverride(6.0f)
			[
				SNew(SProgressBar)
				.Percent_Lambda([this]() -> TOptional<float>
				{
					if (!BoundSubsystem.IsValid()) { return 0.0f; }
					UPGXLoadingSubsystem* Sub = BoundSubsystem.Get();
					if (!Sub->IsLoadingActive()) { return 0.0f; }
					return Sub->GetProgress().TotalProgress;
				})
				.FillColorAndOpacity(FLinearColor(0.914f, 0.118f, 0.388f))
			]
		]

		// PSO Progress
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 8, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PSOLabel", "PSO:"))
				.Font(PGX::Font::SubHeader())
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SAssignNew(PSOProgressWidget, STextBlock)
				.Text(LOCTEXT("Dash3", "—"))
			]
		]

		// Visual Type
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 8, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("VisualLabel", "Visual:"))
				.Font(PGX::Font::SubHeader())
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SAssignNew(VisualTypeWidget, STextBlock)
				.Text(LOCTEXT("Dash4", "—"))
			]
		]

		// Close Conditions
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 8, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("CloseLabel", "Close:"))
				.Font(PGX::Font::SubHeader())
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SAssignNew(CloseConditionsWidget, STextBlock)
				.Text(LOCTEXT("Dash5", "—"))
			]
		];
}

// ============================================================================
// EN: Loading Telemetry Section / ES: Seccion de Telemetria de Carga
// ============================================================================

TSharedRef<SWidget> SPGXLoadingInspectorTab::BuildTelemetrySection()
{
	return SNew(SVerticalBox)

		// EN: Section header / ES: Cabecera de seccion
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 4)
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("TelemetryHeader", "LOADING TELEMETRY"))
			.AccentColor(PGX::System::Loading)
		]

		// EN: Two graphs side by side / ES: Dos graficos lado a lado
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)

			// [50%] Total Progress
			+ SHorizontalBox::Slot()
			.FillWidth(0.5f)
			.Padding(0, 0, 4, 0)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 2)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ProgressGraphLabel", "Total Progress (%)"))
					.Font(PGX::Font::Badge())
					.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SBox)
					.HeightOverride(PGX::Height::GraphDefault)
					[
						SAssignNew(ProgressGraph, SPGXTelemetryGraph)
						.BufferSize(256)
						.LineColor(PGX::System::Loading)
						.MinValue(0.0f)
						.MaxValue(100.0f)
						.bShowGrid(true)
						.bShowLabels(true)
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					PGXEditorUtils::BuildGraphLegend(ProgressGraph, PGX::System::Loading)
				]
			]

			// [50%] PSO Progress
			+ SHorizontalBox::Slot()
			.FillWidth(0.5f)
			.Padding(4, 0, 0, 0)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 2)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("PSOGraphLabel", "PSO Progress (%)"))
					.Font(PGX::Font::Badge())
					.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SBox)
					.HeightOverride(PGX::Height::GraphDefault)
					[
						SAssignNew(PSOProgressGraph, SPGXTelemetryGraph)
						.BufferSize(256)
						.LineColor(PGX::System::PSO)
						.MinValue(0.0f)
						.MaxValue(100.0f)
						.bShowGrid(true)
						.bShowLabels(true)
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					PGXEditorUtils::BuildGraphLegend(PSOProgressGraph, PGX::System::PSO)
				]
			]
		];
}

// ============================================================================
// Panel 2: Profile Catalog
// ============================================================================

TSharedRef<SWidget> SPGXLoadingInspectorTab::BuildProfileCatalogPanel()
{
	return SNew(SVerticalBox)

		// EN: Section header / ES: Cabecera de seccion
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 4)
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("ProfileHeader", "PROFILE CATALOG"))
			.AccentColor(PGX::System::Loading)
		]

		// List
		+ SVerticalBox::Slot()
		.AutoHeight()
		.MaxHeight(250.0f)
		[
			SAssignNew(ProfileListView, SListView<TSharedPtr<FPGXLoadingProfileEntry>>)
			.ListItemsSource(&ProfileEntries)
			.OnGenerateRow(this, &SPGXLoadingInspectorTab::OnGenerateProfileRow)
			.SelectionMode(ESelectionMode::None)
		];
}

// ============================================================================
// Panel 3: Loading History
// ============================================================================

TSharedRef<SWidget> SPGXLoadingInspectorTab::BuildLoadingHistoryPanel()
{
	return SNew(SVerticalBox)

		// EN: Section header / ES: Cabecera de seccion
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 4)
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("HistoryHeader", "LOADING HISTORY"))
			.AccentColor(PGX::System::Loading)
		]

		// List
		+ SVerticalBox::Slot()
		.AutoHeight()
		.MaxHeight(300.0f)
		[
			SAssignNew(HistoryListView, SListView<TSharedPtr<FPGXLoadingRecord>>)
			.ListItemsSource(&HistoryEntries)
			.OnGenerateRow(this, &SPGXLoadingInspectorTab::OnGenerateHistoryRow)
			.SelectionMode(ESelectionMode::None)
		];
}

// ============================================================================
// Panel 4: Debug Controls
// ============================================================================

TSharedRef<SWidget> SPGXLoadingInspectorTab::BuildDebugControlsPanel()
{
	return SNew(SVerticalBox)

		// EN: Section header / ES: Cabecera de seccion
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 4)
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("DebugHeader", "DEBUG CONTROLS"))
			.AccentColor(PGX::System::Loading)
		]

		// Button row
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 4)
		[
			SNew(SHorizontalBox)

			// Request Loading (Default)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 8, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("RequestBtn", "Request Loading"))
				.ToolTipText(LOCTEXT("RequestTooltip", "RequestLoading with Default context tag"))
				.OnClicked_Lambda([this]()
				{
					if (BoundSubsystem.IsValid())
					{
						// EN: Use RequestGameplayTag instead of native tag (cross-DLL safe)
					// ES: Usar RequestGameplayTag en vez de tag nativo (seguro entre DLLs)
					BoundSubsystem->RequestLoading(FGameplayTag::RequestGameplayTag(FName("PGX.Loading.Context.Default")));
						RefreshAll();
					}
					return FReply::Handled();
				})
			]

			// Force Close
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 8, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("ForceCloseBtn", "Force Close"))
				.ToolTipText(LOCTEXT("ForceCloseTooltip", "Force close loading screen from any state"))
				.OnClicked_Lambda([this]()
				{
					if (BoundSubsystem.IsValid())
					{
						BoundSubsystem->ForceClose();
						RefreshAll();
					}
					return FReply::Handled();
				})
			]

			// Request Skip
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("SkipBtn", "Request Skip"))
				.ToolTipText(LOCTEXT("SkipTooltip", "Request skip (validates conditions before closing)"))
				.OnClicked_Lambda([this]()
				{
					if (BoundSubsystem.IsValid())
					{
						BoundSubsystem->RequestSkip();
						RefreshAll();
					}
					return FReply::Handled();
				})
			]
		];
}

// ============================================================================
// Row Generators
// ============================================================================

TSharedRef<ITableRow> SPGXLoadingInspectorTab::OnGenerateProfileRow(
	TSharedPtr<FPGXLoadingProfileEntry> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	// EN: Build comma-separated context tags string (leaf names) / ES: Construir string de tags separados por coma (nombres hoja)
	FString TagsStr;
	FString FullTagsStr;
	for (int32 i = 0; i < Item->ContextTags.Num(); ++i)
	{
		if (i > 0) { TagsStr += TEXT(", "); FullTagsStr += TEXT(", "); }
		TagsStr += PGXEditorUtils::TagToLeafName(Item->ContextTags[i]);
		FullTagsStr += Item->ContextTags[i].ToString();
	}

	return SNew(STableRow<TSharedPtr<FPGXLoadingProfileEntry>>, OwnerTable)
		.Padding(FMargin(4, 2))
		[
			SNew(SHorizontalBox)

			// Profile name
			+ SHorizontalBox::Slot()
			.FillWidth(0.2f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->ProfileName))
				.Font(PGX::Font::Badge())
			]

			// EN: Context tags (leaf names + full tooltip) / ES: Tags de contexto (hojas + tooltip completo)
			+ SHorizontalBox::Slot()
			.FillWidth(0.35f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TagsStr))
				.ToolTipText(FText::FromString(FullTagsStr))
				.Font(PGX::Font::BodySmall())
			]

			// Visual type
			+ SHorizontalBox::Slot()
			.FillWidth(0.15f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(GetVisualTypeText(Item->VisualType))
				.Font(PGX::Font::BodySmall())
			]

			// PSO / Skip
			+ SHorizontalBox::Slot()
			.FillWidth(0.15f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("PSO:%s Skip:%s"),
					Item->bWaitForPSO ? TEXT("Y") : TEXT("N"),
					Item->bAllowSkip ? TEXT("Y") : TEXT("N"))))
				.Font(PGX::Font::BodySmall())
			]

			// Min time
			+ SHorizontalBox::Slot()
			.FillWidth(0.15f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("Min: %.1fs"), Item->MinDisplayTime)))
				.Font(PGX::Font::BodySmall())
			]
		];
}

TSharedRef<ITableRow> SPGXLoadingInspectorTab::OnGenerateHistoryRow(
	TSharedPtr<FPGXLoadingRecord> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	// EN: Color-code based on result / ES: Color segun resultado
	const FLinearColor RowColor = (Item->ResultCode == EPGXLoadingResultCode::Success)
		? PGX::Semantic::Good
		: PGX::Semantic::Error;

	return SNew(STableRow<TSharedPtr<FPGXLoadingRecord>>, OwnerTable)
		.Padding(FMargin(4, 2))
		[
			SNew(SHorizontalBox)

			// EN: Context tag (leaf name + full tooltip) / ES: Tag de contexto (hoja + tooltip completo)
			+ SHorizontalBox::Slot()
			.FillWidth(0.2f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString(PGXEditorUtils::TagToLeafName(Item->ContextTag)))
				.ToolTipText(FText::FromString(Item->ContextTag.IsValid() ? Item->ContextTag.ToString() : TEXT("")))
				.Font(PGX::Font::BodySmall())
				.ColorAndOpacity(FSlateColor(RowColor))
			]

			// Result code
			+ SHorizontalBox::Slot()
			.FillWidth(0.15f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(GetResultCodeText(Item->ResultCode))
				.Font(PGX::Font::Badge())
				.ColorAndOpacity(FSlateColor(RowColor))
			]

			// Total duration
			+ SHorizontalBox::Slot()
			.FillWidth(0.12f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("%.2fs"), Item->TotalDuration)))
				.Font(PGX::Font::BodySmall())
			]

			// Phase durations
			+ SHorizontalBox::Slot()
			.FillWidth(0.28f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("Prep:%.1f Act:%.1f Wait:%.1f Fade:%.1f"),
					Item->PreparingDuration, Item->ActiveDuration,
					Item->WaitingDuration, Item->FadeDuration)))
				.Font(PGX::Font::Caption())
			]

			// Flags
			+ SHorizontalBox::Slot()
			.FillWidth(0.25f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("%s%s%s"),
					Item->bPSOWaited ? TEXT("PSO ") : TEXT(""),
					Item->bTimedOut ? TEXT("TIMEOUT ") : TEXT(""),
					Item->bUserSkipped ? TEXT("SKIPPED") : TEXT(""))))
				.Font(PGX::Font::BodySmall())
				.ColorAndOpacity(FSlateColor(Item->bTimedOut
					? PGX::Semantic::Warn
					: PGX::Text::Muted))
			]
		];
}

// ============================================================================
// Data Refresh
// ============================================================================

void SPGXLoadingInspectorTab::RefreshAll()
{
	RefreshCurrentStatus();
	RefreshProfileCatalog();
	RefreshLoadingHistory();
}

void SPGXLoadingInspectorTab::RefreshCurrentStatus()
{
	if (!BoundSubsystem.IsValid())
	{
		if (StateTextWidget.IsValid())
		{
			StateTextWidget->SetText(LOCTEXT("NotBound", "Not connected"));
			StateTextWidget->SetColorAndOpacity(FSlateColor(PGX::Text::Muted));
		}
		if (ContextWidget.IsValid()) ContextWidget->SetText(LOCTEXT("NA1", "(n/a)"));
		if (ElapsedWidget.IsValid()) ElapsedWidget->SetText(LOCTEXT("Dash6", "—"));
		if (ProgressWidget.IsValid()) ProgressWidget->SetText(LOCTEXT("Dash7", "—"));
		if (PSOProgressWidget.IsValid()) PSOProgressWidget->SetText(LOCTEXT("Dash8", "—"));
		if (VisualTypeWidget.IsValid()) VisualTypeWidget->SetText(LOCTEXT("Dash9", "—"));
		if (CloseConditionsWidget.IsValid()) CloseConditionsWidget->SetText(LOCTEXT("Dash10", "—"));
		return;
	}

	UPGXLoadingSubsystem* Sub = BoundSubsystem.Get();

	// State
	const EPGXLoadingScreenState State = Sub->GetCurrentState();
	if (StateTextWidget.IsValid())
	{
		StateTextWidget->SetText(GetStateText(State));
		StateTextWidget->SetColorAndOpacity(FSlateColor(GetStateColor(State)));
	}

	// Context
	const FGameplayTag CurrentCtx = Sub->GetCurrentContext();
	if (ContextWidget.IsValid())
	{
		ContextWidget->SetText(FText::FromString(PGXEditorUtils::TagToLeafName(CurrentCtx)));
		ContextWidget->SetToolTipText(FText::FromString(
			CurrentCtx.IsValid() ? CurrentCtx.ToString() : TEXT("")));
	}

	// Elapsed
	if (ElapsedWidget.IsValid())
	{
		if (Sub->IsLoadingActive())
		{
			ElapsedWidget->SetText(FText::FromString(
				FString::Printf(TEXT("%.2fs"), Sub->GetElapsedTime())));
		}
		else
		{
			ElapsedWidget->SetText(LOCTEXT("DashElapsed", "—"));
		}
	}

	// Progress
	const FPGXLoadingProgress Progress = Sub->GetProgress();
	if (ProgressWidget.IsValid())
	{
		if (Sub->IsLoadingActive())
		{
			ProgressWidget->SetText(FText::FromString(
				FString::Printf(TEXT("Total: %.1f%%  Asset: %.1f%%"),
					Progress.TotalProgress * 100.0f,
					Progress.AssetProgress * 100.0f)));
		}
		else
		{
			ProgressWidget->SetText(LOCTEXT("DashProg", "—"));
		}
	}

	// PSO Progress
	if (PSOProgressWidget.IsValid())
	{
		if (Sub->IsLoadingActive())
		{
			PSOProgressWidget->SetText(FText::FromString(
				FString::Printf(TEXT("%.1f%%"), Progress.PSOProgress * 100.0f)));
		}
		else
		{
			PSOProgressWidget->SetText(LOCTEXT("DashPSO", "—"));
		}
	}

	// Visual Type
	if (VisualTypeWidget.IsValid())
	{
		if (Sub->IsLoadingActive())
		{
			VisualTypeWidget->SetText(GetVisualTypeText(Sub->GetActiveVisualType()));
		}
		else
		{
			VisualTypeWidget->SetText(LOCTEXT("DashVisual", "—"));
		}
	}

	// EN: Close Conditions — show real state during loading, summary when idle
	// ES: Condiciones de cierre — mostrar estado real durante loading, resumen cuando idle
	if (CloseConditionsWidget.IsValid())
	{
		if (Sub->IsLoadingActive())
		{
			if (State == EPGXLoadingScreenState::WaitingClose)
			{
				CloseConditionsWidget->SetText(LOCTEXT("Evaluating", "Evaluating close conditions..."));
				CloseConditionsWidget->SetColorAndOpacity(FSlateColor(PGX::Semantic::Warn));
			}
			else
			{
				CloseConditionsWidget->SetText(FText::Format(
					LOCTEXT("ActiveClose", "State: {0} | Elapsed: {1}s"),
					GetStateText(State),
					FText::FromString(FString::Printf(TEXT("%.1f"), Sub->GetElapsedTime()))));
				CloseConditionsWidget->SetColorAndOpacity(FSlateColor(PGX::Text::Secondary));
			}
		}
		else
		{
			CloseConditionsWidget->SetText(FText::Format(
				LOCTEXT("IdleClose", "Profiles: {0} | History: {1}"),
				FText::AsNumber(Sub->GetDiscoveredProfileCount()),
				FText::AsNumber(Sub->GetLoadingHistory().Num())));
			CloseConditionsWidget->SetColorAndOpacity(FSlateColor(PGX::Text::Muted));
		}
	}
}

void SPGXLoadingInspectorTab::RefreshProfileCatalog()
{
	ProfileEntries.Empty();

	// EN: Scan AssetRegistry for all UPGXLoadingProfile DAs (works without PIE)
	// ES: Escanear AssetRegistry para todos los DAs UPGXLoadingProfile (funciona sin PIE)
	IAssetRegistry& AR = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	TArray<FAssetData> FoundProfiles;
	AR.GetAssetsByClass(UPGXLoadingProfile::StaticClass()->GetClassPathName(), FoundProfiles, true);

	for (const FAssetData& AssetData : FoundProfiles)
	{
		if (UPGXLoadingProfile* Profile = Cast<UPGXLoadingProfile>(AssetData.GetAsset()))
		{
			TSharedPtr<FPGXLoadingProfileEntry> Row = MakeShared<FPGXLoadingProfileEntry>();
			Row->ProfileName = Profile->GetName();
			Row->ContextTags = Profile->ContextTags;
			Row->VisualType = Profile->DefaultVisualType;
			Row->bWaitForPSO = Profile->bWaitForPSO;
			Row->MinDisplayTime = Profile->MinDisplayTime;
			Row->bAllowSkip = Profile->bAllowSkip;
			ProfileEntries.Add(Row);
		}
	}

	PGX_LOG_INFO(LogPGXLoadingInspector, TEXT("[Loading] Profile catalog: %d profiles from AssetRegistry"), ProfileEntries.Num());

	if (ProfileListView.IsValid())
	{
		ProfileListView->RequestListRefresh();
	}
}

void SPGXLoadingInspectorTab::RefreshLoadingHistory()
{
	HistoryEntries.Empty();

	if (BoundSubsystem.IsValid())
	{
		UPGXLoadingSubsystem* Sub = BoundSubsystem.Get();
		const TArray<FPGXLoadingRecord> History = Sub->GetLoadingHistory();

		// EN: Show newest first / ES: Mostrar mas reciente primero
		for (int32 i = History.Num() - 1; i >= 0; --i)
		{
			HistoryEntries.Add(MakeShared<FPGXLoadingRecord>(History[i]));
		}
	}

	if (HistoryListView.IsValid())
	{
		HistoryListView->RequestListRefresh();
	}
}

// ============================================================================
// PIE Lifecycle
// ============================================================================

void SPGXLoadingInspectorTab::BindPIEDelegates()
{
	PGX_LOG_INFO(LogPGXLoadingInspector, TEXT("[Loading] PIE delegates bound"));
	PIEStartedHandle = FEditorDelegates::PostPIEStarted.AddSP(
		SharedThis(this), &SPGXLoadingInspectorTab::OnPIEStarted);
	PIEEndedHandle = FEditorDelegates::EndPIE.AddSP(
		SharedThis(this), &SPGXLoadingInspectorTab::OnPIEEnded);
}

void SPGXLoadingInspectorTab::OnPIEStarted(bool bIsSimulating)
{
	PGX_LOG_INFO(LogPGXLoadingInspector, TEXT("[Loading] PIE started (simulating=%s)"),
		bIsSimulating ? TEXT("true") : TEXT("false"));
	BindToSubsystem();
	RefreshAll();
	RefreshKPIChips();

	if (StatusBarWidget.IsValid())
	{
		StatusBarWidget->SetText(LOCTEXT("PIEActive", "Loading Inspector — PIE Active"));
		StatusBarWidget->SetColorAndOpacity(FSlateColor(PGX::Semantic::Good));
	}
}

void SPGXLoadingInspectorTab::OnPIEEnded(bool /*bIsSimulating*/)
{
	PGX_LOG_INFO(LogPGXLoadingInspector, TEXT("[Loading] PIE ended (history entries retained: %d)"), HistoryEntries.Num());
	UnbindFromSubsystem();

	// EN: Keep data visible after PIE ends (post-PIE data retention)
	// ES: Mantener datos visibles tras finalizar PIE (retencion post-PIE)

	if (StatusBarWidget.IsValid())
	{
		StatusBarWidget->SetText(LOCTEXT("PIEEnded", "Loading Inspector — PIE Ended (data retained)"));
		StatusBarWidget->SetColorAndOpacity(FSlateColor(PGX::Text::Muted));
	}
}

void SPGXLoadingInspectorTab::BindToSubsystem()
{
	UnbindFromSubsystem();

	// EN: Find PIE world → GameInstance → Subsystem
	// ES: Encontrar PIE world → GameInstance → Subsystem
	if (!GEditor) return;

	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::PIE && Context.World())
		{
			UGameInstance* GI = Context.World()->GetGameInstance();
			if (GI)
			{
				UPGXLoadingSubsystem* Sub = GI->GetSubsystem<UPGXLoadingSubsystem>();
				if (Sub)
				{
					BoundSubsystem = Sub;

					// EN: Bind to native delegates / ES: Bind a delegados nativos
					StateChangedHandle = Sub->OnLoadingStateChangedNative.AddSP(
						SharedThis(this), &SPGXLoadingInspectorTab::OnLoadingStateChanged);
					LoadingStartedHandle = Sub->OnLoadingStartedNative.AddSP(
						SharedThis(this), &SPGXLoadingInspectorTab::OnLoadingStarted);
					ProgressHandle = Sub->OnLoadingProgressNative.AddSP(
						SharedThis(this), &SPGXLoadingInspectorTab::OnLoadingProgress);
					CompletedHandle = Sub->OnLoadingCompletedNative.AddSP(
						SharedThis(this), &SPGXLoadingInspectorTab::OnLoadingCompleted);

					PGX_LOG_INFO(LogPGXLoadingInspector, TEXT("[Loading] Bound to subsystem (4 delegates)"));
					RefreshKPIChips();
					break;
				}
			}
		}
	}
}

void SPGXLoadingInspectorTab::UnbindFromSubsystem()
{
	if (BoundSubsystem.IsValid())
	{
		PGX_LOG_INFO(LogPGXLoadingInspector, TEXT("[Loading] Unbinding from subsystem (4 delegates)"));
		UPGXLoadingSubsystem* Sub = BoundSubsystem.Get();

		if (StateChangedHandle.IsValid())
		{
			Sub->OnLoadingStateChangedNative.Remove(StateChangedHandle);
			StateChangedHandle.Reset();
		}
		if (LoadingStartedHandle.IsValid())
		{
			Sub->OnLoadingStartedNative.Remove(LoadingStartedHandle);
			LoadingStartedHandle.Reset();
		}
		if (ProgressHandle.IsValid())
		{
			Sub->OnLoadingProgressNative.Remove(ProgressHandle);
			ProgressHandle.Reset();
		}
		if (CompletedHandle.IsValid())
		{
			Sub->OnLoadingCompletedNative.Remove(CompletedHandle);
			CompletedHandle.Reset();
		}
	}

	BoundSubsystem.Reset();
}

// ============================================================================
// Delegate Callbacks
// ============================================================================

void SPGXLoadingInspectorTab::OnLoadingStateChanged(EPGXLoadingScreenState NewState, EPGXLoadingScreenState OldState)
{
	PGX_LOG_INFO(LogPGXLoadingInspector, TEXT("[Loading] State changed: %s -> %s"),
		*GetStateText(OldState).ToString(), *GetStateText(NewState).ToString());

	// EN: Live update all panels on state change / ES: Actualizar todos los paneles en cambio de estado
	RefreshCurrentStatus();
	RefreshKPIChips();

	// EN: Refresh history when a loading completes or fails
	// ES: Refrescar historial cuando un loading completa o falla
	if (NewState == EPGXLoadingScreenState::Idle && OldState == EPGXLoadingScreenState::FadingOut)
	{
		RefreshLoadingHistory();
	}
}

void SPGXLoadingInspectorTab::OnLoadingStarted(FGameplayTag ContextTag)
{
	PGX_LOG_INFO(LogPGXLoadingInspector, TEXT("[Loading] Loading started: %s"),
		*ContextTag.ToString());

	// EN: Clear graphs for new loading session / ES: Limpiar graficos para nueva sesion de carga
	if (ProgressGraph.IsValid()) ProgressGraph->Clear();
	if (PSOProgressGraph.IsValid()) PSOProgressGraph->Clear();

	RefreshAll();
	RefreshKPIChips();
}

void SPGXLoadingInspectorTab::OnLoadingProgress(float Progress, const FText& /*Status*/)
{
	// EN: Live-update progress display (called frequently during loading)
	// ES: Actualizar progreso en vivo (llamado frecuentemente durante loading)
	RefreshCurrentStatus();

	// EN: Push progress data to telemetry graphs / ES: Enviar datos de progreso a graficos de telemetria
	if (ProgressGraph.IsValid())
	{
		ProgressGraph->PushValue(Progress * 100.0f);
	}
	if (PSOProgressGraph.IsValid() && BoundSubsystem.IsValid())
	{
		const FPGXLoadingProgress LoadingProgress = BoundSubsystem->GetProgress();
		PSOProgressGraph->PushValue(LoadingProgress.PSOProgress * 100.0f);
	}
}

void SPGXLoadingInspectorTab::OnLoadingCompleted(const FPGXLoadingRecord& Record)
{
	PGX_LOG_INFO(LogPGXLoadingInspector, TEXT("[Loading] Loading completed: %s (%.2fs, %s)"),
		*Record.ContextTag.ToString(), Record.TotalDuration,
		*GetResultCodeText(Record.ResultCode).ToString());
	RefreshAll();
	RefreshKPIChips();
}

// ============================================================================
// KPI
// ============================================================================

void SPGXLoadingInspectorTab::RefreshKPIChips()
{
	// EN: State chip / ES: Chip de estado
	if (KPIStateChip.IsValid())
	{
		if (BoundSubsystem.IsValid())
		{
			const EPGXLoadingScreenState State = BoundSubsystem->GetCurrentState();
			KPIStateChip->SetText(GetStateText(State));
			KPIStateChip->SetColorAndOpacity(FSlateColor(GetStateColor(State)));
		}
		else
		{
			KPIStateChip->SetText(LOCTEXT("KPIStateOff", "--"));
			KPIStateChip->SetColorAndOpacity(FSlateColor(PGX::Text::Muted));
		}
	}

	// EN: Profiles chip / ES: Chip de perfiles
	if (KPIProfilesChip.IsValid())
	{
		KPIProfilesChip->SetText(FText::AsNumber(ProfileEntries.Num()));
	}

	// EN: History chip / ES: Chip de historial
	if (KPIHistoryChip.IsValid())
	{
		KPIHistoryChip->SetText(FText::AsNumber(HistoryEntries.Num()));
	}
}

// ============================================================================
// Helpers
// ============================================================================

FLinearColor SPGXLoadingInspectorTab::GetStateColor(EPGXLoadingScreenState State)
{
	switch (State)
	{
	case EPGXLoadingScreenState::Idle:         return PGX::GetStatusPaletteColor(PGX::EStatusPalette::Idle);
	case EPGXLoadingScreenState::Preparing:    return PGX::GetStatusPaletteColor(PGX::EStatusPalette::Preparing);
	case EPGXLoadingScreenState::FadingIn:     return PGX::GetStatusPaletteColor(PGX::EStatusPalette::FadingIn);
	case EPGXLoadingScreenState::Active:       return PGX::GetStatusPaletteColor(PGX::EStatusPalette::Active);
	case EPGXLoadingScreenState::WaitingClose: return PGX::GetStatusPaletteColor(PGX::EStatusPalette::Waiting);
	case EPGXLoadingScreenState::FadingOut:    return PGX::GetStatusPaletteColor(PGX::EStatusPalette::FadingOut);
	default:                                   return PGX::GetStatusPaletteColor(PGX::EStatusPalette::Unknown);
	}
}

FText SPGXLoadingInspectorTab::GetStateText(EPGXLoadingScreenState State)
{
	switch (State)
	{
	case EPGXLoadingScreenState::Idle:         return LOCTEXT("StIdle", "Idle");
	case EPGXLoadingScreenState::Preparing:    return LOCTEXT("StPreparing", "Preparing");
	case EPGXLoadingScreenState::FadingIn:     return LOCTEXT("StFadingIn", "Fading In");
	case EPGXLoadingScreenState::Active:       return LOCTEXT("StActive", "Active");
	case EPGXLoadingScreenState::WaitingClose: return LOCTEXT("StWaiting", "Waiting Close");
	case EPGXLoadingScreenState::FadingOut:    return LOCTEXT("StFadingOut", "Fading Out");
	default:                                   return LOCTEXT("StUnknown", "Unknown");
	}
}

FText SPGXLoadingInspectorTab::GetVisualTypeText(EPGXLoadingVisualType Type)
{
	switch (Type)
	{
	case EPGXLoadingVisualType::Minimal:          return LOCTEXT("VTMinimal", "Minimal");
	case EPGXLoadingVisualType::StaticImage:      return LOCTEXT("VTStaticImage", "StaticImage");
	case EPGXLoadingVisualType::Slideshow:        return LOCTEXT("VTSlideshow", "Slideshow");
	case EPGXLoadingVisualType::MaterialAnimated: return LOCTEXT("VTMaterial", "Material");
	case EPGXLoadingVisualType::Video:            return LOCTEXT("VTVideo", "Video");
	case EPGXLoadingVisualType::Custom:           return LOCTEXT("VTCustom", "Custom");
	default:                                      return LOCTEXT("VTUnknown", "Unknown");
	}
}

FText SPGXLoadingInspectorTab::GetResultCodeText(EPGXLoadingResultCode Code)
{
	switch (Code)
	{
	case EPGXLoadingResultCode::Success:         return LOCTEXT("RCSuccess", "Success");
	case EPGXLoadingResultCode::ProfileNotFound: return LOCTEXT("RCNotFound", "ProfileNotFound");
	case EPGXLoadingResultCode::InvalidProfile:  return LOCTEXT("RCInvalid", "InvalidProfile");
	case EPGXLoadingResultCode::AlreadyActive:   return LOCTEXT("RCActive", "AlreadyActive");
	case EPGXLoadingResultCode::ForceClosed:     return LOCTEXT("RCForced", "ForceClosed");
	case EPGXLoadingResultCode::TimedOut:        return LOCTEXT("RCTimeout", "TimedOut");
	case EPGXLoadingResultCode::AssetLoadFailed: return LOCTEXT("RCAssetFail", "AssetLoadFailed");
	case EPGXLoadingResultCode::Cancelled:       return LOCTEXT("RCCancelled", "Cancelled");
	default:                                     return LOCTEXT("RCUnknown", "Unknown");
	}
}

#undef LOCTEXT_NAMESPACE
