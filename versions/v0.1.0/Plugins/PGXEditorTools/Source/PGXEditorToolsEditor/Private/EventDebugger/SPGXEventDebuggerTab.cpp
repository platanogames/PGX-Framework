// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "EventDebugger/SPGXEventDebuggerTab.h"
#include "EventHandler/PGXEventHandlerSubsystem.h"
#include "EventHandler/PGXEventHandlerTypes.h"
#include "EventHandler/PGXEventHandlerDelegates.h"
#include "Logging/PGXLogMacros.h"
#include "Widgets/SPGXEmptyStateV2.h"
#include "Utils/PGXEditorUtils.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/STableRow.h"
#include "Styling/CoreStyle.h"
#include "Styling/AppStyle.h"
#include "Style/PGXVisualTokens.h"
#include "Style/PGXEditorStyle.h"
#include "Widgets/SPGXPremiumShell.h"
#include "Widgets/SPGXSectionDivider.h"
#include "Widgets/SPGXKPIChip.h"
#include "Widgets/PGXButtonHelpers.h"
#include "Widgets/SPGXSparkline.h"
#include "Utils/SPGXTelemetryGraph.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "Notifications/PGXEditorNotification.h"
#include "EventHandler/PGXEventHandlerLog.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Editor.h"

#define LOCTEXT_NAMESPACE "PGXEventDebugger"

// EN: System color: EventHandler — via PGX Visual Tokens / ES: Color del sistema: EventHandler — via PGX Visual Tokens
static const FLinearColor GEventDebuggerColor = PGX::System::EventHandler;

// ========================================================================
// Helper: Map EPGXEventResult to display text + color
// ========================================================================
static void MapResultToDisplay(EPGXEventResult Result, FString& OutText, FLinearColor& OutColor)
{
	switch (Result)
	{
	case EPGXEventResult::Success:  OutText = TEXT("OK");      OutColor = PGX::Semantic::Good;  break;
	case EPGXEventResult::Partial:  OutText = TEXT("Partial"); OutColor = PGX::Semantic::Warn;  break;
	case EPGXEventResult::Failed:   OutText = TEXT("FAIL");    OutColor = PGX::Semantic::Error;  break;
	case EPGXEventResult::Skipped:  OutText = TEXT("Skip");    OutColor = PGX::Text::Muted;  break;
	case EPGXEventResult::NotFound: OutText = TEXT("N/F");     OutColor = FLinearColor(0.9f, 0.5f, 0.2f);  break;
	case EPGXEventResult::Blocked:  OutText = TEXT("Blocked"); OutColor = PGX::Semantic::Error;  break;
	default:                        OutText = TEXT("?");       OutColor = FLinearColor(0.5f, 0.5f, 0.5f);  break;
	}
}

// ========================================================================
// Row Widgets for SListView tables
// ========================================================================

class SPGXHandlerTableRow : public SMultiColumnTableRow<TSharedPtr<FPGXHandlerRowData>>
{
public:
	SLATE_BEGIN_ARGS(SPGXHandlerTableRow) {}
		SLATE_ARGUMENT(TSharedPtr<FPGXHandlerRowData>, Item)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable)
	{
		Item = InArgs._Item;
		SMultiColumnTableRow::Construct(FSuperRowType::FArguments(), InOwnerTable);
	}

	TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
	{
		if (!Item.IsValid()) return SNullWidget::NullWidget;

		if (ColumnName == "Tag")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->LeafTag))
				.ToolTipText(FText::FromString(Item->FullTag))
			];
		}
		if (ColumnName == "Class")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock).Text(FText::FromString(Item->ClassName))
				.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
			];
		}
		if (ColumnName == "Category")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->CategoryLeaf))
				.ToolTipText(FText::FromString(Item->CategoryFull))
				.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
			];
		}
		if (ColumnName == "Lifecycle")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->LifecycleName))
				.Font(PGX::Font::BodySmall())
			];
		}
		if (ColumnName == "Cached")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock)
				.Text(Item->bCached ? LOCTEXT("CachedYes", "Yes") : LOCTEXT("CachedNo", "No"))
				.ColorAndOpacity(Item->bCached
					? FSlateColor(PGX::Semantic::Good)
					: FSlateColor(PGX::Text::Muted))
			];
		}
		if (ColumnName == "Enabled")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock)
				.Text(Item->bEnabled ? LOCTEXT("EnabledYes", "Yes") : LOCTEXT("EnabledNo", "DISABLED"))
				.ColorAndOpacity(Item->bEnabled
					? FSlateColor(FLinearColor::White)
					: FSlateColor(PGX::Semantic::Error))
			];
		}
		return SNullWidget::NullWidget;
	}

private:
	TSharedPtr<FPGXHandlerRowData> Item;
};

class SPGXTelemetryTableRow : public SMultiColumnTableRow<TSharedPtr<FPGXTelemetryRowData>>
{
public:
	SLATE_BEGIN_ARGS(SPGXTelemetryTableRow) {}
		SLATE_ARGUMENT(TSharedPtr<FPGXTelemetryRowData>, Item)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable)
	{
		Item = InArgs._Item;
		SMultiColumnTableRow::Construct(FSuperRowType::FArguments(), InOwnerTable);
	}

	TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
	{
		if (!Item.IsValid()) return SNullWidget::NullWidget;

		if (ColumnName == "Tag")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->LeafTag))
				.ToolTipText(FText::FromString(Item->FullTag))
			];
		}
		if (ColumnName == "Exec")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock).Text(FText::AsNumber(Item->ExecutionCount))
			];
		}
		if (ColumnName == "OK")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock)
				.Text(FText::AsNumber(Item->SuccessCount))
				.ColorAndOpacity(FSlateColor(PGX::Semantic::Good))
			];
		}
		if (ColumnName == "Fail")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock)
				.Text(FText::AsNumber(Item->FailCount))
				.ColorAndOpacity(Item->FailCount > 0
					? FSlateColor(PGX::Semantic::Error)
					: FSlateColor(PGX::Text::Muted))
			];
		}
		if (ColumnName == "Skip")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock)
				.Text(FText::AsNumber(Item->SkipCount))
				.ColorAndOpacity(Item->SkipCount > 0
					? FSlateColor(PGX::Semantic::Warn)
					: FSlateColor(PGX::Text::Muted))
			];
		}
		if (ColumnName == "Avg")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("%.2f"), Item->AvgMs)))
			];
		}
		if (ColumnName == "Max")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("%.2f"), Item->MaxMs)))
			];
		}
		return SNullWidget::NullWidget;
	}

private:
	TSharedPtr<FPGXTelemetryRowData> Item;
};

class SPGXBlackboxTableRow : public SMultiColumnTableRow<TSharedPtr<FPGXBlackboxRowData>>
{
public:
	SLATE_BEGIN_ARGS(SPGXBlackboxTableRow) {}
		SLATE_ARGUMENT(TSharedPtr<FPGXBlackboxRowData>, Item)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable)
	{
		Item = InArgs._Item;
		SMultiColumnTableRow::Construct(FSuperRowType::FArguments(), InOwnerTable);
	}

	TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
	{
		if (!Item.IsValid()) return SNullWidget::NullWidget;

		if (ColumnName == "Time")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock)
				.Text(PGXEditorUtils::FormatRelativeTime(Item->Timestamp))
				.ToolTipText(FText::FromString(FString::Printf(TEXT("%.3f s"), Item->Timestamp)))
				.Font(PGX::Font::BodySmall())
			];
		}
		if (ColumnName == "EventTag")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->LeafTag))
				.ToolTipText(FText::FromString(Item->FullTag))
			];
		}
		if (ColumnName == "Result")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->ResultText))
				.ColorAndOpacity(FSlateColor(Item->ResultColor))
				.Font(PGX::Font::Badge())
			];
		}
		if (ColumnName == "Handler")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->HandlerClassName))
				.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
				.Font(PGX::Font::BodySmall())
			];
		}
		if (ColumnName == "Duration")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("%.2f"), Item->ExecutionTimeMs)))
				.Font(PGX::Font::BodySmall())
			];
		}
		return SNullWidget::NullWidget;
	}

private:
	TSharedPtr<FPGXBlackboxRowData> Item;
};

// ========================================================================
// SPGXEventDebuggerTab Implementation
// ========================================================================

void SPGXEventDebuggerTab::Construct(const FArguments& /*InArgs*/)
{
	PGX_LOG_INFO(LogPGXEventHandler, TEXT("[EventDebugger] Construct — building panel with universal shell"));

	ChildSlot
	[
		// EN: Premium Shell — covers UE chrome, provides title bar + footer
		// ES: Shell Premium — cubre chrome de UE, provee title bar + footer
		SNew(SPGXPremiumShell)
		.SystemColor(PGX::System::EventHandler)
		.Title(LOCTEXT("PanelTitle", "Event Debugger"))
		.Subtitle(LOCTEXT("PanelSubtitle", "Behavior resolution bus live monitor"))
		.Icon(FPGXEditorStyle::Get().GetBrush("PGXEditor.Icon.EventHandler"))
		.bShowFooter(true)
		.FooterLeftContent()
		[
			SAssignNew(FooterStatusText, STextBlock)
			.Text(LOCTEXT("FooterConnected", "Connected to PIE"))
			.Font(PGX::Font::BodySmall())
			.ColorAndOpacity(FSlateColor(PGX::Semantic::Good))
		]
		.Content()
		[
			SAssignNew(ContentSwitcher, SWidgetSwitcher)
			.WidgetIndex(0)

			// EN: Slot 0 — Empty state (no PIE) / ES: Slot 0 — Estado vacio (sin PIE)
			+ SWidgetSwitcher::Slot()
			[
				SNew(SPGXEmptyStateV2)
				.Icon(FAppStyle::GetBrush("Icons.Warning"))
				.Message(LOCTEXT("EmptyReason", "No PIE session active"))
				.Hint(LOCTEXT("EmptySuggestion", "Start Play-In-Editor to see live event handler data"))
			]

			// EN: Slot 1 — Live content / ES: Slot 1 — Contenido live
			+ SWidgetSwitcher::Slot()
			[
				SNew(SVerticalBox)

				// EN: Status line / ES: Linea de estado
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 4)
				[
					SAssignNew(StatusText, STextBlock)
					.Text(LOCTEXT("StatusLoading", "Status: Waiting for PIE..."))
					.Font(PGX::Font::Body())
				]

				// EN: KPI chips row / ES: Fila de chips KPI
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 4)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
					[
						SNew(SPGXKPIChip)
						.Label(LOCTEXT("RegisteredKPI", "REGISTERED"))
						.AccentColor(PGX::System::EventHandler)
						.ValueWidget()
						[
							SAssignNew(RegisteredText, STextBlock)
							.Text(LOCTEXT("RegisteredDefault", "0"))
							.Font(PGX::Font::KPIValue())
						]
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
					[
						SNew(SPGXKPIChip)
						.Label(LOCTEXT("CachedKPI", "CACHED"))
						.AccentColor(PGX::System::EventHandler)
						.ValueWidget()
						[
							SAssignNew(CachedText, STextBlock)
							.Text(LOCTEXT("CachedDefault", "0"))
							.Font(PGX::Font::KPIValue())
						]
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
					[
						SNew(SPGXKPIChip)
						.Label(LOCTEXT("HitRatioKPI", "HIT RATIO"))
						.AccentColor(PGX::System::EventHandler)
						.ValueWidget()
						[
							SAssignNew(HitRatioText, STextBlock)
							.Text(LOCTEXT("HitRatioDefault", "--"))
							.Font(PGX::Font::KPIValue())
						]
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(SPGXKPIChip)
						.Label(LOCTEXT("EvictionsKPI", "EVICTIONS"))
						.AccentColor(PGX::System::EventHandler)
						.ValueWidget()
						[
							SAssignNew(EvictionsText, STextBlock)
							.Text(LOCTEXT("EvictionsDefault", "0"))
							.Font(PGX::Font::KPIValue())
						]
					]
				]

				// EN: Body (scrollable sections) / ES: Cuerpo (secciones scrolleables)
				+ SVerticalBox::Slot().FillHeight(1.0f)
				[
					SNew(SScrollBox)
					+ SScrollBox::Slot().Padding(0, 4)
					[
						BuildHandlerBrowserSection()
					]
					+ SScrollBox::Slot().Padding(0, 0)
					[
						SNew(SSeparator)
					]
					+ SScrollBox::Slot().Padding(0, 4)
					[
						BuildGraphSection()
					]
					+ SScrollBox::Slot().Padding(0, 0)
					[
						SNew(SSeparator)
					]
					+ SScrollBox::Slot().Padding(0, 4)
					[
						BuildTelemetrySection()
					]
					+ SScrollBox::Slot().Padding(0, 0)
					[
						SNew(SSeparator)
					]
					+ SScrollBox::Slot().Padding(0, 4)
					[
						BuildBlackboxSection()
					]
					+ SScrollBox::Slot().Padding(0, 0)
					[
						SNew(SSeparator)
					]
					+ SScrollBox::Slot().Padding(0, 4)
					[
						BuildActionsSection()
					]
				]
			]
		]
	];

	// EN: Bind PIE lifecycle delegates (S1) / ES: Bindear delegates de ciclo de vida PIE (S1)
	BindPIEDelegates();

	// EN: If PIE is already running, bind immediately / ES: Si PIE ya esta corriendo, bindear inmediatamente
	if (GEditor && GEditor->PlayWorld)
	{
		PGX_LOG_INFO(LogPGXEventHandler, TEXT("[EventDebugger] PIE already active at Construct time — binding to subsystem"));
		BindToSubsystem();
		if (ContentSwitcher.IsValid())
		{
			ContentSwitcher->SetActiveWidgetIndex(1);
		}
	}
}

SPGXEventDebuggerTab::~SPGXEventDebuggerTab()
{
	PGX_LOG_INFO(LogPGXEventHandler, TEXT("[EventDebugger] Destructor — unbinding all delegates"));

	// EN: Unbind subsystem delegates first / ES: Desbindear delegates del subsistema primero
	UnbindFromSubsystem();

	// EN: Unbind PIE lifecycle delegates (S1 symmetric) / ES: Desbindear delegates de ciclo de vida PIE (S1 simetrico)
	if (PIEStartedHandle.IsValid())
	{
		FEditorDelegates::PostPIEStarted.Remove(PIEStartedHandle);
		PIEStartedHandle.Reset();
	}
	if (PIEEndedHandle.IsValid())
	{
		FEditorDelegates::EndPIE.Remove(PIEEndedHandle);
		PIEEndedHandle.Reset();
	}
}

void SPGXEventDebuggerTab::BindPIEDelegates()
{
	PIEStartedHandle = FEditorDelegates::PostPIEStarted.AddSP(SharedThis(this), &SPGXEventDebuggerTab::OnPIEStarted);
	PIEEndedHandle = FEditorDelegates::EndPIE.AddSP(SharedThis(this), &SPGXEventDebuggerTab::OnPIEEnded);
	PGX_LOG_INFO(LogPGXEventHandler, TEXT("[EventDebugger] PIE delegates bound (PostPIEStarted + EndPIE)"));
}

void SPGXEventDebuggerTab::OnPIEStarted(bool /*bIsSimulating*/)
{
	PGX_LOG_INFO(LogPGXEventHandler, TEXT("[EventDebugger] OnPIEStarted — switching to live view"));

	BindToSubsystem();

	// EN: Clear telemetry graphs for fresh PIE session / ES: Limpiar grafos de telemetria para sesion PIE fresca
	if (ExecTimeGraph.IsValid()) { ExecTimeGraph->Clear(); }
	if (CacheHitSparkline.IsValid()) { CacheHitSparkline->Clear(); }

	if (ContentSwitcher.IsValid())
	{
		ContentSwitcher->SetActiveWidgetIndex(1);
	}
	if (FooterStatusText.IsValid())
	{
		FooterStatusText->SetText(LOCTEXT("FooterConnected", "Connected to PIE"));
		FooterStatusText->SetColorAndOpacity(FSlateColor(PGX::Semantic::Good));
	}
}

void SPGXEventDebuggerTab::OnPIEEnded(bool /*bIsSimulating*/)
{
	PGX_LOG_INFO(LogPGXEventHandler, TEXT("[EventDebugger] OnPIEEnded — switching to empty state"));

	UnbindFromSubsystem();

	if (ContentSwitcher.IsValid())
	{
		ContentSwitcher->SetActiveWidgetIndex(0);
	}
}

void SPGXEventDebuggerTab::BindToSubsystem()
{
	UPGXEventHandlerSubsystem* Sub = GetEventHandlerSubsystem();
	if (!IsValid(Sub))
	{
		PGX_LOG_WARNING(LogPGXEventHandler, TEXT("[EventDebugger] BindToSubsystem — subsystem not available"));
		return;
	}

	// EN: Bind native delegate for real-time blackbox updates / ES: Bindear delegate nativo para updates en tiempo real
	if (!HandlerExecutedHandle.IsValid())
	{
		HandlerExecutedHandle = Sub->OnHandlerExecutedNative.AddSP(
			SharedThis(this), &SPGXEventDebuggerTab::OnHandlerExecuted);
		PGX_LOG_INFO(LogPGXEventHandler, TEXT("[EventDebugger] Bound to OnHandlerExecutedNative"));
	}
}

void SPGXEventDebuggerTab::UnbindFromSubsystem()
{
	UPGXEventHandlerSubsystem* Sub = GetEventHandlerSubsystem();
	if (IsValid(Sub) && HandlerExecutedHandle.IsValid())
	{
		Sub->OnHandlerExecutedNative.Remove(HandlerExecutedHandle);
		PGX_LOG_INFO(LogPGXEventHandler, TEXT("[EventDebugger] Unbound from OnHandlerExecutedNative"));
	}
	HandlerExecutedHandle.Reset();
}

void SPGXEventDebuggerTab::OnHandlerExecuted(FGameplayTag EventTag, EPGXEventResult Result, double ExecutionTimeMs)
{
	// EN: Immediately append to blackbox list (no need to wait for tick)
	// ES: Agregar inmediatamente al blackbox (sin esperar tick)
	auto Row = MakeShared<FPGXBlackboxRowData>();
	Row->Timestamp = FPlatformTime::Seconds();
	Row->FullTag = EventTag.ToString();
	Row->LeafTag = PGXEditorUtils::TagToLeafName(EventTag);
	Row->ExecutionTimeMs = static_cast<float>(ExecutionTimeMs);

	MapResultToDisplay(Result, Row->ResultText, Row->ResultColor);

	// EN: Push execution time to telemetry graph / ES: Empujar tiempo de ejecucion al grafo de telemetria
	if (ExecTimeGraph.IsValid())
	{
		ExecTimeGraph->PushValue(static_cast<float>(ExecutionTimeMs));
	}

	// EN: Insert newest first, cap at 100 entries / ES: Insertar mas reciente primero, maximo 100 entradas
	BlackboxItems.Insert(Row, 0);
	if (BlackboxItems.Num() > 100)
	{
		BlackboxItems.SetNum(100);
	}

	if (BlackboxListView.IsValid())
	{
		BlackboxListView->RequestListRefresh();
	}

	PGX_LOG_VERBOSE(LogPGXEventHandler, TEXT("[EventDebugger] OnHandlerExecuted — %s -> %s (%.2fms)"),
		*EventTag.ToString(), *Row->ResultText, ExecutionTimeMs);
}

void SPGXEventDebuggerTab::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	RefreshAccumulator += InDeltaTime;
	if (RefreshAccumulator >= RefreshInterval)
	{
		RefreshAccumulator = 0.0f;
		RefreshData();
	}
}

// EN: BuildOverviewSection — now inlined into Construct via SPGXPremiumShell + SPGXKPIChip
// ES: BuildOverviewSection — ahora inline en Construct via SPGXPremiumShell + SPGXKPIChip

TSharedRef<SWidget> SPGXEventDebuggerTab::BuildGraphSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("ExecTelemetryTitle", "EXECUTION TELEMETRY"))
			.AccentColor(PGX::System::EventHandler)
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(0, 4)
		[
			SNew(SHorizontalBox)

			// EN: [70%] Execution time graph / ES: [70%] Grafico de tiempo de ejecucion
			+ SHorizontalBox::Slot()
			.FillWidth(0.7f)
			.Padding(0, 0, PGX::Spacing::MD, 0)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ExecTimeLabel", "Handler Execution Time (ms)"))
					.Font(PGX::Font::SubHeader())
					.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
				[
					SNew(SBox)
					.HeightOverride(PGX::Height::GraphDefault)
					[
						SAssignNew(ExecTimeGraph, SPGXTelemetryGraph)
						.BufferSize(256)
						.LineColor(PGX::System::EventHandler)
						.MinValue(0.0f)
						.MaxValue(10.0f)
						.bShowGrid(true)
						.bShowAvgLine(true)
						.bShowPeakMarkers(true)
						.bShowLabels(true)
					]
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					PGXEditorUtils::BuildGraphLegend(ExecTimeGraph, PGX::System::EventHandler)
				]
			]

			// EN: [30%] Cache hit ratio sparkline / ES: [30%] Sparkline de ratio de cache hits
			+ SHorizontalBox::Slot()
			.FillWidth(0.3f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("CacheHitLabel", "Cache Hit Ratio (%)"))
					.Font(PGX::Font::SubHeader())
					.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
				[
					SNew(SBox)
					.HeightOverride(PGX::Height::GraphCompact)
					[
						SAssignNew(CacheHitSparkline, SPGXSparkline)
						.LineColor(PGX::Semantic::Good)
					]
				]
			]
		];
}

TSharedRef<SWidget> SPGXEventDebuggerTab::BuildHandlerBrowserSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("HandlersTitle", "HANDLER BROWSER"))
			.AccentColor(PGX::System::EventHandler)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 4)
		[
			SNew(SBox)
			.MaxDesiredHeight(200.0f)
			[
				SAssignNew(HandlerListView, SListView<TSharedPtr<FPGXHandlerRowData>>)
				.ListItemsSource(&HandlerItems)
				.OnGenerateRow(this, &SPGXEventDebuggerTab::OnGenerateHandlerRow)
				.HeaderRow
				(
					SNew(SHeaderRow)
					+ SHeaderRow::Column("Tag")
						.DefaultLabel(LOCTEXT("HColTag", "Event Tag"))
						.FillWidth(1.0f)
					+ SHeaderRow::Column("Class")
						.DefaultLabel(LOCTEXT("HColClass", "Handler Class"))
						.FillWidth(0.7f)
					+ SHeaderRow::Column("Category")
						.DefaultLabel(LOCTEXT("HColCat", "Category"))
						.FillWidth(0.5f)
					+ SHeaderRow::Column("Lifecycle")
						.DefaultLabel(LOCTEXT("HColLife", "Lifecycle"))
						.FillWidth(0.4f)
					+ SHeaderRow::Column("Cached")
						.DefaultLabel(LOCTEXT("HColCached", "Cached"))
						.FillWidth(0.3f)
					+ SHeaderRow::Column("Enabled")
						.DefaultLabel(LOCTEXT("HColEnabled", "Enabled"))
						.FillWidth(0.3f)
				)
			]
		];
}

TSharedRef<SWidget> SPGXEventDebuggerTab::BuildTelemetrySection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("TelemetryTitle", "TELEMETRY DASHBOARD"))
			.AccentColor(PGX::System::EventHandler)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 4)
		[
			SNew(SBox)
			.MaxDesiredHeight(200.0f)
			[
				SAssignNew(TelemetryListView, SListView<TSharedPtr<FPGXTelemetryRowData>>)
				.ListItemsSource(&TelemetryItems)
				.OnGenerateRow(this, &SPGXEventDebuggerTab::OnGenerateTelemetryRow)
				.HeaderRow
				(
					SNew(SHeaderRow)
					+ SHeaderRow::Column("Tag")
						.DefaultLabel(LOCTEXT("TColTag", "Event Tag"))
						.FillWidth(1.0f)
					+ SHeaderRow::Column("Exec")
						.DefaultLabel(LOCTEXT("TColExec", "Exec"))
						.FillWidth(0.3f)
					+ SHeaderRow::Column("OK")
						.DefaultLabel(LOCTEXT("TColOK", "OK"))
						.FillWidth(0.3f)
					+ SHeaderRow::Column("Fail")
						.DefaultLabel(LOCTEXT("TColFail", "Fail"))
						.FillWidth(0.3f)
					+ SHeaderRow::Column("Skip")
						.DefaultLabel(LOCTEXT("TColSkip", "Skip"))
						.FillWidth(0.3f)
					+ SHeaderRow::Column("Avg")
						.DefaultLabel(LOCTEXT("TColAvg", "Avg(ms)"))
						.FillWidth(0.35f)
					+ SHeaderRow::Column("Max")
						.DefaultLabel(LOCTEXT("TColMax", "Max(ms)"))
						.FillWidth(0.35f)
				)
			]
		];
}

TSharedRef<SWidget> SPGXEventDebuggerTab::BuildBlackboxSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("BlackboxTitle", "BLACKBOX HISTORY"))
			.AccentColor(PGX::System::EventHandler)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 4)
		[
			SNew(SBox)
			.MaxDesiredHeight(200.0f)
			[
				SAssignNew(BlackboxListView, SListView<TSharedPtr<FPGXBlackboxRowData>>)
				.ListItemsSource(&BlackboxItems)
				.OnGenerateRow(this, &SPGXEventDebuggerTab::OnGenerateBlackboxRow)
				.HeaderRow
				(
					SNew(SHeaderRow)
					+ SHeaderRow::Column("Time")
						.DefaultLabel(LOCTEXT("BBColTime", "Time"))
						.FillWidth(0.4f)
					+ SHeaderRow::Column("EventTag")
						.DefaultLabel(LOCTEXT("BBColTag", "Event Tag"))
						.FillWidth(1.0f)
					+ SHeaderRow::Column("Result")
						.DefaultLabel(LOCTEXT("BBColResult", "Result"))
						.FillWidth(0.35f)
					+ SHeaderRow::Column("Handler")
						.DefaultLabel(LOCTEXT("BBColHandler", "Handler"))
						.FillWidth(0.7f)
					+ SHeaderRow::Column("Duration")
						.DefaultLabel(LOCTEXT("BBColDuration", "ms"))
						.FillWidth(0.3f)
				)
			]
		];
}

TSharedRef<SWidget> SPGXEventDebuggerTab::BuildActionsSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("ActionsTitle", "ACTIONS"))
			.AccentColor(PGX::System::EventHandler)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 4)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("ClearCache", "Clear Cache"))
				.ToolTipText(LOCTEXT("ClearCacheTip", "Invalidate all cached handler instances"))
				.OnClicked_Lambda([this]() -> FReply
				{
					if (UPGXEventHandlerSubsystem* Sub = GetEventHandlerSubsystem())
					{
						Sub->InvalidateCache();
						PGX_LOG_INFO(LogPGXEventHandler, TEXT("[EventDebugger] Action: Clear Cache executed"));
					}
					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("ResetTelemetry", "Reset Telemetry"))
				.ToolTipText(LOCTEXT("ResetTelemetryTip", "Reset all execution counters and timing data"))
				.OnClicked_Lambda([this]() -> FReply
				{
					if (UPGXEventHandlerSubsystem* Sub = GetEventHandlerSubsystem())
					{
						Sub->ResetTelemetry();
						PGX_LOG_INFO(LogPGXEventHandler, TEXT("[EventDebugger] Action: Reset Telemetry executed"));
					}
					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("ExportReport", "Export Report"))
				.ToolTipText(LOCTEXT("ExportReportTip", "Export a full markdown report to Saved/PGX/"))
				.OnClicked_Lambda([this]() -> FReply
				{
					if (UPGXEventHandlerSubsystem* Sub = GetEventHandlerSubsystem())
					{
						const FString Report = Sub->ExportReport();
						const FString DirPath = FPaths::ProjectSavedDir() / TEXT("PGX");
						const FString FilePath = DirPath / TEXT("EventHandler_Report.md");

						// EN: Ensure directory exists before writing / ES: Asegurar que el directorio existe antes de escribir
						IFileManager::Get().MakeDirectory(*DirPath, true);

						if (FFileHelper::SaveStringToFile(Report, *FilePath))
						{
							PGX_LOG_INFO(LogPGXEventHandler, TEXT("[EventDebugger] Action: Report exported to: %s"), *FilePath);
							UPGXEditorNotification::ShowInfo(FText::Format(
								NSLOCTEXT("PGXEventDebugger", "ExportSuccess", "EventHandler report exported to {0}"),
								FText::FromString(FilePath)));
						}
						else
						{
							PGX_LOG_ERROR(LogPGXEventHandler, TEXT("[EventDebugger] Action: Failed to export report to: %s"), *FilePath);
							UPGXEditorNotification::ShowError(
								NSLOCTEXT("PGXEventDebugger", "ExportFailed", "Failed to export EventHandler report. Check Output Log."));
						}
					}
					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("ExecuteTest", "Execute Test Handler"))
				.ToolTipText(LOCTEXT("ExecuteTestTip", "Execute a handler for PGX.EventHandler.Test tag with empty payload"))
				.OnClicked_Lambda([this]() -> FReply
				{
					if (UPGXEventHandlerSubsystem* Sub = GetEventHandlerSubsystem())
					{
						FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(
							FName(TEXT("PGX.EventHandler.Test")), false);
						if (TestTag.IsValid())
						{
							FInstancedStruct EmptyPayload;
							Sub->ResolveAndExecute(TestTag, nullptr, EmptyPayload);
							PGX_LOG_INFO(LogPGXEventHandler, TEXT("[EventDebugger] Action: Execute Test Handler triggered"));
						}
						else
						{
							PGX_LOG_WARNING(LogPGXEventHandler, TEXT("[EventDebugger] Action: PGX.EventHandler.Test tag not found — cannot execute"));
						}
					}
					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("SimulateBurst", "Simulate Burst (5x)"))
				.ToolTipText(LOCTEXT("SimulateBurstTip", "Execute the first 5 registered handlers to generate telemetry data"))
				.OnClicked_Lambda([this]() -> FReply
				{
					if (UPGXEventHandlerSubsystem* Sub = GetEventHandlerSubsystem())
					{
						const TArray<FGameplayTag> AllTags = Sub->GetAllRegisteredTags();
						const int32 Limit = FMath::Min(AllTags.Num(), 5);
						FInstancedStruct EmptyPayload;
						for (int32 i = 0; i < Limit; ++i)
						{
							Sub->ResolveAndExecute(AllTags[i], nullptr, EmptyPayload);
						}
						PGX_LOG_INFO(LogPGXEventHandler, TEXT("[EventDebugger] Action: Simulate Burst — executed %d handlers"), Limit);
					}
					return FReply::Handled();
				})
			]
		];
}

TSharedRef<ITableRow> SPGXEventDebuggerTab::OnGenerateHandlerRow(
	TSharedPtr<FPGXHandlerRowData> Item, const TSharedRef<STableViewBase>& Owner)
{
	return SNew(SPGXHandlerTableRow, Owner)
		.Item(Item);
}

TSharedRef<ITableRow> SPGXEventDebuggerTab::OnGenerateTelemetryRow(
	TSharedPtr<FPGXTelemetryRowData> Item, const TSharedRef<STableViewBase>& Owner)
{
	return SNew(SPGXTelemetryTableRow, Owner)
		.Item(Item);
}

TSharedRef<ITableRow> SPGXEventDebuggerTab::OnGenerateBlackboxRow(
	TSharedPtr<FPGXBlackboxRowData> Item, const TSharedRef<STableViewBase>& Owner)
{
	return SNew(SPGXBlackboxTableRow, Owner)
		.Item(Item);
}

UPGXEventHandlerSubsystem* SPGXEventDebuggerTab::GetEventHandlerSubsystem() const
{
	if (GEditor)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::PIE && IsValid(Context.World()))
			{
				UGameInstance* GI = Context.World()->GetGameInstance();
				if (IsValid(GI))
				{
					return GI->GetSubsystem<UPGXEventHandlerSubsystem>();
				}
			}
		}
	}
	return nullptr;
}

void SPGXEventDebuggerTab::RefreshData()
{
	UPGXEventHandlerSubsystem* Sub = GetEventHandlerSubsystem();
	if (!IsValid(Sub))
	{
		// EN: Switch to empty state if not already / ES: Cambiar a estado vacio si no lo esta ya
		if (ContentSwitcher.IsValid() && ContentSwitcher->GetActiveWidgetIndex() != 0)
		{
			ContentSwitcher->SetActiveWidgetIndex(0);
		}
		return;
	}

	// EN: Ensure live view is active / ES: Asegurar que la vista live esta activa
	if (ContentSwitcher.IsValid() && ContentSwitcher->GetActiveWidgetIndex() != 1)
	{
		ContentSwitcher->SetActiveWidgetIndex(1);
	}

	const FPGXHandlerCacheStats CacheStats = Sub->GetCacheStats();
	const TArray<FGameplayTag> AllTags = Sub->GetAllRegisteredTags();

	// EN: Overview metrics / ES: Metricas del resumen
	if (StatusText.IsValid())
	{
		StatusText->SetText(FText::FromString(FString::Printf(TEXT("Status: Active | Handlers: %d | Cache: %d/%d"),
			AllTags.Num(), CacheStats.CachedHandlers, CacheStats.MaxCachedHandlers)));
	}
	if (RegisteredText.IsValid()) RegisteredText->SetText(FText::AsNumber(AllTags.Num()));
	if (CachedText.IsValid())
	{
		CachedText->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"),
			CacheStats.CachedHandlers, CacheStats.MaxCachedHandlers)));
	}
	if (HitRatioText.IsValid())
	{
		const int32 Total = CacheStats.CacheHits + CacheStats.CacheMisses;
		if (Total > 0)
		{
			const float Ratio = static_cast<float>(CacheStats.CacheHits) / static_cast<float>(Total) * 100.0f;
			HitRatioText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), Ratio)));
		}
		else
		{
			HitRatioText->SetText(FText::FromString(TEXT("N/A")));
		}
	}
	if (EvictionsText.IsValid()) EvictionsText->SetText(FText::AsNumber(CacheStats.Evictions));

	// EN: Push cache hit ratio to sparkline / ES: Empujar ratio de cache hits al sparkline
	if (CacheHitSparkline.IsValid())
	{
		const int32 Total = CacheStats.CacheHits + CacheStats.CacheMisses;
		const float HitRatio = Total > 0
			? static_cast<float>(CacheStats.CacheHits) / static_cast<float>(Total) * 100.0f
			: 0.0f;
		CacheHitSparkline->PushValue(HitRatio);
	}

	// EN: Handler Browser — populate table / ES: Browser de Handlers — popular tabla
	{
		HandlerItems.Empty(AllTags.Num());
		for (const FGameplayTag& EventTag : AllTags)
		{
			const FPGXEventHandlerInfo Info = Sub->GetHandlerInfo(EventTag);
			auto Row = MakeShared<FPGXHandlerRowData>();
			Row->FullTag = EventTag.ToString();
			Row->LeafTag = PGXEditorUtils::TagToLeafName(EventTag);
			Row->ClassName = Info.HandlerClassName;
			Row->bCached = Info.bCached;
			Row->bEnabled = Info.bEnabled;

			// EN: Category + Lifecycle columns (Change 5) / ES: Columnas de Categoria + Ciclo de vida (Cambio 5)
			Row->CategoryLeaf = Info.CategoryTag.IsValid()
				? PGXEditorUtils::TagToLeafName(Info.CategoryTag) : TEXT("-");
			Row->CategoryFull = Info.CategoryTag.IsValid()
				? Info.CategoryTag.ToString() : TEXT("-");

			switch (Info.Lifecycle)
			{
			case EPGXHandlerLifecycle::Singleton:  Row->LifecycleName = TEXT("Singleton");  break;
			case EPGXHandlerLifecycle::Cached:     Row->LifecycleName = TEXT("Cached");     break;
			case EPGXHandlerLifecycle::Ephemeral:  Row->LifecycleName = TEXT("Ephemeral");  break;
			default:                               Row->LifecycleName = TEXT("?");           break;
			}

			HandlerItems.Add(Row);
		}
		if (HandlerListView.IsValid()) HandlerListView->RequestListRefresh();
	}

	// EN: Telemetry — populate table / ES: Telemetria — popular tabla
	{
		const TArray<FPGXHandlerTelemetry> AllTelemetry = Sub->GetAllTelemetry();
		TelemetryItems.Empty(AllTelemetry.Num());
		for (const FPGXHandlerTelemetry& Tel : AllTelemetry)
		{
			auto Row = MakeShared<FPGXTelemetryRowData>();
			Row->FullTag = Tel.EventTag.ToString();
			Row->LeafTag = PGXEditorUtils::TagToLeafName(Tel.EventTag);
			Row->ExecutionCount = Tel.ExecutionCount;
			Row->SuccessCount = Tel.SuccessCount;
			Row->FailCount = Tel.FailCount;
			Row->SkipCount = Tel.SkipCount;
			Row->AvgMs = static_cast<float>(Tel.AvgExecutionTimeMs);
			Row->MaxMs = static_cast<float>(Tel.MaxExecutionTimeMs);
			TelemetryItems.Add(Row);
		}
		if (TelemetryListView.IsValid()) TelemetryListView->RequestListRefresh();
	}

	// EN: Blackbox — populate from subsystem entries (Change 4) / ES: Blackbox — popular desde entries del subsistema (Cambio 4)
	// NOTE: Only populate from subsystem if blackbox is empty (delegate-driven entries take priority)
	if (BlackboxItems.Num() == 0)
	{
		const TArray<FPGXBlackboxEntry>& Entries = Sub->GetBlackboxEntries();
		BlackboxItems.Empty(Entries.Num());
		for (int32 i = Entries.Num() - 1; i >= 0; --i) // EN: Newest first / ES: Mas reciente primero
		{
			const FPGXBlackboxEntry& Entry = Entries[i];
			auto Row = MakeShared<FPGXBlackboxRowData>();
			Row->Timestamp = Entry.Timestamp;
			Row->FullTag = Entry.EventTag.ToString();
			Row->LeafTag = PGXEditorUtils::TagToLeafName(Entry.EventTag);
			Row->HandlerClassName = Entry.HandlerClassName;
			Row->ExecutionTimeMs = static_cast<float>(Entry.ExecutionTimeMs);
			Row->InstigatorName = Entry.InstigatorName;

			MapResultToDisplay(Entry.Result, Row->ResultText, Row->ResultColor);
			BlackboxItems.Add(Row);
		}
		if (BlackboxListView.IsValid()) BlackboxListView->RequestListRefresh();
	}

	// EN: Update footer / ES: Actualizar footer
	if (FooterStatusText.IsValid())
	{
		FooterStatusText->SetText(FText::Format(
			LOCTEXT("FooterLive", "Connected to PIE | {0} handlers | {1} blackbox entries"),
			FText::AsNumber(AllTags.Num()),
			FText::AsNumber(BlackboxItems.Num())));
		FooterStatusText->SetColorAndOpacity(FSlateColor(PGX::Semantic::Good));
	}
}

#undef LOCTEXT_NAMESPACE
