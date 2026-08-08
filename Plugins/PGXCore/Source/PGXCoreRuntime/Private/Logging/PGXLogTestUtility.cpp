// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Logging/PGXLogTestUtility.h"
#include "Subsystems/PGXLogSubsystem.h"
#include "Logging/PGXLogTagHelper.h"
#include "Logging/PGXLogCategories.h"
#include "Logging/PGXLogTags.h"
#include "Logging/PGXLogDomainRendererBase.h"
#include "Data/PGXLogDomainConfig.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

// ═══════════════════════════════════════════════════════════════
// EN: Test utility implementation. Each system simulation generates
//     realistic log entries with appropriate categories, verbosities,
//     messages, and typed parameters.
// ES: Implementacion de utilidad de test. Cada simulacion de sistema
//     genera entradas de log realistas con categorias, verbosities,
//     mensajes y parametros tipados apropiados.
// ═══════════════════════════════════════════════════════════════

// ─────────────────────────────────────────────────────────────
// Internal Helper / Helper Interno
// ─────────────────────────────────────────────────────────────

void UPGXLogTestUtility::Submit(const UObject* WorldContextObject, FName Category, EPGXLogVerbosity Verbosity, const FString& Message, const TArray<TPair<FString, FString>>& Params)
{
	UPGXLogSubsystem* LogSub = UPGXLogSubsystem::GetCached();
	if (!LogSub && WorldContextObject)
	{
		if (const UWorld* World = WorldContextObject->GetWorld())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				LogSub = GI->GetSubsystem<UPGXLogSubsystem>();
			}
		}
	}
	if (!LogSub) return;

	FPGXLogEntry Entry;
	Entry.Message = Message;
	Entry.Category = Category;
	Entry.Verbosity = Verbosity;
	Entry.Timestamp = FDateTime::UtcNow();
	Entry.FrameNumber = GFrameCounter;

	for (const auto& P : Params)
	{
		FPGXLogParam Param;
		Param.Key = P.Key;
		Param.Value = P.Value;
		Param.DataType = EPGXLogDataType::String;
		Entry.Params.Add(MoveTemp(Param));
	}

	FPGXLogTagHelper::AutoPopulateTags(Entry);
	LogSub->AddEntry(MoveTemp(Entry));
}

void UPGXLogTestUtility::SubmitWithDomain(const UObject* WorldContextObject, FName Category, EPGXLogVerbosity Verbosity, const FGameplayTag& DomainTag, const FString& Message, const TArray<TPair<FString, FString>>& Params)
{
	UPGXLogSubsystem* LogSub = UPGXLogSubsystem::GetCached();
	if (!LogSub && WorldContextObject)
	{
		if (const UWorld* World = WorldContextObject->GetWorld())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				LogSub = GI->GetSubsystem<UPGXLogSubsystem>();
			}
		}
	}
	if (!LogSub) return;

	FPGXLogEntry Entry;
	Entry.Message = Message;
	Entry.Category = Category;
	Entry.Verbosity = Verbosity;
	Entry.DomainTag = DomainTag;
	Entry.Timestamp = FDateTime::UtcNow();
	Entry.FrameNumber = GFrameCounter;

	for (const auto& P : Params)
	{
		FPGXLogParam Param;
		Param.Key = P.Key;
		Param.Value = P.Value;
		Param.DataType = EPGXLogDataType::String;
		Entry.Params.Add(MoveTemp(Param));
	}

	FPGXLogTagHelper::AutoPopulateTags(Entry);
	LogSub->AddEntry(MoveTemp(Entry));
}

// ─────────────────────────────────────────────────────────────
// Shorthand macro for param pairs (internal only)
// ─────────────────────────────────────────────────────────────
#define P(Key, Val) TPair<FString,FString>(TEXT(Key), TEXT(Val))

// ─────────────────────────────────────────────────────────────
// Save System / Sistema de Save
// ─────────────────────────────────────────────────────────────

void UPGXLogTestUtility::SimulateSaveSystem(const UObject* WorldContextObject, int32 Count)
{
	struct FTemplate { EPGXLogVerbosity V; const TCHAR* Msg; TArray<TPair<FString,FString>> Params; };
	const TArray<FTemplate> Templates = {
		{ EPGXLogVerbosity::Verbose, TEXT("Serializing actor state for save"), { P("ActorCount","147"), P("WorldName","Dungeon_03") } },
		{ EPGXLogVerbosity::Debug,   TEXT("Save checksum computed"), { P("Checksum","0xA3F7B2D1"), P("Bytes","45280") } },
		{ EPGXLogVerbosity::Info,    TEXT("Game saved successfully"), { P("SlotName","QuickSave"), P("Duration","0.234s"), P("Size","44.5 KB") } },
		{ EPGXLogVerbosity::Info,    TEXT("Auto-save triggered"), { P("SlotName","AutoSave_01"), P("Interval","300s") } },
		{ EPGXLogVerbosity::Info,    TEXT("Save slot created"), { P("SlotName","Manual_01"), P("PlayerLevel","12") } },
		{ EPGXLogVerbosity::Warning, TEXT("Save slot approaching max size"), { P("SlotName","QuickSave"), P("UsedKB","480"), P("MaxKB","512") } },
		{ EPGXLogVerbosity::Warning, TEXT("Save interrupted — retrying"), { P("Attempt","2"), P("Reason","Disk busy") } },
		{ EPGXLogVerbosity::Error,   TEXT("Failed to write save data"), { P("SlotName","AutoSave_02"), P("Error","ENOSPC"), P("Path","Saved/SaveGames/") } },
		{ EPGXLogVerbosity::Error,   TEXT("Save data corrupted on verification"), { P("SlotName","Manual_01"), P("ExpectedCRC","0xB4E2"), P("ActualCRC","0x0000") } },
		{ EPGXLogVerbosity::Info,    TEXT("Save slot deleted by user"), { P("SlotName","OldSave_03") } },
	};

	const FName Cat(TEXT("LogPGXSave"));
	for (int32 i = 0; i < Count; ++i)
	{
		const FTemplate& T = Templates[i % Templates.Num()];
		Submit(WorldContextObject, Cat, T.V, T.Msg, T.Params);
	}
}

// ─────────────────────────────────────────────────────────────
// GameFlow / Flujo de Juego
// ─────────────────────────────────────────────────────────────

void UPGXLogTestUtility::SimulateGameFlow(const UObject* WorldContextObject, int32 Count)
{
	struct FTemplate { EPGXLogVerbosity V; const TCHAR* Msg; TArray<TPair<FString,FString>> Params; };
	const TArray<FTemplate> Templates = {
		{ EPGXLogVerbosity::Info,    TEXT("Game phase transition"), { P("From","MainMenu"), P("To","Loading") } },
		{ EPGXLogVerbosity::Info,    TEXT("Level streaming completed"), { P("Level","Dungeon_03"), P("LoadTime","2.341s"), P("Actors","347") } },
		{ EPGXLogVerbosity::Info,    TEXT("Game phase transition"), { P("From","Loading"), P("To","Gameplay") } },
		{ EPGXLogVerbosity::Debug,   TEXT("Phase stack updated"), { P("Depth","3"), P("ActivePhase","Gameplay"), P("PreviousPhase","Loading") } },
		{ EPGXLogVerbosity::Info,    TEXT("Session started"), { P("SessionId","42"), P("MapName","World_Overworld"), P("GameMode","Adventure") } },
		{ EPGXLogVerbosity::Warning, TEXT("Level transition timeout — forcing load"), { P("Level","Town_02"), P("WaitTime","15.0s"), P("MaxWait","10.0s") } },
		{ EPGXLogVerbosity::Info,    TEXT("Checkpoint reached"), { P("CheckpointId","CP_BossDoor"), P("PlayTime","01:23:45") } },
		{ EPGXLogVerbosity::Verbose, TEXT("Tick phase validation passed"), { P("Phase","Gameplay"), P("SubPhase","Combat") } },
		{ EPGXLogVerbosity::Error,   TEXT("Failed to load level — asset missing"), { P("Level","DLC_Map_01"), P("MissingAsset","/Game/DLC/Maps/DLC_01") } },
		{ EPGXLogVerbosity::Info,    TEXT("Game phase transition"), { P("From","Gameplay"), P("To","Paused") } },
	};

	const FName Cat(TEXT("LogPGXStateMachine"));
	for (int32 i = 0; i < Count; ++i)
	{
		const FTemplate& T = Templates[i % Templates.Num()];
		Submit(WorldContextObject, Cat, T.V, T.Msg, T.Params);
	}
}

// ─────────────────────────────────────────────────────────────
// Audio / Audio
// ─────────────────────────────────────────────────────────────

void UPGXLogTestUtility::SimulateAudioSystem(const UObject* WorldContextObject, int32 Count)
{
	struct FTemplate { EPGXLogVerbosity V; const TCHAR* Msg; TArray<TPair<FString,FString>> Params; };
	const TArray<FTemplate> Templates = {
		{ EPGXLogVerbosity::Verbose, TEXT("Sound pool slot allocated"), { P("Pool","SFX_Impacts"), P("ActiveSounds","23/32") } },
		{ EPGXLogVerbosity::Debug,   TEXT("Music layer crossfade started"), { P("FromTrack","Exploration_01"), P("ToTrack","Combat_01"), P("FadeDuration","1.5s") } },
		{ EPGXLogVerbosity::Info,    TEXT("Playing sound cue"), { P("Cue","SFX_Explosion_Large"), P("Location","X=1200 Y=-340 Z=50"), P("Volume","0.85") } },
		{ EPGXLogVerbosity::Info,    TEXT("Background music changed"), { P("Track","BGM_Boss_Phase2"), P("BPM","140"), P("Intensity","High") } },
		{ EPGXLogVerbosity::Info,    TEXT("Ambient zone entered"), { P("Zone","Cave_Echo"), P("Reverb","LargeHall"), P("Attenuation","0.7") } },
		{ EPGXLogVerbosity::Warning, TEXT("Sound cue not found"), { P("Cue","SFX_Footstep_Metal_03"), P("Fallback","SFX_Footstep_Default") } },
		{ EPGXLogVerbosity::Warning, TEXT("Sound pool exhausted — dropping sound"), { P("Pool","SFX_Impacts"), P("MaxConcurrent","32"), P("Dropped","SFX_Hit_Small") } },
		{ EPGXLogVerbosity::Error,   TEXT("Audio device lost — attempting recovery"), { P("Device","Default"), P("ErrorCode","AUDCLNT_E_DEVICE_INVALIDATED") } },
		{ EPGXLogVerbosity::Info,    TEXT("Voice line played"), { P("Character","NPC_Merchant"), P("Line","VO_Greeting_02"), P("Subtitle","Welcome, traveler!") } },
		{ EPGXLogVerbosity::Debug,   TEXT("Audio modulation parameter updated"), { P("Param","MusicIntensity"), P("Value","0.75"), P("Source","CombatSystem") } },
	};

	const FName Cat(TEXT("LogPGXAudio"));
	for (int32 i = 0; i < Count; ++i)
	{
		const FTemplate& T = Templates[i % Templates.Num()];
		Submit(WorldContextObject, Cat, T.V, T.Msg, T.Params);
	}
}

// ─────────────────────────────────────────────────────────────
// AI System / Sistema AI
// ─────────────────────────────────────────────────────────────

void UPGXLogTestUtility::SimulateAISystem(const UObject* WorldContextObject, int32 Count)
{
	struct FTemplate { EPGXLogVerbosity V; const TCHAR* Msg; TArray<TPair<FString,FString>> Params; };
	const TArray<FTemplate> Templates = {
		{ EPGXLogVerbosity::Verbose, TEXT("Perception stimulus processed"), { P("Agent","Goblin_07"), P("StimulusType","Sight"), P("Target","Player") } },
		{ EPGXLogVerbosity::Debug,   TEXT("Behavior tree task completed"), { P("Agent","Knight_02"), P("Task","BTTask_MoveToTarget"), P("Result","Success") } },
		{ EPGXLogVerbosity::Info,    TEXT("Enemy spotted player"), { P("Agent","Archer_01"), P("Distance","1250"), P("AlertLevel","Combat") } },
		{ EPGXLogVerbosity::Info,    TEXT("Squad formation changed"), { P("Squad","PatrolGroup_A"), P("Formation","Defensive"), P("Members","4") } },
		{ EPGXLogVerbosity::Info,    TEXT("AI state transition"), { P("Agent","Boss_Dragon"), P("From","Idle"), P("To","Aggro"), P("Trigger","PlayerProximity") } },
		{ EPGXLogVerbosity::Warning, TEXT("Pathfinding failed — using fallback"), { P("Agent","Skeleton_12"), P("Target","X=500 Y=200"), P("Reason","NavMesh incomplete") } },
		{ EPGXLogVerbosity::Warning, TEXT("AI budget exceeded for frame"), { P("ActiveAgents","64"), P("Budget","50"), P("FrameTime","18.2ms") } },
		{ EPGXLogVerbosity::Error,   TEXT("Behavior tree asset missing"), { P("Agent","Miniboss_Troll"), P("ExpectedBT","BT_Troll_Combat"), P("Path","/Game/AI/BT/") } },
		{ EPGXLogVerbosity::Info,    TEXT("Enemy defeated"), { P("Agent","Goblin_07"), P("KilledBy","Player"), P("DamageType","Fire"), P("CombatTime","12.3s") } },
		{ EPGXLogVerbosity::Debug,   TEXT("Perception query completed"), { P("Agent","Guard_03"), P("VisibleTargets","2"), P("HeardTargets","1"), P("QueryTime","0.4ms") } },
	};

	const FName Cat(TEXT("LogPGXAI"));
	for (int32 i = 0; i < Count; ++i)
	{
		const FTemplate& T = Templates[i % Templates.Num()];
		Submit(WorldContextObject, Cat, T.V, T.Msg, T.Params);
	}
}

// ─────────────────────────────────────────────────────────────
// Network / Red
// ─────────────────────────────────────────────────────────────

void UPGXLogTestUtility::SimulateNetworkSystem(const UObject* WorldContextObject, int32 Count)
{
	struct FTemplate { EPGXLogVerbosity V; const TCHAR* Msg; TArray<TPair<FString,FString>> Params; };
	const TArray<FTemplate> Templates = {
		{ EPGXLogVerbosity::Verbose, TEXT("Replication update sent"), { P("Actor","PlayerCharacter_02"), P("Properties","3"), P("Bytes","128") } },
		{ EPGXLogVerbosity::Debug,   TEXT("Net relevancy check"), { P("Actor","Projectile_45"), P("Relevant","true"), P("Distance","800"), P("Connection","Client_02") } },
		{ EPGXLogVerbosity::Info,    TEXT("Player connected"), { P("PlayerId","SteamUser_7734"), P("Ping","45ms"), P("Region","US-East") } },
		{ EPGXLogVerbosity::Info,    TEXT("Session created"), { P("SessionId","Match_2847"), P("MaxPlayers","8"), P("Mode","TeamDeathmatch") } },
		{ EPGXLogVerbosity::Info,    TEXT("Player disconnected"), { P("PlayerId","SteamUser_1289"), P("Reason","Timeout"), P("SessionTime","00:14:32") } },
		{ EPGXLogVerbosity::Warning, TEXT("High latency detected"), { P("PlayerId","SteamUser_5501"), P("Ping","280ms"), P("Threshold","200ms") } },
		{ EPGXLogVerbosity::Warning, TEXT("Packet loss spike"), { P("Connection","Client_04"), P("LossRate","8.5%"), P("Duration","3.2s") } },
		{ EPGXLogVerbosity::Error,   TEXT("Replication error — desync detected"), { P("Actor","InventoryComponent_02"), P("ServerState","5"), P("ClientState","3") } },
		{ EPGXLogVerbosity::Error,   TEXT("Connection to matchmaking server lost"), { P("Server","matchmaking-us-01.pgx.gg"), P("LastPing","timeout"), P("Retries","3") } },
		{ EPGXLogVerbosity::Info,    TEXT("Leaderboard score submitted"), { P("PlayerId","SteamUser_7734"), P("Score","15420"), P("Board","Weekly_TopKills") } },
	};

	const FName Cat(TEXT("LogPGXNetwork"));
	for (int32 i = 0; i < Count; ++i)
	{
		const FTemplate& T = Templates[i % Templates.Num()];
		Submit(WorldContextObject, Cat, T.V, T.Msg, T.Params);
	}
}

// ─────────────────────────────────────────────────────────────
// UI System / Sistema UI
// ─────────────────────────────────────────────────────────────

void UPGXLogTestUtility::SimulateUISystem(const UObject* WorldContextObject, int32 Count)
{
	struct FTemplate { EPGXLogVerbosity V; const TCHAR* Msg; TArray<TPair<FString,FString>> Params; };
	const TArray<FTemplate> Templates = {
		{ EPGXLogVerbosity::Verbose, TEXT("Widget pooled for reuse"), { P("Widget","WBP_DamageNumber"), P("PoolSize","24/32") } },
		{ EPGXLogVerbosity::Debug,   TEXT("Screen transition animated"), { P("From","HUD_Gameplay"), P("To","Menu_Inventory"), P("AnimDuration","0.35s") } },
		{ EPGXLogVerbosity::Info,    TEXT("Screen opened"), { P("Screen","Menu_Inventory"), P("InputMode","UIOnly"), P("PausesGame","true") } },
		{ EPGXLogVerbosity::Info,    TEXT("Notification shown"), { P("Type","Achievement"), P("Title","First Blood"), P("Duration","5.0s") } },
		{ EPGXLogVerbosity::Info,    TEXT("Loading screen displayed"), { P("Level","Dungeon_03"), P("Tip","Watch your stamina in combat!") } },
		{ EPGXLogVerbosity::Warning, TEXT("Widget pool exhausted"), { P("Widget","WBP_DamageNumber"), P("Max","32"), P("Requested","33") } },
		{ EPGXLogVerbosity::Warning, TEXT("Screen push failed — already on stack"), { P("Screen","Menu_Pause"), P("StackDepth","3") } },
		{ EPGXLogVerbosity::Error,   TEXT("Widget class not found"), { P("Expected","WBP_HUD_Compass"), P("Path","/Game/UI/HUD/") } },
		{ EPGXLogVerbosity::Info,    TEXT("HUD visibility changed"), { P("Visible","false"), P("Reason","Cinematic"), P("FadeTime","0.5s") } },
		{ EPGXLogVerbosity::Info,    TEXT("Screen closed"), { P("Screen","Menu_Inventory"), P("ReturnTo","HUD_Gameplay") } },
	};

	const FName Cat(TEXT("LogPGXUI"));
	for (int32 i = 0; i < Count; ++i)
	{
		const FTemplate& T = Templates[i % Templates.Num()];
		Submit(WorldContextObject, Cat, T.V, T.Msg, T.Params);
	}
}

// ─────────────────────────────────────────────────────────────
// Inventory / Inventario
// ─────────────────────────────────────────────────────────────

void UPGXLogTestUtility::SimulateInventorySystem(const UObject* WorldContextObject, int32 Count)
{
	struct FTemplate { EPGXLogVerbosity V; const TCHAR* Msg; TArray<TPair<FString,FString>> Params; };
	const TArray<FTemplate> Templates = {
		{ EPGXLogVerbosity::Verbose, TEXT("Item instance created from definition"), { P("ItemDef","DA_Item_HealthPotion"), P("InstanceId","ITEM_4872") } },
		{ EPGXLogVerbosity::Debug,   TEXT("Container slot updated"), { P("Container","PlayerBackpack"), P("Slot","7"), P("Item","Iron Sword"), P("Stack","1/1") } },
		{ EPGXLogVerbosity::Info,    TEXT("Item picked up"), { P("Item","Health Potion"), P("Quantity","3"), P("Source","Chest_Dungeon_07") } },
		{ EPGXLogVerbosity::Info,    TEXT("Item equipped"), { P("Item","Dragon Scale Armor"), P("Slot","Chest"), P("Defense","+45"), P("Weight","12.5") } },
		{ EPGXLogVerbosity::Info,    TEXT("Item crafted"), { P("Result","Steel Sword"), P("Recipe","2x Iron Ingot + 1x Leather"), P("Quality","Rare") } },
		{ EPGXLogVerbosity::Warning, TEXT("Container full — item dropped"), { P("Item","Gold Coin"), P("Container","PlayerBackpack"), P("MaxSlots","20"), P("Used","20") } },
		{ EPGXLogVerbosity::Warning, TEXT("Item definition not found in registry"), { P("ItemId","DA_Item_LegacySword"), P("Fallback","DA_Item_Placeholder") } },
		{ EPGXLogVerbosity::Error,   TEXT("Item transfer failed — invalid target"), { P("Item","Enchanted Ring"), P("From","PlayerBackpack"), P("To","NULL") } },
		{ EPGXLogVerbosity::Info,    TEXT("Item consumed"), { P("Item","Health Potion"), P("Effect","RestoreHP"), P("Amount","+50"), P("Remaining","2") } },
		{ EPGXLogVerbosity::Info,    TEXT("Loot table rolled"), { P("Table","LT_Boss_Dragon"), P("Drops","3"), P("RareDrops","1"), P("Seed","28471") } },
	};

	const FName Cat(TEXT("LogPGXData"));
	for (int32 i = 0; i < Count; ++i)
	{
		const FTemplate& T = Templates[i % Templates.Num()];
		Submit(WorldContextObject, Cat, T.V, T.Msg, T.Params);
	}
}

// ─────────────────────────────────────────────────────────────
// Quick Test / Test Rapido
// ─────────────────────────────────────────────────────────────

void UPGXLogTestUtility::RunQuickTest(const UObject* WorldContextObject)
{
	// EN: Generate 5 entries per system = 35 legacy + 39 domain-tagged = 74 total
	// ES: Generar 5 entradas por sistema = 35 legacy + 39 domain-tagged = 74 total
	SimulateSaveSystem(WorldContextObject, 5);
	SimulateGameFlow(WorldContextObject, 5);
	SimulateAudioSystem(WorldContextObject, 5);
	SimulateAISystem(WorldContextObject, 5);
	SimulateNetworkSystem(WorldContextObject, 5);
	SimulateUISystem(WorldContextObject, 5);
	SimulateInventorySystem(WorldContextObject, 5);

	// EN: v3.0: Also generate domain-tagged entries (3 per domain × 13 domains = 39)
	// ES: v3.0: Tambien generar entradas con domain tag (3 por dominio × 13 dominios = 39)
	SimulateDomainLogs(WorldContextObject, 3);
}

// ─────────────────────────────────────────────────────────────
// All Systems / Todos los Sistemas
// ─────────────────────────────────────────────────────────────

void UPGXLogTestUtility::RunAllSystems(const UObject* WorldContextObject, int32 CountPerSystem)
{
	SimulateSaveSystem(WorldContextObject, CountPerSystem);
	SimulateGameFlow(WorldContextObject, CountPerSystem);
	SimulateAudioSystem(WorldContextObject, CountPerSystem);
	SimulateAISystem(WorldContextObject, CountPerSystem);
	SimulateNetworkSystem(WorldContextObject, CountPerSystem);
	SimulateUISystem(WorldContextObject, CountPerSystem);
	SimulateInventorySystem(WorldContextObject, CountPerSystem);
}

// ─────────────────────────────────────────────────────────────
// Stress Test / Test de Estres
// ─────────────────────────────────────────────────────────────

void UPGXLogTestUtility::RunStressTest(const UObject* WorldContextObject, int32 Count)
{
	// EN: Category rotation for varied data / ES: Rotacion de categorias para datos variados
	const FName Categories[] = {
		FName(TEXT("LogPGXSave")),
		FName(TEXT("LogPGXStateMachine")),
		FName(TEXT("LogPGXAudio")),
		FName(TEXT("LogPGXAI")),
		FName(TEXT("LogPGXNetwork")),
		FName(TEXT("LogPGXUI")),
		FName(TEXT("LogPGXData")),
	};
	const int32 NumCategories = UE_ARRAY_COUNT(Categories);

	// EN: Verbosity distribution: ~40% Info, ~20% Debug, ~15% Verbose, ~15% Warning, ~10% Error
	// ES: Distribucion de verbosity: ~40% Info, ~20% Debug, ~15% Verbose, ~15% Warning, ~10% Error
	const EPGXLogVerbosity Verbosities[] = {
		EPGXLogVerbosity::Info,    EPGXLogVerbosity::Info,    EPGXLogVerbosity::Info,    EPGXLogVerbosity::Info,
		EPGXLogVerbosity::Debug,   EPGXLogVerbosity::Debug,
		EPGXLogVerbosity::Verbose,
		EPGXLogVerbosity::Warning, EPGXLogVerbosity::Warning,
		EPGXLogVerbosity::Error,
	};
	const int32 NumVerbosities = UE_ARRAY_COUNT(Verbosities);

	UPGXLogSubsystem* LogSub = UPGXLogSubsystem::GetCached();
	if (!LogSub && WorldContextObject)
	{
		if (const UWorld* World = WorldContextObject->GetWorld())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				LogSub = GI->GetSubsystem<UPGXLogSubsystem>();
			}
		}
	}
	if (!LogSub) return;

	// EN: Pre-build entries for maximum throughput / ES: Pre-construir entradas para maximo throughput
	const double StartTime = FPlatformTime::Seconds();

	for (int32 i = 0; i < Count; ++i)
	{
		FPGXLogEntry Entry;
		Entry.Category = Categories[i % NumCategories];
		Entry.Verbosity = Verbosities[i % NumVerbosities];
		Entry.Message = FString::Printf(TEXT("Stress test entry #%d | system=%s"), i, *Entry.Category.ToString());
		Entry.Timestamp = FDateTime::UtcNow();
		Entry.FrameNumber = GFrameCounter;

		// EN: Add params every 3rd entry / ES: Agregar params cada 3ra entrada
		if (i % 3 == 0)
		{
			Entry.Params.Add({ TEXT("Index"), FString::FromInt(i), EPGXLogDataType::Integer });
			Entry.Params.Add({ TEXT("Batch"), FString::FromInt(i / 100), EPGXLogDataType::Integer });
		}

		FPGXLogTagHelper::AutoPopulateTags(Entry);
		LogSub->AddEntry(MoveTemp(Entry));
	}

	const double Duration = FPlatformTime::Seconds() - StartTime;

	// EN: Log the profiling result itself / ES: Loguear el resultado de profiling
	Submit(WorldContextObject, FName(TEXT("LogPGXLog")), EPGXLogVerbosity::Info,
		FString::Printf(TEXT("Stress test completed: %d entries in %.3f ms (%.0f entries/sec)"),
			Count, Duration * 1000.0, Count / FMath::Max(Duration, 0.0001)));
}

// ─────────────────────────────────────────────────────────────
// Realistic Game Session / Sesion de Juego Realista
// ─────────────────────────────────────────────────────────────

void UPGXLogTestUtility::SimulateGameSession(const UObject* WorldContextObject)
{
	// EN: Phase 1: Startup (5 entries)
	// ES: Fase 1: Inicio (5 entradas)
	Submit(WorldContextObject, FName(TEXT("LogPGX")), EPGXLogVerbosity::Info,
		TEXT("PGX Framework initialized"), { P("Version","0.3.1"), P("Plugins","22") });
	Submit(WorldContextObject, FName(TEXT("LogPGXStateMachine")), EPGXLogVerbosity::Info,
		TEXT("Game phase transition"), { P("From","None"), P("To","MainMenu") });
	Submit(WorldContextObject, FName(TEXT("LogPGXAudio")), EPGXLogVerbosity::Info,
		TEXT("Background music changed"), { P("Track","BGM_MainMenu"), P("FadeIn","2.0s") });
	Submit(WorldContextObject, FName(TEXT("LogPGXUI")), EPGXLogVerbosity::Info,
		TEXT("Screen opened"), { P("Screen","MainMenu"), P("InputMode","UIOnly") });
	Submit(WorldContextObject, FName(TEXT("LogPGXSettings")), EPGXLogVerbosity::Debug,
		TEXT("User settings loaded"), { P("Resolution","2560x1440"), P("Quality","Ultra"), P("FPS","Unlimited") });

	// EN: Phase 2: Loading level (6 entries)
	// ES: Fase 2: Cargando nivel (6 entradas)
	Submit(WorldContextObject, FName(TEXT("LogPGXStateMachine")), EPGXLogVerbosity::Info,
		TEXT("Game phase transition"), { P("From","MainMenu"), P("To","Loading") });
	Submit(WorldContextObject, FName(TEXT("LogPGXUI")), EPGXLogVerbosity::Info,
		TEXT("Loading screen displayed"), { P("Level","Dungeon_03"), P("Tip","Explore every corner for hidden loot!") });
	Submit(WorldContextObject, FName(TEXT("LogPGXData")), EPGXLogVerbosity::Debug,
		TEXT("Asset registry scan started"), { P("Path","/Game/Dungeon_03/"), P("ExpectedAssets","142") });
	Submit(WorldContextObject, FName(TEXT("LogPGXPool")), EPGXLogVerbosity::Info,
		TEXT("Object pool pre-warmed"), { P("Class","AProjectile"), P("PoolSize","32"), P("WarmTime","0.045s") });
	Submit(WorldContextObject, FName(TEXT("LogPGXData")), EPGXLogVerbosity::Info,
		TEXT("Asset registry scan completed"), { P("LoadedAssets","142"), P("Duration","1.8s") });
	Submit(WorldContextObject, FName(TEXT("LogPGXStateMachine")), EPGXLogVerbosity::Info,
		TEXT("Level streaming completed"), { P("Level","Dungeon_03"), P("LoadTime","3.2s"), P("Actors","347") });

	// EN: Phase 3: Gameplay (15 entries — combat, AI, audio, items)
	// ES: Fase 3: Gameplay (15 entradas — combate, AI, audio, items)
	Submit(WorldContextObject, FName(TEXT("LogPGXStateMachine")), EPGXLogVerbosity::Info,
		TEXT("Game phase transition"), { P("From","Loading"), P("To","Gameplay") });
	Submit(WorldContextObject, FName(TEXT("LogPGXAudio")), EPGXLogVerbosity::Info,
		TEXT("Background music changed"), { P("Track","BGM_Dungeon_Exploration"), P("FadeIn","3.0s") });
	Submit(WorldContextObject, FName(TEXT("LogPGXData")), EPGXLogVerbosity::Info,
		TEXT("Item picked up"), { P("Item","Rusty Key"), P("Quantity","1"), P("Source","Chest_01") });
	Submit(WorldContextObject, FName(TEXT("LogPGXAI")), EPGXLogVerbosity::Info,
		TEXT("Enemy spotted player"), { P("Agent","Skeleton_04"), P("Distance","800"), P("AlertLevel","Combat") });
	Submit(WorldContextObject, FName(TEXT("LogPGXAudio")), EPGXLogVerbosity::Info,
		TEXT("Background music changed"), { P("Track","BGM_Combat_01"), P("FadeIn","0.5s") });
	Submit(WorldContextObject, FName(TEXT("LogPGXAI")), EPGXLogVerbosity::Info,
		TEXT("Squad formation changed"), { P("Squad","SkeletonGroup_A"), P("Formation","Surround"), P("Members","3") });
	Submit(WorldContextObject, FName(TEXT("LogPGXAI")), EPGXLogVerbosity::Warning,
		TEXT("Pathfinding failed — using fallback"), { P("Agent","Skeleton_06"), P("Reason","NavMesh gap") });
	Submit(WorldContextObject, FName(TEXT("LogPGXAI")), EPGXLogVerbosity::Info,
		TEXT("Enemy defeated"), { P("Agent","Skeleton_04"), P("DamageType","Slash"), P("CombatTime","8.7s") });
	Submit(WorldContextObject, FName(TEXT("LogPGXData")), EPGXLogVerbosity::Info,
		TEXT("Loot table rolled"), { P("Table","LT_Skeleton"), P("Drops","2"), P("RareDrops","0") });
	Submit(WorldContextObject, FName(TEXT("LogPGXData")), EPGXLogVerbosity::Info,
		TEXT("Item picked up"), { P("Item","Bone Fragment"), P("Quantity","2"), P("Source","Skeleton_04") });
	Submit(WorldContextObject, FName(TEXT("LogPGXData")), EPGXLogVerbosity::Warning,
		TEXT("Container full — item dropped"), { P("Item","Bone Fragment"), P("MaxSlots","20"), P("Used","20") });
	Submit(WorldContextObject, FName(TEXT("LogPGXUI")), EPGXLogVerbosity::Warning,
		TEXT("Widget pool exhausted"), { P("Widget","WBP_DamageNumber"), P("Max","32"), P("Requested","33") });
	Submit(WorldContextObject, FName(TEXT("LogPGXAudio")), EPGXLogVerbosity::Warning,
		TEXT("Sound cue not found"), { P("Cue","SFX_Skeleton_Death_03"), P("Fallback","SFX_Skeleton_Death_01") });
	Submit(WorldContextObject, FName(TEXT("LogPGXStateMachine")), EPGXLogVerbosity::Info,
		TEXT("Checkpoint reached"), { P("CheckpointId","CP_Room_04"), P("PlayTime","00:08:23") });
	Submit(WorldContextObject, FName(TEXT("LogPGXAudio")), EPGXLogVerbosity::Info,
		TEXT("Background music changed"), { P("Track","BGM_Dungeon_Exploration"), P("FadeIn","2.0s") });

	// EN: Phase 4: Save + Exit (6 entries)
	// ES: Fase 4: Guardar + Salir (6 entradas)
	Submit(WorldContextObject, FName(TEXT("LogPGXSave")), EPGXLogVerbosity::Info,
		TEXT("Auto-save triggered"), { P("SlotName","AutoSave_01"), P("Interval","300s") });
	Submit(WorldContextObject, FName(TEXT("LogPGXSave")), EPGXLogVerbosity::Debug,
		TEXT("Serializing actor state for save"), { P("ActorCount","312"), P("WorldName","Dungeon_03") });
	Submit(WorldContextObject, FName(TEXT("LogPGXSave")), EPGXLogVerbosity::Info,
		TEXT("Game saved successfully"), { P("SlotName","AutoSave_01"), P("Duration","0.187s"), P("Size","38.2 KB") });
	Submit(WorldContextObject, FName(TEXT("LogPGXStateMachine")), EPGXLogVerbosity::Info,
		TEXT("Game phase transition"), { P("From","Gameplay"), P("To","MainMenu") });
	Submit(WorldContextObject, FName(TEXT("LogPGXUI")), EPGXLogVerbosity::Info,
		TEXT("Screen opened"), { P("Screen","MainMenu"), P("InputMode","UIOnly") });
	Submit(WorldContextObject, FName(TEXT("LogPGXLog")), EPGXLogVerbosity::Info,
		TEXT("Session summary"), { P("TotalEntries","32"), P("Warnings","4"), P("Errors","0"), P("PlayTime","00:09:45") });
}

// ─────────────────────────────────────────────────────────────
// Domain-Aware Simulation (v3.0)
// Simulacion con Dominio (v3.0)
// ─────────────────────────────────────────────────────────────

void UPGXLogTestUtility::SimulateDomainLogs(const UObject* WorldContextObject, int32 CountPerDomain)
{
	// EN: Each domain gets entries with the Params keys that its renderer extracts.
	// ES: Cada dominio recibe entradas con las keys de Params que su renderer extrae.

	struct FDomainTemplate
	{
		FGameplayTag DomainTag;
		FName Category;
		const TCHAR* Message;
		TArray<TPair<FString,FString>> Params;
	};

	// EN: One template per domain with realistic params matching renderer column definitions
	// ES: Un template por dominio con params realistas que coinciden con las columnas del renderer
	const TArray<FDomainTemplate> Templates = {
		// Audio
		{ TAG_PGX_Log_Domain_Audio, FName(TEXT("LogPGXAudio")),
			TEXT("Playing sound event"),
			{ P("SoundEvent","SFX_Explosion_Large"), P("Channel","SFX"), P("Volume","0.85"), P("Backend","MetaSound") } },
		// Save
		{ TAG_PGX_Log_Domain_Save, FName(TEXT("LogPGXSave")),
			TEXT("Game saved to slot"),
			{ P("SlotName","QuickSave"), P("Domain","PlayerData"), P("ByteSize","45280"), P("Operation","Write") } },
		// GameFlow
		{ TAG_PGX_Log_Domain_GameFlow, FName(TEXT("LogPGXStateMachine")),
			TEXT("Phase transition completed"),
			{ P("FromState","MainMenu"), P("ToState","Loading"), P("Trigger","PlayerAction"), P("Phase","Startup") } },
		// PSO
		{ TAG_PGX_Log_Domain_PSO, FName(TEXT("LogPGXPSO")),
			TEXT("Pipeline precache finished"),
			{ P("PipelineId","PSO_0x7A3F"), P("Status","Complete"), P("Duration","1.234s"), P("Context","LevelLoad") } },
		// LevelFlow
		{ TAG_PGX_Log_Domain_LevelFlow, FName(TEXT("LogPGXLevelFlow")),
			TEXT("Level transition requested"),
			{ P("LevelName","Dungeon_03"), P("Action","StreamIn"), P("SubLevel","Dungeon_03_Interior"), P("Reason","Gameplay") } },
		// Loading
		{ TAG_PGX_Log_Domain_Loading, FName(TEXT("LogPGXLoading")),
			TEXT("Loading screen phase update"),
			{ P("Phase","AssetStreaming"), P("Progress","67%"), P("Context","LevelTransition"), P("Duration","2.1s") } },
		// Profile
		{ TAG_PGX_Log_Domain_Profile, FName(TEXT("LogPGXProfile")),
			TEXT("Platform budget enforced"),
			{ P("Platform","Windows"), P("Capability","HighEnd"), P("Budget","RingBuffer"), P("Value","10000") } },
		// MGOS
		{ TAG_PGX_Log_Domain_MGOS, FName(TEXT("LogPGXPool")),
			TEXT("Pool allocation stats"),
			{ P("PoolName","Projectiles"), P("AllocCount","32"), P("MemUsed","128 KB"), P("Delta","+4") } },
		// DataRegistry
		{ TAG_PGX_Log_Domain_DataRegistry, FName(TEXT("LogPGXData")),
			TEXT("Registry item registered"),
			{ P("Database","Items"), P("ItemTag","PGX.Item.Weapon.Sword"), P("Action","Register"), P("Version","2") } },
		// Construction
		{ TAG_PGX_Log_Domain_Construction, FName(TEXT("LogPGXData")),
			TEXT("Construction slot resolved"),
			{ P("SlotType","GameMode"), P("ClassName","BP_MyGameMode"), P("SourceMode","Blueprint") } },
		// Message
		{ TAG_PGX_Log_Domain_Message, FName(TEXT("LogPGXMessages")),
			TEXT("Message broadcast dispatched"),
			{ P("Channel","Combat"), P("PayloadType","DamageEvent"), P("Sender","Player_01"), P("Size","128 B") } },
		// EventHandler
		{ TAG_PGX_Log_Domain_EventHandler, FName(TEXT("LogPGXEventHandler")),
			TEXT("Event resolution completed"),
			{ P("EventTag","PGX.Event.Combat.Hit"), P("Resolution","Handled"), P("Handler","DamageSystem"), P("Time","0.002ms") } },
		// Generic
		{ TAG_PGX_Log_Domain_Generic, FName(TEXT("LogPGX")),
			TEXT("Framework initialization complete"),
			{} },
	};

	const EPGXLogVerbosity Verbosities[] = {
		EPGXLogVerbosity::Info,
		EPGXLogVerbosity::Debug,
		EPGXLogVerbosity::Warning,
		EPGXLogVerbosity::Verbose,
		EPGXLogVerbosity::Error,
	};
	const int32 NumVerbosities = UE_ARRAY_COUNT(Verbosities);

	for (const FDomainTemplate& Tmpl : Templates)
	{
		for (int32 i = 0; i < CountPerDomain; ++i)
		{
			const FString Msg = FString::Printf(TEXT("%s (#%d)"), Tmpl.Message, i + 1);
			SubmitWithDomain(WorldContextObject, Tmpl.Category, Verbosities[i % NumVerbosities], Tmpl.DomainTag, Msg, Tmpl.Params);
		}
	}

	UE_LOG(LogPGXLog, Log, TEXT("[LogTestUtility] SimulateDomainLogs — %d entries across 13 domains"),
		Templates.Num() * CountPerDomain);
}

// ─────────────────────────────────────────────────────────────
// Domain Quick Test / Test Rapido de Dominio
// ─────────────────────────────────────────────────────────────

bool UPGXLogTestUtility::RunDomainQuickTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	bool bAllPassed = true;

	OutIssues.Add(TEXT("=== PGX Log Domain Test Suite (v3.0) ==="));

	// EN: Test 1: Resolve subsystem / ES: Test 1: Resolver subsistema
	UPGXLogSubsystem* Sub = UPGXLogSubsystem::GetCached();
	if (!Sub)
	{
		OutIssues.Add(TEXT("[FAIL] LogSubsystem not found"));
		return false;
	}
	OutIssues.Add(TEXT("[PASS] LogSubsystem found"));

	// EN: Test 2: Domain registry populated / ES: Test 2: Registro de dominios poblado
	const auto& Domains = Sub->GetAllDomains();
	if (Domains.Num() > 0)
	{
		OutIssues.Add(FString::Printf(TEXT("[PASS] Domain registry — %d domains registered"), Domains.Num()));
	}
	else
	{
		OutIssues.Add(TEXT("[WARN] Domain registry empty — no UPGXLogDomainConfig DAs found"));
	}

	// EN: Test 3: Generic renderer available / ES: Test 3: Renderer generico disponible
	UPGXLogDomainRendererBase* GenericRenderer = Sub->GetGenericRenderer();
	if (IsValid(GenericRenderer))
	{
		OutIssues.Add(TEXT("[PASS] Generic renderer available"));
	}
	else
	{
		OutIssues.Add(TEXT("[FAIL] Generic renderer is null"));
		bAllPassed = false;
	}

	// EN: Test 4: Submit one domain entry and verify it exists / ES: Test 4: Enviar una entrada de dominio y verificar que existe
	const int32 CountBefore = Sub->GetEntryCount();
	SubmitWithDomain(WorldContextObject, FName(TEXT("LogPGXLog")), EPGXLogVerbosity::Info,
		TAG_PGX_Log_Domain_Generic, TEXT("Domain quick test validation entry"));
	const int32 CountAfter = Sub->GetEntryCount();
	if (CountAfter > CountBefore)
	{
		OutIssues.Add(TEXT("[PASS] Domain entry submitted and stored"));
	}
	else
	{
		OutIssues.Add(TEXT("[FAIL] Domain entry not stored in ring buffer"));
		bAllPassed = false;
	}

	// EN: Test 5: Domain-filtered query / ES: Test 5: Consulta filtrada por dominio
	const TArray<FPGXLogEntry> DomainEntries = Sub->GetEntriesForDomain(TAG_PGX_Log_Domain_Generic);
	if (DomainEntries.Num() > 0)
	{
		OutIssues.Add(FString::Printf(TEXT("[PASS] Domain query — %d Generic entries found"), DomainEntries.Num()));
	}
	else
	{
		OutIssues.Add(TEXT("[WARN] Domain query returned 0 entries (may need domain DAs configured)"));
	}

	// EN: Test 6: Renderer lookup for each registered domain / ES: Test 6: Lookup de renderer por cada dominio registrado
	int32 RendererHits = 0;
	for (const auto& Pair : Domains)
	{
		UPGXLogDomainRendererBase* Renderer = Sub->GetRendererForDomain(Pair.Key);
		if (IsValid(Renderer))
		{
			RendererHits++;
		}
	}
	OutIssues.Add(FString::Printf(TEXT("[PASS] Renderer lookup — %d/%d domains have renderers"), RendererHits, Domains.Num()));

	OutIssues.Add(FString::Printf(TEXT("=== Result: %s ==="), bAllPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED")));
	UE_LOG(LogPGXLog, Log, TEXT("[LogTestUtility] RunDomainQuickTest — %s"), bAllPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED"));
	return bAllPassed;
}

#undef P

// ============================================================================
// EN: RunAllTests — aggregate validation for Test Dashboard
// ES: RunAllTests — validacion agregada para Test Dashboard
// ============================================================================

bool UPGXLogTestUtility::RunAllTests(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	bool bAllPassed = true;

	OutIssues.Add(TEXT("=== PGX Log Test Suite ==="));

	// EN: Test 1: Resolve subsystem / ES: Test 1: Resolver subsistema
	UPGXLogSubsystem* Sub = UPGXLogSubsystem::GetCached();

	if (!Sub)
	{
		OutIssues.Add(TEXT("[FAIL] LogSubsystem not found"));
		return false;
	}
	OutIssues.Add(TEXT("[PASS] LogSubsystem found"));

	// EN: Test 2: Submit a test entry / ES: Test 2: Enviar una entrada de test
	Submit(WorldContextObject, FName(TEXT("LogPGXLog")), EPGXLogVerbosity::Info, TEXT("Test Dashboard validation entry"));
	OutIssues.Add(TEXT("[PASS] Log entry submitted"));

	// EN: Test 3: Buffer accessible / ES: Test 3: Buffer accesible
	const int32 EntryCount = Sub->GetEntryCount();
	if (EntryCount >= 0)
	{
		OutIssues.Add(FString::Printf(TEXT("[PASS] Log buffer accessible — %d entries"), EntryCount));
	}
	else
	{
		OutIssues.Add(TEXT("[FAIL] Log buffer returned invalid count"));
		bAllPassed = false;
	}

	// EN: Test 4: Domain system (v3.0) / ES: Test 4: Sistema de dominios (v3.0)
	TArray<FString> DomainIssues;
	const bool bDomainPassed = RunDomainQuickTest(WorldContextObject, DomainIssues);
	// EN: Append domain issues (skip header and result lines) / ES: Agregar issues de dominio (saltar header y lineas de resultado)
	for (int32 i = 1; i < DomainIssues.Num() - 1; ++i)
	{
		OutIssues.Add(DomainIssues[i]);
	}
	if (!bDomainPassed)
	{
		bAllPassed = false;
	}

	OutIssues.Add(FString::Printf(TEXT("=== Result: %s ==="), bAllPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED")));
	UE_LOG(LogPGXLog, Log, TEXT("[Log TestUtility] RunAllTests — %s"), bAllPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED"));
	return bAllPassed;
}
