// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Observer/SPGXSystemObserverTab.h"

#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/AppStyle.h"
#include "Utils/PGXEditorUtils.h"
#include "Style/PGXVisualTokens.h"
#include "Style/PGXEditorStyle.h"
#include "Widgets/SPGXPremiumShell.h"
#include "Widgets/SPGXSectionDivider.h"
#include "Widgets/PGXButtonHelpers.h"
#include "Widgets/SPGXStatusBadge.h"
#include "Widgets/SPGXKPIChip.h"
#include "Widgets/SPGXEmptyStateV2.h"
#include "Widgets/SPGXHealthDot.h"
#include "Widgets/SPGXSparkline.h"
#include "Utils/SPGXTelemetryGraph.h"
#include "Editor.h"
#include "Modules/ModuleManager.h"

// EN: PGX subsystem headers for data collection
// ES: Headers de subsistemas PGX para recoleccion de datos
#include "Profile/PGXProfileSubsystem.h"
#include "Subsystems/PGXLogSubsystem.h"
#include "PGXSaveSubsystem.h"
#include "Logging/PGXLogMacros.h"
#include "PGXGameFlowSubsystem.h"
#include "PGXPSOSubsystem.h"
#include "PGXLoadingSubsystem.h"
#include "PGXLevelFlowSubsystem.h"
#include "PGXGCObserverSubsystem.h"
#include "PGXAudioSubsystem.h"
#include "Registry/PGXDataRegistrySubsystem.h"
#include "PGXSourceControlSubsystem.h"
#include "Messages/PGXMessageSubsystem.h"
#include "EventHandler/PGXEventHandlerSubsystem.h"
#include "Construction/PGXWorldConstructionSubsystem.h"

// EN: Trace infrastructure for diagnostics
// ES: Infraestructura de traza para diagnosticos
#include "Trace/PGXTraceHelper.h"
#include "Path/PGXPathResolver.h"

#include "Engine/GameInstance.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"

#define LOCTEXT_NAMESPACE "PGXSystemObserver"
DEFINE_LOG_CATEGORY_STATIC(LogPGXObserver, Log, All);

// ============================================================================
// EN: System Color Helper / ES: Helper de Color de Sistema
// ============================================================================

FLinearColor SPGXSystemObserverTab::GetSystemColorByName(FName Name)
{
	// EN: Delegate to centralized token lookup (PGXVisualTokens.h SSOT)
	// ES: Delegar al lookup centralizado de tokens (PGXVisualTokens.h SSOT)
	return PGX::System::GetColorByName(Name);
}

// ============================================================================
// EN: Construct / Destructor
// ES: Construccion / Destructor
// ============================================================================

void SPGXSystemObserverTab::Construct(const FArguments& /*InArgs*/)
{
	PGX_LOG_INFO(LogPGXObserver, TEXT("[Observer] Construct"));

	CachedSubsystemCount = LOCTEXT("SubCountInit", "--/--");
	CachedFooterText = LOCTEXT("FooterInit", "System Observer -- Start PIE for live data");

	ChildSlot
	[
		// EN: Premium Shell -- covers UE chrome, provides title bar + footer
		// ES: Shell Premium -- cubre chrome de UE, provee title bar + footer
		SNew(SPGXPremiumShell)
		.SystemColor(PGX::System::MGOS)
		.Title(LOCTEXT("ObserverTitle", "System Observer"))
		.Subtitle(LOCTEXT("ObserverSub", "Live framework health dashboard"))
		.Icon(FPGXEditorStyle::Get().GetBrush("PGXEditor.Icon.SystemObserver"))
		.TitleRightContent()
		[
			BuildToolbar()
		]
		.bShowFooter(true)
		.StatusText_Lambda([this]() { return CachedFooterText; })
		.StatusColor_Lambda([this]() { return bIsPIEActive ? PGX::Semantic::Good : PGX::Text::Muted; })
		.Content()
		[
			// EN: Main content scrollable area / ES: Area de contenido principal scrollable
			SNew(SScrollBox)

			// EN: Section 1: Core Systems / ES: Seccion 1: Sistemas Core
			+ SScrollBox::Slot()
			.Padding(PGX::Spacing::SM)
			[
				BuildCoreSystemsSection()
			]

			// EN: Section 2: Subsystems / ES: Seccion 2: Subsistemas
			+ SScrollBox::Slot()
			.Padding(PGX::Spacing::SM)
			[
				BuildSubsystemsSection()
			]

			// EN: Section 2.5: Telemetry Overview / ES: Seccion 2.5: Resumen de Telemetria
			+ SScrollBox::Slot()
			.Padding(PGX::Spacing::SM)
			[
				BuildTelemetryOverviewSection()
			]

			// EN: Section 3: Active Instances / ES: Seccion 3: Instancias Activas
			+ SScrollBox::Slot()
			.Padding(PGX::Spacing::SM)
			[
				BuildActiveInstancesSection()
			]

			// EN: Section 4: Plugins / ES: Seccion 4: Plugins
			+ SScrollBox::Slot()
			.Padding(PGX::Spacing::SM)
			[
				BuildPluginGridSection()
			]

			// EN: Section 5: Data Registry / ES: Seccion 5: Data Registry
			+ SScrollBox::Slot()
			.Padding(PGX::Spacing::SM)
			[
				BuildDataRegistrySection()
			]
		]
	];

	BindPIEDelegates();

	// EN: Collect plugin data immediately (doesn't require PIE)
	// ES: Recolectar datos de plugins inmediatamente (no requiere PIE)
	CollectPlugins();

	// EN: If PIE is already running when the panel opens, bind immediately
	// ES: Si PIE ya esta corriendo cuando se abre el panel, bindear inmediatamente
	if (GEditor && GEditor->PlayWorld)
	{
		OnPIEStarted(false);
	}

	PGX_LOG_INFO(LogPGXObserver, TEXT("[Observer] Construct complete -- PIE delegates bound"));
}

SPGXSystemObserverTab::~SPGXSystemObserverTab()
{
	PGX_LOG_INFO(LogPGXObserver, TEXT("[Observer] Destructor -- cleaning up delegates + ticker"));

	StopPolling();

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
// EN: UI Build / ES: Construccion de UI
// ============================================================================

TSharedRef<SWidget> SPGXSystemObserverTab::BuildToolbar()
{
	return SNew(SHorizontalBox)

		// EN: KPI chip: subsystem count / ES: KPI chip: conteo de subsistemas
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.0f, 0.0f, PGX::Spacing::MD, 0.0f)
		[
			SAssignNew(SubsystemChip, SPGXKPIChip)
			.Label(LOCTEXT("SubCountLabel", "Subsystems"))
			.Value_Lambda([this]() { return CachedSubsystemCount; })
			.AccentColor(PGX::Semantic::Info)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(PGX::Spacing::SM, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Refresh", "Refresh"))
			.OnClicked(this, &SPGXSystemObserverTab::OnRefreshClicked)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(PGX::Spacing::MD, 0.0f)
		[
			SAssignNew(StatusText, STextBlock)
			.Text(LOCTEXT("StatusIdle", "Idle -- Start PIE for live data"))
			.ColorAndOpacity(PGX::Text::Secondary)
		];
}

TSharedRef<SWidget> SPGXSystemObserverTab::BuildCoreSystemsSection()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("CoreSystems", "CORE SYSTEMS"))
			.AccentColor(PGX::Semantic::Info)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(PGX::Spacing::MD, PGX::Spacing::SM)
		[
			SAssignNew(CoreClassesBox, SVerticalBox)

			// EN: Placeholder / ES: Placeholder
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SPGXEmptyStateV2)
				.Message(LOCTEXT("CoreWaiting", "Waiting for PIE..."))
				.Hint(LOCTEXT("CoreWaitingHint", "Start PIE to see core system classes"))
			]
		];
}

// EN: Change 6 -- SHeaderRow for subsystem SListView
// ES: Cambio 6 -- SHeaderRow para SListView de subsistemas
TSharedRef<SWidget> SPGXSystemObserverTab::BuildSubsystemsSection()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("Subsystems", "SUBSYSTEMS"))
			.AccentColor(PGX::Semantic::Info)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(PGX::Spacing::MD, PGX::Spacing::SM)
		[
			SAssignNew(SubsystemListView, SListView<TSharedPtr<FPGXSubsystemSnapshot>>)
			.ListItemsSource(&SubsystemSnapshots)
			.OnGenerateRow(this, &SPGXSystemObserverTab::OnGenerateSubsystemRow)
			.SelectionMode(ESelectionMode::None)
			.HeaderRow(
				SNew(SHeaderRow)
				+ SHeaderRow::Column("Color")
				.DefaultLabel(FText::GetEmpty())
				.FixedWidth(8.0f)

				+ SHeaderRow::Column("Health")
				.DefaultLabel(LOCTEXT("ColHealth", "Health"))
				.FixedWidth(70.0f)

				+ SHeaderRow::Column("Name")
				.DefaultLabel(LOCTEXT("ColName", "Subsystem"))
				.FillWidth(0.3f)

				+ SHeaderRow::Column("Version")
				.DefaultLabel(LOCTEXT("ColVer", "Ver"))
				.FixedWidth(50.0f)

				+ SHeaderRow::Column("Status")
				.DefaultLabel(LOCTEXT("ColStatus", "Status"))
				.FillWidth(0.7f)
			)
		];
}

// ============================================================================
// EN: Telemetry Overview -- Active Subsystem Count Graph
// ES: Resumen de Telemetria -- Grafico de Conteo de Subsistemas Activos
// ============================================================================

TSharedRef<SWidget> SPGXSystemObserverTab::BuildTelemetryOverviewSection()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("TelemetryOverview", "TELEMETRY OVERVIEW"))
			.AccentColor(PGX::Semantic::Info)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ActiveSubsystemsLabel", "Active Subsystems Over Time"))
			.Font(PGX::Font::SubHeader())
			.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2)
		[
			SNew(SBox)
			.HeightOverride(PGX::Height::GraphCompact)
			[
				SAssignNew(ActiveSubsystemGraph, SPGXTelemetryGraph)
				.BufferSize(128)
				.LineColor(PGX::Semantic::Good)
				.MinValue(0.0f)
				.MaxValue(20.0f)
				.bShowGrid(true)
				.bShowAvgLine(true)
				.bShowLabels(true)
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, PGX::Spacing::MD)
		[
			PGXEditorUtils::BuildGraphLegend(ActiveSubsystemGraph, PGX::Semantic::Good)
		];
}

// EN: Change 3 -- Active Instances: SVerticalBox with styled rows instead of STextBlock dump
// ES: Cambio 3 -- Instancias Activas: SVerticalBox con filas estilizadas en vez de dump STextBlock
TSharedRef<SWidget> SPGXSystemObserverTab::BuildActiveInstancesSection()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("ActiveInstances", "ACTIVE INSTANCES"))
			.AccentColor(PGX::Semantic::Info)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(PGX::Spacing::MD, PGX::Spacing::SM)
		[
			SAssignNew(InstanceDetailBox, SVerticalBox)

			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SPGXEmptyStateV2)
				.Message(LOCTEXT("InstancesWaiting", "Waiting for PIE..."))
				.Hint(LOCTEXT("InstancesWaitingHint", "Start PIE to see active subsystem instances"))
			]
		];
}

TSharedRef<SWidget> SPGXSystemObserverTab::BuildPluginGridSection()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("Plugins", "PLUGINS (L2)"))
			.AccentColor(PGX::Semantic::Info)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(PGX::Spacing::MD, PGX::Spacing::SM)
		[
			SAssignNew(PluginGrid, SWrapBox)
			.UseAllottedSize(true)
		];
}

// ============================================================================
// EN: SListView Row Generator / ES: Generador de Filas SListView
// ============================================================================

// EN: Changes 1+2 -- STUB badge + system color bar in subsystem rows
// ES: Cambios 1+2 -- Badge STUB + barra de color de sistema en filas de subsistemas
TSharedRef<ITableRow> SPGXSystemObserverTab::OnGenerateSubsystemRow(
	TSharedPtr<FPGXSubsystemSnapshot> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	const FLinearColor HealthColor = GetHealthColor(Item->Health);
	const FString HealthStr = GetHealthText(Item->Health);
	const FLinearColor SystemColor = GetSystemColorByName(Item->SubsystemName);

	// EN: Build row content
	// ES: Construir contenido de fila
	TSharedRef<SHorizontalBox> RowContent = SNew(SHorizontalBox)

		// EN: Change 2 -- 4px system color bar / ES: Cambio 2 -- Barra de color de sistema de 4px
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.0f, 0.0f, PGX::Spacing::XS, 0.0f)
		[
			SNew(SBox)
			.WidthOverride(PGX::Width::AccentStripe)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderBackgroundColor(SystemColor)
			]
		]

		// EN: Health dot / ES: Dot de salud
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(PGX::Spacing::SM, PGX::Spacing::XS)
		[
			SNew(SPGXHealthDot)
			.State(Item->Health == EPGXSystemHealth::Active ? EPGXHealthState::Active
				: Item->Health == EPGXSystemHealth::Warning ? EPGXHealthState::Warning
				: Item->Health == EPGXSystemHealth::Error ? EPGXHealthState::Error
				: EPGXHealthState::Offline)
		];

	// EN: Change 1 -- [STUB] badge for non-implemented subsystems
	// ES: Cambio 1 -- Badge [STUB] para subsistemas no implementados
	if (!Item->bIsImplemented)
	{
		RowContent->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(PGX::Spacing::SM, PGX::Spacing::XS)
		[
			SNew(SPGXStatusBadge)
			.Shape(EPGXBadgeShape::Pill)
			.Color(PGX::Semantic::Neutral)
			.Label(LOCTEXT("StubBadge", "STUB"))
		];
	}

	// EN: Subsystem name / ES: Nombre del subsistema
	RowContent->AddSlot()
	.AutoWidth()
	.VAlign(VAlign_Center)
	.Padding(PGX::Spacing::MD, PGX::Spacing::XS)
	[
		SNew(STextBlock)
		.Text(FText::FromName(Item->SubsystemName))
		.Font(PGX::Font::SubHeader())
		.ColorAndOpacity(SystemColor)
	];

	// EN: Version / ES: Version
	RowContent->AddSlot()
	.AutoWidth()
	.VAlign(VAlign_Center)
	.Padding(PGX::Spacing::MD, PGX::Spacing::XS)
	[
		SNew(STextBlock)
		.Text(FText::FromString(Item->Version))
		.ColorAndOpacity(PGX::Text::Secondary)
	];

	// EN: Status detail / ES: Detalle de estado
	RowContent->AddSlot()
	.FillWidth(1.0f)
	.VAlign(VAlign_Center)
	.Padding(PGX::Spacing::MD, PGX::Spacing::XS)
	[
		SNew(STextBlock)
		.Text(FText::FromString(Item->StatusDetail))
		.ColorAndOpacity(PGX::Text::Secondary)
	];

	// EN: Per-subsystem sparkline / ES: Sparkline por subsistema
	{
		TSharedPtr<SPGXSparkline>& SparkRef = PerSubsystemSparklines.FindOrAdd(Item->SubsystemName);
		if (!SparkRef.IsValid())
		{
			SAssignNew(SparkRef, SPGXSparkline)
				.LineColor(SystemColor);
		}
		RowContent->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(PGX::Spacing::SM, PGX::Spacing::XS)
		[
			SNew(SBox)
			.WidthOverride(40.0f)
			.HeightOverride(PGX::Height::SparklineDefault)
			[
				SparkRef.ToSharedRef()
			]
		];
	}

	return SNew(STableRow<TSharedPtr<FPGXSubsystemSnapshot>>, OwnerTable)
		[
			RowContent
		];
}

// ============================================================================
// EN: Data Collection / ES: Recoleccion de Datos
// ============================================================================

void SPGXSystemObserverTab::CollectCoreClasses()
{
	CoreSnapshots.Empty();

	if (!bIsPIEActive || !GEditor)
	{
		return;
	}

	UWorld* PIEWorld = nullptr;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::PIE && Context.World())
		{
			PIEWorld = Context.World();
			break;
		}
	}

	if (!PIEWorld)
	{
		return;
	}

	// EN: GameInstance / ES: GameInstance
	{
		FPGXCoreClassSnapshot Snap;
		Snap.Role = TEXT("GameInstance");
		UGameInstance* GI = PIEWorld->GetGameInstance();
		Snap.ClassName = GI ? GI->GetClass()->GetName() : TEXT("NULL");
		Snap.Health = GI ? EPGXSystemHealth::Active : EPGXSystemHealth::Error;
		CoreSnapshots.Add(MoveTemp(Snap));
	}

	// EN: GameMode / ES: GameMode
	{
		FPGXCoreClassSnapshot Snap;
		Snap.Role = TEXT("GameMode");
		AGameModeBase* GM = PIEWorld->GetAuthGameMode();
		Snap.ClassName = GM ? GM->GetClass()->GetName() : TEXT("NULL");
		Snap.Health = GM ? EPGXSystemHealth::Active : EPGXSystemHealth::Inactive;
		CoreSnapshots.Add(MoveTemp(Snap));
	}

	// EN: PlayerController / ES: PlayerController
	{
		FPGXCoreClassSnapshot Snap;
		Snap.Role = TEXT("PlayerController");
		APlayerController* PC = PIEWorld->GetFirstPlayerController();
		Snap.ClassName = PC ? PC->GetClass()->GetName() : TEXT("NULL");
		Snap.Health = PC ? EPGXSystemHealth::Active : EPGXSystemHealth::Inactive;
		CoreSnapshots.Add(MoveTemp(Snap));
	}

	// EN: GameState / ES: GameState
	{
		FPGXCoreClassSnapshot Snap;
		Snap.Role = TEXT("GameState");
		AGameStateBase* GS = PIEWorld->GetGameState();
		Snap.ClassName = GS ? GS->GetClass()->GetName() : TEXT("NULL");
		Snap.Health = GS ? EPGXSystemHealth::Active : EPGXSystemHealth::Inactive;
		CoreSnapshots.Add(MoveTemp(Snap));
	}

	// EN: Update UI / ES: Actualizar UI
	if (CoreClassesBox.IsValid())
	{
		CoreClassesBox->ClearChildren();
		for (const FPGXCoreClassSnapshot& Snap : CoreSnapshots)
		{
			const FLinearColor Color = GetHealthColor(Snap.Health);
			CoreClassesBox->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 1.0f)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, PGX::Spacing::MD, 0.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(FString::Printf(TEXT("%s:"), *Snap.Role.ToString())))
					.Font(PGX::Font::Badge())
					.MinDesiredWidth(120.0f)
				]

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Snap.ClassName))
					.ColorAndOpacity(Color)
				]
			];
		}
	}
}

// ============================================================================
// EN: Observation patterns of this panel (4 distinct data sources, NOT one).
//     Each Collect* function reads a different live surface -- none consume the
//     IPGXObservable type registry (that is a class catalog, not a runtime view):
//       - subsystem-health   : CollectSubsystems / CollectActiveInstances
//                               via GetSubsystem<UPGX*Subsystem>() (live singletons).
//       - runtime-core / PIE  : CollectCoreClasses via PIEWorld->GetGameInstance().
//       - module-inventory    : CollectPlugins via FModuleManager::IsModuleLoaded.
//       - dataregistry-dbs    : CollectDataRegistry via UPGXDataRegistrySubsystem
//                               (gameplay databases -- a DIFFERENT registry from
//                               FPGXObservabilityRegistry in Observability/).
// ES: Patrones de observacion de este panel (4 fuentes de datos distintas, no una).
//     Ninguna Collect* consume el registro de tipos IPGXObservable (es un catalogo
//     de clases, no una vista runtime). El panel del ObservabilityRegistry (mostrar
//     las clases registradas) es feature futura, no parte de este observer.
// ============================================================================

void SPGXSystemObserverTab::CollectSubsystems()
{
	SubsystemSnapshots.Empty();

	if (!bIsPIEActive)
	{
		if (SubsystemListView.IsValid())
		{
			SubsystemListView->RequestListRefresh();
		}
		return;
	}

	// EN: Get GameInstance and World from PIE context
	// ES: Obtener GameInstance y World del contexto PIE
	UGameInstance* GI = nullptr;
	UWorld* PIEWorld = nullptr;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::PIE && Context.World())
		{
			PIEWorld = Context.World();
			GI = PIEWorld->GetGameInstance();
			break;
		}
	}

	if (!GI)
	{
		return;
	}

	// EN: PGXProfile / ES: PGXProfile
	{
		auto Snap = MakeShared<FPGXSubsystemSnapshot>();
		Snap->SubsystemName = TEXT("PGXProfile");
		UPGXProfileSubsystem* Sub = GI->GetSubsystem<UPGXProfileSubsystem>();
		if (Sub)
		{
			Snap->Health = Sub->IsProfileResolved() ? EPGXSystemHealth::Active : EPGXSystemHealth::Warning;
			Snap->Version = TEXT("v2.0");
			Snap->StatusDetail = FString::Printf(TEXT("State: %s"),
				*UEnum::GetValueAsString(Sub->GetProfileState()));
			Snap->bIsImplemented = true;
		}
		else
		{
			Snap->Health = EPGXSystemHealth::Inactive;
			Snap->bIsImplemented = false;
		}
		SubsystemSnapshots.Add(Snap);
	}

	// EN: PGXLog / ES: PGXLog
	{
		auto Snap = MakeShared<FPGXSubsystemSnapshot>();
		Snap->SubsystemName = TEXT("PGXLog");
		UPGXLogSubsystem* Sub = GI->GetSubsystem<UPGXLogSubsystem>();
		if (Sub)
		{
			Snap->Health = EPGXSystemHealth::Active;
			Snap->Version = TEXT("v3.0");
			Snap->StatusDetail = FString::Printf(TEXT("%d entries"), Sub->GetEntryCount());
			Snap->MetricValue = static_cast<float>(Sub->GetEntryCount());
			Snap->bIsImplemented = true;
		}
		else
		{
			Snap->Health = EPGXSystemHealth::Inactive;
			Snap->bIsImplemented = false;
		}
		SubsystemSnapshots.Add(Snap);
	}

	// EN: PGXSave / ES: PGXSave
	{
		auto Snap = MakeShared<FPGXSubsystemSnapshot>();
		Snap->SubsystemName = TEXT("PGXSave");
		UPGXSaveSubsystem* Sub = GI->GetSubsystem<UPGXSaveSubsystem>();
		if (Sub)
		{
			Snap->Health = EPGXSystemHealth::Active;
			Snap->Version = TEXT("v1.0");
			Snap->StatusDetail = FString::Printf(TEXT("%d contexts"), Sub->GetContextCount());
			Snap->bIsImplemented = true;
		}
		else
		{
			Snap->Health = EPGXSystemHealth::Inactive;
			Snap->bIsImplemented = false;
		}
		SubsystemSnapshots.Add(Snap);
	}

	// EN: PGXGameFlow / ES: PGXGameFlow
	{
		auto Snap = MakeShared<FPGXSubsystemSnapshot>();
		Snap->SubsystemName = TEXT("PGXGameFlow");
		UPGXGameFlowSubsystem* Sub = GI->GetSubsystem<UPGXGameFlowSubsystem>();
		if (Sub)
		{
			Snap->Health = EPGXSystemHealth::Active;
			Snap->Version = TEXT("v1.0");
			// EN: Show current Global channel flow tag / ES: Mostrar tag de flujo actual del canal Global
			const FGameplayTag CurrentTag = Sub->GetCurrentFlowTag(EPGXFlowChannel::Global);
			Snap->StatusDetail = FString::Printf(TEXT("%d channels, Global: %s"),
				PGX_FLOW_CHANNEL_COUNT, CurrentTag.IsValid() ? *CurrentTag.ToString() : TEXT("(none)"));
			Snap->bIsImplemented = true;
		}
		else
		{
			Snap->Health = EPGXSystemHealth::Inactive;
			Snap->bIsImplemented = false;
		}
		SubsystemSnapshots.Add(Snap);
	}

	// EN: PGXPSO / ES: PGXPSO
	{
		auto Snap = MakeShared<FPGXSubsystemSnapshot>();
		Snap->SubsystemName = TEXT("PGXPSO");
		UPGXPSOSubsystem* Sub = GI->GetSubsystem<UPGXPSOSubsystem>();
		if (Sub)
		{
			Snap->Health = EPGXSystemHealth::Active;
			Snap->Version = TEXT("v2.0");
			Snap->StatusDetail = FString::Printf(TEXT("State: %s"),
				*UEnum::GetValueAsString(Sub->GetWarmUpState()));
			Snap->bIsImplemented = true;
		}
		else
		{
			Snap->Health = EPGXSystemHealth::Inactive;
			Snap->bIsImplemented = false;
		}
		SubsystemSnapshots.Add(Snap);
	}

	// EN: PGXLoading / ES: PGXLoading
	{
		auto Snap = MakeShared<FPGXSubsystemSnapshot>();
		Snap->SubsystemName = TEXT("PGXLoading");
		UPGXLoadingSubsystem* Sub = GI->GetSubsystem<UPGXLoadingSubsystem>();
		if (Sub)
		{
			Snap->Health = EPGXSystemHealth::Active;
			Snap->Version = TEXT("v1.0");
			Snap->StatusDetail = FString::Printf(TEXT("State: %s"),
				*UEnum::GetValueAsString(Sub->GetCurrentState()));
			Snap->bIsImplemented = true;
		}
		else
		{
			Snap->Health = EPGXSystemHealth::Inactive;
			Snap->bIsImplemented = false;
		}
		SubsystemSnapshots.Add(Snap);
	}

	// EN: PGXLevelFlow / ES: PGXLevelFlow
	{
		auto Snap = MakeShared<FPGXSubsystemSnapshot>();
		Snap->SubsystemName = TEXT("PGXLevelFlow");
		UPGXLevelFlowSubsystem* Sub = GI->GetSubsystem<UPGXLevelFlowSubsystem>();
		if (Sub)
		{
			Snap->Health = EPGXSystemHealth::Active;
			Snap->Version = TEXT("v1.0");
			Snap->StatusDetail = FString::Printf(TEXT("Level: %s"),
				*Sub->GetCurrentLevelTag().ToString());
			Snap->bIsImplemented = true;
		}
		else
		{
			Snap->Health = EPGXSystemHealth::Inactive;
			Snap->bIsImplemented = false;
		}
		SubsystemSnapshots.Add(Snap);
	}

	// EN: PGXMGOS (UEngineSubsystem) / ES: PGXMGOS (UEngineSubsystem)
	{
		auto Snap = MakeShared<FPGXSubsystemSnapshot>();
		Snap->SubsystemName = TEXT("PGXMGOS");
		UPGXGCObserverSubsystem* Sub = GEngine->GetEngineSubsystem<UPGXGCObserverSubsystem>();
		if (Sub)
		{
			Snap->Health = Sub->IsInitialized() ? EPGXSystemHealth::Active : EPGXSystemHealth::Warning;
			Snap->Version = TEXT("v1.0");
			Snap->StatusDetail = Sub->IsInitialized() ? TEXT("Monitoring GC") : TEXT("Not initialized");
			Snap->bIsImplemented = true;
		}
		else
		{
			Snap->Health = EPGXSystemHealth::Inactive;
			Snap->bIsImplemented = false;
		}
		SubsystemSnapshots.Add(Snap);
	}

	// EN: PGXAudio / ES: PGXAudio
	{
		auto Snap = MakeShared<FPGXSubsystemSnapshot>();
		Snap->SubsystemName = TEXT("PGXAudio");
		UPGXAudioSubsystem* AudioSub = GI->GetSubsystem<UPGXAudioSubsystem>();
		if (AudioSub)
		{
			Snap->Health = EPGXSystemHealth::Active;
			Snap->Version = TEXT("v1.1");
			Snap->StatusDetail = FString::Printf(TEXT("Backend: %s | Sounds: %d"),
				*AudioSub->GetBackendStatusText(), AudioSub->GetActiveSoundCount());
			Snap->MetricValue = static_cast<float>(AudioSub->GetActiveSoundCount());
			Snap->bIsImplemented = true;
		}
		else
		{
			Snap->Health = EPGXSystemHealth::Inactive;
			Snap->bIsImplemented = false;
		}
		SubsystemSnapshots.Add(Snap);
	}

	// EN: PASS2A module-status rows for source-backed editor-inspector plugins
	// ES: Filas de estado de modulo PASS2A para plugins con inspector source-backed
	auto AddModuleStatusSnapshot = [this](const TCHAR* DisplayName, const TCHAR* RuntimeModule, const TCHAR* StatusHint)
	{
		auto Snap = MakeShared<FPGXSubsystemSnapshot>();
		Snap->SubsystemName = FName(DisplayName);
		const bool bModuleLoaded = FModuleManager::Get().IsModuleLoaded(RuntimeModule);
		Snap->Health = bModuleLoaded ? EPGXSystemHealth::Active : EPGXSystemHealth::Inactive;
		Snap->Version = TEXT("v1.0");
		Snap->StatusDetail = FString::Printf(TEXT("%s | Runtime module: %s"), StatusHint,
			bModuleLoaded ? TEXT("loaded") : TEXT("not loaded"));
		Snap->MetricValue = bModuleLoaded ? 1.0f : 0.0f;
		Snap->bIsImplemented = true;
		SubsystemSnapshots.Add(Snap);
	};

	AddModuleStatusSnapshot(TEXT("PGXTrade"), TEXT("PGXTradeRuntime"), TEXT("Observable Trade config + inspector"));
	AddModuleStatusSnapshot(TEXT("PGXCrafting"), TEXT("PGXCraftingRuntime"), TEXT("Observable recipe definition + inspector"));
	AddModuleStatusSnapshot(TEXT("PGXVehicles"), TEXT("PGXVehiclesRuntime"), TEXT("Observable vehicle definition + inspector"));
	AddModuleStatusSnapshot(TEXT("PGXSpawn"), TEXT("PGXSpawnRuntime"), TEXT("Observable spawn config/wave definitions + inspector"));

	// EN: PGXDataRegistry / ES: PGXDataRegistry
	{
		auto Snap = MakeShared<FPGXSubsystemSnapshot>();
		Snap->SubsystemName = TEXT("PGXDataRegistry");
		UPGXDataRegistrySubsystem* Sub = UPGXDataRegistrySubsystem::GetCached();
		if (Sub)
		{
			Snap->Health = EPGXSystemHealth::Active;
			Snap->Version = TEXT("v2.0");
			Snap->StatusDetail = FString::Printf(TEXT("Databases: %d"),
				Sub->GetAllDatabaseTags().Num());
			Snap->bIsImplemented = true;
		}
		else
		{
			Snap->Health = EPGXSystemHealth::Inactive;
			Snap->bIsImplemented = false;
		}
		SubsystemSnapshots.Add(Snap);
	}

	// EN: PGXConstruction (WorldSubsystem) / ES: PGXConstruction (WorldSubsystem)
	if (PIEWorld)
	{
		auto Snap = MakeShared<FPGXSubsystemSnapshot>();
		Snap->SubsystemName = TEXT("PGXConstruction");
		UPGXWorldConstructionSubsystem* Sub = PIEWorld->GetSubsystem<UPGXWorldConstructionSubsystem>();
		if (Sub)
		{
			Snap->Health = EPGXSystemHealth::Active;
			Snap->Version = TEXT("v1.3");
			Snap->StatusDetail = TEXT("DA resolver active");
			Snap->bIsImplemented = true;
		}
		else
		{
			Snap->Health = EPGXSystemHealth::Inactive;
			Snap->bIsImplemented = false;
		}
		SubsystemSnapshots.Add(Snap);
	}

	// EN: Infrastructure diagnostics / ES: Diagnosticos de infraestructura
	{
		auto Snap = MakeShared<FPGXSubsystemSnapshot>();
		Snap->SubsystemName = TEXT("PathResolver");
		Snap->Health = EPGXSystemHealth::Active;
		Snap->Version = TEXT("v0.4");
		Snap->StatusDetail = FString::Printf(TEXT("%d cached paths"), FPGXPathResolver::GetCacheSize());
		Snap->bIsImplemented = true;
		SubsystemSnapshots.Add(Snap);
	}

	{
		auto Snap = MakeShared<FPGXSubsystemSnapshot>();
		Snap->SubsystemName = TEXT("TraceHelper");
		Snap->Health = FPGXTraceHelper::IsTracingAvailable() ? EPGXSystemHealth::Active : EPGXSystemHealth::Warning;
		Snap->Version = TEXT("v0.4");
		Snap->StatusDetail = FString::Printf(TEXT("%d systems registered"),
			FPGXTraceHelper::GetRegisteredSystemCount());
		Snap->bIsImplemented = true;
		SubsystemSnapshots.Add(Snap);
	}

	// EN: PGXVersionControl (EditorSubsystem) / ES: PGXVersionControl (EditorSubsystem)
	{
		auto Snap = MakeShared<FPGXSubsystemSnapshot>();
		Snap->SubsystemName = TEXT("PGXVersionControl");
		if (GEditor)
		{
			UPGXSourceControlSubsystem* VCSub = GEditor->GetEditorSubsystem<UPGXSourceControlSubsystem>();
			if (VCSub)
			{
				Snap->Health = VCSub->IsProviderConnected() ? EPGXSystemHealth::Active : EPGXSystemHealth::Warning;
				Snap->Version = TEXT("v1.0");
				Snap->StatusDetail = FString::Printf(TEXT("Provider: %s | CLs: %d"),
					*VCSub->GetProviderName(), VCSub->GetChangelists().Num());
				Snap->bIsImplemented = true;
			}
			else
			{
				Snap->Health = EPGXSystemHealth::Inactive;
				Snap->bIsImplemented = false;
			}
		}
		else
		{
			Snap->Health = EPGXSystemHealth::Inactive;
			Snap->bIsImplemented = false;
		}
		SubsystemSnapshots.Add(Snap);
	}

	// EN: PGXMessage / ES: PGXMessage
	{
		auto Snap = MakeShared<FPGXSubsystemSnapshot>();
		Snap->SubsystemName = TEXT("PGXMessage");
		UPGXMessageSubsystem* Sub = GI->GetSubsystem<UPGXMessageSubsystem>();
		if (Sub)
		{
			Snap->Health = EPGXSystemHealth::Active;
			Snap->Version = TEXT("v1.0");
			FPGXMessageStats MsgStats = Sub->GetStats();
			Snap->StatusDetail = FString::Printf(TEXT("Channels: %d | Listeners: %d | Broadcasts: %d"),
				Sub->GetAllActiveChannels().Num(), Sub->GetTotalListenerCount(), MsgStats.TotalBroadcasts);
			Snap->bIsImplemented = true;
		}
		else
		{
			Snap->Health = EPGXSystemHealth::Inactive;
			Snap->bIsImplemented = false;
		}
		SubsystemSnapshots.Add(Snap);
	}

	// EN: PGXEventHandler / ES: PGXEventHandler
	{
		auto Snap = MakeShared<FPGXSubsystemSnapshot>();
		Snap->SubsystemName = TEXT("PGXEventHandler");
		UPGXEventHandlerSubsystem* Sub = GI->GetSubsystem<UPGXEventHandlerSubsystem>();
		if (Sub)
		{
			Snap->Health = EPGXSystemHealth::Active;
			Snap->Version = TEXT("v1.0");
			// EN: Show registered handler count / ES: Mostrar cantidad de handlers registrados
			Snap->StatusDetail = FString::Printf(TEXT("%d handlers registered"), Sub->GetAllRegisteredTags().Num());
			Snap->bIsImplemented = true;
		}
		else
		{
			Snap->Health = EPGXSystemHealth::Inactive;
			Snap->bIsImplemented = false;
		}
		SubsystemSnapshots.Add(Snap);
	}

	if (SubsystemListView.IsValid())
	{
		SubsystemListView->RequestListRefresh();
	}
}

// EN: Change 3 -- Active Instances as styled rows instead of text dump
// ES: Cambio 3 -- Instancias Activas como filas estilizadas en vez de texto dump
void SPGXSystemObserverTab::CollectActiveInstances()
{
	if (!InstanceDetailBox.IsValid())
	{
		return;
	}

	InstanceDetailBox->ClearChildren();

	if (!bIsPIEActive)
	{
		InstanceDetailBox->AddSlot()
		.AutoHeight()
		[
			SNew(SPGXEmptyStateV2)
			.Message(LOCTEXT("InstancesIdle", "No active PIE session"))
			.Hint(LOCTEXT("InstancesIdleHint", "Start PIE to inspect live subsystem data"))
		];
		return;
	}

	UGameInstance* GI = nullptr;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::PIE && Context.World())
		{
			GI = Context.World()->GetGameInstance();
			break;
		}
	}

	if (!GI)
	{
		InstanceDetailBox->AddSlot()
		.AutoHeight()
		[
			SNew(SPGXEmptyStateV2)
			.Message(LOCTEXT("NoGI", "GameInstance not available"))
			.Hint(LOCTEXT("NoGIHint", "Ensure your project has a valid GameInstance"))
		];
		return;
	}

	// EN: Helper lambda to add a styled instance row
	// ES: Lambda helper para agregar una fila de instancia estilizada
	auto AddInstanceRow = [this](const FName& SystemName, const FText& Label, const FString& Detail, const FLinearColor& ValueColor)
	{
		const FLinearColor SysColor = GetSystemColorByName(SystemName);
		InstanceDetailBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 1.0f)
		[
			SNew(SHorizontalBox)

			// EN: System color bar / ES: Barra de color de sistema
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, PGX::Spacing::XS, 0.0f)
			[
				SNew(SBox).WidthOverride(PGX::Width::AccentStripe)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
					.BorderBackgroundColor(SysColor)
				]
			]

			// EN: System label / ES: Label de sistema
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(PGX::Spacing::MD, 0.0f)
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(PGX::Font::Badge())
				.MinDesiredWidth(100.0f)
				.ColorAndOpacity(SysColor)
			]

			// EN: Detail value / ES: Valor de detalle
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Detail))
				.ColorAndOpacity(ValueColor)
			]
		];
	};

	// EN: Log stats / ES: Estadisticas de Log
	if (UPGXLogSubsystem* Log = GI->GetSubsystem<UPGXLogSubsystem>())
	{
		const FPGXLogSession Session = Log->GetCurrentSession();
		const FLinearColor ValueColor = Session.ErrorCount > 0
			? PGX::Semantic::Error
			: PGX::Text::Secondary;
		AddInstanceRow(
			FName(TEXT("PGXLog")),
			LOCTEXT("LogLabel", "Log"),
			FString::Printf(TEXT("%d entries | %d warnings | %d errors"),
				Session.EntryCount, Session.WarningCount, Session.ErrorCount),
			ValueColor);
	}

	// EN: Save stats / ES: Estadisticas de Save
	if (UPGXSaveSubsystem* Save = GI->GetSubsystem<UPGXSaveSubsystem>())
	{
		AddInstanceRow(
			FName(TEXT("PGXSave")),
			LOCTEXT("SaveLabel", "Save"),
			FString::Printf(TEXT("%d contexts registered"), Save->GetContextCount()),
			PGX::Text::Secondary);
	}

	// EN: GameFlow stats / ES: Estadisticas de GameFlow
	if (UPGXGameFlowSubsystem* GameFlow = GI->GetSubsystem<UPGXGameFlowSubsystem>())
	{
		const FGameplayTag CurrentTag = GameFlow->GetCurrentFlowTag(EPGXFlowChannel::Global);
		AddInstanceRow(
			FName(TEXT("PGXGameFlow")),
			LOCTEXT("GameFlowLabel", "GameFlow"),
			FString::Printf(TEXT("%d channels, Global: %s"),
				PGX_FLOW_CHANNEL_COUNT, CurrentTag.IsValid() ? *CurrentTag.ToString() : TEXT("(none)")),
			PGX::Text::Secondary);
	}

	// EN: PSO stats / ES: Estadisticas de PSO
	if (UPGXPSOSubsystem* PSO = GI->GetSubsystem<UPGXPSOSubsystem>())
	{
		const FPGXPSOWarmUpProgress Progress = PSO->GetWarmUpProgress();
		const FLinearColor PSOColor = (Progress.CompletedEntries >= Progress.TotalEntries && Progress.TotalEntries > 0)
			? PGX::Semantic::Good
			: PGX::Semantic::Warn;
		AddInstanceRow(
			FName(TEXT("PGXPSO")),
			LOCTEXT("PSOLabel", "PSO"),
			FString::Printf(TEXT("%d/%d compiled, state: %s"),
				Progress.CompletedEntries, Progress.TotalEntries,
				*UEnum::GetValueAsString(PSO->GetWarmUpState())),
			PSOColor);
	}

	// EN: Path + Trace infrastructure / ES: Infraestructura Path + Trace
	AddInstanceRow(
		FName(TEXT("PathResolver")),
		LOCTEXT("PathLabel", "PathCache"),
		FString::Printf(TEXT("%d entries"), FPGXPathResolver::GetCacheSize()),
		PGX::Text::Secondary);

	AddInstanceRow(
		FName(TEXT("TraceHelper")),
		LOCTEXT("TraceLabel", "TraceSystems"),
		FString::Printf(TEXT("%d registered"), FPGXTraceHelper::GetRegisteredSystemCount()),
		PGX::Text::Secondary);
}

void SPGXSystemObserverTab::CollectPlugins()
{
	PluginSnapshots.Empty();

	// EN: All L2 plugins in PGX (must match actual .uplugin files)
	// ES: Todos los plugins L2 en PGX (debe coincidir con archivos .uplugin reales)
	struct FPluginDef { const TCHAR* Display; const TCHAR* Module; };
	static const FPluginDef Plugins[] = {
		{ TEXT("PGXSave"),         TEXT("PGXSaveRuntime") },
		{ TEXT("PGXGameFlow"),     TEXT("PGXGameFlowRuntime") },
		{ TEXT("PGXPSO"),          TEXT("PGXPSORuntime") },
		{ TEXT("PGXLoading"),      TEXT("PGXLoadingRuntime") },
		{ TEXT("PGXMGOS"),         TEXT("PGXMGOSRuntime") },
		{ TEXT("PGXAudio"),        TEXT("PGXAudioRuntime") },
		{ TEXT("PGXEnvironment"),  TEXT("PGXEnvironmentRuntime") },
		{ TEXT("PGXTrade"),        TEXT("PGXTradeRuntime") },
		{ TEXT("PGXCrafting"),     TEXT("PGXCraftingRuntime") },
		{ TEXT("PGXVehicles"),     TEXT("PGXVehiclesRuntime") },
		{ TEXT("PGXColony"),       TEXT("PGXColonyRuntime") },
		{ TEXT("PGXInput"),        TEXT("PGXInputRuntime") },
		{ TEXT("PGXCamera"),       TEXT("PGXCameraRuntime") },
		{ TEXT("PGXUI"),           TEXT("PGXUIRuntime") },
		{ TEXT("PGXInteraction"),  TEXT("PGXInteractionRuntime") },
		{ TEXT("PGXInventory"),    TEXT("PGXInventoryRuntime") },
		{ TEXT("PGXAbility"),      TEXT("PGXAbilityRuntime") },
		{ TEXT("PGXSpawn"),        TEXT("PGXSpawnRuntime") },
		{ TEXT("PGXAI"),           TEXT("PGXAIRuntime") },
		{ TEXT("PGXOnline"),       TEXT("PGXOnlineRuntime") },
		{ TEXT("PGXMultiplayer"),   TEXT("PGXMultiplayerRuntime") },
		{ TEXT("PGXCinematic"),    TEXT("PGXCinematicRuntime") },
		{ TEXT("PGXAnimation"),    TEXT("PGXAnimationRuntime") },
		{ TEXT("PGXVFX"),          TEXT("PGXVFXRuntime") },
		{ TEXT("PGXMaterials"),    TEXT("PGXMaterialsRuntime") },
		{ TEXT("PGXEditorTools"),  TEXT("PGXEditorToolsEditor") },
	};

	for (const auto& Def : Plugins)
	{
		FPGXPluginSnapshot Snap;
		Snap.PluginName = Def.Display;
		Snap.ModuleName = Def.Module;
		Snap.bModuleLoaded = FModuleManager::Get().IsModuleLoaded(Def.Module);
		Snap.Health = Snap.bModuleLoaded ? EPGXSystemHealth::Active : EPGXSystemHealth::Inactive;
		PluginSnapshots.Add(MoveTemp(Snap));
	}

	// EN: Update plugin grid UI / ES: Actualizar UI de grid de plugins
	if (PluginGrid.IsValid())
	{
		PluginGrid->ClearChildren();

		for (const FPGXPluginSnapshot& Snap : PluginSnapshots)
		{
			const FLinearColor Color = GetHealthColor(Snap.Health);
			const FString Icon = Snap.bModuleLoaded ? TEXT("[OK]") : TEXT("[--]");

			PluginGrid->AddSlot()
			.Padding(PGX::Spacing::SM, PGX::Spacing::XS)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(STextBlock)
					.Text(FText::FromString(Icon))
					.ColorAndOpacity(Color)
					.Font(PGX::Font::BodySmall())
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(PGX::Spacing::SM, 0.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromName(Snap.PluginName))
					.Font(PGX::Font::BodySmall())
					.MinDesiredWidth(120.0f)
				]
			];
		}
	}
}

void SPGXSystemObserverTab::RefreshAllData()
{
	CollectCoreClasses();
	CollectSubsystems();
	CollectActiveInstances();
	CollectPlugins();
	CollectDataRegistry();

	// EN: Change 5 -- Update KPI chip counter in toolbar
	// ES: Cambio 5 -- Actualizar KPI chip contador en toolbar
	{
		int32 ActiveCount = 0;
		for (const auto& Snap : SubsystemSnapshots)
		{
			if (Snap->Health == EPGXSystemHealth::Active)
			{
				ActiveCount++;
			}

			// EN: Push metric data to per-subsystem sparklines
			// ES: Empujar datos de metrica a sparklines por subsistema
			if (TSharedPtr<SPGXSparkline>* SparkPtr = PerSubsystemSparklines.Find(Snap->SubsystemName))
			{
				if (SparkPtr->IsValid())
				{
					(*SparkPtr)->PushValue(Snap->MetricValue);
				}
			}
		}

		// EN: Push active count to global telemetry graph
		// ES: Empujar conteo activo al grafo de telemetria global
		if (ActiveSubsystemGraph.IsValid())
		{
			ActiveSubsystemGraph->PushValue(static_cast<float>(ActiveCount));
		}

		CachedSubsystemCount = FText::FromString(
			FString::Printf(TEXT("%d/%d active"), ActiveCount, SubsystemSnapshots.Num()));
		CachedFooterText = FText::FromString(
			FString::Printf(TEXT("%d subsystems | %d active"), SubsystemSnapshots.Num(), ActiveCount));
	}
}

TSharedRef<SWidget> SPGXSystemObserverTab::BuildDataRegistrySection()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("DataRegistry", "DATA REGISTRY"))
			.AccentColor(PGX::System::DataRegistry)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(PGX::Spacing::MD, PGX::Spacing::SM)
		[
			SAssignNew(DataRegistryBox, SVerticalBox)

			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SPGXEmptyStateV2)
				.Message(LOCTEXT("RegistryWaiting", "Waiting for PIE..."))
				.Hint(LOCTEXT("RegistryWaitingHint", "Start PIE to see registered databases"))
			]
		];
}

void SPGXSystemObserverTab::CollectDataRegistry()
{
	if (!DataRegistryBox.IsValid())
	{
		return;
	}

	DataRegistryBox->ClearChildren();

	UPGXDataRegistrySubsystem* Registry = bIsPIEActive ? UPGXDataRegistrySubsystem::GetCached() : nullptr;
	if (!Registry)
	{
		DataRegistryBox->AddSlot()
		.AutoHeight()
		[
			SNew(SPGXEmptyStateV2)
			.Message(bIsPIEActive
				? LOCTEXT("RegistryUnavailable", "Data Registry subsystem not available")
				: LOCTEXT("RegistryNoPIE", "Waiting for PIE..."))
			.Hint(bIsPIEActive
				? LOCTEXT("RegistryUnavailableHint", "Check subsystem initialization")
				: LOCTEXT("RegistryNoPIEHint", "Start PIE to access the Data Registry"))
		];
		return;
	}

	TArray<FGameplayTag> DatabaseTags = Registry->GetAllDatabaseTags();

	if (DatabaseTags.Num() == 0)
	{
		DataRegistryBox->AddSlot()
		.AutoHeight()
		[
			SNew(SPGXEmptyStateV2)
			.Message(LOCTEXT("RegistryEmpty", "No databases registered"))
			.Hint(LOCTEXT("RegistryEmptyHint", "Create a UPGXRegistryDefinition asset to register databases"))
		];
		return;
	}

	// EN: Header row / ES: Fila de encabezado
	DataRegistryBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 2.0f)
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.FillWidth(0.4f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("RegColTag", "Database"))
			.Font(PGX::Font::Badge())
		]

		+ SHorizontalBox::Slot()
		.FillWidth(0.15f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("RegColEntries", "Entries"))
			.Font(PGX::Font::Badge())
		]

		+ SHorizontalBox::Slot()
		.FillWidth(0.15f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("RegColCached", "Cached"))
			.Font(PGX::Font::Badge())
		]

		+ SHorizontalBox::Slot()
		.FillWidth(0.3f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("RegColClass", "Asset Class"))
			.Font(PGX::Font::Badge())
		]
	];

	for (const FGameplayTag& DbTag : DatabaseTags)
	{
		FPGXDatabaseStats Stats = Registry->GetDatabaseStats(DbTag);

		DataRegistryBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 1.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(0.4f)
			.VAlign(VAlign_Center)
			[
				// EN: Change 4 -- Leaf tag name with full tag tooltip
				// ES: Cambio 4 -- Nombre leaf del tag con tooltip de tag completo
				SNew(STextBlock)
				.Text(FText::FromString(PGXEditorUtils::TagToLeafName(DbTag)))
				.ToolTipText(FText::FromString(DbTag.ToString()))
				.ColorAndOpacity(PGX::System::DataRegistry)
			]

			+ SHorizontalBox::Slot()
			.FillWidth(0.15f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::AsNumber(Stats.TotalEntries))
			]

			+ SHorizontalBox::Slot()
			.FillWidth(0.15f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::AsNumber(Stats.LoadedEntries))
				.ColorAndOpacity(Stats.LoadedEntries > 0
					? PGX::Semantic::Good
					: PGX::Text::Secondary)
			]

			+ SHorizontalBox::Slot()
			.FillWidth(0.3f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromName(Stats.AssetClassName))
				.ColorAndOpacity(PGX::Text::Secondary)
			]
		];
	}
}

// ============================================================================
// EN: PIE Lifecycle / ES: Ciclo de Vida PIE
// ============================================================================

void SPGXSystemObserverTab::BindPIEDelegates()
{
	PIEStartedHandle = FEditorDelegates::PostPIEStarted.AddSP(
		SharedThis(this), &SPGXSystemObserverTab::OnPIEStarted);
	PIEEndedHandle = FEditorDelegates::EndPIE.AddSP(
		SharedThis(this), &SPGXSystemObserverTab::OnPIEEnded);

	PGX_LOG_INFO(LogPGXObserver, TEXT("[Observer] PIE delegates bound"));
}

void SPGXSystemObserverTab::OnPIEStarted(bool /*bIsSimulating*/)
{
	bIsPIEActive = true;
	PGX_LOG_INFO(LogPGXObserver, TEXT("[Observer] PIE started -- switching to LIVE mode"));

	if (StatusText.IsValid())
	{
		StatusText->SetText(LOCTEXT("StatusLive", "LIVE -- Polling every 1s"));
		StatusText->SetColorAndOpacity(PGX::Semantic::Good);
	}

	RefreshAllData();
	StartPolling();
}

void SPGXSystemObserverTab::OnPIEEnded(bool /*bIsSimulating*/)
{
	bIsPIEActive = false;
	StopPolling();
	PGX_LOG_INFO(LogPGXObserver, TEXT("[Observer] PIE ended -- retaining last snapshot"));

	if (StatusText.IsValid())
	{
		StatusText->SetText(LOCTEXT("StatusRetained", "PIE Ended -- Last snapshot retained"));
		StatusText->SetColorAndOpacity(PGX::Semantic::Warn);
	}

	// EN: Update KPI to show disconnected state
	// ES: Actualizar KPI para mostrar estado desconectado
	CachedSubsystemCount = LOCTEXT("SubCountDisconnected", "--/-- (offline)");
}

void SPGXSystemObserverTab::StartPolling()
{
	StopPolling();

	// EN: Poll every 1 second / ES: Polling cada 1 segundo
	PollingHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateSP(this, &SPGXSystemObserverTab::OnPollingTick),
		1.0f // interval
	);
}

void SPGXSystemObserverTab::StopPolling()
{
	if (PollingHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(PollingHandle);
		PollingHandle.Reset();
	}
}

bool SPGXSystemObserverTab::OnPollingTick(float /*DeltaTime*/)
{
	if (bIsPIEActive)
	{
		RefreshAllData();
	}
	return true; // EN: Continue ticking / ES: Continuar ticking
}

// ============================================================================
// EN: Actions / ES: Acciones
// ============================================================================

FReply SPGXSystemObserverTab::OnRefreshClicked()
{
	PGX_LOG_INFO(LogPGXObserver, TEXT("[Observer] Manual refresh triggered"));
	RefreshAllData();
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
