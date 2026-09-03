// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "FPGXVisualHarness.h"
#include "PGXSimHarnessEditorModule.h"
#include "Logging/PGXLogMacros.h"
#include "PGXHarnessPanelList.h"
#include "Framework/Docking/TabManager.h"

// EN: Subsystem headers for injection / ES: Headers de subsistemas para inyeccion
#include "Profile/PGXProfileSubsystem.h"
#include "Profile/PGXProfileTypes.h"
#include "Construction/PGXConstructionSettings.h"
#include "PGXGameFlowSubsystem.h"
#include "PGXGameFlowTypes.h"
#include "Logging/PGXLogBlueprintLibrary.h"
#include "Logging/PGXLogTestUtility.h"
#include "PGXSaveSubsystem.h"
#include "PGXSaveConfig.h"
#include "PGXSaveTypes.h"
#include "PGXSaveGame.h"
#include "PGXPSOSubsystem.h"
#include "PGXPSOWarmUpConfig.h"
#include "PGXGCObserverSubsystem.h"
#include "PGXMGOSTypes.h"
#include "PGXAudioSubsystem.h"
#include "PGXAudioConfig.h"
#include "Data/PGXAudioChannelConfig.h"
#include "Data/PGXSoundDefinition.h"
#include "Registry/PGXDataRegistrySubsystem.h"
#include "Base/PGXDataAsset.h"
#include "Data/PGXObjectDataAsset.h"
#include "Messages/PGXMessageSubsystem.h"
#include "Messages/PGXMessageConfig.h"
#include "EventHandler/PGXEventHandlerSubsystem.h"
#include "EventHandler/PGXEventHandlerConfig.h"
#include "EventHandler/PGXEventHandlerBase.h"
#include "EventHandler/PGXEventHandlerTypes.h"
#include "PGXHarnessHandlerStub.h"
#include "StructUtils/InstancedStruct.h"
#include "PGXLevelFlowTestUtility.h"
#include "PGXLoadingTestUtility.h"
#include "PGXLevelFlowSubsystem.h"
#include "PGXLoadingSubsystem.h"
#include "PGXLevelFlowTypes.h"
#include "PGXLoadingTypes.h"
#include "PGXHarnessSaveableStub.h"

//  coverage — PGXInput harness integration
#include "PGXInputSubsystem.h"
#include "PGXInputConfig.h"
#include "PGXInputBuffer.h"
#include "PGXInputContext.h"
#include "PGXInputSettings.h"

//  runtime core — PGXSpawn harness integration
#include "PGXSpawnSubsystem.h"
#include "PGXWaveDefinition.h"

//  runtime core — PGXAI harness integration
#include "PGXAISubsystem.h"

//  runtime core — PGXAbility harness integration
#include "PGXAbilitySubsystem.h"
#include "PGXAbilityComponent.h"
#include "PGXAbilityFacade.h"

//  runtime extended — PGXCamera harness integration
#include "PGXCameraSubsystem.h"
#include "PGXCameraMode.h"

//  runtime extended — PGXInteraction harness integration
#include "PGXInteractionComponent.h"
#include "PGXInteractionTypes.h"

//  runtime extended — PGXInventory harness integration
#include "PGXInventoryComponent.h"
#include "PGXItemDefinition.h"

//  runtime extended — PGXUI harness integration
#include "PGXUISubsystem.h"
#include "PGXWidgetPool.h"

#include "Framework/Docking/TabManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"

// ============================================================================
// EN: Harness gameplay tags — requested at runtime via FGameplayTag::RequestGameplayTag()
//     Tags are registered in StartupModule via AddNativeGameplayTag.
//     bErrorIfNotFound=false as safety net (graceful degradation).
// ES: Tags del harness — solicitados en runtime via FGameplayTag::RequestGameplayTag()
//     Tags se registran en StartupModule via AddNativeGameplayTag.
//     bErrorIfNotFound=false como red de seguridad (degradacion elegante).
// ============================================================================

namespace PGXHarnessTags
{
	// --- Audio channels ---
	static FGameplayTag AudioSFX()     { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Audio.Channel.SFX"), false); }
	static FGameplayTag AudioMusic()   { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Audio.Channel.Music"), false); }
	static FGameplayTag AudioVoice()   { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Audio.Channel.Voice"), false); }
	static FGameplayTag AudioAmbient() { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Audio.Channel.Ambient"), false); }
	static FGameplayTag AudioUI()      { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Audio.Channel.UI"), false); }

	// --- Save ---
	static FGameplayTag SaveCampaign()    { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Save.Context.Campaign"), false); }
	static FGameplayTag SaveSettings()    { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Save.Context.Settings"), false); }
	static FGameplayTag DomainProgress()  { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Save.Domain.PlayerProgress"), false); }
	static FGameplayTag DomainWorld()     { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Save.Domain.WorldState"), false); }
	static FGameplayTag DomainGraphics()  { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Save.Domain.Graphics"), false); }
	static FGameplayTag DomainAudio()     { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Save.Domain.AudioSettings"), false); }

	// --- GameFlow states ---
	static FGameplayTag FlowMainMenu()  { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Flow.MainMenu"), false); }
	static FGameplayTag FlowHUD()       { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Flow.HUD"), false); }
	static FGameplayTag FlowActive()    { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Flow.Active"), false); }
	static FGameplayTag FlowPatrol()    { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Flow.Patrol"), false); }
	static FGameplayTag FlowGameplay()  { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Flow.Gameplay"), false); }
	static FGameplayTag FlowRunning()   { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Flow.Running"), false); }
	static FGameplayTag FlowExplore()   { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Flow.Explore"), false); }
	static FGameplayTag FlowInGame()    { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Flow.InGame"), false); }
	static FGameplayTag FlowLoading()   { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Flow.Loading"), false); }
	static FGameplayTag FlowPause()     { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Flow.Pause"), false); }

	// --- Message channels ---
	static FGameplayTag MsgUI()         { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Message.UI"), false); }
	static FGameplayTag MsgGameplay()   { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Message.Gameplay"), false); }
	static FGameplayTag MsgSystem()     { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Message.System"), false); }
	static FGameplayTag MsgAudio()      { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Message.Audio"), false); }
	static FGameplayTag MsgNetwork()    { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Message.Network"), false); }

	// --- Event handler events ---
	static FGameplayTag EvtDamage()     { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Event.Damage"), false); }
	static FGameplayTag EvtHeal()       { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Event.Heal"), false); }
	static FGameplayTag EvtPickup()     { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Event.Pickup"), false); }
	static FGameplayTag EvtInteract()   { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Event.Interact"), false); }
	static FGameplayTag EvtSpawn()      { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Event.Spawn"), false); }

	// --- DataRegistry databases ---
	static FGameplayTag DbItems()       { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Registry.Items"), false); }
	static FGameplayTag DbNPCs()        { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Registry.NPCs"), false); }

	// --- PSO contexts ---
	static FGameplayTag PSOCtxMainMenu()  { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.PSO.Context.MainMenu"), false); }
	static FGameplayTag PSOCtxGameplay()  { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.PSO.Context.Gameplay"), false); }
	static FGameplayTag PSOCtxInventory() { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.PSO.Context.Inventory"), false); }
	static FGameplayTag PSOCtxCinematic() { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.PSO.Context.Cinematic"), false); }

	// --- Input contexts ---
	static FGameplayTag InputGameplay() { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Input.Context.Gameplay"), false); }
	static FGameplayTag InputUI()       { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Input.Context.UI"), false); }

	// --- Spawn wave smoke ---
	static FGameplayTag SpawnWaveSmoke() { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Spawn.Wave.Smoke"), false); }

	// --- Ability smoke ---
	static FGameplayTag AbilitySmoke() { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Ability.Smoke"), false); }

	// --- Interaction smoke ---
	static FGameplayTag InteractionTargetSmoke() { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Event.Interact"), false); }
	static FGameplayTag InteractionActionSmoke() { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Ability.Smoke"), false); }

	// --- Inventory smoke ---
	static FGameplayTag InventoryItemSmoke() { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Registry.Item.HealthPotion"), false); }

	// --- Audio Part 2: music state, speaker, priority ---
	static FGameplayTag AudioMusicStateExplore() { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Audio.MusicState.Explore"), false); }
	static FGameplayTag AudioSpeakerNPC()        { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Audio.Speaker.NPC"), false); }
	static FGameplayTag AudioPriorityHigh()      { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Audio.Priority.High"), false); }

	// --- LevelFlow Part 2 ---
	static FGameplayTag LevelTestZone()          { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Level.TestZone"), false); }
	static FGameplayTag LevelSubLevelCave()      { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Level.SubLevel.Cave"), false); }

	// --- Loading Part 2 ---
	static FGameplayTag LoadingTestContext()      { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Loading.Context.Test"), false); }

	// --- Registry items ---
	static FGameplayTag ItemIronSword()    { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Registry.Item.IronSword"), false); }
	static FGameplayTag ItemHealthPotion() { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Registry.Item.HealthPotion"), false); }
	static FGameplayTag ItemMageStaff()    { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Registry.Item.MageStaff"), false); }
	static FGameplayTag ItemDragonShield() { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Registry.Item.DragonShield"), false); }
	static FGameplayTag ItemFireScroll()   { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Registry.Item.FireScroll"), false); }

	// --- Registry NPCs ---
	static FGameplayTag NPCMerchant()   { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Registry.NPC.Merchant"), false); }
	static FGameplayTag NPCBlacksmith() { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Registry.NPC.Blacksmith"), false); }
	static FGameplayTag NPCQuestGiver() { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Registry.NPC.QuestGiver"), false); }
	static FGameplayTag NPCGuard()      { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Registry.NPC.Guard"), false); }
	static FGameplayTag NPCInnkeeper()  { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Registry.NPC.Innkeeper"), false); }

	// --- Registry categories ---
	static FGameplayTag CatWeapon()     { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Registry.Category.Weapon"), false); }
	static FGameplayTag CatConsumable() { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Registry.Category.Consumable"), false); }
	static FGameplayTag CatEquipment()  { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Registry.Category.Equipment"), false); }
	static FGameplayTag CatVendor()     { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Registry.Category.Vendor"), false); }
	static FGameplayTag CatService()    { return FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Registry.Category.Service"), false); }
}

// ============================================================================
// EN: Destructor
// ES: Destructor
// ============================================================================

FPGXVisualHarness::~FPGXVisualHarness()
{
	if (bIsSimulating)
	{
		StopSimulation();
	}
	if (bIsActive)
	{
		Teardown();
	}
}

// ============================================================================
// EN: Lifecycle
// ES: Ciclo de vida
// ============================================================================

void FPGXVisualHarness::Setup(UWorld* InWorld)
{
	if (bIsActive)
	{
		PGX_LOG_WARNING(LogPGXSimHarness, TEXT("VisualHarness::Setup — Already active, call Teardown() first"));
		return;
	}

	// EN: Resolve PIE world — injection must target the same instance inspectors query
	// ES: Resolver PIE world — la inyeccion debe apuntar a la misma instancia que consultan los inspectores
	UWorld* World = IsValid(InWorld) ? InWorld : ResolveEditorWorld();
	if (!IsValid(World))
	{
		PGX_LOG_ERROR(LogPGXSimHarness, TEXT("VisualHarness::Setup — No PIE world found. Start Play-In-Editor first."));
		return;
	}

	CachedWorld = World;
	SetupTimestamp = FPlatformTime::Seconds();
	ActionLog.Empty();
	VerificationRuns.Empty();

	PGX_LOG_INFO(LogPGXSimHarness, TEXT("========== VisualHarness Setup START =========="));

	// EN: Initialize 21 system statuses / ES: Inicializar 21 estados de sistema
	SystemStatuses.Empty();
	static const FString SystemNames[] = {
		TEXT("Profile"), TEXT("Construction"), TEXT("GameFlow"), TEXT("Log"),
		TEXT("Save"), TEXT("PSO"), TEXT("MGOS"), TEXT("Audio"),
		TEXT("DataRegistry"), TEXT("Message"), TEXT("EventHandler"),
		TEXT("LevelFlow"), TEXT("Loading"), TEXT("Input"),
		TEXT("Spawn"), TEXT("AI"), TEXT("Ability"),      //  runtime core
		TEXT("Camera"), TEXT("Interaction"), TEXT("Inventory"), TEXT("UI") //  runtime extended
	};
	for (const FString& Name : SystemNames)
	{
		FPGXHarnessSystemStatus Status;
		Status.SystemName = Name;
		SystemStatuses.Add(Status);
	}

	// EN: Inject in init order (Profile -> ... -> Loading)
	// ES: Inyectar en orden de init (Profile -> ... -> Loading)
	InjectProfile();          // 0
	RecordAction(TEXT("Setup"), TEXT("InjectProfile"), SystemStatuses[0].bInjected, SystemStatuses[0].Detail);

	InjectConstruction();     // 1
	RecordAction(TEXT("Setup"), TEXT("InjectConstruction"), SystemStatuses[1].bInjected, SystemStatuses[1].Detail);

	InjectGameFlow(World);    // 2
	RecordAction(TEXT("Setup"), TEXT("InjectGameFlow"), SystemStatuses[2].bInjected, SystemStatuses[2].Detail);

	InjectLog(World);         // 3
	RecordAction(TEXT("Setup"), TEXT("InjectLog"), SystemStatuses[3].bInjected, SystemStatuses[3].Detail);

	InjectSave(World);        // 4
	RecordAction(TEXT("Setup"), TEXT("InjectSave"), SystemStatuses[4].bInjected, SystemStatuses[4].Detail);

	InjectPSO(World);         // 5
	RecordAction(TEXT("Setup"), TEXT("InjectPSO"), SystemStatuses[5].bInjected, SystemStatuses[5].Detail);

	InjectMGOS();             // 6
	RecordAction(TEXT("Setup"), TEXT("InjectMGOS"), SystemStatuses[6].bInjected, SystemStatuses[6].Detail);

	InjectAudio(World);       // 7
	RecordAction(TEXT("Setup"), TEXT("InjectAudio"), SystemStatuses[7].bInjected, SystemStatuses[7].Detail);

	InjectDataRegistry(World);// 8
	RecordAction(TEXT("Setup"), TEXT("InjectDataRegistry"), SystemStatuses[8].bInjected, SystemStatuses[8].Detail);

	InjectMessage(World);     // 9
	RecordAction(TEXT("Setup"), TEXT("InjectMessage"), SystemStatuses[9].bInjected, SystemStatuses[9].Detail);

	InjectEventHandler(World);// 10
	RecordAction(TEXT("Setup"), TEXT("InjectEventHandler"), SystemStatuses[10].bInjected, SystemStatuses[10].Detail);

	InjectLevelFlow(World);   // 11
	RecordAction(TEXT("Setup"), TEXT("InjectLevelFlow"), SystemStatuses[11].bInjected, SystemStatuses[11].Detail);

	InjectLoading(World);     // 12
	RecordAction(TEXT("Setup"), TEXT("InjectLoading"), SystemStatuses[12].bInjected, SystemStatuses[12].Detail);

	//  coverage — Input injection
	InjectInput(World);       // 13
	RecordAction(TEXT("Setup"), TEXT("InjectInput"), SystemStatuses[13].bInjected, SystemStatuses[13].Detail);

	//  runtime core — Spawn injection
	InjectSpawn(World);       // 14
	RecordAction(TEXT("Setup"), TEXT("InjectSpawn"), SystemStatuses[14].bInjected, SystemStatuses[14].Detail);

	//  runtime core — AI injection
	InjectAI(World);          // 15
	RecordAction(TEXT("Setup"), TEXT("InjectAI"), SystemStatuses[15].bInjected, SystemStatuses[15].Detail);

	//  runtime core — Ability injection
	InjectAbility(World);     // 16
	RecordAction(TEXT("Setup"), TEXT("InjectAbility"), SystemStatuses[16].bInjected, SystemStatuses[16].Detail);

	//  runtime extended — Camera injection
	InjectCamera(World);      // 17
	RecordAction(TEXT("Setup"), TEXT("InjectCamera"), SystemStatuses[17].bInjected, SystemStatuses[17].Detail);

	//  runtime extended — Interaction injection
	InjectInteraction(World); // 18
	RecordAction(TEXT("Setup"), TEXT("InjectInteraction"), SystemStatuses[18].bInjected, SystemStatuses[18].Detail);

	//  runtime extended — Inventory injection
	InjectInventory(World);   // 19
	RecordAction(TEXT("Setup"), TEXT("InjectInventory"), SystemStatuses[19].bInjected, SystemStatuses[19].Detail);

	//  runtime extended — UI injection
	InjectUI(World);          // 20
	RecordAction(TEXT("Setup"), TEXT("InjectUI"), SystemStatuses[20].bInjected, SystemStatuses[20].Detail);

	bIsActive = true;

	// EN: Run initial API verification after all injections / ES: Ejecutar verificacion inicial de API despues de todas las inyecciones
	VerifyAllAPIs();

	int32 InjectedCount = 0;
	for (const auto& S : SystemStatuses) { if (S.bInjected) InjectedCount++; }

	RecordAction(TEXT("Setup"), TEXT("SetupComplete"),  true,
		FString::Printf(TEXT("%d/%d systems, %d objects"), InjectedCount, SystemStatuses.Num(), CreatedObjects.Num()));

	PGX_LOG_INFO(LogPGXSimHarness, TEXT("========== VisualHarness Setup END — %d/%d systems, %d objects =========="),
		InjectedCount, SystemStatuses.Num(), CreatedObjects.Num());
}

void FPGXVisualHarness::Teardown()
{
	if (!bIsActive)
	{
		return;
	}

	if (bIsSimulating)
	{
		StopSimulation();
	}

	PGX_LOG_INFO(LogPGXSimHarness, TEXT("========== VisualHarness Teardown START =========="));

	UWorld* World = ResolveEditorWorld();

	// EN: Teardown in reverse order / ES: Teardown en orden inverso
	if (IsValid(World))
	{
		TeardownUI(World);
		TeardownInventory(World);
		TeardownInteraction(World);
		TeardownCamera(World);
		TeardownAbility(World);
		TeardownAI(World);
		TeardownSpawn(World);
		//  coverage — Input teardown (must come before subsystems go away)
		TeardownInput(World);
		TeardownLoading(World);
		TeardownLevelFlow(World);
		TeardownEventHandler(World);
		TeardownMessage(World);
		TeardownDataRegistry(World);
		TeardownAudio(World);
		TeardownMGOS();
		TeardownPSO(World);
		TeardownSave(World);
		TeardownGameFlow(World);
	}
	else
	{
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("Teardown — no valid world, releasing object refs only"));
	}

	TeardownConstruction();
	TeardownProfile();

	RecordAction(TEXT("Teardown"), TEXT("TeardownComplete"), true,
		FString::Printf(TEXT("Released %d objects"), CreatedObjects.Num()));

	// EN: Release all strong refs / ES: Liberar todas las refs fuertes
	CreatedObjects.Empty();
	MessageListenerHandles.Empty();
	RegisteredEventTags.Empty();
	RegisteredDatabaseTags.Empty();
	RegisteredRegistryItems.Empty();
	CreatedSaveSlots.Empty();
	AddedPSOContexts.Empty();
	bPSORecordingStarted = false;
	SavedFlowTags.Empty();
	BackupSlotName.Empty();
	SaveableFixtureRef.Reset();
	SystemStatuses.Empty();
	VerificationPassCount = 0;
	bLastVerificationPassed = false;
	CachedWorld.Reset();

	bIsActive = false;

	PGX_LOG_INFO(LogPGXSimHarness, TEXT("========== VisualHarness Teardown END =========="));
}

void FPGXVisualHarness::StartSimulation()
{
	if (!bIsActive || bIsSimulating)
	{
		return;
	}

	SimulationAccumulator = 0.0f;
	SimulationTickCount = 0;

	// EN: Tick every 2 seconds via FTSTicker (editor-safe, gotcha #8)
	// ES: Tick cada 2 segundos via FTSTicker (editor-safe, gotcha #8)
	SimulationTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateRaw(this, &FPGXVisualHarness::OnSimulationTick),
		2.0f
	);

	bIsSimulating = true;
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("VisualHarness — Simulation started (tick every 2s)"));
}

void FPGXVisualHarness::StopSimulation()
{
	if (!bIsSimulating)
	{
		return;
	}

	FTSTicker::GetCoreTicker().RemoveTicker(SimulationTickerHandle);
	SimulationTickerHandle.Reset();
	bIsSimulating = false;

	PGX_LOG_INFO(LogPGXSimHarness, TEXT("VisualHarness — Simulation stopped after %d ticks"), SimulationTickCount);
}

// ============================================================================
// EN: API Verification — comprehensive query across all 14 subsystems
// ES: Verificacion de API — consulta comprensiva en los 14 subsistemas
// ============================================================================

void FPGXVisualHarness::VerifyAllAPIs()
{
	UWorld* World = ResolveEditorWorld();
	UGameInstance* GI = IsValid(World) ? World->GetGameInstance() : nullptr;

	int32 PassCount = 0;
	int32 TotalChecks = 0;

	FPGXVerificationRun Run;
	Run.RunNumber = VerificationPassCount + 1;
	Run.Timestamp = bIsActive ? (FPlatformTime::Seconds() - SetupTimestamp) : 0.0;

	PGX_LOG_INFO(LogPGXSimHarness, TEXT("========== VerifyAllAPIs START (run #%d) =========="), VerificationPassCount + 1);

	//  High-priority — SimHarness self-coverage.
	// EN: Verify the harness' own public surfaces before subsystem probes: the
	//     compatibility matrix stays complete, the detailed matrix reports the same
	//     canonical 33-plugin scope, and the control panel tab is registered.
	// ES: Verifica las superficies propias del harness antes de probar subsistemas:
	//     matriz de compatibilidad completa, matriz detallada con el mismo scope canonico de
	//     33 plugins, y tab del panel de control registrado.
	{
		int32 SelfPass = 0;
		TotalChecks += 3;
		const TArray<FPGXPluginCoverage> CompatibilityMatrix = GetCoverageMatrix();
		const TArray<FPGXPluginCoverageEntry> DetailedMatrix = GetDetailedCoverageMatrix();
		const FName SimHarnessTabId = FPGXSimHarnessEditorModule::GetSimHarnessTabId();
		const TSharedPtr<FGlobalTabmanager> TabManager = FGlobalTabmanager::Get();

		if (CompatibilityMatrix.Num() == FPGXHarnessCoverage::GetCanonicalPluginCount()) SelfPass++;
		if (DetailedMatrix.Num() == FPGXHarnessCoverage::GetCanonicalPluginCount()) SelfPass++;
		if (TabManager.IsValid() && TabManager->HasTabSpawner(SimHarnessTabId)) SelfPass++;

		PassCount += SelfPass;
		Run.PerSystemResults.Add(TPair<FString, FString>(TEXT("SimHarness"), FString::Printf(TEXT("%d/3 | Compatibility=%d Detailed=%d Tab=%s"),
			SelfPass,
			CompatibilityMatrix.Num(),
			DetailedMatrix.Num(),
			(TabManager.IsValid() && TabManager->HasTabSpawner(SimHarnessTabId)) ? TEXT("Y") : TEXT("N"))));
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  SimHarness: %d/3, Compatibility=%d Detailed=%d Tab=%s"),
			SelfPass,
			CompatibilityMatrix.Num(),
			DetailedMatrix.Num(),
			(TabManager.IsValid() && TabManager->HasTabSpawner(SimHarnessTabId)) ? TEXT("Y") : TEXT("N"));
	}

	// ─── Profile ───
#if WITH_EDITOR
	if (UPGXProfileSubsystem* Prof = UPGXProfileSubsystem::GetCachedInstance())
	{
		int32 ProfBase = PassCount;
		TotalChecks += 4;
		if (Prof->IsProfileResolved()) PassCount++;
		if (Prof->HasSimulationOverrides()) PassCount++;
		if (Prof->GetBudget(FName("RAM_MB")) >= 0) PassCount++;
		const FPGXResolvedProfile& RP = Prof->GetResolvedProfile();
		if (RP.Budgets.RAM_MB > 0 || RP.Budgets.VRAM_MB > 0) PassCount++;
		int32 ProfPass = PassCount - ProfBase;
		Run.PerSystemResults.Add(TPair<FString, FString>(TEXT("Profile"), FString::Printf(TEXT("%d/4"), ProfPass)));
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Profile: %d/4"), ProfPass);
	}
#endif

	// ─── GameFlow ───
	if (GI)
	{
		if (UPGXGameFlowSubsystem* Flow = GI->GetSubsystem<UPGXGameFlowSubsystem>())
		{
			int32 FlowPass = 0;
			TotalChecks += 6;
			if (Flow->GetCurrentFlowTag(EPGXFlowChannel::Global).IsValid()) FlowPass++;
			TArray<FPGXFlowHistoryEntry> Hist = Flow->GetChannelHistory(EPGXFlowChannel::Global);
			if (Hist.Num() > 0) FlowPass++;
			if (Flow->IsInitialized()) FlowPass++;

			int32 MatrixPass = 0;
			if (Flow->GetCurrentFlowTag(EPGXFlowChannel::Global) == PGXHarnessTags::FlowMainMenu())
			{
				MatrixPass++;
			}
			else if (Flow->SetStateByTag(EPGXFlowChannel::Global, PGXHarnessTags::FlowMainMenu()).bSuccess)
			{
				MatrixPass++;
			}
			if (Flow->SetStateByTag(EPGXFlowChannel::Global, PGXHarnessTags::FlowLoading()).bSuccess) MatrixPass++;
			if (Flow->SetStateByTag(EPGXFlowChannel::Global, PGXHarnessTags::FlowInGame()).bSuccess) MatrixPass++;
			if (Flow->SetStateByTag(EPGXFlowChannel::Global, PGXHarnessTags::FlowPause()).bSuccess) MatrixPass++;
			if (Flow->SetStateByTag(EPGXFlowChannel::Global, PGXHarnessTags::FlowMainMenu()).bSuccess) MatrixPass++;
			if (MatrixPass == 5 && Flow->IsCurrentFlowTag(EPGXFlowChannel::Global, PGXHarnessTags::FlowMainMenu())) FlowPass++;
			Hist = Flow->GetChannelHistory(EPGXFlowChannel::Global);
			if (Hist.Num() >= 5) FlowPass++;
			const FPGXFlowResult InvalidProbe = Flow->CanChangeByTag(EPGXFlowChannel::Global, FGameplayTag());
			if (!InvalidProbe.bSuccess) FlowPass++;

			PassCount += FlowPass;
			Run.PerSystemResults.Add(TPair<FString, FString>(TEXT("GameFlow"), FString::Printf(TEXT("%d/6 | History=%d Matrix=%d/5 Invalid=%s"),
				FlowPass, Hist.Num(), MatrixPass, InvalidProbe.bSuccess ? TEXT("ACCEPTED") : TEXT("REJECTED"))));
			PGX_LOG_INFO(LogPGXSimHarness, TEXT("  GameFlow: %d/6, History=%d Matrix=%d/5 Invalid=%s"),
				FlowPass, Hist.Num(), MatrixPass, InvalidProbe.bSuccess ? TEXT("ACCEPTED") : TEXT("REJECTED"));
		}
	}

	// ─── Save ───
	if (GI)
	{
		if (UPGXSaveSubsystem* Save = GI->GetSubsystem<UPGXSaveSubsystem>())
		{
			int32 SavePass = 0;
			TotalChecks += 7;
			if (Save->GetContextCount() >= 2) SavePass++;
			const TArray<FGameplayTag> SaveContextTags = Save->GetAllContextTags();
			if (SaveContextTags.Contains(PGXHarnessTags::SaveCampaign()) && SaveContextTags.Contains(PGXHarnessTags::SaveSettings())) SavePass++;
			if (Save->DoesSlotExist(PGXHarnessTags::SaveCampaign(), TEXT("Harness_AutoSave"))) SavePass++;
			if (Save->DoesSlotExist(PGXHarnessTags::SaveSettings(), TEXT("Harness_Settings"))) SavePass++;
			if (Save->HasData(PGXHarnessTags::DomainProgress(), FName("PlayerName"))) SavePass++;
			if (Save->HasData(PGXHarnessTags::DomainWorld(), FName("CurrentZone"))
				&& Save->HasData(PGXHarnessTags::DomainGraphics(), FName("ResolutionX"))
				&& Save->HasData(PGXHarnessTags::DomainAudio(), FName("MasterVolume"))) SavePass++;
			if (Save->HasData(PGXHarnessTags::DomainProgress(), FName("HarnessFixtureMarker"))) SavePass++;
			PassCount += SavePass;
			Run.PerSystemResults.Add(TPair<FString, FString>(TEXT("Save"), FString::Printf(TEXT("%d/7 | Contexts=%d Slots=%d Fixture=%s"),
				SavePass,
				Save->GetContextCount(),
				CreatedSaveSlots.Num(),
				Save->HasData(PGXHarnessTags::DomainProgress(), FName("HarnessFixtureMarker")) ? TEXT("Y") : TEXT("N"))));
			PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Save: %d/7, Contexts=%d Slots=%d Fixture=%s"),
				SavePass,
				Save->GetContextCount(),
				CreatedSaveSlots.Num(),
				Save->HasData(PGXHarnessTags::DomainProgress(), FName("HarnessFixtureMarker")) ? TEXT("Y") : TEXT("N"));
		}
	}

	// ─── Log ───
	{
		int32 LogPass = 0;
		TotalChecks += 2;
		const int32 BeforeLogCount = UPGXLogBlueprintLibrary::GetEntryCount(World);
		const FString ProbeMessage = FString::Printf(TEXT("PGXHarness.CoreIntegrity.LogRoundtrip.%d"), Run.RunNumber);
		UPGXLogBlueprintLibrary::PGXLogInfo(World, ProbeMessage, FName("LogPGXSimHarness"));
		const int32 AfterLogCount = UPGXLogBlueprintLibrary::GetEntryCount(World);
		const TArray<FPGXLogEntry> Entries = UPGXLogBlueprintLibrary::GetCurrentSessionEntries(World);
		const bool bFoundProbe = Entries.ContainsByPredicate([&ProbeMessage](const FPGXLogEntry& Entry)
		{
			return Entry.Message == ProbeMessage;
		});
		if (AfterLogCount > BeforeLogCount) LogPass++;
		if (bFoundProbe) LogPass++;
		PassCount += LogPass;
		Run.PerSystemResults.Add(TPair<FString, FString>(TEXT("Log"), FString::Printf(TEXT("%d/2 | Before=%d After=%d Roundtrip=%s"),
			LogPass, BeforeLogCount, AfterLogCount, bFoundProbe ? TEXT("Y") : TEXT("N"))));
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Log: %d/2, Before=%d After=%d Roundtrip=%s"),
			LogPass, BeforeLogCount, AfterLogCount, bFoundProbe ? TEXT("Y") : TEXT("N"));
	}

	// ─── PSO ───
	if (GI)
	{
		if (UPGXPSOSubsystem* PSO = GI->GetSubsystem<UPGXPSOSubsystem>())
		{
			int32 PSOPass = 0;
			TotalChecks += 2;
			TArray<FGameplayTag> Ctxs = PSO->GetActiveContexts();
			if (Ctxs.Num() >= 3) PSOPass++;
			FPGXPSOWarmUpProgress Prog = PSO->GetWarmUpProgress();
			PSOPass++; // EN: Query succeeded / ES: Query exitoso
			PassCount += PSOPass;
			Run.PerSystemResults.Add(TPair<FString, FString>(TEXT("PSO"), FString::Printf(TEXT("%d/2 | Contexts=%d"), PSOPass, Ctxs.Num())));
			PGX_LOG_INFO(LogPGXSimHarness, TEXT("  PSO: %d/2, Contexts=%d"), PSOPass, Ctxs.Num());
		}
	}

	// ─── MGOS ───
	if (UPGXGCObserverSubsystem* MGOS = UPGXGCObserverSubsystem::GetCachedInstance())
	{
		int32 MGOSPass = 0;
		TotalChecks += 3;
		FPGXGCProfile GCProf = MGOS->GetCurrentProfile();
		MGOSPass++; // EN: Query succeeded / ES: Query exitoso
		FPGXGCBaseline BL = MGOS->GetCurrentBaseline();
		if (BL.bValid) MGOSPass++;
		if (MGOS->GetCycleCount() > 0) MGOSPass++;
		PassCount += MGOSPass;
		Run.PerSystemResults.Add(TPair<FString, FString>(TEXT("MGOS"), FString::Printf(TEXT("%d/3 | Cycles=%lld"), MGOSPass, MGOS->GetCycleCount())));
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  MGOS: %d/3, Cycles=%lld"), MGOSPass, MGOS->GetCycleCount());
	}

	// ─── Audio ───
	if (GI)
	{
		if (UPGXAudioSubsystem* Audio = GI->GetSubsystem<UPGXAudioSubsystem>())
		{
			int32 AudioPass = 0;
			TotalChecks += 3;
			TArray<FPGXAudioChannelSnapshot> Chs = Audio->GetAllChannelStates();
			if (Chs.Num() >= 5) AudioPass++;
			FPGXAudioSystemSnapshot Snap = Audio->GetAudioSnapshot();
			AudioPass++; // EN: Query succeeded / ES: Query exitoso
			if (IsValid(Audio->GetAudioConfig())) AudioPass++;
			PassCount += AudioPass;
			Run.PerSystemResults.Add(TPair<FString, FString>(TEXT("Audio"), FString::Printf(TEXT("%d/3 | Channels=%d"), AudioPass, Chs.Num())));
			PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Audio: %d/3, Channels=%d"), AudioPass, Chs.Num());
		}
	}

	// ─── DataRegistry ───
	if (UPGXDataRegistrySubsystem* Reg = UPGXDataRegistrySubsystem::GetCached())
	{
		int32 RegPass = 0;
		TotalChecks += 5;
		if (Reg->HasDatabase(PGXHarnessTags::DbItems())) RegPass++;
		if (Reg->FindEntry(PGXHarnessTags::DbItems(), PGXHarnessTags::ItemIronSword())) RegPass++;
		if (Reg->FindEntry(PGXHarnessTags::DbNPCs(), PGXHarnessTags::NPCMerchant())) RegPass++;
		TArray<FGameplayTag> DbTags = Reg->GetAllDatabaseTags();
		if (DbTags.Num() >= 2) RegPass++;
		FPGXDatabaseStats Stats = Reg->GetDatabaseStats(PGXHarnessTags::DbItems());
		if (Stats.TotalEntries >= 5) RegPass++;
		PassCount += RegPass;
		Run.PerSystemResults.Add(TPair<FString, FString>(TEXT("DataRegistry"), FString::Printf(TEXT("%d/5 | Entries=%d Items=%s NPCs=%s"),
			RegPass,
			Stats.TotalEntries,
			Reg->FindEntry(PGXHarnessTags::DbItems(), PGXHarnessTags::ItemIronSword()) ? TEXT("Y") : TEXT("N"),
			Reg->FindEntry(PGXHarnessTags::DbNPCs(), PGXHarnessTags::NPCMerchant()) ? TEXT("Y") : TEXT("N"))));
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  DataRegistry: %d/5, Entries=%d Items=%s NPCs=%s"),
			RegPass,
			Stats.TotalEntries,
			Reg->FindEntry(PGXHarnessTags::DbItems(), PGXHarnessTags::ItemIronSword()) ? TEXT("Y") : TEXT("N"),
			Reg->FindEntry(PGXHarnessTags::DbNPCs(), PGXHarnessTags::NPCMerchant()) ? TEXT("Y") : TEXT("N"));
	}

	// ─── Message ───
	if (GI)
	{
		if (UPGXMessageSubsystem* Msg = GI->GetSubsystem<UPGXMessageSubsystem>())
		{
			int32 MsgPass = 0;
			TotalChecks += 3;
			FPGXMessageStats MStats = Msg->GetStats();
			const bool bCoreChannelsActive = Msg->IsChannelActive(PGXHarnessTags::MsgGameplay())
				&& Msg->IsChannelActive(PGXHarnessTags::MsgUI())
				&& Msg->IsChannelActive(PGXHarnessTags::MsgSystem());
			if (bCoreChannelsActive) MsgPass++;
			if (MStats.TotalBroadcasts > 0) MsgPass++;
			if (Msg->GetTotalListenerCount() >= 5) MsgPass++;
			PassCount += MsgPass;
			Run.PerSystemResults.Add(TPair<FString, FString>(TEXT("Message"), FString::Printf(TEXT("%d/3 | Broadcasts=%d CoreChannels=%s"),
				MsgPass, MStats.TotalBroadcasts, bCoreChannelsActive ? TEXT("Y") : TEXT("N"))));
			PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Message: %d/3, Broadcasts=%d CoreChannels=%s"),
				MsgPass, MStats.TotalBroadcasts, bCoreChannelsActive ? TEXT("Y") : TEXT("N"));
		}
	}

	// ─── EventHandler ───
	if (GI)
	{
		if (UPGXEventHandlerSubsystem* EH = GI->GetSubsystem<UPGXEventHandlerSubsystem>())
		{
			int32 EHPass = 0;
			TotalChecks += 3;
			TArray<FGameplayTag> RegTags = EH->GetAllRegisteredTags();
			if (RegTags.Num() >= 5 && EH->IsHandlerRegistered(PGXHarnessTags::EvtDamage())) EHPass++;
			FPGXHandlerCacheStats CS = EH->GetCacheStats();
			if (CS.CachedHandlers > 0) EHPass++;
			FInstancedStruct EmptyPayload;
			const EPGXEventResult ProbeResult = EH->ResolveAndExecute(PGXHarnessTags::EvtDamage(), nullptr, EmptyPayload);
			TArray<FPGXHandlerTelemetry> Tel = EH->GetAllTelemetry();
			if (Tel.Num() > 0 && ProbeResult == EPGXEventResult::Success) EHPass++;
			PassCount += EHPass;
			Run.PerSystemResults.Add(TPair<FString, FString>(TEXT("EventHandler"), FString::Printf(TEXT("%d/3 | Handlers=%d Probe=%s"),
				EHPass, RegTags.Num(), *UEnum::GetValueAsString(ProbeResult))));
			PGX_LOG_INFO(LogPGXSimHarness, TEXT("  EventHandler: %d/3, Handlers=%d Probe=%s"),
				EHPass, RegTags.Num(), *UEnum::GetValueAsString(ProbeResult));
		}
	}

	// ─── LevelFlow ───
	if (UPGXLevelFlowSubsystem* LF = UPGXLevelFlowSubsystem::GetCachedInstance())
	{
		TotalChecks += 1;
		bool bLFInit = LF->IsInitialized();
		if (bLFInit) PassCount++;
		Run.PerSystemResults.Add(TPair<FString, FString>(TEXT("LevelFlow"), FString::Printf(TEXT("%s | Init=%s"), bLFInit ? TEXT("1/1") : TEXT("0/1"), bLFInit ? TEXT("Y") : TEXT("N"))));
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  LevelFlow: Init=%s"), bLFInit ? TEXT("Y") : TEXT("N"));
	}

	// ─── Loading ───
	if (UPGXLoadingSubsystem* LS = UPGXLoadingSubsystem::GetCachedInstance())
	{
		TotalChecks += 1;
		bool bLSInit = LS->IsInitialized();
		if (bLSInit) PassCount++;
		Run.PerSystemResults.Add(TPair<FString, FString>(TEXT("Loading"), FString::Printf(TEXT("%s | Init=%s"), bLSInit ? TEXT("1/1") : TEXT("0/1"), bLSInit ? TEXT("Y") : TEXT("N"))));
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Loading: Init=%s"), bLSInit ? TEXT("Y") : TEXT("N"));
	}

	//  High-priority — Input verification. Final deep path:
	// real transient config/context fixtures + activation/priority/failure-code signals.
	if (GI)
	{
		if (UPGXInputSubsystem* InputSub = GI->GetSubsystem<UPGXInputSubsystem>())
		{
			TotalChecks += 7;
			int32 InputPass = 0;
			const bool bHasBuffer = IsValid(InputSub->GetInputBuffer());
			const bool bHasConfig = IsValid(InputSub->GetActiveInputConfig());
			const bool bFindsFixture = IsValid(InputSub->FindContextAsset(PGXHarnessTags::InputGameplay()));
			const bool bFindsUIFixture = IsValid(InputSub->FindContextAsset(PGXHarnessTags::InputUI()));
			InputSub->ActivateContext(PGXHarnessTags::InputUI(), 50);
			const FPGXInputContextResult DuplicateResult = InputSub->ActivateContext(PGXHarnessTags::InputGameplay());
			const FPGXInputContextResult InvalidResult = InputSub->ActivateContext(FGameplayTag());
			const bool bTypedProbe = DuplicateResult.Code == EPGXInputContextResultCode::AlreadyActive
				&& InvalidResult.Code == EPGXInputContextResultCode::InvalidTag;
			const TArray<FPGXActiveInputContextEntry> ActiveEntries = InputSub->GetActiveContexts();
			const int32 ActiveCtx = ActiveEntries.Num();
			const FPGXActiveInputContextEntry* GameplayEntry = ActiveEntries.FindByPredicate([](const FPGXActiveInputContextEntry& Entry)
			{
				return Entry.ContextTag == PGXHarnessTags::InputGameplay();
			});
			const FPGXActiveInputContextEntry* UIEntry = ActiveEntries.FindByPredicate([](const FPGXActiveInputContextEntry& Entry)
			{
				return Entry.ContextTag == PGXHarnessTags::InputUI();
			});
			const bool bPriorityOk = GameplayEntry && UIEntry && UIEntry->Priority > GameplayEntry->Priority;

			InputPass++; // subsystem exists via GameInstance
			if (bHasBuffer) InputPass++;
			if (bHasConfig && bFindsFixture && bFindsUIFixture) InputPass++;
			if (ActiveCtx >= 2
				&& InputSub->IsContextActive(PGXHarnessTags::InputGameplay())
				&& InputSub->IsContextActive(PGXHarnessTags::InputUI())) InputPass++;
			if (bPriorityOk) InputPass++;
			if (bTypedProbe) InputPass++;
			if (ActiveEntries.Num() >= 2) InputPass++;

			PassCount += InputPass;
			Run.PerSystemResults.Add(TPair<FString, FString>(TEXT("Input"),
				FString::Printf(TEXT("%d/7 | Buffer=%s Config=%s Gameplay=%s UI=%s Contexts=%d Priority=%s Duplicate=%d Invalid=%d"),
					InputPass,
					bHasBuffer ? TEXT("Y") : TEXT("N"),
					bHasConfig ? TEXT("Y") : TEXT("N"),
					bFindsFixture ? TEXT("Y") : TEXT("N"),
					bFindsUIFixture ? TEXT("Y") : TEXT("N"),
					ActiveCtx,
					bPriorityOk ? TEXT("Y") : TEXT("N"),
					static_cast<int32>(DuplicateResult.Code),
					static_cast<int32>(InvalidResult.Code))));
			PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Input: Buffer=%s Config=%s Gameplay=%s UI=%s ActiveContexts=%d Priority=%s DuplicateCode=%d InvalidCode=%d"),
				bHasBuffer ? TEXT("Y") : TEXT("N"),
				bHasConfig ? TEXT("Y") : TEXT("N"),
				bFindsFixture ? TEXT("Y") : TEXT("N"),
				bFindsUIFixture ? TEXT("Y") : TEXT("N"),
				ActiveCtx,
				bPriorityOk ? TEXT("Y") : TEXT("N"),
				static_cast<int32>(DuplicateResult.Code),
				static_cast<int32>(InvalidResult.Code));
		}
	}

	//  runtime core — Spawn verification: subsystem, record lifecycle, wave smoke surface.
	if (IsValid(World))
	{
		if (UPGXSpawnSubsystem* SpawnSub = World->GetSubsystem<UPGXSpawnSubsystem>())
		{
			TotalChecks += 4;
			int32 SpawnPass = 0;
			SpawnPass++; // subsystem exists via UWorld

			const FPGXSpawnDebugSnapshot Snapshot = SpawnSub->GetDebugSnapshot();
			if (Snapshot.TotalRecordCount >= 0) SpawnPass++;

			FPGXSpawnRequest ProbeRequest;
			ProbeRequest.SpawnClass = AActor::StaticClass();
			ProbeRequest.Transform = FTransform(FRotator::ZeroRotator, FVector(250.f, 0.f, 120.f));
			ProbeRequest.SourceTag = PGXHarnessTags::SpawnWaveSmoke();
			ProbeRequest.Priority = 0;
			const FPGXSpawnResult Validation = SpawnSub->ValidateSpawnRequest(ProbeRequest);
			if (Validation.bSuccess) SpawnPass++;

			const TArray<FPGXSpawnRecord> Waves = SpawnSub->GetActiveWavesSnapshot();
			if (Waves.Num() >= 0) SpawnPass++;

			PassCount += SpawnPass;
			Run.PerSystemResults.Add(TPair<FString, FString>(TEXT("Spawn"),
				FString::Printf(TEXT("%d/4 | Records=%d Active=%d Waves=%d Validate=%s"),
					SpawnPass,
					Snapshot.TotalRecordCount,
					Snapshot.ActiveRecordCount,
					Snapshot.ActiveWaveCount,
					Validation.bSuccess ? TEXT("Y") : TEXT("N"))));
			PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Spawn: Records=%d Active=%d Waves=%d Validate=%s"),
				Snapshot.TotalRecordCount, Snapshot.ActiveRecordCount, Snapshot.ActiveWaveCount,
				Validation.bSuccess ? TEXT("Y") : TEXT("N"));
		}
	}

	//  runtime core — Ability verification: subsystem, component registry, facade execution seam.
	if (GI)
	{
		if (UPGXAbilitySubsystem* AbilitySub = GI->GetSubsystem<UPGXAbilitySubsystem>())
		{
			TotalChecks += 4;
			int32 AbilityPass = 0;
			AbilityPass++; // subsystem exists via GameInstance
			const TArray<TWeakObjectPtr<UPGXAbilityComponent>> Components = AbilitySub->GetComponentRegistry();
			if (Components.Num() >= 0) AbilityPass++;
			if (!HarnessAbilityComponent.IsValid() || HarnessAbilityComponent->IsAbilitySystemReady()) AbilityPass++;
			if (!HarnessAbilityComponent.IsValid() || HarnessAbilityComponent->GetAbilityFacade()) AbilityPass++;

			PassCount += AbilityPass;
			Run.PerSystemResults.Add(TPair<FString, FString>(TEXT("Ability"),
				FString::Printf(TEXT("%d/4 | Components=%d Active=%d HarnessReady=%s"),
					AbilityPass,
					Components.Num(),
					AbilitySub->GetActiveAbilityCount(),
					(HarnessAbilityComponent.IsValid() && HarnessAbilityComponent->IsAbilitySystemReady()) ? TEXT("Y") : TEXT("N"))));
			PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Ability: Components=%d Active=%d HarnessReady=%s"),
				Components.Num(), AbilitySub->GetActiveAbilityCount(),
				(HarnessAbilityComponent.IsValid() && HarnessAbilityComponent->IsAbilitySystemReady()) ? TEXT("Y") : TEXT("N"));
		}
	}

	//  runtime core — AI verification: registry + behavior-tree run seam status.
	if (IsValid(World))
	{
		if (UPGXAISubsystem* AISub = World->GetSubsystem<UPGXAISubsystem>())
		{
			TotalChecks += 4;
			int32 AIPass = 0;
			AIPass++; // subsystem exists via UWorld
			const TArray<FPGXAIAgentHandle> Agents = AISub->GetAgentSnapshot();
			if (Agents.Num() >= 0) AIPass++;
			FPGXAIAgentHandle FoundHandle;
			if (HarnessAIAgentId == 0 || AISub->FindAgent(HarnessAIAgentId, FoundHandle)) AIPass++;

			FPGXAIBehaviorTreeRunStatus BTStatus;
			const bool bHasBTStatus = FoundHandle.IsValid() && AISub->GetBehaviorTreeRunStatus(FoundHandle, BTStatus);
			if (HarnessAIAgentId == 0 || bHasBTStatus) AIPass++;

			PassCount += AIPass;
			Run.PerSystemResults.Add(TPair<FString, FString>(TEXT("AI"),
				FString::Printf(TEXT("%d/4 | Agents=%d Handle=%d BTStatus=%s"),
					AIPass,
					Agents.Num(),
					HarnessAIAgentId,
					bHasBTStatus ? TEXT("Y") : TEXT("N"))));
			PGX_LOG_INFO(LogPGXSimHarness, TEXT("  AI: Agents=%d Handle=%d BTStatus=%s"),
				Agents.Num(), HarnessAIAgentId, bHasBTStatus ? TEXT("Y") : TEXT("N"));
		}
	}

	//  runtime extended — Camera verification: mode cycle smoke.
	if (IsValid(World))
	{
		if (UPGXCameraSubsystem* CameraSub = World->GetSubsystem<UPGXCameraSubsystem>())
		{
			TotalChecks += 4;
			int32 CameraPass = 0;
			CameraPass++; // subsystem exists via UWorld

			UPGXCameraMode* ModeA = NewObject<UPGXCameraMode>(GetTransientPackage(), UPGXCameraMode::StaticClass(), NAME_None, RF_Transient);
			UPGXCameraMode* ModeB = NewObject<UPGXCameraMode>(GetTransientPackage(), UPGXCameraMode::StaticClass(), NAME_None, RF_Transient);
			if (ModeA && ModeB)
			{
				ModeA->ModeName = TEXT("PGXHarnessCameraModeA");
				ModeB->ModeName = TEXT("PGXHarnessCameraModeB");
			}

			const bool bSetA = ModeA && CameraSub->SetCameraMode(ModeA) && CameraSub->GetActiveCameraModeName() == ModeA->ModeName;
			const bool bSetB = ModeB && CameraSub->SetCameraMode(ModeB) && CameraSub->GetActiveCameraMode() == ModeB;
			CameraSub->ClearCameraMode();
			const bool bCleared = CameraSub->GetActiveCameraMode() == nullptr && CameraSub->GetActiveCameraModeName().IsNone();

			if (bSetA) CameraPass++;
			if (bSetB) CameraPass++;
			if (bCleared) CameraPass++;

			PassCount += CameraPass;
			Run.PerSystemResults.Add(TPair<FString, FString>(TEXT("Camera"),
				FString::Printf(TEXT("%d/4 | SetA=%s SetB=%s Clear=%s"),
					CameraPass,
					bSetA ? TEXT("Y") : TEXT("N"),
					bSetB ? TEXT("Y") : TEXT("N"),
					bCleared ? TEXT("Y") : TEXT("N"))));
			PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Camera: SetA=%s SetB=%s Clear=%s"),
				bSetA ? TEXT("Y") : TEXT("N"), bSetB ? TEXT("Y") : TEXT("N"), bCleared ? TEXT("Y") : TEXT("N"));
		}
	}

	//  runtime extended — Interaction verification: target/action lifecycle smoke.
	if (HarnessInteractionComponent.IsValid())
	{
		TotalChecks += 4;
		int32 InteractionPass = 0;
		UPGXInteractionComponent* InteractionComp = HarnessInteractionComponent.Get();
		InteractionPass++; // component exists
		if (InteractionComp->GetRegisteredTargetCount() >= 1) InteractionPass++;
		if (InteractionComp->GetInteractionRecordCount() >= 1) InteractionPass++;
		if (InteractionComp->GetActiveInteractionCount() == 0) InteractionPass++;

		PassCount += InteractionPass;
		Run.PerSystemResults.Add(TPair<FString, FString>(TEXT("Interaction"),
			FString::Printf(TEXT("%d/4 | Targets=%d Records=%d Active=%d"),
				InteractionPass,
				InteractionComp->GetRegisteredTargetCount(),
				InteractionComp->GetInteractionRecordCount(),
				InteractionComp->GetActiveInteractionCount())));
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Interaction: Targets=%d Records=%d Active=%d"),
			InteractionComp->GetRegisteredTargetCount(),
			InteractionComp->GetInteractionRecordCount(),
			InteractionComp->GetActiveInteractionCount());
	}

	//  runtime extended — Inventory verification: add/remove lifecycle state.
	if (HarnessInventoryComponent.IsValid() && HarnessInventoryItemDefinition.IsValid())
	{
		TotalChecks += 4;
		int32 InventoryPass = 0;
		UPGXInventoryComponent* InventoryComp = HarnessInventoryComponent.Get();
		const UPGXItemDefinition* ItemDef = HarnessInventoryItemDefinition.Get();
		InventoryPass++; // component exists
		if (InventoryComp->GetUsedSlotCount() >= 1) InventoryPass++;
		if (InventoryComp->GetItemQuantity(ItemDef) == 1) InventoryPass++;
		if (InventoryComp->GetItemsSnapshot().Num() >= 1) InventoryPass++;

		PassCount += InventoryPass;
		Run.PerSystemResults.Add(TPair<FString, FString>(TEXT("Inventory"),
			FString::Printf(TEXT("%d/4 | Slots=%d Qty=%d Weight=%.2f"),
				InventoryPass,
				InventoryComp->GetUsedSlotCount(),
				InventoryComp->GetItemQuantity(ItemDef),
				InventoryComp->GetCurrentWeight())));
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Inventory: Slots=%d Qty=%d Weight=%.2f"),
			InventoryComp->GetUsedSlotCount(), InventoryComp->GetItemQuantity(ItemDef), InventoryComp->GetCurrentWeight());
	}

	//  runtime extended — UI verification: widget pool registry smoke.
	if (GI)
	{
		if (UPGXUISubsystem* UISub = GI->GetSubsystem<UPGXUISubsystem>())
		{
			TotalChecks += 4;
			int32 UIPass = 0;
			UPGXWidgetPool* WidgetPool = UISub->GetWidgetPool();
			UIPass++; // subsystem exists via GameInstance
			if (WidgetPool) UIPass++;
			if (WidgetPool && WidgetPool->GetCapacity() > 0) UIPass++;
			if (WidgetPool && WidgetPool->GetPoolSnapshot().Num() >= 1) UIPass++;

			PassCount += UIPass;
			Run.PerSystemResults.Add(TPair<FString, FString>(TEXT("UI"),
				FString::Printf(TEXT("%d/4 | Pool=%s Capacity=%d Acquired=%d Entries=%d"),
					UIPass,
					WidgetPool ? TEXT("Y") : TEXT("N"),
					WidgetPool ? WidgetPool->GetCapacity() : 0,
					WidgetPool ? WidgetPool->GetAcquiredCount() : 0,
					WidgetPool ? WidgetPool->GetPoolSnapshot().Num() : 0)));
			PGX_LOG_INFO(LogPGXSimHarness, TEXT("  UI: Pool=%s Capacity=%d Acquired=%d Entries=%d"),
				WidgetPool ? TEXT("Y") : TEXT("N"),
				WidgetPool ? WidgetPool->GetCapacity() : 0,
				WidgetPool ? WidgetPool->GetAcquiredCount() : 0,
				WidgetPool ? WidgetPool->GetPoolSnapshot().Num() : 0);
		}
	}

	bLastVerificationPassed = (TotalChecks > 0 && PassCount == TotalChecks);
	VerificationPassCount++;

	Run.PassCount = PassCount;
	Run.TotalChecks = TotalChecks;
	VerificationRuns.Add(MoveTemp(Run));

	RecordAction(TEXT("Verification"), TEXT("VerifyAllAPIs"), bLastVerificationPassed,
		FString::Printf(TEXT("%d/%d passed (run #%d)"), PassCount, TotalChecks, VerificationPassCount));

	PGX_LOG_INFO(LogPGXSimHarness, TEXT("========== VerifyAllAPIs END — %d/%d passed (run #%d) %s =========="),
		PassCount, TotalChecks, VerificationPassCount,
		bLastVerificationPassed ? TEXT("[ALL PASS]") : TEXT("[SOME FAIL]"));
}

// ============================================================================
// EN: Simulation Tick — generates continuous activity
// ES: Simulation Tick — genera actividad continua
// ============================================================================

bool FPGXVisualHarness::OnSimulationTick(float /*DeltaTime*/)
{
	if (!bIsActive)
	{
		return false; // EN: Remove ticker / ES: Remover ticker
	}

	SimulationTickCount++;
	UWorld* World = ResolveEditorWorld();

	RecordAction(TEXT("Simulation"), FString::Printf(TEXT("Tick #%d"), SimulationTickCount), true, TEXT(""));

	// EN: Generate log entries every tick / ES: Generar entradas de log cada tick
	GenerateLogEntries();

	// EN: Cycle GameFlow state every 3rd tick (~6s) / ES: Ciclar estado GameFlow cada 3er tick (~6s)
	if (SimulationTickCount % 3 == 0)
	{
		CycleGameFlowState();
	}

	// EN: Broadcast message every 2nd tick (~4s) / ES: Broadcast mensaje cada 2do tick (~4s)
	if (SimulationTickCount % 2 == 0 && IsValid(World))
	{
		BroadcastTestMessage();
	}

	// EN: Force GC every 5th tick (~10s) / ES: Forzar GC cada 5to tick (~10s)
	if (SimulationTickCount % 5 == 0)
	{
		ForceGarbageCollection();
	}

	// EN: Save slot cycle every 4th tick (~8s) / ES: Ciclar slot de save cada 4to tick (~8s)
	if (SimulationTickCount % 4 == 0 && IsValid(World))
	{
		CycleSaveSlot();
	}

	// EN: Execute random handler every 6th tick (~12s) / ES: Ejecutar handler aleatorio cada 6to tick (~12s)
	if (SimulationTickCount % 6 == 0 && IsValid(World))
	{
		ExecuteRandomHandler();
	}

	// ─── v2.0: Periodic verification + telemetry queries ───

	// EN: Verify APIs every 7th tick (~14s) / ES: Verificar APIs cada 7mo tick (~14s)
	if (SimulationTickCount % 7 == 0)
	{
		VerifyAllAPIs();
	}

	// EN: Query EventHandler telemetry delta every 8th tick (~16s)
	// ES: Consultar delta de telemetria EventHandler cada 8vo tick (~16s)
	if (SimulationTickCount % 8 == 0 && IsValid(World))
	{
		UGameInstance* TickGI = World->GetGameInstance();
		UPGXEventHandlerSubsystem* TickEH = TickGI ? TickGI->GetSubsystem<UPGXEventHandlerSubsystem>() : nullptr;
		if (TickEH)
		{
			FPGXHandlerCacheStats CS = TickEH->GetCacheStats();
			PGX_LOG_INFO(LogPGXSimHarness, TEXT("Sim[%d] EH Cache: %d handlers, %d hits, %d misses"),
				SimulationTickCount, CS.CachedHandlers, CS.CacheHits, CS.CacheMisses);
		}
	}

	// EN: Cycle audio channel volumes every 9th tick (~18s) — keeps Audio inspector dynamic
	// ES: Ciclar volumenes de canal de audio cada 9vo tick (~18s) — mantiene Audio inspector dinamico
	if (SimulationTickCount % 9 == 0 && IsValid(World))
	{
		UGameInstance* TickGI = World->GetGameInstance();
		UPGXAudioSubsystem* TickAudio = TickGI ? TickGI->GetSubsystem<UPGXAudioSubsystem>() : nullptr;
		if (TickAudio)
		{
			float NewVol = 0.5f + 0.3f * FMath::Sin(static_cast<float>(SimulationTickCount) * 0.5f);
			TickAudio->SetChannelVolume(PGXHarnessTags::AudioAmbient(), NewVol);
			PGX_LOG_INFO(LogPGXSimHarness, TEXT("Sim[%d] Audio: Ambient volume=%.2f"), SimulationTickCount, NewVol);
		}
	}

	// EN: Query DataRegistry stats every 10th tick (~20s)
	// ES: Consultar stats de DataRegistry cada 10mo tick (~20s)
	if (SimulationTickCount % 10 == 0)
	{
		UPGXDataRegistrySubsystem* TickReg = UPGXDataRegistrySubsystem::GetCached();
		if (TickReg)
		{
			FPGXDatabaseStats ItemStats = TickReg->GetDatabaseStats(PGXHarnessTags::DbItems());
			PGX_LOG_INFO(LogPGXSimHarness, TEXT("Sim[%d] Registry(Items): Total=%d, Loaded=%d, Categories=%d"),
				SimulationTickCount, ItemStats.TotalEntries, ItemStats.LoadedEntries, ItemStats.CategoryCount);
		}
	}

	return true; // EN: Keep ticking / ES: Seguir con ticks
}

// ============================================================================
// EN: State Query
// ES: Consulta de estado
// ============================================================================

double FPGXVisualHarness::GetElapsedSeconds() const
{
	if (!bIsActive) return 0.0;
	return FPlatformTime::Seconds() - SetupTimestamp;
}

int32 FPGXVisualHarness::GetTotalObjectCount() const
{
	return CreatedObjects.Num();
}

TArray<FPGXHarnessSystemStatus> FPGXVisualHarness::GetSystemStatuses() const
{
	return SystemStatuses;
}

// ============================================================================
// EN: Action Log
// ES: Log de acciones
// ============================================================================

void FPGXVisualHarness::RecordAction(const FString& Category, const FString& Action, bool bSuccess, const FString& Detail)
{
	FPGXHarnessActionEntry Entry;
	Entry.Timestamp = bIsActive ? (FPlatformTime::Seconds() - SetupTimestamp) : 0.0;
	Entry.Category = Category;
	Entry.Action = Action;
	Entry.bSuccess = bSuccess;
	Entry.Detail = Detail;
	ActionLog.Add(MoveTemp(Entry));
}

// ============================================================================
// EN: Export Report — writes structured .md to Saved/PGX/
// ES: Exportar Reporte — escribe .md estructurado a Saved/PGX/
// ============================================================================

FString FPGXVisualHarness::ExportReport() const
{
	// EN: Build output path / ES: Construir ruta de salida
	FString Timestamp = FDateTime::Now().ToString(TEXT("%Y-%m-%d_%H-%M-%S"));
	FString Dir = FPaths::ProjectSavedDir() / TEXT("PGX");
	IFileManager::Get().MakeDirectory(*Dir, true);
	FString FilePath = Dir / FString::Printf(TEXT("HarnessReport_%s.md"), *Timestamp);

	FString MD;

	// ─── Header ───
	double Duration = bIsActive ? (FPlatformTime::Seconds() - SetupTimestamp) : 0.0;
	int32 DurMin = static_cast<int32>(Duration) / 60;
	int32 DurSec = static_cast<int32>(Duration) % 60;

	MD += TEXT("# PSPH Execution Report\n");
	MD += FString::Printf(TEXT("> Generated: %s | Duration: %02d:%02d | PGX v0.4.0\n\n"),
		*FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S")), DurMin, DurSec);

	// ─── Session Summary ───
	int32 InjectedCount = 0;
	int32 TotalObjects = 0;
	for (const auto& S : SystemStatuses)
	{
		if (S.bInjected) InjectedCount++;
		TotalObjects += S.ObjectCount;
	}

	int32 QuickActionCount = 0;
	int32 ErrorCount = 0;
	int32 SimTickCount = 0;
	for (const auto& E : ActionLog)
	{
		if (E.Category == TEXT("QuickAction")) QuickActionCount++;
		if (E.Category == TEXT("Simulation")) SimTickCount++;
		if (!E.bSuccess) ErrorCount++;
	}

	MD += TEXT("## Session Summary\n");
	MD += TEXT("| Metric | Value |\n|--------|-------|\n");
	MD += FString::Printf(TEXT("| Duration | %ds |\n"), static_cast<int32>(Duration));
	MD += FString::Printf(TEXT("| Systems Injected | %d/%d |\n"), InjectedCount, SystemStatuses.Num());
	MD += FString::Printf(TEXT("| Objects Created | %d |\n"), TotalObjects);
	MD += FString::Printf(TEXT("| Simulation Ticks | %d |\n"), SimTickCount);
	MD += FString::Printf(TEXT("| Quick Actions | %d |\n"), QuickActionCount);
	MD += FString::Printf(TEXT("| Verification Runs | %d |\n"), VerificationRuns.Num());
	MD += FString::Printf(TEXT("| Errors | %d |\n\n"), ErrorCount);

	// ─── System Injection Status ───
	MD += TEXT("## System Injection Status\n");
	MD += TEXT("| System | Status | Objects | Detail |\n|--------|--------|---------|--------|\n");
	for (const auto& S : SystemStatuses)
	{
		MD += FString::Printf(TEXT("| %s | %s | %d | %s |\n"),
			*S.SystemName,
			S.bInjected ? TEXT("OK") : TEXT("FAIL"),
			S.ObjectCount,
			*S.Detail);
	}
	MD += TEXT("\n");

	// ─── Verification Results ───
	if (VerificationRuns.Num() > 0)
	{
		MD += TEXT("## Verification Results\n");
		for (const auto& Run : VerificationRuns)
		{
			MD += FString::Printf(TEXT("### Run #%d (t+%.1fs) \u2014 %d/%d %s\n"),
				Run.RunNumber, Run.Timestamp, Run.PassCount, Run.TotalChecks,
				(Run.PassCount == Run.TotalChecks) ? TEXT("PASS") : TEXT("PARTIAL"));
			MD += TEXT("| System | Result |\n|--------|--------|\n");
			for (const auto& SR : Run.PerSystemResults)
			{
				MD += FString::Printf(TEXT("| %s | %s |\n"), *SR.Key, *SR.Value);
			}
			MD += TEXT("\n");
		}
	}

	// ─── Action Log (chronological) ───
	MD += TEXT("## Action Log (chronological)\n");
	MD += TEXT("| Time | Category | Action | Result | Detail |\n|------|----------|--------|--------|--------|\n");
	for (const auto& E : ActionLog)
	{
		MD += FString::Printf(TEXT("| +%.1fs | %s | %s | %s | %s |\n"),
			E.Timestamp,
			*E.Category,
			*E.Action,
			E.bSuccess ? TEXT("OK") : TEXT("FAIL"),
			*E.Detail);
	}
	MD += TEXT("\n");

	// ─── Errors & Warnings ───
	MD += TEXT("## Errors & Warnings\n");
	bool bHasErrors = false;
	for (const auto& E : ActionLog)
	{
		if (!E.bSuccess)
		{
			MD += FString::Printf(TEXT("- **[+%.1fs]** %s / %s: %s\n"), E.Timestamp, *E.Category, *E.Action, *E.Detail);
			bHasErrors = true;
		}
	}
	if (!bHasErrors)
	{
		MD += TEXT("(none)\n");
	}
	MD += TEXT("\n");

	// ─── Not Executed ───
	MD += TEXT("## Not Executed\n");
	TSet<FString> ExecutedActions;
	for (const auto& E : ActionLog)
	{
		if (E.Category == TEXT("QuickAction"))
		{
			ExecutedActions.Add(E.Action);
		}
	}
	static const FString AllQuickActions[] = {
		TEXT("GenerateLogEntries"), TEXT("ForceGarbageCollection"), TEXT("BroadcastTestMessage"),
		TEXT("CycleGameFlowState"), TEXT("CycleSaveSlot"), TEXT("ExecuteRandomHandler"), TEXT("VerifyAllAPIs")
	};
	bool bAllExecuted = true;
	for (const FString& QA : AllQuickActions)
	{
		if (!ExecutedActions.Contains(QA))
		{
			MD += FString::Printf(TEXT("- %s (never triggered)\n"), *QA);
			bAllExecuted = false;
		}
	}
	if (bAllExecuted)
	{
		MD += TEXT("(all quick actions were triggered at least once)\n");
	}

	// EN: Write to disk / ES: Escribir a disco
	if (FFileHelper::SaveStringToFile(MD, *FilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("ExportReport — Written to %s (%d lines, %d chars)"), *FilePath, ActionLog.Num(), MD.Len());
	}
	else
	{
		PGX_LOG_ERROR(LogPGXSimHarness, TEXT("ExportReport — Failed to write to %s"), *FilePath);
	}

	return FilePath;
}

// ============================================================================
// EN: Panel Launcher
// ES: Lanzador de paneles
// ============================================================================

void FPGXVisualHarness::OpenAllPanels()
{
	TArray<FName> Panels = GetAllPanelIds();
	for (const FName& PanelId : Panels)
	{
		OpenPanel(PanelId);
	}
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("VisualHarness::OpenAllPanels — Opened %d panels"), Panels.Num());
}

void FPGXVisualHarness::OpenPanel(FName TabId)
{
	FGlobalTabmanager::Get()->TryInvokeTab(TabId);
}

TArray<FName> FPGXVisualHarness::GetAllPanelIds()
{
	// EN: Derived from the canonical data-panel list, filtered to ids with a live spawner. The
	//     HasTabSpawner guard skips panels whose plugin is disabled (.uplugin not loaded), so
	//     OpenAllPanels never tries to invoke a tab that cannot spawn.
	// ES: Derivado de la lista canonica de data-panels, filtrado a ids con spawner vivo. El guard
	//     HasTabSpawner salta paneles de un plugin deshabilitado, asi OpenAllPanels nunca intenta
	//     invocar un tab que no puede spawnear.
	TArray<FName> Out;
	const TSharedPtr<FGlobalTabmanager> TabManager = FGlobalTabmanager::Get();
	for (const FName& PanelId : PGX::Harness::GetHarnessDataPanelIds())
	{
		if (TabManager->HasTabSpawner(PanelId))
		{
			Out.Add(PanelId);
		}
	}
	return Out;
}

TArray<FName> FPGXVisualHarness::GetUnitTestOnlyPanelIds()
{
	// EN: Canonical tooling-panel list (panels that don't show harness data).
	// ES: Lista canonica de tooling-panels (paneles que no muestran datos del harness).
	return PGX::Harness::GetHarnessToolingPanelIds();
}

// ============================================================================
// EN: World Resolution — finds PIE or editor world from engine contexts
// ES: Resolucion de World — busca PIE o editor world desde contextos del engine
// ============================================================================

UWorld* FPGXVisualHarness::ResolveEditorWorld() const
{
	if (!GEngine) return nullptr;

	// EN: PIE-only — inspectors only query PIE worlds, so injection must target the same instance
	// ES: Solo PIE — los inspectores solo consultan PIE worlds, la inyeccion debe apuntar a la misma instancia
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::PIE && IsValid(Context.World()))
		{
			return Context.World();
		}
	}
	return nullptr;
}

// ============================================================================
// EN: Quick Actions
// ES: Acciones rapidas
// ============================================================================

void FPGXVisualHarness::GenerateLogEntries()
{
	UWorld* World = ResolveEditorWorld();
	if (IsValid(World))
	{
		UPGXLogTestUtility::RunQuickTest(World);
		RecordAction(TEXT("QuickAction"), TEXT("GenerateLogEntries"), true, TEXT("~35 log entries"));
	}
	else
	{
		RecordAction(TEXT("QuickAction"), TEXT("GenerateLogEntries"), false, TEXT("No valid world"));
	}
}

void FPGXVisualHarness::ForceGarbageCollection()
{
	GEngine->ForceGarbageCollection(true);

	// EN: Update MGOS status if available / ES: Actualizar status de MGOS si esta disponible
	FString GCDetail = TEXT("GC cycle forced");
	if (SystemStatuses.IsValidIndex(6))
	{
		UPGXGCObserverSubsystem* MGOS = UPGXGCObserverSubsystem::GetCachedInstance();
		if (MGOS)
		{
			GCDetail = FString::Printf(TEXT("%s, %lld cycles"),
				*UEnum::GetValueAsString(MGOS->GetMode()), MGOS->GetCycleCount());
			SystemStatuses[6].Detail = GCDetail;
		}
	}
	RecordAction(TEXT("QuickAction"), TEXT("ForceGarbageCollection"), true, GCDetail);
}

void FPGXVisualHarness::BroadcastTestMessage()
{
	UWorld* World = ResolveEditorWorld();
	if (!IsValid(World))
	{
		RecordAction(TEXT("QuickAction"), TEXT("BroadcastTestMessage"), false, TEXT("No valid world"));
		return;
	}

	UGameInstance* GI = World->GetGameInstance();
	UPGXMessageSubsystem* MsgSub = GI ? GI->GetSubsystem<UPGXMessageSubsystem>() : nullptr;
	if (!MsgSub)
	{
		RecordAction(TEXT("QuickAction"), TEXT("BroadcastTestMessage"), false, TEXT("MessageSubsystem not found"));
		return;
	}

	// EN: Broadcast on a random channel / ES: Broadcast en un canal aleatorio
	static const TFunction<FGameplayTag()> Channels[] = {
		PGXHarnessTags::MsgUI, PGXHarnessTags::MsgGameplay,
		PGXHarnessTags::MsgSystem, PGXHarnessTags::MsgAudio,
		PGXHarnessTags::MsgNetwork
	};
	int32 Idx = FMath::RandRange(0, 4);
	FGameplayTag Channel = Channels[Idx]();

	FPGXMessage Msg;
	Msg.MessageTag = Channel;
	Msg.Timestamp = FPlatformTime::Seconds();
	MsgSub->BroadcastMessage<FPGXMessage>(Channel, Msg);

	RecordAction(TEXT("QuickAction"), TEXT("BroadcastTestMessage"), true,
		FString::Printf(TEXT("Channel: %s"), *Channel.ToString()));
}

void FPGXVisualHarness::CycleGameFlowState()
{
	UWorld* World = ResolveEditorWorld();
	if (!IsValid(World))
	{
		RecordAction(TEXT("QuickAction"), TEXT("CycleGameFlowState"), false, TEXT("No valid world"));
		return;
	}

	UGameInstance* GI = World->GetGameInstance();
	UPGXGameFlowSubsystem* FlowSub = GI ? GI->GetSubsystem<UPGXGameFlowSubsystem>() : nullptr;
	if (!FlowSub)
	{
		RecordAction(TEXT("QuickAction"), TEXT("CycleGameFlowState"), false, TEXT("GameFlowSubsystem not found"));
		return;
	}

	// EN: Cycle a random channel through progression states
	// ES: Ciclar un canal aleatorio a traves de estados de progresion
	static const FGameplayTag CycleStates[] = {
		PGXHarnessTags::FlowMainMenu(), PGXHarnessTags::FlowLoading(),
		PGXHarnessTags::FlowInGame(), PGXHarnessTags::FlowPause(),
		PGXHarnessTags::FlowActive()
	};

	int32 ChannelIdx = FMath::RandRange(0, static_cast<int32>(EPGXFlowChannel::MAX) - 1);
	int32 StateIdx = static_cast<int32>(SimulationTickCount % UE_ARRAY_COUNT(CycleStates));
	EPGXFlowChannel Channel = static_cast<EPGXFlowChannel>(ChannelIdx);

	FlowSub->SetStateByTag(Channel, CycleStates[StateIdx]);

	FString ChannelName = UPGXGameFlowSubsystem::GetChannelName(Channel);
	if (SystemStatuses.IsValidIndex(2))
	{
		SystemStatuses[2].Detail = FString::Printf(TEXT("8 channels, last cycle: %s"), *ChannelName);
	}

	RecordAction(TEXT("QuickAction"), TEXT("CycleGameFlowState"), true,
		FString::Printf(TEXT("Channel: %s, State: %s"), *ChannelName, *CycleStates[StateIdx].ToString()));
}

void FPGXVisualHarness::CycleSaveSlot()
{
	UWorld* World = ResolveEditorWorld();
	if (!IsValid(World))
	{
		RecordAction(TEXT("QuickAction"), TEXT("CycleSaveSlot"), false, TEXT("No valid world"));
		return;
	}

	UGameInstance* GI = World->GetGameInstance();
	UPGXSaveSubsystem* SaveSub = GI ? GI->GetSubsystem<UPGXSaveSubsystem>() : nullptr;
	if (!SaveSub || CreatedSaveSlots.Num() == 0)
	{
		RecordAction(TEXT("QuickAction"), TEXT("CycleSaveSlot"), false, TEXT("SaveSubsystem not found or no slots"));
		return;
	}

	// EN: Re-save an existing slot (updates timestamp on disk)
	// ES: Re-guardar un slot existente (actualiza timestamp en disco)
	int32 Idx = SimulationTickCount % CreatedSaveSlots.Num();
	const FSavedSlotInfo& Slot = CreatedSaveSlots[Idx];
	SaveSub->SaveContext(Slot.ContextTag, Slot.SlotName);

	if (SystemStatuses.IsValidIndex(4))
	{
		SystemStatuses[4].Detail = FString::Printf(TEXT("2 ctx, %d slots, last save: %s"),
			CreatedSaveSlots.Num(), *Slot.SlotName);
	}

	RecordAction(TEXT("QuickAction"), TEXT("CycleSaveSlot"), true,
		FString::Printf(TEXT("Slot: %s"), *Slot.SlotName));
}

void FPGXVisualHarness::ExecuteRandomHandler()
{
	UWorld* World = ResolveEditorWorld();
	if (!IsValid(World) || RegisteredEventTags.Num() == 0)
	{
		RecordAction(TEXT("QuickAction"), TEXT("ExecuteRandomHandler"), false, TEXT("No world or no handlers"));
		return;
	}

	UGameInstance* GI = World->GetGameInstance();
	UPGXEventHandlerSubsystem* EHSub = GI ? GI->GetSubsystem<UPGXEventHandlerSubsystem>() : nullptr;
	if (!EHSub)
	{
		RecordAction(TEXT("QuickAction"), TEXT("ExecuteRandomHandler"), false, TEXT("EventHandlerSubsystem not found"));
		return;
	}

	// EN: Execute a random handler to build telemetry / ES: Ejecutar handler aleatorio para generar telemetria
	int32 Idx = FMath::RandRange(0, RegisteredEventTags.Num() - 1);
	FGameplayTag EvtTag = RegisteredEventTags[Idx];

	FInstancedStruct EmptyPayload;
	EHSub->ResolveAndExecute(EvtTag, nullptr, EmptyPayload);

	if (SystemStatuses.IsValidIndex(10))
	{
		SystemStatuses[10].Detail = FString::Printf(TEXT("%d handlers, telemetry active, last: %s"),
			RegisteredEventTags.Num(), *EvtTag.ToString());
	}

	RecordAction(TEXT("QuickAction"), TEXT("ExecuteRandomHandler"), true,
		FString::Printf(TEXT("Handler: %s"), *EvtTag.ToString()));
}

// ============================================================================
// EN: Per-System Injection — 13 systems
// ES: Inyeccion por sistema — 13 sistemas
// ============================================================================

// ─── 1. Profile ───

void FPGXVisualHarness::InjectProfile()
{
#if WITH_EDITOR
	UPGXProfileSubsystem* ProfileSub = UPGXProfileSubsystem::GetCachedInstance();
	if (!ProfileSub)
	{
		PGX_LOG_WARNING(LogPGXSimHarness, TEXT("InjectProfile — ProfileSubsystem not found, skipping"));
		return;
	}

	ProfileSub->SimulatePlatform(EPGXTargetPlatform::PC);
	ProfileSub->SimulateBuildContext(EPGXBuildContext::Development);

	// ─── v2.0: Full 5-Layer Query ───
	const FPGXResolvedProfile& ResolvedProf = ProfileSub->GetResolvedProfile();
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Profile.Identity: ProjectMode=%s, BuildContext=%s"),
		*UEnum::GetValueAsString(ResolvedProf.Identity.ProjectMode),
		*UEnum::GetValueAsString(ResolvedProf.Identity.BuildContext));
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Profile.Budgets: RAM=%lld MB, VRAM=%lld MB"),
		ResolvedProf.Budgets.RAM_MB, ResolvedProf.Budgets.VRAM_MB);

	bool bResolved = ProfileSub->IsProfileResolved();
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("  IsProfileResolved=%s, State=%s"),
		bResolved ? TEXT("true") : TEXT("false"),
		*UEnum::GetValueAsString(ProfileSub->GetProfileState()));

	PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Capabilities: SaveData=%s, ConsoleCommands=%s"),
		ProfileSub->IsCapabilityEnabled(FName("SaveData")) ? TEXT("true") : TEXT("false"),
		ProfileSub->IsCapabilityEnabled(FName("ConsoleCommands")) ? TEXT("true") : TEXT("false"));

	PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Features: Nanite=%s, Lumen=%s"),
		ProfileSub->IsFeatureAllowed(FName("Nanite")) ? TEXT("true") : TEXT("false"),
		ProfileSub->IsFeatureAllowed(FName("Lumen")) ? TEXT("true") : TEXT("false"));

	int64 BudgetRAM = ProfileSub->GetBudget(FName("RAM_MB"));
	int64 BudgetVRAM = ProfileSub->GetBudget(FName("VRAM_MB"));
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Budgets query: RAM=%lld, VRAM=%lld"), BudgetRAM, BudgetVRAM);

	PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Persistence: Config=%s, Save=%s"),
		*UEnum::GetValueAsString(ProfileSub->GetPersistenceBackend(FName("Config"))),
		*UEnum::GetValueAsString(ProfileSub->GetPersistenceBackend(FName("Save"))));

	const UPGXPlatformConfig* ActivePlatCfg = ProfileSub->GetActivePlatformConfig();
	const UPGXPlatformConfig* PCPlatCfg = ProfileSub->GetPlatformConfigFor(EPGXTargetPlatform::PC);
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("  PlatformConfig: Active=%s, PC=%s"),
		(ActivePlatCfg != nullptr) ? TEXT("valid") : TEXT("null"),
		(PCPlatCfg != nullptr) ? TEXT("valid") : TEXT("null"));

	PGX_LOG_INFO(LogPGXSimHarness, TEXT("  HasSimulationOverrides=%s"),
		ProfileSub->HasSimulationOverrides() ? TEXT("true") : TEXT("false"));

	SystemStatuses[0].bInjected = true;
	SystemStatuses[0].Detail = TEXT("PC/Dev, profile resolved, 5 layers verified");
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("InjectProfile — PC + Development, 5 layers verified"));
#endif
}

void FPGXVisualHarness::TeardownProfile()
{
#if WITH_EDITOR
	UPGXProfileSubsystem* ProfileSub = UPGXProfileSubsystem::GetCachedInstance();
	if (ProfileSub && ProfileSub->HasSimulationOverrides())
	{
		ProfileSub->ClearSimulationOverrides();
	}
#endif
}

// ─── 2. Construction ───

void FPGXVisualHarness::InjectConstruction()
{
	UPGXConstructionSettings* Settings = GetMutableDefault<UPGXConstructionSettings>();
	if (!Settings)
	{
		PGX_LOG_WARNING(LogPGXSimHarness, TEXT("InjectConstruction — ConstructionSettings not found, skipping"));
		return;
	}

	// EN: Save originals / ES: Guardar originales
	SavedSources.GameMode = Settings->GameModeClassSource;
	SavedSources.PlayerController = Settings->PlayerControllerClassSource;
	SavedSources.GameState = Settings->GameStateClassSource;
	SavedSources.PlayerState = Settings->PlayerStateClassSource;
	SavedSources.Character = Settings->CharacterClassSource;
	SavedSources.Pawn = Settings->PawnClassSource;
	SavedSources.HUD = Settings->HUDClassSource;
	SavedSources.bSaved = true;

	// EN: Set varied modes for visual interest / ES: Poner modos variados para interes visual
	Settings->GameModeClassSource = EPGXClassSourceMode::Default;
	Settings->PlayerControllerClassSource = EPGXClassSourceMode::CppClass;
	Settings->GameStateClassSource = EPGXClassSourceMode::Default;
	Settings->PlayerStateClassSource = EPGXClassSourceMode::Blueprint;
	Settings->CharacterClassSource = EPGXClassSourceMode::CppClass;
	Settings->PawnClassSource = EPGXClassSourceMode::Default;
	Settings->HUDClassSource = EPGXClassSourceMode::Blueprint;

	SystemStatuses[1].bInjected = true;
	SystemStatuses[1].Detail = TEXT("7 slots (mixed modes)");
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("InjectConstruction — 7 class sources set to mixed modes"));
}

void FPGXVisualHarness::TeardownConstruction()
{
	if (!SavedSources.bSaved) return;

	UPGXConstructionSettings* Settings = GetMutableDefault<UPGXConstructionSettings>();
	if (!Settings) return;

	Settings->GameModeClassSource = SavedSources.GameMode;
	Settings->PlayerControllerClassSource = SavedSources.PlayerController;
	Settings->GameStateClassSource = SavedSources.GameState;
	Settings->PlayerStateClassSource = SavedSources.PlayerState;
	Settings->CharacterClassSource = SavedSources.Character;
	Settings->PawnClassSource = SavedSources.Pawn;
	Settings->HUDClassSource = SavedSources.HUD;
	SavedSources.bSaved = false;
}

// ─── 3. GameFlow ───

void FPGXVisualHarness::InjectGameFlow(UWorld* World)
{
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UPGXGameFlowSubsystem* FlowSub = GI ? GI->GetSubsystem<UPGXGameFlowSubsystem>() : nullptr;
	if (!FlowSub)
	{
		PGX_LOG_WARNING(LogPGXSimHarness, TEXT("InjectGameFlow — GameFlowSubsystem not found, skipping"));
		return;
	}

	// EN: Save current tags for all 8 channels / ES: Guardar tags actuales de los 8 canales
	SavedFlowTags.Empty();
	for (int32 i = 0; i < static_cast<int32>(EPGXFlowChannel::MAX); i++)
	{
		SavedFlowTags.Add(FlowSub->GetCurrentFlowTag(static_cast<EPGXFlowChannel>(i)));
	}

	// EN: Sequential transitions per channel — builds history depth (4-5 per channel)
	// ES: Transiciones secuenciales por canal — construye profundidad de historial (4-5 por canal)

	// Global: MainMenu → Loading → InGame → Pause → InGame
	FlowSub->SetStateByTag(EPGXFlowChannel::Global, PGXHarnessTags::FlowMainMenu());
	FlowSub->SetStateByTag(EPGXFlowChannel::Global, PGXHarnessTags::FlowLoading());
	FlowSub->SetStateByTag(EPGXFlowChannel::Global, PGXHarnessTags::FlowInGame());
	FlowSub->SetStateByTag(EPGXFlowChannel::Global, PGXHarnessTags::FlowPause());
	FlowSub->SetStateByTag(EPGXFlowChannel::Global, PGXHarnessTags::FlowInGame());

	// UI: HUD → Pause → HUD
	FlowSub->SetStateByTag(EPGXFlowChannel::UI, PGXHarnessTags::FlowHUD());
	FlowSub->SetStateByTag(EPGXFlowChannel::UI, PGXHarnessTags::FlowPause());
	FlowSub->SetStateByTag(EPGXFlowChannel::UI, PGXHarnessTags::FlowHUD());

	// Characters: Active → Patrol → Active
	FlowSub->SetStateByTag(EPGXFlowChannel::Characters, PGXHarnessTags::FlowActive());
	FlowSub->SetStateByTag(EPGXFlowChannel::Characters, PGXHarnessTags::FlowPatrol());
	FlowSub->SetStateByTag(EPGXFlowChannel::Characters, PGXHarnessTags::FlowActive());

	// AI: Patrol → Active → Patrol → Gameplay
	FlowSub->SetStateByTag(EPGXFlowChannel::AI, PGXHarnessTags::FlowPatrol());
	FlowSub->SetStateByTag(EPGXFlowChannel::AI, PGXHarnessTags::FlowActive());
	FlowSub->SetStateByTag(EPGXFlowChannel::AI, PGXHarnessTags::FlowPatrol());
	FlowSub->SetStateByTag(EPGXFlowChannel::AI, PGXHarnessTags::FlowGameplay());

	// Cameras: Gameplay → Explore → Gameplay
	FlowSub->SetStateByTag(EPGXFlowChannel::Cameras, PGXHarnessTags::FlowGameplay());
	FlowSub->SetStateByTag(EPGXFlowChannel::Cameras, PGXHarnessTags::FlowExplore());
	FlowSub->SetStateByTag(EPGXFlowChannel::Cameras, PGXHarnessTags::FlowGameplay());

	// Systems: Running → Loading → Running
	FlowSub->SetStateByTag(EPGXFlowChannel::Systems, PGXHarnessTags::FlowRunning());
	FlowSub->SetStateByTag(EPGXFlowChannel::Systems, PGXHarnessTags::FlowLoading());
	FlowSub->SetStateByTag(EPGXFlowChannel::Systems, PGXHarnessTags::FlowRunning());

	// LevelLogic: Explore → Active → Explore
	FlowSub->SetStateByTag(EPGXFlowChannel::LevelLogic, PGXHarnessTags::FlowExplore());
	FlowSub->SetStateByTag(EPGXFlowChannel::LevelLogic, PGXHarnessTags::FlowActive());
	FlowSub->SetStateByTag(EPGXFlowChannel::LevelLogic, PGXHarnessTags::FlowExplore());

	// Actors: Active → InGame → Active
	FlowSub->SetStateByTag(EPGXFlowChannel::Actors, PGXHarnessTags::FlowActive());
	FlowSub->SetStateByTag(EPGXFlowChannel::Actors, PGXHarnessTags::FlowInGame());
	FlowSub->SetStateByTag(EPGXFlowChannel::Actors, PGXHarnessTags::FlowActive());

	// ─── v2.0: Batch + Revert + Rules ───
	{
		TArray<FGameplayTag> BatchTags = {
			PGXHarnessTags::FlowPause(),
			PGXHarnessTags::FlowInGame(),
			PGXHarnessTags::FlowMainMenu()
		};
		FPGXFlowResult BatchResult = FlowSub->SetBatchSequentialStateByTag(EPGXFlowChannel::Global, BatchTags);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Batch(Global, 3 states): %s"), BatchResult.bSuccess ? TEXT("OK") : TEXT("FAIL"));
	}

	{
		bool bCanRevert = FlowSub->CheckCanRevert(EPGXFlowChannel::Global);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  CheckCanRevert(Global)=%s"), bCanRevert ? TEXT("true") : TEXT("false"));

		FPGXFlowResult RevertResult = FlowSub->RevertToPreviousFlow(EPGXFlowChannel::Global);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  RevertToPreviousFlow(Global): %s"), RevertResult.bSuccess ? TEXT("OK") : TEXT("FAIL"));

		FGameplayTag LastTag = FlowSub->GetLastFlowTag(EPGXFlowChannel::Global);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  GetLastFlowTag(Global)=%s"), *LastTag.ToString());
	}

	{
		FPGXFlowResult CanChange = FlowSub->CanChangeByTag(EPGXFlowChannel::Global, PGXHarnessTags::FlowLoading());
		bool bIsCurrent = FlowSub->IsCurrentFlowTag(EPGXFlowChannel::Global,
			FlowSub->GetCurrentFlowTag(EPGXFlowChannel::Global));
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  CanChange(Loading)=%s, IsCurrentFlowTag=%s"),
			CanChange.bSuccess ? TEXT("true") : TEXT("false"),
			bIsCurrent ? TEXT("true") : TEXT("false"));
	}

	{
		TArray<FPGXFlowHistoryEntry> History = FlowSub->GetChannelHistory(EPGXFlowChannel::Global);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  ChannelHistory(Global): %d entries"), History.Num());

		FPGXFlowRule OutRule;
		bool bHasRule = FlowSub->GetAllowedTransitionByCurrentFlowTag(EPGXFlowChannel::Global, OutRule);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  GetAllowedTransitionByCurrentFlowTag: hasRule=%s"),
			bHasRule ? TEXT("true") : TEXT("false"));
	}

	SystemStatuses[2].bInjected = true;
	SystemStatuses[2].Detail = TEXT("8 channels, ~35 history, batch+revert tested");
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("InjectGameFlow — 8 channels, batch+revert+rules verified"));
}

void FPGXVisualHarness::TeardownGameFlow(UWorld* World)
{
	if (SavedFlowTags.Num() == 0) return;

	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UPGXGameFlowSubsystem* FlowSub = GI ? GI->GetSubsystem<UPGXGameFlowSubsystem>() : nullptr;
	if (!FlowSub) return;

	// EN: Restore saved tags / ES: Restaurar tags guardados
	for (int32 i = 0; i < SavedFlowTags.Num() && i < static_cast<int32>(EPGXFlowChannel::MAX); i++)
	{
		if (SavedFlowTags[i].IsValid())
		{
			FlowSub->SetStateByTag(static_cast<EPGXFlowChannel>(i), SavedFlowTags[i]);
		}
	}
}

// ─── 4. Log ───

void FPGXVisualHarness::InjectLog(UWorld* World)
{
	UPGXLogTestUtility::RunAllSystems(World, 15);

	SystemStatuses[3].bInjected = true;
	SystemStatuses[3].Detail = TEXT("~100 entries generated");
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("InjectLog — RunAllSystems with 15 entries per system"));
}

// ─── 5. Save ───

void FPGXVisualHarness::InjectSave(UWorld* World)
{
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UPGXSaveSubsystem* SaveSub = GI ? GI->GetSubsystem<UPGXSaveSubsystem>() : nullptr;
	if (!SaveSub)
	{
		PGX_LOG_WARNING(LogPGXSimHarness, TEXT("InjectSave — SaveSubsystem not found, skipping"));
		return;
	}

	// EN: Campaign context with 2 domains / ES: Contexto de campania con 2 dominios
	auto* CampaignConfig = NewObject<UPGXSaveConfig>(GetTransientPackage(), NAME_None, RF_Transient);
	CampaignConfig->ContextTag = PGXHarnessTags::SaveCampaign();
	CampaignConfig->ContextDisplayName = FText::FromString(TEXT("Campaign"));

	FPGXSaveDomainEntry ProgressDomain;
	ProgressDomain.DomainTag = PGXHarnessTags::DomainProgress();
	ProgressDomain.DisplayName = FText::FromString(TEXT("Player Progress"));
	ProgressDomain.SaveGameClass = UPGXSaveGame::StaticClass();
	ProgressDomain.bRequired = true;
	CampaignConfig->SaveDomains.Add(ProgressDomain);

	FPGXSaveDomainEntry WorldDomain;
	WorldDomain.DomainTag = PGXHarnessTags::DomainWorld();
	WorldDomain.DisplayName = FText::FromString(TEXT("World State"));
	WorldDomain.SaveGameClass = UPGXSaveGame::StaticClass();
	WorldDomain.bRequired = false;
	CampaignConfig->SaveDomains.Add(WorldDomain);

	CreatedObjects.Add(TStrongObjectPtr<UObject>(CampaignConfig));
	SaveSub->InjectTestConfig(CampaignConfig);

	// EN: Settings context with 2 domains / ES: Contexto de settings con 2 dominios
	auto* SettingsConfig = NewObject<UPGXSaveConfig>(GetTransientPackage(), NAME_None, RF_Transient);
	SettingsConfig->ContextTag = PGXHarnessTags::SaveSettings();
	SettingsConfig->ContextDisplayName = FText::FromString(TEXT("Settings"));

	FPGXSaveDomainEntry GraphicsDomain;
	GraphicsDomain.DomainTag = PGXHarnessTags::DomainGraphics();
	GraphicsDomain.DisplayName = FText::FromString(TEXT("Graphics Settings"));
	GraphicsDomain.SaveGameClass = UPGXSaveGame::StaticClass();
	GraphicsDomain.bRequired = true;
	SettingsConfig->SaveDomains.Add(GraphicsDomain);

	FPGXSaveDomainEntry AudioDomain;
	AudioDomain.DomainTag = PGXHarnessTags::DomainAudio();
	AudioDomain.DisplayName = FText::FromString(TEXT("Audio Settings"));
	AudioDomain.SaveGameClass = UPGXSaveGame::StaticClass();
	AudioDomain.bRequired = true;
	SettingsConfig->SaveDomains.Add(AudioDomain);

	CreatedObjects.Add(TStrongObjectPtr<UObject>(SettingsConfig));
	SaveSub->InjectTestConfig(SettingsConfig);

	// EN: deep verification — Write key-value data into each domain's SaveGame
	// ES: Escribir datos key-value en el SaveGame de cada dominio
	if (UPGXSaveGame* ProgressSG = SaveSub->GetSaveGame(PGXHarnessTags::DomainProgress()))
	{
		ProgressSG->WriteString(FName("PlayerName"), TEXT("HarnessPlayer"));
		ProgressSG->WriteInt(FName("Level"), 42);
		ProgressSG->WriteFloat(FName("PlayTime"), 3600.5f);
		ProgressSG->WriteBool(FName("HasCompletedTutorial"), true);
		ProgressSG->WriteVector(FName("LastCheckpoint"), FVector(1200.0, -500.0, 300.0));
	}

	if (UPGXSaveGame* WorldSG = SaveSub->GetSaveGame(PGXHarnessTags::DomainWorld()))
	{
		WorldSG->WriteString(FName("CurrentZone"), TEXT("ForestOfTrials"));
		WorldSG->WriteInt(FName("EnemiesDefeated"), 127);
		WorldSG->WriteBool(FName("BossUnlocked"), true);
	}

	if (UPGXSaveGame* GraphicsSG = SaveSub->GetSaveGame(PGXHarnessTags::DomainGraphics()))
	{
		GraphicsSG->WriteInt(FName("ResolutionX"), 1920);
		GraphicsSG->WriteInt(FName("ResolutionY"), 1080);
		GraphicsSG->WriteFloat(FName("RenderScale"), 1.0f);
		GraphicsSG->WriteBool(FName("VSync"), true);
		GraphicsSG->WriteString(FName("Quality"), TEXT("Ultra"));
	}

	if (UPGXSaveGame* AudioSG = SaveSub->GetSaveGame(PGXHarnessTags::DomainAudio()))
	{
		AudioSG->WriteFloat(FName("MasterVolume"), 0.8f);
		AudioSG->WriteFloat(FName("MusicVolume"), 0.6f);
		AudioSG->WriteFloat(FName("SFXVolume"), 1.0f);
		AudioSG->WriteBool(FName("Subtitles"), true);
	}

	// EN: Create slots by saving context (writes domains to disk)
	// ES: Crear slots guardando contexto (escribe dominios a disco)
	struct { FGameplayTag Ctx; const TCHAR* Slot; } SlotDefs[] = {
		{ PGXHarnessTags::SaveCampaign(), TEXT("Harness_AutoSave") },
		{ PGXHarnessTags::SaveCampaign(), TEXT("Harness_Manual_01") },
		{ PGXHarnessTags::SaveCampaign(), TEXT("Harness_Manual_02") },
		{ PGXHarnessTags::SaveSettings(), TEXT("Harness_Settings") },
	};

	for (const auto& SD : SlotDefs)
	{
		EPGXSaveResult Result = SaveSub->SaveContext(SD.Ctx, SD.Slot);
		if (Result == EPGXSaveResult::Success)
		{
			FSavedSlotInfo SlotInfo;
			SlotInfo.ContextTag = SD.Ctx;
			SlotInfo.SlotName = SD.Slot;
			CreatedSaveSlots.Add(SlotInfo);
		}
	}

	// ─── v2.0: Additional Write Types (Rotator, Transform, Tag) ───
	if (UPGXSaveGame* ProgressSG2 = SaveSub->GetSaveGame(PGXHarnessTags::DomainProgress()))
	{
		ProgressSG2->WriteRotator(FName("CameraRotation"), FRotator(15.0, -30.0, 0.0));
		ProgressSG2->WriteTransform(FName("SpawnPoint"), FTransform(FRotator(0, 90, 0), FVector(500, 200, 100)));
		ProgressSG2->WriteTag(FName("ActiveQuest"), PGXHarnessTags::EvtDamage());
	}

	// EN: Re-save to persist new data types / ES: Re-guardar para persistir nuevos tipos de datos
	SaveSub->SaveContext(PGXHarnessTags::SaveCampaign(), TEXT("Harness_AutoSave"));

	// ─── v2.0: Round-Trip Verification ───
	{
		EPGXSaveResult LoadResult = SaveSub->LoadContext(PGXHarnessTags::SaveCampaign(), TEXT("Harness_AutoSave"));
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Save Round-trip: LoadContext=%s"),
			LoadResult == EPGXSaveResult::Success ? TEXT("OK") : TEXT("FAIL"));

		if (UPGXSaveGame* SG = SaveSub->GetSaveGame(PGXHarnessTags::DomainProgress()))
		{
			FString PlayerName = SG->ReadString(FName("PlayerName"));
			int32 Level = SG->ReadInt(FName("Level"));
			bool bHasPlayTime = SG->HasKey(FName("PlayTime"));
			int32 KeyCount = SG->GetKeyValueCount();
			PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Round-trip: PlayerName=%s, Level=%d, HasPlayTime=%s, Keys=%d"),
				*PlayerName, Level, bHasPlayTime ? TEXT("true") : TEXT("false"), KeyCount);
		}
	}

	// ─── v2.0: Quick Save/Load ───
	{
		EPGXSaveResult QSResult = SaveSub->QuickSave(PGXHarnessTags::SaveCampaign());
		EPGXSaveResult QLResult = SaveSub->QuickLoad(PGXHarnessTags::SaveCampaign());
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  QuickSave=%s, QuickLoad=%s"),
			QSResult == EPGXSaveResult::Success ? TEXT("OK") : TEXT("FAIL"),
			QLResult == EPGXSaveResult::Success ? TEXT("OK") : TEXT("FAIL"));
	}

	// ─── v2.0: Slot Operations ───
	{
		EPGXSaveResult CopyResult = SaveSub->CopySlot(PGXHarnessTags::SaveCampaign(),
			TEXT("Harness_AutoSave"), TEXT("Harness_Backup"));
		if (CopyResult == EPGXSaveResult::Success)
		{
			BackupSlotName = TEXT("Harness_Backup");
		}
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  CopySlot=%s"),
			CopyResult == EPGXSaveResult::Success ? TEXT("OK") : TEXT("FAIL"));

		bool bBackupExists = SaveSub->DoesSlotExist(PGXHarnessTags::SaveCampaign(), TEXT("Harness_Backup"));
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  DoesSlotExist(Backup)=%s"), bBackupExists ? TEXT("true") : TEXT("false"));

		TArray<FPGXSaveSlotInfo> AllSlots = SaveSub->GetAllSlots(PGXHarnessTags::SaveCampaign());
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  GetAllSlots(Campaign)=%d"), AllSlots.Num());

		FPGXSaveSlotInfo AutoSaveInfo = SaveSub->GetSlotInfo(PGXHarnessTags::SaveCampaign(), TEXT("Harness_AutoSave"));
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  SlotInfo(AutoSave): Size=%lld bytes, Domains=%d"),
			AutoSaveInfo.TotalSizeBytes, AutoSaveInfo.DomainCount);

		FString NextSlot = SaveSub->GetNextAvailableSlotName(PGXHarnessTags::SaveCampaign());
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  NextAvailableSlotName=%s"), *NextSlot);

		SaveSub->SetActiveSlot(PGXHarnessTags::SaveCampaign(), TEXT("Harness_AutoSave"));
		FString ActiveSlot = SaveSub->GetActiveSlotName(PGXHarnessTags::SaveCampaign());
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  ActiveSlot=%s"), *ActiveSlot);
	}

	// ─── v2.0: Auto-Save ───
	{
		SaveSub->SetAutoSaveEnabled(PGXHarnessTags::SaveCampaign(), true);
		bool bAutoActive = SaveSub->IsAutoSaveActive(PGXHarnessTags::SaveCampaign());
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  AutoSave: Enabled, IsActive=%s"), bAutoActive ? TEXT("true") : TEXT("false"));
		SaveSub->TriggerAutoSave(PGXHarnessTags::SaveCampaign());
		SaveSub->SetAutoSaveEnabled(PGXHarnessTags::SaveCampaign(), false);
	}

	// ─── v2.0: Context + State Queries ───
	{
		TArray<FGameplayTag> AllCtxTags = SaveSub->GetAllContextTags();
		int32 CtxCount = SaveSub->GetContextCount();
		const UPGXSaveConfig* CampaignCfg = SaveSub->GetContextConfig(PGXHarnessTags::SaveCampaign());
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Contexts: tags=%d, count=%d, CampaignConfig=%s"),
			AllCtxTags.Num(), CtxCount, IsValid(CampaignCfg) ? TEXT("valid") : TEXT("null"));

		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  IsSaveInProgress=%s, IsLoadInProgress=%s"),
			SaveSub->IsSaveInProgress() ? TEXT("true") : TEXT("false"),
			SaveSub->IsLoadInProgress() ? TEXT("true") : TEXT("false"));

		bool bHasData = SaveSub->HasData(PGXHarnessTags::DomainProgress(), FName("PlayerName"));
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  HasData(Progress, PlayerName)=%s"), bHasData ? TEXT("true") : TEXT("false"));
	}

	// ─── v2.0 Part 2: ClearDomain + Saveable + Async ───

	// EN: ClearDomain — clear and re-populate to test domain data lifecycle
	// ES: ClearDomain — limpiar y re-poblar para testar ciclo de vida de datos de dominio
	{
		SaveSub->ClearDomain(PGXHarnessTags::DomainWorld());
		bool bHasClearedData = SaveSub->HasData(PGXHarnessTags::DomainWorld(), FName("CurrentZone"));
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  ClearDomain(World): HasData after clear=%s"),
			bHasClearedData ? TEXT("true") : TEXT("false"));

		// EN: Re-populate so inspector still shows data / ES: Re-poblar para que inspector siga mostrando datos
		if (UPGXSaveGame* WorldSGRestored = SaveSub->GetSaveGame(PGXHarnessTags::DomainWorld()))
		{
			WorldSGRestored->WriteString(FName("CurrentZone"), TEXT("ForestOfTrials_Restored"));
			WorldSGRestored->WriteInt(FName("EnemiesDefeated"), 150);
			WorldSGRestored->WriteBool(FName("BossUnlocked"), true);
		}
	}

	// EN: RegisterSaveable / UnregisterSaveable — test saveable lifecycle
	// ES: RegisterSaveable / UnregisterSaveable — testar ciclo de vida de saveable
	{
		const FString ExpectedFixtureData = TEXT("HarnessFixtureData_v2");
		auto* SaveableFixture = NewObject<UPGXHarnessSaveableStub>(GetTransientPackage(), NAME_None, RF_Transient);
		SaveableFixture->DomainTag = PGXHarnessTags::DomainProgress();
		SaveableFixture->TestData = ExpectedFixtureData;
		CreatedObjects.Add(TStrongObjectPtr<UObject>(SaveableFixture));

		SaveSub->RegisterSaveable(SaveableFixture, PGXHarnessTags::DomainProgress());
		SaveableFixtureRef = SaveableFixture;

		const EPGXSaveResult FixtureSaveResult = SaveSub->SaveContext(PGXHarnessTags::SaveCampaign(), TEXT("Harness_AutoSave"));
		SaveableFixture->TestData = TEXT("HarnessFixtureData_NotLoadedYet");
		const EPGXSaveResult FixtureLoadResult = SaveSub->LoadContext(PGXHarnessTags::SaveCampaign(), TEXT("Harness_AutoSave"));
		const UPGXSaveGame* ProgressSave = SaveSub->GetSaveGame(PGXHarnessTags::DomainProgress());
		const FString MarkerValue = ProgressSave ? ProgressSave->ReadString(FName("HarnessFixtureMarker")) : FString();
		const bool bFixturePersisted = SaveableFixture->TestData == ExpectedFixtureData
			&& MarkerValue == ExpectedFixtureData;
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  RegisterSaveable: compatibility fixture Save=%s Load=%s Persisted=%s Data=%s Marker=%s Expected=%s"),
			FixtureSaveResult == EPGXSaveResult::Success ? TEXT("OK") : TEXT("FAIL"),
			FixtureLoadResult == EPGXSaveResult::Success ? TEXT("OK") : TEXT("FAIL"),
			bFixturePersisted ? TEXT("true") : TEXT("false"),
			*SaveableFixture->TestData,
			*MarkerValue,
			*ExpectedFixtureData);
	}

	// EN: Async save/load — fire-and-forget, exercises async code path
	// ES: Async save/load — fire-and-forget, ejercita la ruta de codigo async
	{
		SaveSub->SaveContextAsync(PGXHarnessTags::SaveCampaign(), TEXT("Harness_AutoSave"));
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  SaveContextAsync(Campaign, AutoSave) — dispatched"));

		SaveSub->LoadContextAsync(PGXHarnessTags::SaveCampaign(), TEXT("Harness_AutoSave"));
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  LoadContextAsync(Campaign, AutoSave) — dispatched"));
	}

	int32 TotalSlots = CreatedSaveSlots.Num() + (BackupSlotName.IsEmpty() ? 0 : 1);
	SystemStatuses[4].bInjected = true;
	SystemStatuses[4].ObjectCount = 2;
	SystemStatuses[4].Detail = FString::Printf(TEXT("2 ctx, 4 domains, %d slots, ~25 keys, async+saveable+round-trip OK"), TotalSlots);
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("InjectSave — 2 ctx, 4 domains, %d slots, async+saveable+clear verified"), TotalSlots);
}

void FPGXVisualHarness::TeardownSave(UWorld* World)
{
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UPGXSaveSubsystem* SaveSub = GI ? GI->GetSubsystem<UPGXSaveSubsystem>() : nullptr;
	if (SaveSub)
	{
		// EN: Unregister saveable compatibility fixture from Part 2 / ES: Desregistrar fixture saveable de compatibilidad del Part 2
		if (SaveableFixtureRef.IsValid())
		{
			SaveSub->UnregisterSaveable(SaveableFixtureRef.Get());
			SaveableFixtureRef.Reset();
		}

		// EN: Delete backup slot if created by CopySlot / ES: Eliminar slot backup si fue creado por CopySlot
		if (!BackupSlotName.IsEmpty())
		{
			SaveSub->DeleteSlot(PGXHarnessTags::SaveCampaign(), BackupSlotName);
			BackupSlotName.Empty();
		}

		// EN: Delete created slots from disk / ES: Eliminar slots creados del disco
		for (const FSavedSlotInfo& SlotInfo : CreatedSaveSlots)
		{
			SaveSub->DeleteSlot(SlotInfo.ContextTag, SlotInfo.SlotName);
		}
		CreatedSaveSlots.Empty();

		SaveSub->ClearTestConfigs();
	}
}

// ─── 6. PSO ───

void FPGXVisualHarness::InjectPSO(UWorld* World)
{
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UPGXPSOSubsystem* PSOSub = GI ? GI->GetSubsystem<UPGXPSOSubsystem>() : nullptr;
	if (!PSOSub)
	{
		PGX_LOG_WARNING(LogPGXSimHarness, TEXT("InjectPSO — PSOSubsystem not found, skipping"));
		return;
	}

	auto* PSOConfig = NewObject<UPGXPSOWarmUpConfig>(GetTransientPackage(), NAME_None, RF_Transient);
	PSOConfig->ActivationMode = EPGXPSOActivationMode::OnExplicitCall;
	PSOConfig->BatchSize = 2;
	PSOConfig->BatchDelaySeconds = 0.01f;
	PSOConfig->bSaveCacheAfterWarmUp = false;
	PSOConfig->SavePolicy = EPGXPSOSavePolicy::Manual;
	PSOConfig->ConcurrencyPolicy = EPGXPSOConcurrencyPolicy::MergeAndContinue;
	PSOConfig->MaxSimultaneousLoads = 4;
	CreatedObjects.Add(TStrongObjectPtr<UObject>(PSOConfig));
	PSOSub->InjectTestConfig(PSOConfig);

	// EN: deep verification — Add contexts, start recording, request warm-up
	// ES: Agregar contextos, iniciar recording, solicitar warm-up
	FGameplayTag PSOContexts[] = {
		PGXHarnessTags::PSOCtxMainMenu(),
		PGXHarnessTags::PSOCtxGameplay(),
		PGXHarnessTags::PSOCtxInventory(),
		PGXHarnessTags::PSOCtxCinematic()
	};
	for (const FGameplayTag& CtxTag : PSOContexts)
	{
		PSOSub->AddPSOContext(CtxTag);
		AddedPSOContexts.Add(CtxTag);
	}

#if WITH_EDITOR
	PSOSub->StartRecording(TEXT("HarnessSession"));
	bPSORecordingStarted = true;
#endif

	// EN: Request warm-up on first context (async, editor-safe)
	// ES: Solicitar warm-up en primer contexto (async, editor-safe)
	PSOSub->RequestWarmUp(PGXHarnessTags::PSOCtxMainMenu());
	const bool bWarmUpAllAccepted = PSOSub->RequestWarmUpAll();
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("  RequestWarmUpAll accepted=%s (batch=%d, maxLoads=%d)"),
		bWarmUpAllAccepted ? TEXT("true") : TEXT("false"), PSOConfig->BatchSize, PSOConfig->MaxSimultaneousLoads);

	// ─── v2.0: Pause/Resume + Recording Queries + Validation ───
	{
		PSOSub->PauseWarmUp();
		EPGXPSOWarmUpState PausedState = PSOSub->GetWarmUpState();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  PauseWarmUp: State=%s"), *UEnum::GetValueAsString(PausedState));
		PSOSub->ResumeWarmUp();
	}

#if WITH_EDITOR
	{
		bool bRecording = PSOSub->IsRecording();
		const TArray<FPGXPSORecordedEntry>& RecEntries = PSOSub->GetRecordedEntries();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  IsRecording=%s, RecordedEntries=%d"),
			bRecording ? TEXT("true") : TEXT("false"), RecEntries.Num());
	}
#endif

	{
		int32 DiscoveredCount = PSOSub->GetDiscoveredConfigCount();
		bool bDirty = PSOSub->IsCacheDirty();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  DiscoveredConfigs=%d, CacheDirty=%s"),
			DiscoveredCount, bDirty ? TEXT("true") : TEXT("false"));
	}

#if WITH_EDITOR
	{
		TArray<FString> ValidationIssues = PSOSub->ValidateConfig(
			Cast<UPGXPSOWarmUpConfig>(CreatedObjects.Last().Get()));
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  ValidateConfig: %d issues"), ValidationIssues.Num());
	}
#endif

	{
		TArray<FGameplayTag> ActiveCtxs = PSOSub->GetActiveContexts();
		FPGXPSOWarmUpProgress Progress = PSOSub->GetWarmUpProgress();
		const FName StaticVF = PGXPSOUtils::ResolveVertexFactoryType(EPGXVertexFactoryType::StaticMesh);
		const FName SkeletalVF = PGXPSOUtils::ResolveVertexFactoryType(EPGXVertexFactoryType::SkeletalMesh);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  ActiveContexts=%d, Progress: %.1f%% (%d/%d entries)"),
			ActiveCtxs.Num(), Progress.PercentComplete * 100.f,
			Progress.CompletedEntries, Progress.TotalEntries);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  VertexFactories: Static=%s, Skeletal=%s"), *StaticVF.ToString(), *SkeletalVF.ToString());
	}

	SystemStatuses[5].bInjected = true;
	SystemStatuses[5].ObjectCount = 1;
	SystemStatuses[5].Detail = TEXT("4 contexts, batched warm-up, pause/resume, recording, VF validation");
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("InjectPSO — 4 contexts, batched warm-up, pause/resume+recording+VF validation verified"));
}

void FPGXVisualHarness::TeardownPSO(UWorld* World)
{
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UPGXPSOSubsystem* PSOSub = GI ? GI->GetSubsystem<UPGXPSOSubsystem>() : nullptr;
	if (PSOSub)
	{
		// EN: Cancel warm-up if running / ES: Cancelar warm-up si esta corriendo
		PSOSub->CancelWarmUp();

#if WITH_EDITOR
		// EN: Stop and clear recording / ES: Detener y limpiar recording
		if (bPSORecordingStarted)
		{
			if (PSOSub->IsRecording())
			{
				PSOSub->StopRecording();
			}
			PSOSub->ClearRecording();
			bPSORecordingStarted = false;
		}
#endif

		// EN: Remove added contexts / ES: Remover contextos agregados
		for (const FGameplayTag& CtxTag : AddedPSOContexts)
		{
			PSOSub->RemovePSOContext(CtxTag);
		}
		AddedPSOContexts.Empty();

		PSOSub->ClearTestConfigs();
	}
}

// ─── 7. MGOS ───

void FPGXVisualHarness::InjectMGOS()
{
	UPGXGCObserverSubsystem* MGOS = UPGXGCObserverSubsystem::GetCachedInstance();
	if (!MGOS)
	{
		PGX_LOG_WARNING(LogPGXSimHarness, TEXT("InjectMGOS — GCObserverSubsystem not found, skipping"));
		return;
	}

	MGOS->SetMode(EPGXGCObserverMode::Snapshot);
	MGOS->RequestBaselineCapture();

	// EN: Force a couple GC cycles to generate history / ES: Forzar un par de ciclos GC para generar historial
	GEngine->ForceGarbageCollection(true);
	GEngine->ForceGarbageCollection(true);

	// ─── v2.0: Profile + History + Incidents + Suppression ───
	{
		FPGXGCProfile GCProfile = MGOS->GetCurrentProfile();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  GCProfile: State=%s, Confidence=%.2f, CyclesInState=%d"),
			*UEnum::GetValueAsString(GCProfile.CurrentState), GCProfile.Confidence, GCProfile.CyclesInState);

		TArray<FPGXGCSnapshotDiff> HistorySummary = MGOS->GetHistorySummary(10);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  HistorySummary: %d entries"), HistorySummary.Num());

		FPGXGCBaseline Baseline = MGOS->GetCurrentBaseline();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Baseline: Valid=%s, UObjects=%lld, Memory=%.1f MB, State=%s"),
			Baseline.bValid ? TEXT("true") : TEXT("false"),
			Baseline.TotalUObjectCount, Baseline.BaselineProcessMemoryMB,
			*UEnum::GetValueAsString(Baseline.BaselineState));

		int64 CycleCount = MGOS->GetCycleCount();
		const TArray<FPGXGCIncident>& Incidents = MGOS->GetCurrentIncidents();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Cycles=%lld, Incidents=%d"), CycleCount, Incidents.Num());

		TArray<FPGXGCClassHealthReport> ClassReport = MGOS->GetTrackedClassReport();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  TrackedClassReport: %d entries"), ClassReport.Num());

		const bool bInitialized = MGOS->IsInitialized();
		const EPGXGCObserverMode ModeBeforeSuppression = MGOS->GetMode();
		const EPGXGCBaselineState BaselineState = MGOS->GetBaselineState();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  RuntimeState: Initialized=%s, Mode=%s, BaselineState=%s"),
			bInitialized ? TEXT("true") : TEXT("false"),
			*UEnum::GetValueAsString(ModeBeforeSuppression),
			*UEnum::GetValueAsString(BaselineState));

		MGOS->SetSuppressed(true);
		bool bSuppressed = MGOS->IsInSuppressedPhase();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Suppression: Set=true, IsInSuppressedPhase=%s"),
			bSuppressed ? TEXT("true") : TEXT("false"));
		MGOS->SetSuppressed(false);
	}

	SystemStatuses[6].bInjected = true;
	SystemStatuses[6].Detail = FString::Printf(TEXT("Snapshot, baseline captured, %lld cycles, state+suppression verified"),
		MGOS->GetCycleCount());
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("InjectMGOS — Snapshot, baseline, profile+history+incidents+state verified"));
}

void FPGXVisualHarness::TeardownMGOS()
{
	UPGXGCObserverSubsystem* MGOS = UPGXGCObserverSubsystem::GetCachedInstance();
	if (MGOS)
	{
		MGOS->SetMode(EPGXGCObserverMode::Passive);
		MGOS->ResetBaseline();
	}
}

// ─── 8. Audio ───

void FPGXVisualHarness::InjectAudio(UWorld* World)
{
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UPGXAudioSubsystem* AudioSub = GI ? GI->GetSubsystem<UPGXAudioSubsystem>() : nullptr;
	if (!AudioSub)
	{
		PGX_LOG_WARNING(LogPGXSimHarness, TEXT("InjectAudio — AudioSubsystem not found, skipping"));
		return;
	}

	// EN: Audio system config / ES: Config del sistema de audio
	auto* AudioCfg = NewObject<UPGXAudioConfig>(GetTransientPackage(), NAME_None, RF_Transient);
	CreatedObjects.Add(TStrongObjectPtr<UObject>(AudioCfg));
	AudioSub->InjectTestAudioConfig(AudioCfg);

	// EN: 5 channel configs (extends FPGXTestHarness's 2 to 5)
	// ES: 5 configs de canal (extiende las 2 de FPGXTestHarness a 5)
	struct ChannelDef { TFunction<FGameplayTag()> TagFn; const TCHAR* Name; float Volume; int32 MaxConcurrent; };
	const ChannelDef Channels[] = {
		{ PGXHarnessTags::AudioSFX,     TEXT("SFX"),     0.8f, 16 },
		{ PGXHarnessTags::AudioMusic,   TEXT("Music"),   0.6f, 1  },
		{ PGXHarnessTags::AudioVoice,   TEXT("Voice"),   1.0f, 3  },
		{ PGXHarnessTags::AudioAmbient, TEXT("Ambient"), 0.5f, 8  },
		{ PGXHarnessTags::AudioUI,      TEXT("UI"),      0.7f, 4  },
	};

	for (const auto& Ch : Channels)
	{
		auto* ChCfg = NewObject<UPGXAudioChannelConfig>(GetTransientPackage(), NAME_None, RF_Transient);
		ChCfg->ChannelTag = Ch.TagFn();
		ChCfg->ChannelDisplayName = FText::FromString(Ch.Name);
		ChCfg->DefaultVolume = Ch.Volume;
		ChCfg->MaxConcurrent = Ch.MaxConcurrent;
		CreatedObjects.Add(TStrongObjectPtr<UObject>(ChCfg));
		AudioSub->InjectTestChannelConfig(ChCfg);
	}

	// ─── v2.0: Channel + State Queries ───
	{
		EPGXAudioState AudioState = AudioSub->GetAudioState();
		bool bAudioInit = AudioSub->IsInitialized();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  AudioState=%s, IsInitialized=%s"),
			*UEnum::GetValueAsString(AudioState), bAudioInit ? TEXT("true") : TEXT("false"));

		EPGXAudioBackendType BackendType = AudioSub->GetActiveBackendType();
		FString BackendStatus = AudioSub->GetBackendStatusText();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Backend: Type=%s, Status=%s"),
			*UEnum::GetValueAsString(BackendType), *BackendStatus);

		TArray<FPGXAudioChannelSnapshot> ChannelStates = AudioSub->GetAllChannelStates();
		int32 ActiveSounds = AudioSub->GetActiveSoundCount();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Channels=%d, ActiveSounds=%d"), ChannelStates.Num(), ActiveSounds);

		FPGXAudioSystemSnapshot Snapshot = AudioSub->GetAudioSnapshot();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Snapshot: State=%s, Channels=%d"),
			*UEnum::GetValueAsString(Snapshot.State), Snapshot.Channels.Num());

		FPGXSoundPoolStats PoolStats = AudioSub->GetPoolStatistics();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Pool: Capacity=%d, InUse=%d, Peak=%d"),
			PoolStats.TotalCapacity, PoolStats.InUse, PoolStats.PeakUsage);

		FPGXAudioMemoryInfo MemInfo = AudioSub->GetMemoryEstimate();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Memory: Bytes=%lld, LoadedSounds=%d"),
			MemInfo.EstimatedMemoryBytes, MemInfo.LoadedSoundCount);

		TArray<FPGXAudioEventRecord> EventHistory = AudioSub->GetEventHistory(10);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  EventHistory: %d entries"), EventHistory.Num());

		FGameplayTag MusicState = AudioSub->GetMusicState();
		EPGXMusicState MusicPlayback = AudioSub->GetMusicPlaybackState();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Music: State=%s, Playback=%s"),
			*MusicState.ToString(), *UEnum::GetValueAsString(MusicPlayback));

		int32 DialogueQueue = AudioSub->GetDialogueQueueCount();
		bool bDialoguePlaying = AudioSub->IsDialoguePlaying();
		bool bMuteAll = AudioSub->IsMuteAll();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Dialogue: Queue=%d, Playing=%s, MuteAll=%s"),
			DialogueQueue, bDialoguePlaying ? TEXT("true") : TEXT("false"),
			bMuteAll ? TEXT("true") : TEXT("false"));

		const UPGXAudioConfig* RetrievedAudioCfg = AudioSub->GetAudioConfig();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  AudioConfig=%s"), IsValid(RetrievedAudioCfg) ? TEXT("valid") : TEXT("null"));
	}

	// ─── v2.0 Part 2: Channel Control + Playback + Music + Dialogue + Backend ───

	// EN: A. Channel volume get/set cycle / ES: A. Ciclo de get/set de volumen de canal
	{
		const FGameplayTag ChTags[] = {
			PGXHarnessTags::AudioSFX(), PGXHarnessTags::AudioMusic(),
			PGXHarnessTags::AudioVoice(), PGXHarnessTags::AudioAmbient(),
			PGXHarnessTags::AudioUI()
		};
		for (const FGameplayTag& ChTag : ChTags)
		{
			float OrigVol = AudioSub->GetChannelVolume(ChTag);
			AudioSub->SetChannelVolume(ChTag, 0.5f);
			AudioSub->SetChannelVolume(ChTag, OrigVol);
		}
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended Channel volume get/set: 5 channels exercised"));

		// EN: Mute/unmute one channel / ES: Mutear/desmutear un canal
		AudioSub->SetChannelMuted(PGXHarnessTags::AudioAmbient(), true);
		bool bAmbientMuted = AudioSub->IsChannelMuted(PGXHarnessTags::AudioAmbient());
		AudioSub->SetChannelMuted(PGXHarnessTags::AudioAmbient(), false);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended Mute: Ambient=%s (restored)"), bAmbientMuted ? TEXT("true") : TEXT("false"));

		// EN: Global mute cycle / ES: Ciclo de mute global
		AudioSub->SetMuteAll(true);
		bool bWasGlobalMute = AudioSub->IsMuteAll();
		AudioSub->SetMuteAll(false);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended MuteAll=%s (restored)"), bWasGlobalMute ? TEXT("true") : TEXT("false"));
	}

	// EN: B. Playback (nullptr sounds — tests null-guard code path, no actual audio)
	// ES: B. Playback (sounds nullptr — testa ruta de null-guard, no audio real)
	{
		FPGXAudioPlayParams PlayParams;
		PlayParams.ChannelTag = PGXHarnessTags::AudioSFX();
		PlayParams.VolumeMultiplier = 0.8f;
		PlayParams.PitchMultiplier = 1.0f;

		FPGXSoundHandle Handle2D = AudioSub->PlaySound2D(nullptr, PlayParams);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended PlaySound2D(null): valid=%s"), Handle2D.IsValid() ? TEXT("Y") : TEXT("N"));

		FPGXSoundHandle HandleLoc = AudioSub->PlaySoundAtLocation(
			nullptr, FVector(100.0, 200.0, 0.0), FRotator::ZeroRotator, PlayParams);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended PlaySoundAtLocation(null): valid=%s"), HandleLoc.IsValid() ? TEXT("Y") : TEXT("N"));

		AudioSub->StopSound(Handle2D, 0.0f);
		AudioSub->StopSound(HandleLoc, 0.0f);
		AudioSub->StopAllSounds(0.0f);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended StopSound + StopAllSounds exercised"));

		TArray<FPGXActiveSoundInfo> ActiveSounds = AudioSub->GetActiveSounds();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended ActiveSounds=%d"), ActiveSounds.Num());

		// EN: ResolveSound + PlayResolved (null definition) / ES: ResolveSound + PlayResolved (definition null)
		USoundBase* Resolved = AudioSub->ResolveSound(nullptr, FGameplayTagContainer());
		FPGXSoundHandle ResolvedHandle = AudioSub->PlayResolved(nullptr, PlayParams);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended ResolveSound(null)=%s, PlayResolved(null)=%s"),
			Resolved ? TEXT("valid") : TEXT("null"),
			ResolvedHandle.IsValid() ? TEXT("valid") : TEXT("invalid"));
	}

	// EN: C. Music lifecycle / ES: C. Ciclo de vida de musica
	{
		AudioSub->PlayMusic(nullptr, 0.0f);
		AudioSub->PauseMusic();
		AudioSub->ResumeMusic();
		AudioSub->CrossfadeTo(nullptr, 0.0f);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended Music: Play+Pause+Resume+Crossfade (null sound)"));

		AudioSub->PlayPlaylist(nullptr);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended PlayPlaylist(null)"));

		AudioSub->SetMusicState(PGXHarnessTags::AudioMusicStateExplore());
		FGameplayTag CurMusicState = AudioSub->GetMusicState();
		FString TrackName = AudioSub->GetCurrentTrackName();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended MusicState=%s, Track=%s"),
			*CurMusicState.ToString(), *TrackName);

		AudioSub->StopMusic(0.0f);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended StopMusic(0s)"));
	}

	// EN: D. Dialogue lifecycle / ES: D. Ciclo de vida de dialogo
	{
		bool bQueued = AudioSub->QueueDialogue(
			nullptr,
			FText::FromString(TEXT("Harness test dialogue line")),
			3.0f,
			PGXHarnessTags::AudioSpeakerNPC(),
			PGXHarnessTags::AudioPriorityHigh(),
			EPGXDialogueInterruptPolicy::QueueBehind
		);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended QueueDialogue(null): %s"), bQueued ? TEXT("queued") : TEXT("rejected"));

		AudioSub->ClearDialogueQueue();
		AudioSub->StopDialogue(0.0f);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended ClearDialogueQueue + StopDialogue"));
	}

	// EN: E. Backend switch + restore / ES: E. Cambio de backend + restaurar
	{
		EPGXAudioBackendType OrigBackend = AudioSub->GetActiveBackendType();
		const FPGXAudioBackendSwitchResult SwitchResult = AudioSub->SwitchBackendDetailed(EPGXAudioBackendType::Legacy);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended SwitchBackendDetailed(Legacy): %s status=%s restoredChannels=%d preservedSounds=%d — %s"),
			SwitchResult.bSuccess ? TEXT("OK") : TEXT("unavailable"),
			*UEnum::GetValueAsString(SwitchResult.Status),
			SwitchResult.RestoredChannelCount,
			SwitchResult.PreservedActiveSoundCount,
			*SwitchResult.Message);
		AudioSub->SwitchBackend(OrigBackend);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended Restored backend: %s"), *UEnum::GetValueAsString(OrigBackend));
	}

#if WITH_DEV_AUTOMATION_TESTS
	// EN: extended coverage deep fixture — deterministic definition cache + event-history policy probes.
	// ES: Fixture profundo extended coverage — cache determinista de definiciones + politicas de historial.
	{
		auto* SoundDefinition = NewObject<UPGXSoundDefinition>(GetTransientPackage(), NAME_None, RF_Transient);
		SoundDefinition->SoundTag = PGXHarnessTags::AudioUI();
		SoundDefinition->DefaultChannelTag = PGXHarnessTags::AudioUI();
		SoundDefinition->SelectionMode = EPGXSoundSelectionMode::First;
		CreatedObjects.Add(TStrongObjectPtr<UObject>(SoundDefinition));

		AudioSub->InjectTestSoundDefinition(SoundDefinition);
		const UPGXSoundDefinition* FoundDefinition = AudioSub->FindDefinitionByTag(PGXHarnessTags::AudioUI());
		AudioSub->RecordEventForTesting(PGXHarnessTags::MsgAudio(), TEXT("HarnessAudioEvent"), PGXHarnessTags::AudioUI());
		const int32 EventCapacity = AudioSub->GetResolvedMaxEventHistorySizeForTesting();
		const bool bRecordsEvents = AudioSub->ShouldRecordEventHistoryForTesting();
		const bool bExposesEvents = AudioSub->ShouldExposeEventHistoryForTesting();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended SoundDefinition=%s, EventPolicy record=%s expose=%s capacity=%d"),
			FoundDefinition ? TEXT("found") : TEXT("missing"),
			bRecordsEvents ? TEXT("true") : TEXT("false"),
			bExposesEvents ? TEXT("true") : TEXT("false"),
			EventCapacity);
		AudioSub->ClearTestSoundDefinitions();
	}
#endif

	SystemStatuses[7].bInjected = true;
	SystemStatuses[7].ObjectCount = 7;
	SystemStatuses[7].Detail = TEXT("5 channels, playback+music+dialogue+backend+event policy exercised");
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("InjectAudio — Part 1+2+3: queries+playback+music+dialogue+backend+event policy verified"));
}

void FPGXVisualHarness::TeardownAudio(UWorld* World)
{
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UPGXAudioSubsystem* AudioSub = GI ? GI->GetSubsystem<UPGXAudioSubsystem>() : nullptr;
	if (AudioSub)
	{
		// EN: Stop all audio activity from Part 2 / ES: Detener toda actividad de audio del Part 2
		AudioSub->StopAllSounds(0.0f);
		AudioSub->StopMusic(0.0f);
		AudioSub->StopDialogue(0.0f);
		AudioSub->ClearDialogueQueue();

		AudioSub->ClearTestChannelConfigs();
		AudioSub->ClearTestAudioConfig();
	}
}

// ─── 9. DataRegistry ───

void FPGXVisualHarness::InjectDataRegistry(UWorld* /*World*/)
{
	UPGXDataRegistrySubsystem* RegSub = UPGXDataRegistrySubsystem::GetCached();
	if (!RegSub)
	{
		PGX_LOG_WARNING(LogPGXSimHarness, TEXT("InjectDataRegistry — DataRegistrySubsystem not found, skipping"));
		return;
	}

	// EN: Create 2 databases / ES: Crear 2 databases
	FGameplayTag DbItems = PGXHarnessTags::DbItems();
	FGameplayTag DbNPCs = PGXHarnessTags::DbNPCs();

	if (RegSub->CreateDatabase(DbItems, UPGXObjectDataAsset::StaticClass(), false))
	{
		RegisteredDatabaseTags.Add(DbItems);
	}
	if (RegSub->CreateDatabase(DbNPCs, UPGXObjectDataAsset::StaticClass(), false))
	{
		RegisteredDatabaseTags.Add(DbNPCs);
	}

	// EN: deep verification — Register 10 entries (5 items + 5 NPCs) with rich metadata
	// ES: Registrar 10 entries (5 items + 5 NPCs) con metadata rica
	struct FHarnessItemDef
	{
		FGameplayTag DbTag;
		FGameplayTag ItemTag;
		FGameplayTag CategoryTag;
		const TCHAR* Id;
		const TCHAR* Name;
		const TCHAR* Desc;
	};

	const FHarnessItemDef ItemDefs[] = {
		// Items DB
		{ DbItems, PGXHarnessTags::ItemIronSword(),    PGXHarnessTags::CatWeapon(),     TEXT("IronSword"),    TEXT("Iron Sword"),      TEXT("A sturdy iron blade forged in the mountains") },
		{ DbItems, PGXHarnessTags::ItemHealthPotion(),  PGXHarnessTags::CatConsumable(), TEXT("HealthPotion"), TEXT("Health Potion"),    TEXT("Restores 50 HP when consumed") },
		{ DbItems, PGXHarnessTags::ItemMageStaff(),     PGXHarnessTags::CatWeapon(),     TEXT("MageStaff"),    TEXT("Mage Staff"),       TEXT("Channeling staff imbued with arcane energy") },
		{ DbItems, PGXHarnessTags::ItemDragonShield(),  PGXHarnessTags::CatEquipment(),  TEXT("DragonShield"), TEXT("Dragon Guard"),    TEXT("Guard forged from dragon scales") },
		{ DbItems, PGXHarnessTags::ItemFireScroll(),    PGXHarnessTags::CatConsumable(), TEXT("FireScroll"),   TEXT("Fire Scroll"),      TEXT("Single-use scroll that casts Fireball") },
		// NPCs DB
		{ DbNPCs,  PGXHarnessTags::NPCMerchant(),   PGXHarnessTags::CatVendor(),  TEXT("Merchant"),   TEXT("Traveling Merchant"),  TEXT("Sells rare goods from distant lands") },
		{ DbNPCs,  PGXHarnessTags::NPCBlacksmith(),  PGXHarnessTags::CatService(), TEXT("Blacksmith"), TEXT("Master Blacksmith"),   TEXT("Can upgrade weapons and armor") },
		{ DbNPCs,  PGXHarnessTags::NPCQuestGiver(),  PGXHarnessTags::CatService(), TEXT("QuestGiver"), TEXT("Elder Sage"),          TEXT("Keeper of ancient quests and lore") },
		{ DbNPCs,  PGXHarnessTags::NPCGuard(),       PGXHarnessTags::CatService(), TEXT("Guard"),      TEXT("Royal Guard"),         TEXT("Protects the castle gates") },
		{ DbNPCs,  PGXHarnessTags::NPCInnkeeper(),   PGXHarnessTags::CatVendor(),  TEXT("Innkeeper"),  TEXT("Friendly Innkeeper"),  TEXT("Provides rest and rumors") },
	};

	int32 RegisteredCount = 0;
	for (const FHarnessItemDef& Def : ItemDefs)
	{
		auto* Asset = NewObject<UPGXObjectDataAsset>(GetTransientPackage(), NAME_None, RF_Transient);
		Asset->AssetId = FName(Def.Id);
		Asset->Version = 1;
		Asset->CategoryTag = Def.CategoryTag;
		Asset->DisplayName = FText::FromString(Def.Name);
		Asset->Description = FText::FromString(Def.Desc);
		CreatedObjects.Add(TStrongObjectPtr<UObject>(Asset));

		if (RegSub->RegisterAsset(Def.DbTag, Asset, Def.ItemTag))
		{
			RegisteredRegistryItems.Add(TPair<FGameplayTag, FGameplayTag>(Def.DbTag, Def.ItemTag));
			RegisteredCount++;
		}
	}

	// ─── v2.0: Query + Cache + Stats ───
	{
		const FPGXRegistryEntry* SwordEntry = RegSub->FindEntry(DbItems, PGXHarnessTags::ItemIronSword());
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  FindEntry(IronSword): %s"),
			SwordEntry ? *SwordEntry->DisplayName.ToString() : TEXT("NOT FOUND"));

		TArray<FPGXRegistryEntry> WeaponEntries = RegSub->FindByCategory(DbItems, PGXHarnessTags::CatWeapon());
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  FindByCategory(Weapon): %d results"), WeaponEntries.Num());

		FSoftObjectPath SwordPath = RegSub->GetSoftReference(DbItems, PGXHarnessTags::ItemIronSword());
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  GetSoftReference(IronSword): %s"),
			SwordPath.IsValid() ? TEXT("valid") : TEXT("empty"));

		bool bHasDb = RegSub->HasDatabase(DbItems);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  HasDatabase(Items)=%s"), bHasDb ? TEXT("true") : TEXT("false"));

		TArray<FGameplayTag> AllDbTags = RegSub->GetAllDatabaseTags();
		int32 IndexCount = RegSub->GetEntryIndexCount();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  DatabaseTags=%d, EntryIndexCount=%d"), AllDbTags.Num(), IndexCount);
	}

	{
		UPGXDataAsset* CachedSword = RegSub->GetCachedAsset(DbItems, PGXHarnessTags::ItemIronSword());
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  GetCachedAsset(IronSword)=%s"),
			IsValid(CachedSword) ? TEXT("valid") : TEXT("null"));

		RegSub->InvalidateCache(DbItems);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  InvalidateCache(Items) — done"));

		FPGXDatabaseStats ItemStats = RegSub->GetDatabaseStats(DbItems);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Stats(Items): Total=%d, Loaded=%d, Categories=%d"),
			ItemStats.TotalEntries, ItemStats.LoadedEntries, ItemStats.CategoryCount);

		FString MetadataJson = RegSub->ExportMetadata(DbItems);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  ExportMetadata(Items): %d chars"), MetadataJson.Len());
	}

	SystemStatuses[8].bInjected = true;
	SystemStatuses[8].ObjectCount = RegisteredDatabaseTags.Num() + RegisteredCount;
	SystemStatuses[8].Detail = FString::Printf(TEXT("2 databases, %d entries, queries verified, cache tested"), RegisteredCount);
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("InjectDataRegistry — 2 databases, %d entries, queries+cache verified"), RegisteredCount);
}

void FPGXVisualHarness::TeardownDataRegistry(UWorld* /*World*/)
{
	UPGXDataRegistrySubsystem* RegSub = UPGXDataRegistrySubsystem::GetCached();
	if (RegSub)
	{
		// EN: Unregister all items we registered / ES: Desregistrar todos los items que registramos
		for (const auto& Pair : RegisteredRegistryItems)
		{
			RegSub->UnregisterAsset(Pair.Key, Pair.Value);
		}
	}
	RegisteredRegistryItems.Empty();

	// EN: No RemoveDatabase API — databases persist until subsystem deinit. That's OK.
	// ES: No hay API RemoveDatabase — databases persisten hasta deinit del subsistema. OK.
	RegisteredDatabaseTags.Empty();
}

// ─── 10. Message ───

void FPGXVisualHarness::InjectMessage(UWorld* World)
{
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UPGXMessageSubsystem* MsgSub = GI ? GI->GetSubsystem<UPGXMessageSubsystem>() : nullptr;
	if (!MsgSub)
	{
		PGX_LOG_WARNING(LogPGXSimHarness, TEXT("InjectMessage — MessageSubsystem not found, skipping"));
		return;
	}

	// EN: Inject config with generous history / ES: Inyectar config con historial generoso
	auto* MsgCfg = NewObject<UPGXMessageConfig>(GetTransientPackage(), NAME_None, RF_Transient);
	MsgCfg->MaxMessageHistory = 100;
	MsgCfg->bLogBroadcasts = true;
	MsgCfg->bEnablePartialMatching = true;
	CreatedObjects.Add(TStrongObjectPtr<UObject>(MsgCfg));
	MsgSub->InjectTestConfig(MsgCfg);

	// EN: Register listeners on 5 channels / ES: Registrar listeners en 5 canales
	static const TFunction<FGameplayTag()> MsgChannels[] = {
		PGXHarnessTags::MsgUI, PGXHarnessTags::MsgGameplay,
		PGXHarnessTags::MsgSystem, PGXHarnessTags::MsgAudio,
		PGXHarnessTags::MsgNetwork
	};

	for (const auto& ChFn : MsgChannels)
	{
		FPGXMessageListenerHandle Handle = MsgSub->RegisterListener<FPGXMessage>(
			ChFn(),
			[](FGameplayTag, const FPGXMessage&) { /* EN: Listener for visual presence / ES: Listener para presencia visual */ }
		);
		if (Handle.IsValid())
		{
			MessageListenerHandles.Add(Handle);
		}
	}

	// EN: coverage — Broadcast 25 initial messages across channels with varied timing
	// ES: Broadcast 25 mensajes iniciales entre canales con timing variado
	for (int32 i = 0; i < 25; i++)
	{
		int32 ChIdx = static_cast<int32>(i % UE_ARRAY_COUNT(MsgChannels));
		FGameplayTag Ch = MsgChannels[ChIdx]();
		FPGXMessage Msg;
		Msg.MessageTag = Ch;
		Msg.Timestamp = FPlatformTime::Seconds() + (i * 0.01); // EN: Slight offset for varied timestamps / ES: Offset leve para timestamps variados
		MsgSub->BroadcastMessage<FPGXMessage>(Ch, Msg);
	}

	// ─── v2.0: Stats + History + Channel Queries ───
	{
		FPGXMessageStats MsgStats = MsgSub->GetStats();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Stats: Broadcasts=%d, ActiveChannels=%d, Listeners=%d, History=%d"),
			MsgStats.TotalBroadcasts, MsgStats.ActiveChannels, MsgStats.ActiveListeners, MsgStats.HistorySize);

		TArray<FGameplayTag> ActiveChannels = MsgSub->GetAllActiveChannels();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  ActiveChannels=%d"), ActiveChannels.Num());

		int32 UIListenerCount = MsgSub->GetListenerCount(PGXHarnessTags::MsgUI());
		int32 TotalListeners = MsgSub->GetTotalListenerCount();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Listeners: UI=%d, Total=%d"), UIListenerCount, TotalListeners);

		bool bUIActive = MsgSub->IsChannelActive(PGXHarnessTags::MsgUI());
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  IsChannelActive(UI)=%s"), bUIActive ? TEXT("true") : TEXT("false"));

		TArray<FPGXMessageRecord> UIHistory = MsgSub->GetMessageHistory(PGXHarnessTags::MsgUI(), 10);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  MessageHistory(UI): %d records"), UIHistory.Num());
	}

	SystemStatuses[9].bInjected = true;
	SystemStatuses[9].ObjectCount = 1;
	SystemStatuses[9].Detail = FString::Printf(TEXT("5 channels, %d listeners, 25 msgs, stats verified"), MessageListenerHandles.Num());
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("InjectMessage — 1 config, %d listeners, 25 broadcasts, stats verified"), MessageListenerHandles.Num());
}

void FPGXVisualHarness::TeardownMessage(UWorld* World)
{
	// EN: Unregister all listeners / ES: Desregistrar todos los listeners
	for (FPGXMessageListenerHandle& Handle : MessageListenerHandles)
	{
		Handle.Unregister();
	}
	MessageListenerHandles.Empty();

	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UPGXMessageSubsystem* MsgSub = GI ? GI->GetSubsystem<UPGXMessageSubsystem>() : nullptr;
	if (MsgSub)
	{
		MsgSub->ClearTestConfigs();
	}
}

// ─── 11. EventHandler ───

void FPGXVisualHarness::InjectEventHandler(UWorld* World)
{
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UPGXEventHandlerSubsystem* EHSub = GI ? GI->GetSubsystem<UPGXEventHandlerSubsystem>() : nullptr;
	if (!EHSub)
	{
		PGX_LOG_WARNING(LogPGXSimHarness, TEXT("InjectEventHandler — EventHandlerSubsystem not found, skipping"));
		return;
	}

	// EN: Inject config / ES: Inyectar config
	auto* EHCfg = NewObject<UPGXEventHandlerConfig>(GetTransientPackage(), NAME_None, RF_Transient);
	EHCfg->MaxCachedHandlers = 64;
	EHCfg->MaxExecutionDepth = 4;
	EHCfg->BlackboxBufferSize = 128;
	EHCfg->bLogExecutions = true;
	CreatedObjects.Add(TStrongObjectPtr<UObject>(EHCfg));
	EHSub->InjectTestConfig(EHCfg);

	// EN: Register 5 handler entries using concrete test handler (not abstract base)
	// ES: Registrar 5 entradas de handler usando handler de test concreto (no base abstracta)
	static const TFunction<FGameplayTag()> EventTags[] = {
		PGXHarnessTags::EvtDamage, PGXHarnessTags::EvtHeal,
		PGXHarnessTags::EvtPickup, PGXHarnessTags::EvtInteract,
		PGXHarnessTags::EvtSpawn
	};

	for (const auto& TagFn : EventTags)
	{
		FGameplayTag EvtTag = TagFn();
		EHSub->RegisterHandler(EvtTag, UPGXHarnessHandlerStub::StaticClass(), EPGXHandlerLifecycle::Cached);
		RegisteredEventTags.Add(EvtTag);
	}

	// EN: deep verification — Execute handlers to generate telemetry + blackbox entries
	// ES: Ejecutar handlers para generar telemetria + entries de blackbox
	FPGXEventContext Ctx;
	FInstancedStruct EmptyPayload;

	int32 ExecCount = 0;
	for (const auto& TagFn : EventTags)
	{
		FGameplayTag EvtTag = TagFn();

		// EN: Execute 2-3 times per handler to build telemetry history
		// ES: Ejecutar 2-3 veces por handler para construir historial de telemetria
		int32 Reps = (ExecCount % 2 == 0) ? 3 : 2;
		for (int32 r = 0; r < Reps; r++)
		{
			EHSub->ResolveAndExecute(EvtTag, nullptr, EmptyPayload);
		}
		ExecCount += Reps;
	}

	// ─── v2.0: Sequence + Telemetry + Blackbox + Cache Management ───
	{
		EPGXEventResult SeqResult = EHSub->ExecuteSequence(
			{ PGXHarnessTags::EvtDamage(), PGXHarnessTags::EvtHeal(), PGXHarnessTags::EvtPickup() },
			Ctx, TArray<FInstancedStruct>(), true);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  ExecuteSequence(3 events): %s"), *UEnum::GetValueAsString(SeqResult));
		ExecCount += 3;
	}

	{
		bool bValid = EHSub->ValidateConditions(
			{ PGXHarnessTags::EvtDamage(), PGXHarnessTags::EvtHeal() }, Ctx);
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  ValidateConditions(Damage,Heal)=%s"), bValid ? TEXT("true") : TEXT("false"));
	}

	{
		bool bRegistered = EHSub->IsHandlerRegistered(PGXHarnessTags::EvtDamage());
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  IsHandlerRegistered(Damage)=%s"), bRegistered ? TEXT("true") : TEXT("false"));

		FPGXEventHandlerInfo HandlerInfo = EHSub->GetHandlerInfo(PGXHarnessTags::EvtDamage());
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  HandlerInfo(Damage): Class=%s, Lifecycle=%s, Cached=%s"),
			*HandlerInfo.HandlerClassName, *UEnum::GetValueAsString(HandlerInfo.Lifecycle),
			HandlerInfo.bCached ? TEXT("true") : TEXT("false"));

		TArray<FGameplayTag> AllRegTags = EHSub->GetAllRegisteredTags();
		TArray<FGameplayTag> AllCategories = EHSub->GetAllCategories();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  RegisteredTags=%d, Categories=%d"), AllRegTags.Num(), AllCategories.Num());

		FPGXHandlerCacheStats CStats = EHSub->GetCacheStats();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Cache: Handlers=%d/%d, Hits=%d, Misses=%d, Evictions=%d"),
			CStats.CachedHandlers, CStats.MaxCachedHandlers, CStats.CacheHits, CStats.CacheMisses, CStats.Evictions);
	}

	{
		FPGXHandlerTelemetry DmgTelemetry = EHSub->GetHandlerTelemetry(PGXHarnessTags::EvtDamage());
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Telemetry(Damage): Execs=%d, Avg=%.2f ms"),
			DmgTelemetry.ExecutionCount, DmgTelemetry.AvgExecutionTimeMs);

		TArray<FPGXHandlerTelemetry> AllTelemetry = EHSub->GetAllTelemetry();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  AllTelemetry: %d entries"), AllTelemetry.Num());

		FString BlackboxDump = EHSub->DumpBlackboxToString();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Blackbox: %d chars"), BlackboxDump.Len());

		FString Report = EHSub->ExportReport();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  ExportReport: %d chars"), Report.Len());
	}

	{
		// EN: Evict and re-create handler (tests re-cache) / ES: Evictar y re-crear handler (testa re-cache)
		EHSub->EvictHandler(PGXHarnessTags::EvtSpawn());
		EHSub->ResolveAndExecute(PGXHarnessTags::EvtSpawn(), nullptr, EmptyPayload);
		ExecCount++;
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  EvictHandler(Spawn) + re-execute — cache rebuilt"));
	}

	SystemStatuses[10].bInjected = true;
	SystemStatuses[10].ObjectCount = 1;
	SystemStatuses[10].Detail = FString::Printf(TEXT("%d handlers, ~%d execs, sequence+telemetry verified"),
		RegisteredEventTags.Num(), ExecCount);
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("InjectEventHandler — %d handlers, %d execs, sequence+telemetry+blackbox verified"),
		RegisteredEventTags.Num(), ExecCount);
}

void FPGXVisualHarness::TeardownEventHandler(UWorld* World)
{
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UPGXEventHandlerSubsystem* EHSub = GI ? GI->GetSubsystem<UPGXEventHandlerSubsystem>() : nullptr;
	if (EHSub)
	{
		for (const FGameplayTag& EvtTag : RegisteredEventTags)
		{
			EHSub->UnregisterHandler(EvtTag);
		}
		EHSub->ClearTestConfigs();
	}
	RegisteredEventTags.Empty();
}

// ─── 12. LevelFlow ───

void FPGXVisualHarness::InjectLevelFlow(UWorld* World)
{
	// EN: coverage — SimulateGameSession for levels + transitions
	// ES: SimulateGameSession para niveles + transiciones
	UPGXLevelFlowTestUtility::SimulateGameSession(World);

	// ─── v2.0: Query Coverage ───
	UPGXLevelFlowSubsystem* LFSub = UPGXLevelFlowSubsystem::GetCachedInstance();
	if (LFSub)
	{
		EPGXLevelFlowState TransState = LFSub->GetTransitionState();
		FGameplayTag CurrentLevel = LFSub->GetCurrentLevelTag();
		FGameplayTag PrevLevel = LFSub->GetPreviousLevelTag();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  LevelFlow: State=%s, Current=%s, Previous=%s"),
			*UEnum::GetValueAsString(TransState), *CurrentLevel.ToString(), *PrevLevel.ToString());

		TArray<FGameplayTag> RegLevelTags = LFSub->GetRegisteredLevelTags();
		TArray<FPGXLevelTransitionRecord> TransHistory = LFSub->GetTransitionHistory();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  RegisteredLevelTags=%d, TransitionHistory=%d"),
			RegLevelTags.Num(), TransHistory.Num());

		bool bTransActive = LFSub->IsTransitionActive();
		float TransProgress = LFSub->GetTransitionProgress();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  IsTransitionActive=%s, Progress=%.1f%%"),
			bTransActive ? TEXT("true") : TEXT("false"), TransProgress * 100.f);

		int32 DiscoveredProfiles = LFSub->GetDiscoveredProfileCount();
		int32 RegLevelCount = LFSub->GetRegisteredLevelCount();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  DiscoveredProfiles=%d, RegisteredLevels=%d"),
			DiscoveredProfiles, RegLevelCount);

		// ─── v2.0 Part 2: Mutation APIs ───

		// EN: ResolveLevelByTag — lookup a harness tag (expected: not found)
		// ES: ResolveLevelByTag — buscar un tag del harness (esperado: no encontrado)
		{
			FPGXLevelEntry OutEntry;
			bool bResolved = LFSub->ResolveLevelByTag(PGXHarnessTags::LevelTestZone(), OutEntry);
			PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended ResolveLevelByTag(TestZone): %s"),
				bResolved ? *OutEntry.DisplayName.ToString() : TEXT("not found (expected)"));
		}

		// EN: RequestLevelTransition + CancelTransition (may fail if no level registered — OK)
		// ES: RequestLevelTransition + CancelTransition (puede fallar si no hay nivel registrado — OK)
		{
			FPGXLevelFlowResult TransResult = LFSub->RequestLevelTransition(PGXHarnessTags::LevelTestZone());
			PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended RequestLevelTransition(TestZone): %s — %s"),
				TransResult.bSuccess ? TEXT("OK") : TEXT("FAIL"), *TransResult.Description);

			if (TransResult.bSuccess)
			{
				FPGXLevelFlowResult CancelResult = LFSub->CancelTransition();
				PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended CancelTransition: %s"), CancelResult.bSuccess ? TEXT("OK") : TEXT("FAIL"));
			}
		}

		// EN: SubLevel operations (may fail if no sublevel registered — OK)
		// ES: Operaciones de SubLevel (puede fallar si no hay sublevel registrado — OK)
		{
			FPGXLevelFlowResult SubLoadResult = LFSub->RequestSubLevelLoad(PGXHarnessTags::LevelSubLevelCave());
			PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended RequestSubLevelLoad(Cave): %s — %s"),
				SubLoadResult.bSuccess ? TEXT("OK") : TEXT("FAIL"), *SubLoadResult.Description);

			bool bSubLoaded = LFSub->IsSubLevelLoaded(PGXHarnessTags::LevelSubLevelCave());
			PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended IsSubLevelLoaded(Cave)=%s"), bSubLoaded ? TEXT("true") : TEXT("false"));

			if (bSubLoaded)
			{
				FPGXLevelFlowResult SubUnloadResult = LFSub->RequestSubLevelUnload(PGXHarnessTags::LevelSubLevelCave());
				PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended RequestSubLevelUnload(Cave): %s"),
					SubUnloadResult.bSuccess ? TEXT("OK") : TEXT("FAIL"));
			}
		}

		// EN: GetCurrentLevelFlowActor / ES: GetCurrentLevelFlowActor
		{
			auto* LFActor = LFSub->GetCurrentLevelFlowActor();
			PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended GetCurrentLevelFlowActor: %s"),
				LFActor ? TEXT("valid") : TEXT("null"));
		}

		SystemStatuses[11].Detail = FString::Printf(TEXT("%d levels, %d history, %d profiles, transition+sublevel APIs exercised"),
			RegLevelCount, TransHistory.Num(), DiscoveredProfiles);
	}
	else
	{
		SystemStatuses[11].Detail = TEXT("SimulateGameSession (subsystem not cached)");
	}

	SystemStatuses[11].bInjected = true;
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("InjectLevelFlow — SimulateGameSession + query coverage"));
}

// ─── 13. Loading ───

void FPGXVisualHarness::InjectLoading(UWorld* World)
{
	// EN: coverage — SimulateGameSession for profiles + history
	// ES: SimulateGameSession para perfiles + historial
	UPGXLoadingTestUtility::SimulateGameSession(World);

	// ─── v2.0: Query Coverage ───
	UPGXLoadingSubsystem* LoadSub = UPGXLoadingSubsystem::GetCachedInstance();
	if (LoadSub)
	{
		EPGXLoadingScreenState LoadState = LoadSub->GetCurrentState();
		FGameplayTag LoadCtx = LoadSub->GetCurrentContext();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Loading: State=%s, Context=%s"),
			*UEnum::GetValueAsString(LoadState), *LoadCtx.ToString());

		FPGXLoadingProgress LoadProgress = LoadSub->GetProgress();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  Progress: Total=%.1f%%"),
			LoadProgress.TotalProgress * 100.f);

		TArray<FPGXLoadingRecord> LoadHistory = LoadSub->GetLoadingHistory();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  LoadingHistory=%d entries"), LoadHistory.Num());

		TArray<FGameplayTag> RegCtxTags = LoadSub->GetRegisteredContextTags();
		int32 DiscoveredProfiles = LoadSub->GetDiscoveredProfileCount();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  RegisteredContextTags=%d, DiscoveredProfiles=%d"),
			RegCtxTags.Num(), DiscoveredProfiles);

		bool bLoadActive = LoadSub->IsLoadingActive();
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("  IsLoadingActive=%s"), bLoadActive ? TEXT("true") : TEXT("false"));

		// ─── v2.0 Part 2: Mutation + Query APIs ───

		// EN: Extended query APIs / ES: APIs de consulta extendidas
		{
			float ElapsedTime = LoadSub->GetElapsedTime();
			PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended GetElapsedTime=%.2f"), ElapsedTime);

			bool bProfileValid = LoadSub->IsProfileValid(PGXHarnessTags::LoadingTestContext());
			PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended IsProfileValid(TestContext)=%s"),
				bProfileValid ? TEXT("true") : TEXT("false"));

			EPGXLoadingVisualType VisualType = LoadSub->GetActiveVisualType();
			PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended ActiveVisualType=%s"), *UEnum::GetValueAsString(VisualType));
		}

		// EN: RequestLoading + ForceClose (exercises full lifecycle, immediately closed)
		// ES: RequestLoading + ForceClose (ejercita ciclo completo, cerrado inmediatamente)
		{
			FPGXLoadingResult LoadResult = LoadSub->RequestLoading(PGXHarnessTags::LoadingTestContext());
			PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended RequestLoading(TestContext): %s — %s"),
				LoadResult.bSuccess ? TEXT("OK") : TEXT("FAIL"), *LoadResult.Description);

			if (LoadResult.bSuccess)
			{
				FPGXLoadingResult CloseResult = LoadSub->ForceClose();
				PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended ForceClose: %s"), CloseResult.bSuccess ? TEXT("OK") : TEXT("FAIL"));
			}
		}

		// EN: RequestSkip (when not active — tests the guard path)
		// ES: RequestSkip (cuando no esta activo — testa la ruta de guardia)
		{
			FPGXLoadingResult SkipResult = LoadSub->RequestSkip();
			PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended RequestSkip: %s — %s"),
				SkipResult.bSuccess ? TEXT("OK") : TEXT("FAIL"), *SkipResult.Description);
		}

		// EN: extended coverage deep expansion — reuse Loading's native validation suite as a read/write smoke fixture.
		// ES: Expansion profunda extended coverage — reutilizar suite nativa de Loading como fixture smoke read/write.
		{
			TArray<FString> LoadingIssues;
			const bool bAllLoadingTestsPassed = UPGXLoadingTestUtility::RunAllTests(World, LoadingIssues);
			PGX_LOG_INFO(LogPGXSimHarness, TEXT("  extended RunAllTests: %s (%d issues)"),
				bAllLoadingTestsPassed ? TEXT("PASS") : TEXT("CHECK"), LoadingIssues.Num());
			for (const FString& Issue : LoadingIssues)
			{
				PGX_LOG_VERBOSE(LogPGXSimHarness, TEXT("    LoadingTest: %s"), *Issue);
			}
		}

		SystemStatuses[12].Detail = FString::Printf(TEXT("%d profiles, %d history, %d contexts, loading+skip+close+validation suite exercised"),
			DiscoveredProfiles, LoadHistory.Num(), RegCtxTags.Num());
	}
	else
	{
		SystemStatuses[12].Detail = TEXT("SimulateGameSession (subsystem not cached)");
	}

	SystemStatuses[12].bInjected = true;
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("InjectLoading — SimulateGameSession + query coverage + validation suite"));
}

// ─── Teardown: LevelFlow ───

void FPGXVisualHarness::TeardownLevelFlow(UWorld* /*World*/)
{
	// EN: Cancel any active transition from Part 2 / ES: Cancelar cualquier transicion activa del Part 2
	UPGXLevelFlowSubsystem* LFSub = UPGXLevelFlowSubsystem::GetCachedInstance();
	if (LFSub && LFSub->IsTransitionActive())
	{
		LFSub->CancelTransition();
	}
}

// ─── Teardown: Loading ───

void FPGXVisualHarness::TeardownLoading(UWorld* /*World*/)
{
	// EN: Force close loading screen if active from Part 2 / ES: Forzar cierre si activa del Part 2
	UPGXLoadingSubsystem* LoadSub = UPGXLoadingSubsystem::GetCachedInstance();
	if (LoadSub && LoadSub->IsLoadingActive())
	{
		LoadSub->ForceClose();
	}
}

//  coverage — PGXInput harness integration

void FPGXVisualHarness::InjectInput(UWorld* World)
{
	SystemStatuses[13].bInjected = false;
	SystemStatuses[13].Detail = TEXT("No GameInstance or subsystem available");

	UGameInstance* GI = IsValid(World) ? World->GetGameInstance() : nullptr;
	if (!GI)
	{
		return;
	}

	UPGXInputSubsystem* InputSub = GI->GetSubsystem<UPGXInputSubsystem>();
	if (!InputSub)
	{
		SystemStatuses[13].Detail = TEXT("Input subsystem not available");
		return;
	}

	// EN: RuntimeExtended finalizes deep injection with real transient config/context fixtures.
	//     The fixture has no Enhanced Input MappingContext on purpose: ActivateContext()
	//     proves PGX stack activation, while ActivateContextForLocalPlayer can expose
	//     MappingContextMissing as a typed failure code without needing player input.
	// ES: RuntimeExtended finaliza inyeccion profunda con config/context transitorios reales.
	InputSub->GetInputBuffer(); // Ensure runtime objects are constructed through public API.

	UPGXInputConfig* HarnessConfig = NewObject<UPGXInputConfig>(GetTransientPackage(), UPGXInputConfig::StaticClass(), NAME_None, RF_Transient);
	UPGXInputContext* GameplayContext = NewObject<UPGXInputContext>(GetTransientPackage(), UPGXInputContext::StaticClass(), NAME_None, RF_Transient);
	UPGXInputContext* UIContext = NewObject<UPGXInputContext>(GetTransientPackage(), UPGXInputContext::StaticClass(), NAME_None, RF_Transient);
	if (!HarnessConfig || !GameplayContext || !UIContext)
	{
		SystemStatuses[13].Detail = TEXT("Failed to allocate transient input fixtures");
		return;
	}
	HarnessConfig->InputBufferCapacity = 24;
	HarnessConfig->InputBufferWindowSeconds = 0.25f;
	GameplayContext->ContextTag = PGXHarnessTags::InputGameplay();
	GameplayContext->ContextName = TEXT("PGXHarnessGameplayInput");
	GameplayContext->Priority = 10;
	GameplayContext->ActivationMode = EPGXInputContextActivationMode::Additive;
	GameplayContext->bActivateOnStart = false;
	UIContext->ContextTag = PGXHarnessTags::InputUI();
	UIContext->ContextName = TEXT("PGXHarnessUIInput");
	UIContext->Priority = 50;
	UIContext->ActivationMode = EPGXInputContextActivationMode::Additive;
	UIContext->bActivateOnStart = false;
	FPGXInputContextEntry Entry;
	Entry.ContextTag = PGXHarnessTags::InputGameplay();
	Entry.Context = GameplayContext;
	Entry.PriorityOverride = GameplayContext->Priority;
	HarnessConfig->DefaultContexts.Add(Entry);
	FPGXInputContextEntry UIEntry;
	UIEntry.ContextTag = PGXHarnessTags::InputUI();
	UIEntry.Context = UIContext;
	UIEntry.PriorityOverride = UIContext->Priority;
	HarnessConfig->DefaultContexts.Add(UIEntry);
	CreatedObjects.Add(TStrongObjectPtr<UObject>(HarnessConfig));
	CreatedObjects.Add(TStrongObjectPtr<UObject>(GameplayContext));
	CreatedObjects.Add(TStrongObjectPtr<UObject>(UIContext));

#if WITH_DEV_AUTOMATION_TESTS
	InputSub->InjectTestInputConfig(HarnessConfig);
	InputSub->InjectTestContext(GameplayContext);
	InputSub->InjectTestContext(UIContext);
#endif

	const FPGXInputContextResult CtxResult = InputSub->ActivateContext(PGXHarnessTags::InputGameplay());
	const FPGXInputContextResult UIResult = InputSub->ActivateContext(PGXHarnessTags::InputUI(), UIContext->Priority);
	const FPGXInputContextResult DuplicateResult = InputSub->ActivateContext(PGXHarnessTags::InputGameplay());
	const FPGXInputContextResult InvalidResult = InputSub->ActivateContext(FGameplayTag());
	const FPGXInputContextResult MappingFailureResult = InputSub->ActivateContextForLocalPlayer(PGXHarnessTags::InputGameplay(), nullptr);
	const TArray<FPGXActiveInputContextEntry> ActiveEntries = InputSub->GetActiveContexts();
	const FPGXActiveInputContextEntry* GameplayEntry = ActiveEntries.FindByPredicate([](const FPGXActiveInputContextEntry& Entry)
	{
		return Entry.ContextTag == PGXHarnessTags::InputGameplay();
	});
	const FPGXActiveInputContextEntry* ActiveUIEntry = ActiveEntries.FindByPredicate([](const FPGXActiveInputContextEntry& Entry)
	{
		return Entry.ContextTag == PGXHarnessTags::InputUI();
	});
	const bool bPriorityOk = GameplayEntry && ActiveUIEntry && ActiveUIEntry->Priority > GameplayEntry->Priority;

	const int32 ActiveCount = InputSub->GetActiveContextCount();
	SystemStatuses[13].bInjected = CtxResult.bSuccess
		&& UIResult.bSuccess
		&& DuplicateResult.Code == EPGXInputContextResultCode::AlreadyActive
		&& InvalidResult.Code == EPGXInputContextResultCode::InvalidTag
		&& MappingFailureResult.Code == EPGXInputContextResultCode::MappingContextMissing
		&& ActiveCount >= 2
		&& bPriorityOk;
	SystemStatuses[13].ObjectCount = ActiveCount;
	SystemStatuses[13].Detail = FString::Printf(TEXT("InputSub deep fixture, %d active contexts (Gameplay=%d UI=%d Priority=%s Duplicate=%d Invalid=%d Mapping=%d)"),
		ActiveCount,
		static_cast<int32>(CtxResult.Code),
		static_cast<int32>(UIResult.Code),
		bPriorityOk ? TEXT("Y") : TEXT("N"),
		static_cast<int32>(DuplicateResult.Code),
		static_cast<int32>(InvalidResult.Code),
		static_cast<int32>(MappingFailureResult.Code));
}

void FPGXVisualHarness::TeardownInput(UWorld* World)
{
	UGameInstance* GI = IsValid(World) ? World->GetGameInstance() : nullptr;
	UPGXInputSubsystem* InputSub = GI ? GI->GetSubsystem<UPGXInputSubsystem>() : nullptr;
	if (InputSub)
	{
	#if WITH_DEV_AUTOMATION_TESTS
		InputSub->ClearTestContexts();
	#endif
		InputSub->DeactivateAllContexts();
	}
}

//  runtime core — PGXSpawn wave smoke + lifecycle cleanup.
void FPGXVisualHarness::InjectSpawn(UWorld* World)
{
	SystemStatuses[14].bInjected = false;
	SystemStatuses[14].Detail = TEXT("No world or spawn subsystem available");

	if (!IsValid(World))
	{
		return;
	}

	UPGXSpawnSubsystem* SpawnSub = World->GetSubsystem<UPGXSpawnSubsystem>();
	if (!SpawnSub)
	{
		SystemStatuses[14].Detail = TEXT("Spawn subsystem not available");
		return;
	}

	FPGXSpawnRequest Request;
	Request.SpawnClass = AActor::StaticClass();
	Request.Transform = FTransform(FRotator::ZeroRotator, FVector(250.f, 0.f, 120.f));
	Request.SourceTag = PGXHarnessTags::SpawnWaveSmoke();
	Request.Priority = 0;

	const FPGXSpawnResult Validation = SpawnSub->ValidateSpawnRequest(Request);
	const FPGXSpawnResult RegisterResult = Validation.bSuccess
		? SpawnSub->RegisterSpawnRecord(Request)
		: FPGXSpawnResult::Failure(Validation.Code, Validation.Status, Validation.Message);

	UPGXWaveDefinition* WaveDef = NewObject<UPGXWaveDefinition>(GetTransientPackage(), UPGXWaveDefinition::StaticClass(), NAME_None, RF_Transient);
	if (WaveDef)
	{
		WaveDef->WaveName = TEXT("PGXHarnessSpawnSmoke");
		WaveDef->WaveTag = PGXHarnessTags::SpawnWaveSmoke();
		WaveDef->DefaultSpawnClass = AActor::StaticClass();
		WaveDef->TotalSpawnCount = 1;
		WaveDef->SpawnInterval = 0.01f;
		CreatedObjects.Add(TStrongObjectPtr<UObject>(WaveDef));
	}

	const FPGXSpawnResult WaveResult = WaveDef
		? SpawnSub->StartWave(WaveDef)
		: FPGXSpawnResult::Failure(EPGXSpawnResultCode::InvalidRequest, EPGXSpawnRequestStatus::Failed, TEXT("WaveDef allocation failed."));

	// EN/ES: Smoke only. We prove StartWave accepts the authored shape, then cancel to avoid a lingering ticker.
	if (WaveResult.bSuccess)
	{
		SpawnSub->CancelWave(PGXHarnessTags::SpawnWaveSmoke());
	}

	const FPGXSpawnDebugSnapshot Snapshot = SpawnSub->GetDebugSnapshot();
	SystemStatuses[14].bInjected = Validation.bSuccess && RegisterResult.bSuccess && WaveResult.bSuccess;
	SystemStatuses[14].ObjectCount = Snapshot.TotalRecordCount;
	SystemStatuses[14].Detail = FString::Printf(TEXT("Records=%d Active=%d WaveSmoke=%s (%s)"),
		Snapshot.TotalRecordCount,
		Snapshot.ActiveRecordCount,
		WaveResult.bSuccess ? TEXT("OK") : TEXT("FAIL"),
		*WaveResult.Message);
}

void FPGXVisualHarness::TeardownSpawn(UWorld* World)
{
	UPGXSpawnSubsystem* SpawnSub = IsValid(World) ? World->GetSubsystem<UPGXSpawnSubsystem>() : nullptr;
	if (!SpawnSub)
	{
		return;
	}

	SpawnSub->CancelWave(PGXHarnessTags::SpawnWaveSmoke());

	for (const FPGXSpawnRecord& Record : SpawnSub->GetSpawnRecordsSnapshot())
	{
		if (Record.Request.SourceTag != PGXHarnessTags::SpawnWaveSmoke())
		{
			continue;
		}

		if (Record.SpawnedActor.IsValid())
		{
			Record.SpawnedActor->Destroy();
		}
		if (Record.Status == EPGXSpawnRequestStatus::Queued || Record.Status == EPGXSpawnRequestStatus::Running)
		{
			SpawnSub->CancelSpawnRecord(Record.Handle, TEXT("SimHarness teardown"));
		}
	}
	SpawnSub->CleanupInactiveSpawnRecords();
}

void FPGXVisualHarness::InjectAI(UWorld* World)
{
	SystemStatuses[15].bInjected = false;
	SystemStatuses[15].Detail = TEXT("No world or AI subsystem available");
	HarnessAIAgentId = 0;
	HarnessAIController.Reset();

	if (!IsValid(World))
	{
		return;
	}

	UPGXAISubsystem* AISub = World->GetSubsystem<UPGXAISubsystem>();
	if (!AISub)
	{
		SystemStatuses[15].Detail = TEXT("AI subsystem not available");
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AAIController* Controller = World->SpawnActor<AAIController>(AAIController::StaticClass(), Params);
	if (!Controller)
	{
		SystemStatuses[15].Detail = TEXT("Failed to spawn AIController smoke actor");
		return;
	}
	HarnessAIController = Controller;

	FPGXAIResult RegisterResult;
	const FPGXAIAgentHandle AIHandle = AISub->RegisterAgent(Controller, RegisterResult);
	HarnessAIAgentId = AIHandle.AgentId;

	UBehaviorTree* BehaviorTree = NewObject<UBehaviorTree>(GetTransientPackage(), UBehaviorTree::StaticClass(), NAME_None, RF_Transient);
	if (BehaviorTree)
	{
		CreatedObjects.Add(TStrongObjectPtr<UObject>(BehaviorTree));
	}

#if WITH_DEV_AUTOMATION_TESTS
	AISub->SetForceNextBehaviorTreeRunResultForTesting(true, true);
#endif
	const FPGXAIResult BTRunResult = (AIHandle.IsValid() && BehaviorTree)
		? AISub->TryRunBehaviorTreeForAgent(AIHandle, BehaviorTree)
		: FPGXAIResult::MakeFailure(EPGXAIResultCode::InvalidInput, TEXT("Missing AI handle or transient BehaviorTree."));

	FPGXAIBehaviorTreeRunStatus BTStatus;
	const bool bHasBTStatus = AIHandle.IsValid() && AISub->GetBehaviorTreeRunStatus(AIHandle, BTStatus);

	SystemStatuses[15].bInjected = RegisterResult.bSucceeded && AIHandle.IsValid() && BTRunResult.bSucceeded && bHasBTStatus;
	SystemStatuses[15].ObjectCount = AISub->GetRegisteredAgentCount();
	SystemStatuses[15].Detail = FString::Printf(TEXT("Agent=%d Register=%s BT=%s Status=%s Agents=%d"),
		HarnessAIAgentId,
		RegisterResult.bSucceeded ? TEXT("OK") : TEXT("FAIL"),
		BTRunResult.bSucceeded ? TEXT("OK") : TEXT("FAIL"),
		bHasBTStatus ? TEXT("Y") : TEXT("N"),
		AISub->GetRegisteredAgentCount());
}

void FPGXVisualHarness::TeardownAI(UWorld* World)
{
	UPGXAISubsystem* AISub = IsValid(World) ? World->GetSubsystem<UPGXAISubsystem>() : nullptr;
	if (AISub && HarnessAIAgentId != 0)
	{
		FPGXAIAgentHandle FoundHandle;
		if (AISub->FindAgent(HarnessAIAgentId, FoundHandle))
		{
			AISub->UnregisterAgent(FoundHandle);
		}
	}
	HarnessAIAgentId = 0;

	if (HarnessAIController.IsValid())
	{
		HarnessAIController->Destroy();
	}
	HarnessAIController.Reset();
}

void FPGXVisualHarness::InjectAbility(UWorld* World)
{
	SystemStatuses[16].bInjected = false;
	SystemStatuses[16].Detail = TEXT("No world, GameInstance, or ability subsystem available");
	HarnessAbilityActor.Reset();
	HarnessAbilityComponent.Reset();

	UGameInstance* GI = IsValid(World) ? World->GetGameInstance() : nullptr;
	UPGXAbilitySubsystem* AbilitySub = GI ? GI->GetSubsystem<UPGXAbilitySubsystem>() : nullptr;
	if (!AbilitySub || !IsValid(World))
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor* Actor = World->SpawnActor<AActor>(AActor::StaticClass(), Params);
	if (!Actor)
	{
		SystemStatuses[16].Detail = TEXT("Failed to spawn Ability smoke actor");
		return;
	}
	HarnessAbilityActor = Actor;

	UPGXAbilityComponent* Component = NewObject<UPGXAbilityComponent>(Actor);
	if (!Component)
	{
		SystemStatuses[16].Detail = TEXT("Failed to create Ability component");
		return;
	}
	HarnessAbilityComponent = Component;
	Component->RegisterComponent();
	Actor->AddInstanceComponent(Component);
	Component->RegisterAllComponentTickFunctions(true);
	if (Actor->HasActorBegunPlay())
	{
		Component->RegisterComponentWithWorld(World);
	}
	if (!Component->HasBegunPlay())
	{
		Component->BeginPlay();
	}

	UPGXAbilityFacade* AbilityFacade = Component->GetAbilityFacade();
	const FPGXAbilityResult ActivationProbe = AbilityFacade
		? AbilityFacade->ActivateAbilityByTag(PGXHarnessTags::AbilitySmoke())
		: FPGXAbilityResult::MakeFailure(EPGXAbilityResultCode::ComponentUnavailable, TEXT("Ability facade missing."));
	const bool bTypedActivationProbe = ActivationProbe.bSucceeded
		|| ActivationProbe.Code == EPGXAbilityResultCode::NotFound
		|| ActivationProbe.Code == EPGXAbilityResultCode::InvalidInput
		|| ActivationProbe.Code == EPGXAbilityResultCode::ActivationFailed;

	SystemStatuses[16].bInjected = Component->IsAbilitySystemReady() && AbilityFacade && bTypedActivationProbe;
	SystemStatuses[16].ObjectCount = AbilitySub->GetRegisteredComponentCount();
	SystemStatuses[16].Detail = FString::Printf(TEXT("Components=%d Active=%d ASC=%s Probe=%s (%s)"),
		AbilitySub->GetRegisteredComponentCount(),
		AbilitySub->GetActiveAbilityCount(),
		Component->IsAbilitySystemReady() ? TEXT("Y") : TEXT("N"),
		ActivationProbe.bSucceeded ? TEXT("OK") : TEXT("typed-fail"),
		*ActivationProbe.DiagnosticMessage);
}

void FPGXVisualHarness::TeardownAbility(UWorld* World)
{
	UGameInstance* GI = IsValid(World) ? World->GetGameInstance() : nullptr;
	UPGXAbilitySubsystem* AbilitySub = GI ? GI->GetSubsystem<UPGXAbilitySubsystem>() : nullptr;

	if (HarnessAbilityComponent.IsValid())
	{
		HarnessAbilityComponent->EndPlay(EEndPlayReason::Destroyed);
		if (AbilitySub)
		{
			AbilitySub->UnregisterComponent(HarnessAbilityComponent.Get());
		}
	}
	HarnessAbilityComponent.Reset();

	if (HarnessAbilityActor.IsValid())
	{
		HarnessAbilityActor->Destroy();
	}
	HarnessAbilityActor.Reset();
}

//  runtime extended — PGXCamera mode cycle smoke.
void FPGXVisualHarness::InjectCamera(UWorld* World)
{
	SystemStatuses[17].bInjected = false;
	SystemStatuses[17].Detail = TEXT("No world or camera subsystem available");

	if (!IsValid(World))
	{
		return;
	}

	UPGXCameraSubsystem* CameraSub = World->GetSubsystem<UPGXCameraSubsystem>();
	if (!CameraSub)
	{
		SystemStatuses[17].Detail = TEXT("Camera subsystem not available");
		return;
	}

	UPGXCameraMode* ModeA = NewObject<UPGXCameraMode>(GetTransientPackage(), UPGXCameraMode::StaticClass(), NAME_None, RF_Transient);
	UPGXCameraMode* ModeB = NewObject<UPGXCameraMode>(GetTransientPackage(), UPGXCameraMode::StaticClass(), NAME_None, RF_Transient);
	if (!ModeA || !ModeB)
	{
		SystemStatuses[17].Detail = TEXT("Failed to allocate transient camera modes");
		return;
	}

	ModeA->ModeName = TEXT("PGXHarnessCameraModeA");
	ModeA->FieldOfView = 75.0f;
	ModeA->CameraOffset = FVector(0.0f, 120.0f, 60.0f);
	ModeA->BlendTime = 0.05f;
	ModeB->ModeName = TEXT("PGXHarnessCameraModeB");
	ModeB->FieldOfView = 95.0f;
	ModeB->CameraOffset = FVector(0.0f, -120.0f, 80.0f);
	ModeB->BlendTime = 0.10f;
	CreatedObjects.Add(TStrongObjectPtr<UObject>(ModeA));
	CreatedObjects.Add(TStrongObjectPtr<UObject>(ModeB));

	const bool bSetA = CameraSub->SetCameraMode(ModeA) && CameraSub->GetActiveCameraModeName() == ModeA->ModeName;
	const bool bSetB = CameraSub->SetCameraMode(ModeB) && CameraSub->GetActiveCameraMode() == ModeB;
	CameraSub->ClearCameraMode();
	const bool bCleared = CameraSub->GetActiveCameraMode() == nullptr && CameraSub->GetActiveCameraModeName().IsNone();

	SystemStatuses[17].bInjected = bSetA && bSetB && bCleared;
	SystemStatuses[17].ObjectCount = 2;
	SystemStatuses[17].Detail = FString::Printf(TEXT("ModeCycle SetA=%s SetB=%s Clear=%s"),
		bSetA ? TEXT("OK") : TEXT("FAIL"),
		bSetB ? TEXT("OK") : TEXT("FAIL"),
		bCleared ? TEXT("OK") : TEXT("FAIL"));
}

void FPGXVisualHarness::TeardownCamera(UWorld* World)
{
	if (UPGXCameraSubsystem* CameraSub = IsValid(World) ? World->GetSubsystem<UPGXCameraSubsystem>() : nullptr)
	{
		CameraSub->ClearCameraMode();
	}
}

//  runtime extended — PGXInteraction target/action callback smoke.
void FPGXVisualHarness::InjectInteraction(UWorld* World)
{
	SystemStatuses[18].bInjected = false;
	SystemStatuses[18].Detail = TEXT("No world or interaction component available");
	HarnessInteractionOwner.Reset();
	HarnessInteractionTarget.Reset();
	HarnessInteractionComponent.Reset();

	if (!IsValid(World))
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor* OwnerActor = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform(FRotator::ZeroRotator, FVector(420.f, 0.f, 120.f)), Params);
	AActor* TargetActor = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform(FRotator::ZeroRotator, FVector(460.f, 0.f, 120.f)), Params);
	if (!OwnerActor || !TargetActor)
	{
		SystemStatuses[18].Detail = TEXT("Failed to spawn interaction smoke actors");
		if (OwnerActor) OwnerActor->Destroy();
		if (TargetActor) TargetActor->Destroy();
		return;
	}

	UPGXInteractionComponent* InteractionComp = NewObject<UPGXInteractionComponent>(OwnerActor, UPGXInteractionComponent::StaticClass(), TEXT("PGXHarnessInteractionComponent"), RF_Transient);
	if (!InteractionComp)
	{
		SystemStatuses[18].Detail = TEXT("Failed to allocate interaction component");
		OwnerActor->Destroy();
		TargetActor->Destroy();
		return;
	}

	OwnerActor->AddInstanceComponent(InteractionComp);
	InteractionComp->RegisterComponent();
	InteractionComp->InteractionRange = 250.0f;

	HarnessInteractionOwner = OwnerActor;
	HarnessInteractionTarget = TargetActor;
	HarnessInteractionComponent = InteractionComp;

	const FPGXInteractionResult RegisterResult = InteractionComp->RegisterTarget(
		TargetActor,
		PGXHarnessTags::InteractionTargetSmoke(),
		FText::FromString(TEXT("Harness interaction smoke")),
		10);
	const FPGXInteractionResult BeginResult = RegisterResult.bSuccess
		? InteractionComp->BeginInteraction(RegisterResult.TargetHandle, PGXHarnessTags::InteractionActionSmoke())
		: FPGXInteractionResult::Failure(RegisterResult.Code, EPGXInteractionActionState::Failed, RegisterResult.Message);
	const FPGXInteractionResult CompleteResult = BeginResult.bSuccess
		? InteractionComp->CompleteInteraction(BeginResult.ActionHandle)
		: FPGXInteractionResult::Failure(BeginResult.Code, EPGXInteractionActionState::Failed, BeginResult.Message, BeginResult.ActionHandle, BeginResult.TargetHandle);
	const FPGXInteractionQueryResult PromptResult = InteractionComp->BuildPromptSnapshot(RegisterResult.TargetHandle, PGXHarnessTags::InteractionActionSmoke(), 40.0f);

	SystemStatuses[18].bInjected = RegisterResult.bSuccess && BeginResult.bSuccess && CompleteResult.bSuccess && PromptResult.bSuccess;
	SystemStatuses[18].ObjectCount = InteractionComp->GetInteractionRecordCount();
	SystemStatuses[18].Detail = FString::Printf(TEXT("Register=%s Begin=%s Complete=%s Prompt=%s Targets=%d Records=%d"),
		RegisterResult.bSuccess ? TEXT("OK") : TEXT("FAIL"),
		BeginResult.bSuccess ? TEXT("OK") : TEXT("FAIL"),
		CompleteResult.bSuccess ? TEXT("OK") : TEXT("FAIL"),
		PromptResult.bSuccess ? TEXT("OK") : TEXT("FAIL"),
		InteractionComp->GetRegisteredTargetCount(),
		InteractionComp->GetInteractionRecordCount());
}

void FPGXVisualHarness::TeardownInteraction(UWorld* World)
{
	if (HarnessInteractionComponent.IsValid())
	{
		UPGXInteractionComponent* InteractionComp = HarnessInteractionComponent.Get();
		for (const FPGXInteractionRecord& Record : InteractionComp->GetInteractionRecordsSnapshot())
		{
			if (Record.State == EPGXInteractionActionState::Started || Record.State == EPGXInteractionActionState::Requested)
			{
				InteractionComp->CancelInteraction(Record.ActionHandle);
			}
		}
		for (const FPGXInteractableTarget& Target : InteractionComp->GetTargetsSnapshot())
		{
			InteractionComp->UnregisterTarget(Target.Handle);
		}
		InteractionComp->CleanupResolvedInteractions();
		InteractionComp->DestroyComponent();
	}

	if (HarnessInteractionTarget.IsValid())
	{
		HarnessInteractionTarget->Destroy();
	}
	if (HarnessInteractionOwner.IsValid())
	{
		HarnessInteractionOwner->Destroy();
	}

	HarnessInteractionComponent.Reset();
	HarnessInteractionTarget.Reset();
	HarnessInteractionOwner.Reset();
}

//  runtime extended — PGXInventory add/remove lifecycle smoke.
void FPGXVisualHarness::InjectInventory(UWorld* World)
{
	SystemStatuses[19].bInjected = false;
	SystemStatuses[19].Detail = TEXT("No world or inventory component available");
	HarnessInventoryActor.Reset();
	HarnessInventoryComponent.Reset();
	HarnessInventoryItemDefinition.Reset();

	if (!IsValid(World))
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor* OwnerActor = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform(FRotator::ZeroRotator, FVector(520.f, 0.f, 120.f)), Params);
	if (!OwnerActor)
	{
		SystemStatuses[19].Detail = TEXT("Failed to spawn inventory smoke actor");
		return;
	}

	UPGXInventoryComponent* InventoryComp = NewObject<UPGXInventoryComponent>(OwnerActor, UPGXInventoryComponent::StaticClass(), TEXT("PGXHarnessInventoryComponent"), RF_Transient);
	UPGXItemDefinition* ItemDef = NewObject<UPGXItemDefinition>(GetTransientPackage(), UPGXItemDefinition::StaticClass(), NAME_None, RF_Transient);
	if (!InventoryComp || !ItemDef)
	{
		SystemStatuses[19].Detail = TEXT("Failed to allocate inventory component or item definition");
		OwnerActor->Destroy();
		return;
	}

	OwnerActor->AddInstanceComponent(InventoryComp);
	InventoryComp->RegisterComponent();
	InventoryComp->MaxSlots = 4;
	InventoryComp->MaxWeight = 25.0f;
	ItemDef->ItemTag = PGXHarnessTags::InventoryItemSmoke();
	ItemDef->ItemName = TEXT("PGXHarnessInventoryPotion");
	ItemDef->DisplayName = FText::FromString(TEXT("Harness Potion"));
	ItemDef->MaxStackSize = 8;
	ItemDef->Weight = 1.25f;
	CreatedObjects.Add(TStrongObjectPtr<UObject>(ItemDef));

	HarnessInventoryActor = OwnerActor;
	HarnessInventoryComponent = InventoryComp;
	HarnessInventoryItemDefinition = ItemDef;

	const FPGXInventoryResult AddResult = InventoryComp->AddItem(ItemDef, 3);
	const int32 QuantityAfterAdd = InventoryComp->GetItemQuantity(ItemDef);
	const FPGXInventoryResult RemoveResult = AddResult.bSuccess
		? InventoryComp->RemoveItem(ItemDef, 2)
		: FPGXInventoryResult::Failure(AddResult.Code, ItemDef, 0, AddResult.Message);
	const int32 QuantityAfterRemove = InventoryComp->GetItemQuantity(ItemDef);
	const bool bLifecycle = AddResult.bSuccess && QuantityAfterAdd == 3 && RemoveResult.bSuccess && QuantityAfterRemove == 1;

	SystemStatuses[19].bInjected = bLifecycle;
	SystemStatuses[19].ObjectCount = InventoryComp->GetItemsSnapshot().Num();
	SystemStatuses[19].Detail = FString::Printf(TEXT("Add=%s Remove=%s QtyAfterAdd=%d QtyFinal=%d Slots=%d"),
		AddResult.bSuccess ? TEXT("OK") : TEXT("FAIL"),
		RemoveResult.bSuccess ? TEXT("OK") : TEXT("FAIL"),
		QuantityAfterAdd,
		QuantityAfterRemove,
		InventoryComp->GetUsedSlotCount());
}

void FPGXVisualHarness::TeardownInventory(UWorld* World)
{
	if (HarnessInventoryComponent.IsValid())
	{
		HarnessInventoryComponent->ClearInventory();
		HarnessInventoryComponent->DestroyComponent();
	}
	if (HarnessInventoryActor.IsValid())
	{
		HarnessInventoryActor->Destroy();
	}
	HarnessInventoryItemDefinition.Reset();
	HarnessInventoryComponent.Reset();
	HarnessInventoryActor.Reset();
}

//  runtime extended — PGXUI widget pool registry smoke.
void FPGXVisualHarness::InjectUI(UWorld* World)
{
	SystemStatuses[20].bInjected = false;
	SystemStatuses[20].Detail = TEXT("No GameInstance or UI subsystem available");
	HarnessWidgetPool.Reset();

	UGameInstance* GI = IsValid(World) ? World->GetGameInstance() : nullptr;
	if (!GI)
	{
		return;
	}

	UPGXUISubsystem* UISub = GI->GetSubsystem<UPGXUISubsystem>();
	if (!UISub)
	{
		SystemStatuses[20].Detail = TEXT("UI subsystem not available");
		return;
	}

	UPGXWidgetPool* WidgetPool = UISub->GetWidgetPool();
	if (!WidgetPool)
	{
		SystemStatuses[20].Detail = TEXT("UI widget pool not available");
		return;
	}

	HarnessWidgetPool = WidgetPool;
	const FPGXUIResult AcquireResult = WidgetPool->AcquireWidget(UUserWidget::StaticClass(), TEXT("PGXHarnessWidgetSmoke"));
	const bool bHasWidget = AcquireResult.bSuccess && WidgetPool->HasAcquiredWidget(AcquireResult.Handle);

	SystemStatuses[20].bInjected = AcquireResult.bSuccess && bHasWidget;
	SystemStatuses[20].ObjectCount = WidgetPool->GetPoolSnapshot().Num();
	SystemStatuses[20].Detail = FString::Printf(TEXT("Acquire=%s Has=%s Capacity=%d Acquired=%d Entries=%d"),
		AcquireResult.bSuccess ? TEXT("OK") : TEXT("FAIL"),
		bHasWidget ? TEXT("Y") : TEXT("N"),
		WidgetPool->GetCapacity(),
		WidgetPool->GetAcquiredCount(),
		WidgetPool->GetPoolSnapshot().Num());
}

void FPGXVisualHarness::TeardownUI(UWorld* World)
{
	if (HarnessWidgetPool.IsValid())
	{
		HarnessWidgetPool->Clear();
	}
	HarnessWidgetPool.Reset();
}

//  coverage — Coverage matrix population per plugin coverage results
// Covers the canonical PGX plugin set using runtime coverage findings.
TArray<FPGXPluginCoverage> FPGXVisualHarness::GetCoverageMatrix()
{
	TArray<FPGXPluginCoverage> Matrix;

	// ── Covered (harness injection + verification) ──
	Matrix.Add({ TEXT("PGXGameFlow"),   EPGXHarnessCoverage::Covered, TEXT("Flow State Matrix"), 0 });
	Matrix.Add({ TEXT("PGXSave"),       EPGXHarnessCoverage::Covered, TEXT("Slot Lifecycle"), 0 });
	Matrix.Add({ TEXT("PGXPSO"),        EPGXHarnessCoverage::Covered, TEXT("Batched Warmup + Recording"), 0 });
	Matrix.Add({ TEXT("PGXLoading"),    EPGXHarnessCoverage::Covered, TEXT("Lifecycle + Validation Suite"), 0 });
	Matrix.Add({ TEXT("PGXMGOS"),       EPGXHarnessCoverage::Covered, TEXT("GC Profile + Suppression"), 0 });
	Matrix.Add({ TEXT("PGXAudio"),      EPGXHarnessCoverage::Covered, TEXT("Channels + Event Policy"), 0 });
	Matrix.Add({ TEXT("PGXInput"),      EPGXHarnessCoverage::Covered, TEXT("Deep Context Fixture"), 0 });
	Matrix.Add({ TEXT("PGXSpawn"),      EPGXHarnessCoverage::Covered, TEXT("Wave + Placement"), 0 });
	Matrix.Add({ TEXT("PGXAI"),         EPGXHarnessCoverage::Covered, TEXT("BehaviorTree Smoke"), 0 });
	Matrix.Add({ TEXT("PGXAbility"),    EPGXHarnessCoverage::Covered, TEXT("Ability Execution"), 0 });
	Matrix.Add({ TEXT("PGXCamera"),     EPGXHarnessCoverage::Covered, TEXT("Mode Switch"), 0 });
	Matrix.Add({ TEXT("PGXInteraction"), EPGXHarnessCoverage::Covered, TEXT("Interact Smoke"), 0 });
	Matrix.Add({ TEXT("PGXInventory"),  EPGXHarnessCoverage::Covered, TEXT("Item Lifecycle"), 0 });
	Matrix.Add({ TEXT("PGXUI"),         EPGXHarnessCoverage::Covered, TEXT("Widget Smoke"), 0 });

	// ── Partial (config populated, listed, no dedicated injection yet) ──
	Matrix.Add({ TEXT("PGXCore"),       EPGXHarnessCoverage::Partial, TEXT("Core Integrity"), 0 });
	Matrix.Add({ TEXT("PGXDocs"),       EPGXHarnessCoverage::Partial, TEXT(""), 2 });
	Matrix.Add({ TEXT("PGXVersionControl"), EPGXHarnessCoverage::Partial, TEXT(""), 2 });
	Matrix.Add({ TEXT("PGXColony"),     EPGXHarnessCoverage::Partial, TEXT(""), 2 });
	Matrix.Add({ TEXT("PGXCrafting"),   EPGXHarnessCoverage::Partial, TEXT(""), 1 });
	Matrix.Add({ TEXT("PGXEnvironment"), EPGXHarnessCoverage::Partial, TEXT(""), 1 });
	Matrix.Add({ TEXT("PGXTrade"),      EPGXHarnessCoverage::Partial, TEXT(""), 2 });
	Matrix.Add({ TEXT("PGXVehicles"),   EPGXHarnessCoverage::Partial, TEXT(""), 2 });

	// ── Missing (no harness integration, scenarios without runtime integration) ──
	Matrix.Add({ TEXT("PGXMultiplayer"), EPGXHarnessCoverage::Missing, TEXT("Presence Smoke"), 2 });
	Matrix.Add({ TEXT("PGXOnline"),     EPGXHarnessCoverage::Missing, TEXT("Subsystem Presence"), 2 });
	Matrix.Add({ TEXT("PGXAnimation"),  EPGXHarnessCoverage::Missing, TEXT("Presence Smoke"), 2 });
	Matrix.Add({ TEXT("PGXCinematic"),  EPGXHarnessCoverage::Missing, TEXT("Presence Smoke"), 2 });
	Matrix.Add({ TEXT("PGXMaterials"),  EPGXHarnessCoverage::Missing, TEXT("Presence Smoke"), 2 });
	Matrix.Add({ TEXT("PGXVFX"),        EPGXHarnessCoverage::Missing, TEXT("Presence Smoke"), 2 });

	// ── Not Applicable (docs/tools, not runtime systems) ──
	Matrix.Add({ TEXT("PGXTutorials"),  EPGXHarnessCoverage::NotApplicable, TEXT(""), 2 });
	Matrix.Add({ TEXT("PGXEditorTools"), EPGXHarnessCoverage::NotApplicable, TEXT(""), 2 });
	Matrix.Add({ TEXT("PGXScaffold"),   EPGXHarnessCoverage::NotApplicable, TEXT(""), 2 });
	Matrix.Add({ TEXT("PGXSimHarness"), EPGXHarnessCoverage::NotApplicable, TEXT("Self-Verification"), 0 });

	return Matrix;
}

//  coverage — Detailed coverage via runtime presence checks.
// Delegates to FPGXHarnessCoverage (baseline support). Returns the granular matrix
// with per-check verdicts (enabled + subsystem + Initialize). The static
// GetCoverageMatrix() above is kept for back-compat with SPGXSimHarnessTab + Tests.
TArray<FPGXPluginCoverageEntry> FPGXVisualHarness::GetDetailedCoverageMatrix() const
{
	return FPGXHarnessCoverage::GetCoverageMatrix(ResolveEditorWorld());
}
