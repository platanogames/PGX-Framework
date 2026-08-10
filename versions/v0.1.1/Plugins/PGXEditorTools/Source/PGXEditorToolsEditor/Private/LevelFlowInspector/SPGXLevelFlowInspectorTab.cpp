// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "LevelFlowInspector/SPGXLevelFlowInspectorTab.h"
#include "PGXLevelFlowSubsystem.h"
#include "Logging/PGXLogMacros.h"
#include "PGXLevelFlowTypes.h"
#include "ShaderPipelineCache.h"

#include "Widgets/SPGXPremiumShell.h"
#include "Widgets/PGXButtonHelpers.h"
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

#define LOCTEXT_NAMESPACE "PGXLevelFlowInspector"
DEFINE_LOG_CATEGORY_STATIC(LogPGXLevelFlowInspector, Log, All);

// EN: LevelFlow system color (Blue) — via PGX Visual Tokens / ES: Color del sistema LevelFlow (Azul) — via PGX Visual Tokens
static const FLinearColor GLevelFlowColor = PGX::System::LevelFlow;

// ============================================================================
// Construct
// ============================================================================

void SPGXLevelFlowInspectorTab::Construct(const FArguments& /*InArgs*/)
{
	PGX_LOG_INFO(LogPGXLevelFlowInspector, TEXT("[LevelFlow] Construct"));

	ChildSlot
	[
		// EN: Premium Shell — covers UE chrome, provides title bar + footer
		// ES: Shell Premium — cubre chrome de UE, provee title bar + footer
		SNew(SPGXPremiumShell)
		.SystemColor(PGX::System::LevelFlow)
		.Title(LOCTEXT("HeaderTitle", "LevelFlow Inspector"))
		.Subtitle(LOCTEXT("HeaderSubtitle", "Level transitions, streaming, and sub-levels"))
		.Icon(FPGXEditorStyle::Get().GetBrush("PGXEditor.Icon.LevelFlow"))
		.bShowFooter(true)
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
				.AccentColor(PGX::System::LevelFlow)
				.ValueWidget()
				[
					SAssignNew(KPIStateChip, STextBlock)
					.Text(LOCTEXT("KPIStateInit", "Idle"))
					.Font(PGX::Font::Mono())
					.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
				]
			]

			// EN: KPI chip: Levels / ES: Chip KPI: Niveles
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0, 0, 4, 0)
			[
				SNew(SPGXKPIChip)
				.Label(LOCTEXT("KPILevelsLabel", "Levels"))
				.AccentColor(PGX::System::LevelFlow)
				.ValueWidget()
				[
					SAssignNew(KPILevelsChip, STextBlock)
					.Text(LOCTEXT("KPILevelsInit", "0"))
					.Font(PGX::Font::Mono())
					.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
				]
			]

			// EN: KPI chip: Profiles / ES: Chip KPI: Perfiles
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0, 0, 8, 0)
			[
				SNew(SPGXKPIChip)
				.Label(LOCTEXT("KPIProfilesLabel", "Profiles"))
				.AccentColor(PGX::System::LevelFlow)
				.ValueWidget()
				[
					SAssignNew(KPIProfilesChip, STextBlock)
					.Text(LOCTEXT("KPIProfilesInit", "0"))
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
					PGX_LOG_INFO(LogPGXLevelFlowInspector, TEXT("[LevelFlow] Manual refresh"));
					RefreshAll();
					RefreshKPIChips();
					return FReply::Handled();
				})
			]
		]
		.StatusText_Lambda([this]() -> FText
		{
			if (StatusBarWidget.IsValid())
			{
				return StatusBarWidget->GetText();
			}
			return LOCTEXT("StatusIdle2b", "LevelFlow Inspector — Waiting for PIE");
		})
		.Content()
		[
			SNew(SScrollBox)

			// Panel 1: Transition Status
			+ SScrollBox::Slot()
			.Padding(8)
			[
				BuildTransitionStatusPanel()
			]

			// Panel 2: Level Catalog
			+ SScrollBox::Slot()
			.Padding(8)
			[
				BuildLevelCatalogPanel()
			]

			// Panel 3: Transition History
			+ SScrollBox::Slot()
			.Padding(8)
			[
				BuildTransitionHistoryPanel()
			]

			// Panel 4: Sub-Levels
			+ SScrollBox::Slot()
			.Padding(8)
			[
				BuildSubLevelsPanel()
			]
		]
	];

	// EN: Hidden text block for footer status (read by StatusText lambda)
	// ES: Text block oculto para estado del footer (leido por lambda StatusText)
	SAssignNew(StatusBarWidget, STextBlock)
		.Text(LOCTEXT("StatusIdle", "LevelFlow Inspector — Waiting for PIE"));

	BindPIEDelegates();

	// EN: If PIE is already running when panel opens, bind immediately
	// ES: Si PIE ya esta corriendo cuando se abre el panel, enlazar inmediatamente
	if (GEditor && GEditor->PlayWorld)
	{
		PGX_LOG_INFO(LogPGXLevelFlowInspector, TEXT("[LevelFlow] PIE already running at Construct — binding"));
		OnPIEStarted(false);
	}
}

SPGXLevelFlowInspectorTab::~SPGXLevelFlowInspectorTab()
{
	PGX_LOG_INFO(LogPGXLevelFlowInspector, TEXT("[LevelFlow] Destructor — cleaning up"));
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
// Panel 1: Transition Status
// ============================================================================

TSharedRef<SWidget> SPGXLevelFlowInspectorTab::BuildTransitionStatusPanel()
{
	return SNew(SVerticalBox)

		// EN: Section header / ES: Cabecera de seccion
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 4)
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("TransitionStatusHeader", "TRANSITION STATUS"))
			.AccentColor(PGX::System::LevelFlow)
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
				.ColorAndOpacity(FSlateColor(GetStateColor(EPGXLevelFlowState::Idle)))
			]
		]

		// Current Level
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
				.Text(LOCTEXT("CurrentLevelLabel", "Current Level:"))
				.Font(PGX::Font::SubHeader())
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SAssignNew(CurrentLevelWidget, STextBlock)
				.Text(LOCTEXT("NoLevel", "(none)"))
			]
		]

		// Previous Level
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
				.Text(LOCTEXT("PreviousLevelLabel", "Previous Level:"))
				.Font(PGX::Font::SubHeader())
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SAssignNew(PreviousLevelWidget, STextBlock)
				.Text(LOCTEXT("NoPrevLevel", "(none)"))
			]
		]

		// Progress
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
				.Text(LOCTEXT("NoProgress", "—"))
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
					UPGXLevelFlowSubsystem* Sub = BoundSubsystem.Get();
					if (!Sub->IsTransitionActive()) { return 0.0f; }
					return Sub->GetTransitionProgress();
				})
				.FillColorAndOpacity(FLinearColor(0.129f, 0.588f, 0.953f))
			]
		]

		// Timing Info
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
				.Text(LOCTEXT("TimingLabel", "Timing:"))
				.Font(PGX::Font::SubHeader())
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SAssignNew(TimingInfoWidget, STextBlock)
				.Text(LOCTEXT("NoTiming", "—"))
			]
		]

		// EN: Cancel Transition button / ES: Boton de cancelar transicion
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 6, 0, 2)
		[
			SAssignNew(CancelButton, SButton)
			.Text(LOCTEXT("CancelTransition", "Cancel Transition"))
			.ToolTipText(LOCTEXT("CancelTip", "Cancel the active level transition (not possible during Transitioning state)"))
			.IsEnabled_Lambda([this]() -> bool
			{
				if (!BoundSubsystem.IsValid()) return false;
				const EPGXLevelFlowState State = BoundSubsystem->GetTransitionState();
				return State == EPGXLevelFlowState::Preparing
					|| State == EPGXLevelFlowState::Loading
					|| State == EPGXLevelFlowState::PostLoad;
			})
			.OnClicked_Lambda([this]() -> FReply
			{
				if (BoundSubsystem.IsValid())
				{
					PGX_LOG_INFO(LogPGXLevelFlowInspector, TEXT("[LevelFlow] Cancel Transition requested"));
					BoundSubsystem->CancelTransition();
				}
				return FReply::Handled();
			})
		];
}

// ============================================================================
// Panel 2: Level Catalog
// ============================================================================

TSharedRef<SWidget> SPGXLevelFlowInspectorTab::BuildLevelCatalogPanel()
{
	return SNew(SVerticalBox)

		// EN: Section header / ES: Cabecera de seccion
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 4)
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("CatalogHeader", "LEVEL CATALOG"))
			.AccentColor(PGX::System::LevelFlow)
		]

		// List
		+ SVerticalBox::Slot()
		.AutoHeight()
		.MaxHeight(250.0f)
		[
			SAssignNew(CatalogListView, SListView<TSharedPtr<FPGXLevelCatalogEntry>>)
			.ListItemsSource(&CatalogEntries)
			.OnGenerateRow(this, &SPGXLevelFlowInspectorTab::OnGenerateCatalogRow)
			.SelectionMode(ESelectionMode::None)
		];
}

// ============================================================================
// Panel 3: Transition History
// ============================================================================

TSharedRef<SWidget> SPGXLevelFlowInspectorTab::BuildTransitionHistoryPanel()
{
	return SNew(SVerticalBox)

		// EN: Section header / ES: Cabecera de seccion
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 4)
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("HistoryHeader", "TRANSITION HISTORY"))
			.AccentColor(PGX::System::LevelFlow)
		]

		// List
		+ SVerticalBox::Slot()
		.AutoHeight()
		.MaxHeight(300.0f)
		[
			SAssignNew(HistoryListView, SListView<TSharedPtr<FPGXLevelTransitionRecord>>)
			.ListItemsSource(&HistoryEntries)
			.OnGenerateRow(this, &SPGXLevelFlowInspectorTab::OnGenerateHistoryRow)
			.SelectionMode(ESelectionMode::None)
		];
}

// ============================================================================
// Panel 4: Sub-Levels
// ============================================================================

TSharedRef<SWidget> SPGXLevelFlowInspectorTab::BuildSubLevelsPanel()
{
	return SNew(SVerticalBox)

		// EN: Section header / ES: Cabecera de seccion
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 4)
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("SubLevelsHeader", "SUB-LEVELS"))
			.AccentColor(PGX::System::LevelFlow)
		]

		// List
		+ SVerticalBox::Slot()
		.AutoHeight()
		.MaxHeight(200.0f)
		[
			SAssignNew(SubLevelListView, SListView<TSharedPtr<FPGXSubLevelStatusRow>>)
			.ListItemsSource(&SubLevelEntries)
			.OnGenerateRow(this, &SPGXLevelFlowInspectorTab::OnGenerateSubLevelRow)
			.SelectionMode(ESelectionMode::None)
		];
}

// ============================================================================
// Row Generators
// ============================================================================

TSharedRef<ITableRow> SPGXLevelFlowInspectorTab::OnGenerateCatalogRow(
	TSharedPtr<FPGXLevelCatalogEntry> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FPGXLevelCatalogEntry>>, OwnerTable)
		.Padding(FMargin(4, 2))
		[
			SNew(SHorizontalBox)

			// EN: Tag (leaf name + full tooltip) / ES: Tag (nombre hoja + tooltip completo)
			+ SHorizontalBox::Slot()
			.FillWidth(0.3f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString(PGXEditorUtils::TagToLeafName(Item->LevelTag)))
				.ToolTipText(FText::FromString(Item->LevelTag.ToString()))
				.Font(PGX::Font::BodySmall())
			]

			// Display Name
			+ SHorizontalBox::Slot()
			.FillWidth(0.2f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(Item->DisplayName)
				.Font(PGX::Font::BodySmall())
			]

			// Strategy
			+ SHorizontalBox::Slot()
			.FillWidth(0.15f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(GetLoadStrategyText(Item->LoadStrategy))
				.Font(PGX::Font::BodySmall())
			]

			// Transition Mode
			+ SHorizontalBox::Slot()
			.FillWidth(0.2f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(GetTransitionModeText(Item->TransitionMode))
				.Font(PGX::Font::BodySmall())
			]

			// Sub-Levels count
			+ SHorizontalBox::Slot()
			.FillWidth(0.15f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(FText::Format(LOCTEXT("SubLevelCountFmt", "{0} sub-levels"), FText::AsNumber(Item->SubLevelCount)))
				.Font(PGX::Font::BodySmall())
			]
		];
}

TSharedRef<ITableRow> SPGXLevelFlowInspectorTab::OnGenerateHistoryRow(
	TSharedPtr<FPGXLevelTransitionRecord> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	// EN: Color-code based on result / ES: Color segun resultado
	const FLinearColor RowColor = (Item->ResultCode == EPGXLevelFlowResultCode::Success)
		? PGX::Semantic::Good
		: PGX::Semantic::Error;

	const FString FromStr = PGXEditorUtils::TagToLeafName(Item->FromLevelTag);
	const FString ToStr = PGXEditorUtils::TagToLeafName(Item->ToLevelTag);
	const FString FullTooltip = FString::Printf(TEXT("%s -> %s"),
		*( Item->FromLevelTag.IsValid() ? Item->FromLevelTag.ToString() : TEXT("(none)") ),
		*( Item->ToLevelTag.IsValid() ? Item->ToLevelTag.ToString() : TEXT("(none)") ));

	return SNew(STableRow<TSharedPtr<FPGXLevelTransitionRecord>>, OwnerTable)
		.Padding(FMargin(4, 2))
		[
			SNew(SHorizontalBox)

			// EN: From -> To (leaf names + full tag tooltip) / ES: Desde -> Hasta (hojas + tooltip completo)
			+ SHorizontalBox::Slot()
			.FillWidth(0.35f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("%s -> %s"), *FromStr, *ToStr)))
				.ToolTipText(FText::FromString(FullTooltip))
				.Font(PGX::Font::BodySmall())
				.ColorAndOpacity(FSlateColor(RowColor))
			]

			// Load Time
			+ SHorizontalBox::Slot()
			.FillWidth(0.15f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("Load: %.2fs"), Item->LoadDurationSeconds)))
				.Font(PGX::Font::BodySmall())
			]

			// Wait Time
			+ SHorizontalBox::Slot()
			.FillWidth(0.15f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("Wait: %.2fs"), Item->WaitDurationSeconds)))
				.Font(PGX::Font::BodySmall())
			]

			// Shaders
			+ SHorizontalBox::Slot()
			.FillWidth(0.15f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("Shaders: %d"), Item->ShaderCompilationsOnEntry)))
				.Font(PGX::Font::BodySmall())
			]

			// Timed Out
			+ SHorizontalBox::Slot()
			.FillWidth(0.2f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(Item->bTimedOut ? LOCTEXT("TimedOut", "TIMED OUT") : LOCTEXT("OnTime", "OK"))
				.Font(PGX::Font::BodySmall())
				.ColorAndOpacity(FSlateColor(Item->bTimedOut ? PGX::Semantic::Warn : PGX::Text::Muted))
			]
		];
}

TSharedRef<ITableRow> SPGXLevelFlowInspectorTab::OnGenerateSubLevelRow(
	TSharedPtr<FPGXSubLevelStatusRow> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	const FLinearColor StatusColor = Item->bIsLoaded
		? PGX::Semantic::Good
		: PGX::Semantic::Neutral;

	return SNew(STableRow<TSharedPtr<FPGXSubLevelStatusRow>>, OwnerTable)
		.Padding(FMargin(4, 2))
		[
			SNew(SHorizontalBox)

			// EN: Tag (leaf name + full tooltip) / ES: Tag (nombre hoja + tooltip completo)
			+ SHorizontalBox::Slot()
			.FillWidth(0.7f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString(PGXEditorUtils::TagToLeafName(Item->SubLevelTag)))
				.ToolTipText(FText::FromString(Item->SubLevelTag.ToString()))
				.Font(PGX::Font::BodySmall())
			]

			// Status
			+ SHorizontalBox::Slot()
			.FillWidth(0.3f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(Item->bIsLoaded ? LOCTEXT("Loaded", "LOADED") : LOCTEXT("NotLoaded", "NOT LOADED"))
				.Font(PGX::Font::Badge())
				.ColorAndOpacity(FSlateColor(StatusColor))
			]
		];
}

// ============================================================================
// Data Refresh
// ============================================================================

void SPGXLevelFlowInspectorTab::RefreshAll()
{
	RefreshTransitionStatus();
	RefreshLevelCatalog();
	RefreshTransitionHistory();
	RefreshSubLevels();
}

void SPGXLevelFlowInspectorTab::RefreshTransitionStatus()
{
	if (!BoundSubsystem.IsValid())
	{
		if (StateTextWidget.IsValid())
		{
			StateTextWidget->SetText(LOCTEXT("NotBound", "Not connected"));
			StateTextWidget->SetColorAndOpacity(FSlateColor(PGX::Text::Muted));
		}
		if (CurrentLevelWidget.IsValid()) CurrentLevelWidget->SetText(LOCTEXT("NA", "(n/a)"));
		if (PreviousLevelWidget.IsValid()) PreviousLevelWidget->SetText(LOCTEXT("NA2", "(n/a)"));
		if (ProgressWidget.IsValid()) ProgressWidget->SetText(LOCTEXT("Dash", "—"));
		if (TimingInfoWidget.IsValid()) TimingInfoWidget->SetText(LOCTEXT("Dash2", "—"));
		return;
	}

	UPGXLevelFlowSubsystem* Sub = BoundSubsystem.Get();

	// State
	const EPGXLevelFlowState State = Sub->GetTransitionState();
	if (StateTextWidget.IsValid())
	{
		StateTextWidget->SetText(GetStateText(State));
		StateTextWidget->SetColorAndOpacity(FSlateColor(GetStateColor(State)));
	}

	// Current / Previous level
	const FGameplayTag CurrentTag = Sub->GetCurrentLevelTag();
	const FGameplayTag PrevTag = Sub->GetPreviousLevelTag();

	if (CurrentLevelWidget.IsValid())
	{
		CurrentLevelWidget->SetText(FText::FromString(PGXEditorUtils::TagToLeafName(CurrentTag)));
		CurrentLevelWidget->SetToolTipText(FText::FromString(
			CurrentTag.IsValid() ? CurrentTag.ToString() : TEXT("")));
	}
	if (PreviousLevelWidget.IsValid())
	{
		PreviousLevelWidget->SetText(FText::FromString(PGXEditorUtils::TagToLeafName(PrevTag)));
		PreviousLevelWidget->SetToolTipText(FText::FromString(
			PrevTag.IsValid() ? PrevTag.ToString() : TEXT("")));
	}

	// Progress
	if (ProgressWidget.IsValid())
	{
		if (Sub->IsTransitionActive())
		{
			const float Progress = Sub->GetTransitionProgress();
			ProgressWidget->SetText(FText::FromString(
				FString::Printf(TEXT("%.1f%%"), Progress * 100.0f)));
		}
		else
		{
			ProgressWidget->SetText(LOCTEXT("DashProg", "—"));
		}
	}

	// Timing / Shaders
	if (TimingInfoWidget.IsValid())
	{
		const int32 ShadersRemaining = static_cast<int32>(FShaderPipelineCache::NumPrecompilesRemaining());
		TimingInfoWidget->SetText(FText::FromString(
			FString::Printf(TEXT("Shaders remaining: %d"), ShadersRemaining)));
	}
}

void SPGXLevelFlowInspectorTab::RefreshLevelCatalog()
{
	CatalogEntries.Empty();

	if (BoundSubsystem.IsValid())
	{
		UPGXLevelFlowSubsystem* Sub = BoundSubsystem.Get();
		const TArray<FGameplayTag> Tags = Sub->GetRegisteredLevelTags();

		for (const FGameplayTag& CtxTag : Tags)
		{
			FPGXLevelEntry Entry;
			if (Sub->ResolveLevelByTag(CtxTag, Entry))
			{
				TSharedPtr<FPGXLevelCatalogEntry> Row = MakeShared<FPGXLevelCatalogEntry>();
				Row->LevelTag = CtxTag;
				Row->DisplayName = Entry.DisplayName;
				Row->LoadStrategy = Entry.LoadStrategy;
				Row->TransitionMode = Entry.TransitionMode;
				Row->SubLevelCount = Entry.SubLevels.Num();
				CatalogEntries.Add(Row);
			}
		}
	}

	if (CatalogListView.IsValid())
	{
		CatalogListView->RequestListRefresh();
	}
}

void SPGXLevelFlowInspectorTab::RefreshTransitionHistory()
{
	HistoryEntries.Empty();

	if (BoundSubsystem.IsValid())
	{
		UPGXLevelFlowSubsystem* Sub = BoundSubsystem.Get();
		const TArray<FPGXLevelTransitionRecord> History = Sub->GetTransitionHistory();

		// EN: Show newest first / ES: Mostrar mas reciente primero
		for (int32 i = History.Num() - 1; i >= 0; --i)
		{
			HistoryEntries.Add(MakeShared<FPGXLevelTransitionRecord>(History[i]));
		}
	}

	if (HistoryListView.IsValid())
	{
		HistoryListView->RequestListRefresh();
	}
}

void SPGXLevelFlowInspectorTab::RefreshSubLevels()
{
	SubLevelEntries.Empty();

	if (BoundSubsystem.IsValid())
	{
		UPGXLevelFlowSubsystem* Sub = BoundSubsystem.Get();
		const FGameplayTag CurrentTag = Sub->GetCurrentLevelTag();

		// EN: Get ALL configured sub-levels for current level, mark loaded/unloaded
		// ES: Obtener TODOS los sub-levels configurados del nivel actual, marcar loaded/unloaded
		if (CurrentTag.IsValid())
		{
			FPGXLevelEntry Entry;
			if (Sub->ResolveLevelByTag(CurrentTag, Entry))
			{
				for (const FPGXSubLevelEntry& SubEntry : Entry.SubLevels)
				{
					TSharedPtr<FPGXSubLevelStatusRow> Row = MakeShared<FPGXSubLevelStatusRow>();
					Row->SubLevelTag = SubEntry.SubLevelTag;
					Row->bIsLoaded = Sub->IsSubLevelLoaded(SubEntry.SubLevelTag);
					SubLevelEntries.Add(Row);
				}
			}
		}

		// EN: Add any dynamically loaded sub-levels not in the configured list
		// ES: Agregar sub-levels cargados dinamicamente que no esten en la lista configurada
		const TArray<FGameplayTag> LoadedSubs = Sub->GetLoadedSubLevels();
		for (const FGameplayTag& SubTag : LoadedSubs)
		{
			bool bFound = false;
			for (const auto& Existing : SubLevelEntries)
			{
				if (Existing->SubLevelTag == SubTag) { bFound = true; break; }
			}
			if (!bFound)
			{
				TSharedPtr<FPGXSubLevelStatusRow> Row = MakeShared<FPGXSubLevelStatusRow>();
				Row->SubLevelTag = SubTag;
				Row->bIsLoaded = true;
				SubLevelEntries.Add(Row);
			}
		}
	}

	if (SubLevelListView.IsValid())
	{
		SubLevelListView->RequestListRefresh();
	}
}

// ============================================================================
// PIE Lifecycle
// ============================================================================

void SPGXLevelFlowInspectorTab::BindPIEDelegates()
{
	PGX_LOG_INFO(LogPGXLevelFlowInspector, TEXT("[LevelFlow] PIE delegates bound"));
	PIEStartedHandle = FEditorDelegates::PostPIEStarted.AddSP(
		SharedThis(this), &SPGXLevelFlowInspectorTab::OnPIEStarted);
	PIEEndedHandle = FEditorDelegates::EndPIE.AddSP(
		SharedThis(this), &SPGXLevelFlowInspectorTab::OnPIEEnded);
}

void SPGXLevelFlowInspectorTab::OnPIEStarted(bool bIsSimulating)
{
	PGX_LOG_INFO(LogPGXLevelFlowInspector, TEXT("[LevelFlow] PIE started (simulating=%d)"), bIsSimulating);
	BindToSubsystem();
	RefreshAll();
	RefreshKPIChips();

	if (StatusBarWidget.IsValid())
	{
		StatusBarWidget->SetText(LOCTEXT("PIEActive", "LevelFlow Inspector — PIE Active"));
		StatusBarWidget->SetColorAndOpacity(FSlateColor(PGX::Semantic::Good));
	}
}

void SPGXLevelFlowInspectorTab::OnPIEEnded(bool /*bIsSimulating*/)
{
	PGX_LOG_INFO(LogPGXLevelFlowInspector, TEXT("[LevelFlow] PIE ended — data retained"));
	UnbindFromSubsystem();

	// EN: Keep data visible after PIE ends (post-PIE data retention)
	// ES: Mantener datos visibles tras finalizar PIE (retencion post-PIE)

	if (StatusBarWidget.IsValid())
	{
		StatusBarWidget->SetText(LOCTEXT("PIEEnded", "LevelFlow Inspector — PIE Ended (data retained)"));
		StatusBarWidget->SetColorAndOpacity(FSlateColor(PGX::Text::Muted));
	}
}

void SPGXLevelFlowInspectorTab::BindToSubsystem()
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
				UPGXLevelFlowSubsystem* Sub = GI->GetSubsystem<UPGXLevelFlowSubsystem>();
				if (Sub)
				{
					BoundSubsystem = Sub;
					PGX_LOG_INFO(LogPGXLevelFlowInspector, TEXT("[LevelFlow] Bound to subsystem"));

					// EN: Bind to native delegates / ES: Bind a delegados nativos
					StateChangedHandle = Sub->OnLevelFlowStateChangedNative.AddSP(
						SharedThis(this), &SPGXLevelFlowInspectorTab::OnLevelFlowStateChanged);
					TransitionStartedHandle = Sub->OnTransitionStartedNative.AddSP(
						SharedThis(this), &SPGXLevelFlowInspectorTab::OnTransitionStarted);
					TransitionCompletedHandle = Sub->OnTransitionCompletedNative.AddSP(
						SharedThis(this), &SPGXLevelFlowInspectorTab::OnTransitionCompleted);

					break;
				}
			}
		}
	}
}

void SPGXLevelFlowInspectorTab::UnbindFromSubsystem()
{
	PGX_LOG_INFO(LogPGXLevelFlowInspector, TEXT("[LevelFlow] Unbinding from subsystem"));
	UPGXLevelFlowSubsystem* Sub = BoundSubsystem.Get();
	if (Sub)
	{
		if (StateChangedHandle.IsValid()) Sub->OnLevelFlowStateChangedNative.Remove(StateChangedHandle);
		if (TransitionStartedHandle.IsValid()) Sub->OnTransitionStartedNative.Remove(TransitionStartedHandle);
		if (TransitionCompletedHandle.IsValid()) Sub->OnTransitionCompletedNative.Remove(TransitionCompletedHandle);
	}
	StateChangedHandle.Reset();
	TransitionStartedHandle.Reset();
	TransitionCompletedHandle.Reset();
	BoundSubsystem.Reset();
}

// ============================================================================
// Delegate Callbacks
// ============================================================================

void SPGXLevelFlowInspectorTab::OnLevelFlowStateChanged(EPGXLevelFlowState NewState, FGameplayTag LevelTag)
{
	PGX_LOG_INFO(LogPGXLevelFlowInspector, TEXT("[LevelFlow] State changed: %s (level: %s)"),
		*GetStateText(NewState).ToString(), *LevelTag.ToString());

	// EN: Always refresh status on any state change (including intermediates)
	// ES: Siempre refrescar status en cualquier cambio de estado (incluidos intermedios)
	RefreshTransitionStatus();
	RefreshKPIChips();

	// EN: Refresh history and sub-levels when a transition completes or fails
	// ES: Refrescar historial y sub-levels cuando una transicion completa o falla
	if (NewState == EPGXLevelFlowState::Complete || NewState == EPGXLevelFlowState::Failed)
	{
		RefreshTransitionHistory();
		RefreshSubLevels();
	}
}

void SPGXLevelFlowInspectorTab::OnTransitionStarted(FGameplayTag LevelTag)
{
	PGX_LOG_INFO(LogPGXLevelFlowInspector, TEXT("[LevelFlow] Transition started to: %s"), *LevelTag.ToString());
	RefreshAll();
	RefreshKPIChips();
}

void SPGXLevelFlowInspectorTab::OnTransitionCompleted(FGameplayTag LevelTag)
{
	PGX_LOG_INFO(LogPGXLevelFlowInspector, TEXT("[LevelFlow] Transition completed to: %s"), *LevelTag.ToString());
	RefreshAll();
	RefreshKPIChips();
}

void SPGXLevelFlowInspectorTab::RefreshKPIChips()
{
	if (KPIStateChip.IsValid())
	{
		if (BoundSubsystem.IsValid())
		{
			const EPGXLevelFlowState State = BoundSubsystem->GetTransitionState();
			KPIStateChip->SetText(GetStateText(State));
			KPIStateChip->SetColorAndOpacity(FSlateColor(GetStateColor(State)));
		}
		else
		{
			KPIStateChip->SetText(LOCTEXT("KPIStateDisc", "--"));
			KPIStateChip->SetColorAndOpacity(FSlateColor(PGX::Text::Muted));
		}
	}

	if (KPILevelsChip.IsValid())
	{
		const int32 Count = BoundSubsystem.IsValid() ? BoundSubsystem->GetRegisteredLevelCount() : 0;
		KPILevelsChip->SetText(FText::AsNumber(Count));
	}

	if (KPIProfilesChip.IsValid())
	{
		const int32 Count = BoundSubsystem.IsValid() ? BoundSubsystem->GetDiscoveredProfileCount() : 0;
		KPIProfilesChip->SetText(FText::AsNumber(Count));
	}
}

// ============================================================================
// Helpers
// ============================================================================

FLinearColor SPGXLevelFlowInspectorTab::GetStateColor(EPGXLevelFlowState State)
{
	switch (State)
	{
	case EPGXLevelFlowState::Idle:          return PGX::GetStatusPaletteColor(PGX::EStatusPalette::Idle);
	case EPGXLevelFlowState::Preparing:     return PGX::GetStatusPaletteColor(PGX::EStatusPalette::Preparing);
	case EPGXLevelFlowState::Loading:       return PGX::GetStatusPaletteColor(PGX::EStatusPalette::Loading);
	case EPGXLevelFlowState::Transitioning: return PGX::GetStatusPaletteColor(PGX::EStatusPalette::Transitioning);
	case EPGXLevelFlowState::PostLoad:      return PGX::GetStatusPaletteColor(PGX::EStatusPalette::PostLoad);
	case EPGXLevelFlowState::Complete:      return PGX::GetStatusPaletteColor(PGX::EStatusPalette::Complete);
	case EPGXLevelFlowState::Failed:        return PGX::GetStatusPaletteColor(PGX::EStatusPalette::Failed);
	default:                                return PGX::GetStatusPaletteColor(PGX::EStatusPalette::Unknown);
	}
}

FText SPGXLevelFlowInspectorTab::GetStateText(EPGXLevelFlowState State)
{
	switch (State)
	{
	case EPGXLevelFlowState::Idle:          return LOCTEXT("StateIdle2", "Idle");
	case EPGXLevelFlowState::Preparing:     return LOCTEXT("StatePreparing", "Preparing");
	case EPGXLevelFlowState::Loading:       return LOCTEXT("StateLoading", "Loading");
	case EPGXLevelFlowState::Transitioning: return LOCTEXT("StateTransitioning", "Transitioning");
	case EPGXLevelFlowState::PostLoad:      return LOCTEXT("StatePostLoad", "PostLoad");
	case EPGXLevelFlowState::Complete:      return LOCTEXT("StateComplete", "Complete");
	case EPGXLevelFlowState::Failed:        return LOCTEXT("StateFailed", "Failed");
	default:                                return LOCTEXT("StateUnknown", "Unknown");
	}
}

FText SPGXLevelFlowInspectorTab::GetLoadStrategyText(EPGXLoadStrategy Strategy)
{
	switch (Strategy)
	{
	case EPGXLoadStrategy::AsyncLoad:      return LOCTEXT("StratAsync", "AsyncLoad");
	case EPGXLoadStrategy::SyncLoad:       return LOCTEXT("StratSync", "SyncLoad");
	case EPGXLoadStrategy::StreamSubLevel: return LOCTEXT("StratStream", "StreamSubLevel");
	default:                               return LOCTEXT("StratUnknown", "Unknown");
	}
}

FText SPGXLevelFlowInspectorTab::GetTransitionModeText(EPGXTransitionMode Mode)
{
	switch (Mode)
	{
	case EPGXTransitionMode::Instant:         return LOCTEXT("ModeInstant", "Instant");
	case EPGXTransitionMode::FadeOut_FadeIn:  return LOCTEXT("ModeFade", "Fade");
	case EPGXTransitionMode::Custom:          return LOCTEXT("ModeCustom", "Custom");
	default:                                  return LOCTEXT("ModeUnknown", "Unknown");
	}
}

#undef LOCTEXT_NAMESPACE
