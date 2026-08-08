// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "GameFlowInspector/SPGXGameFlowInspectorTab.h"
#include "GameFlowInspector/SPGXGameFlowChannelRow.h"
#include "PGXGameFlowSubsystem.h"
#include "Logging/PGXLogMacros.h"

#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/STableRow.h"
#include "Styling/AppStyle.h"
#include "Utils/PGXEditorUtils.h"
#include "Style/PGXVisualTokens.h"
#include "Widgets/SPGXPremiumShell.h"
#include "Widgets/SPGXSectionDivider.h"
#include "Style/PGXEditorStyle.h"
#include "Widgets/SPGXKPIChip.h"
#include "Widgets/SPGXStatusBadge.h"
#include "Widgets/SPGXHealthDot.h"
#include "Utils/SPGXTelemetryGraph.h"
#include "Widgets/SPGXSparkline.h"
#include "Editor.h"
#include "Engine/GameInstance.h"

// EN: GameFlow Inspector - Real-time visualization of 8 FSM channels
// ES: GameFlow Inspector - Visualizacion en tiempo real de 8 canales FSM

#define LOCTEXT_NAMESPACE "PGXGameFlowInspector"
DEFINE_LOG_CATEGORY_STATIC(LogPGXGameFlowInspector, Log, All);

// EN: GameFlow system color now from PGX::System::GameFlow token
// ES: Color del sistema GameFlow ahora desde token PGX::System::GameFlow

// ============================================================================
// EN: Construction / Destruction
// ES: Construccion / Destruccion
// ============================================================================

void SPGXGameFlowInspectorTab::Construct(const FArguments& /*InArgs*/)
{
	PGX_LOG_INFO(LogPGXGameFlowInspector, TEXT("[GameFlow] Construct"));
	// EN: Initialize 8 channel summaries / ES: Inicializar 8 resumenes de canal
	for (int32 i = 0; i < PGX_FLOW_CHANNEL_COUNT; ++i)
	{
		TSharedPtr<FPGXFlowChannelSummary> Summary = MakeShared<FPGXFlowChannelSummary>();
		Summary->Channel = static_cast<EPGXFlowChannel>(i);
		Summary->ChannelName = UPGXGameFlowSubsystem::GetChannelName(Summary->Channel);
		ChannelSummaries.Add(Summary);
	}

	ChildSlot
	[
		SNew(SPGXPremiumShell)
		.SystemColor(PGX::System::GameFlow)
		.Title(LOCTEXT("PanelTitle", "PGX GAMEFLOW INSPECTOR"))
		.Subtitle(LOCTEXT("PanelSubtitle", "8-Channel FSM"))
		.Icon(FPGXEditorStyle::Get().GetBrush("PGXEditor.Icon.GameFlow"))
		.FooterLeftContent()
		[
			SAssignNew(StatusText, STextBlock)
			.Text(LOCTEXT("StatusDisconnected", "Not connected - Start PIE to inspect GameFlow channels"))
			.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
		]
		.Content()
		[
			SNew(SVerticalBox)

			// EN: Toolbar / ES: Barra de herramientas
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(PGX::Spacing::SM)
			[
				BuildToolbar()
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSeparator)
			]

			// EN: Main content area (scrollable) / ES: Area de contenido principal (scrollable)
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SScrollBox)

				// EN: Panel 1 - Current States / ES: Panel 1 - Estados Actuales
				+ SScrollBox::Slot()
				.Padding(4.0f)
				[
					BuildCurrentStatesPanel()
				]

				// EN: Transition Rate Telemetry / ES: Telemetria de Tasa de Transiciones
				+ SScrollBox::Slot()
				.Padding(4.0f)
				[
					BuildTelemetrySection()
				]

				+ SScrollBox::Slot()
				.Padding(4.0f, 0.0f)
				[
					SNew(SSeparator)
				]

				// EN: Panel 2 - Transition History / ES: Panel 2 - Historial de Transiciones
				+ SScrollBox::Slot()
				.Padding(4.0f)
				[
					BuildTransitionHistoryPanel()
				]

				+ SScrollBox::Slot()
				.Padding(4.0f, 0.0f)
				[
					SNew(SSeparator)
				]

				// EN: Panel 3 - Channel Detail / ES: Panel 3 - Detalle de Canal
				+ SScrollBox::Slot()
				.Padding(4.0f)
				[
					BuildChannelDetailPanel()
				]
			]
		]
	];

	BindPIEDelegates();

	// EN: If PIE is already running when the panel opens, bind immediately
	// ES: Si PIE ya esta corriendo cuando se abre el panel, bindear inmediatamente
	if (GEditor && GEditor->PlayWorld)
	{
		BindToSubsystem();
	}
}

SPGXGameFlowInspectorTab::~SPGXGameFlowInspectorTab()
{
	PGX_LOG_INFO(LogPGXGameFlowInspector, TEXT("[GameFlow] Destructor — cleaning up delegates"));
	UnbindFromSubsystem();

	if (PIEStartedHandle.IsValid())
	{
		FEditorDelegates::PostPIEStarted.Remove(PIEStartedHandle);
	}
	if (PIEEndedHandle.IsValid())
	{
		FEditorDelegates::EndPIE.Remove(PIEEndedHandle);
	}
}

// ============================================================================
// EN: UI Build
// ES: Construccion de UI
// ============================================================================

TSharedRef<SWidget> SPGXGameFlowInspectorTab::BuildToolbar()
{
	return SNew(SHorizontalBox)

		// EN: KPI chip: Active channels / ES: Chip KPI: Canales activos
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4.0f, 0.0f)
		[
			SNew(SPGXKPIChip)
			.Label(LOCTEXT("KPIActiveLbl", "ACTIVE"))
			.Value_Lambda([this]() -> FText
			{
				int32 Active = 0;
				for (const auto& S : ChannelSummaries) { if (S->CurrentTag.IsValid()) Active++; }
				return FText::FromString(FString::Printf(TEXT("%d/%d"), Active, PGX_FLOW_CHANNEL_COUNT));
			})
			.AccentColor(PGX::System::GameFlow)
		]

		// EN: KPI chip: Transition count / ES: Chip KPI: Conteo de transiciones
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4.0f, 0.0f)
		[
			SNew(SPGXKPIChip)
			.Label(LOCTEXT("KPITransLbl", "TRANSITIONS"))
			.Value_Lambda([this]() -> FText
			{
				return FText::AsNumber(AllHistoryEntries.Num());
			})
			.AccentColor(PGX::Semantic::Info)
		]

		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(SSpacer)
		]

		// EN: Auto-scroll checkbox / ES: Checkbox de auto-scroll
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4.0f, 0.0f)
		[
			SNew(SCheckBox)
			.IsChecked_Lambda([this]() { return bAutoScroll ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
			.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) { bAutoScroll = (NewState == ECheckBoxState::Checked); })
			[
				SNew(STextBlock)
				.Text(LOCTEXT("AutoScroll", "Auto-scroll"))
				.Font(PGX::Font::BodySmall())
			]
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(4.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Refresh", "Refresh"))
			.ToolTipText(LOCTEXT("RefreshTooltip", "Refresh all panels from subsystem state"))
			.OnClicked(this, &SPGXGameFlowInspectorTab::OnRefreshClicked)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Clear", "Clear"))
			.ToolTipText(LOCTEXT("ClearTooltip", "Clear transition history"))
			.OnClicked(this, &SPGXGameFlowInspectorTab::OnClearClicked)
		];
}

TSharedRef<SWidget> SPGXGameFlowInspectorTab::BuildCurrentStatesPanel()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 2.0f)
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("StatesHeader", "CURRENT STATES"))
			.AccentColor(PGX::System::GameFlow)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(ChannelListView, SListView<TSharedPtr<FPGXFlowChannelSummary>>)
			.ListItemsSource(&ChannelSummaries)
			.OnGenerateRow(this, &SPGXGameFlowInspectorTab::OnGenerateChannelRow)
			.OnSelectionChanged(this, &SPGXGameFlowInspectorTab::OnChannelSelected)
			.SelectionMode(ESelectionMode::Single)
		];
}

TSharedRef<SWidget> SPGXGameFlowInspectorTab::BuildTransitionHistoryPanel()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 2.0f)
		[
			SAssignNew(HistoryHeaderText, STextBlock)
			.Text(LOCTEXT("HistoryHeaderAll", "TRANSITION HISTORY (all channels)"))
			.Font(PGX::Font::Badge())
			.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.MaxHeight(300.0f)
		[
			SAssignNew(HistoryListView, SListView<TSharedPtr<FPGXFlowTransitionRecord>>)
			.ListItemsSource(&FilteredHistoryEntries)
			.OnGenerateRow(this, &SPGXGameFlowInspectorTab::OnGenerateHistoryRow)
			.SelectionMode(ESelectionMode::None)
		];
}

TSharedRef<SWidget> SPGXGameFlowInspectorTab::BuildChannelDetailPanel()
{
	// EN: Helper lambda to build a labeled row / ES: Lambda helper para construir una fila con label
	auto MakeDetailRow = [](const FText& Label, TSharedPtr<STextBlock>& OutText) -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(120.0f)
				[
					SNew(STextBlock)
					.Text(Label)
					.Font(PGX::Font::Badge())
					.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SAssignNew(OutText, STextBlock)
				.Text(LOCTEXT("DetailDash", "--"))
				.Font(PGX::Font::BodySmall())
			];
	};

	// EN: Build state section (always visible when channel selected)
	// ES: Construir seccion de estado (siempre visible cuando hay canal seleccionado)
	TSharedRef<SVerticalBox> StateSection = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(4.0f, 2.0f)
		[ MakeDetailRow(LOCTEXT("LblCurrent", "Current State:"), DetailCurrentText) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(4.0f, 2.0f)
		[ MakeDetailRow(LOCTEXT("LblPrevious", "Previous State:"), DetailPreviousText) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(4.0f, 2.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(120.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("LblCanRevert", "Can Revert:"))
					.Font(PGX::Font::Badge())
					.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(DetailCanRevertBadge, SBorder)
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderBackgroundColor(PGX::Text::Muted)
				.Padding(FMargin(6.0f, 1.0f))
				[
					SAssignNew(DetailCanRevertText, STextBlock)
					.Text(LOCTEXT("RevertNo", "NO"))
					.Font(PGX::Font::CaptionBold())
					.ColorAndOpacity(FSlateColor(FLinearColor::White))
				]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(4.0f, 2.0f)
		[ MakeDetailRow(LOCTEXT("LblHistDepth", "History Depth:"), DetailHistoryDepthText) ];

	DetailStateSection = StateSection;

	// EN: Build rule section (visible only when rule found)
	// ES: Construir seccion de regla (visible solo cuando se encuentra regla)
	TSharedRef<SVerticalBox> RuleSection = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 2.0f)
		[ SNew(SPGXSectionDivider).Title(LOCTEXT("RuleHeader", "ACTIVE RULE")).AccentColor(PGX::System::GameFlow) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(4.0f, 2.0f)
		[ MakeDetailRow(LOCTEXT("LblRuleName", "Name:"), DetailRuleNameText) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(4.0f, 2.0f)
		[ MakeDetailRow(LOCTEXT("LblRuleDesc", "Description:"), DetailRuleDescText) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(4.0f, 2.0f)
		[ MakeDetailRow(LOCTEXT("LblRuleRevert", "AllowRevert:"), DetailRuleRevertText) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(4.0f, 2.0f)
		[ MakeDetailRow(LOCTEXT("LblRuleError", "ErrorCode:"), DetailRuleErrorText) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(4.0f, 4.0f, 0.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("AllowedHeader", "Allowed Destinations:"))
			.Font(PGX::Font::Badge())
			.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(12.0f, 0.0f, 0.0f, 4.0f)
		[ SAssignNew(DetailAllowedBox, SVerticalBox) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(4.0f, 4.0f, 0.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("DisallowedHeader", "Disallowed Queries:"))
			.Font(PGX::Font::Badge())
			.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(12.0f, 0.0f, 0.0f, 4.0f)
		[ SAssignNew(DetailDisallowedBox, SVerticalBox) ];

	DetailRuleSection = RuleSection;
	DetailRuleSection->SetVisibility(EVisibility::Collapsed);

	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 2.0f)
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("DetailHeader", "CHANNEL DETAIL"))
			.AccentColor(PGX::System::GameFlow)
		]

		// EN: Empty state (visible when no channel selected)
		// ES: Estado vacio (visible cuando no hay canal seleccionado)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f, 4.0f)
		[
			SAssignNew(DetailEmptyText, STextBlock)
			.Text(LOCTEXT("DetailEmpty", "Select a channel above to view full state details"))
			.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
		]

		// EN: State section / ES: Seccion de estado
		+ SVerticalBox::Slot()
		.AutoHeight()
		[ StateSection ]

		// EN: Rule section / ES: Seccion de regla
		+ SVerticalBox::Slot()
		.AutoHeight()
		[ RuleSection ]

		// EN: Debug actions / ES: Acciones de debug
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f, 8.0f, 0.0f, 4.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(RevertButton, SButton)
				.Text(LOCTEXT("RevertBtn", "Revert to Previous"))
				.ToolTipText(LOCTEXT("RevertTip", "Revert selected channel to its previous state"))
				.IsEnabled(false)
				.OnClicked(this, &SPGXGameFlowInspectorTab::OnRevertClicked)
			]
		];
}

// ============================================================================
// EN: Transition Rate Telemetry / ES: Telemetria de Tasa de Transiciones
// ============================================================================

TSharedRef<SWidget> SPGXGameFlowInspectorTab::BuildTelemetrySection()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 2.0f)
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("TelemetryHeader", "TRANSITION RATE"))
			.AccentColor(PGX::System::GameFlow)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)
			.HeightOverride(PGX::Height::GraphDefault)
			[
				SAssignNew(TransitionRateGraph, SPGXTelemetryGraph)
				.BufferSize(128)
				.LineColor(PGX::System::GameFlow)
				.MinValue(0.0f)
				.MaxValue(20.0f)
				.bShowGrid(true)
				.bShowAvgLine(true)
				.bShowLabels(true)
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			PGXEditorUtils::BuildGraphLegend(TransitionRateGraph, PGX::System::GameFlow)
		];
}

// ============================================================================
// EN: Tick — Sparkline accumulator / ES: Tick — Acumulador de sparklines
// ============================================================================

void SPGXGameFlowInspectorTab::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	SparklineAccumulator += static_cast<float>(InDeltaTime);
	if (SparklineAccumulator < 1.0f) return;
	SparklineAccumulator = 0.0f;

	// EN: Push counter values to sparklines and sum to graph / ES: Enviar valores de contadores a sparklines y suma al grafo
	int32 TotalTransitions = 0;
	for (auto& Pair : TransitionCounters)
	{
		TotalTransitions += Pair.Value;

		TSharedPtr<SPGXSparkline>* SparklinePtr = ChannelSparklines.Find(Pair.Key);
		if (SparklinePtr && SparklinePtr->IsValid())
		{
			(*SparklinePtr)->PushValue(static_cast<float>(Pair.Value));
		}

		Pair.Value = 0;
	}

	if (TransitionRateGraph.IsValid())
	{
		TransitionRateGraph->PushValue(static_cast<float>(TotalTransitions));
	}
}

// ============================================================================
// EN: SListView row generators
// ES: Generadores de filas SListView
// ============================================================================

TSharedRef<ITableRow> SPGXGameFlowInspectorTab::OnGenerateChannelRow(
	TSharedPtr<FPGXFlowChannelSummary> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	const FLinearColor Color = GetChannelColor(Item->Channel);

	TSharedPtr<SPGXGameFlowChannelRow> RowWidget;

	// EN: Look up or create per-channel sparkline / ES: Buscar o crear sparkline por canal
	TSharedPtr<SPGXSparkline>& Sparkline = ChannelSparklines.FindOrAdd(Item->Channel);
	if (!Sparkline.IsValid())
	{
		SAssignNew(Sparkline, SPGXSparkline)
		.LineColor(Color);
	}

	// EN: HealthDot state: valid tag → Active, no tag → Offline
	// ES: Estado HealthDot: tag valido → Active, sin tag → Offline
	const EPGXHealthState DotState = Item->CurrentTag.IsValid()
		? EPGXHealthState::Active
		: EPGXHealthState::Offline;

	TSharedRef<ITableRow> TableRow = SNew(STableRow<TSharedPtr<FPGXFlowChannelSummary>>, OwnerTable)
		.Padding(FMargin(0.0f, 1.0f))
		[
			SNew(SHorizontalBox)

			// EN: HealthDot / ES: HealthDot
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0, 0, 4, 0)
			[
				SNew(SPGXHealthDot)
				.State(DotState)
			]

			// EN: Existing channel row widget / ES: Widget de fila de canal existente
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SAssignNew(RowWidget, SPGXGameFlowChannelRow)
				.Summary(Item)
				.Subsystem(BoundSubsystem)
				.ChannelColor(Color)
			]

			// EN: Per-channel sparkline / ES: Sparkline por canal
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4, 0, 0, 0)
			[
				SNew(SBox)
				.WidthOverride(40.0f)
				.HeightOverride(PGX::Height::SparklineDefault)
				[
					Sparkline.ToSharedRef()
				]
			]
		];

	// EN: Cache widget reference for live updates / ES: Cachear referencia del widget para actualizaciones en vivo
	ChannelRowWidgets.Add(Item->Channel, RowWidget);

	return TableRow;
}

TSharedRef<ITableRow> SPGXGameFlowInspectorTab::OnGenerateHistoryRow(
	TSharedPtr<FPGXFlowTransitionRecord> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	const FLinearColor ChannelCol = GetChannelColor(Item->Channel);
	const FString TimeStr = Item->Timestamp.ToString(TEXT("%H:%M:%S.%s"));
	const FString NewTagStr = PGXEditorUtils::TagToLeafName(Item->NewTag);
	const FString NewTagFull = Item->NewTag.IsValid() ? Item->NewTag.ToString() : TEXT("None");
	const FString PrevTagStr = PGXEditorUtils::TagToLeafName(Item->PreviousTag);
	const FString PrevTagFull = Item->PreviousTag.IsValid() ? Item->PreviousTag.ToString() : TEXT("None");

	// EN: Inline expandable row (NoBorder SButton pattern with shared state)
	// ES: Fila expandible inline (patron NoBorder SButton con estado compartido)
	struct FHistoryRowState
	{
		bool bExpanded = false;
		TSharedPtr<SWidget> ExpandedPanel;
	};
	TSharedRef<FHistoryRowState> RowState = MakeShared<FHistoryRowState>();
	TSharedRef<SVerticalBox> RowRoot = SNew(SVerticalBox);

	RowRoot->AddSlot()
		.AutoHeight()
		[
			SNew(SButton)
			.ButtonStyle(FAppStyle::Get(), "NoBorder")
			.OnClicked_Lambda([RowState, RowRoot, Item, ChannelCol, TimeStr, NewTagStr, PrevTagStr, NewTagFull, PrevTagFull]() mutable -> FReply
			{
				RowState->bExpanded = !RowState->bExpanded;

				if (RowState->bExpanded && !RowState->ExpandedPanel.IsValid())
				{
					// EN: Build expanded detail panel / ES: Construir panel expandido de detalle
					TSharedRef<SVerticalBox> Detail = SNew(SVerticalBox);

					Detail->AddSlot().AutoHeight().Padding(20.0f, 2.0f, 0.0f, 1.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(FString::Printf(TEXT("Channel: %s"), *Item->ChannelName)))
						.Font(PGX::Font::Caption())
						.ColorAndOpacity(FSlateColor(ChannelCol))
					];

					Detail->AddSlot().AutoHeight().Padding(20.0f, 1.0f, 0.0f, 1.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(FString::Printf(TEXT("New Tag: %s"), *NewTagStr)))
						.ToolTipText(FText::FromString(NewTagFull))
						.Font(PGX::Font::Caption())
						.ColorAndOpacity(FSlateColor(PGX::Semantic::Good))
					];

					Detail->AddSlot().AutoHeight().Padding(20.0f, 1.0f, 0.0f, 1.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(FString::Printf(TEXT("Previous: %s"), *PrevTagStr)))
						.ToolTipText(FText::FromString(PrevTagFull))
						.Font(PGX::Font::Caption())
						.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
					];

					Detail->AddSlot().AutoHeight().Padding(20.0f, 1.0f, 0.0f, 1.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(FString::Printf(TEXT("Timestamp: %s"), *Item->Timestamp.ToString(TEXT("%Y-%m-%d %H:%M:%S.%s")))))
						.Font(PGX::Font::Caption())
						.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
					];

					Detail->AddSlot().AutoHeight().Padding(20.0f, 1.0f, 0.0f, 4.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(FString::Printf(TEXT("Source: %s"), *Item->SourceName)))
						.Font(PGX::Font::Caption())
						.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
					];

					RowState->ExpandedPanel = Detail;
					RowRoot->AddSlot().AutoHeight()[ Detail ];
				}
				else if (RowState->ExpandedPanel.IsValid())
				{
					RowState->ExpandedPanel->SetVisibility(RowState->bExpanded ? EVisibility::Visible : EVisibility::Collapsed);
				}

				return FReply::Handled();
			})
			[
				SNew(SHorizontalBox)

				// EN: Timestamp (mono) / ES: Marca de tiempo (mono)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(100.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TimeStr))
						.Font(PGX::Font::Mono())
						.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
					]
				]

				// EN: Channel name (colored) / ES: Nombre de canal (con color)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4.0f, 0.0f)
				[
					SNew(SBox)
					.WidthOverride(90.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(Item->ChannelName))
						.Font(PGX::Font::Badge())
						.ColorAndOpacity(FSlateColor(ChannelCol))
					]
				]

				// EN: Arrow + New tag / ES: Flecha + Nuevo tag
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				.Padding(4.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(FString::Printf(TEXT("-> %s"), *NewTagStr)))
					.ToolTipText(FText::FromString(NewTagFull))
					.Font(PGX::Font::BodySmall())
					.ColorAndOpacity(FSlateColor(PGX::Text::OnColor))
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				]

				// EN: From: previous tag (gray) / ES: From: tag anterior (gris)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4.0f, 0.0f)
				[
					SNew(SBox)
					.WidthOverride(200.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(FString::Printf(TEXT("from: %s"), *PrevTagStr)))
						.ToolTipText(FText::FromString(PrevTagFull))
						.Font(PGX::Font::Caption())
						.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					]
				]
			]
		];

	return SNew(STableRow<TSharedPtr<FPGXFlowTransitionRecord>>, OwnerTable)
		.Padding(FMargin(0.0f, 1.0f))
		[
			RowRoot
		];
}

// ============================================================================
// EN: Channel selection
// ES: Seleccion de canal
// ============================================================================

void SPGXGameFlowInspectorTab::OnChannelSelected(
	TSharedPtr<FPGXFlowChannelSummary> Item, ESelectInfo::Type /*SelectInfo*/)
{
	if (!Item.IsValid())
	{
		SelectedChannel = EPGXFlowChannel::MAX;
	}
	else
	{
		SelectedChannel = Item->Channel;
	}

	RefreshHistoryForChannel();
	RefreshChannelDetail();
}

// ============================================================================
// EN: Refresh methods
// ES: Metodos de refresh
// ============================================================================

void SPGXGameFlowInspectorTab::RefreshChannelStates()
{
	UPGXGameFlowSubsystem* Sub = BoundSubsystem.Get();

	for (int32 i = 0; i < PGX_FLOW_CHANNEL_COUNT; ++i)
	{
		if (!ChannelSummaries.IsValidIndex(i))
		{
			continue;
		}

		TSharedPtr<FPGXFlowChannelSummary>& Summary = ChannelSummaries[i];
		const EPGXFlowChannel Ch = static_cast<EPGXFlowChannel>(i);

		if (Sub)
		{
			Summary->CurrentTag = Sub->GetCurrentFlowTag(Ch);
			Summary->LastTag = Sub->GetLastFlowTag(Ch);
			Summary->HistoryCount = Sub->GetChannelHistory(Ch).Num();
			Summary->bCanRevert = Sub->CheckCanRevert(Ch);
		}
		else
		{
			Summary->CurrentTag = FGameplayTag();
			Summary->LastTag = FGameplayTag();
			Summary->HistoryCount = 0;
			Summary->bCanRevert = false;
		}

		// EN: Update cached row widget if present / ES: Actualizar widget de fila cacheado si existe
		TSharedPtr<SPGXGameFlowChannelRow>* RowPtr = ChannelRowWidgets.Find(Ch);
		if (RowPtr && RowPtr->IsValid())
		{
			(*RowPtr)->UpdateSummary(*Summary);
		}
	}

	if (ChannelListView.IsValid())
	{
		ChannelListView->RequestListRefresh();
	}

	RefreshKPIChips();
}

void SPGXGameFlowInspectorTab::RefreshHistoryForChannel()
{
	FilteredHistoryEntries.Empty();

	if (SelectedChannel == EPGXFlowChannel::MAX)
	{
		// EN: Show all / ES: Mostrar todos
		FilteredHistoryEntries = AllHistoryEntries;

		if (HistoryHeaderText.IsValid())
		{
			HistoryHeaderText->SetText(LOCTEXT("HistoryHeaderAll", "TRANSITION HISTORY (all channels)"));
		}
	}
	else
	{
		for (const TSharedPtr<FPGXFlowTransitionRecord>& Entry : AllHistoryEntries)
		{
			if (Entry->Channel == SelectedChannel)
			{
				FilteredHistoryEntries.Add(Entry);
			}
		}

		if (HistoryHeaderText.IsValid())
		{
			const FString ChName = UPGXGameFlowSubsystem::GetChannelName(SelectedChannel);
			HistoryHeaderText->SetText(FText::FromString(
				FString::Printf(TEXT("TRANSITION HISTORY (%s)"), *ChName)));
		}
	}

	if (HistoryListView.IsValid())
	{
		HistoryListView->RequestListRefresh();

		if (bAutoScroll && FilteredHistoryEntries.Num() > 0)
		{
			HistoryListView->RequestScrollIntoView(FilteredHistoryEntries.Last());
		}
	}
}

void SPGXGameFlowInspectorTab::RefreshChannelDetail()
{
	if (SelectedChannel == EPGXFlowChannel::MAX)
	{
		// EN: No channel selected — show empty state, hide structured content
		// ES: Ningun canal seleccionado — mostrar estado vacio, ocultar contenido estructurado
		if (DetailEmptyText.IsValid()) DetailEmptyText->SetVisibility(EVisibility::Visible);
		if (DetailStateSection.IsValid()) DetailStateSection->SetVisibility(EVisibility::Collapsed);
		if (DetailRuleSection.IsValid()) DetailRuleSection->SetVisibility(EVisibility::Collapsed);
		if (RevertButton.IsValid()) RevertButton->SetEnabled(false);
		return;
	}

	// EN: Channel selected — show structured content, hide empty state
	// ES: Canal seleccionado — mostrar contenido estructurado, ocultar estado vacio
	if (DetailEmptyText.IsValid()) DetailEmptyText->SetVisibility(EVisibility::Collapsed);
	if (DetailStateSection.IsValid()) DetailStateSection->SetVisibility(EVisibility::Visible);

	UPGXGameFlowSubsystem* Sub = BoundSubsystem.Get();
	if (!Sub)
	{
		if (DetailCurrentText.IsValid()) DetailCurrentText->SetText(LOCTEXT("NoSub", "(disconnected)"));
		if (DetailRuleSection.IsValid()) DetailRuleSection->SetVisibility(EVisibility::Collapsed);
		if (RevertButton.IsValid()) RevertButton->SetEnabled(false);
		return;
	}

	// EN: Section A: Channel state / ES: Seccion A: Estado del canal
	const FGameplayTag CurrentTag = Sub->GetCurrentFlowTag(SelectedChannel);
	const FGameplayTag LastTag = Sub->GetLastFlowTag(SelectedChannel);
	const bool bCanRevert = Sub->CheckCanRevert(SelectedChannel);
	const TArray<FPGXFlowHistoryEntry> History = Sub->GetChannelHistory(SelectedChannel);

	if (DetailCurrentText.IsValid())
	{
		DetailCurrentText->SetText(FText::FromString(PGXEditorUtils::TagToLeafName(CurrentTag)));
		DetailCurrentText->SetToolTipText(FText::FromString(CurrentTag.IsValid() ? CurrentTag.ToString() : TEXT("None")));
	}
	if (DetailPreviousText.IsValid())
	{
		DetailPreviousText->SetText(FText::FromString(PGXEditorUtils::TagToLeafName(LastTag)));
		DetailPreviousText->SetToolTipText(FText::FromString(LastTag.IsValid() ? LastTag.ToString() : TEXT("None")));
	}
	if (DetailCanRevertBadge.IsValid() && DetailCanRevertText.IsValid())
	{
		DetailCanRevertText->SetText(bCanRevert ? LOCTEXT("Yes", "YES") : LOCTEXT("No", "NO"));
		DetailCanRevertBadge->SetBorderBackgroundColor(bCanRevert
			? PGX::Semantic::Good
			: PGX::Semantic::Error);
	}
	if (DetailHistoryDepthText.IsValid())
	{
		DetailHistoryDepthText->SetText(FText::AsNumber(History.Num()));
	}
	if (RevertButton.IsValid())
	{
		RevertButton->SetEnabled(bCanRevert);
	}

	// EN: Section B: Active rule / ES: Seccion B: Regla activa
	FPGXFlowRule OutRule;
	if (Sub->GetAllowedTransitionByCurrentFlowTag(SelectedChannel, OutRule))
	{
		if (DetailRuleSection.IsValid()) DetailRuleSection->SetVisibility(EVisibility::Visible);

		if (DetailRuleNameText.IsValid())
			DetailRuleNameText->SetText(FText::FromString(OutRule.RuleName.ToString()));
		if (DetailRuleDescText.IsValid())
			DetailRuleDescText->SetText(OutRule.Description.IsEmpty()
				? LOCTEXT("NoDesc", "(none)") : FText::FromString(OutRule.Description.ToString()));
		if (DetailRuleRevertText.IsValid())
			DetailRuleRevertText->SetText(OutRule.bAllowRevert ? LOCTEXT("Yes2", "Yes") : LOCTEXT("No2", "No"));
		if (DetailRuleErrorText.IsValid())
			DetailRuleErrorText->SetText(FText::AsNumber(OutRule.ErrorCode));

		// EN: Allowed destinations / ES: Destinos permitidos
		if (DetailAllowedBox.IsValid())
		{
			DetailAllowedBox->ClearChildren();
			if (OutRule.AllowedDestinations.Num() > 0)
			{
				for (const FGameplayTag& DestTag : OutRule.AllowedDestinations)
				{
					DetailAllowedBox->AddSlot().AutoHeight().Padding(0.0f, 1.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(FString::Printf(TEXT("- %s"), *PGXEditorUtils::TagToLeafName(DestTag))))
						.ToolTipText(FText::FromString(DestTag.ToString()))
						.Font(PGX::Font::BodySmall())
						.ColorAndOpacity(FSlateColor(PGX::Semantic::Good))
					];
				}
			}
			else
			{
				DetailAllowedBox->AddSlot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("AllAllowed", "(all — no whitelist)"))
					.Font(PGX::Font::Hint())
					.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
				];
			}
		}

		// EN: Disallowed queries / ES: Queries deshabilitadas
		if (DetailDisallowedBox.IsValid())
		{
			DetailDisallowedBox->ClearChildren();
			if (OutRule.DisallowedTagQueries.Num() > 0)
			{
				for (const FGameplayTag& VetoTag : OutRule.DisallowedTagQueries)
				{
					DetailDisallowedBox->AddSlot().AutoHeight().Padding(0.0f, 1.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(FString::Printf(TEXT("- %s"), *PGXEditorUtils::TagToLeafName(VetoTag))))
						.ToolTipText(FText::FromString(VetoTag.ToString()))
						.Font(PGX::Font::BodySmall())
						.ColorAndOpacity(FSlateColor(PGX::Semantic::Error))
					];
				}
			}
			else
			{
				DetailDisallowedBox->AddSlot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("NoneDisallowed", "(none)"))
					.Font(PGX::Font::Hint())
					.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
				];
			}
		}
	}
	else
	{
		if (DetailRuleSection.IsValid()) DetailRuleSection->SetVisibility(EVisibility::Collapsed);
	}

	PGX_LOG_INFO(LogPGXGameFlowInspector, TEXT("[GameFlow] RefreshChannelDetail — channel=%d, current=%s, history=%d"),
		(int32)SelectedChannel,
		CurrentTag.IsValid() ? *CurrentTag.ToString() : TEXT("None"),
		History.Num());
}

void SPGXGameFlowInspectorTab::LoadExistingHistory()
{
	UPGXGameFlowSubsystem* Sub = BoundSubsystem.Get();
	if (!Sub)
	{
		return;
	}

	// EN: Load history from all channels into our local records
	// ES: Cargar historial de todos los canales en nuestros registros locales
	for (int32 i = 0; i < PGX_FLOW_CHANNEL_COUNT; ++i)
	{
		const EPGXFlowChannel Ch = static_cast<EPGXFlowChannel>(i);
		const TArray<FPGXFlowHistoryEntry> History = Sub->GetChannelHistory(Ch);
		const FString ChName = UPGXGameFlowSubsystem::GetChannelName(Ch);

		for (int32 j = 0; j < History.Num(); ++j)
		{
			TSharedPtr<FPGXFlowTransitionRecord> Record = MakeShared<FPGXFlowTransitionRecord>();
			Record->Channel = Ch;
			Record->NewTag = History[j].FlowTag;
			Record->PreviousTag = (j > 0) ? History[j - 1].FlowTag : FGameplayTag();
			Record->Timestamp = History[j].Timestamp;
			Record->SourceName = TEXT("(pre-existing)");
			Record->ChannelName = ChName;
			AllHistoryEntries.Add(Record);
		}
	}

	// EN: Sort by timestamp / ES: Ordenar por timestamp
	AllHistoryEntries.Sort([](const TSharedPtr<FPGXFlowTransitionRecord>& A, const TSharedPtr<FPGXFlowTransitionRecord>& B)
	{
		return A->Timestamp < B->Timestamp;
	});
}

// ============================================================================
// EN: PIE Lifecycle
// ES: Ciclo de Vida PIE
// ============================================================================

void SPGXGameFlowInspectorTab::BindPIEDelegates()
{
	PIEStartedHandle = FEditorDelegates::PostPIEStarted.AddSP(
		SharedThis(this), &SPGXGameFlowInspectorTab::OnPIEStarted);
	PIEEndedHandle = FEditorDelegates::EndPIE.AddSP(
		SharedThis(this), &SPGXGameFlowInspectorTab::OnPIEEnded);
	PGX_LOG_INFO(LogPGXGameFlowInspector, TEXT("[GameFlow] PIE delegates bound"));
}

void SPGXGameFlowInspectorTab::OnPIEStarted(bool bIsSimulating)
{
	PGX_LOG_INFO(LogPGXGameFlowInspector, TEXT("[GameFlow] PIE started (simulating=%d)"), bIsSimulating);
	BindToSubsystem();
}

void SPGXGameFlowInspectorTab::OnPIEEnded(bool bIsSimulating)
{
	PGX_LOG_INFO(LogPGXGameFlowInspector, TEXT("[GameFlow] PIE ended (simulating=%d)"), bIsSimulating);
	UnbindFromSubsystem();

	if (StatusText.IsValid())
	{
		StatusText->SetText(FText::Format(
			LOCTEXT("StatusDisconnectedRetain", "Disconnected - {0} history entries retained for post-mortem"),
			FText::AsNumber(AllHistoryEntries.Num())));
		StatusText->SetColorAndOpacity(FSlateColor(PGX::Text::Muted));
	}
}

void SPGXGameFlowInspectorTab::BindToSubsystem()
{
	// EN: Find the PIE world's GameInstance and its GameFlowSubsystem
	// ES: Encontrar el GameInstance del mundo PIE y su GameFlowSubsystem
	UPGXGameFlowSubsystem* Sub = nullptr;

	if (GEditor)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::PIE && Context.World())
			{
				UGameInstance* GI = Context.World()->GetGameInstance();
				if (GI)
				{
					Sub = GI->GetSubsystem<UPGXGameFlowSubsystem>();
					break;
				}
			}
		}
	}

	if (!Sub)
	{
		if (StatusText.IsValid())
		{
			StatusText->SetText(LOCTEXT("StatusNoSubsystem", "PIE active but GameFlowSubsystem not found"));
			StatusText->SetColorAndOpacity(FSlateColor(PGX::Semantic::Warn));
		}
		return;
	}

	BoundSubsystem = Sub;
	PGX_LOG_INFO(LogPGXGameFlowInspector, TEXT("[GameFlow] Bound to subsystem"));

	// EN: Bind to native delegate for live updates
	// ES: Bindear a delegado nativo para actualizaciones en vivo
	FlowStateChangedHandle = Sub->OnFlowStateChangedNative.AddSP(
		SharedThis(this), &SPGXGameFlowInspectorTab::OnFlowStateChanged);

	if (StatusText.IsValid())
	{
		StatusText->SetText(FText::Format(
			LOCTEXT("StatusConnected", "Connected to PIE - {0} channels active"),
			FText::AsNumber(PGX_FLOW_CHANNEL_COUNT)));
		StatusText->SetColorAndOpacity(FSlateColor(PGX::Semantic::Good));
	}

	// EN: Load snapshot: existing states + history / ES: Cargar snapshot: estados existentes + historial
	RefreshChannelStates();
	LoadExistingHistory();
	RefreshHistoryForChannel();
	RefreshChannelDetail();
}

void SPGXGameFlowInspectorTab::UnbindFromSubsystem()
{
	UPGXGameFlowSubsystem* Sub = BoundSubsystem.Get();
	if (Sub)
	{
		if (FlowStateChangedHandle.IsValid())
		{
			Sub->OnFlowStateChangedNative.Remove(FlowStateChangedHandle);
			FlowStateChangedHandle.Reset();
		}
	}

	BoundSubsystem.Reset();
}

// ============================================================================
// EN: Delegate callback
// ES: Callback de delegado
// ============================================================================

void SPGXGameFlowInspectorTab::OnFlowStateChanged(
	EPGXFlowChannel Channel, FGameplayTag FlowTag, UObject* Source)
{
	const int32 Idx = static_cast<int32>(Channel);
	if (!ChannelSummaries.IsValidIndex(Idx))
	{
		return;
	}

	// EN: Increment transition counter for sparkline / ES: Incrementar contador de transicion para sparkline
	TransitionCounters.FindOrAdd(Channel)++;

	// EN: Build enriched transition record / ES: Construir registro de transicion enriquecido
	TSharedPtr<FPGXFlowTransitionRecord> Record = MakeShared<FPGXFlowTransitionRecord>();
	Record->Channel = Channel;
	Record->NewTag = FlowTag;
	Record->PreviousTag = ChannelSummaries[Idx]->CurrentTag; // EN: Capture before update / ES: Capturar antes del update
	Record->Timestamp = FDateTime::UtcNow();
	Record->SourceName = Source ? Source->GetName() : TEXT("(none)");
	Record->ChannelName = UPGXGameFlowSubsystem::GetChannelName(Channel);

	// EN: Update channel summary / ES: Actualizar resumen de canal
	UPGXGameFlowSubsystem* Sub = BoundSubsystem.Get();
	if (Sub)
	{
		ChannelSummaries[Idx]->CurrentTag = Sub->GetCurrentFlowTag(Channel);
		ChannelSummaries[Idx]->LastTag = Sub->GetLastFlowTag(Channel);
		ChannelSummaries[Idx]->HistoryCount = Sub->GetChannelHistory(Channel).Num();
		ChannelSummaries[Idx]->bCanRevert = Sub->CheckCanRevert(Channel);

		// EN: Update row widget / ES: Actualizar widget de fila
		TSharedPtr<SPGXGameFlowChannelRow>* RowPtr = ChannelRowWidgets.Find(Channel);
		if (RowPtr && RowPtr->IsValid())
		{
			(*RowPtr)->UpdateSummary(*ChannelSummaries[Idx]);
		}
	}

	// EN: Append to history / ES: Agregar al historial
	AllHistoryEntries.Add(Record);

	// EN: Update filtered history if channel matches or showing all
	// ES: Actualizar historial filtrado si el canal coincide o se muestra todos
	if (SelectedChannel == EPGXFlowChannel::MAX || SelectedChannel == Channel)
	{
		FilteredHistoryEntries.Add(Record);
		if (HistoryListView.IsValid())
		{
			HistoryListView->RequestListRefresh();
			if (bAutoScroll)
			{
				HistoryListView->RequestScrollIntoView(Record);
			}
		}
	}

	// EN: Update channel list (for HistoryCount badge) / ES: Actualizar lista de canal (para badge de HistoryCount)
	if (ChannelListView.IsValid())
	{
		ChannelListView->RequestListRefresh();
	}

	// EN: Update detail if selected channel matches / ES: Actualizar detalle si el canal seleccionado coincide
	if (SelectedChannel == Channel)
	{
		RefreshChannelDetail();
	}

	RefreshKPIChips();
}

// ============================================================================
// EN: Actions
// ES: Acciones
// ============================================================================

FReply SPGXGameFlowInspectorTab::OnRefreshClicked()
{
	PGX_LOG_INFO(LogPGXGameFlowInspector, TEXT("[GameFlow] Manual refresh"));
	ChannelRowWidgets.Empty();
	RefreshChannelStates();
	RefreshHistoryForChannel();
	RefreshChannelDetail();
	return FReply::Handled();
}

FReply SPGXGameFlowInspectorTab::OnClearClicked()
{
	PGX_LOG_INFO(LogPGXGameFlowInspector, TEXT("[GameFlow] Clear — %d history entries removed"), AllHistoryEntries.Num());
	AllHistoryEntries.Empty();
	FilteredHistoryEntries.Empty();

	if (HistoryListView.IsValid())
	{
		HistoryListView->RequestListRefresh();
	}

	RefreshKPIChips();
	return FReply::Handled();
}

FReply SPGXGameFlowInspectorTab::OnRevertClicked()
{
	if (SelectedChannel != EPGXFlowChannel::MAX && BoundSubsystem.IsValid())
	{
		PGX_LOG_INFO(LogPGXGameFlowInspector, TEXT("[GameFlow] Revert to previous — channel=%d"), (int32)SelectedChannel);
		BoundSubsystem->RevertToPreviousFlow(SelectedChannel, nullptr);
		// EN: Subsystem fires delegate -> OnFlowStateChanged() -> auto-refresh
		// ES: El subsistema dispara delegado -> OnFlowStateChanged() -> auto-refresh
	}
	return FReply::Handled();
}

void SPGXGameFlowInspectorTab::RefreshKPIChips()
{
	// EN: Count active channels (those with a valid CurrentTag)
	// ES: Contar canales activos (aquellos con CurrentTag valido)
	int32 Active = 0;
	for (const auto& S : ChannelSummaries)
	{
		if (S->CurrentTag.IsValid()) Active++;
	}

	if (KPIActiveChannels.IsValid())
	{
		KPIActiveChannels->SetText(FText::FromString(
			FString::Printf(TEXT("Active: %d/%d"), Active, PGX_FLOW_CHANNEL_COUNT)));
		KPIActiveChannels->SetColorAndOpacity(
			(Active == PGX_FLOW_CHANNEL_COUNT)
				? FSlateColor(PGX::Semantic::Good)  // All active = green
				: (Active > 0)
					? FSlateColor(PGX::System::GameFlow)               // Some active = orange
					: FSlateColor(PGX::Text::Muted)); // None = gray
	}

	if (KPITransitionCount.IsValid())
	{
		KPITransitionCount->SetText(FText::FromString(
			FString::Printf(TEXT("Transitions: %d"), AllHistoryEntries.Num())));
	}
}

// ============================================================================
// EN: Helpers
// ES: Helpers
// ============================================================================

FLinearColor SPGXGameFlowInspectorTab::GetChannelColor(EPGXFlowChannel Channel)
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
	default:                          return PGX::Text::OnColor;
	}
}

#undef LOCTEXT_NAMESPACE
