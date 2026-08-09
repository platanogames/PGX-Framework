// Copyright PGX Framework. All Rights Reserved.

#include "PGXDemoPopulator.h"
#include "Logging/PGXLogMacros.h"
#include "PGXSimHarnessEditorModule.h"

// EN: Config DA headers — full includes needed for Cast<> (gotcha #5: TSoftClassPtr requires complete type)
// ES: Headers de Config DA — includes completos necesarios para Cast<> (gotcha #5: TSoftClassPtr requiere tipo completo)

// PGXCoreRuntime
#include "Profile/PGXProjectProfileConfig.h"
#include "Profile/PGXPlatformConfig.h"
#include "Messages/PGXMessageConfig.h"
#include "EventHandler/PGXEventHandlerConfig.h"
#include "Construction/PGXGameModeConstruction.h"

// PGXGameFlowRuntime
#include "PGXGameFlowConfig.h"
#include "PGXFlowRulesConfig.h"

// PGXSaveRuntime
#include "PGXSaveConfig.h"

// PGXPSORuntime
#include "PGXPSOWarmUpConfig.h"

// PGXLoadingRuntime
#include "PGXLevelFlowConfig.h"
#include "PGXLevelProfile.h"
#include "PGXLoadingConfig.h"
#include "PGXLoadingProfile.h"

// PGXAudioRuntime
#include "PGXAudioConfig.h"
#include "Data/PGXAudioChannelConfig.h"
#include "Data/PGXSoundDefinition.h"
#include "Data/PGXMusicPlaylist.h"
#include "Data/PGXAudioDuckingConfig.h"
#include "Data/PGXAudioProfile.h"
#include "Data/PGXLevelAudioConfig.h"

// PGXCoreRuntime — Data
#include "Data/PGXObjectDataAsset.h"

// PGXMGOSRuntime
#include "PGXGCObserverConfig.h"

// PGXDocs
#include "PGXDocsConfig.h"

// EN: Main dispatcher function that identifies the DA type and calls the appropriate populate function.
// ES: Funcion principal que identifica el tipo de DA y llama a la funcion de poblar apropiada.
bool FPGXDemoPopulator::PopulateDemo(UObject* DataAsset)
{
	if (!IsValid(DataAsset))
	{
		return false;
	}

	// EN: Dispatch to type-specific populate function via Cast chain.
	//     Order follows the 7 demo categories for clarity.
	// ES: Despachar a funcion de poblar especifica por tipo via cadena Cast.
	//     El orden sigue las 7 categorias demo por claridad.

	// 1. Foundation
	if (auto* DA = Cast<UPGXProjectProfileConfig>(DataAsset))     { PopulateProjectProfile(DA); return true; }
	if (auto* DA = Cast<UPGXPlatformConfig>(DataAsset))            { PopulatePlatformConfig(DA); return true; }
	if (auto* DA = Cast<UPGXGameModeConstruction>(DataAsset))      { PopulateGameModeConstruction(DA); return true; }

	// 2. Game State
	if (auto* DA = Cast<UPGXGameFlowConfig>(DataAsset))            { PopulateGameFlowConfig(DA); return true; }
	if (auto* DA = Cast<UPGXFlowRulesConfig>(DataAsset))           { PopulateFlowRulesConfig(DA); return true; }

	// 3. Persistence
	if (auto* DA = Cast<UPGXSaveConfig>(DataAsset))                { PopulateSaveConfig(DA); return true; }
	if (auto* DA = Cast<UPGXPSOWarmUpConfig>(DataAsset))           { PopulatePSOConfig(DA); return true; }

	// 4. Level Management
	if (auto* DA = Cast<UPGXLevelFlowConfig>(DataAsset))           { PopulateLevelFlowConfig(DA); return true; }
	if (auto* DA = Cast<UPGXLevelProfile>(DataAsset))              { PopulateLevelProfile(DA); return true; }
	if (auto* DA = Cast<UPGXLoadingConfig>(DataAsset))             { PopulateLoadingConfig(DA); return true; }
	if (auto* DA = Cast<UPGXLoadingProfile>(DataAsset))            { PopulateLoadingProfile(DA); return true; }

	// 5. Audio
	if (auto* DA = Cast<UPGXAudioConfig>(DataAsset))               { PopulateAudioConfig(DA); return true; }
	if (auto* DA = Cast<UPGXAudioChannelConfig>(DataAsset))        { PopulateAudioChannelConfig(DA); return true; }
	if (auto* DA = Cast<UPGXSoundDefinition>(DataAsset))           { PopulateSoundDefinition(DA); return true; }
	if (auto* DA = Cast<UPGXMusicPlaylist>(DataAsset))             { PopulatePlaylist(DA); return true; }
	if (auto* DA = Cast<UPGXAudioDuckingConfig>(DataAsset))        { PopulateDuckingConfig(DA); return true; }
	if (auto* DA = Cast<UPGXAudioProfile>(DataAsset))              { PopulateAudioProfile(DA); return true; }
	if (auto* DA = Cast<UPGXLevelAudioConfig>(DataAsset))          { PopulateLevelAudioConfig(DA); return true; }

	// 6. Infrastructure
	if (auto* DA = Cast<UPGXMessageConfig>(DataAsset))             { PopulateMessageConfig(DA); return true; }
	if (auto* DA = Cast<UPGXEventHandlerConfig>(DataAsset))        { PopulateEventHandlerConfig(DA); return true; }
	if (auto* DA = Cast<UPGXGCObserverConfig>(DataAsset))          { PopulateGCObserverConfig(DA); return true; }

	// 7. Tools
	if (auto* DA = Cast<UPGXDocsConfig>(DataAsset))                { PopulateDocsConfig(DA); return true; }

	// ─── 11. Demo Gameplay Assets ───
	if (auto* DA = Cast<UPGXObjectDataAsset>(DataAsset))           { PopulateObjectDataAsset(DA); return true; }

	PGX_LOG_WARNING(LogPGXSimHarness, TEXT("FPGXDemoPopulator: Unrecognized DA class %s"), *DataAsset->GetClass()->GetName());
	return false;
}

// ═══════════════════════════════════════════
// 1. Foundation
// ═══════════════════════════════════════════

void FPGXDemoPopulator::PopulateProjectProfile(UPGXProjectProfileConfig* /*DA*/)
{
	// EN: ProjectProfileConfig uses complex structs (FPGXProfileIdentity, etc.)
	//     These will be populated in v0.2 when struct members are fully mapped.
	//     For now, the DA opens with all struct categories visible for exploration.
	// ES: ProjectProfileConfig usa structs complejos. Se poblaran en v0.2.
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("DemoPopulator: ProjectProfile — struct properties left at defaults (v0.2 will populate)"));
}

void FPGXDemoPopulator::PopulatePlatformConfig(UPGXPlatformConfig* /*DA*/)
{
	// EN: PlatformConfig uses budget structs per subsystem.
	//     Complex structs left at defaults — user sees the category layout.
	// ES: PlatformConfig usa structs de presupuesto por subsistema.
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("DemoPopulator: PlatformConfig — budget structs left at defaults (v0.2 will populate)"));
}

void FPGXDemoPopulator::PopulateGameModeConstruction(UPGXGameModeConstruction* /*DA*/)
{
	// EN: Construction DA has TSoftClassPtr fields — leave empty so user picks their classes.
	//     The DA layout itself teaches the class assignment pattern.
	// ES: Construction DA tiene campos TSoftClassPtr — dejar vacios para que el usuario elija sus clases.
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("DemoPopulator: GameModeConstruction — class refs left empty (user assigns their classes)"));
}

// ═══════════════════════════════════════════
// 2. Game State
// ═══════════════════════════════════════════


// EN: GameFlowConfig has simple properties that can be demoed with example values.
// ES: GameFlowConfig tiene propiedades simples que pueden ser demoed con valores de ejemplo.
void FPGXDemoPopulator::PopulateGameFlowConfig(UPGXGameFlowConfig* DA)
{
	DA->MaxHistoryDepth = 25;
	DA->bLogTransitions = true;
	DA->bVerboseDebug = false;

	PGX_LOG_INFO(LogPGXSimHarness, TEXT("DemoPopulator: GameFlowConfig — MaxHistory=25, LogTransitions=true"));
}

// EN: FlowRulesConfig requires specific channel/state definitions to be meaningful.
// ES: FlowRulesConfig requiere definiciones especificas de canal/estado para ser significativo.
void FPGXDemoPopulator::PopulateFlowRulesConfig(UPGXFlowRulesConfig* /*DA*/)
{
	// EN: FlowRulesConfig needs specific channel/state definitions.
	//     Left at defaults — the DA layout shows the rules structure.
	// ES: FlowRulesConfig necesita definiciones especificas de canal/estado.
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("DemoPopulator: FlowRulesConfig — left at defaults (user defines rules)"));
}

// ═══════════════════════════════════════════
// 3. Persistence
// ═══════════════════════════════════════════


// EN: SaveConfig has straightforward properties that can be demoed with example values.
// ES: SaveConfig tiene propiedades directas que pueden ser demoed con valores de ejemplo.
void FPGXDemoPopulator::PopulateSaveConfig(UPGXSaveConfig* DA)
{
	DA->ContextDisplayName = FText::FromString(TEXT("Demo Save Context"));
	DA->MaxSaveSlots = 5;
	DA->SlotNamePattern = TEXT("Slot_{NN}");
	DA->bAutoSaveEnabled = true;
	DA->AutoSaveIntervalSeconds = 120.0f;
	DA->MaxAutoSaveSlots = 3;
	DA->bEnableQuickSave = true;
	DA->QuickSaveSlotName = TEXT("QuickSave");
	DA->bCompressSaveData = false;
	DA->bValidateChecksum = true;
	DA->CurrentSaveVersion = 1;
	DA->SaveFileExtension = TEXT(".sav");
	DA->bCreateBackupBeforeSave = true;
	DA->MaxBackupsPerSlot = 2;
	DA->BaseDirectory = TEXT("SaveGames");
	DA->FileNamePrefix = TEXT("Save");

	PGX_LOG_INFO(LogPGXSimHarness, TEXT("DemoPopulator: SaveConfig — 5 slots, 2min autosave, quicksave enabled"));
}

void FPGXDemoPopulator::PopulatePSOConfig(UPGXPSOWarmUpConfig* DA)
{
	DA->BatchSize = 10;
	DA->bSaveCacheAfterWarmUp = true;
	DA->MaxSimultaneousLoads = 20;

	PGX_LOG_INFO(LogPGXSimHarness, TEXT("DemoPopulator: PSOConfig — BatchSize=10, SaveCache=true"));
}

// ═══════════════════════════════════════════
// 4. Level Management
// ═══════════════════════════════════════════

void FPGXDemoPopulator::PopulateLevelFlowConfig(UPGXLevelFlowConfig* /*DA*/)
{
	// EN: LevelFlowConfig contains level catalog — user defines their levels.
	// ES: LevelFlowConfig contiene catalogo de niveles — el usuario define sus niveles.
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("DemoPopulator: LevelFlowConfig — left at defaults (user defines levels)"));
}

void FPGXDemoPopulator::PopulateLevelProfile(UPGXLevelProfile* /*DA*/)
{
	// EN: LevelProfile needs specific level references — user fills these.
	// ES: LevelProfile necesita referencias de nivel especificas — el usuario las llena.
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("DemoPopulator: LevelProfile — left at defaults (user defines level profile)"));
}

void FPGXDemoPopulator::PopulateLoadingConfig(UPGXLoadingConfig* DA)
{
	DA->DefaultMinDisplayTime = 2.0f;
	DA->OverlayZOrder = 1000;
	DA->CriticalErrorZOrder = 5000;
	DA->PreparingTimeout = 5.0f;
	DA->WaitingCloseTimeout = 20.0f;
	DA->PostLoadFrameDelay = 2;
	DA->bWaitForPSOByDefault = true;
	DA->PSOWaitTimeout = 15.0f;
	DA->PSOProgressWeight = 0.3f;
	DA->bAutoActivateOnLevelFlow = true;
	DA->MaxHistoryDepth = 50;

	PGX_LOG_INFO(LogPGXSimHarness, TEXT("DemoPopulator: LoadingConfig — 2s min display, PSO wait enabled, 50 history"));
}

void FPGXDemoPopulator::PopulateLoadingProfile(UPGXLoadingProfile* /*DA*/)
{
	// EN: LoadingProfile needs texture/widget references — user fills these.
	// ES: LoadingProfile necesita referencias de texturas/widgets — el usuario las llena.
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("DemoPopulator: LoadingProfile — left at defaults (user defines visuals)"));
}

// ═══════════════════════════════════════════
// 5. Audio
// ═══════════════════════════════════════════

void FPGXDemoPopulator::PopulateAudioConfig(UPGXAudioConfig* DA)
{
	DA->MasterVolume = 0.8f;
	DA->MusicVolume = 0.6f;
	DA->SFXVolume = 1.0f;
	DA->VoiceVolume = 1.0f;
	DA->DefaultMusicCrossfadeDuration = 2.0f;
	DA->SoundPoolInitialSize = 32;
	DA->SoundPoolMaxSize = 128;
	DA->bDefaultHDRAudioEnabled = false;
	DA->bDefaultHRTFEnabled = false;
	DA->bEnableAudioOcclusion = false;
	DA->bEnableTrace = false;
	DA->MaxEventHistorySize = 200;

	PGX_LOG_INFO(LogPGXSimHarness, TEXT("DemoPopulator: AudioConfig — Master=0.8, Music=0.6, SFX=1.0, Pool=32-128"));
}

void FPGXDemoPopulator::PopulateAudioChannelConfig(UPGXAudioChannelConfig* DA)
{
	DA->ChannelDisplayName = FText::FromString(TEXT("SFX Channel"));
	DA->DefaultVolume = 1.0f;
	DA->MaxConcurrent = 16;
	DA->bUserAdjustable = true;
	DA->bAffectedByGlobalMute = true;

	PGX_LOG_INFO(LogPGXSimHarness, TEXT("DemoPopulator: AudioChannelConfig — SFX, Vol=1.0, MaxConc=16"));
}

void FPGXDemoPopulator::PopulateSoundDefinition(UPGXSoundDefinition* /*DA*/)
{
	// EN: SoundDefinition needs USoundBase references — user fills these.
	// ES: SoundDefinition necesita referencias USoundBase — el usuario las llena.
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("DemoPopulator: SoundDefinition — left at defaults (user assigns sound assets)"));
}

void FPGXDemoPopulator::PopulatePlaylist(UPGXMusicPlaylist* /*DA*/)
{
	// EN: MusicPlaylist needs music track references — user fills these.
	// ES: MusicPlaylist necesita referencias de pistas musicales — el usuario las llena.
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("DemoPopulator: MusicPlaylist — left at defaults (user assigns tracks)"));
}

void FPGXDemoPopulator::PopulateDuckingConfig(UPGXAudioDuckingConfig* /*DA*/)
{
	// EN: DuckingConfig needs ducking rules with channel relationships — user fills these.
	// ES: DuckingConfig necesita reglas de ducking con relaciones entre canales — el usuario las llena.
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("DemoPopulator: DuckingConfig — left at defaults (user defines ducking rules)"));
}

void FPGXDemoPopulator::PopulateAudioProfile(UPGXAudioProfile* /*DA*/)
{
	// EN: Container-only fallback (deferred population). Demo values are populated by PGXAudio. Until then the asset opens at
	//     defaults so its category layout is visible.
	// ES: Fallback solo-contenedor (poblado diferido). PGXAudio completa los valores demo; mientras tanto el asset abre con defaults.
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("DemoPopulator: AudioProfile — container-only (PGXAudio fills demo values)"));
}

void FPGXDemoPopulator::PopulateLevelAudioConfig(UPGXLevelAudioConfig* /*DA*/)
{
	// EN: Container-only fallback (deferred population). Demo values belong to PGXAudio.
	// ES: Fallback solo-contenedor (poblado diferido). PGXAudio completa los valores demo.
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("DemoPopulator: LevelAudioConfig — container-only (PGXAudio fills demo values)"));
}

// ═══════════════════════════════════════════
// 6. Infrastructure
// ═══════════════════════════════════════════

void FPGXDemoPopulator::PopulateMessageConfig(UPGXMessageConfig* DA)
{
	DA->MaxMessageHistory = 50;
	DA->bLogBroadcasts = true;
	DA->bLogRegistrations = true;
	DA->bEnablePartialMatching = true;

	PGX_LOG_INFO(LogPGXSimHarness, TEXT("DemoPopulator: MessageConfig — History=50, logging enabled, partial matching"));
}

void FPGXDemoPopulator::PopulateEventHandlerConfig(UPGXEventHandlerConfig* DA)
{
	DA->MaxCachedHandlers = 64;
	DA->MaxExecutionDepth = 8;
	DA->BlackboxBufferSize = 128;
	DA->bLogExecutions = true;
	DA->bLogCacheOperations = false;
	DA->bLogRegistration = true;
	DA->bAutoExportOnPIEEnd = true;
	DA->BlackboxEntriesInReport = 50;

	PGX_LOG_INFO(LogPGXSimHarness, TEXT("DemoPopulator: EventHandlerConfig — Cache=64, Depth=8, logging + auto-export"));
}

void FPGXDemoPopulator::PopulateGCObserverConfig(UPGXGCObserverConfig* DA)
{
	DA->bEnableIncidents = true;
	DA->HistoryWindowSize = 64;
	DA->TestWindowSize = 4;
	DA->WarmupCycles = 16;
	DA->ConfirmCycles = 8;
	DA->TopKClasses = 10;
	DA->MaxIncidentHistory = 256;
	DA->bEnableInterCycleMonitoring = true;
	DA->InterCycleCheckInterval = 5.0f;
	DA->bEnableProcessMemoryTracking = true;
	DA->ProcessMemoryDriftThresholdMB = 100.0f;

	PGX_LOG_INFO(LogPGXSimHarness, TEXT("DemoPopulator: GCObserverConfig — Incidents=true, History=64, InterCycle monitoring"));
}

// ═══════════════════════════════════════════
// 7. Tools
// ═══════════════════════════════════════════

void FPGXDemoPopulator::PopulateDocsConfig(UPGXDocsConfig* DA)
{
	DA->bIncludeProjectDocs = true;
	DA->bIncludePluginDocs = true;
	DA->BaseFontSize = 14;
	DA->SidebarWidthPercent = 25;
	DA->bShowFrontMatterPanel = true;
	DA->bShowStatusBar = true;
	DA->MaxSearchResults = 25;
	DA->bAutoRebuildIndex = true;
	DA->bEnableClassLinks = true;
	DA->bEnableBlueprintLinks = true;
	DA->bEnableDocLinks = true;
	DA->bEnableExternalLinks = false;
	DA->bSandboxImagePaths = true;

	PGX_LOG_INFO(LogPGXSimHarness, TEXT("DemoPopulator: DocsConfig — Project+Plugin docs, font=14, search=25, links enabled"));
}

// ═══════════════════════════════════════════
// 11. Demo Gameplay Assets
// ═══════════════════════════════════════════

void FPGXDemoPopulator::PopulateObjectDataAsset(UPGXObjectDataAsset* DA)
{
	// EN: Derive human-readable display name from asset name by stripping prefix.
	// ES: Derivar nombre legible desde el prefijo del nombre del asset.
	//  Live — items & NPC DAs get DisplayName populated
	FString AssetName = DA->GetName();
	AssetName.RemoveFromStart(TEXT("DA_Item_"));
	AssetName.RemoveFromStart(TEXT("DA_NPC_"));
	if (!AssetName.IsEmpty())
	{
		DA->DisplayName = FText::FromString(AssetName);
	}

	PGX_LOG_INFO(LogPGXSimHarness, TEXT("DemoPopulator: ObjectDataAsset — display=%s"), *DA->DisplayName.ToString());
}
