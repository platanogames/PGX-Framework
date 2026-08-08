// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: PGXTutorials module — startup/shutdown + Hub NomadTab + 20 console commands.
// ES: Modulo PGXTutorials — startup/shutdown + NomadTab Hub + 20 comandos de consola.

#include "PGXTutorialsModule.h"
#include "Logging/PGXLogMacros.h"
#include "PGXTutorialRunner.h"
#include "SPGXTutorialHub.h"

#include "HAL/IConsoleManager.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Workspace/PGXWorkspaceMenu.h"
#include "Style/PGXEditorStyle.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXTutorials"

DEFINE_LOG_CATEGORY_STATIC(LogPGXTutorialsModule, Log, All);

// ============================================================================
// Startup / Shutdown
// ============================================================================

void FPGXTutorialsEditorModule::StartupModule()
{
	PGX_LOG_INFO(LogPGXTutorialsModule, TEXT("PGXTutorials module starting up."));

	TutorialRunner = MakeUnique<FPGXTutorialRunner>();
	FPGXTutorialRunner::LoadLanguagePreference();
	FPGXTutorialRunner::LoadBasePathPreference();
	FPGXTutorialRunner::LoadKeepHubOpenPreference();

	RegisterHubTab();
	RegisterConsoleCommands();

	PGX_LOG_INFO(LogPGXTutorialsModule,
		TEXT("PGXTutorials module started. Use 'pgx.tutorials.hub' to open the Tutorial Hub."));
}

void FPGXTutorialsEditorModule::ShutdownModule()
{
	PGX_LOG_INFO(LogPGXTutorialsModule, TEXT("PGXTutorials module shutting down."));

	// EN: Close active tutorial before teardown
	// ES: Cerrar tutorial activo antes del teardown
	if (TutorialRunner.IsValid() && TutorialRunner->IsActive())
	{
		TutorialRunner->Close();
	}
	TutorialRunner.Reset();

	UnregisterConsoleCommands();

	// EN: Unregister NomadTab spawner
	// ES: Desregistrar spawner del NomadTab
	PGX::Editor::UnregisterNomadTab(TEXT("PGXTutorialsHub"));
}

// ============================================================================
// Hub NomadTab Registration
// ============================================================================

void FPGXTutorialsEditorModule::RegisterHubTab()
{
	FPGXTutorialRunner* RunnerPtr = TutorialRunner.Get();

	PGX::Editor::RegisterNomadTab(
		TEXT("PGXTutorialsHub"),
		FOnSpawnTab::CreateLambda([RunnerPtr](const FSpawnTabArgs&) -> TSharedRef<SDockTab>
		{
			return SNew(SDockTab)
				.TabRole(ETabRole::NomadTab)
				.Label(LOCTEXT("HubTabLabel", "PGX Tutorials"))
				[
					SNew(SPGXTutorialHub)
					.Runner(RunnerPtr)
				];
		}))
		.SetDisplayName(LOCTEXT("HubTabDisplay", "Tutorials"))
		.SetTooltipText(LOCTEXT("HubTabTooltip", "Launch guided tutorials for every PGX system"))
		.SetMenuType(ETabSpawnerMenuType::Enabled)
		.SetGroup(FPGXWorkspaceMenu::GetRoot())
		// Use the dedicated tutorials panel brush; the general tutorials brush remains available to console contexts.
		.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.TutorialsPanel"));
}

// ============================================================================
// Console Commands (20 total)
// ============================================================================

void FPGXTutorialsEditorModule::RegisterConsoleCommands()
{
	// EN: Helper lambda to register a command that opens the hub
	// ES: Lambda helper para registrar un comando que abre el hub
	auto RegisterCmd = [this](const TCHAR* Name, const TCHAR* Help, TFunction<void()> Func) -> IConsoleCommand*
	{
		return IConsoleManager::Get().RegisterConsoleCommand(Name, Help,
			FConsoleCommandDelegate::CreateLambda(MoveTemp(Func)), ECVF_Default);
	};

	// -- Hub --
	ConsoleCommands.Add(RegisterCmd(
		TEXT("pgx.tutorials.hub"),
		TEXT("Opens the PGX Tutorial Hub."),
		[]() { FGlobalTabmanager::Get()->TryInvokeTab(FName(TEXT("PGXTutorialsHub"))); }
	));

	// -- Legacy demo (backward compat) --
	ConsoleCommands.Add(RegisterCmd(
		TEXT("pgx.tutorials.demo"),
		TEXT("Opens the PGX Tutorial Hub (legacy alias)."),
		[]() { FGlobalTabmanager::Get()->TryInvokeTab(FName(TEXT("PGXTutorialsHub"))); }
	));

	// -- Onboarding T1-T5 hub aliases --
	// These commands intentionally open the tutorial hub rather than launching
	// a tutorial directly. The `hub` namespace makes that behavior explicit to
	// users and automation.
	struct FTutorialCmd { const TCHAR* CmdName; const TCHAR* Help; };
	static const FTutorialCmd OnboardingCmds[] =
	{
		{ TEXT("pgx.tutorials.hub.t1"), TEXT("Hub alias for T1: What is PGX?") },
		{ TEXT("pgx.tutorials.hub.t2"), TEXT("Hub alias for T2: Zero-Config Game Loop") },
		{ TEXT("pgx.tutorials.hub.t3"), TEXT("Hub alias for T3: Data-Driven Workflow") },
		{ TEXT("pgx.tutorials.hub.t4"), TEXT("Hub alias for T4: Full Observability") },
		{ TEXT("pgx.tutorials.hub.t5"), TEXT("Hub alias for T5: Decoupled Architecture") },
	};
	for (const FTutorialCmd& Cmd : OnboardingCmds)
	{
		ConsoleCommands.Add(RegisterCmd(Cmd.CmdName, Cmd.Help,
			[]() { FGlobalTabmanager::Get()->TryInvokeTab(FName(TEXT("PGXTutorialsHub"))); }
		));
	}

	// -- System S1-S13 hub aliases (same Option A rationale as Onboarding above) --
	// ES: Mismo rationale Option A que Onboarding arriba.
	static const FTutorialCmd SystemCmds[] =
	{
		{ TEXT("pgx.tutorials.hub.profile"),      TEXT("Hub alias for S1: Profile System") },
		{ TEXT("pgx.tutorials.hub.gameflow"),     TEXT("Hub alias for S2: GameFlow System") },
		{ TEXT("pgx.tutorials.hub.save"),         TEXT("Hub alias for S3: Save System") },
		{ TEXT("pgx.tutorials.hub.pso"),          TEXT("Hub alias for S4: PSO System") },
		{ TEXT("pgx.tutorials.hub.loading"),      TEXT("Hub alias for S5: Loading System") },
		{ TEXT("pgx.tutorials.hub.levelflow"),    TEXT("Hub alias for S6: LevelFlow System") },
		{ TEXT("pgx.tutorials.hub.audio"),        TEXT("Hub alias for S7: Audio System") },
		{ TEXT("pgx.tutorials.hub.log"),          TEXT("Hub alias for S8: Log System") },
		{ TEXT("pgx.tutorials.hub.registry"),     TEXT("Hub alias for S9: Data Registry") },
		{ TEXT("pgx.tutorials.hub.construction"), TEXT("Hub alias for S10: Construction System") },
		{ TEXT("pgx.tutorials.hub.message"),      TEXT("Hub alias for S11: Message System") },
		{ TEXT("pgx.tutorials.hub.eventhandler"), TEXT("Hub alias for S12: EventHandler System") },
		{ TEXT("pgx.tutorials.hub.mgos"),         TEXT("Hub alias for S13: MGOS System") },
		{ TEXT("pgx.tutorials.hub.trade"),        TEXT("Hub alias for S14: Trade System") },
	};
	for (const FTutorialCmd& Cmd : SystemCmds)
	{
		ConsoleCommands.Add(RegisterCmd(Cmd.CmdName, Cmd.Help,
			[]() { FGlobalTabmanager::Get()->TryInvokeTab(FName(TEXT("PGXTutorialsHub"))); }
		));
	}

	PGX_LOG_INFO(LogPGXTutorialsModule, TEXT("Registered %d tutorial console commands."), ConsoleCommands.Num());
}

void FPGXTutorialsEditorModule::UnregisterConsoleCommands()
{
	for (IConsoleCommand* Cmd : ConsoleCommands)
	{
		if (Cmd)
		{
			IConsoleManager::Get().UnregisterConsoleObject(Cmd);
		}
	}
	ConsoleCommands.Empty();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXTutorialsEditorModule, PGXTutorialsEditor)
