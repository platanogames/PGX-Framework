// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "TestDashboard/SPGXTestDashboardTab.h"
#include "PGXEditorToolsEditor.h"
#include "Logging/PGXLogMacros.h"

// EN: Slate / ES: Slate
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "Utils/PGXEditorUtils.h"
#include "Style/PGXVisualTokens.h"
#include "Widgets/SPGXPremiumShell.h"
#include "Widgets/SPGXStatusBadge.h"
#include "Widgets/SPGXSectionDivider.h"
#include "Style/PGXEditorStyle.h"
#include "Widgets/SPGXKPIChip.h"
#include "Widgets/SPGXEmptyStateV2.h"

// EN: Editor / ES: Editor
#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformTime.h"
#include "HAL/PlatformFileManager.h"
#include "DesktopPlatformModule.h"

// EN: PGX TestUtilities — one per system / ES: PGX TestUtilities — uno por sistema
#include "Profile/PGXProfileTestUtility.h"
#include "Logging/PGXLogTestUtility.h"
#include "PGXSaveTestUtility.h"
#include "PGXGameFlowTestUtility.h"
#include "PGXPSOTestUtility.h"
#include "PGXLevelFlowTestUtility.h"
#include "PGXLoadingTestUtility.h"
#include "PGXMGOSTestUtility.h"
#include "PGXAudioTestUtility.h"
#include "Registry/PGXRegistryTestUtility.h"
#include "Construction/PGXConstructionTestUtility.h"
#include "Messages/PGXMessageTestUtility.h"
#include "EventHandler/PGXEventHandlerTestUtility.h"

#define LOCTEXT_NAMESPACE "PGXTestDashboard"

// ============================================================================
// EN: System definitions for ordered iteration
// ES: Definiciones de sistemas para iteracion ordenada
// ============================================================================

struct FSystemTestDef
{
	FString Name;
	FString Version;
	bool bRequiresWorldContext;
};

static const TArray<FSystemTestDef> GSystemDefs = {
	{ TEXT("Profile"),      TEXT("v2.0"), true },
	{ TEXT("GameFlow"),     TEXT("v1.0"), true },
	{ TEXT("Log"),          TEXT("v2.2"), true },
	{ TEXT("Save"),         TEXT("v1.0"), true },
	{ TEXT("PSO"),          TEXT("v2.0"), true },
	{ TEXT("LevelFlow"),    TEXT("v1.0"), true },
	{ TEXT("Loading"),      TEXT("v1.0"), true },
	{ TEXT("MGOS"),         TEXT("v1.0"), false },
	{ TEXT("Audio"),        TEXT("v1.1"), true },
	{ TEXT("Registry"),     TEXT("v2.0"), true },
	{ TEXT("Construction"), TEXT("v1.2"), true },
	{ TEXT("Message"),      TEXT("v1.0"), true },
	{ TEXT("EventHandler"), TEXT("v1.0"), true },
};

// ============================================================================
// EN: Construction / Destruction
// ES: Construccion / Destruccion
// ============================================================================

// EN: Test Dashboard accent color / ES: Color de acento del Test Dashboard
static const FLinearColor GTestColor = PGX::Semantic::Good;

void SPGXTestDashboardTab::Construct(const FArguments& /*InArgs*/)
{
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[TestDashboard] Construct"));

	CachedFooterText = LOCTEXT("FooterInit", "Public systems | 0 tests");

	// EN: Bind to collector changes / ES: Enlazar a cambios del collector
	CollectorChangedHandle = FPGXTestResultCollector::Get().OnResultsChanged.AddSP(SharedThis(this), &SPGXTestDashboardTab::OnResultsChanged);

	// EN: Bind to PIE events / ES: Enlazar a eventos PIE
	PIEStartedHandle = FEditorDelegates::PostPIEStarted.AddSP(SharedThis(this), &SPGXTestDashboardTab::OnPIEStarted);
	PIEEndedHandle = FEditorDelegates::EndPIE.AddSP(SharedThis(this), &SPGXTestDashboardTab::OnPIEEnded);

	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[TestDashboard] Bound 3 delegates (Collector, PIEStarted, PIEEnded)"));

	// EN: Check if PIE is already running / ES: Verificar si PIE ya esta corriendo
	bIsPIEActive = (GetPIEWorld() != nullptr);
	if (bIsPIEActive)
	{
		PGX_LOG_INFO(LogPGXEditorTools, TEXT("[TestDashboard] PIE already running at Construct"));
	}

	ChildSlot
	[
		SNew(SPGXPremiumShell)
		.SystemColor(PGX::System::MGOS)
		.Title(LOCTEXT("TestDashboardTitle", "TEST DASHBOARD"))
		.Subtitle(LOCTEXT("TestDashboardSub", "System validation tests"))
		.Icon(FPGXEditorStyle::Get().GetBrush("PGXEditor.Icon.TestDashboard"))
		.bShowFooter(true)
		.StatusText_Lambda([this]() { return CachedFooterText; })
		.StatusColor_Lambda([this]() { return bIsPIEActive ? PGX::Semantic::Good : PGX::Text::Muted; })
		.TitleRightContent()
		[
			BuildToolbar()
		]
		.Content()
		[
			SNew(SVerticalBox)

			// EN: Status bar / ES: Barra de estado
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4.0f, 0.0f)
			[
				BuildStatusBar()
			]

			// EN: Summary bar / ES: Barra de resumen
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4.0f)
			[
				BuildSummaryBar()
			]

			// EN: Filter row / ES: Fila de filtros
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4.0f, 2.0f)
			[
				SNew(SHorizontalBox)

				// EN: Search box / ES: Caja de busqueda
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(SSearchBox)
					.HintText(LOCTEXT("FilterHint", "Filter by system or test name..."))
					.OnTextChanged_Lambda([this](const FText& Text)
					{
						FilterText = Text.ToString();
						PGX_LOG_INFO(LogPGXEditorTools, TEXT("[TestDashboard] Filter changed: '%s'"), *FilterText);
						RefreshResults();
					})
				]

				// EN: Fail-only toggle / ES: Toggle de solo fallidos
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SCheckBox)
					.IsChecked_Lambda([this]() { return bShowFailedOnly ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
					.OnCheckStateChanged_Lambda([this](ECheckBoxState S)
					{
						bShowFailedOnly = (S == ECheckBoxState::Checked);
						PGX_LOG_INFO(LogPGXEditorTools, TEXT("[TestDashboard] Show failed only: %s"), bShowFailedOnly ? TEXT("ON") : TEXT("OFF"));
						RefreshResults();
					})
					[
						SNew(STextBlock)
						.Text(LOCTEXT("FailOnly", "Failures only"))
						.Font(PGX::Font::BodySmall())
					]
				]
			]

			// EN: Separator / ES: Separador
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSeparator)
			]

			// EN: Results area / ES: Area de resultados
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(4.0f)
			[
				BuildResultsArea()
			]
		]
	];
}

SPGXTestDashboardTab::~SPGXTestDashboardTab()
{
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[TestDashboard] Destructor — cleaning up"));

	// EN: Safety teardown — ensure no residual test data if tab is closed during harness run
	// ES: Teardown de seguridad — asegurar que no queden datos residuales si el tab se cierra durante harness
	if (TestHarness.IsActive())
	{
		TestHarness.Teardown();
		PGX_LOG_INFO(LogPGXEditorTools, TEXT("[TestDashboard] Harness teardown in destructor"));
	}

	// EN: Unbind all delegates (S1 compliance) / ES: Desenlazar todos los delegados (cumplimiento S1)
	FPGXTestResultCollector::Get().OnResultsChanged.Remove(CollectorChangedHandle);
	FEditorDelegates::PostPIEStarted.Remove(PIEStartedHandle);
	FEditorDelegates::EndPIE.Remove(PIEEndedHandle);
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[TestDashboard] Unbound 3 delegates"));
}

// ============================================================================
// EN: UI Builders
// ES: Constructores de UI
// ============================================================================

TSharedRef<SWidget> SPGXTestDashboardTab::BuildToolbar()
{
	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.0f, 0.0f, 4.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("RunAll", "Run All Tests"))
			.ToolTipText(LOCTEXT("RunAllTip", "Execute all system validation tests (requires PIE)"))
			.OnClicked(this, &SPGXTestDashboardTab::OnRunAllClicked)
			.IsEnabled_Lambda([this]() { return bIsPIEActive; })
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.0f, 0.0f, 4.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Clear", "Clear"))
			.ToolTipText(LOCTEXT("ClearTip", "Clear all test results"))
			.OnClicked(this, &SPGXTestDashboardTab::OnClearClicked)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.0f, 0.0f, 4.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("ExportJSON", "Export JSON"))
			.ToolTipText(LOCTEXT("ExportJSONTip", "Export results to JSON file in Saved/PGXTests/"))
			.OnClicked(this, &SPGXTestDashboardTab::OnExportJsonClicked)
			.IsEnabled_Lambda([this]() { return FPGXTestResultCollector::Get().GetTotalTests() > 0; })
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.0f, 0.0f, 4.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("ExportMD", "Export MD"))
			.ToolTipText(LOCTEXT("ExportMDTip", "Export results to Markdown file in Saved/PGXTests/"))
			.OnClicked(this, &SPGXTestDashboardTab::OnExportMarkdownClicked)
			.IsEnabled_Lambda([this]() { return FPGXTestResultCollector::Get().GetTotalTests() > 0; })
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.0f, 0.0f, 8.0f, 0.0f)
		[
			SNew(SButton)
			.Text_Lambda([this]() { return ReportLanguage == EPGXReportLanguage::English ? LOCTEXT("LangEN", "EN") : LOCTEXT("LangES", "ES"); })
			.ToolTipText(LOCTEXT("LangToggleTip", "Toggle report language between English and Spanish"))
			.OnClicked(this, &SPGXTestDashboardTab::OnToggleLanguageClicked)
		]

		// EN: Harness Mode toggle / ES: Toggle de Harness Mode
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4.0f, 0.0f, 2.0f, 0.0f)
		[
			SNew(SCheckBox)
			.IsChecked_Lambda([this]() { return bHarnessMode ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
			.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) { bHarnessMode = (NewState == ECheckBoxState::Checked); })
			.ToolTipText(LOCTEXT("HarnessTip", "Inject transient test data into Audio, PSO, Save, Construction, Message, EventHandler before running tests"))
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.0f, 0.0f, 4.0f, 0.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("HarnessLabel", "Harness Mode"))
			.Font(PGX::Font::BodySmall())
		]

		// EN: [HARNESS] badge — visible only when harness is active / ES: Badge [HARNESS] — visible solo cuando el harness esta activo
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(2.0f, 0.0f, 0.0f, 0.0f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(PGX::Semantic::Warn)
			.Padding(FMargin(4.0f, 1.0f))
			.Visibility_Lambda([this]() { return TestHarness.IsActive() ? EVisibility::Visible : EVisibility::Collapsed; })
			[
				SNew(STextBlock)
				.Text(LOCTEXT("HarnessBadge", "[HARNESS]"))
				.Font(PGX::Font::CaptionBold())
				.ColorAndOpacity(FLinearColor::White)
			]
		]

		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNullWidget::NullWidget
		];
}

TSharedRef<SWidget> SPGXTestDashboardTab::BuildStatusBar()
{
	const FLinearColor StatusColor = bIsPIEActive ? PGX::Semantic::Good : PGX::Semantic::Neutral;
	const FString StatusStr = bIsPIEActive ? TEXT("LIVE") : TEXT("OFFLINE");

	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SAssignNew(StatusText, STextBlock)
			.Text(FText::FromString(StatusStr))
			.Font(PGX::Font::SubHeader())
			.ColorAndOpacity(FSlateColor(StatusColor))
		];
}

TSharedRef<SWidget> SPGXTestDashboardTab::BuildSummaryBar()
{
	const int32 Total = FPGXTestResultCollector::Get().GetTotalTests();
	const int32 Passed = FPGXTestResultCollector::Get().GetTotalPassed();
	const int32 Failed = FPGXTestResultCollector::Get().GetTotalFailed();
	const float Progress = Total > 0 ? static_cast<float>(Passed) / static_cast<float>(Total) : 0.0f;

	CachedPassedValue = FText::FromString(Total > 0
		? FString::Printf(TEXT("%d/%d"), Passed, Total) : TEXT("--/--"));
	CachedFailedValue = FText::FromString(Failed > 0
		? FString::Printf(TEXT("%d"), Failed) : TEXT("0"));

	return SNew(SHorizontalBox)

		// EN: Passed KPI chip / ES: KPI chip de aprobados
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.0f, 0.0f, 8.0f, 0.0f)
		[
			SAssignNew(PassedChip, SPGXKPIChip)
			.Label(LOCTEXT("KPIPassed", "Passed"))
			.Value_Lambda([this]() { return CachedPassedValue; })
			.AccentColor(PGX::Semantic::Good)
		]

		// EN: Failed count KPI chip (red, visible only when >0) / ES: KPI chip de fallidos (rojo, visible solo cuando >0)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.0f, 0.0f, 8.0f, 0.0f)
		[
			SAssignNew(FailedChip, SPGXKPIChip)
			.Label(LOCTEXT("KPIFailed", "Failed"))
			.Value_Lambda([this]() { return CachedFailedValue; })
			.AccentColor(PGX::Semantic::Error)
			.Visibility(Failed > 0 ? EVisibility::Visible : EVisibility::Collapsed)
		]

		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		[
			SAssignNew(SummaryProgress, SProgressBar)
			.Percent(Progress)
			.FillColorAndOpacity(Progress >= 1.0f ? PGX::Semantic::Good : PGX::Semantic::Warn)
		]

		// EN: Last run timestamp / ES: Timestamp de ultima ejecucion
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(8.0f, 0.0f, 0.0f, 0.0f)
		[
			SAssignNew(LastRunText, STextBlock)
			.Text(LOCTEXT("NeverRun", "Never run"))
			.Font(PGX::Font::Hint())
			.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
		];
}

TSharedRef<SWidget> SPGXTestDashboardTab::BuildResultsArea()
{
	SAssignNew(ResultsBox, SVerticalBox);

	// EN: Populate with current results / ES: Poblar con resultados actuales
	const TArray<FPGXSystemTestSummary> Summaries = FPGXTestResultCollector::Get().GetSystemSummaries();
	if (Summaries.Num() == 0)
	{
		ResultsBox->AddSlot()
		.AutoHeight()
		.Padding(8.0f, 16.0f)
		[
			SNew(SPGXEmptyStateV2)
			.Message(LOCTEXT("NoResults", "No tests run yet"))
			.Hint(LOCTEXT("NoResultsHint", "Start PIE and click \"Run All Tests\""))
		];
	}
	else
	{
		for (const FPGXSystemTestSummary& Summary : Summaries)
		{
			ResultsBox->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 4.0f)
			[
				BuildSystemSection(Summary)
			];
		}
	}

	return SNew(SVerticalBox)

		// EN: Column header row / ES: Fila de cabecera de columnas
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(12.0f, 2.0f, 0.0f, 2.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 6.0f, 0.0f)
			[
				SNew(SBox)
				.WidthOverride(40.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ColStatus", "STATUS"))
					.Font(PGX::Font::CaptionBold())
					.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
				]
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ColTestName", "TEST NAME"))
				.Font(PGX::Font::CaptionBold())
				.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(8.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ColTime", "TIME"))
				.Font(PGX::Font::CaptionBold())
				.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(80.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ColIssues", "ISSUES"))
					.Font(PGX::Font::CaptionBold())
					.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
				]
			]
		]

		// EN: Scrollable results / ES: Resultados con scroll
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				ResultsBox.ToSharedRef()
			]
		];
}

TSharedRef<SWidget> SPGXTestDashboardTab::BuildSystemSection(const FPGXSystemTestSummary& Summary)
{
	const FLinearColor SystemColor = GetSystemColor(Summary.SystemName);
	const FString HeaderStr = FString::Printf(TEXT("%s %s  [%d/%d]"),
		*Summary.SystemName,
		*Summary.SystemVersion,
		Summary.PassedCount,
		Summary.PassedCount + Summary.FailedCount);

	// EN: Per-system last run text / ES: Texto de ultima ejecucion por sistema
	FString PerSystemTimeStr;
	if (Summary.LastRunTime != FDateTime())
	{
		const FTimespan Elapsed = FDateTime::UtcNow() - Summary.LastRunTime;
		if (Elapsed.GetTotalSeconds() < 60)
		{
			PerSystemTimeStr = FString::Printf(TEXT("%ds ago"), (int32)Elapsed.GetTotalSeconds());
		}
		else if (Elapsed.GetTotalMinutes() < 60)
		{
			PerSystemTimeStr = FString::Printf(TEXT("%dm ago"), (int32)Elapsed.GetTotalMinutes());
		}
		else
		{
			PerSystemTimeStr = FString::Printf(TEXT("%dh ago"), (int32)Elapsed.GetTotalHours());
		}
	}

	// EN: Capture system name for lambda / ES: Capturar nombre de sistema para lambda
	const FString SysName = Summary.SystemName;

	TSharedRef<SVerticalBox> SectionBox = SNew(SVerticalBox);

	// EN: System section divider with Run button / ES: Divisor de seccion de sistema con boton Run
	SectionBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 2.0f)
	[
		SNew(SPGXSectionDivider)
		.Title(FText::FromString(HeaderStr))
		.AccentColor(SystemColor)
		.RightContent()
		[
			SNew(SHorizontalBox)

			// EN: Per-system relative time / ES: Tiempo relativo por sistema
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(PerSystemTimeStr))
				.Font(PGX::Font::Hint())
				.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
				.Visibility(PerSystemTimeStr.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible)
			]

			// EN: Per-system Run button / ES: Boton Run por sistema
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("RunSystem", "Run"))
				.ToolTipText(FText::Format(LOCTEXT("RunSystemTip", "Run tests for {0} only"), FText::FromString(SysName)))
				.IsEnabled_Lambda([this]() { return bIsPIEActive; })
				.OnClicked_Lambda([this, SysName]()
				{
					OnRunSystemClicked(SysName);
					return FReply::Handled();
				})
			]
		]
	];

	// EN: Individual results (filtered) / ES: Resultados individuales (filtrados)
	for (const FPGXTestResult& Result : Summary.Results)
	{
		// EN: Filter individual results by fail-only / ES: Filtrar resultados individuales por solo fallidos
		if (bShowFailedOnly && Result.bPassed)
		{
			continue;
		}

		// EN: Filter individual results by text / ES: Filtrar resultados individuales por texto
		if (!FilterText.IsEmpty() && !Result.TestName.Contains(FilterText, ESearchCase::IgnoreCase))
		{
			continue;
		}

		SectionBox->AddSlot()
		.AutoHeight()
		.Padding(12.0f, 1.0f, 0.0f, 1.0f)
		[
			BuildResultRow(Result)
		];
	}

	return SectionBox;
}

TSharedRef<SWidget> SPGXTestDashboardTab::BuildResultRow(const FPGXTestResult& Result)
{
	const FLinearColor BadgeColor = Result.bPassed
		? PGX::Semantic::Good
		: PGX::Semantic::Error;
	const FText BadgeText = Result.bPassed ? LOCTEXT("Pass", "PASS") : LOCTEXT("Fail", "FAIL");

	TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);

	// EN: PASS/FAIL badge / ES: Badge PASS/FAIL
	Row->AddSlot()
	.AutoWidth()
	.VAlign(VAlign_Center)
	.Padding(0.0f, 0.0f, 6.0f, 0.0f)
	[
		SNew(SPGXStatusBadge)
		.Shape(EPGXBadgeShape::Pill)
		.Color(BadgeColor)
		.Label(BadgeText)
	];

	// EN: Test name / ES: Nombre del test
	Row->AddSlot()
	.FillWidth(1.0f)
	.VAlign(VAlign_Center)
	[
		SNew(STextBlock)
		.Text(FText::FromString(Result.TestName))
		.Font(PGX::Font::BodySmall())
	];

	// EN: Duration / ES: Duracion
	Row->AddSlot()
	.AutoWidth()
	.VAlign(VAlign_Center)
	.Padding(8.0f, 0.0f)
	[
		SNew(STextBlock)
		.Text(FText::FromString(FString::Printf(TEXT("%.1f ms"), Result.DurationMs)))
		.Font(PGX::Font::Caption())
		.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
	];

	// EN: Issues (if any, show count) / ES: Issues (si hay, mostrar conteo)
	if (Result.Issues.Num() > 0 && !Result.bPassed)
	{
		// EN: Show issues as tooltip / ES: Mostrar issues como tooltip
		FString IssueText;
		for (const FString& Issue : Result.Issues)
		{
			if (!IssueText.IsEmpty()) IssueText += TEXT("\n");
			IssueText += Issue;
		}

		Row->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("[%d issues]"), Result.Issues.Num())))
			.Font(PGX::Font::Caption())
			.ColorAndOpacity(FSlateColor(PGX::Semantic::Warn))
			.ToolTipText(FText::FromString(IssueText))
		];
	}

	return Row;
}

// ============================================================================
// EN: Actions
// ES: Acciones
// ============================================================================

FReply SPGXTestDashboardTab::OnRunAllClicked()
{
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[TestDashboard] Run All Tests clicked (Harness=%s)"), bHarnessMode ? TEXT("ON") : TEXT("OFF"));

	UWorld* PIEWorld = GetPIEWorld();
	if (!PIEWorld)
	{
		PGX_LOG_WARNING(LogPGXEditorTools, TEXT("[TestDashboard] No PIE world — cannot run tests"));
		return FReply::Handled();
	}

	// EN: Setup harness if enabled — injects transient test data into subsystem caches
	// ES: Setup del harness si esta habilitado — inyecta datos transient en caches de subsistemas
	if (bHarnessMode)
	{
		TestHarness.Setup(PIEWorld);
	}

	FPGXTestResultCollector& Collector = FPGXTestResultCollector::Get();
	Collector.ClearAll();

	// EN: Run each system's tests / ES: Ejecutar tests de cada sistema
	for (const FSystemTestDef& Def : GSystemDefs)
	{
		TArray<FString> Issues;
		bool bPassed = false;
		const double StartTime = FPlatformTime::Seconds();

		// EN: Dispatch to the correct TestUtility / ES: Despachar al TestUtility correcto
		if (Def.Name == TEXT("Profile"))         bPassed = UPGXProfileTestUtility::RunAllTests(PIEWorld, Issues);
		else if (Def.Name == TEXT("GameFlow"))   bPassed = UPGXGameFlowTestUtility::RunAllTests(PIEWorld, Issues);
		else if (Def.Name == TEXT("Log"))         bPassed = UPGXLogTestUtility::RunAllTests(PIEWorld, Issues);
		else if (Def.Name == TEXT("Save"))        bPassed = UPGXSaveTestUtility::RunAllTests(PIEWorld, Issues);
		else if (Def.Name == TEXT("PSO"))         bPassed = UPGXPSOTestUtility::RunAllTests(PIEWorld, Issues);
		else if (Def.Name == TEXT("LevelFlow"))   bPassed = UPGXLevelFlowTestUtility::RunAllTests(PIEWorld, Issues);
		else if (Def.Name == TEXT("Loading"))     bPassed = UPGXLoadingTestUtility::RunAllTests(PIEWorld, Issues);
		else if (Def.Name == TEXT("MGOS"))        bPassed = UPGXMGOSTestUtility::RunAllTests(Issues);
		else if (Def.Name == TEXT("Audio"))       bPassed = UPGXAudioTestUtility::RunAllTests(PIEWorld, Issues);
		else if (Def.Name == TEXT("Registry"))    bPassed = UPGXRegistryTestUtility::RunAllTests(PIEWorld, Issues);
		else if (Def.Name == TEXT("Construction")) bPassed = UPGXConstructionTestUtility::RunAllTests(PIEWorld, Issues);
		else if (Def.Name == TEXT("Message"))      bPassed = UPGXMessageTestUtility::RunAllTests(PIEWorld, Issues);
		else if (Def.Name == TEXT("EventHandler"))  bPassed = UPGXEventHandlerTestUtility::RunAllTests(PIEWorld, Issues);

		const double Duration = (FPlatformTime::Seconds() - StartTime) * 1000.0;

		FPGXTestResult TestResult;
		TestResult.SystemName = Def.Name;
		TestResult.SystemVersion = Def.Version;
		TestResult.TestName = TestHarness.IsActive() ? TEXT("RunAllTests [HARNESS]") : TEXT("RunAllTests");
		TestResult.bPassed = bPassed;
		TestResult.Issues = MoveTemp(Issues);
		TestResult.Timestamp = FDateTime::UtcNow();
		TestResult.DurationMs = Duration;

		Collector.RecordResult(TestResult);
	}

	// EN: Teardown harness — remove injected data from caches / ES: Teardown del harness — remover datos inyectados de caches
	if (bHarnessMode)
	{
		TestHarness.Teardown();
	}

	LastRunTimestamp = FDateTime::UtcNow();
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[TestDashboard] Run All completed — %d systems tested"), GSystemDefs.Num());

	return FReply::Handled();
}

// ============================================================================
// EN: Per-System Run
// ES: Ejecutar tests de un sistema individual
// ============================================================================

void SPGXTestDashboardTab::OnRunSystemClicked(const FString& SystemName)
{
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[TestDashboard] Run System clicked: %s (Harness=%s)"), *SystemName, bHarnessMode ? TEXT("ON") : TEXT("OFF"));

	UWorld* PIEWorld = GetPIEWorld();
	if (!PIEWorld)
	{
		PGX_LOG_WARNING(LogPGXEditorTools, TEXT("[TestDashboard] No PIE world — cannot run tests for %s"), *SystemName);
		return;
	}

	if (bHarnessMode)
	{
		TestHarness.Setup(PIEWorld);
	}

	FPGXTestResultCollector& Collector = FPGXTestResultCollector::Get();
	Collector.ClearSystem(SystemName);

	// EN: Find the system def and dispatch / ES: Encontrar la def del sistema y despachar
	for (const FSystemTestDef& Def : GSystemDefs)
	{
		if (Def.Name != SystemName)
		{
			continue;
		}

		TArray<FString> Issues;
		bool bPassed = false;
		const double StartTime = FPlatformTime::Seconds();

		if (Def.Name == TEXT("Profile"))          bPassed = UPGXProfileTestUtility::RunAllTests(PIEWorld, Issues);
		else if (Def.Name == TEXT("GameFlow"))    bPassed = UPGXGameFlowTestUtility::RunAllTests(PIEWorld, Issues);
		else if (Def.Name == TEXT("Log"))          bPassed = UPGXLogTestUtility::RunAllTests(PIEWorld, Issues);
		else if (Def.Name == TEXT("Save"))         bPassed = UPGXSaveTestUtility::RunAllTests(PIEWorld, Issues);
		else if (Def.Name == TEXT("PSO"))          bPassed = UPGXPSOTestUtility::RunAllTests(PIEWorld, Issues);
		else if (Def.Name == TEXT("LevelFlow"))    bPassed = UPGXLevelFlowTestUtility::RunAllTests(PIEWorld, Issues);
		else if (Def.Name == TEXT("Loading"))      bPassed = UPGXLoadingTestUtility::RunAllTests(PIEWorld, Issues);
		else if (Def.Name == TEXT("MGOS"))         bPassed = UPGXMGOSTestUtility::RunAllTests(Issues);
		else if (Def.Name == TEXT("Audio"))        bPassed = UPGXAudioTestUtility::RunAllTests(PIEWorld, Issues);
		else if (Def.Name == TEXT("Registry"))     bPassed = UPGXRegistryTestUtility::RunAllTests(PIEWorld, Issues);
		else if (Def.Name == TEXT("Construction")) bPassed = UPGXConstructionTestUtility::RunAllTests(PIEWorld, Issues);
		else if (Def.Name == TEXT("Message"))       bPassed = UPGXMessageTestUtility::RunAllTests(PIEWorld, Issues);
		else if (Def.Name == TEXT("EventHandler"))   bPassed = UPGXEventHandlerTestUtility::RunAllTests(PIEWorld, Issues);

		const double Duration = (FPlatformTime::Seconds() - StartTime) * 1000.0;

		FPGXTestResult TestResult;
		TestResult.SystemName = Def.Name;
		TestResult.SystemVersion = Def.Version;
		TestResult.TestName = TestHarness.IsActive() ? TEXT("RunAllTests [HARNESS]") : TEXT("RunAllTests");
		TestResult.bPassed = bPassed;
		TestResult.Issues = MoveTemp(Issues);
		TestResult.Timestamp = FDateTime::UtcNow();
		TestResult.DurationMs = Duration;

		Collector.RecordResult(TestResult);

		PGX_LOG_INFO(LogPGXEditorTools, TEXT("[TestDashboard] %s: %s (%.1f ms, %d issues)"),
			*SystemName, bPassed ? TEXT("PASS") : TEXT("FAIL"), Duration, TestResult.Issues.Num());

		break;
	}

	if (bHarnessMode)
	{
		TestHarness.Teardown();
	}

	LastRunTimestamp = FDateTime::UtcNow();
}

FReply SPGXTestDashboardTab::OnClearClicked()
{
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[TestDashboard] Clear all results"));
	FPGXTestResultCollector::Get().ClearAll();
	LastRunTimestamp = FDateTime();
	return FReply::Handled();
}

FReply SPGXTestDashboardTab::OnExportJsonClicked()
{
	const FString JsonString = FPGXTestResultCollector::Get().ExportToJson();

	// EN: Save to Saved/PGXTests/ / ES: Guardar en Saved/PGXTests/
	const FString Dir = FPaths::ProjectSavedDir() / TEXT("PGXTests");
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.CreateDirectoryTree(*Dir);

	const FString Filename = FString::Printf(TEXT("PGXTestReport_%s.json"),
		*FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
	const FString FullPath = Dir / Filename;

	if (FFileHelper::SaveStringToFile(JsonString, *FullPath))
	{
		PGX_LOG_INFO(LogPGXEditorTools, TEXT("[PGX Test Dashboard] Report exported to: %s"), *FullPath);
	}
	else
	{
		PGX_LOG_ERROR(LogPGXEditorTools, TEXT("[PGX Test Dashboard] Failed to export report to: %s"), *FullPath);
	}

	return FReply::Handled();
}

FReply SPGXTestDashboardTab::OnExportMarkdownClicked()
{
	const FString MdString = FPGXTestResultCollector::Get().ExportToMarkdown(ReportLanguage);

	const FString Dir = FPaths::ProjectSavedDir() / TEXT("PGXTests");
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.CreateDirectoryTree(*Dir);

	const FString LangSuffix = (ReportLanguage == EPGXReportLanguage::English) ? TEXT("EN") : TEXT("ES");
	const FString Filename = FString::Printf(TEXT("PGXTestReport_%s_%s.md"),
		*FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")), *LangSuffix);
	const FString FullPath = Dir / Filename;

	if (FFileHelper::SaveStringToFile(MdString, *FullPath))
	{
		PGX_LOG_INFO(LogPGXEditorTools, TEXT("[PGX Test Dashboard] Markdown report exported to: %s"), *FullPath);
	}
	else
	{
		PGX_LOG_ERROR(LogPGXEditorTools, TEXT("[PGX Test Dashboard] Failed to export Markdown report to: %s"), *FullPath);
	}

	return FReply::Handled();
}

FReply SPGXTestDashboardTab::OnToggleLanguageClicked()
{
	ReportLanguage = (ReportLanguage == EPGXReportLanguage::English)
		? EPGXReportLanguage::Spanish
		: EPGXReportLanguage::English;
	return FReply::Handled();
}

// ============================================================================
// EN: Callbacks
// ES: Callbacks
// ============================================================================

void SPGXTestDashboardTab::OnResultsChanged()
{
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[TestDashboard] Results changed — refreshing"));
	RefreshResults();
}

void SPGXTestDashboardTab::OnPIEStarted(bool bIsSimulating)
{
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[TestDashboard] PIE started (simulating=%s)"), bIsSimulating ? TEXT("true") : TEXT("false"));
	bIsPIEActive = true;

	if (StatusText.IsValid())
	{
		StatusText->SetText(FText::FromString(TEXT("LIVE")));
		StatusText->SetColorAndOpacity(FSlateColor(PGX::Semantic::Good));
	}
}

void SPGXTestDashboardTab::OnPIEEnded(bool /*bIsSimulating*/)
{
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[TestDashboard] PIE ended"));

	// EN: Safety teardown — ensure no residual harness data when PIE ends
	// ES: Teardown de seguridad — asegurar que no queden datos del harness cuando PIE termina
	if (TestHarness.IsActive())
	{
		TestHarness.Teardown();
		PGX_LOG_INFO(LogPGXEditorTools, TEXT("[TestDashboard] Harness teardown on PIE end"));
	}

	bIsPIEActive = false;

	if (StatusText.IsValid())
	{
		StatusText->SetText(FText::FromString(TEXT("OFFLINE")));
		StatusText->SetColorAndOpacity(FSlateColor(PGX::Text::Muted));
	}
}

void SPGXTestDashboardTab::RefreshResults()
{
	if (!ResultsBox.IsValid())
	{
		return;
	}

	ResultsBox->ClearChildren();

	const TArray<FPGXSystemTestSummary> Summaries = FPGXTestResultCollector::Get().GetSystemSummaries();

	if (Summaries.Num() == 0)
	{
		ResultsBox->AddSlot()
		.AutoHeight()
		.Padding(8.0f, 16.0f)
		[
			SNew(SPGXEmptyStateV2)
			.Message(LOCTEXT("NoResults", "No tests run yet"))
			.Hint(LOCTEXT("NoResultsHint", "Start PIE and click \"Run All Tests\""))
		];
	}
	else
	{
		int32 VisibleSystems = 0;
		for (const FPGXSystemTestSummary& Summary : Summaries)
		{
			// EN: Filter by text (system name match) / ES: Filtrar por texto (nombre del sistema)
			bool bSystemNameMatch = FilterText.IsEmpty() || Summary.SystemName.Contains(FilterText, ESearchCase::IgnoreCase);

			// EN: Also check individual test names / ES: Tambien verificar nombres de tests individuales
			bool bAnyTestMatch = false;
			if (!bSystemNameMatch && !FilterText.IsEmpty())
			{
				for (const FPGXTestResult& R : Summary.Results)
				{
					if (R.TestName.Contains(FilterText, ESearchCase::IgnoreCase))
					{
						bAnyTestMatch = true;
						break;
					}
				}
			}

			if (!bSystemNameMatch && !bAnyTestMatch)
			{
				continue;
			}

			// EN: Filter by fail-only at system level / ES: Filtrar por solo fallidos a nivel de sistema
			if (bShowFailedOnly && Summary.FailedCount == 0)
			{
				continue;
			}

			ResultsBox->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 4.0f)
			[
				BuildSystemSection(Summary)
			];

			VisibleSystems++;
		}

		// EN: Show filter empty state / ES: Mostrar estado vacio de filtro
		if (VisibleSystems == 0)
		{
			ResultsBox->AddSlot()
			.AutoHeight()
			.Padding(8.0f, 16.0f)
			[
				SNew(SPGXEmptyStateV2)
				.Message(LOCTEXT("NoFilterMatch", "No results match the current filter"))
				.Hint(LOCTEXT("NoFilterHintMsg", "Try adjusting your search or disabling \"Failures only\""))
			];
		}
	}

	// EN: Update summary / ES: Actualizar resumen
	const int32 Total = FPGXTestResultCollector::Get().GetTotalTests();
	const int32 Passed = FPGXTestResultCollector::Get().GetTotalPassed();
	const int32 Failed = FPGXTestResultCollector::Get().GetTotalFailed();
	const float Progress = Total > 0 ? static_cast<float>(Passed) / static_cast<float>(Total) : 0.0f;

	// EN: Update passed KPI chip / ES: Actualizar KPI chip de aprobados
	CachedPassedValue = FText::FromString(Total > 0
		? FString::Printf(TEXT("%d/%d"), Passed, Total)
		: TEXT("--/--"));

	if (SummaryProgress.IsValid())
	{
		SummaryProgress->SetPercent(TOptional<float>(Progress));
	}

	// EN: Update failed count KPI chip / ES: Actualizar KPI chip de fallidos
	if (FailedChip.IsValid())
	{
		if (Failed > 0)
		{
			CachedFailedValue = FText::FromString(FString::Printf(TEXT("%d"), Failed));
			FailedChip->SetVisibility(EVisibility::Visible);
		}
		else
		{
			FailedChip->SetVisibility(EVisibility::Collapsed);
		}
	}

	// EN: Update footer / ES: Actualizar footer
	CachedFooterText = FText::FromString(FString::Printf(TEXT("%d systems | %d tests"),
		GSystemDefs.Num(), Total));

	// EN: Update last run timestamp / ES: Actualizar timestamp de ultima ejecucion
	if (LastRunText.IsValid() && LastRunTimestamp != FDateTime())
	{
		const FTimespan Elapsed = FDateTime::UtcNow() - LastRunTimestamp;
		FString TimeStr;
		if (Elapsed.GetTotalSeconds() < 60)
		{
			TimeStr = FString::Printf(TEXT("Ran %ds ago"), (int32)Elapsed.GetTotalSeconds());
		}
		else if (Elapsed.GetTotalMinutes() < 60)
		{
			TimeStr = FString::Printf(TEXT("Ran %dm ago"), (int32)Elapsed.GetTotalMinutes());
		}
		else
		{
			TimeStr = FString::Printf(TEXT("Ran %dh ago"), (int32)Elapsed.GetTotalHours());
		}
		LastRunText->SetText(FText::FromString(TimeStr));
		LastRunText->SetToolTipText(FText::FromString(LastRunTimestamp.ToString()));
	}
	else if (LastRunText.IsValid())
	{
		LastRunText->SetText(LOCTEXT("NeverRun", "Never run"));
	}
}

// ============================================================================
// EN: Helpers
// ES: Helpers
// ============================================================================

UWorld* SPGXTestDashboardTab::GetPIEWorld() const
{
	if (!GEngine)
	{
		return nullptr;
	}

	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::PIE && Context.World())
		{
			return Context.World();
		}
	}
	return nullptr;
}

FLinearColor SPGXTestDashboardTab::GetSystemColor(const FString& SystemName)
{
	// EN: Colors from PGXVisualTokens SSOT / ES: Colores desde PGXVisualTokens SSOT
	if (SystemName == TEXT("Save"))          return PGX::System::Save;
	if (SystemName == TEXT("GameFlow"))      return PGX::System::GameFlow;
	if (SystemName == TEXT("PSO"))           return PGX::System::PSO;
	if (SystemName == TEXT("LevelFlow"))     return PGX::System::LevelFlow;
	if (SystemName == TEXT("Loading"))       return PGX::System::Loading;
	if (SystemName == TEXT("Profile"))       return PGX::System::Profile;
	if (SystemName == TEXT("MGOS"))          return PGX::System::MGOS;
	if (SystemName == TEXT("Audio"))         return PGX::System::Audio;
	if (SystemName == TEXT("Registry"))      return PGX::System::DataRegistry;
	if (SystemName == TEXT("Construction"))  return PGX::System::Construction;
	if (SystemName == TEXT("Log"))           return PGX::System::Log;
	if (SystemName == TEXT("Message"))       return PGX::System::Message;
	if (SystemName == TEXT("EventHandler"))  return PGX::System::EventHandler;

	return PGX::Semantic::Neutral;
}

#undef LOCTEXT_NAMESPACE
