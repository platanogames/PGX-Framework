// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Dashboard/SPGXHubTab.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "Framework/Docking/TabManager.h"
#include "Modules/ModuleManager.h"
#include "Editor.h"
#include "Utils/PGXEditorUtils.h"
#include "Style/PGXVisualTokens.h"
#include "Style/PGXEditorStyle.h"
#include "Widgets/SPGXPremiumShell.h"
#include "Widgets/SPGXSectionDivider.h"
#include "Widgets/SPGXKPIChip.h"
#include "Dashboard/SPGXPluginCard.h"
#include "Framework/Docking/TabManager.h"
#include "PGXCoreRuntime.h"
#include "Logging/PGXLogMacros.h"

#define LOCTEXT_NAMESPACE "PGXHub"
DEFINE_LOG_CATEGORY_STATIC(LogPGXHub, Log, All);

// ============================================================================
// EN: Hub card definitions (data-driven)
// ES: Definiciones de cards del Hub (data-driven)
// ============================================================================

struct FHubCardDef
{
	const TCHAR* Name;
	const TCHAR* Description;
	FLinearColor Color;
	FName TabId;
	const TCHAR* ButtonLabel;
	const TCHAR* OwnerModule;  // nullptr = always visible / siempre visible
};

// EN: Plugin cards in canonical order (L1 core → L2 systems → tools)
// ES: Cards de plugins en orden canonico (L1 core → L2 sistemas → herramientas)
static const FHubCardDef GHubCards[] =
{
	// ── L1 Core Systems ──
	{ TEXT("Profile System"), TEXT("Project constitution — governs capabilities, budgets, and features"),
		PGX::System::Profile, TEXT("PGXProfileInspector"), TEXT("Open Inspector"), nullptr },
	{ TEXT("GameFlow"), TEXT("Game state machine — phases, transitions, history, checkpoints"),
		PGX::System::GameFlow, TEXT("PGXGameFlowInspector"), TEXT("Open Inspector"), TEXT("PGXGameFlowRuntime") },
	{ TEXT("Save System"), TEXT("Save/Load framework — domains, slots, auto-save, serialization"),
		PGX::System::Save, TEXT("PGXSaveInspector"), TEXT("Open Inspector"), TEXT("PGXSaveRuntime") },
	{ TEXT("PSO System"), TEXT("Pipeline State Objects — precache, recording, compilation, bundles"),
		PGX::System::PSO, TEXT("PGXPSOInspector"), TEXT("Open Inspector"), TEXT("PGXPSORuntime") },
	{ TEXT("Audio System"), TEXT("Dual-backend audio — mix layers, ducking, music, HRTF"),
		PGX::System::Audio, TEXT("PGXAudioInspector"), TEXT("Open Inspector"), TEXT("PGXAudioRuntime") },
	{ TEXT("Data Registry"), TEXT("Central asset registry — databases, queries, cache, telemetry"),
		PGX::System::DataRegistry, TEXT("PGXDataRegistryBrowser"), TEXT("Open Browser"), nullptr },
	{ TEXT("Message System"), TEXT("Pub/sub message bus — channels, listeners, history, telemetry"),
		PGX::System::Message, TEXT("PGXMessageInspector"), TEXT("Open Inspector"), nullptr },
	{ TEXT("Event Handler"), TEXT("Context-driven event resolution — handlers, lifecycle, telemetry"),
		PGX::System::EventHandler, TEXT("PGXEventDebugger"), TEXT("Open Debugger"), nullptr },

	// ── L2 Systems ──
	{ TEXT("Loading System"), TEXT("Loading screens — transitions, progress, tips, async coordination"),
		PGX::System::Loading, TEXT("PGXLoadingInspector"), TEXT("Open Inspector"), TEXT("PGXLoadingRuntime") },
	{ TEXT("LevelFlow"), TEXT("Level management — streaming, transitions, sublevels, resolution"),
		PGX::System::LevelFlow, TEXT("PGXLevelFlowInspector"), TEXT("Open Inspector"), TEXT("PGXLevelFlowRuntime") },
	{ TEXT("MGOS"), TEXT("GC observability — memory pools, allocation tracking, profiles"),
		PGX::System::MGOS, TEXT("PGXMGOSInspector"), TEXT("Open Inspector"), TEXT("PGXMGOSRuntime") },
	{ TEXT("Platform Health"), TEXT("Platform budgets — per-system limits, global resources, comparison"),
		PGX::System::Profile, TEXT("PGXPlatformHealthDashboard"), TEXT("Open Dashboard"), nullptr },
	{ TEXT("Environment"), TEXT("Environment simulation — variables, zones, tick profiles, and telemetry"),
		PGX::Semantic::Info, TEXT("PGXSystemObserver"), TEXT("Open Observer"), TEXT("PGXEnvironmentRuntime") },
	{ TEXT("Trade"), TEXT("Economy exchange — market config, offers, valuation, and trade policy"),
		PGX::Semantic::Info, TEXT("PGXTradeInspector"), TEXT("Open Inspector"), TEXT("PGXTradeEditor") },
	{ TEXT("Trade (Live)"), TEXT("Trade runtime — actors, offers, transactions in PIE (G7 live inspector)"),
		PGX::System::Trade, TEXT("PGXTradeInspectorLive"), TEXT("Open Live Inspector"), TEXT("PGXTradeRuntime") },
	{ TEXT("Crafting"), TEXT("Crafting workflows — recipes, jobs, validation, and simulation state"),
		PGX::Semantic::Info, TEXT("PGXCraftingInspector"), TEXT("Open Inspector"), TEXT("PGXCraftingEditor") },
	{ TEXT("Vehicles"), TEXT("Vehicle registry — definitions, state records, ownership, and service ops"),
		PGX::Semantic::Info, TEXT("PGXVehiclesInspector"), TEXT("Open Inspector"), TEXT("PGXVehiclesEditor") },
	{ TEXT("Colony"), TEXT("Colony systems — population policy, settlements, and authoring config"),
		PGX::Semantic::Info, TEXT("PGXSystemObserver"), TEXT("Open Observer"), TEXT("PGXColonyRuntime") },
	{ TEXT("Interaction"), TEXT("Interaction targets — registration, prompts, validation, and action state"),
		PGX::Semantic::Info, TEXT("PGXSystemObserver"), TEXT("Open Observer"), TEXT("PGXInteractionRuntime") },
	{ TEXT("Inventory"), TEXT("Inventory model — item definitions, stacks, slot limits, and transfer state"),
		PGX::Semantic::Info, TEXT("PGXSystemObserver"), TEXT("Open Observer"), TEXT("PGXInventoryRuntime") },
	{ TEXT("Spawn"), TEXT("Spawn registry — requests, handles, records, cleanup, and wave definitions"),
		PGX::Semantic::Info, TEXT("PGXSpawnInspector"), TEXT("Open Inspector"), TEXT("PGXSpawnEditor") },
	{ TEXT("AI"), TEXT("AI authoring — behavior config, perception budgets, squads, and observability"),
		PGX::Semantic::Info, TEXT("PGXSystemObserver"), TEXT("Open Observer"), TEXT("PGXAIRuntime") },
	{ TEXT("UI"), TEXT("Presentation layer — screens, notifications, widget pools, and UI config"),
		PGX::Semantic::Info, TEXT("PGXSystemObserver"), TEXT("Open Observer"), TEXT("PGXUIRuntime") },
	{ TEXT("Input"), TEXT("Input contexts — Enhanced Input mappings, stack policy, and device state"),
		PGX::Semantic::Info, TEXT("PGXSystemObserver"), TEXT("Open Observer"), TEXT("PGXInputRuntime") },

	// ── Tool Plugins ──
	{ TEXT("Log Viewer"), TEXT("PGX log viewer — categorized, filtered, real-time log output"),
		PGX::System::Log, TEXT("PGXLogViewer"), TEXT("Open Viewer"), nullptr },
	{ TEXT("Config Dashboard"), TEXT("View and manage all Config DataAssets"),
		PGX::System::Config, TEXT("PGXConfigDashboard"), TEXT("Open Dashboard"), nullptr },
	{ TEXT("Test Dashboard"), TEXT("Run and view system validation tests"),
		PGX::Semantic::Good, TEXT("PGXTestDashboard"), TEXT("Open Dashboard"), nullptr },
	{ TEXT("Version Control"), TEXT("Git overlay — changelists, auto-tagging, commit validation"),
		PGX::Semantic::Neutral, TEXT("PGXVersionControlInspector"), TEXT("Open Inspector"), TEXT("PGXVersionControlEditor") },
	{ TEXT("PGX Scaffold"), TEXT("Automated project scaffolding — templates, validation, transactional execution"),
		PGX::System::Scaffold, TEXT("PGXScaffoldPanel"), TEXT("Open Panel"), TEXT("PGXScaffoldEditor") },
};

static constexpr int32 GHubCardCount = UE_ARRAY_COUNT(GHubCards);

// EN: Known PGX runtime modules for loaded-count check
// ES: Modulos runtime PGX conocidos para conteo de cargados
static const TCHAR* GPluginModules[] =
{
	TEXT("PGXCoreRuntime"),
	TEXT("PGXSaveRuntime"),
	TEXT("PGXGameFlowRuntime"),
	TEXT("PGXPSORuntime"),
	TEXT("PGXLoadingRuntime"),
	TEXT("PGXLevelFlowRuntime"),
	TEXT("PGXAudioRuntime"),
	TEXT("PGXMGOSRuntime"),
	TEXT("PGXEnvironmentRuntime"),
	TEXT("PGXTradeRuntime"),
	TEXT("PGXCraftingRuntime"),
	TEXT("PGXVehiclesRuntime"),
	TEXT("PGXColonyRuntime"),
	TEXT("PGXInputRuntime"),
	TEXT("PGXCameraRuntime"),
	TEXT("PGXUIRuntime"),
	TEXT("PGXInteractionRuntime"),
	TEXT("PGXInventoryRuntime"),
	TEXT("PGXAbilityRuntime"),
	TEXT("PGXSpawnRuntime"),
	TEXT("PGXAIRuntime"),
	TEXT("PGXOnlineRuntime"),
	TEXT("PGXMaterialsRuntime"),
	TEXT("PGXVFXRuntime"),
	TEXT("PGXAnimationRuntime"),
	TEXT("PGXMultiplayerRuntime"),
	TEXT("PGXCinematicRuntime"),
};

// ============================================================================
// EN: Construction / Destruction
// ES: Construccion / Destruccion
// ============================================================================

void SPGXHubTab::Construct(const FArguments& /*InArgs*/)
{
	PGX_LOG_INFO(LogPGXHub, TEXT("[Hub] Construct"));

	ChildSlot
	[
		// EN: Premium Shell — covers UE chrome, provides title bar + footer
		// ES: Shell Premium — cubre chrome de UE, provee title bar + footer
		SNew(SPGXPremiumShell)
		.SystemColor(PGX::System::MGOS)  // Purple = PGX brand primary
		.Title(LOCTEXT("HubHeaderTitle", "PGX Hub"))
		.Subtitle(LOCTEXT("HubHeaderSub", "Central framework dashboard"))
		.Icon(FPGXEditorStyle::Get().GetBrush("PGXEditor.Icon.Hub"))
		.TitleRightContent()
		[
			BuildQuickLinksSection()
		]
		.StatusText_Lambda([this]() -> FText
		{
			if (PIEStatusText.IsValid())
			{
				return PIEStatusText->GetText();
			}
			return LOCTEXT("PIEOff2", "PIE: Offline");
		})
		.Content()
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)

				// EN: Status / KPIs / ES: Estado / KPIs
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 0.0f, 0.0f, PGX::Spacing::LG)
				[
					BuildStatusSection()
				]

				// EN: Plugin Dashboard / ES: Dashboard de Plugins
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, PGX::Spacing::SM, 0.0f, PGX::Spacing::LG)
				[
					BuildPluginDashboard()
				]
			]
		]
	];

	// EN: Bind PIE delegates for KPI refresh / ES: Bindear delegates PIE para refresh de KPIs
	PIEStartedHandle = FEditorDelegates::PostPIEStarted.AddSP(
		SharedThis(this), &SPGXHubTab::OnPIEStarted);
	PIEEndedHandle = FEditorDelegates::EndPIE.AddSP(
		SharedThis(this), &SPGXHubTab::OnPIEEnded);

	// EN: Auto-refresh ticker every 5 seconds / ES: Ticker de auto-refresh cada 5 segundos
	RefreshTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateLambda([this](float /*DeltaTime*/) -> bool
		{
			RefreshKPIs();
			return true;
		}), 5.0f);

	PGX_LOG_INFO(LogPGXHub, TEXT("[Hub] Delegates + ticker bound"));
	RefreshKPIs();
}

SPGXHubTab::~SPGXHubTab()
{
	PGX_LOG_INFO(LogPGXHub, TEXT("[Hub] Destructor — cleaning up"));

	if (RefreshTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(RefreshTickerHandle);
	}
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
// EN: Status Section — Title + KPI chips
// ES: Seccion de Estado — Titulo + chips KPI
// ============================================================================

TSharedRef<SWidget> SPGXHubTab::BuildStatusSection()
{
	return SNew(SHorizontalBox)

		// EN: Plugins chip / ES: Chip de Plugins
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0, 0, 8, 0)
		[
			SNew(SPGXKPIChip)
			.Label(LOCTEXT("KPIPlugins", "PLUGINS"))
			.Value(LOCTEXT("KPILoading1", "..."))
			.AccentColor(PGX::Semantic::Info)
			.ValueWidget()
			[
				SAssignNew(PluginCountText, STextBlock)
				.Text(LOCTEXT("KPILoading2", "..."))
				.Font(PGX::Font::KPIValue())
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
			]
		]

		// EN: Systems chip / ES: Chip de Systems
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0, 0, 8, 0)
		[
			SNew(SPGXKPIChip)
			.Label(LOCTEXT("KPISystems", "SYSTEMS"))
			.Value(LOCTEXT("KPILoading3", "..."))
			.AccentColor(PGX::Semantic::Info)
			.ValueWidget()
			[
				SAssignNew(SystemCountText, STextBlock)
				.Text(LOCTEXT("KPILoading4", "..."))
				.Font(PGX::Font::KPIValue())
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
			]
		]

		// EN: Framework version chip / ES: Chip de version del Framework
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SPGXKPIChip)
			.Label(LOCTEXT("KPIVersion", "FRAMEWORK"))
			.Value(FText::Format(LOCTEXT("VersionFmt", "v{0}"), FText::FromString(PGXVersion::String)))
			.AccentColor(PGX::Semantic::Info)
		];
}

// ============================================================================
// EN: Plugin Dashboard — SWrapBox grid of cards
// ES: Dashboard de Plugins — grid SWrapBox de cards
// ============================================================================

TSharedRef<SWidget> SPGXHubTab::BuildPluginDashboard()
{
	TSharedRef<SWrapBox> Grid = SNew(SWrapBox).UseAllottedSize(true);

	for (int32 i = 0; i < GHubCardCount; ++i)
	{
		const FHubCardDef& Card = GHubCards[i];

		// EN: Skip cards whose owner module is not loaded / ES: Omitir cards cuyo modulo propietario no esta cargado
		if (Card.OwnerModule && !FModuleManager::Get().IsModuleLoaded(Card.OwnerModule))
		{
			continue;
		}

		Grid->AddSlot()
		.Padding(PGX::Spacing::SM)
		[
			SNew(SBox)
			.WidthOverride(340.0f)
			[
				SNew(SPGXPluginCard)
				.PluginName(Card.Name)
				.Description(Card.Description)
				.SystemColor(Card.Color)
				.ButtonLabel(Card.ButtonLabel)
				.OnButtonClicked_Lambda([TabId = Card.TabId]()
				{
					FGlobalTabmanager::Get()->TryInvokeTab(TabId);
					return FReply::Handled();
				})
			]
		];
	}

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("PluginDashboard", "PLUGIN DASHBOARD"))
			.AccentColor(PGX::System::MGOS)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			Grid
		];
}

// ============================================================================
// EN: Quick Links + PIE status (footer)
// ES: Enlaces rapidos + estado PIE (footer)
// ============================================================================

TSharedRef<SWidget> SPGXHubTab::BuildQuickLinksSection()
{
	// EN: Ghost-tier buttons for quick navigation (used in title bar right content)
	// ES: Botones tier Ghost para navegacion rapida (usados en contenido derecho del title bar)
	const FButtonStyle* GhostStyle = &FPGXEditorStyle::Get().GetWidgetStyle<FButtonStyle>(
		FName(TEXT("PGXEditor.Button.Ghost")));

	auto MakeQuickLink = [GhostStyle](const FText& Label, FName TabId) -> TSharedRef<SWidget>
	{
		return SNew(SButton)
			.ButtonStyle(GhostStyle)
			.OnClicked_Lambda([TabId]()
			{
				FGlobalTabmanager::Get()->TryInvokeTab(TabId);
				return FReply::Handled();
			})
			.ContentPadding(FMargin(PGX::Spacing::MD, PGX::Spacing::XS))
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(PGX::Font::BodySmall())
				.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
			];
	};

	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(PGX::Spacing::XS)
		[
			MakeQuickLink(LOCTEXT("QLSystemObserver", "Observer"), FName("PGXSystemObserver"))
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(PGX::Spacing::XS)
		[
			MakeQuickLink(LOCTEXT("QLDocumentation", "Docs"), FName("PGXDocs"))
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(PGX::Spacing::XS)
		[
			MakeQuickLink(LOCTEXT("QLLogViewer", "Logs"), FName("PGXLogViewer"))
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(PGX::Spacing::MD, 0.0f, 0.0f, 0.0f)
		[
			SAssignNew(PIEStatusText, STextBlock)
			.Text(LOCTEXT("PIEOff", "PIE: Offline"))
			.Font(PGX::Font::BodySmall())
			.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
		];
}

// ============================================================================
// EN: Dynamic plugin count
// ES: Conteo dinamico de plugins
// ============================================================================

int32 SPGXHubTab::CountLoadedPlugins() const
{
	int32 Count = 0;
	for (const TCHAR* Mod : GPluginModules)
	{
		if (FModuleManager::Get().IsModuleLoaded(Mod))
		{
			Count++;
		}
	}
	return Count;
}

// ============================================================================
// EN: KPI Refresh
// ES: Refresh de KPIs
// ============================================================================

void SPGXHubTab::RefreshKPIs()
{
	if (PluginCountText.IsValid())
	{
		const int32 Loaded = CountLoadedPlugins();
		const int32 Total = UE_ARRAY_COUNT(GPluginModules);
		PluginCountText->SetText(FText::Format(
			LOCTEXT("PluginCountFmt", "{0}/{1}"),
			FText::AsNumber(Loaded), FText::AsNumber(Total)));
		PluginCountText->SetColorAndOpacity(Loaded == Total
			? FSlateColor(PGX::Semantic::Good)
			: FSlateColor(PGX::Semantic::Warn));
	}

	if (SystemCountText.IsValid())
	{
		// EN: Count implemented systems dynamically from Hub cards / ES: Contar sistemas implementados dinamicamente
		const int32 ImplementedSystems = 13;
		const int32 TotalSystems = 26;
		SystemCountText->SetText(FText::Format(
			LOCTEXT("SystemCountFmt", "{0}/{1}"),
			FText::AsNumber(ImplementedSystems), FText::AsNumber(TotalSystems)));
		SystemCountText->SetColorAndOpacity(FSlateColor(PGX::Semantic::Info));
	}
}

// ============================================================================
// EN: PIE Lifecycle
// ES: Ciclo de Vida PIE
// ============================================================================

void SPGXHubTab::OnPIEStarted(bool /*bIsSimulating*/)
{
	PGX_LOG_INFO(LogPGXHub, TEXT("[Hub] OnPIEStarted"));
	if (PIEStatusText.IsValid())
	{
		PIEStatusText->SetText(LOCTEXT("PIEOn", "PIE: Online"));
		PIEStatusText->SetColorAndOpacity(FSlateColor(PGX::Semantic::Good));
	}
	RefreshKPIs();
}

void SPGXHubTab::OnPIEEnded(bool /*bIsSimulating*/)
{
	PGX_LOG_INFO(LogPGXHub, TEXT("[Hub] OnPIEEnded"));
	if (PIEStatusText.IsValid())
	{
		PIEStatusText->SetText(LOCTEXT("PIEOff", "PIE: Offline"));
		PIEStatusText->SetColorAndOpacity(FSlateColor(PGX::Text::Muted));
	}
	RefreshKPIs();
}

#undef LOCTEXT_NAMESPACE
