// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "AssetTools/PGXAssetCreationRegistry.h"

// ─── Blueprint Entries ───

static const TArray<FPGXCreatableAssetEntry> GBlueprintEntries = {
	// Core (6)
	{ TEXT("PGX Actor"),             TEXT("/Script/PGXCoreRuntime.PGXActorBase"),             TEXT("BP_PGXActor"),             TEXT("Core"),        TEXT("ClassIcon.Actor"),            true },
	{ TEXT("PGX Character"),         TEXT("/Script/PGXCoreRuntime.PGXCharacterBase"),         TEXT("BP_PGXCharacter"),         TEXT("Core"),        TEXT("ClassIcon.Character"),        true },
	{ TEXT("PGX GameMode"),          TEXT("/Script/PGXCoreRuntime.PGXGameModeBase"),          TEXT("BP_PGXGameMode"),          TEXT("Core"),        TEXT("ClassIcon.GameModeBase"),     true },
	{ TEXT("PGX PlayerController"),  TEXT("/Script/PGXCoreRuntime.PGXPlayerControllerBase"),  TEXT("BP_PGXPlayerController"),  TEXT("Core"),        TEXT("ClassIcon.PlayerController"), true },
	{ TEXT("PGX GameState"),         TEXT("/Script/PGXCoreRuntime.PGXGameStateBase"),         TEXT("BP_PGXGameState"),         TEXT("Core"),        TEXT("ClassIcon.GameStateBase"),    true },
	{ TEXT("PGX Component"),         TEXT("/Script/PGXCoreRuntime.PGXBaseComponent"),         TEXT("BP_PGXComponent"),         TEXT("Core"),        TEXT("ClassIcon.ActorComponent"),   true },
	{ TEXT("PGX Game Instance"),    TEXT("/Script/PGXCoreRuntime.PGXGameInstance"),          TEXT("BP_PGXGameInstance"),      TEXT("Core"),        TEXT("ClassIcon.GameInstance"),     true },
	{ TEXT("PGX Player State"),     TEXT("/Script/PGXCoreRuntime.PGXPlayerStateBase"),       TEXT("BP_PGXPlayerState"),       TEXT("Core"),        TEXT("ClassIcon.PlayerState"),      true },
	{ TEXT("PGX Pawn"),             TEXT("/Script/PGXCoreRuntime.PGXPawnBase"),              TEXT("BP_PGXPawn"),              TEXT("Core"),        TEXT("ClassIcon.Pawn"),             true },
	{ TEXT("PGX HUD"),              TEXT("/Script/PGXCoreRuntime.PGXCoreHUD"),               TEXT("BP_PGXHUD"),               TEXT("Core"),        TEXT("ClassIcon.HUD"),              true },

	// Loading (1)
	{ TEXT("PGX Loading Strategy"), TEXT("/Script/PGXLoadingRuntime.PGXLoadingStrategyBase"), TEXT("BP_PGXLoadingStrategy"), TEXT("Loading"), TEXT("ClassIcon.UserDefinedStruct"), true },

	// Audio (1)
	{ TEXT("PGX Audio Component"), TEXT("/Script/PGXAudioRuntime.PGXAudioComponent"), TEXT("BP_PGXAudioComponent"), TEXT("Audio"), TEXT("ClassIcon.AudioComponent"), true },

	// EventHandler (1)
	{ TEXT("PGX Event Handler"), TEXT("/Script/PGXCoreRuntime.PGXEventHandlerBase"), TEXT("BP_PGXEventHandler"), TEXT("EventHandler"), TEXT("PGXEditor.Icon.EventHandler"), true },

	// Log (1) — custom domain renderer
	{ TEXT("PGX Log Domain Renderer"), TEXT("/Script/PGXCoreRuntime.PGXLogDomainRendererBase"), TEXT("BP_PGXLogDomainRenderer"), TEXT("Log"), TEXT("ClassIcon.Object"), true },
};

// ─── DataAsset Entries (organized by system) ───

static const TArray<FPGXCreatableAssetEntry> GDataAssetEntries = {
	// Core (2) — generic PGX base DataAssets
	{ TEXT("Config DataAsset"),  TEXT("/Script/PGXCoreRuntime.PGXConfigDataAsset"),  TEXT("DA_PGXConfig"),  TEXT("Core"),  TEXT("ClassIcon.Object"), false, FColor(56, 103, 214) },
	{ TEXT("Object DataAsset"),  TEXT("/Script/PGXCoreRuntime.PGXObjectDataAsset"),  TEXT("DA_PGXObject"),  TEXT("Core"),  TEXT("ClassIcon.Object"), false, FColor(86, 174, 80) },

	// GameFlow (2)
	{ TEXT("GameFlow Config"),   TEXT("/Script/PGXGameFlowRuntime.PGXGameFlowConfig"),  TEXT("DA_PGXGameFlowConfig"), TEXT("GameFlow"), TEXT("ClassIcon.GameModeBase"), false, FColor(255, 152, 0) },
	{ TEXT("Flow Rules Config"), TEXT("/Script/PGXGameFlowRuntime.PGXFlowRulesConfig"), TEXT("DA_PGXFlowRules"),      TEXT("GameFlow"), TEXT("ClassIcon.GameModeBase"), false, FColor(255, 152, 0) },

	// PSO (1)
	{ TEXT("PSO WarmUp Config"), TEXT("/Script/PGXPSORuntime.PGXPSOWarmUpConfig"),   TEXT("DA_PGXPSOWarmUp"),     TEXT("PSO"),       TEXT("ClassIcon.Object"),           false, FColor(0, 188, 212) },

	// Save (1)
	{ TEXT("Save Config"),       TEXT("/Script/PGXSaveRuntime.PGXSaveConfig"),       TEXT("DA_PGXSaveConfig"),    TEXT("Save"),      TEXT("ClassIcon.SaveGame"),         false, FColor(76, 175, 80) },

	// Loading (4)
	{ TEXT("LevelFlow Config"),  TEXT("/Script/PGXLoadingRuntime.PGXLevelFlowConfig"), TEXT("DA_PGXLevelFlow"),    TEXT("Loading"),  TEXT("ClassIcon.World"),            false, FColor(33, 150, 243) },
	{ TEXT("Level Profile"),     TEXT("/Script/PGXLoadingRuntime.PGXLevelProfile"),    TEXT("DA_PGXLevelProfile"), TEXT("Loading"),  TEXT("ClassIcon.World"),            false, FColor(63, 81, 181) },
	{ TEXT("Loading Config"),    TEXT("/Script/PGXLoadingRuntime.PGXLoadingConfig"),   TEXT("DA_PGXLoadingConfig"), TEXT("Loading"), TEXT("ClassIcon.Object"),           false, FColor(233, 30, 99) },
	{ TEXT("Loading Profile"),   TEXT("/Script/PGXLoadingRuntime.PGXLoadingProfile"),  TEXT("DA_PGXLoadingProfile"), TEXT("Loading"), TEXT("ClassIcon.Object"),          false, FColor(244, 143, 177) },

	// Profile (2)
	{ TEXT("Profile Config"),    TEXT("/Script/PGXCoreRuntime.PGXProjectProfileConfig"), TEXT("DA_PGXProjectProfile"), TEXT("Profile"), TEXT("ClassIcon.PlayerController"), false, FColor(255, 193, 7) },
	{ TEXT("Platform Config"),   TEXT("/Script/PGXCoreRuntime.PGXPlatformConfig"),       TEXT("DA_PGXPlatform"),       TEXT("Profile"), TEXT("ClassIcon.PlayerController"), false, FColor(255, 193, 7) },

	// MGOS (1)
	{ TEXT("GC Observer Config"), TEXT("/Script/PGXMGOSRuntime.PGXGCObserverConfig"), TEXT("DA_PGXGCObserver"),   TEXT("MGOS"),         TEXT("ClassIcon.Object"),       false, FColor(127, 0, 255) },

	// Documentation (1)
	{ TEXT("Docs Config"),       TEXT("/Script/PGXDocs.PGXDocsConfig"),              TEXT("DA_PGXDocsConfig"),    TEXT("Documentation"), TEXT("ClassIcon.Object"),       false, FColor(0, 139, 139) },

	// Audio (7)
	{ TEXT("Audio Config"),      TEXT("/Script/PGXAudioRuntime.PGXAudioConfig"),         TEXT("DA_PGXAudioConfig"),   TEXT("Audio"), TEXT("ClassIcon.SoundCue"), false, FColor(255, 152, 0) },
	{ TEXT("Channel Config"),    TEXT("/Script/PGXAudioRuntime.PGXAudioChannelConfig"),  TEXT("DA_PGXAudioChannel"),  TEXT("Audio"), TEXT("ClassIcon.SoundCue"), false, FColor(255, 152, 0) },
	{ TEXT("Sound Definition"),  TEXT("/Script/PGXAudioRuntime.PGXSoundDefinition"),     TEXT("DA_PGXSound"),         TEXT("Audio"), TEXT("ClassIcon.SoundCue"), false, FColor(255, 152, 0) },
	{ TEXT("Audio Profile"),     TEXT("/Script/PGXAudioRuntime.PGXAudioProfile"),        TEXT("DA_PGXAudioProfile"),  TEXT("Audio"), TEXT("ClassIcon.SoundCue"), false, FColor(255, 152, 0) },
	{ TEXT("Music Playlist"),    TEXT("/Script/PGXAudioRuntime.PGXMusicPlaylist"),       TEXT("DA_PGXPlaylist"),       TEXT("Audio"), TEXT("ClassIcon.SoundCue"), false, FColor(255, 152, 0) },
	{ TEXT("Ducking Config"),    TEXT("/Script/PGXAudioRuntime.PGXAudioDuckingConfig"),  TEXT("DA_PGXDucking"),       TEXT("Audio"), TEXT("ClassIcon.SoundCue"), false, FColor(255, 152, 0) },
	{ TEXT("Level Audio Config"),TEXT("/Script/PGXAudioRuntime.PGXLevelAudioConfig"),    TEXT("DA_PGXLevelAudio"),    TEXT("Audio"), TEXT("ClassIcon.SoundCue"), false, FColor(255, 152, 0) },

	// Data Registry (2)
	{ TEXT("PGX DataTable"),         TEXT("/Script/PGXCoreRuntime.PGXDataTableAsset"),     TEXT("DT_PGX"),         TEXT("Registry"), TEXT("PGXEditor.Icon.DataRegistry"), false, FColor(0, 204, 255) },
	{ TEXT("Registry Definition"),   TEXT("/Script/PGXCoreRuntime.PGXRegistryDefinition"), TEXT("DA_PGXRegistry"), TEXT("Registry"), TEXT("PGXEditor.Icon.DataRegistry"), false, FColor(0, 204, 255) },

	// Message (1)
	{ TEXT("Message Config"),    TEXT("/Script/PGXCoreRuntime.PGXMessageConfig"),       TEXT("DA_PGXMessageConfig"),    TEXT("Message"),      TEXT("PGXEditor.Icon.Message"),     false, FColor(75, 0, 130) },

	// EventHandler (1)
	{ TEXT("EventHandler Config"), TEXT("/Script/PGXCoreRuntime.PGXEventHandlerConfig"), TEXT("DA_PGXEventHandlerConfig"), TEXT("EventHandler"), TEXT("PGXEditor.Icon.EventHandler"), false, FColor(38, 64, 217) },

	// Log (1) — domain-specific rendering config
	{ TEXT("Log Domain Config"), TEXT("/Script/PGXCoreRuntime.PGXLogDomainConfig"), TEXT("DA_PGXLogDomain"), TEXT("Log"), TEXT("ClassIcon.Object"), false, FColor(128, 153, 179) },

	// Construction (8) — class construction DataAssets
	{ TEXT("Actor Construction"),             TEXT("/Script/PGXCoreRuntime.PGXActorConstruction"),             TEXT("DA_PGXActorConstruction"),             TEXT("Construction"), TEXT("ClassIcon.Actor"),            false, FColor(0, 150, 136) },
	{ TEXT("GameMode Construction"),          TEXT("/Script/PGXCoreRuntime.PGXGameModeConstruction"),          TEXT("DA_PGXGameModeConstruction"),          TEXT("Construction"), TEXT("ClassIcon.GameModeBase"),     false, FColor(0, 150, 136) },
	{ TEXT("PlayerController Construction"),  TEXT("/Script/PGXCoreRuntime.PGXPlayerControllerConstruction"),  TEXT("DA_PGXPCConstruction"),                TEXT("Construction"), TEXT("ClassIcon.PlayerController"), false, FColor(0, 150, 136) },
	{ TEXT("GameState Construction"),         TEXT("/Script/PGXCoreRuntime.PGXGameStateConstruction"),         TEXT("DA_PGXGameStateConstruction"),         TEXT("Construction"), TEXT("ClassIcon.GameStateBase"),    false, FColor(0, 150, 136) },
	{ TEXT("PlayerState Construction"),       TEXT("/Script/PGXCoreRuntime.PGXPlayerStateConstruction"),       TEXT("DA_PGXPlayerStateConstruction"),       TEXT("Construction"), TEXT("ClassIcon.PlayerController"), false, FColor(0, 150, 136) },
	{ TEXT("Character Construction"),         TEXT("/Script/PGXCoreRuntime.PGXCharacterConstruction"),         TEXT("DA_PGXCharacterConstruction"),         TEXT("Construction"), TEXT("ClassIcon.Character"),        false, FColor(0, 150, 136) },
	{ TEXT("Pawn Construction"),              TEXT("/Script/PGXCoreRuntime.PGXPawnConstruction"),              TEXT("DA_PGXPawnConstruction"),              TEXT("Construction"), TEXT("ClassIcon.Pawn"),             false, FColor(0, 150, 136) },
	{ TEXT("HUD Construction"),              TEXT("/Script/PGXCoreRuntime.PGXHUDConstruction"),               TEXT("DA_PGXHUDConstruction"),               TEXT("Construction"), TEXT("ClassIcon.HUD"),              false, FColor(0, 150, 136) },
};

// ─── Public API ───

const TArray<FPGXCreatableAssetEntry>& FPGXAssetCreationRegistry::GetBlueprintEntries()
{
	return GBlueprintEntries;
}

const TArray<FPGXCreatableAssetEntry>& FPGXAssetCreationRegistry::GetDataAssetEntries()
{
	return GDataAssetEntries;
}

TArray<FString> FPGXAssetCreationRegistry::GetBlueprintCategories()
{
	TArray<FString> Categories;
	TSet<FString> Seen;
	for (const auto& Entry : GBlueprintEntries)
	{
		if (!Seen.Contains(Entry.Category))
		{
			Seen.Add(Entry.Category);
			Categories.Add(Entry.Category);
		}
	}
	return Categories;
}

TArray<FPGXCreatableAssetEntry> FPGXAssetCreationRegistry::GetBlueprintEntriesForCategory(const FString& Category)
{
	TArray<FPGXCreatableAssetEntry> Result;
	for (const auto& Entry : GBlueprintEntries)
	{
		if (Entry.Category == Category)
		{
			Result.Add(Entry);
		}
	}
	return Result;
}

TArray<FString> FPGXAssetCreationRegistry::GetDataAssetCategories()
{
	TArray<FString> Categories;
	TSet<FString> Seen;
	for (const auto& Entry : GDataAssetEntries)
	{
		if (!Seen.Contains(Entry.Category))
		{
			Seen.Add(Entry.Category);
			Categories.Add(Entry.Category);
		}
	}
	return Categories;
}

TArray<FPGXCreatableAssetEntry> FPGXAssetCreationRegistry::GetDataAssetEntriesForCategory(const FString& Category)
{
	TArray<FPGXCreatableAssetEntry> Result;
	for (const auto& Entry : GDataAssetEntries)
	{
		if (Entry.Category == Category)
		{
			Result.Add(Entry);
		}
	}
	return Result;
}

TArray<FString> FPGXAssetCreationRegistry::GetAllSystemCategories()
{
	TArray<FString> Categories;
	TSet<FString> Seen;

	// EN: BP categories first (Core, UI, Camera...), then DA-only categories
	// ES: Categorias BP primero (Core, UI, Camera...), luego categorias solo-DA
	for (const auto& Entry : GBlueprintEntries)
	{
		if (!Seen.Contains(Entry.Category))
		{
			Seen.Add(Entry.Category);
			Categories.Add(Entry.Category);
		}
	}
	for (const auto& Entry : GDataAssetEntries)
	{
		if (!Seen.Contains(Entry.Category))
		{
			Seen.Add(Entry.Category);
			Categories.Add(Entry.Category);
		}
	}

	// EN: Sort alphabetically but keep Core first / ES: Ordenar alfabeticamente pero mantener Core primero
	Categories.Sort([](const FString& A, const FString& B)
	{
		if (A == TEXT("Core")) return true;
		if (B == TEXT("Core")) return false;
		return A < B;
	});

	return Categories;
}
