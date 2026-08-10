// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "MGOSInspector/SPGXMGOSInspectorTab.h"
#include "PGXGCObserverSubsystem.h"
#include "Logging/PGXLogMacros.h"
#include "PGXMGOSDelegates.h"
#include "Engine/Engine.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/STableRow.h"
#include "Styling/AppStyle.h"
#include "Utils/PGXEditorUtils.h"
#include "Style/PGXVisualTokens.h"
#include "Widgets/SPGXPremiumShell.h"
#include "Widgets/PGXButtonHelpers.h"
#include "Widgets/SPGXSectionDivider.h"
#include "Widgets/SPGXKPIChip.h"
#include "Widgets/SPGXStatusBadge.h"
#include "Widgets/SPGXHealthDot.h"
#include "Style/PGXEditorStyle.h"
#include "Utils/SPGXTelemetryGraph.h"

#define LOCTEXT_NAMESPACE "PGXMGOSInspector"
DEFINE_LOG_CATEGORY_STATIC(LogPGXMGOSInspector, Log, All);

// EN: MGOS system color (Purple) / ES: Color del sistema MGOS (Morado)
static const FLinearColor GMGOSColor = PGX::System::MGOS;

// ============================================================================
// Construction / Destruction
// ============================================================================

void SPGXMGOSInspectorTab::Construct(const FArguments& /*InArgs*/)
{
	PGX_LOG_INFO(LogPGXMGOSInspector, TEXT("[MGOS] Construct"));

	ChildSlot
	[
		// EN: Premium Shell — covers UE chrome, provides title bar + footer
		// ES: Shell Premium — cubre chrome de UE, provee title bar + footer
		SNew(SPGXPremiumShell)
		.SystemColor(PGX::System::MGOS)
		.Title(LOCTEXT("MGOSTitle", "MGOS Inspector"))
		.Subtitle(LOCTEXT("MGOSSub", "GC Observability"))
		.Icon(FPGXEditorStyle::Get().GetBrush("PGXEditor.Icon.MGOS"))
		.bShowFooter(true)
		.TitleRightContent()
		[
			BuildToolbar()
		]
		.StatusText_Lambda([this]() -> FText
		{
			if (StatusBarWidget.IsValid())
			{
				return StatusBarWidget->GetText();
			}
			return LOCTEXT("StatusReady2", "MGOS Inspector — Engine subsystem (always available)");
		})
		.Content()
		[
			SNew(SScrollBox)

			// Panel 1: Status
			+ SScrollBox::Slot()
			.Padding(8)
			[
				BuildStatusPanel()
			]

			// Panel 1.5: GC Telemetry Graphs
			+ SScrollBox::Slot()
			.Padding(8)
			[
				BuildTelemetryPanel()
			]

			// Panel 2: History
			+ SScrollBox::Slot()
			.Padding(8)
			[
				BuildHistoryPanel()
			]

			// Panel 3: Class Report
			+ SScrollBox::Slot()
			.Padding(8)
			[
				BuildClassReportPanel()
			]

			// Panel 4: Incidents
			+ SScrollBox::Slot()
			.Padding(8)
			[
				BuildIncidentsPanel()
			]

			// Panel 5: Baseline & Config
			+ SScrollBox::Slot()
			.Padding(8)
			[
				BuildBaselinePanel()
			]
		]
	];

	// EN: Hidden text block for status (updated by RefreshAll, read by FooterBar lambda)
	// ES: Text block oculto para estado (actualizado por RefreshAll, leido por lambda de FooterBar)
	SAssignNew(StatusBarWidget, STextBlock)
		.Text(LOCTEXT("StatusReady", "MGOS Inspector — Engine subsystem (always available)"));

	BindSubsystemDelegates();

	// EN: Seed telemetry graphs with historical data / ES: Sembrar grafos de telemetria con datos historicos
	if (UPGXGCObserverSubsystem* Sub = GetSubsystem())
	{
		TArray<FPGXGCSnapshotDiff> History = Sub->GetHistorySummary(128);
		for (const FPGXGCSnapshotDiff& D : History)
		{
			if (GCDurationGraph.IsValid())
			{
				GCDurationGraph->PushValue(static_cast<float>(D.DurationSeconds * 1000.0));
			}
			if (MemoryDeltaGraph.IsValid())
			{
				MemoryDeltaGraph->PushValue(static_cast<float>(D.ProcessMemoryDeltaMB));
			}
		}
	}

	RefreshAll();
	RefreshKPIChips();
}

SPGXMGOSInspectorTab::~SPGXMGOSInspectorTab()
{
	PGX_LOG_INFO(LogPGXMGOSInspector, TEXT("[MGOS] Destructor — cleaning up"));
	UnbindSubsystemDelegates();
}

// ============================================================================
// Toolbar
// ============================================================================

TSharedRef<SWidget> SPGXMGOSInspectorTab::BuildToolbar()
{
	return SNew(SHorizontalBox)

		// EN: KPI chip: Cycles / ES: Chip KPI: Ciclos
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4.0f, 0.0f)
		[
			SNew(SPGXKPIChip)
			.Label(LOCTEXT("KPICyclesLbl", "CYCLES"))
			.Value(LOCTEXT("KPICyclesInit", "0"))
			.AccentColor(GMGOSColor)
			.ValueWidget()
			[
				SAssignNew(KPICyclesChip, STextBlock)
				.Text(LOCTEXT("KPICyclesInit2", "0"))
				.Font(PGX::Font::KPIValue())
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
			]
		]

		// EN: KPI chip: Incidents / ES: Chip KPI: Incidentes
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4.0f, 0.0f)
		[
			SNew(SPGXKPIChip)
			.Label(LOCTEXT("KPIIncidentsLbl", "INCIDENTS"))
			.Value(LOCTEXT("KPIIncidentsInit", "0"))
			.AccentColor(GMGOSColor)
			.ValueWidget()
			[
				SAssignNew(KPIIncidentsChip, STextBlock)
				.Text(LOCTEXT("KPIIncidentsInit2", "0"))
				.Font(PGX::Font::KPIValue())
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
			]
		]

		// EN: KPI chip: Memory / ES: Chip KPI: Memoria
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4.0f, 0.0f)
		[
			SNew(SPGXKPIChip)
			.Label(LOCTEXT("KPIMemoryLbl", "MEMORY"))
			.Value(LOCTEXT("KPIMemoryInit", "---"))
			.AccentColor(GMGOSColor)
			.ValueWidget()
			[
				SAssignNew(KPIMemoryChip, STextBlock)
				.Text(LOCTEXT("KPIMemoryInit2", "---"))
				.Font(PGX::Font::KPIValue())
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
			]
		]

		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNullWidget::NullWidget
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(4, 0)
		[
			SNew(SButton)
			.Text(LOCTEXT("Refresh", "Refresh"))
			.ToolTipText(LOCTEXT("RefreshTooltip", "Refresh all panels from subsystem data"))
			.OnClicked(this, &SPGXMGOSInspectorTab::OnRefreshClicked)
		];
}

// ============================================================================
// Panel 1: Status
// ============================================================================

TSharedRef<SWidget> SPGXMGOSInspectorTab::BuildStatusPanel()
{
	return SNew(SVerticalBox)

		// EN: Section header / ES: Cabecera de seccion
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 4)
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("StatusHeader", "GC OBSERVER STATUS"))
			.AccentColor(GMGOSColor)
		]

		// Mode
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
			[
				SNew(STextBlock).Text(LOCTEXT("Mode", "Mode:"))
				.Font(PGX::Font::SubHeader())
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f)
			[ SAssignNew(ModeText, STextBlock).Text(LOCTEXT("ModeDefault", "—")) ]
		]

		// EN: Mode switch buttons / ES: Botones de cambio de modo
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 4)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
			[
				SNew(STextBlock).Text(LOCTEXT("SwitchMode", "Switch:"))
				.Font(PGX::Font::SubHeader())
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)
			[
				SNew(SButton).Text(LOCTEXT("ModePassive", "Passive"))
				.ToolTipText(LOCTEXT("PassiveTip", "Lightweight — global counts only"))
				.OnClicked_Lambda([this]()
				{
					if (auto* S = GetSubsystem()) { S->SetMode(EPGXGCObserverMode::Passive); RefreshAll(); RefreshKPIChips(); }
					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)
			[
				SNew(SButton).Text(LOCTEXT("ModeSnapshot", "Snapshot"))
				.ToolTipText(LOCTEXT("SnapshotTip", "Per-class tracking via TrackedClasses list"))
				.OnClicked_Lambda([this]()
				{
					if (auto* S = GetSubsystem()) { S->SetMode(EPGXGCObserverMode::Snapshot); RefreshAll(); RefreshKPIChips(); }
					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SButton).Text(LOCTEXT("ModeDeepTrack", "DeepTrack"))
				.ToolTipText(LOCTEXT("DeepTrackTip", "Full analysis — global iteration + per-class health"))
				.OnClicked_Lambda([this]()
				{
					if (auto* S = GetSubsystem()) { S->SetMode(EPGXGCObserverMode::DeepTrack); RefreshAll(); RefreshKPIChips(); }
					return FReply::Handled();
				})
			]
		]

		// Baseline
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
			[
				SNew(STextBlock).Text(LOCTEXT("Baseline", "Baseline:"))
				.Font(PGX::Font::SubHeader())
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f)
			[ SAssignNew(BaselineStateText, STextBlock).Text(LOCTEXT("BaselineDefault", "—")) ]
		]

		// Profile
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
			[
				SNew(STextBlock).Text(LOCTEXT("Profile", "Profile:"))
				.Font(PGX::Font::SubHeader())
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f)
			[ SAssignNew(ProfileStateText, STextBlock).Text(LOCTEXT("ProfileDefault", "—")) ]
		]

		// Cycles
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
			[
				SNew(STextBlock).Text(LOCTEXT("Cycles", "Cycles:"))
				.Font(PGX::Font::SubHeader())
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f)
			[ SAssignNew(CycleCountText, STextBlock).Text(LOCTEXT("CyclesDefault", "0")) ]
		]

		// Incidents
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
			[
				SNew(STextBlock).Text(LOCTEXT("IncidentsLabel", "Incidents:"))
				.Font(PGX::Font::SubHeader())
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f)
			[ SAssignNew(IncidentCountText, STextBlock).Text(LOCTEXT("IncidentsDefault", "0")) ]
		]

		// Process Memory
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
			[
				SNew(STextBlock).Text(LOCTEXT("Memory", "Memory:"))
				.Font(PGX::Font::SubHeader())
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f)
			[ SAssignNew(ProcessMemoryText, STextBlock).Text(LOCTEXT("MemDefault", "— MB")) ]
		]

		// Suppressed
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
			[
				SNew(STextBlock).Text(LOCTEXT("Suppressed", "Suppressed:"))
				.Font(PGX::Font::SubHeader())
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f)
			[ SAssignNew(SuppressedText, STextBlock).Text(LOCTEXT("SuppDefault", "No")) ]
		];
}

// ============================================================================
// Panel 1.5: GC Telemetry Graphs
// ============================================================================

TSharedRef<SWidget> SPGXMGOSInspectorTab::BuildTelemetryPanel()
{
	return SNew(SVerticalBox)

		// EN: Section header / ES: Cabecera de seccion
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 4)
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("TelemetryHeader", "GC TELEMETRY"))
			.AccentColor(GMGOSColor)
		]

		// EN: GC Cycle Duration graph / ES: Grafico de duracion de ciclo GC
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("GCDurationLabel", "GC Cycle Duration (ms)"))
			.Font(PGX::Font::SubHeader())
			.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2)
		[
			SNew(SBox)
			.HeightOverride(PGX::Height::GraphDefault)
			[
				SAssignNew(GCDurationGraph, SPGXTelemetryGraph)
				.BufferSize(128)
				.LineColor(PGX::System::MGOS)
				.MinValue(0.0f)
				.MaxValue(50.0f)
				.bShowGrid(true)
				.bShowAvgLine(true)
				.bShowPeakMarkers(true)
				.bShowLabels(true)
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, PGX::Spacing::MD)
		[
			PGXEditorUtils::BuildGraphLegend(GCDurationGraph, PGX::System::MGOS)
		]

		// EN: Process Memory Delta graph / ES: Grafico de delta de memoria de proceso
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("MemDeltaLabel", "Process Memory Delta (MB)"))
			.Font(PGX::Font::SubHeader())
			.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2)
		[
			SNew(SBox)
			.HeightOverride(PGX::Height::GraphDefault)
			[
				SAssignNew(MemoryDeltaGraph, SPGXTelemetryGraph)
				.BufferSize(128)
				.LineColor(PGX::Semantic::Info)
				.MinValue(-10.0f)
				.MaxValue(50.0f)
				.bShowGrid(true)
				.bShowAvgLine(true)
				.bShowLabels(true)
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, PGX::Spacing::MD)
		[
			PGXEditorUtils::BuildGraphLegend(MemoryDeltaGraph, PGX::Semantic::Info)
		];
}

// ============================================================================
// Panel 2: History (SListView)
// ============================================================================

TSharedRef<SWidget> SPGXMGOSInspectorTab::BuildHistoryPanel()
{
	return SNew(SVerticalBox)

		// EN: Section header / ES: Cabecera de seccion
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 4)
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("HistoryHeader", "GC CYCLE HISTORY"))
			.AccentColor(GMGOSColor)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.MaxHeight(200.0f)
		[
			SAssignNew(HistoryListView, SListView<TSharedPtr<FPGXGCSnapshotDiff>>)
			.ListItemsSource(&HistoryItems)
			.OnGenerateRow(this, &SPGXMGOSInspectorTab::OnGenerateHistoryRow)
			.SelectionMode(ESelectionMode::None)
			.HeaderRow(
				SNew(SHeaderRow)
				+ SHeaderRow::Column("Cycle").DefaultLabel(LOCTEXT("HCycle", "Cycle")).FillWidth(0.12f)
				+ SHeaderRow::Column("Duration").DefaultLabel(LOCTEXT("HDur", "Duration")).FillWidth(0.18f)
				+ SHeaderRow::Column("DeltaUObj").DefaultLabel(LOCTEXT("HDeltaU", "Delta UObj")).FillWidth(0.2f)
				+ SHeaderRow::Column("DeltaPK").DefaultLabel(LOCTEXT("HDeltaPK", "Delta PK")).FillWidth(0.18f)
				+ SHeaderRow::Column("MemDelta").DefaultLabel(LOCTEXT("HMem", "Mem Delta")).FillWidth(0.32f)
			)
		];
}

// ============================================================================
// Panel 3: Class Report (SListView)
// ============================================================================

TSharedRef<SWidget> SPGXMGOSInspectorTab::BuildClassReportPanel()
{
	return SNew(SVerticalBox)

		// EN: Section header / ES: Cabecera de seccion
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 4)
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("ClassReportHeader", "TRACKED CLASS HEALTH"))
			.AccentColor(GMGOSColor)
		]

		// EN: Empty state text (shown when no data) / ES: Texto de estado vacio
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(ClassReportEmptyText, STextBlock)
			.Text(LOCTEXT("ClassReportEmpty", "Per-class tracking requires Snapshot or DeepTrack mode"))
			.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.MaxHeight(200.0f)
		[
			SAssignNew(ClassReportListView, SListView<TSharedPtr<FPGXGCClassHealthReport>>)
			.ListItemsSource(&ClassReportItems)
			.OnGenerateRow(this, &SPGXMGOSInspectorTab::OnGenerateClassReportRow)
			.SelectionMode(ESelectionMode::None)
			.HeaderRow(
				SNew(SHeaderRow)
				+ SHeaderRow::Column("Class").DefaultLabel(LOCTEXT("CRClass", "Class")).FillWidth(0.3f)
				+ SHeaderRow::Column("Current").DefaultLabel(LOCTEXT("CRCurrent", "Current")).FillWidth(0.14f)
				+ SHeaderRow::Column("Baseline").DefaultLabel(LOCTEXT("CRBase", "Baseline")).FillWidth(0.14f)
				+ SHeaderRow::Column("Slope").DefaultLabel(LOCTEXT("CRSlope", "Growth/Cycle")).FillWidth(0.18f)
				+ SHeaderRow::Column("Health").DefaultLabel(LOCTEXT("CRHealth", "Health")).FillWidth(0.24f)
			)
		];
}

// ============================================================================
// Panel 4: Incidents (SListView)
// ============================================================================

TSharedRef<SWidget> SPGXMGOSInspectorTab::BuildIncidentsPanel()
{
	return SNew(SVerticalBox)

		// EN: Section header / ES: Cabecera de seccion
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 4)
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("IncidentsHeader", "ACTIVE INCIDENTS"))
			.AccentColor(GMGOSColor)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.MaxHeight(150.0f)
		[
			SAssignNew(IncidentListView, SListView<TSharedPtr<FPGXGCIncident>>)
			.ListItemsSource(&IncidentItems)
			.OnGenerateRow(this, &SPGXMGOSInspectorTab::OnGenerateIncidentRow)
			.SelectionMode(ESelectionMode::None)
			.HeaderRow(
				SNew(SHeaderRow)
				+ SHeaderRow::Column("Severity").DefaultLabel(LOCTEXT("ISev", "Severity")).FillWidth(0.15f)
				+ SHeaderRow::Column("Cycle").DefaultLabel(LOCTEXT("ICycle", "Cycle")).FillWidth(0.12f)
				+ SHeaderRow::Column("Category").DefaultLabel(LOCTEXT("ICat", "Category")).FillWidth(0.2f)
				+ SHeaderRow::Column("Description").DefaultLabel(LOCTEXT("IDesc", "Description")).FillWidth(0.53f)
			)
		];
}

// ============================================================================
// Panel 5: Baseline & Config
// ============================================================================

TSharedRef<SWidget> SPGXMGOSInspectorTab::BuildBaselinePanel()
{
	return SNew(SVerticalBox)

		// EN: Section header / ES: Cabecera de seccion
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 4)
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("BaselineHeader", "BASELINE & CONFIG"))
			.AccentColor(GMGOSColor)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2)
		[
			SAssignNew(BaselineInfoText, STextBlock)
			.Text(LOCTEXT("BaselineInfoDefault", "No baseline captured"))
			.AutoWrapText(true)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 8)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 4, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("CaptureBaseline", "Capture Baseline"))
				.ToolTipText(LOCTEXT("CaptureTip", "Request baseline capture from current state"))
				.OnClicked(this, &SPGXMGOSInspectorTab::OnCaptureBaselineClicked)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 4, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("ResetBaseline", "Reset Baseline"))
				.ToolTipText(LOCTEXT("ResetTip", "Reset baseline to Uninitialized"))
				.OnClicked(this, &SPGXMGOSInspectorTab::OnResetBaselineClicked)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("ToggleSuppress", "Toggle Suppression"))
				.ToolTipText(LOCTEXT("ToggleTip", "Toggle GC incident suppression"))
				.OnClicked(this, &SPGXMGOSInspectorTab::OnToggleSuppressClicked)
			]
		];
}

// ============================================================================
// Row Generators
// ============================================================================

TSharedRef<ITableRow> SPGXMGOSInspectorTab::OnGenerateHistoryRow(
	TSharedPtr<FPGXGCSnapshotDiff> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	// EN: Color by UObject delta magnitude / ES: Color por magnitud del delta UObject
	const FLinearColor DeltaColor = (Item->DeltaTotalUObject > 100)
		? PGX::Semantic::Error   // Red — growth
		: (Item->DeltaTotalUObject < -100)
			? PGX::Semantic::Good // Cyan — burst clean
			: PGX::Text::Secondary; // Gray — normal

	return SNew(STableRow<TSharedPtr<FPGXGCSnapshotDiff>>, OwnerTable)
		.Padding(FMargin(4, 2))
		[
			SNew(SHorizontalBox)

			// Cycle
			+ SHorizontalBox::Slot()
			.FillWidth(0.12f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("%lld"), Item->CycleID)))
				.Font(PGX::Font::BodySmall())
			]

			// Duration
			+ SHorizontalBox::Slot()
			.FillWidth(0.18f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("%.2f ms"), Item->DurationSeconds * 1000.0)))
				.Font(PGX::Font::BodySmall())
			]

			// Delta UObj
			+ SHorizontalBox::Slot()
			.FillWidth(0.2f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("%lld"), Item->DeltaTotalUObject)))
				.Font(PGX::Font::BodySmall())
				.ColorAndOpacity(FSlateColor(DeltaColor))
			]

			// Delta PK
			+ SHorizontalBox::Slot()
			.FillWidth(0.18f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("%lld"), Item->DeltaPendingKill)))
				.Font(PGX::Font::BodySmall())
			]

			// Mem Delta
			+ SHorizontalBox::Slot()
			.FillWidth(0.32f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("%.1f MB"), Item->ProcessMemoryDeltaMB)))
				.Font(PGX::Font::BodySmall())
				.ColorAndOpacity(FSlateColor(Item->ProcessMemoryDeltaMB > 10.0f
					? PGX::Semantic::Warn
					: PGX::Text::Secondary))
			]
		];
}

TSharedRef<ITableRow> SPGXMGOSInspectorTab::OnGenerateClassReportRow(
	TSharedPtr<FPGXGCClassHealthReport> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	const FLinearColor HealthColor = GetClassHealthColor(Item->HealthState);

	return SNew(STableRow<TSharedPtr<FPGXGCClassHealthReport>>, OwnerTable)
		.Padding(FMargin(4, 2))
		[
			SNew(SHorizontalBox)

			// Class name
			+ SHorizontalBox::Slot()
			.FillWidth(0.3f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromName(Item->ClassName))
				.Font(PGX::Font::BodySmall())
				.ToolTipText(FText::FromString(FString::Printf(TEXT("NoReturn: %s | Peak Dev: %.1f | Ratio: %.2f"),
					Item->bNoReturn ? TEXT("Yes") : TEXT("No"),
					Item->PeakDeviation, Item->OverBaselineRatio)))
			]

			// Current count
			+ SHorizontalBox::Slot()
			.FillWidth(0.14f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("%lld"), Item->CurrentCount)))
				.Font(PGX::Font::BodySmall())
			]

			// Baseline count
			+ SHorizontalBox::Slot()
			.FillWidth(0.14f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("%lld"), Item->BaselineCount)))
				.Font(PGX::Font::BodySmall())
			]

			// Growth slope
			+ SHorizontalBox::Slot()
			.FillWidth(0.18f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("%.2f/cycle"), Item->GrowthSlope)))
				.Font(PGX::Font::BodySmall())
				.ColorAndOpacity(FSlateColor(Item->GrowthSlope > 1.0f
					? PGX::Semantic::Warn
					: PGX::Text::Secondary))
			]

			// EN: Health state dot + badge / ES: Dot de estado de salud + badge
			+ SHorizontalBox::Slot()
			.FillWidth(0.24f)
			.Padding(2, 0)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0, 0, PGX::Spacing::SM, 0)
				[
					SNew(SPGXHealthDot)
					.State(Item->HealthState == EPGXGCClassHealth::Stable ? EPGXHealthState::Active
						: Item->HealthState == EPGXGCClassHealth::LeakSuspected ? EPGXHealthState::Error
						: EPGXHealthState::Warning)
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(SPGXStatusBadge)
					.Shape(EPGXBadgeShape::Pill)
					.Color(HealthColor)
					.Label(FText::FromString(UEnum::GetValueAsString(Item->HealthState)))
				]
			]
		];
}

TSharedRef<ITableRow> SPGXMGOSInspectorTab::OnGenerateIncidentRow(
	TSharedPtr<FPGXGCIncident> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	const FLinearColor SevColor = GetSeverityColor(Item->Severity);

	return SNew(STableRow<TSharedPtr<FPGXGCIncident>>, OwnerTable)
		.Padding(FMargin(4, 2))
		[
			SNew(SHorizontalBox)

			// EN: Severity badge / ES: Badge de severidad
			+ SHorizontalBox::Slot()
			.FillWidth(0.15f)
			.Padding(2, 0)
			[
				SNew(SPGXStatusBadge)
				.Shape(EPGXBadgeShape::Pill)
				.Color(SevColor)
				.Label(FText::FromString(UEnum::GetValueAsString(Item->Severity)))
			]

			// Cycle
			+ SHorizontalBox::Slot()
			.FillWidth(0.12f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("%lld"), Item->CycleID)))
				.Font(PGX::Font::BodySmall())
			]

			// Category
			+ SHorizontalBox::Slot()
			.FillWidth(0.2f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromName(Item->Category))
				.Font(PGX::Font::BodySmall())
			]

			// Description
			+ SHorizontalBox::Slot()
			.FillWidth(0.53f)
			.Padding(2, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->Description))
				.Font(PGX::Font::BodySmall())
				.AutoWrapText(true)
			]
		];
}

// ============================================================================
// Data Refresh
// ============================================================================

void SPGXMGOSInspectorTab::RefreshAll()
{
	UPGXGCObserverSubsystem* Sub = GetSubsystem();
	if (!Sub)
	{
		if (ModeText.IsValid())
		{
			ModeText->SetText(LOCTEXT("NotAvailable", "Subsystem not available"));
		}
		return;
	}

	// ── Status panel ────────────────────────────────────────────────────
	if (ModeText.IsValid())
	{
		ModeText->SetText(FText::FromString(UEnum::GetValueAsString(Sub->GetMode())));
	}
	if (BaselineStateText.IsValid())
	{
		BaselineStateText->SetText(FText::FromString(UEnum::GetValueAsString(Sub->GetBaselineState())));
	}

	const FPGXGCProfile Profile = Sub->GetCurrentProfile();
	if (ProfileStateText.IsValid())
	{
		ProfileStateText->SetText(FText::FromString(
			FString::Printf(TEXT("%s (%.0f%%, %d cycles)"),
				*UEnum::GetValueAsString(Profile.CurrentState),
				Profile.Confidence * 100.0f,
				Profile.CyclesInState)));
		ProfileStateText->SetColorAndOpacity(FSlateColor(GetProfileStateColor(Profile.CurrentState)));
	}

	if (CycleCountText.IsValid())
	{
		CycleCountText->SetText(FText::FromString(FString::Printf(TEXT("%lld"), Sub->GetCycleCount())));
	}

	const TArray<FPGXGCIncident>& Incidents = Sub->GetCurrentIncidents();
	if (IncidentCountText.IsValid())
	{
		IncidentCountText->SetText(FText::FromString(FString::Printf(TEXT("%d"), Incidents.Num())));
		IncidentCountText->SetColorAndOpacity(FSlateColor(
			Incidents.Num() > 0 ? PGX::Semantic::Error : PGX::Semantic::Good));
	}
	if (SuppressedText.IsValid())
	{
		SuppressedText->SetText(Sub->IsInSuppressedPhase() ? LOCTEXT("Yes", "Yes") : LOCTEXT("No", "No"));
	}

	// EN: Process memory from baseline / ES: Memoria del proceso desde baseline
	const FPGXGCBaseline Baseline = Sub->GetCurrentBaseline();
	if (ProcessMemoryText.IsValid())
	{
		if (Baseline.bValid)
		{
			ProcessMemoryText->SetText(FText::FromString(
				FString::Printf(TEXT("%.1f MB (baseline)"), Baseline.BaselineProcessMemoryMB)));
		}
		else
		{
			ProcessMemoryText->SetText(LOCTEXT("MemNA", "— (no baseline)"));
		}
	}

	// ── History panel (SListView) ───────────────────────────────────────
	TArray<FPGXGCSnapshotDiff> History = Sub->GetHistorySummary(10);
	HistoryItems.Empty();
	for (FPGXGCSnapshotDiff& D : History)
	{
		HistoryItems.Add(MakeShared<FPGXGCSnapshotDiff>(MoveTemp(D)));
	}
	if (HistoryListView.IsValid())
	{
		HistoryListView->RequestListRefresh();
	}

	// ── Class Report panel (SListView) ──────────────────────────────────
	TArray<FPGXGCClassHealthReport> Reports = Sub->GetTrackedClassReport();
	ClassReportItems.Empty();
	for (FPGXGCClassHealthReport& R : Reports)
	{
		ClassReportItems.Add(MakeShared<FPGXGCClassHealthReport>(MoveTemp(R)));
	}
	if (ClassReportListView.IsValid())
	{
		ClassReportListView->RequestListRefresh();
	}
	// EN: Show/hide empty state / ES: Mostrar/ocultar estado vacio
	if (ClassReportEmptyText.IsValid())
	{
		const EPGXGCObserverMode Mode = Sub->GetMode();
		if (ClassReportItems.Num() > 0)
		{
			ClassReportEmptyText->SetVisibility(EVisibility::Collapsed);
		}
		else if (Mode == EPGXGCObserverMode::Passive || Mode == EPGXGCObserverMode::Off)
		{
			ClassReportEmptyText->SetText(LOCTEXT("ClassReportNeedMode", "Per-class tracking requires Snapshot or DeepTrack mode"));
			ClassReportEmptyText->SetVisibility(EVisibility::Visible);
		}
		else
		{
			ClassReportEmptyText->SetText(LOCTEXT("ClassReportNoData", "No tracked classes — check PGXGCObserverConfig"));
			ClassReportEmptyText->SetVisibility(EVisibility::Visible);
		}
	}

	// ── Incidents panel (SListView) ─────────────────────────────────────
	IncidentItems.Empty();
	for (const FPGXGCIncident& Inc : Incidents)
	{
		IncidentItems.Add(MakeShared<FPGXGCIncident>(Inc));
	}
	if (IncidentListView.IsValid())
	{
		IncidentListView->RequestListRefresh();
	}

	// ── Baseline panel ──────────────────────────────────────────────────
	if (BaselineInfoText.IsValid())
	{
		if (Baseline.bValid)
		{
			FString BaseStr = FString::Printf(
				TEXT("State: %s\nCaptured at cycle: %lld\nUObjects: %lld\nPendingKill: %lld\nProcess Memory: %.1f MB\nTracked Classes: %d"),
				*UEnum::GetValueAsString(Baseline.BaselineState),
				Baseline.BaselineCycleID,
				Baseline.TotalUObjectCount,
				Baseline.PendingKillCount,
				Baseline.BaselineProcessMemoryMB,
				Baseline.ClassCounts.Num());
			BaselineInfoText->SetText(FText::FromString(BaseStr));
		}
		else
		{
			BaselineInfoText->SetText(FText::FromString(
				FString::Printf(TEXT("State: %s\nNo baseline captured — will auto-capture after warmup cycles"),
					*UEnum::GetValueAsString(Sub->GetBaselineState()))));
		}
	}

	// EN: Update status bar / ES: Actualizar barra de estado
	if (StatusBarWidget.IsValid())
	{
		StatusBarWidget->SetText(FText::Format(
			LOCTEXT("StatusFmt", "MGOS Inspector — Mode: {0} | Cycles: {1} | Incidents: {2}"),
			FText::FromString(UEnum::GetValueAsString(Sub->GetMode())),
			FText::FromString(FString::Printf(TEXT("%lld"), Sub->GetCycleCount())),
			FText::AsNumber(Incidents.Num())));
	}
}

// ============================================================================
// KPI
// ============================================================================

void SPGXMGOSInspectorTab::RefreshKPIChips()
{
	UPGXGCObserverSubsystem* Sub = GetSubsystem();

	if (KPICyclesChip.IsValid())
	{
		if (Sub)
		{
			KPICyclesChip->SetText(FText::FromString(FString::Printf(TEXT("%lld"), Sub->GetCycleCount())));
		}
		else
		{
			KPICyclesChip->SetText(LOCTEXT("KPICyclesOff", "---"));
		}
	}

	if (KPIIncidentsChip.IsValid())
	{
		if (Sub)
		{
			const int32 IncCount = Sub->GetCurrentIncidents().Num();
			KPIIncidentsChip->SetText(FText::AsNumber(IncCount));
			KPIIncidentsChip->SetColorAndOpacity(FSlateColor(
				IncCount > 0 ? PGX::Semantic::Error : PGX::Semantic::Good));
		}
		else
		{
			KPIIncidentsChip->SetText(LOCTEXT("KPIIncidentsOff", "---"));
		}
	}

	if (KPIMemoryChip.IsValid())
	{
		if (Sub)
		{
			const FPGXGCBaseline BL = Sub->GetCurrentBaseline();
			if (BL.bValid)
			{
				KPIMemoryChip->SetText(FText::FromString(
					FString::Printf(TEXT("%.0f MB"), BL.BaselineProcessMemoryMB)));
			}
			else
			{
				KPIMemoryChip->SetText(LOCTEXT("KPIMemNA", "---"));
			}
		}
		else
		{
			KPIMemoryChip->SetText(LOCTEXT("KPIMemOff", "---"));
		}
	}
}

// ============================================================================
// Button Handlers
// ============================================================================

FReply SPGXMGOSInspectorTab::OnRefreshClicked()
{
	PGX_LOG_INFO(LogPGXMGOSInspector, TEXT("[MGOS] Manual refresh"));
	RefreshAll();
	RefreshKPIChips();
	return FReply::Handled();
}

FReply SPGXMGOSInspectorTab::OnCaptureBaselineClicked()
{
	PGX_LOG_INFO(LogPGXMGOSInspector, TEXT("[MGOS] Capture Baseline requested"));
	if (UPGXGCObserverSubsystem* Sub = GetSubsystem())
	{
		Sub->RequestBaselineCapture();
		RefreshAll();
		RefreshKPIChips();
	}
	return FReply::Handled();
}

FReply SPGXMGOSInspectorTab::OnResetBaselineClicked()
{
	PGX_LOG_INFO(LogPGXMGOSInspector, TEXT("[MGOS] Reset Baseline requested"));
	if (UPGXGCObserverSubsystem* Sub = GetSubsystem())
	{
		Sub->ResetBaseline();
		RefreshAll();
		RefreshKPIChips();
	}
	return FReply::Handled();
}

FReply SPGXMGOSInspectorTab::OnToggleSuppressClicked()
{
	if (UPGXGCObserverSubsystem* Sub = GetSubsystem())
	{
		const bool bNew = !Sub->IsInSuppressedPhase();
		PGX_LOG_INFO(LogPGXMGOSInspector, TEXT("[MGOS] Toggle Suppression: %s"), bNew ? TEXT("On") : TEXT("Off"));
		Sub->SetSuppressed(bNew);
		RefreshAll();
		RefreshKPIChips();
	}
	return FReply::Handled();
}

// ============================================================================
// Delegate Lifecycle
// ============================================================================

void SPGXMGOSInspectorTab::BindSubsystemDelegates()
{
	UPGXGCObserverSubsystem* Sub = GetSubsystem();
	if (!Sub) return;

	GCCompletedHandle = Sub->OnGCCompletedNative.AddSP(
		SharedThis(this), &SPGXMGOSInspectorTab::OnGCCycleCompleted);
	IncidentRaisedHandle = Sub->OnIncidentRaisedNative.AddSP(
		SharedThis(this), &SPGXMGOSInspectorTab::OnIncidentRaised);
	ProfileChangedHandle = Sub->OnProfileStateChangedNative.AddSP(
		SharedThis(this), &SPGXMGOSInspectorTab::OnProfileChanged);

	PGX_LOG_INFO(LogPGXMGOSInspector, TEXT("[MGOS] Bound 3 subsystem delegates"));
}

void SPGXMGOSInspectorTab::UnbindSubsystemDelegates()
{
	UPGXGCObserverSubsystem* Sub = GetSubsystem();
	if (!Sub) return;

	if (GCCompletedHandle.IsValid())
	{
		Sub->OnGCCompletedNative.Remove(GCCompletedHandle);
		GCCompletedHandle.Reset();
	}
	if (IncidentRaisedHandle.IsValid())
	{
		Sub->OnIncidentRaisedNative.Remove(IncidentRaisedHandle);
		IncidentRaisedHandle.Reset();
	}
	if (ProfileChangedHandle.IsValid())
	{
		Sub->OnProfileStateChangedNative.Remove(ProfileChangedHandle);
		ProfileChangedHandle.Reset();
	}

	PGX_LOG_INFO(LogPGXMGOSInspector, TEXT("[MGOS] Unbound subsystem delegates"));
}

// ============================================================================
// Delegate Callbacks
// ============================================================================

void SPGXMGOSInspectorTab::OnGCCycleCompleted(uint64 CycleID, const FPGXGCSnapshot& /*Snapshot*/, const FPGXGCSnapshotDiff& Diff)
{
	PGX_LOG_INFO(LogPGXMGOSInspector, TEXT("[MGOS] GC cycle completed: %llu"), CycleID);

	// EN: Push telemetry data to graphs / ES: Empujar datos de telemetria a los grafos
	if (GCDurationGraph.IsValid())
	{
		GCDurationGraph->PushValue(static_cast<float>(Diff.DurationSeconds * 1000.0));
	}
	if (MemoryDeltaGraph.IsValid())
	{
		MemoryDeltaGraph->PushValue(static_cast<float>(Diff.ProcessMemoryDeltaMB));
	}

	RefreshAll();
	RefreshKPIChips();
}

void SPGXMGOSInspectorTab::OnIncidentRaised(const FPGXGCIncident& Incident)
{
	PGX_LOG_INFO(LogPGXMGOSInspector, TEXT("[MGOS] Incident raised: %s (%s)"),
		*Incident.Category.ToString(), *UEnum::GetValueAsString(Incident.Severity));
	RefreshAll();
	RefreshKPIChips();
}

void SPGXMGOSInspectorTab::OnProfileChanged(EPGXGCProfileState NewState, EPGXGCProfileState OldState)
{
	PGX_LOG_INFO(LogPGXMGOSInspector, TEXT("[MGOS] Profile state changed: %s -> %s"),
		*UEnum::GetValueAsString(OldState), *UEnum::GetValueAsString(NewState));
	RefreshAll();
	RefreshKPIChips();
}

// ============================================================================
// Helpers
// ============================================================================

UPGXGCObserverSubsystem* SPGXMGOSInspectorTab::GetSubsystem() const
{
	// EN: Engine subsystem — always available, no PIE dependency
	// ES: Subsistema de Engine — siempre disponible, sin dependencia de PIE
	return UPGXGCObserverSubsystem::GetCachedInstance();
}

FLinearColor SPGXMGOSInspectorTab::GetSeverityColor(EPGXGCSeverity Sev)
{
	switch (Sev)
	{
	case EPGXGCSeverity::Critical: return PGX::Semantic::Error;
	case EPGXGCSeverity::Warning:  return PGX::Semantic::Warn;
	default:                       return PGX::Semantic::Neutral;
	}
}

FLinearColor SPGXMGOSInspectorTab::GetClassHealthColor(EPGXGCClassHealth Health)
{
	switch (Health)
	{
	case EPGXGCClassHealth::Stable:                return PGX::Semantic::Good;
	case EPGXGCClassHealth::Accumulating:          return PGX::Semantic::Warn;
	case EPGXGCClassHealth::PersistentOverBaseline: return PGX::Semantic::Warn;
	case EPGXGCClassHealth::LeakSuspected:         return PGX::Semantic::Error;
	default:                                       return PGX::Semantic::Neutral;
	}
}

FLinearColor SPGXMGOSInspectorTab::GetProfileStateColor(EPGXGCProfileState State)
{
	switch (State)
	{
	case EPGXGCProfileState::Stable:                return PGX::Semantic::Good;
	case EPGXGCProfileState::Accumulation:          return PGX::Semantic::Warn;
	case EPGXGCProfileState::LeakSuspected:         return PGX::Semantic::Error;
	case EPGXGCProfileState::BurstClean:            return PGX::Semantic::Good;
	case EPGXGCProfileState::PendingKillSaturation: return PGX::Semantic::Warn;
	case EPGXGCProfileState::RootExpansion:         return FLinearColor(0.8f, 0.3f, 0.9f); // KEEP: unique GC state (purple)
	default:                                        return PGX::Semantic::Neutral;
	}
}

#undef LOCTEXT_NAMESPACE
