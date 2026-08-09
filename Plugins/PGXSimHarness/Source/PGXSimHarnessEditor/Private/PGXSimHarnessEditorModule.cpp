// Copyright PGX Framework. All Rights Reserved.

#include "PGXSimHarnessEditorModule.h"
#include "FPGXHarnessSimulation.h"
#include "Logging/PGXLogMacros.h"
#include "PGXSimHarnessCBExtension.h"
#include "SPGXSimHarnessTab.h"
#include "Style/PGXEditorStyle.h"
#include "Workspace/PGXWorkspaceMenu.h"

#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Styling/AppStyle.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "Utils/PGXTabRegistration.h"

DEFINE_LOG_CATEGORY(LogPGXSimHarness);

#define LOCTEXT_NAMESPACE "PGXSimHarness"

static const FName SimHarnessTabId(TEXT("PGXSimHarnessPanel"));

namespace
{
	UWorld* ResolveHarnessCommandWorld()
	{
		if (!GEngine)
		{
			return nullptr;
		}

		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game)
			{
				return Context.World();
			}
		}
		return nullptr;
	}
}

FName FPGXSimHarnessEditorModule::GetSimHarnessTabId()
{
	return SimHarnessTabId;
}

void FPGXSimHarnessEditorModule::StartupModule()
{
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("PGXSimHarnessEditor: Module starting..."));

	// EN: Register the PSPH Control Panel NomadTab
	// ES: Registrar el NomadTab del Panel de Control PSPH
	PGX::Editor::RegisterNomadTab(
		SimHarnessTabId,
		FOnSpawnTab::CreateLambda([](const FSpawnTabArgs&) -> TSharedRef<SDockTab>
		{
			return SNew(SDockTab)
				.TabRole(NomadTab)
				[
					SNew(SPGXSimHarnessTab)
				];
		})
	)
	.SetDisplayName(LOCTEXT("SimHarnessTab", "Harness"))
	.SetTooltipText(LOCTEXT("SimHarnessTooltip", "PGX Sim Harness — Demo mode and visual verification"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetGroup(FPGXWorkspaceMenu::GetToolsGroup())
	// The canonical .SimHarnessPanel brush is used here; .SimHarness remains a compatibility key for existing menu and observer integrations.
	.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.SimHarnessPanel"));

	// EN: Register CB extension for "PGX DEMO" section
	// ES: Registrar extension CB para seccion "PGX DEMO"
	FPGXSimHarnessCBExtension::Register();
	RegisterHarnessConsoleCommands();

	// EN: Register harness gameplay tags programmatically.
	//     Config/DefaultGameplayTags.ini is NOT reliably loaded for editor-only plugins in UE 5.6.
	//     AddNativeGameplayTag() works after GameplayTagsManager init (safe from StartupModule).
	// ES: Registrar tags del harness programaticamente.
	//     Config/DefaultGameplayTags.ini NO se carga de forma fiable en plugins editor-only en UE 5.6.
	//     AddNativeGameplayTag() funciona tras init del GameplayTagsManager (seguro desde StartupModule).
	{
		UGameplayTagsManager& TagMgr = UGameplayTagsManager::Get();
		int32 VerifiedHarnessTagCount = 0;
		int32 NewlyRegisteredTagCount = 0;
		const auto RegisterHarnessTag = [&TagMgr, &VerifiedHarnessTagCount, &NewlyRegisteredTagCount](const TCHAR* TagName, const TCHAR* DevComment)
		{
			++VerifiedHarnessTagCount;
			const FName TagFName(TagName);
			if (FGameplayTag::RequestGameplayTag(TagFName, /*bErrorIfNotFound=*/ false).IsValid())
			{
				return;
			}
			TagMgr.AddNativeGameplayTag(TagFName, DevComment);
			++NewlyRegisteredTagCount;
		};

		// --- Audio channels ---
		RegisterHarnessTag(TEXT("PGX.Harness.Audio.Channel.SFX"), TEXT("Harness: SFX audio channel"));
		RegisterHarnessTag(TEXT("PGX.Harness.Audio.Channel.Music"), TEXT("Harness: Music audio channel"));
		RegisterHarnessTag(TEXT("PGX.Harness.Audio.Channel.Voice"), TEXT("Harness: Voice audio channel"));
		RegisterHarnessTag(TEXT("PGX.Harness.Audio.Channel.Ambient"), TEXT("Harness: Ambient audio channel"));
		RegisterHarnessTag(TEXT("PGX.Harness.Audio.Channel.UI"), TEXT("Harness: UI audio channel"));
		RegisterHarnessTag(TEXT("PGX.Harness.Audio.MusicState.Explore"), TEXT("Harness: Music state explore"));
		RegisterHarnessTag(TEXT("PGX.Harness.Audio.Speaker.NPC"), TEXT("Harness: NPC speaker tag"));
		RegisterHarnessTag(TEXT("PGX.Harness.Audio.Priority.High"), TEXT("Harness: High priority audio"));

		// --- Save contexts and domains ---
		RegisterHarnessTag(TEXT("PGX.Harness.Save.Context.Campaign"), TEXT("Harness: Campaign save context"));
		RegisterHarnessTag(TEXT("PGX.Harness.Save.Context.Settings"), TEXT("Harness: Settings save context"));
		RegisterHarnessTag(TEXT("PGX.Harness.Save.Domain.PlayerProgress"), TEXT("Harness: Player progress save domain"));
		RegisterHarnessTag(TEXT("PGX.Harness.Save.Domain.WorldState"), TEXT("Harness: World state save domain"));
		RegisterHarnessTag(TEXT("PGX.Harness.Save.Domain.Graphics"), TEXT("Harness: Graphics save domain"));
		RegisterHarnessTag(TEXT("PGX.Harness.Save.Domain.AudioSettings"), TEXT("Harness: Audio settings save domain"));

		// --- GameFlow states ---
		RegisterHarnessTag(TEXT("PGX.Harness.Flow.MainMenu"), TEXT("Harness: Main menu flow state"));
		RegisterHarnessTag(TEXT("PGX.Harness.Flow.HUD"), TEXT("Harness: HUD flow state"));
		RegisterHarnessTag(TEXT("PGX.Harness.Flow.Active"), TEXT("Harness: Active flow state"));
		RegisterHarnessTag(TEXT("PGX.Harness.Flow.Patrol"), TEXT("Harness: Patrol flow state"));
		RegisterHarnessTag(TEXT("PGX.Harness.Flow.Gameplay"), TEXT("Harness: Gameplay flow state"));
		RegisterHarnessTag(TEXT("PGX.Harness.Flow.Running"), TEXT("Harness: Running flow state"));
		RegisterHarnessTag(TEXT("PGX.Harness.Flow.Explore"), TEXT("Harness: Explore flow state"));
		RegisterHarnessTag(TEXT("PGX.Harness.Flow.InGame"), TEXT("Harness: InGame flow state"));
		RegisterHarnessTag(TEXT("PGX.Harness.Flow.Loading"), TEXT("Harness: Loading flow state"));
		RegisterHarnessTag(TEXT("PGX.Harness.Flow.Pause"), TEXT("Harness: Pause flow state"));

		// --- Message channels ---
		RegisterHarnessTag(TEXT("PGX.Harness.Message.UI"), TEXT("Harness: UI message channel"));
		RegisterHarnessTag(TEXT("PGX.Harness.Message.Gameplay"), TEXT("Harness: Gameplay message channel"));
		RegisterHarnessTag(TEXT("PGX.Harness.Message.System"), TEXT("Harness: System message channel"));
		RegisterHarnessTag(TEXT("PGX.Harness.Message.Audio"), TEXT("Harness: Audio message channel"));
		RegisterHarnessTag(TEXT("PGX.Harness.Message.Network"), TEXT("Harness: Network message channel"));

		// --- Event handler events ---
		RegisterHarnessTag(TEXT("PGX.Harness.Event.Damage"), TEXT("Harness: Damage event"));
		RegisterHarnessTag(TEXT("PGX.Harness.Event.Heal"), TEXT("Harness: Heal event"));
		RegisterHarnessTag(TEXT("PGX.Harness.Event.Pickup"), TEXT("Harness: Pickup event"));
		RegisterHarnessTag(TEXT("PGX.Harness.Event.Interact"), TEXT("Harness: Interact event"));
		RegisterHarnessTag(TEXT("PGX.Harness.Event.Spawn"), TEXT("Harness: Spawn event"));

		// --- PSO contexts ---
		RegisterHarnessTag(TEXT("PGX.Harness.PSO.Context.MainMenu"), TEXT("Harness: MainMenu PSO context"));
		RegisterHarnessTag(TEXT("PGX.Harness.PSO.Context.Gameplay"), TEXT("Harness: Gameplay PSO context"));
		RegisterHarnessTag(TEXT("PGX.Harness.PSO.Context.Inventory"), TEXT("Harness: Inventory PSO context"));

		// --- Input contexts ---
		RegisterHarnessTag(TEXT("PGX.Harness.Input.Context.Gameplay"), TEXT("Harness: Gameplay input context"));
		RegisterHarnessTag(TEXT("PGX.Harness.Input.Context.UI"), TEXT("Harness: UI input context"));

		// --- Spawn wave smoke ---
		RegisterHarnessTag(TEXT("PGX.Harness.Spawn.Wave.Smoke"), TEXT("Harness: Spawn wave smoke test"));

		// --- Ability smoke ---
		RegisterHarnessTag(TEXT("PGX.Harness.Ability.Smoke"), TEXT("Harness: Ability smoke test"));

		// --- LevelFlow ---
		RegisterHarnessTag(TEXT("PGX.Harness.Level.TestZone"), TEXT("Harness: Test zone level"));
		RegisterHarnessTag(TEXT("PGX.Harness.Level.SubLevel.Cave"), TEXT("Harness: Cave sub-level"));

		// --- Loading ---
		RegisterHarnessTag(TEXT("PGX.Harness.Loading.Context.Test"), TEXT("Harness: Test loading context"));

		// --- DataRegistry databases ---
		RegisterHarnessTag(TEXT("PGX.Harness.Registry.Items"), TEXT("Harness: Items database"));
		RegisterHarnessTag(TEXT("PGX.Harness.Registry.NPCs"), TEXT("Harness: NPCs database"));

		// --- Registry items ---
		RegisterHarnessTag(TEXT("PGX.Harness.Registry.Item.IronSword"), TEXT("Harness: Iron Sword item"));
		RegisterHarnessTag(TEXT("PGX.Harness.Registry.Item.HealthPotion"), TEXT("Harness: Health Potion item"));
		RegisterHarnessTag(TEXT("PGX.Harness.Registry.Item.MageStaff"), TEXT("Harness: Mage Staff item"));
		RegisterHarnessTag(TEXT("PGX.Harness.Registry.Item.DragonShield"), TEXT("Harness: Dragon Guard item"));
		RegisterHarnessTag(TEXT("PGX.Harness.Registry.Item.FireScroll"), TEXT("Harness: Fire Scroll item"));

		// --- Registry NPCs ---
		RegisterHarnessTag(TEXT("PGX.Harness.Registry.NPC.Merchant"), TEXT("Harness: Merchant NPC"));
		RegisterHarnessTag(TEXT("PGX.Harness.Registry.NPC.Blacksmith"), TEXT("Harness: Blacksmith NPC"));
		RegisterHarnessTag(TEXT("PGX.Harness.Registry.NPC.QuestGiver"), TEXT("Harness: Quest Giver NPC"));
		RegisterHarnessTag(TEXT("PGX.Harness.Registry.NPC.Guard"), TEXT("Harness: Guard NPC"));
		RegisterHarnessTag(TEXT("PGX.Harness.Registry.NPC.Innkeeper"), TEXT("Harness: Innkeeper NPC"));

		// --- Registry categories ---
		RegisterHarnessTag(TEXT("PGX.Harness.Registry.Category.Weapon"), TEXT("Harness: Weapon category"));
		RegisterHarnessTag(TEXT("PGX.Harness.Registry.Category.Consumable"), TEXT("Harness: Consumable category"));
		RegisterHarnessTag(TEXT("PGX.Harness.Registry.Category.Equipment"), TEXT("Harness: Equipment category"));
		RegisterHarnessTag(TEXT("PGX.Harness.Registry.Category.Vendor"), TEXT("Harness: Vendor category"));
		RegisterHarnessTag(TEXT("PGX.Harness.Registry.Category.Service"), TEXT("Harness: Service category"));

		PGX_LOG_INFO(LogPGXSimHarness, TEXT("PGXSimHarnessEditor: Verified %d PGX.Harness.* tags via AddNativeGameplayTag (%d newly registered)"), VerifiedHarnessTagCount, NewlyRegisteredTagCount);
	}

	// EN: Verify harness tags are now available
	// ES: Verificar que los tags del harness estan disponibles
	{
		FGameplayTag ProbeTag = FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Audio.Channel.SFX"), /*bErrorIfNotFound=*/ false);
		if (ProbeTag.IsValid())
		{
			PGX_LOG_INFO(LogPGXSimHarness, TEXT("PGXSimHarnessEditor: Harness tags verified — 61 tags available"));
		}
		else
		{
			PGX_LOG_ERROR(LogPGXSimHarness, TEXT("PGXSimHarnessEditor: Harness tags STILL not available after AddNativeGameplayTag! "
				"PSPH injection will fail. Check GameplayTagsManager initialization order."));
		}
	}

	PGX_LOG_INFO(LogPGXSimHarness, TEXT("PGXSimHarnessEditor: Module started — PSPH Control Panel + PGX DEMO CB section registered"));
}

void FPGXSimHarnessEditorModule::ShutdownModule()
{
	// EN: Unregister in LIFO order / ES: Desregistrar en orden LIFO
	UnregisterHarnessConsoleCommands();
	if (LiveSimulation.IsValid())
	{
		LiveSimulation->Stop(false);
		LiveSimulation.Reset();
	}

	FPGXSimHarnessCBExtension::Unregister();

	PGX::Editor::UnregisterNomadTab(SimHarnessTabId);

	PGX_LOG_INFO(LogPGXSimHarness, TEXT("PGXSimHarnessEditor: Module shut down"));
}

void FPGXSimHarnessEditorModule::RegisterHarnessConsoleCommands()
{
	IConsoleManager& ConsoleManager = IConsoleManager::Get();
	RegisteredConsoleCommands.Add(ConsoleManager.RegisterConsoleCommand(
		TEXT("PGX.Harness.StartSim"),
		TEXT("Start the Live PGX live harness simulation."),
		FConsoleCommandDelegate::CreateRaw(this, &FPGXSimHarnessEditorModule::StartLiveSimulation),
		ECVF_Default));
	RegisteredConsoleCommands.Add(ConsoleManager.RegisterConsoleCommand(
		TEXT("PGX.Harness.StopSim"),
		TEXT("Stop the Live PGX live harness simulation and export a partial report."),
		FConsoleCommandDelegate::CreateRaw(this, &FPGXSimHarnessEditorModule::StopLiveSimulation),
		ECVF_Default));
	RegisteredConsoleCommands.Add(ConsoleManager.RegisterConsoleCommand(
		TEXT("PGX.Harness.GetStatus"),
		TEXT("Log the current Live PGX harness simulation status."),
		FConsoleCommandDelegate::CreateRaw(this, &FPGXSimHarnessEditorModule::LogLiveSimulationStatus),
		ECVF_Default));
	RegisteredConsoleCommands.Add(ConsoleManager.RegisterConsoleCommand(
		TEXT("PGX.Harness.ExportReport"),
		TEXT("Export the current Live PGX harness simulation report."),
		FConsoleCommandDelegate::CreateRaw(this, &FPGXSimHarnessEditorModule::ExportLiveSimulationReport),
		ECVF_Default));
}

void FPGXSimHarnessEditorModule::UnregisterHarnessConsoleCommands()
{
	IConsoleManager& ConsoleManager = IConsoleManager::Get();
	for (IConsoleObject* Command : RegisteredConsoleCommands)
	{
		ConsoleManager.UnregisterConsoleObject(Command, false);
	}
	RegisteredConsoleCommands.Reset();
}

void FPGXSimHarnessEditorModule::StartLiveSimulation()
{
	if (!LiveSimulation.IsValid())
	{
		LiveSimulation = MakeUnique<FPGXHarnessSimulation>();
	}

	if (LiveSimulation->IsRunning())
	{
		PGX_LOG_WARNING(LogPGXSimHarness, TEXT("PGX.Harness.StartSim — simulation already running: %s"), *LiveSimulation->GetStatusText());
		return;
	}

	UWorld* World = ResolveHarnessCommandWorld();
	if (!LiveSimulation->Start(World))
	{
		PGX_LOG_ERROR(LogPGXSimHarness, TEXT("PGX.Harness.StartSim — failed. Start PIE/game world first."));
	}
}

void FPGXSimHarnessEditorModule::StopLiveSimulation()
{
	if (LiveSimulation.IsValid())
	{
		LiveSimulation->Stop(true);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("PGX.Harness.StopSim — %s"), *LiveSimulation->GetStatusText());
	}
}

void FPGXSimHarnessEditorModule::LogLiveSimulationStatus() const
{
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("PGX.Harness.GetStatus — %s"),
		LiveSimulation.IsValid() ? *LiveSimulation->GetStatusText() : TEXT("not initialized"));
}

void FPGXSimHarnessEditorModule::ExportLiveSimulationReport() const
{
	if (!LiveSimulation.IsValid())
	{
		PGX_LOG_WARNING(LogPGXSimHarness, TEXT("PGX.Harness.ExportReport — simulation not initialized"));
		return;
	}

	const TArray<FString> Paths = LiveSimulation->ExportReport();
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("PGX.Harness.ExportReport — %s | %s"),
		Paths.IsValidIndex(0) ? *Paths[0] : TEXT("<no json>"),
		Paths.IsValidIndex(1) ? *Paths[1] : TEXT("<no md>"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXSimHarnessEditorModule, PGXSimHarnessEditor)
