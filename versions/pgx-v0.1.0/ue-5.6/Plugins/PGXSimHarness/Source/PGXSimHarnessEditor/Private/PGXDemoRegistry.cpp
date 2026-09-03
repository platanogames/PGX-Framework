// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXDemoRegistry.h"

// EN: Demo entries are organized by numbered categories, following logical configuration order.
// ES: Las entradas demo se organizan por categorias numeradas, siguiendo el orden logico de configuracion.

static const TArray<FPGXDemoEntry> GDemoEntries = {

	// ─── 1. Foundation ───
	{
		TEXT("Project Profile Config"),
		TEXT("/Script/PGXCoreRuntime.PGXProjectProfileConfig"),
		TEXT("DA_Demo_ProjectProfile"),
		TEXT("1. Foundation"),
		0,
		TEXT("Configure project identity, platform detection, and quality tiers"),
		TEXT("ClassIcon.PlayerController")
	},
	{
		TEXT("Platform Config"),
		TEXT("/Script/PGXCoreRuntime.PGXPlatformConfig"),
		TEXT("DA_Demo_PlatformWindows"),
		TEXT("1. Foundation"),
		1,
		TEXT("Define per-platform budgets: memory, audio channels, draw calls, particle limits"),
		TEXT("ClassIcon.PlayerController")
	},
	{
		TEXT("GameMode Construction"),
		TEXT("/Script/PGXCoreRuntime.PGXGameModeConstruction"),
		TEXT("DA_Demo_GameModeConstruction"),
		TEXT("1. Foundation"),
		2,
		TEXT("Configure which classes the GameMode spawns: PlayerController, Pawn, GameState, HUD"),
		TEXT("ClassIcon.GameModeBase")
	},

	// ─── 2. Game State ───
	{
		TEXT("GameFlow Config"),
		TEXT("/Script/PGXGameFlowRuntime.PGXGameFlowConfig"),
		TEXT("DA_Demo_GameFlowConfig"),
		TEXT("2. Game State"),
		0,
		TEXT("Define game state channels, initial states, and transition rules"),
		TEXT("ClassIcon.GameModeBase")
	},
	{
		TEXT("Flow Rules Config"),
		TEXT("/Script/PGXGameFlowRuntime.PGXFlowRulesConfig"),
		TEXT("DA_Demo_FlowRules"),
		TEXT("2. Game State"),
		1,
		TEXT("Define allowed state transitions and validation rules per channel"),
		TEXT("ClassIcon.GameModeBase")
	},

	// ─── 3. Persistence ───
	{
		TEXT("Save Config"),
		TEXT("/Script/PGXSaveRuntime.PGXSaveConfig"),
		TEXT("DA_Demo_SaveConfig"),
		TEXT("3. Persistence"),
		0,
		TEXT("Configure save system: slot count, auto-save interval, serialization settings"),
		TEXT("ClassIcon.SaveGame")
	},
	{
		TEXT("PSO WarmUp Config"),
		TEXT("/Script/PGXPSORuntime.PGXPSOWarmUpConfig"),
		TEXT("DA_Demo_PSOConfig"),
		TEXT("3. Persistence"),
		1,
		TEXT("Configure Pipeline State Object warm-up: materials, shaders, and precaching strategy"),
		TEXT("ClassIcon.Object")
	},

	// ─── 4. Level Management ───
	{
		TEXT("LevelFlow Config"),
		TEXT("/Script/PGXLoadingRuntime.PGXLevelFlowConfig"),
		TEXT("DA_Demo_LevelFlow"),
		TEXT("4. Level Management"),
		0,
		TEXT("Configure level transition flow: streaming strategy, sub-level management"),
		TEXT("ClassIcon.World")
	},
	{
		TEXT("Level Profile"),
		TEXT("/Script/PGXLoadingRuntime.PGXLevelProfile"),
		TEXT("DA_Demo_LevelProfile"),
		TEXT("4. Level Management"),
		1,
		TEXT("Define a level's metadata: display name, loading screen, background music, sub-levels"),
		TEXT("ClassIcon.World")
	},
	{
		TEXT("Loading Config"),
		TEXT("/Script/PGXLoadingRuntime.PGXLoadingConfig"),
		TEXT("DA_Demo_LoadingConfig"),
		TEXT("4. Level Management"),
		2,
		TEXT("Configure loading screen behavior: minimum display time, fade settings"),
		TEXT("ClassIcon.Object")
	},
	{
		TEXT("Loading Profile"),
		TEXT("/Script/PGXLoadingRuntime.PGXLoadingProfile"),
		TEXT("DA_Demo_LoadingProfile"),
		TEXT("4. Level Management"),
		3,
		TEXT("Define loading screen visuals: background, tips, progress bar style"),
		TEXT("ClassIcon.Object")
	},

	// ─── 5. Audio ───
	{
		TEXT("Audio Config"),
		TEXT("/Script/PGXAudioRuntime.PGXAudioConfig"),
		TEXT("DA_Demo_AudioConfig"),
		TEXT("5. Audio"),
		0,
		TEXT("Master audio configuration: backend selection, global volume, channel limits"),
		TEXT("ClassIcon.SoundCue")
	},
	{
		TEXT("Channel Config — SFX"),
		TEXT("/Script/PGXAudioRuntime.PGXAudioChannelConfig"),
		TEXT("DA_Demo_AudioChannelSFX"),
		TEXT("5. Audio"),
		1,
		TEXT("Configure an audio channel: volume, priority, concurrency, spatial settings"),
		TEXT("ClassIcon.SoundCue")
	},
	{
		TEXT("Sound Definition"),
		TEXT("/Script/PGXAudioRuntime.PGXSoundDefinition"),
		TEXT("DA_Demo_SoundExample"),
		TEXT("5. Audio"),
		2,
		TEXT("Define a sound asset: source, channel, variations, attenuation, cooldown"),
		TEXT("ClassIcon.SoundCue")
	},
	{
		TEXT("Music Playlist"),
		TEXT("/Script/PGXAudioRuntime.PGXMusicPlaylist"),
		TEXT("DA_Demo_Playlist"),
		TEXT("5. Audio"),
		3,
		TEXT("Create a music playlist: tracks, shuffle, crossfade, loop settings"),
		TEXT("ClassIcon.SoundCue")
	},
	{
		TEXT("Ducking Config"),
		TEXT("/Script/PGXAudioRuntime.PGXAudioDuckingConfig"),
		TEXT("DA_Demo_DuckingConfig"),
		TEXT("5. Audio"),
		4,
		TEXT("Configure audio ducking: which channels duck others, fade curves, priorities"),
		TEXT("ClassIcon.SoundCue")
	},
	{
		TEXT("Audio Profile"),
		TEXT("/Script/PGXAudioRuntime.PGXAudioProfile"),
		TEXT("DA_Demo_AudioProfile"),
		TEXT("5. Audio"),
		5,
		TEXT("Define an audio profile: per-channel gain, mix layer routing, and HDR/HRTF policy"),
		TEXT("ClassIcon.SoundCue")
	},
	{
		TEXT("Level Audio Config"),
		TEXT("/Script/PGXAudioRuntime.PGXLevelAudioConfig"),
		TEXT("DA_Demo_LevelAudioConfig"),
		TEXT("5. Audio"),
		6,
		TEXT("Configure per-level audio overrides: ambient bed, music selection, ducking profile"),
		TEXT("ClassIcon.SoundCue")
	},

	// ─── 6. Infrastructure ───
	{
		TEXT("Message Config"),
		TEXT("/Script/PGXCoreRuntime.PGXMessageConfig"),
		TEXT("DA_Demo_MessageConfig"),
		TEXT("6. Infrastructure"),
		0,
		TEXT("Configure the pub/sub message bus: channels, listener limits, telemetry"),
		TEXT("PGXEditor.Icon.Message")
	},
	{
		TEXT("EventHandler Config"),
		TEXT("/Script/PGXCoreRuntime.PGXEventHandlerConfig"),
		TEXT("DA_Demo_EventHandlerConfig"),
		TEXT("6. Infrastructure"),
		1,
		TEXT("Configure context-driven event resolution: handler priorities, lifecycle, telemetry"),
		TEXT("PGXEditor.Icon.EventHandler")
	},
	{
		TEXT("GC Observer Config"),
		TEXT("/Script/PGXMGOSRuntime.PGXGCObserverConfig"),
		TEXT("DA_Demo_GCObserverConfig"),
		TEXT("6. Infrastructure"),
		2,
		TEXT("Configure garbage collection observability: sampling interval, baseline, alert thresholds"),
		TEXT("ClassIcon.Object")
	},

	// ─── 7. Tools ───
	{
		TEXT("Docs Config"),
		TEXT("/Script/PGXDocs.PGXDocsConfig"),
		TEXT("DA_Demo_DocsConfig"),
		TEXT("7. Tools"),
		0,
		TEXT("Configure the documentation viewer: root paths, file patterns, search settings"),
		TEXT("ClassIcon.Object")
	},

	// ─── 8. World Systems ───
	{
		TEXT("Environment Config"),
		TEXT("/Script/PGXEnvironmentRuntime.PGXEnvironmentConfig"),
		TEXT("DA_Demo_EnvironmentConfig"),
		TEXT("8. World Systems"),
		0,
		TEXT("Configure environmental variables, default zones, tick policy, and observability"),
		TEXT("PGXEditor.Icon.Environment")
	},
	{
		TEXT("Environment Variable"),
		TEXT("/Script/PGXEnvironmentRuntime.PGXEnvironmentVariable"),
		TEXT("DA_Demo_EnvironmentVariable"),
		TEXT("8. World Systems"),
		1,
		TEXT("Define one authored environmental variable, bounds, units, and default value"),
		TEXT("PGXEditor.Icon.Environment")
	},
	{
		TEXT("Environment Zone Definition"),
		TEXT("/Script/PGXEnvironmentRuntime.PGXEnvironmentZoneDefinition"),
		TEXT("DA_Demo_EnvironmentZoneDefinition"),
		TEXT("8. World Systems"),
		2,
		TEXT("Define a zone profile: identity, modifiers, and local environmental policy"),
		TEXT("PGXEditor.Icon.Environment")
	},
	{
		TEXT("Environment Tick Profile"),
		TEXT("/Script/PGXEnvironmentRuntime.PGXEnvironmentTickProfile"),
		TEXT("DA_Demo_EnvironmentTickProfile"),
		TEXT("8. World Systems"),
		3,
		TEXT("Configure deterministic environment update cadence and tick budgeting"),
		TEXT("PGXEditor.Icon.Environment")
	},
	{
		TEXT("Colony Config"),
		TEXT("/Script/PGXColonyRuntime.PGXColonyConfig"),
		TEXT("DA_Demo_ColonyConfig"),
		TEXT("8. World Systems"),
		4,
		TEXT("Configure population, settlement, morale, and colony simulation policy"),
		TEXT("PGXEditor.Icon.Colony")
	},
	{
		TEXT("Spawn Config"),
		TEXT("/Script/PGXSpawnRuntime.PGXSpawnConfig"),
		TEXT("DA_Demo_SpawnConfig"),
		TEXT("8. World Systems"),
		5,
		TEXT("Configure spawn request budgets, validation, cleanup, and registry policy"),
		TEXT("PGXEditor.Icon.Spawn")
	},
	{
		TEXT("Wave Definition"),
		TEXT("/Script/PGXSpawnRuntime.PGXWaveDefinition"),
		TEXT("DA_Demo_WaveDefinition"),
		TEXT("8. World Systems"),
		6,
		TEXT("Define authored spawn wave composition, cadence, and activation policy"),
		TEXT("PGXEditor.Icon.Spawn")
	},

	// ─── 9. Economy ───
	{
		TEXT("Trade Config"),
		TEXT("/Script/PGXTradeRuntime.PGXTradeConfig"),
		TEXT("DA_Demo_TradeConfig"),
		TEXT("9. Economy"),
		0,
		TEXT("Configure trade valuation, offer constraints, market policy, and diagnostics"),
		TEXT("PGXEditor.Icon.Trade")
	},
	{
		TEXT("Crafting Recipe"),
		TEXT("/Script/PGXCraftingRuntime.PGXRecipeDefinition"),
		TEXT("DA_Demo_CraftingRecipe"),
		TEXT("9. Economy"),
		1,
		TEXT("Define an authored crafting recipe, requirements, outputs, and simulation metadata"),
		TEXT("PGXEditor.Icon.Crafting")
	},
	{
		TEXT("Vehicle Definition"),
		TEXT("/Script/PGXVehiclesRuntime.PGXVehicleDefinitionAsset"),
		TEXT("DA_Demo_VehiclesDefinition"),
		TEXT("9. Economy"),
		2,
		TEXT("Define a vehicle archetype, capacity, fuel, repair, and operation policy"),
		TEXT("PGXEditor.Icon.Vehicles")
	},
	{
		TEXT("Inventory Item Definition"),
		TEXT("/Script/PGXInventoryRuntime.PGXItemDefinition"),
		TEXT("DA_Demo_InventoryItemDefinition"),
		TEXT("9. Economy"),
		3,
		TEXT("Define an inventory item: stack limits, weight, tags, and transfer policy"),
		TEXT("PGXEditor.Icon.Inventory")
	},

	// ─── 10. Systems & Presentation ───
	{
		TEXT("AI Config"),
		TEXT("/Script/PGXAIRuntime.PGXAIConfig"),
		TEXT("DA_Demo_AIConfig"),
		TEXT("10. Systems & Presentation"),
		0,
		TEXT("Configure AI perception, squad limits, update cadence, and fallback policy"),
		TEXT("PGXEditor.Icon.AI")
	},
	{
		TEXT("Input Config"),
		TEXT("/Script/PGXInputRuntime.PGXInputConfig"),
		TEXT("DA_Demo_InputConfig"),
		TEXT("10. Systems & Presentation"),
		1,
		TEXT("Configure input buffering, device policy, dead zones, and default context behavior"),
		TEXT("PGXEditor.Icon.Input")
	},
	{
		TEXT("UI Config"),
		TEXT("/Script/PGXUIRuntime.PGXUIConfig"),
		TEXT("DA_Demo_UIConfig"),
		TEXT("10. Systems & Presentation"),
		2,
		TEXT("Configure screen stack limits, transition defaults, notifications, and widget pool policy"),
		TEXT("PGXEditor.Icon.UI")
	},
	{
		TEXT("Screen Definition"),
		TEXT("/Script/PGXUIRuntime.PGXScreenDefinition"),
		TEXT("DA_Demo_ScreenExample"),
		TEXT("10. Systems & Presentation"),
		3,
		TEXT("Define an authored screen identity, layer, widget class, and transition metadata"),
		TEXT("PGXEditor.Icon.UI")
	},
	{
		TEXT("Notification Profile"),
		TEXT("/Script/PGXUIRuntime.PGXNotificationProfile"),
		TEXT("DA_Demo_NotificationCategory"),
		TEXT("10. Systems & Presentation"),
		4,
		TEXT("Define notification category policy, priority, coalescing, and dismissal behavior"),
		TEXT("PGXEditor.Icon.UI")
	},
	{
		TEXT("Widget Pool Profile"),
		TEXT("/Script/PGXUIRuntime.PGXWidgetPoolProfile"),
		TEXT("DA_Demo_WidgetPoolExample"),
		TEXT("10. Systems & Presentation"),
		5,
		TEXT("Define widget pool identity, capacity, reuse, and reset validation policy"),
		TEXT("PGXEditor.Icon.UI")
	},

	// ─── 11. Demo Gameplay Assets ───
	//
	//  Live — 15 DataAssets: 5 items + 5 NPCs + 5 configs.
	// Created through editor tooling rather than native C++ asset construction:
	//   BP Classes (5):  BP_HarnessCharacter, BP_HarnessNPC, BP_HarnessItem,
	//                     BP_HarnessInteractable, BP_HarnessSpawner
	//   Widgets (2):      WBP_HarnessHUD, WBP_HarnessInventory
	//   Behavior Tree:    BT_HarnessAI
	//   Input Mapping:    IMC_Harness
	//   Abilities (4):    GA_SwingSword, GA_CastHeal, GE_SwordDamage, GE_HealEffect
	// Crear en editor through editor tooling cuando project owner abra el proyecto. .
	//
	{
		TEXT("Iron Sword"),
		TEXT("/Script/PGXCoreRuntime.PGXObjectDataAsset"),
		TEXT("DA_Item_IronSword"),
		TEXT("11. Demo Gameplay Assets"),
		0,
		TEXT("An iron sword — basic melee weapon for early-game combat"),
		TEXT("ClassIcon.Weapon")
	},
	{
		TEXT("Health Potion"),
		TEXT("/Script/PGXCoreRuntime.PGXObjectDataAsset"),
		TEXT("DA_Item_HealthPotion"),
		TEXT("11. Demo Gameplay Assets"),
		1,
		TEXT("A health potion — consumable healing item"),
		TEXT("ClassIcon.Pickup")
	},
	{
		TEXT("Mage Staff"),
		TEXT("/Script/PGXCoreRuntime.PGXObjectDataAsset"),
		TEXT("DA_Item_MageStaff"),
		TEXT("11. Demo Gameplay Assets"),
		2,
		TEXT("A mage staff — magical ranged weapon"),
		TEXT("ClassIcon.Weapon")
	},
	{
		TEXT("Dragon Guard"),
		TEXT("/Script/PGXCoreRuntime.PGXObjectDataAsset"),
		TEXT("DA_Item_DragonShield"),
		TEXT("11. Demo Gameplay Assets"),
		3,
		TEXT("A dragon shield — high-defense protective item"),
		TEXT("ClassIcon.Guard")
	},
	{
		TEXT("Fire Scroll"),
		TEXT("/Script/PGXCoreRuntime.PGXObjectDataAsset"),
		TEXT("DA_Item_FireScroll"),
		TEXT("11. Demo Gameplay Assets"),
		4,
		TEXT("A fire scroll — consumable magic attack item"),
		TEXT("ClassIcon.Pickup")
	},

	{
		TEXT("Merchant"),
		TEXT("/Script/PGXCoreRuntime.PGXObjectDataAsset"),
		TEXT("DA_NPC_Merchant"),
		TEXT("11. Demo Gameplay Assets"),
		5,
		TEXT("A traveling merchant — buys and sells goods with players"),
		TEXT("ClassIcon.AIController")
	},
	{
		TEXT("Blacksmith"),
		TEXT("/Script/PGXCoreRuntime.PGXObjectDataAsset"),
		TEXT("DA_NPC_Blacksmith"),
		TEXT("11. Demo Gameplay Assets"),
		6,
		TEXT("A village blacksmith — crafts and upgrades equipment"),
		TEXT("ClassIcon.AIController")
	},
	{
		TEXT("Guard"),
		TEXT("/Script/PGXCoreRuntime.PGXObjectDataAsset"),
		TEXT("DA_NPC_Guard"),
		TEXT("11. Demo Gameplay Assets"),
		7,
		TEXT("A town guard — patrols and defends settlements"),
		TEXT("ClassIcon.AIController")
	},
	{
		TEXT("Quest Giver"),
		TEXT("/Script/PGXCoreRuntime.PGXObjectDataAsset"),
		TEXT("DA_NPC_QuestGiver"),
		TEXT("11. Demo Gameplay Assets"),
		8,
		TEXT("A quest giver — offers narrative missions to players"),
		TEXT("ClassIcon.AIController")
	},
	{
		TEXT("Innkeeper"),
		TEXT("/Script/PGXCoreRuntime.PGXObjectDataAsset"),
		TEXT("DA_NPC_Innkeeper"),
		TEXT("11. Demo Gameplay Assets"),
		9,
		TEXT("An innkeeper — provides rest and services to travelers"),
		TEXT("ClassIcon.AIController")
	},

	{
		TEXT("Harness Profile (Project Config)"),
		TEXT("/Script/PGXCoreRuntime.PGXProjectProfileConfig"),
		TEXT("DA_HarnessProfile"),
		TEXT("11. Demo Gameplay Assets"),
		10,
		TEXT("Configure harness MGOS baseline parameters for GC monitoring"),
		TEXT("ClassIcon.Object")
	},
	{
		TEXT("Harness Audio Config"),
		TEXT("/Script/PGXAudioRuntime.PGXAudioChannelConfig"),
		TEXT("DA_HarnessAudioConfig"),
		TEXT("11. Demo Gameplay Assets"),
		11,
		TEXT("Configure an audio channel for the demo harness"),
		TEXT("ClassIcon.SoundCue")
	},
	{
		TEXT("Harness Input Config"),
		TEXT("/Script/PGXInputRuntime.PGXInputConfig"),
		TEXT("DA_HarnessInputConfig"),
		TEXT("11. Demo Gameplay Assets"),
		12,
		TEXT("Configure input buffering and device policy for the demo harness"),
		TEXT("ClassIcon.Input")
	},
	{
		TEXT("Test Wagon (Vehicle)"),
		TEXT("/Script/PGXVehiclesRuntime.PGXVehicleDefinitionAsset"),
		TEXT("DA_Vehicle_TestWagon"),
		TEXT("11. Demo Gameplay Assets"),
		13,
		TEXT("Define a test wagon archetype for vehicle simulation"),
		TEXT("PGXEditor.Icon.Vehicles")
	},
	{
		TEXT("Iron Sword Recipe"),
		TEXT("/Script/PGXCraftingRuntime.PGXRecipeDefinition"),
		TEXT("DA_Recipe_IronSword"),
		TEXT("11. Demo Gameplay Assets"),
		14,
		TEXT("Define the crafting recipe for an Iron Sword"),
		TEXT("PGXEditor.Icon.Crafting")
	},
};

const TArray<FPGXDemoEntry>& FPGXDemoRegistry::GetDemoEntries()
{
	return GDemoEntries;
}

TArray<FString> FPGXDemoRegistry::GetDemoCategories()
{
	TArray<FString> Categories;
	TSet<FString> Seen;
	for (const FPGXDemoEntry& Entry : GDemoEntries)
	{
		if (!Seen.Contains(Entry.Category))
		{
			Seen.Add(Entry.Category);
			Categories.Add(Entry.Category);
		}
	}
	return Categories;
}


// EN: Get entries for a specific category, sorted by OrderInCategory.
// ES: Obtener entradas para una categoria especifica, ordenadas por OrderInCategory.
TArray<FPGXDemoEntry> FPGXDemoRegistry::GetEntriesForCategory(const FString& Category)
{
	TArray<FPGXDemoEntry> Result;
	for (const FPGXDemoEntry& Entry : GDemoEntries)
	{
		if (Entry.Category == Category)
		{
			Result.Add(Entry);
		}
	}
	// EN: Sort by OrderInCategory / ES: Ordenar por OrderInCategory
	// ES: El orden se asigna manualmente en la lista de entradas para controlar el orden de visualización dentro de cada categoría.
	Result.Sort([](const FPGXDemoEntry& A, const FPGXDemoEntry& B)
	{
		return A.OrderInCategory < B.OrderInCategory;
	});
	return Result;
}
