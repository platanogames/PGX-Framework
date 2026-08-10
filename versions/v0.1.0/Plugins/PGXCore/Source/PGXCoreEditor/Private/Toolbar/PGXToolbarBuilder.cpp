// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Toolbar/PGXToolbarBuilder.h"
#include "Toolbar/PGXMenuCommands.h"
#include "Notifications/PGXEditorNotification.h"
#include "AssetTools/PGXAssetCreationRegistry.h"
#include "AssetTools/PGXBlueprintCreator.h"
#include "Settings/PGXEditorSettings.h"
#include "Validation/PGXFrameworkValidator.h"
#include "Style/PGXEditorStyle.h"
#include "ISettingsModule.h"
#include "ToolMenus.h"
#include "Modules/ModuleManager.h"
#include "Styling/AppStyle.h"
#include "Framework/Docking/TabManager.h"
#include "UnrealEdMisc.h"
#include "FileHelpers.h"
#include "Editor.h"
#include "Misc/MessageDialog.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "PGXToolbar"

// EN: Stored delegate handles for proper cleanup / ES: Handles de delegado almacenados para limpieza correcta
static FDelegateHandle GToolbarStartupHandle;
static FDelegateHandle GQuickAccessStartupHandle;

// EN: Helper — shortcut for PGX custom style icons / ES: Atajo para iconos PGX personalizados
static FSlateIcon PGXIcon(const FName& IconName)
{
	return FSlateIcon(FPGXEditorStyle::GetStyleSetName(), IconName);
}

void FPGXToolbarBuilder::RegisterToolbar()
{
	GToolbarStartupHandle = UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateLambda([]()
	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.User");
		FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PGX");

		// EN: Toolbar button — "PGX" text label with dropdown arrow (no gear icon)
		// ES: Boton toolbar — texto "PGX" con flecha dropdown (sin icono de tuerca)
		FToolMenuEntry& MenuEntry = Section.AddEntry(FToolMenuEntry::InitComboButton(
			"PGXMenu",
			FUIAction(),
			FNewToolMenuDelegate::CreateStatic(&FPGXToolbarBuilder::BuildMenu),
			LOCTEXT("PGX", "PGX"),
			LOCTEXT("PGXTooltip", "PGX Framework Tools")
		));
		MenuEntry.StyleNameOverride = "CalloutToolbar";
	}));
}

void FPGXToolbarBuilder::RegisterQuickAccessButtons()
{
	GQuickAccessStartupHandle = UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateLambda([]()
	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.User");
		FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PGXQuickAccessToolbar");
		const FName PGXStyle = FPGXEditorStyle::GetStyleSetName();

		// ── Hub — always visible (pin only controls this direct button) ──
		{
			FToolMenuEntry HubEntry = FToolMenuEntry::InitToolBarButton(
				"QA_Hub",
				FUIAction(FExecuteAction::CreateLambda([]() { FGlobalTabmanager::Get()->TryInvokeTab(FName("PGXHub")); })),
				LOCTEXT("QAHub", "Hub"),
				LOCTEXT("QAHubTip", "Open PGX Hub"),
				FSlateIcon(PGXStyle, "PGXEditor.Icon.Hub")
			);
			HubEntry.Visibility = TAttribute<EVisibility>::CreateLambda([]() -> EVisibility
			{
				return GetDefault<UPGXEditorSettings>()->bPinHub ? EVisibility::Visible : EVisibility::Collapsed;
			});
			Section.AddEntry(HubEntry);
		}

		// ── Separator ──
		Section.AddSeparator("QA_Sep_Core");

		// ── 3 Combo dropdown buttons — always visible ──
		{
			FToolMenuEntry InspEntry = FToolMenuEntry::InitComboButton(
				"QA_Inspectors",
				FUIAction(),
				FNewToolMenuDelegate::CreateStatic(&FPGXToolbarBuilder::BuildInspectorsDropdown),
				LOCTEXT("QAInspectors", "Inspectors"),
				LOCTEXT("QAInspectorsTip", "System Inspectors: Log, Save, GameFlow, Audio, MGOS, Message, Events, Docs"),
				FSlateIcon(PGXStyle, "PGXEditor.Icon.SystemObserver")
			);
			Section.AddEntry(InspEntry);

			FToolMenuEntry PipeEntry = FToolMenuEntry::InitComboButton(
				"QA_Pipeline",
				FUIAction(),
				FNewToolMenuDelegate::CreateStatic(&FPGXToolbarBuilder::BuildPipelineDropdown),
				LOCTEXT("QAPipeline", "Pipeline"),
				LOCTEXT("QAPipelineTip", "Pipeline & Health: Platform, Config, Registry, PSO, LevelFlow, Loading"),
				FSlateIcon(PGXStyle, "PGXEditor.Icon.PlatformHealth")
			);
			Section.AddEntry(PipeEntry);

			FToolMenuEntry DevEntry = FToolMenuEntry::InitComboButton(
				"QA_DevTools",
				FUIAction(),
				FNewToolMenuDelegate::CreateStatic(&FPGXToolbarBuilder::BuildDevToolsDropdown),
				LOCTEXT("QADevTools", "Dev Tools"),
				LOCTEXT("QADevToolsTip", "Development Tools: Observer, Tests, VC, PSPH, Scaffold, Tutorials, Bridge"),
				FSlateIcon(PGXStyle, "PGXEditor.Icon.Tools")
			);
			Section.AddEntry(DevEntry);
		}

		// ── Separator before pinned buttons ──
		Section.AddSeparator("QA_Sep_Combos");

		// ── Pinned buttons: tools promoted by pin=true in Settings ──
		// EN: Each tool is always in its combo dropdown. Pin=true additionally shows it as a direct toolbar button.
		// ES: Cada herramienta siempre esta en su combo. Pin=true ademas la muestra como boton directo.
		// NOTE: UE 5.6 requires TAttribute<EVisibility>::CreateLambda — plain lambda assignment does NOT bind.
		struct FPinDef
		{
			FName EntryName;
			FText Label;
			FText Tooltip;
			FName IconName;
			FName TabId;
			bool UPGXEditorSettings::* PinMember;
			const TCHAR* OwnerModule;
		};

		const TArray<FPinDef> PinnableDefs = {
			// ── Inspectors ──
			{ "QAPin_LogViewer",     LOCTEXT("QALog", "Log"),           LOCTEXT("QALogTip", "Open Log Viewer"),              "PGXEditor.Icon.LogViewer",      "PGXLogViewer",               &UPGXEditorSettings::bPinLogViewer,                 nullptr },
			{ "QAPin_SaveInspector", LOCTEXT("QASave", "Save"),         LOCTEXT("QASaveTip", "Open Save Inspector"),         "PGXEditor.Icon.SaveInspector",  "PGXSaveInspector",           &UPGXEditorSettings::bPinSaveInspector,             TEXT("PGXSaveRuntime") },
			{ "QAPin_GameFlow",      LOCTEXT("QAGameFlow", "GameFlow"), LOCTEXT("QAGameFlowTip", "Open GameFlow Inspector"), "PGXEditor.Icon.GameFlow",       "PGXGameFlowInspector",       &UPGXEditorSettings::bPinGameFlowInspector,         TEXT("PGXGameFlowRuntime") },
			{ "QAPin_Audio",         LOCTEXT("QAAudio", "Audio"),       LOCTEXT("QAAudioTip", "Open Audio Inspector"),       "PGXEditor.Icon.Audio",          "PGXAudioInspector",          &UPGXEditorSettings::bPinAudioInspector,            TEXT("PGXAudioRuntime") },
			{ "QAPin_MGOS",          LOCTEXT("QAMGOS", "MGOS"),        LOCTEXT("QAMGOSTip", "Open MGOS Inspector"),         "PGXEditor.Icon.MGOS",           "PGXMGOSInspector",           &UPGXEditorSettings::bPinMGOSInspector,             TEXT("PGXMGOSRuntime") },
			{ "QAPin_Message",       LOCTEXT("QAMessage", "Message"),   LOCTEXT("QAMessageTip", "Message Inspector"),        "PGXEditor.Icon.Message",        "PGXMessageInspector",        &UPGXEditorSettings::bPinMessageInspector,          nullptr },
			{ "QAPin_Events",        LOCTEXT("QAEvent", "Events"),      LOCTEXT("QAEventTip", "Event Debugger"),             "PGXEditor.Icon.EventHandler",   "PGXEventDebugger",           &UPGXEditorSettings::bPinEventDebugger,             nullptr },
			{ "QAPin_PSO",           LOCTEXT("QAPSO", "PSO"),           LOCTEXT("QAPSOTip", "Open PSO Inspector"),           "PGXEditor.Icon.PSO",            "PGXPSOInspector",            &UPGXEditorSettings::bPinPSOInspector,              TEXT("PGXPSORuntime") },
			{ "QAPin_LevelFlow",     LOCTEXT("QALevelFlow", "LevelFlow"),LOCTEXT("QALevelFlowTip", "LevelFlow Inspector"),  "PGXEditor.Icon.LevelFlow",      "PGXLevelFlowInspector",      &UPGXEditorSettings::bPinLevelFlowInspector,        TEXT("PGXLoadingRuntime") },
			{ "QAPin_Loading",       LOCTEXT("QALoading", "Loading"),   LOCTEXT("QALoadingTip", "Loading Inspector"),        "PGXEditor.Icon.Loading",        "PGXLoadingInspector",        &UPGXEditorSettings::bPinLoadingInspector,          TEXT("PGXLoadingRuntime") },
			{ "QAPin_Profile",       LOCTEXT("QAProfile", "Profile"),   LOCTEXT("QAProfileTip", "Profile Inspector"),        "PGXEditor.Icon.Profile",        "PGXProfileInspector",        &UPGXEditorSettings::bPinProfileInspector,          nullptr },
			{ "QAPin_Docs",          LOCTEXT("QADocs", "Docs"),         LOCTEXT("QADocsTip", "Open Documentation"),          "PGXEditor.Icon.Docs",           "PGXDocs",                    &UPGXEditorSettings::bPinDocs,                      TEXT("PGXDocsEditor") },
			// ── Pipeline ──
			{ "QAPin_Platform",      LOCTEXT("QAPlatform", "Platform"), LOCTEXT("QAPlatformTip", "Platform Health"),         "PGXEditor.Icon.PlatformHealth", "PGXPlatformHealthDashboard", &UPGXEditorSettings::bPinPlatformHealth,            nullptr },
			{ "QAPin_Config",        LOCTEXT("QAConfig", "Config"),     LOCTEXT("QAConfigTip", "Config Dashboard"),          "PGXEditor.Icon.ConfigDashboard","PGXConfigDashboard",         &UPGXEditorSettings::bPinConfigDashboard,           nullptr },
			{ "QAPin_Registry",      LOCTEXT("QADataReg", "Registry"),  LOCTEXT("QADataRegTip", "Data Registry Browser"),    "PGXEditor.Icon.DataRegistry",   "PGXDataRegistryBrowser",     &UPGXEditorSettings::bPinDataRegistryBrowser,       nullptr },
			// ── Dev Tools ──
			{ "QAPin_Observer",      LOCTEXT("QASysObs", "Observer"),   LOCTEXT("QASysObsTip", "System Observer"),           "PGXEditor.Icon.SystemObserver", "PGXSystemObserver",          &UPGXEditorSettings::bPinSystemObserver,            nullptr },
			{ "QAPin_Tests",         LOCTEXT("QATest", "Tests"),        LOCTEXT("QATestTip", "Test Dashboard"),              "PGXEditor.Icon.TestDashboard",  "PGXTestDashboard",           &UPGXEditorSettings::bPinTestDashboard,             nullptr },
			{ "QAPin_Restart",       LOCTEXT("QARestart", "Restart"),   LOCTEXT("QARestartTip", "Restart Editor"),           "PGXEditor.Icon.Restart",        NAME_None,                    &UPGXEditorSettings::bPinRestart,                   nullptr },
			{ "QAPin_VC",            LOCTEXT("QAVC", "VC"),             LOCTEXT("QAVCTip", "Version Control Inspector"),     "PGXEditor.Icon.VersionControl", "PGXVersionControlInspector", &UPGXEditorSettings::bPinVersionControlInspector,   TEXT("PGXVersionControlEditor") },
			{ "QAPin_Scaffold",      LOCTEXT("QAScaffold", "Scaffold"), LOCTEXT("QAScaffoldTip", "Scaffold Panel"),          "PGXEditor.Icon.Scaffold",       "PGXScaffoldPanel",           &UPGXEditorSettings::bPinScaffold,                  TEXT("PGXScaffoldEditor") },
			{ "QAPin_Tutorials",     LOCTEXT("QATutorials", "Tutorials"),LOCTEXT("QATutorialsTip", "Tutorial Hub"),          "PGXEditor.Icon.Tutorials",      "PGXTutorialsHub",            &UPGXEditorSettings::bPinTutorialHub,               nullptr },
		};

		for (const FPinDef& Pin : PinnableDefs)
		{
			FToolMenuEntry Entry = (Pin.TabId == NAME_None)
				? FToolMenuEntry::InitToolBarButton(Pin.EntryName,
					FUIAction(FExecuteAction::CreateStatic(&FPGXToolbarBuilder::ExecuteSafeRestart)),
					Pin.Label, Pin.Tooltip, FSlateIcon(PGXStyle, Pin.IconName))
				: FToolMenuEntry::InitToolBarButton(Pin.EntryName,
					FUIAction(FExecuteAction::CreateLambda([TabId = Pin.TabId]() { FGlobalTabmanager::Get()->TryInvokeTab(TabId); })),
					Pin.Label, Pin.Tooltip, FSlateIcon(PGXStyle, Pin.IconName));

			// EN: UE 5.6 gotcha — must use TAttribute::CreateLambda, plain lambda assignment does NOT bind
			// ES: Gotcha UE 5.6 — debe usarse TAttribute::CreateLambda, asignacion directa de lambda NO vincula
			bool UPGXEditorSettings::* MemberPtr = Pin.PinMember;
			FName ModName = Pin.OwnerModule ? FName(Pin.OwnerModule) : NAME_None;
			Entry.Visibility = TAttribute<EVisibility>::CreateLambda([MemberPtr, ModName]() -> EVisibility
			{
				if (ModName != NAME_None && !FModuleManager::Get().IsModuleLoaded(ModName)) return EVisibility::Collapsed;
				return (GetDefault<UPGXEditorSettings>()->*MemberPtr) ? EVisibility::Visible : EVisibility::Collapsed;
			});
			Section.AddEntry(Entry);
		}
	}));
}

void FPGXToolbarBuilder::UnregisterToolbar()
{
	if (GToolbarStartupHandle.IsValid())
	{
		UToolMenus::UnRegisterStartupCallback(GToolbarStartupHandle);
		GToolbarStartupHandle.Reset();
	}
	if (GQuickAccessStartupHandle.IsValid())
	{
		UToolMenus::UnRegisterStartupCallback(GQuickAccessStartupHandle);
		GQuickAccessStartupHandle.Reset();
	}
}

// ════════════════════════════════════════════════════════════════════════
// EN: Combo dropdown builders — one per Quick Access group
// ES: Builders de combo dropdown — uno por grupo de Quick Access
// ════════════════════════════════════════════════════════════════════════

void FPGXToolbarBuilder::BuildInspectorsDropdown(UToolMenu* Menu)
{
	FToolMenuSection& Section = Menu->FindOrAddSection("Inspectors");

	// EN: Helper — returns visibility that collapses when module is not loaded
	// ES: Helper — visibilidad que colapsa cuando el modulo no esta cargado
	auto ModuleVis = [](const TCHAR* Mod) -> TAttribute<EVisibility>
	{
		FName ModName(Mod);
		return TAttribute<EVisibility>::CreateLambda([ModName]() -> EVisibility
		{
			return FModuleManager::Get().IsModuleLoaded(ModName) ? EVisibility::Visible : EVisibility::Collapsed;
		});
	};

	struct FDropdownEntry { FName Id; FText Label; FText Tip; FName Icon; FName TabId; const TCHAR* OwnerModule; };
	const TArray<FDropdownEntry> Entries = {
		{ "DDI_Log",       LOCTEXT("DDILog", "Log Viewer"),         LOCTEXT("DDILogTip", "Open PGX Log Viewer"),              "PGXEditor.Icon.LogViewer",     "PGXLogViewer",         nullptr },
		{ "DDI_Save",      LOCTEXT("DDISave", "Save Inspector"),    LOCTEXT("DDISaveTip", "Open Save Inspector"),             "PGXEditor.Icon.SaveInspector", "PGXSaveInspector",     TEXT("PGXSaveRuntime") },
		{ "DDI_TradeLive", LOCTEXT("DDITradeLive", "Trade (Live)"), LOCTEXT("DDITradeLiveTip", "Open Trade Live Inspector (PIE runtime)"), "PGXEditor.Icon.Trade", "PGXTradeInspectorLive", TEXT("PGXTradeRuntime") },
		{ "DDI_GameFlow",  LOCTEXT("DDIGameFlow", "GameFlow Inspector"), LOCTEXT("DDIGameFlowTip", "Open GameFlow Inspector"), "PGXEditor.Icon.GameFlow", "PGXGameFlowInspector", TEXT("PGXGameFlowRuntime") },
		{ "DDI_Audio",     LOCTEXT("DDIAudio", "Audio Inspector"),  LOCTEXT("DDIAudioTip", "Open Audio Inspector"),           "PGXEditor.Icon.Audio",         "PGXAudioInspector",    TEXT("PGXAudioRuntime") },
		{ "DDI_MGOS",      LOCTEXT("DDIMGOS", "MGOS Inspector"),   LOCTEXT("DDIMGOSTip", "Open MGOS Inspector"),             "PGXEditor.Icon.MGOS",          "PGXMGOSInspector",     TEXT("PGXMGOSRuntime") },
		{ "DDI_Message",   LOCTEXT("DDIMessage", "Message Inspector"), LOCTEXT("DDIMessageTip", "Open Message Inspector"),    "PGXEditor.Icon.Message",       "PGXMessageInspector",  nullptr },
		{ "DDI_Events",    LOCTEXT("DDIEvents", "Event Debugger"),  LOCTEXT("DDIEventsTip", "Open Event Debugger"),           "PGXEditor.Icon.EventHandler",  "PGXEventDebugger",     nullptr },
		{ "DDI_Docs",      LOCTEXT("DDIDocs", "Documentation"),     LOCTEXT("DDIDocsTip", "Open Documentation Viewer"),       "PGXEditor.Icon.Docs",          "PGXDocs",              TEXT("PGXDocsEditor") },
	};

	for (const FDropdownEntry& E : Entries)
	{
		FToolMenuEntry& Entry = Section.AddMenuEntry(E.Id, E.Label, E.Tip, PGXIcon(E.Icon),
			FUIAction(FExecuteAction::CreateLambda([TabId = E.TabId]() { FGlobalTabmanager::Get()->TryInvokeTab(TabId); })));
		if (E.OwnerModule) { Entry.Visibility = ModuleVis(E.OwnerModule); }
	}
}

void FPGXToolbarBuilder::BuildPipelineDropdown(UToolMenu* Menu)
{
	FToolMenuSection& Section = Menu->FindOrAddSection("Pipeline");

	auto ModuleVis = [](const TCHAR* Mod) -> TAttribute<EVisibility>
	{
		FName ModName(Mod);
		return TAttribute<EVisibility>::CreateLambda([ModName]() -> EVisibility
		{
			return FModuleManager::Get().IsModuleLoaded(ModName) ? EVisibility::Visible : EVisibility::Collapsed;
		});
	};

	struct FDropdownEntry { FName Id; FText Label; FText Tip; FName Icon; FName TabId; const TCHAR* OwnerModule; };
	const TArray<FDropdownEntry> Entries = {
		{ "DDP_Platform",   LOCTEXT("DDPPlatform", "Platform Health"),       LOCTEXT("DDPPlatformTip", "Platform budget overview"),                    "PGXEditor.Icon.PlatformHealth",  "PGXPlatformHealthDashboard", nullptr },
		{ "DDP_Config",     LOCTEXT("DDPConfig", "Config Dashboard"),        LOCTEXT("DDPConfigTip", "View and manage Config DataAssets"),             "PGXEditor.Icon.ConfigDashboard", "PGXConfigDashboard",         nullptr },
		{ "DDP_Registry",   LOCTEXT("DDPRegistry", "Data Registry Browser"), LOCTEXT("DDPRegistryTip", "Browse Object DataAssets and cache"),          "PGXEditor.Icon.DataRegistry",    "PGXDataRegistryBrowser",     nullptr },
		{ "DDP_PSOPopul",   LOCTEXT("DDPPSOPopul", "PSO Auto-Populator"),    LOCTEXT("DDPPSOPopulTip", "Auto-populate PSO WarmUpConfig DAs"),          "PGXEditor.Icon.PSO",             "PGXPSOAutoPopulator",        TEXT("PGXPSORuntime") },
		{ "DDP_PSOInsp",    LOCTEXT("DDPPSOInsp", "PSO Inspector"),          LOCTEXT("DDPPSOInspTip", "Inspect PSO warm-up pipeline"),                 "PGXEditor.Icon.PSO",             "PGXPSOInspector",            TEXT("PGXPSORuntime") },
		{ "DDP_LevelFlow",  LOCTEXT("DDPLevelFlow", "LevelFlow Inspector"),  LOCTEXT("DDPLevelFlowTip", "Inspect level transitions"),                  "PGXEditor.Icon.LevelFlow",       "PGXLevelFlowInspector",      TEXT("PGXLevelFlowRuntime") },
		{ "DDP_Loading",    LOCTEXT("DDPLoading", "Loading Inspector"),      LOCTEXT("DDPLoadingTip", "Inspect loading screen status and profiles"),   "PGXEditor.Icon.Loading",         "PGXLoadingInspector",        TEXT("PGXLoadingRuntime") },
	};

	for (const FDropdownEntry& E : Entries)
	{
		FToolMenuEntry& Entry = Section.AddMenuEntry(E.Id, E.Label, E.Tip, PGXIcon(E.Icon),
			FUIAction(FExecuteAction::CreateLambda([TabId = E.TabId]() { FGlobalTabmanager::Get()->TryInvokeTab(TabId); })));
		if (E.OwnerModule) { Entry.Visibility = ModuleVis(E.OwnerModule); }
	}
}

void FPGXToolbarBuilder::BuildDevToolsDropdown(UToolMenu* Menu)
{
	FToolMenuSection& Section = Menu->FindOrAddSection("DevTools");

	auto ModuleVis = [](const TCHAR* Mod) -> TAttribute<EVisibility>
	{
		FName ModName(Mod);
		return TAttribute<EVisibility>::CreateLambda([ModName]() -> EVisibility
		{
			return FModuleManager::Get().IsModuleLoaded(ModName) ? EVisibility::Visible : EVisibility::Collapsed;
		});
	};

	struct FDropdownEntry { FName Id; FText Label; FText Tip; FName Icon; FName TabId; const TCHAR* OwnerModule; };
	const TArray<FDropdownEntry> Entries = {
		{ "DDD_Observer",   LOCTEXT("DDDObserver", "System Observer"),          LOCTEXT("DDDObserverTip", "Live dashboard of PGX framework health"),     "PGXEditor.Icon.SystemObserver", "PGXSystemObserver",          nullptr },
		{ "DDD_Tests",      LOCTEXT("DDDTests", "Test Dashboard"),             LOCTEXT("DDDTestsTip", "Run and view PGX validation tests"),             "PGXEditor.Icon.TestDashboard",  "PGXTestDashboard",           nullptr },
		{ "DDD_VC",         LOCTEXT("DDDVC", "Version Control"),               LOCTEXT("DDDVCTip", "PGX Version Control Inspector"),                   "PGXEditor.Icon.VersionControl", "PGXVersionControlInspector", TEXT("PGXVersionControlEditor") },
		{ "DDD_Scaffold",   LOCTEXT("DDDScaffold", "PGX Scaffold"),            LOCTEXT("DDDScaffoldTip", "Automated project scaffolding"),              "PGXEditor.Icon.Scaffold",       "PGXScaffoldPanel",           TEXT("PGXScaffoldEditor") },
		{ "DDD_Tutorials",  LOCTEXT("DDDTutorials", "Tutorial Hub"),           LOCTEXT("DDDTutorialsTip", "Launch guided tutorials for PGX systems"),   "PGXEditor.Icon.Tutorials",      "PGXTutorialsHub",            nullptr },
	};

	for (const FDropdownEntry& E : Entries)
	{
		FToolMenuEntry& Entry = Section.AddMenuEntry(E.Id, E.Label, E.Tip, PGXIcon(E.Icon),
			FUIAction(FExecuteAction::CreateLambda([TabId = E.TabId]() { FGlobalTabmanager::Get()->TryInvokeTab(TabId); })));
		if (E.OwnerModule) { Entry.Visibility = ModuleVis(E.OwnerModule); }
	}
}

void FPGXToolbarBuilder::BuildMenu(UToolMenu* Menu)
{
	// === Main Actions ===
	{
		FToolMenuSection& Section = Menu->FindOrAddSection("PGXMain");
		Section.Label = LOCTEXT("PGXMain", "PGX Framework");

		Section.AddMenuEntry(
			"OpenHub",
			LOCTEXT("OpenHub", "PGX Hub"),
			LOCTEXT("OpenHubTooltip", "Open the PGX Hub dashboard (Ctrl+Shift+P)"),
			PGXIcon("PGXEditor.Icon.Hub"),
			FUIAction(FExecuteAction::CreateLambda([]()
			{
				FGlobalTabmanager::Get()->TryInvokeTab(FName("PGXHub"));
			}))
		);
	}

	// === Quick Access ===
	{
		FToolMenuSection& Section = Menu->FindOrAddSection("PGXQuickAccess");
		Section.Label = LOCTEXT("PGXQuickAccess", "Quick Access");

		Section.AddMenuEntry(
			"OpenLogViewer",
			LOCTEXT("OpenLogViewer", "Log Viewer"),
			LOCTEXT("OpenLogViewerTooltip", "Open PGX Log Viewer — view live PIE logs or import shipping logs"),
			PGXIcon("PGXEditor.Icon.LogViewer"),
			FUIAction(FExecuteAction::CreateLambda([]()
			{
				FGlobalTabmanager::Get()->TryInvokeTab(FName("PGXLogViewer"));
			}))
		);
	}

	// === Create Assets ===
	{
		FToolMenuSection& Section = Menu->FindOrAddSection("PGXCreate");
		Section.Label = LOCTEXT("PGXCreate", "Create");

		Section.AddSubMenu(
			"CreateAsset",
			LOCTEXT("CreateAsset", "Create PGX Asset..."),
			LOCTEXT("CreateAssetTooltip", "Create a new PGX asset"),
			FNewToolMenuDelegate::CreateStatic(&FPGXToolbarBuilder::BuildCreateAssetSubmenu),
			false,
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.AddContent")
		);
	}

	// === Actions ===
	{
		FToolMenuSection& Section = Menu->FindOrAddSection("PGXActions");
		Section.Label = LOCTEXT("PGXActions", "Actions");

		Section.AddMenuEntry(
			"ValidateFramework",
			LOCTEXT("Validate", "Validate Framework"),
			LOCTEXT("ValidateTooltip", "Run full PGX framework validation (not yet implemented)"),
			PGXIcon("PGXEditor.Icon.Validate"),
			FUIAction(FExecuteAction::CreateLambda([]()
			{
				TArray<FText> Errors;
				TArray<FText> Warnings;
				UPGXFrameworkValidator::ValidateFramework(Errors, Warnings);
			}))
		);

		Section.AddMenuEntry(
			"OpenSettings",
			LOCTEXT("Settings", "PGX Settings"),
			LOCTEXT("SettingsTooltip", "Open PGX Framework project settings"),
			PGXIcon("PGXEditor.Icon.Settings"),
			FUIAction(FExecuteAction::CreateLambda([]()
			{
				FModuleManager::LoadModuleChecked<ISettingsModule>("Settings").ShowViewer("PGX", "Editor", "Editor");
			}))
		);

		Section.AddMenuEntry(
			"RestartEditor",
			LOCTEXT("RestartEditor", "Restart Editor"),
			LOCTEXT("RestartEditorTooltip", "Save pending changes and restart the editor (Ctrl+Shift+R)"),
			PGXIcon("PGXEditor.Icon.Restart"),
			FUIAction(FExecuteAction::CreateStatic(&FPGXToolbarBuilder::ExecuteSafeRestart))
		);
	}

	// === Tools ===
	{
		FToolMenuSection& Section = Menu->FindOrAddSection("PGXTools");
		Section.Label = LOCTEXT("PGXTools", "Tools & Docs");

		Section.AddSubMenu(
			"Tools",
			LOCTEXT("Tools", "Tools"),
			LOCTEXT("ToolsTooltip", "PGX editor tools"),
			FNewToolMenuDelegate::CreateStatic(&FPGXToolbarBuilder::BuildToolsSubmenu),
			false,
			PGXIcon("PGXEditor.Icon.Tools")
		);

		Section.AddSubMenu(
			"Documentation",
			LOCTEXT("Docs", "Documentation"),
			LOCTEXT("DocsTooltip", "PGX documentation"),
			FNewToolMenuDelegate::CreateStatic(&FPGXToolbarBuilder::BuildDocsSubmenu),
			false,
			PGXIcon("PGXEditor.Icon.Docs")
		);
	}

	// === About ===
	{
		FToolMenuSection& Section = Menu->FindOrAddSection("PGXAbout");

		Section.AddMenuEntry(
			"About",
			LOCTEXT("About", "About PGX"),
			LOCTEXT("AboutTooltip", "About PGX Framework"),
			PGXIcon("PGXEditor.Icon.About"),
			FUIAction(FExecuteAction::CreateLambda([]()
			{
				UPGXEditorNotification::ShowInfo(LOCTEXT("AboutPGX", "PGX Framework v0.4.0 | 23 Plugins | UE 5.6.1"));
			}))
		);
	}
}

void FPGXToolbarBuilder::BuildCreateAssetSubmenu(UToolMenu* Menu)
{
	// === Config DataAssets (functional via registry) ===
	{
		FToolMenuSection& Section = Menu->FindOrAddSection("ConfigDAs");
		Section.Label = LOCTEXT("ConfigDAs", "Config DataAssets");

		const TArray<FPGXCreatableAssetEntry>& DAEntries = FPGXAssetCreationRegistry::GetDataAssetEntries();
		for (const FPGXCreatableAssetEntry& DAEntry : DAEntries)
		{
			FName EntryName = FName(*DAEntry.DisplayName.Replace(TEXT(" "), TEXT("")));
			Section.AddMenuEntry(
				EntryName,
				FText::FromString(DAEntry.DisplayName),
				FText::Format(LOCTEXT("CreateDAFmt", "Create {0}"), FText::FromString(DAEntry.DisplayName)),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), FName(*DAEntry.IconStyleName)),
				FUIAction(FExecuteAction::CreateLambda([DAEntry]()
				{
					FPGXBlueprintCreator::CreateDataAssetFromEntry(DAEntry);
				}))
			);
		}
	}

	// === Blueprint categories (functional via registry) ===
	TArray<FString> Categories = FPGXAssetCreationRegistry::GetBlueprintCategories();
	for (const FString& Category : Categories)
	{
		FName SectionName = FName(*FString::Printf(TEXT("BP_%s"), *Category));
		FToolMenuSection& Section = Menu->FindOrAddSection(SectionName);
		Section.Label = FText::Format(LOCTEXT("BPCategoryFmt", "Blueprints — {0}"), FText::FromString(Category));

		TArray<FPGXCreatableAssetEntry> Entries = FPGXAssetCreationRegistry::GetBlueprintEntriesForCategory(Category);
		for (const FPGXCreatableAssetEntry& BPEntry : Entries)
		{
			FName EntryName = FName(*BPEntry.DisplayName.Replace(TEXT(" "), TEXT("")));
			Section.AddMenuEntry(
				EntryName,
				FText::Format(LOCTEXT("BPEntryFmt", "{0} Blueprint"), FText::FromString(BPEntry.DisplayName)),
				FText::Format(LOCTEXT("CreateBPFmt", "Create Blueprint based on {0}"), FText::FromString(BPEntry.DisplayName)),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), FName(*BPEntry.IconStyleName)),
				FUIAction(FExecuteAction::CreateLambda([BPEntry]()
				{
					FPGXBlueprintCreator::CreateBlueprintFromEntry(BPEntry);
				}))
			);
		}
	}
}

void FPGXToolbarBuilder::BuildToolsSubmenu(UToolMenu* Menu)
{
	FToolMenuSection& Section = Menu->FindOrAddSection("PGXToolsList");

	// EN: Helper — returns visibility attribute that collapses when module is not loaded
	// ES: Helper — retorna atributo de visibilidad que colapsa cuando el modulo no esta cargado
	auto ModuleVis = [](const TCHAR* Mod) -> TAttribute<EVisibility>
	{
		FName ModName(Mod);
		return TAttribute<EVisibility>::CreateLambda([ModName]() -> EVisibility
		{
			return FModuleManager::Get().IsModuleLoaded(ModName) ? EVisibility::Visible : EVisibility::Collapsed;
		});
	};

	// EN: Helper — add a tool entry with optional module visibility
	// ES: Helper — agregar entrada de herramienta con visibilidad por modulo opcional
	struct FToolMenuDef { FName Id; FText Label; FText Tip; FName Icon; FName TabId; const TCHAR* OwnerModule; };
	const TArray<FToolMenuDef> ToolDefs = {
		// ── Always visible (core) ──
		{ "LogViewer",               LOCTEXT("LogViewer", "Log Viewer"),              LOCTEXT("LogViewerTooltip", "View and search PGX structured logs with GameplayTag filtering"),                "PGXEditor.Icon.LogViewer",      "PGXLogViewer",              nullptr },
		{ "SystemObserver",          LOCTEXT("SystemObserver", "System Observer"),     LOCTEXT("SystemObserverTooltip", "Live dashboard of PGX framework health: core classes, subsystems, instances, and plugins"), "PGXEditor.Icon.SystemObserver", "PGXSystemObserver",        nullptr },
		{ "DataRegistryBrowser",     LOCTEXT("DataRegistryBrowser", "Data Registry Browser"), LOCTEXT("DataRegistryBrowserTooltip", "Browse registered Object DataAssets — databases, entries, and cache"), "PGXEditor.Icon.DataRegistry", "PGXDataRegistryBrowser",    nullptr },
		{ "ConfigDashboard",         LOCTEXT("ConfigDashboard", "Config Dashboard"),  LOCTEXT("ConfigDashboardTooltip", "View and manage all Config DataAssets across the framework"),              "PGXEditor.Icon.ConfigDashboard", "PGXConfigDashboard",       nullptr },
		{ "TestDashboard",           LOCTEXT("TestDashboard", "Test Dashboard"),       LOCTEXT("TestDashboardTooltip", "Run and view PGX system validation tests"),                                "PGXEditor.Icon.TestDashboard",  "PGXTestDashboard",          nullptr },
		{ "MessageInspector",        LOCTEXT("MessageInspector", "Message Inspector"), LOCTEXT("MessageInspectorTooltip", "Inspect PGX Message system: channels, subscribers, message history, and routing"), "PGXEditor.Icon.Message", "PGXMessageInspector",        nullptr },
		{ "EventDebugger",           LOCTEXT("EventDebugger", "Event Debugger"),       LOCTEXT("EventDebuggerTooltip", "Debug PGX Event Handler: context resolution, telemetry, lifecycle, and blackbox"), "PGXEditor.Icon.EventHandler", "PGXEventDebugger",         nullptr },
		{ "PlatformHealth",          LOCTEXT("PlatformHealth", "Platform Health"),     LOCTEXT("PlatformHealthTooltip", "Platform budget overview: per-system limits, global budgets, platform comparison"), "PGXEditor.Icon.PlatformHealth", "PGXPlatformHealthDashboard", nullptr },
		{ "TutorialHub",             LOCTEXT("TutorialHub", "Tutorial Hub"),           LOCTEXT("TutorialHubTooltip", "Launch guided tutorials for every PGX system"),                               "PGXEditor.Icon.Tutorials",     "PGXTutorialsHub",           nullptr },

		// ── Module-gated ──
		{ "SaveInspector",           LOCTEXT("SaveInspector", "Save Inspector"),       LOCTEXT("SaveInspectorTooltip", "Inspect PGX Save system state: contexts, domains, slots and pipeline activity"), "PGXEditor.Icon.SaveInspector", "PGXSaveInspector",       TEXT("PGXSaveRuntime") },
		{ "GameFlowInspector",       LOCTEXT("GameFlowInspector", "GameFlow Inspector"), LOCTEXT("GameFlowInspectorTooltip", "Inspect GameFlow channels: current states, transition history, and rules"), "PGXEditor.Icon.GameFlow", "PGXGameFlowInspector",      TEXT("PGXGameFlowRuntime") },
		{ "PSOAutoPopulator",        LOCTEXT("PSOAutoPopulator", "PSO Auto-Populator"), LOCTEXT("PSOAutoPopulatorTooltip", "Auto-populate PSO WarmUpConfig DAs from Content Browser selections"),   "PGXEditor.Icon.PSO",            "PGXPSOAutoPopulator",       TEXT("PGXPSORuntime") },
		{ "PSOInspector",            LOCTEXT("PSOInspector", "PSO Inspector"),          LOCTEXT("PSOInspectorTooltip", "Inspect PSO warm-up pipeline: state, progress, configs, and recording sessions"), "PGXEditor.Icon.PSO", "PGXPSOInspector",            TEXT("PGXPSORuntime") },
		{ "LevelFlowInspector",      LOCTEXT("LevelFlowInspector", "LevelFlow Inspector"), LOCTEXT("LevelFlowInspectorTooltip", "Inspect level transitions: status, catalog, history, and sub-levels"), "PGXEditor.Icon.LevelFlow", "PGXLevelFlowInspector",  TEXT("PGXLevelFlowRuntime") },
		{ "LoadingInspector",        LOCTEXT("LoadingInspector", "Loading Inspector"),  LOCTEXT("LoadingInspectorTooltip", "Inspect loading screen: status, profiles, history, and debug controls"), "PGXEditor.Icon.Loading", "PGXLoadingInspector",        TEXT("PGXLoadingRuntime") },
		{ "MGOSInspector",           LOCTEXT("MGOSInspector", "MGOS Inspector"),        LOCTEXT("MGOSInspectorTooltip", "GC Observability: mode, baseline, profile, incidents, history, and class health"), "PGXEditor.Icon.MGOS", "PGXMGOSInspector",         TEXT("PGXMGOSRuntime") },
		{ "AudioInspector",          LOCTEXT("AudioInspector", "Audio Inspector"),      LOCTEXT("AudioInspectorTooltip", "Inspect PGX Audio: backend, channels, mix layers, active sounds, and ducking"), "PGXEditor.Icon.Audio", "PGXAudioInspector",        TEXT("PGXAudioRuntime") },
		{ "VersionControlInspector", LOCTEXT("VersionControlInspector", "Version Control Inspector"), LOCTEXT("VersionControlInspectorTooltip", "PGX Version Control: changelists, auto-tagging, validation, and commit workflow"), "PGXEditor.Icon.VersionControl", "PGXVersionControlInspector", TEXT("PGXVersionControlEditor") },
		{ "ScaffoldPanel",           LOCTEXT("ScaffoldPanel", "PGX Scaffold"),          LOCTEXT("ScaffoldPanelTooltip", "Automated project scaffolding — templates, validation, transactional execution"), "PGXEditor.Icon.Scaffold", "PGXScaffoldPanel",        TEXT("PGXScaffoldEditor") },
	};

	for (const FToolMenuDef& Def : ToolDefs)
	{
		FToolMenuEntry& Entry = Section.AddMenuEntry(
			Def.Id,
			Def.Label,
			Def.Tip,
			PGXIcon(Def.Icon),
			FUIAction(FExecuteAction::CreateLambda([TabId = Def.TabId]()
			{
				FGlobalTabmanager::Get()->TryInvokeTab(TabId);
			}))
		);

		if (Def.OwnerModule)
		{
			Entry.Visibility = ModuleVis(Def.OwnerModule);
		}
	}

}

void FPGXToolbarBuilder::BuildDocsSubmenu(UToolMenu* Menu)
{
	FToolMenuSection& Section = Menu->FindOrAddSection("PGXDocsList");

	// EN: Documentation Viewer — opens PGXDocs NomadTab (if PGXDocsEditor module loaded)
	// ES: Visor de Documentacion — abre NomadTab PGXDocs (si el modulo PGXDocsEditor esta cargado)
	Section.AddMenuEntry(
		"DocumentationViewer",
		LOCTEXT("DocumentationViewer", "Documentation Viewer"),
		LOCTEXT("DocumentationViewerTooltip", "Open the PGX Documentation Viewer — browse Markdown docs in-editor"),
		PGXIcon("PGXEditor.Icon.Docs"),
		FUIAction(FExecuteAction::CreateLambda([]()
		{
			FGlobalTabmanager::Get()->TryInvokeTab(FName("PGXDocs"));
		}))
	);

}

void FPGXToolbarBuilder::ExecuteSafeRestart()
{
	// EN: Block if PIE/SIE active / ES: Bloquear si PIE/SIE esta activo
	if (GEditor && GEditor->IsPlaySessionInProgress())
	{
		UPGXEditorNotification::ShowWarning(
			LOCTEXT("RestartPIE", "PGX: Cannot restart while Play-In-Editor is active. Stop the session first."));
		return;
	}

	// EN: Block if compiling / ES: Bloquear si esta compilando
	if (GEditor && GEditor->bIsCompiling)
	{
		UPGXEditorNotification::ShowWarning(
			LOCTEXT("RestartCompiling", "PGX: Cannot restart while compiling. Wait for compilation to finish."));
		return;
	}

	// EN: Check dirty packages / ES: Verificar paquetes sin guardar
	TArray<UPackage*> DirtyPackages;
	FEditorFileUtils::GetDirtyPackages(DirtyPackages);

	if (DirtyPackages.Num() > 0)
	{
		// EN: Show Save All / Don't Save / Cancel dialog
		// ES: Mostrar dialogo Guardar Todo / No Guardar / Cancelar
		FText Title = LOCTEXT("RestartTitle", "Restart Editor");
		FText Message = FText::Format(
			LOCTEXT("RestartDirtyMsg", "You have {0} unsaved package(s).\n\nSave before restarting?"),
			FText::AsNumber(DirtyPackages.Num()));

		EAppReturnType::Type Result = FMessageDialog::Open(
			EAppMsgType::YesNoCancel, Message, Title);

		if (Result == EAppReturnType::Cancel)
		{
			return;
		}

		if (Result == EAppReturnType::Yes)
		{
			bool bSaved = FEditorFileUtils::SaveDirtyPackages(
				/*bPromptUserToSave=*/ false,
				/*bSaveMapPackages=*/ true,
				/*bSaveContentPackages=*/ true);

			if (!bSaved)
			{
				UPGXEditorNotification::ShowError(
					LOCTEXT("RestartSaveFail", "PGX: Failed to save some packages. Restart cancelled."));
				return;
			}
		}
		// EN: If No → proceed without saving / ES: Si No → continuar sin guardar
	}

	// EN: Notify and restart / ES: Notificar y reiniciar
	UPGXEditorNotification::ShowInfo(LOCTEXT("Restarting", "PGX: Restarting editor..."));
	FUnrealEdMisc::Get().RestartEditor(/*bWarn=*/ false);
}

#undef LOCTEXT_NAMESPACE
