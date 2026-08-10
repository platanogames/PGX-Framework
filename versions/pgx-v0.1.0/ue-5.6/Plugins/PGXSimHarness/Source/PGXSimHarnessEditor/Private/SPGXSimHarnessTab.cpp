// Copyright PGX Framework. All Rights Reserved.

#include "SPGXSimHarnessTab.h"
#include "PGXDemoRegistry.h"
#include "Logging/PGXLogMacros.h"
#include "PGXSimHarnessCBExtension.h"
#include "FPGXVisualHarness.h"
#include "PGXSimHarnessEditorModule.h"

#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "Framework/Docking/TabManager.h"
#include "Utils/PGXEditorUtils.h"
#include "Style/PGXVisualTokens.h"
#include "Style/PGXEditorStyle.h"
#include "Widgets/SPGXPremiumShell.h"
#include "Widgets/SPGXSectionDivider.h"
#include "Widgets/SPGXFooterBar.h"
#include "Widgets/SPGXKPIChip.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SWrapBox.h"

#define LOCTEXT_NAMESPACE "PGXSimHarnessTab"

// EN: PSPH system color uses the shared PGX::System::Simulation token.
// ES: El color del sistema PSPH usa el token compartido PGX::System::Simulation.

// ============================================================================
// EN: Construct / Destruct
// ES: Construir / Destruir
// ============================================================================

void SPGXSimHarnessTab::Construct(const FArguments& /*InArgs*/)
{
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("[PSPH] Construct"));
	Harness = MakeShared<FPGXVisualHarness>();

	ChildSlot
	[
		SNew(SPGXPremiumShell)
		.SystemColor(PGX::System::Simulation)
		.Title(LOCTEXT("PanelTitle", "PGX VERIFICATION HARNESS"))
		.Subtitle(LOCTEXT("PanelSubtitle", "Demonstrates implemented systems and reports partial or missing coverage"))
		.Icon(FPGXEditorStyle::Get().GetBrush("PGXEditor.Icon.SimHarness"))
		.bShowFooter(true)
		.FooterLeftContent()
		[
			SAssignNew(FooterText, STextBlock)
			.Text(LOCTEXT("FooterReady", "Ready — Select DEMO or HARNESS mode"))
			.Font(PGX::Font::Hint())
			.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
		]
		.Content()
		[
			SNew(SVerticalBox)

			// EN: Mode toggle / ES: Toggle de modo
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(8.0f, 4.0f)
			[
				BuildModeToggle()
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSeparator)
			]

			// EN: Mode panels / ES: Paneles de modo
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SAssignNew(ModeSwitcher, SWidgetSwitcher)
				.WidgetIndex(0)

				+ SWidgetSwitcher::Slot()
				[
					BuildDemoPanel()
				]

				+ SWidgetSwitcher::Slot()
				[
					BuildHarnessPanel()
				]
			]
		]
	];
}

SPGXSimHarnessTab::~SPGXSimHarnessTab()
{
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("[PSPH] Destructor — cleanup"));
	// EN: Remove UI refresh ticker (S1 compliance) / ES: Remover ticker de refresco UI (cumple S1)
	if (UIRefreshTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(UIRefreshTickerHandle);
		UIRefreshTickerHandle.Reset();
	}

	// EN: Teardown harness if active / ES: Teardown del harness si esta activo
	if (Harness.IsValid())
	{
		if (Harness->IsSimulating())
		{
			Harness->StopSimulation();
		}
		if (Harness->IsActive())
		{
			Harness->Teardown();
		}
	}
}

// ============================================================================
// EN: Mode Toggle
// ES: Toggle de modo
// ============================================================================

TSharedRef<SWidget> SPGXSimHarnessTab::BuildModeToggle()
{
	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.0f, 0.0f, 4.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("DemoMode", "DEMO"))
			.ToolTipText(LOCTEXT("DemoModeTip", "Create pre-configured DataAssets with educational values"))
			.OnClicked_Lambda([this]()
			{
				SetMode(true);
				return FReply::Handled();
			})
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("HarnessMode", "HARNESS"))
			.ToolTipText(LOCTEXT("HarnessModeTip", "Inject rich data into subsystems and verify inspector panels"))
			.OnClicked_Lambda([this]()
			{
				SetMode(false);
				return FReply::Handled();
			})
		];
}

void SPGXSimHarnessTab::SetMode(bool bNewDemoMode)
{
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("[PSPH] SetMode — %s"), bNewDemoMode ? TEXT("DEMO") : TEXT("HARNESS"));
	bIsDemoMode = bNewDemoMode;
	if (ModeSwitcher.IsValid())
	{
		ModeSwitcher->SetActiveWidgetIndex(bIsDemoMode ? 0 : 1);
	}
}

// ============================================================================
// EN: DEMO Panel (unchanged from v0.1)
// ES: Panel DEMO (sin cambios de v0.1)
// ============================================================================

TSharedRef<SWidget> SPGXSimHarnessTab::BuildDemoPanel()
{
	TSharedRef<SScrollBox> ScrollBox = SNew(SScrollBox);

	TArray<FString> Categories = FPGXDemoRegistry::GetDemoCategories();

	for (const FString& Category : Categories)
	{
		ScrollBox->AddSlot()
		.Padding(8.0f, 8.0f, 8.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Category))
			.Font(PGX::Font::SectionHeader())
			.ColorAndOpacity(FSlateColor(PGX::System::Simulation))
		];

		TArray<FPGXDemoEntry> Entries = FPGXDemoRegistry::GetEntriesForCategory(Category);
		for (const FPGXDemoEntry& Entry : Entries)
		{
			FPGXDemoEntry CapturedEntry = Entry;

			ScrollBox->AddSlot()
			.Padding(16.0f, 2.0f, 8.0f, 2.0f)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(FText::FromString(Entry.DisplayName))
						.Font(PGX::Font::Body())
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(FText::FromString(Entry.Tooltip))
						.Font(PGX::Font::Caption())
						.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
					]
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("Create", "Create"))
					.ToolTipText(FText::Format(LOCTEXT("CreateTip", "Create {0} with demo values"), FText::FromString(Entry.DisplayName)))
					.OnClicked_Lambda([CapturedEntry]()
					{
						FPGXSimHarnessCBExtension::CreateDemoAsset(CapturedEntry);
						return FReply::Handled();
					})
				]
			];
		}
	}

	return ScrollBox;
}

// ============================================================================
// EN: HARNESS Panel — Full implementation with 5 sections
// ES: Panel HARNESS — Implementacion completa con 5 secciones
// ============================================================================

TSharedRef<SWidget> SPGXSimHarnessTab::BuildHarnessPanel()
{
	return SNew(SScrollBox)

		// EN: Control Bar / ES: Barra de control
		+ SScrollBox::Slot()
		.Padding(8.0f)
		[
			BuildControlBar()
		]

		+ SScrollBox::Slot()
		.Padding(8.0f, 2.0f)
		[
			SNew(SSeparator)
		]

		// EN: Status Dashboard / ES: Dashboard de estado
		+ SScrollBox::Slot()
		.Padding(8.0f, 4.0f)
		[
			BuildStatusDashboard()
		]

		+ SScrollBox::Slot()
		.Padding(8.0f, 2.0f)
		[
			SNew(SSeparator)
		]

		// EN: System Grid / ES: Grid de sistemas
		+ SScrollBox::Slot()
		.Padding(8.0f, 4.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SPGXSectionDivider)
				.Title(LOCTEXT("SecSystems", "SYSTEMS"))
				.AccentColor(PGX::System::Simulation)
			]

			// EN: Column headers / ES: Cabeceras de columnas
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 2.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SBox).WidthOverride(110.0f)
					[ SNew(STextBlock).Text(LOCTEXT("ColSystem", "System")).Font(PGX::Font::CaptionBold()).ColorAndOpacity(FSlateColor(PGX::Text::Muted)) ]
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SBox).WidthOverride(50.0f)
					[ SNew(STextBlock).Text(LOCTEXT("ColStatus2", "Status")).Font(PGX::Font::CaptionBold()).ColorAndOpacity(FSlateColor(PGX::Text::Muted)) ]
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SBox).WidthOverride(40.0f)
					[ SNew(STextBlock).Text(LOCTEXT("ColObjs", "Objs")).Font(PGX::Font::CaptionBold()).ColorAndOpacity(FSlateColor(PGX::Text::Muted)).Justification(ETextJustify::Right) ]
				]
				+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(8.0f, 0.0f, 0.0f, 0.0f)
				[ SNew(STextBlock).Text(LOCTEXT("ColDetail", "Detail")).Font(PGX::Font::CaptionBold()).ColorAndOpacity(FSlateColor(PGX::Text::Muted)) ]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				BuildSystemGrid()
			]
		]

		// EN: Coverage Matrix / ES: Matriz de cobertura
		+ SScrollBox::Slot()
		.Padding(8.0f, 2.0f)
		[
			BuildCoverageMatrix()
		]

		+ SScrollBox::Slot()
		.Padding(8.0f, 2.0f)
		[
			SNew(SSeparator)
		]

		// EN: Quick Actions / ES: Acciones rapidas
		+ SScrollBox::Slot()
		.Padding(8.0f, 4.0f)
		[
			BuildQuickActions()
		]

		+ SScrollBox::Slot()
		.Padding(8.0f, 2.0f)
		[
			SNew(SSeparator)
		]

		// EN: Panel Launcher / ES: Lanzador de paneles
		+ SScrollBox::Slot()
		.Padding(8.0f, 4.0f)
		[
			BuildPanelLauncher()
		];
}

// ============================================================================
// EN: Control Bar — Start/Stop Harness, Start/Stop Sim, Open All
// ES: Barra de control — Iniciar/Parar Harness, Iniciar/Parar Sim, Abrir Todos
// ============================================================================

TSharedRef<SWidget> SPGXSimHarnessTab::BuildControlBar()
{
	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.0f, 0.0f, 4.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("StartHarness", "Start Harness"))
			.ToolTipText(LOCTEXT("StartHarnessTip", "Inject rich data into 13 subsystems"))
			.IsEnabled_Lambda([this]() { return Harness.IsValid() && !Harness->IsActive(); })
			.OnClicked_Lambda([this]()
			{
				if (Harness.IsValid())
				{
					Harness->Setup(nullptr);
					RefreshHarnessUI();

					// EN: Start UI refresh ticker / ES: Iniciar ticker de refresco UI
					if (!UIRefreshTickerHandle.IsValid())
					{
						UIRefreshTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
							FTickerDelegate::CreateRaw(this, &SPGXSimHarnessTab::OnUIRefreshTick),
							1.0f
						);
					}
				}
				return FReply::Handled();
			})
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.0f, 0.0f, 8.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("StopHarness", "Stop Harness"))
			.ToolTipText(LOCTEXT("StopHarnessTip", "Clean up all injected data"))
			.IsEnabled_Lambda([this]() { return Harness.IsValid() && Harness->IsActive(); })
			.OnClicked_Lambda([this]()
			{
				if (Harness.IsValid())
				{
					Harness->Teardown();

					// EN: Stop UI refresh ticker / ES: Parar ticker de refresco UI
					if (UIRefreshTickerHandle.IsValid())
					{
						FTSTicker::GetCoreTicker().RemoveTicker(UIRefreshTickerHandle);
						UIRefreshTickerHandle.Reset();
					}
					RefreshHarnessUI();
				}
				return FReply::Handled();
			})
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.0f, 0.0f, 4.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("StartSim", "Start Sim"))
			.ToolTipText(LOCTEXT("StartSimTip", "Start continuous simulation (logs, messages, transitions)"))
			.IsEnabled_Lambda([this]() { return Harness.IsValid() && Harness->IsActive() && !Harness->IsSimulating(); })
			.OnClicked_Lambda([this]()
			{
				if (Harness.IsValid())
				{
					Harness->StartSimulation();
					RefreshHarnessUI();
				}
				return FReply::Handled();
			})
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.0f, 0.0f, 8.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("StopSim", "Stop Sim"))
			.ToolTipText(LOCTEXT("StopSimTip", "Stop simulation ticker"))
			.IsEnabled_Lambda([this]() { return Harness.IsValid() && Harness->IsSimulating(); })
			.OnClicked_Lambda([this]()
			{
				if (Harness.IsValid())
				{
					Harness->StopSimulation();
					RefreshHarnessUI();
				}
				return FReply::Handled();
			})
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.0f, 0.0f, 4.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("OpenAll", "Open All Panels"))
			.ToolTipText(LOCTEXT("OpenAllTip", "Open all 19+ registered PGX NomadTabs"))
			.OnClicked_Lambda([this]()
			{
				if (Harness.IsValid())
				{
					Harness->OpenAllPanels();
				}
				return FReply::Handled();
			})
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("ExportReport", "Export Report"))
			.ToolTipText(LOCTEXT("ExportReportTip", "Export execution log as .md to Saved/PGX/"))
			.IsEnabled_Lambda([this]() { return Harness.IsValid() && Harness->IsActive(); })
			.OnClicked_Lambda([this]()
			{
				if (Harness.IsValid())
				{
					FString Path = Harness->ExportReport();
					PGX_LOG_INFO(LogPGXSimHarness, TEXT("Report exported to: %s"), *Path);
				}
				return FReply::Handled();
			})
		];
}

// ============================================================================
// EN: Status Dashboard — Active/Simulating/Elapsed/Objects
// ES: Dashboard de estado — Activo/Simulando/Transcurrido/Objetos
// ============================================================================

TSharedRef<SWidget> SPGXSimHarnessTab::BuildStatusDashboard()
{
	return SNew(SHorizontalBox)

		// EN: Status KPI / ES: KPI de estado
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 16, 0).VAlign(VAlign_Center)
		[
			SNew(SPGXKPIChip)
			.Label(LOCTEXT("LblStatus", "STATUS"))
			.AccentColor(PGX::System::Simulation)
			.ValueWidget()
			[
				SAssignNew(StatusKPI, STextBlock)
				.Text(LOCTEXT("StatusVal", "Inactive"))
				.Font(PGX::Font::SectionHeader())
				.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
			]
		]

		// EN: Simulation KPI / ES: KPI de simulacion
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 16, 0).VAlign(VAlign_Center)
		[
			SNew(SPGXKPIChip)
			.Label(LOCTEXT("LblSim", "SIMULATION"))
			.AccentColor(PGX::System::Simulation)
			.ValueWidget()
			[
				SAssignNew(SimKPI, STextBlock)
				.Text(LOCTEXT("SimVal", "Off"))
				.Font(PGX::Font::SectionHeader())
			]
		]

		// EN: Elapsed KPI / ES: KPI de tiempo transcurrido
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 16, 0).VAlign(VAlign_Center)
		[
			SNew(SPGXKPIChip)
			.Label(LOCTEXT("LblElapsed", "ELAPSED"))
			.AccentColor(PGX::System::Simulation)
			.ValueWidget()
			[
				SAssignNew(ElapsedKPI, STextBlock)
				.Text(LOCTEXT("ElapsedVal", "00:00"))
				.Font(PGX::Font::Mono())
			]
		]

		// EN: Objects KPI / ES: KPI de objetos
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			SNew(SPGXKPIChip)
			.Label(LOCTEXT("LblObjects", "OBJECTS"))
			.AccentColor(PGX::System::Simulation)
			.ValueWidget()
			[
				SAssignNew(ObjectsKPI, STextBlock)
				.Text(LOCTEXT("ObjectsVal", "0"))
				.Font(PGX::Font::SectionHeader())
				.ColorAndOpacity(FSlateColor(PGX::System::Simulation))
			]
		];
}

// ============================================================================
// EN: System Grid — 13 rows with name, status, objects, detail
// ES: Grid de sistemas — 13 filas con nombre, estado, objetos, detalle
// ============================================================================

TSharedRef<SWidget> SPGXSimHarnessTab::BuildSystemGrid()
{
	SAssignNew(SystemGridBox, SVerticalBox);

	SystemGridBox->AddSlot()
	.AutoHeight()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("GridPlaceholder", "Start Harness to see system status"))
		.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
	];

	return SystemGridBox.ToSharedRef();
}

// ============================================================================
// EN: Quick Actions — Log, GC, Broadcast, Cycle Flow
// ES: Acciones rapidas — Log, GC, Broadcast, Ciclar Flow
// ============================================================================

TSharedRef<SWidget> SPGXSimHarnessTab::BuildQuickActions()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("SecQuickActions", "QUICK ACTIONS"))
			.AccentColor(PGX::System::Simulation)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SWrapBox)
			.UseAllottedSize(true)

			+ SWrapBox::Slot().Padding(0.0f, 0.0f, 4.0f, 4.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("AddLogs", "+ Log Entries"))
				.ToolTipText(LOCTEXT("AddLogsTip", "Generate ~35 log entries across all systems"))
				.IsEnabled_Lambda([this]() { return Harness.IsValid() && Harness->IsActive(); })
				.OnClicked_Lambda([this]()
				{
					if (Harness.IsValid()) Harness->GenerateLogEntries();
					return FReply::Handled();
				})
			]

			+ SWrapBox::Slot().Padding(0.0f, 0.0f, 4.0f, 4.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("ForceGC", "Force GC"))
				.ToolTipText(LOCTEXT("ForceGCTip", "Force garbage collection cycle (updates MGOS data)"))
				.IsEnabled_Lambda([this]() { return Harness.IsValid() && Harness->IsActive(); })
				.OnClicked_Lambda([this]()
				{
					if (Harness.IsValid()) Harness->ForceGarbageCollection();
					return FReply::Handled();
				})
			]

			+ SWrapBox::Slot().Padding(0.0f, 0.0f, 4.0f, 4.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("BroadcastMsg", "Broadcast Msg"))
				.ToolTipText(LOCTEXT("BroadcastMsgTip", "Broadcast a test message on a random channel"))
				.IsEnabled_Lambda([this]() { return Harness.IsValid() && Harness->IsActive(); })
				.OnClicked_Lambda([this]()
				{
					if (Harness.IsValid()) Harness->BroadcastTestMessage();
					return FReply::Handled();
				})
			]

			+ SWrapBox::Slot().Padding(0.0f, 0.0f, 4.0f, 4.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("CycleFlow", "Cycle Flow"))
				.ToolTipText(LOCTEXT("CycleFlowTip", "Transition a random GameFlow channel to next state"))
				.IsEnabled_Lambda([this]() { return Harness.IsValid() && Harness->IsActive(); })
				.OnClicked_Lambda([this]()
				{
					if (Harness.IsValid()) Harness->CycleGameFlowState();
					return FReply::Handled();
				})
			]

			+ SWrapBox::Slot().Padding(0.0f, 0.0f, 4.0f, 4.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("SaveSlot", "+ Save Slot"))
				.ToolTipText(LOCTEXT("SaveSlotTip", "Re-save an existing harness slot (updates Save inspector)"))
				.IsEnabled_Lambda([this]() { return Harness.IsValid() && Harness->IsActive(); })
				.OnClicked_Lambda([this]()
				{
					if (Harness.IsValid()) Harness->CycleSaveSlot();
					return FReply::Handled();
				})
			]

			+ SWrapBox::Slot().Padding(0.0f, 0.0f, 4.0f, 4.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("ExecHandler", "Exec Handler"))
				.ToolTipText(LOCTEXT("ExecHandlerTip", "Execute a random EventHandler (generates telemetry)"))
				.IsEnabled_Lambda([this]() { return Harness.IsValid() && Harness->IsActive(); })
				.OnClicked_Lambda([this]()
				{
					if (Harness.IsValid()) Harness->ExecuteRandomHandler();
					return FReply::Handled();
				})
			]

			+ SWrapBox::Slot().Padding(0.0f, 0.0f, 4.0f, 4.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("VerifyAPIs", "Verify APIs"))
				.ToolTipText(LOCTEXT("VerifyAPIsTip", "Run comprehensive API verification across all 13 subsystems"))
				.IsEnabled_Lambda([this]() { return Harness.IsValid() && Harness->IsActive(); })
				.OnClicked_Lambda([this]()
				{
					if (Harness.IsValid()) Harness->VerifyAllAPIs();
					return FReply::Handled();
				})
			]
		];
}

// ============================================================================
// EN: Panel Launcher — Open individual or all panels
// ES: Lanzador de paneles — Abrir paneles individuales o todos
// ============================================================================

TSharedRef<SWidget> SPGXSimHarnessTab::BuildPanelLauncher()
{
	TSharedRef<SVerticalBox> PanelBox = SNew(SVerticalBox);

	// ─── Harness Panels (injected data) ───
	PanelBox->AddSlot()
	.AutoHeight()
	[
		SNew(SPGXSectionDivider)
		.Title(LOCTEXT("SecHarnessPanels", "HARNESS PANELS"))
		.AccentColor(PGX::System::Simulation)
	];

	TArray<FName> PanelIds = FPGXVisualHarness::GetAllPanelIds();
	for (const FName& PanelId : PanelIds)
	{
		FName CapturedId = PanelId;
		PanelBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 1.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(GetPanelDisplayName(PanelId)))
				.Font(PGX::Font::BodySmall())
				.ToolTipText(FText::FromName(PanelId))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SButton)
				.Text(LOCTEXT("Open", "Open"))
				.OnClicked_Lambda([CapturedId]()
				{
					FGlobalTabmanager::Get()->TryInvokeTab(CapturedId);
					return FReply::Handled();
				})
			]
		];
	}

	// ─── Unit Test Only Panels (no harness data) ───
	PanelBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 8.0f, 0.0f, 0.0f)
	[
		SNew(SPGXSectionDivider)
		.Title(LOCTEXT("SecUnitTestOnly", "UNIT TEST ONLY (no harness data)"))
	];

	TArray<FName> UnitTestPanels = FPGXVisualHarness::GetUnitTestOnlyPanelIds();
	for (const FName& PanelId : UnitTestPanels)
	{
		FName CapturedId = PanelId;
		PanelBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 1.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(GetPanelDisplayName(PanelId)))
				.Font(PGX::Font::BodySmall())
				.ToolTipText(FText::FromName(PanelId))
				.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SButton)
				.Text(LOCTEXT("OpenUnit", "Open"))
				.OnClicked_Lambda([CapturedId]()
				{
					FGlobalTabmanager::Get()->TryInvokeTab(CapturedId);
					return FReply::Handled();
				})
			]
		];
	}

	return PanelBox;
}

// ============================================================================
// EN: UI Refresh — Updates status dashboard and system grid every 1s
// ES: Refresco de UI — Actualiza dashboard de estado y grid de sistemas cada 1s
// ============================================================================

bool SPGXSimHarnessTab::OnUIRefreshTick(float /*DeltaTime*/)
{
	RefreshHarnessUI();
	return Harness.IsValid() && Harness->IsActive();
}

void SPGXSimHarnessTab::RefreshHarnessUI()
{
	if (!Harness.IsValid()) return;

	PGX_LOG_VERBOSE(LogPGXSimHarness, TEXT("[PSPH] RefreshHarnessUI — Active=%d Sim=%d"),
		Harness->IsActive(), Harness->IsSimulating());

	// EN: Update 4 KPI text blocks / ES: Actualizar 4 text blocks de KPI
	const bool bActive = Harness->IsActive();
	const bool bSim = Harness->IsSimulating();
	const double Elapsed = Harness->GetElapsedSeconds();
	const int32 Minutes = static_cast<int32>(Elapsed) / 60;
	const int32 Seconds = static_cast<int32>(Elapsed) % 60;
	const int32 TotalObjs = Harness->GetTotalObjectCount();

	if (StatusKPI.IsValid())
	{
		StatusKPI->SetText(FText::FromString(bActive ? TEXT("Active") : TEXT("Inactive")));
		FLinearColor StatusColor = bActive
			? (bSim ? PGX::Semantic::Good : PGX::Semantic::Warn)
			: PGX::Text::Secondary;
		StatusKPI->SetColorAndOpacity(FSlateColor(StatusColor));
	}

	if (SimKPI.IsValid())
	{
		SimKPI->SetText(FText::FromString(bSim ? TEXT("Running") : TEXT("Off")));
		SimKPI->SetColorAndOpacity(FSlateColor(bSim ? PGX::Semantic::Good : PGX::Text::Secondary));
	}

	if (ElapsedKPI.IsValid())
	{
		ElapsedKPI->SetText(FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds)));
	}

	if (ObjectsKPI.IsValid())
	{
		ObjectsKPI->SetText(FText::AsNumber(TotalObjs));
	}

	// EN: Update footer / ES: Actualizar footer
	if (FooterText.IsValid())
	{
		FString FooterStr = bActive
			? (bSim
				? FString::Printf(TEXT("Simulating — %d objects — %02d:%02d elapsed"), TotalObjs, Minutes, Seconds)
				: FString::Printf(TEXT("Harness active — %d objects injected"), TotalObjs))
			: TEXT("Ready — Select DEMO or HARNESS mode");
		FooterText->SetText(FText::FromString(FooterStr));
	}

	// EN: Rebuild system grid / ES: Reconstruir grid de sistemas
	if (SystemGridBox.IsValid())
	{
		SystemGridBox->ClearChildren();

		if (!Harness->IsActive())
		{
			SystemGridBox->AddSlot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("GridPlaceholder2", "Start Harness to see system status"))
				.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
			];
			return;
		}

		TArray<FPGXHarnessSystemStatus> Statuses = Harness->GetSystemStatuses();
		for (const FPGXHarnessSystemStatus& Sys : Statuses)
		{
			FLinearColor StatusColor = Sys.bInjected
				? PGX::Semantic::Good
				: PGX::Semantic::Error;
			FString StatusLabel = Sys.bInjected ? TEXT("[OK]") : TEXT("[--]");

			SystemGridBox->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 1.0f)
			[
				SNew(SHorizontalBox)

				// EN: System name / ES: Nombre del sistema
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBox)
					.WidthOverride(110.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(Sys.SystemName))
						.Font(PGX::Font::BodySmall())
					]
				]

				// EN: Status / ES: Estado
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBox)
					.WidthOverride(50.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(StatusLabel))
						.Font(PGX::Font::Badge())
						.ColorAndOpacity(FSlateColor(StatusColor))
					]
				]

				// EN: Object count / ES: Conteo de objetos
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBox)
					.WidthOverride(40.0f)
					[
						SNew(STextBlock)
						.Text(FText::AsNumber(Sys.ObjectCount))
						.Font(PGX::Font::BodySmall())
						.Justification(ETextJustify::Right)
					]
				]

				// EN: Detail / ES: Detalle
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(8.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Sys.Detail))
					.Font(PGX::Font::BodySmall())
					.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
				]
			];
		}
	}
}

// ============================================================================
// EN: Panel Display Name Map
// ES: Mapa de nombres de paneles legibles
// ============================================================================

FString SPGXSimHarnessTab::GetPanelDisplayName(const FName& PanelId)
{
	// EN: Read the display name from the registered tab spawner — it was set via
	//     RegisterNomadTab(...).SetDisplayName(...). This is the single source of truth and
	//     removes the old hard-coded map, whose ids had drifted from the real registration
	//     (PGXLogInspector / PGXRegistryBrowser / PGXPlatformHealth / PGXDocsViewer were never
	//     registered ids). If an id has no spawner, the strip-PGX fallback makes the gap visible.
	// ES: Lee el display name del tab spawner registrado — se seteo via RegisterNomadTab.
	//     Fuente unica de verdad; elimina el mapa hard-coded cuyos ids habian drifteado del
	//     registro real. Si un id no tiene spawner, el fallback strip-PGX hace visible el hueco.
	if (const TSharedPtr<FTabSpawnerEntry> Spawner = FGlobalTabmanager::Get()->FindTabSpawnerFor(PanelId))
	{
		const FString DisplayName = Spawner->GetDisplayName().ToString();
		if (!DisplayName.IsEmpty())
		{
			return DisplayName;
		}
	}

	// EN: Fallback — strip "PGX" prefix / ES: Fallback — quitar prefijo "PGX"
	FString Name = PanelId.ToString();
	if (Name.StartsWith(TEXT("PGX")))
	{
		Name.RemoveFromStart(TEXT("PGX"));
	}
	return Name;
}

//  coverage — Coverage matrix UI widget. Compact summary + expandable detail.
TSharedRef<SWidget> SPGXSimHarnessTab::BuildCoverageMatrix()
{
	TArray<FPGXPluginCoverage> Matrix = FPGXVisualHarness::GetCoverageMatrix();

	// Count by status
	int32 Covered = 0, Partial = 0, Missing = 0, NA = 0;
	int32 HighPriorityCount = 0, MediumPriorityCount = 0, LowPriorityCount = 0;
	for (const FPGXPluginCoverage& P : Matrix)
	{
		switch (P.Coverage)
		{
		case EPGXHarnessCoverage::Covered:      ++Covered; break;
		case EPGXHarnessCoverage::Partial:      ++Partial; break;
		case EPGXHarnessCoverage::Missing:      ++Missing; break;
		case EPGXHarnessCoverage::NotApplicable: ++NA; break;
		}
		if (!P.ScenarioName.IsEmpty())
		{
			switch (P.Priority) { case 0: ++HighPriorityCount; break; case 1: ++MediumPriorityCount; break; default: ++LowPriorityCount; break; }
		}
	}
	const int32 TotalDesigned = HighPriorityCount + MediumPriorityCount + LowPriorityCount;

	TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);

	// Section header
	Box->AddSlot().AutoHeight()
	[
		SNew(SPGXSectionDivider)
		.Title(LOCTEXT("SecCoverage", "COVERAGE MATRIX"))
		.AccentColor(PGX::System::Simulation)
	];

	// Summary row
	Box->AddSlot().AutoHeight().Padding(4.0f, 4.0f)
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
		[
			SNew(STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("Covered: %d"), Covered)))
			.Font(PGX::Font::Badge())
			.ColorAndOpacity(FSlateColor(PGX::Semantic::Good))
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
		[
			SNew(STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("Partial: %d"), Partial)))
			.Font(PGX::Font::Badge())
			.ColorAndOpacity(FSlateColor(PGX::Semantic::Warn))
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
		[
			SNew(STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("Missing: %d"), Missing)))
			.Font(PGX::Font::Badge())
			.ColorAndOpacity(FSlateColor(PGX::Semantic::Error))
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
		[
			SNew(STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("Scenarios: %d (High:%d Medium:%d Low:%d)"), TotalDesigned, HighPriorityCount, MediumPriorityCount, LowPriorityCount)))
			.Font(PGX::Font::BodySmall())
			.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
		]
	];

	// Detail: Missing plugins with high-priority scenarios
	if (Missing > 0)
	{
		Box->AddSlot().AutoHeight().Padding(4.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("MissingHighPriorityHint", "High-priority scenarios without runtime coverage:"))
			.Font(PGX::Font::Caption())
			.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
		];

		// Gather high-priority missing plugins
		TArray<FString> HighPriorityMissing;
		for (const FPGXPluginCoverage& P : Matrix)
		{
			if (P.Coverage == EPGXHarnessCoverage::Missing && P.Priority == 0 && !P.ScenarioName.IsEmpty())
			{
				HighPriorityMissing.Add(FString::Printf(TEXT("%s [%s]"), *P.PluginName, *P.ScenarioName));
			}
		}

		if (HighPriorityMissing.Num() > 0)
		{
			FString HighPriorityStr = FString::Join(HighPriorityMissing, TEXT(", "));
			Box->AddSlot().AutoHeight().Padding(4.0f, 0.0f, 4.0f, 4.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(HighPriorityStr))
				.Font(PGX::Font::BodySmall())
				.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
				.AutoWrapText(true)
			];
		}
	}

	return Box;
}

#undef LOCTEXT_NAMESPACE
