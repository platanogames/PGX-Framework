// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "UObject/StrongObjectPtr.h"
#include "Containers/Ticker.h"
#include "GameplayTagContainer.h"
#include "Construction/PGXConstructionSettings.h"
#include "Messages/PGXMessage.h"
#include "FPGXHarnessCoverage.h"

class UWorld;
class AAIController;
class AActor;
class UPGXAbilityComponent;
class UPGXInteractionComponent;
class UPGXInventoryComponent;
class UPGXItemDefinition;
class UPGXWidgetPool;

/**
 * EN: Status of a single system within the Visual Harness.
 * ES: Estado de un sistema individual dentro del Visual Harness.
 */
struct FPGXHarnessSystemStatus
{
	FString SystemName;
	bool bInjected = false;
	int32 ObjectCount = 0;
	FString Detail;
};

/**
 * EN: Coverage status for a PGX plugin in the SimHarness verification matrix.
 * ES: Estado de cobertura de un plugin PGX en la matriz de verificacion SimHarness.
 *  coverage — coverage matrix for public demonstration visibility.
 */
enum class EPGXHarnessCoverage : uint8
{
	Covered,      // Full harness injection + verification
	Partial,      // Some harness integration (config populated, listed, but no dedicated injection)
	Missing,      // No harness integration
	NotApplicable // Not a runtime system (docs, tools, scaffolding)
};

struct FPGXPluginCoverage
{
	FString PluginName;
	EPGXHarnessCoverage Coverage;
	FString ScenarioName;   // Designed scenario name, empty if not designed
	uint8 Priority = 2;     // 0=high-priority, 1=medium-priority, 2=extended
};

/**
 * EN: Single action entry in the harness execution log.
 * ES: Entrada individual de accion en el log de ejecucion del harness.
 */
struct FPGXHarnessActionEntry
{
	double Timestamp = 0.0;    // EN: Seconds since Setup() / ES: Segundos desde Setup()
	FString Category;          // "Setup", "QuickAction", "Simulation", "Verification", "Teardown"
	FString Action;            // "InjectProfile", "BroadcastTestMessage", etc.
	bool bSuccess = true;
	FString Detail;            // EN: Human-readable result / ES: Resultado legible
};

/**
 * EN: Snapshot of a single VerifyAllAPIs() run.
 * ES: Snapshot de una ejecucion de VerifyAllAPIs().
 */
struct FPGXVerificationRun
{
	int32 RunNumber = 0;
	double Timestamp = 0.0;    // EN: Seconds since Setup() / ES: Segundos desde Setup()
	int32 PassCount = 0;
	int32 TotalChecks = 0;
	TArray<TPair<FString, FString>> PerSystemResults; // ("Profile", "4/4")
};

/**
 * EN: Visual verification harness for PGX inspector panels.
 *     Injects rich data into 17 subsystems so NomadTabs display meaningful information.
 *     Supports live simulation via FTSTicker for continuous activity generation.
 *
 * ES: Harness de verificacion visual para paneles inspector PGX.
 *     Inyecta datos ricos en 17 subsistemas para que los NomadTabs muestren informacion significativa.
 *     Soporta simulacion en vivo via FTSTicker para generacion continua de actividad.
 *
 * Lifecycle: Setup() -> StartSimulation() -> StopSimulation() -> Teardown()
 */
class PGXSIMHARNESSEDITOR_API FPGXVisualHarness
{
public:
	~FPGXVisualHarness();

	// ─── Lifecycle ───

	/** EN: Inject rich visual data into 17 subsystems / ES: Inyectar datos ricos en 17 subsistemas */
	void Setup(UWorld* InWorld);

	/** EN: Clean up all injected data / ES: Limpiar todos los datos inyectados */
	void Teardown();

	/** EN: Start continuous simulation ticker / ES: Iniciar ticker de simulacion continua */
	void StartSimulation();

	/** EN: Stop simulation ticker / ES: Detener ticker de simulacion */
	void StopSimulation();

	// ─── State Query ───

	bool IsActive() const { return bIsActive; }
	bool IsSimulating() const { return bIsSimulating; }
	double GetElapsedSeconds() const;
	int32 GetTotalObjectCount() const;

	/** EN: Get status of all 17 systems / ES: Obtener estado de los 17 sistemas */
	TArray<FPGXHarnessSystemStatus> GetSystemStatuses() const;

	/** EN: Get coverage matrix for all PGX plugins  / ES: Obtener matriz de cobertura para todos los plugins PGX */
	static TArray<FPGXPluginCoverage> GetCoverageMatrix();

	/**
	 * EN: Get detailed coverage matrix via runtime presence checks.
	 *
	 *     Delegates to FPGXHarnessCoverage::GetCoverageMatrix(World), producing
	 *     one FPGXPluginCoverageEntry per canonical plugin with per-check results
	 *     (enabled + subsystem + Initialize). The curated compatibility
	 *     list returned by GetCoverageMatrix() is kept for back-compat with
	 *     SPGXSimHarnessTab.cpp + Tests/PGXSimHarnessAutomationTests.cpp.
	 *
	 * ES: Matriz detallada con checks de presencia runtime. Convive con
	 *     GetCoverageMatrix() (compatibility) para no romper callers.
	 */
	TArray<FPGXPluginCoverageEntry> GetDetailedCoverageMatrix() const;

	// ─── Panel Launcher ───

	void OpenAllPanels();
	void OpenPanel(FName TabId);
	static TArray<FName> GetAllPanelIds();
	static TArray<FName> GetUnitTestOnlyPanelIds();

	// ─── Quick Actions ───

	void GenerateLogEntries();
	void ForceGarbageCollection();
	void BroadcastTestMessage();
	void CycleGameFlowState();
	void CycleSaveSlot();
	void ExecuteRandomHandler();

	/** EN: Run comprehensive API verification across all 17 subsystems / ES: Ejecutar verificacion comprensiva de APIs en los 17 subsistemas */
	void VerifyAllAPIs();

	/** EN: Get last verification result / ES: Obtener ultimo resultado de verificacion */
	bool GetLastVerificationResult() const { return bLastVerificationPassed; }

	/** EN: Export execution report as .md to Saved/PGX/ / ES: Exportar reporte de ejecucion como .md a Saved/PGX/ */
	FString ExportReport() const;

	/** EN: Get the full action log / ES: Obtener el log completo de acciones */
	const TArray<FPGXHarnessActionEntry>& GetActionLog() const { return ActionLog; }

private:
	// ─── Per-System Injection (14 systems) ───

	void InjectProfile();
	void InjectConstruction();
	void InjectGameFlow(UWorld* World);
	void InjectLog(UWorld* World);
	void InjectSave(UWorld* World);
	void InjectPSO(UWorld* World);
	void InjectMGOS();
	void InjectAudio(UWorld* World);
	void InjectDataRegistry(UWorld* World);
	void InjectMessage(UWorld* World);
	void InjectEventHandler(UWorld* World);
	void InjectLevelFlow(UWorld* World);
	void InjectLoading(UWorld* World);

	//  coverage — PGXInput harness integration
	void InjectInput(UWorld* World);

	//  runtime core — PGXSpawn harness integration
	void InjectSpawn(UWorld* World);
	void InjectAI(UWorld* World);
	void InjectAbility(UWorld* World);

	//  runtime extended — PGXCamera harness integration
	void InjectCamera(UWorld* World);
	void InjectInteraction(UWorld* World);
	void InjectInventory(UWorld* World);
	void InjectUI(UWorld* World);

	// ─── Per-System Teardown ───

	void TeardownProfile();
	void TeardownConstruction();
	void TeardownGameFlow(UWorld* World);
	void TeardownSave(UWorld* World);
	void TeardownPSO(UWorld* World);
	void TeardownMGOS();
	void TeardownAudio(UWorld* World);
	void TeardownDataRegistry(UWorld* World);
	void TeardownMessage(UWorld* World);
	void TeardownEventHandler(UWorld* World);
	void TeardownLevelFlow(UWorld* World);
	void TeardownLoading(UWorld* World);

	// Coverage
	void TeardownInput(UWorld* World);
	void TeardownSpawn(UWorld* World);
	void TeardownAI(UWorld* World);
	void TeardownAbility(UWorld* World);
	void TeardownCamera(UWorld* World);
	void TeardownInteraction(UWorld* World);
	void TeardownInventory(UWorld* World);
	void TeardownUI(UWorld* World);

	// ─── Simulation Tick ───

	bool OnSimulationTick(float DeltaTime);

	// ─── World Resolution ───

	UWorld* ResolveEditorWorld() const;

	// ─── Action Log ───

	/** EN: Record an action into the execution log / ES: Registrar una accion en el log de ejecucion */
	void RecordAction(const FString& Category, const FString& Action, bool bSuccess, const FString& Detail);

	/** EN: Chronological action log / ES: Log cronologico de acciones */
	TArray<FPGXHarnessActionEntry> ActionLog;

	/** EN: All VerifyAllAPIs() runs with per-system detail / ES: Todas las ejecuciones de VerifyAllAPIs() con detalle por sistema */
	TArray<FPGXVerificationRun> VerificationRuns;

	// ─── State ───

	TArray<TStrongObjectPtr<UObject>> CreatedObjects;
	TWeakObjectPtr<UWorld> CachedWorld;

	/** EN: Saved Construction class sources for restore / ES: Class sources de Construction guardados para restaurar */
	struct FSavedClassSources
	{
		EPGXClassSourceMode GameMode = EPGXClassSourceMode::Default;
		EPGXClassSourceMode PlayerController = EPGXClassSourceMode::Default;
		EPGXClassSourceMode GameState = EPGXClassSourceMode::Default;
		EPGXClassSourceMode PlayerState = EPGXClassSourceMode::Default;
		EPGXClassSourceMode Character = EPGXClassSourceMode::Default;
		EPGXClassSourceMode Pawn = EPGXClassSourceMode::Default;
		EPGXClassSourceMode HUD = EPGXClassSourceMode::Default;
		bool bSaved = false;
	};
	FSavedClassSources SavedSources;

	/** EN: Saved GameFlow tags per channel for restore / ES: Tags de GameFlow guardados por canal para restaurar */
	TArray<FGameplayTag> SavedFlowTags;

	/** EN: Message listener handles for cleanup / ES: Handles de listeners de Message para limpieza */
	TArray<FPGXMessageListenerHandle> MessageListenerHandles;

	/** EN: Registered event handler tags for cleanup / ES: Tags de event handler registrados para limpieza */
	TArray<FGameplayTag> RegisteredEventTags;

	/** EN: Registered database tags for cleanup / ES: Tags de databases registrados para limpieza */
	TArray<FGameplayTag> RegisteredDatabaseTags;

	/** EN: Save slots created for cleanup / ES: Slots de save creados para limpieza */
	struct FSavedSlotInfo { FGameplayTag ContextTag; FString SlotName; };
	TArray<FSavedSlotInfo> CreatedSaveSlots;

	/** EN: PSO contexts added for cleanup / ES: Contextos PSO agregados para limpieza */
	TArray<FGameplayTag> AddedPSOContexts;
	bool bPSORecordingStarted = false;

	/** EN: Registry items registered for cleanup / ES: Items de registry registrados para limpieza */
	TArray<TPair<FGameplayTag, FGameplayTag>> RegisteredRegistryItems;

	/** EN: Backup slot name for CopySlot cleanup / ES: Nombre de slot backup para limpieza CopySlot */
	FString BackupSlotName;

	/** EN: Saveable compatibility fixture ref for RegisterSaveable cleanup / ES: Ref del fixture de compatibilidad saveable para limpieza de RegisterSaveable */
	TWeakObjectPtr<UObject> SaveableFixtureRef;

	/** EN: AI agent/controller created by RuntimeCore harness / ES: Agente/controlador IA creado por harness RuntimeCore */
	int32 HarnessAIAgentId = 0;
	TWeakObjectPtr<AAIController> HarnessAIController;

	/** EN: Ability smoke actor/component created by RuntimeCore harness / ES: Actor/componente Ability creado por harness RuntimeCore */
	TWeakObjectPtr<AActor> HarnessAbilityActor;
	TWeakObjectPtr<UPGXAbilityComponent> HarnessAbilityComponent;

	/** EN: Interaction smoke actors/component created by RuntimeExtended harness / ES: Actores/componente Interaction creados por harness RuntimeExtended */
	TWeakObjectPtr<AActor> HarnessInteractionOwner;
	TWeakObjectPtr<AActor> HarnessInteractionTarget;
	TWeakObjectPtr<UPGXInteractionComponent> HarnessInteractionComponent;

	/** EN: Inventory smoke actor/component/definition created by RuntimeExtended harness / ES: Actor/componente/definicion Inventory creados por harness RuntimeExtended */
	TWeakObjectPtr<AActor> HarnessInventoryActor;
	TWeakObjectPtr<UPGXInventoryComponent> HarnessInventoryComponent;
	TWeakObjectPtr<UPGXItemDefinition> HarnessInventoryItemDefinition;

	/** EN: UI widget pool touched by RuntimeExtended harness / ES: Pool de widgets UI usado por harness RuntimeExtended */
	TWeakObjectPtr<UPGXWidgetPool> HarnessWidgetPool;

	/** EN: Per-system status tracking / ES: Seguimiento de estado por sistema */
	TArray<FPGXHarnessSystemStatus> SystemStatuses;

	/** EN: Verification tracking / ES: Seguimiento de verificacion */
	int32 VerificationPassCount = 0;
	bool bLastVerificationPassed = false;

	FTSTicker::FDelegateHandle SimulationTickerHandle;
	double SetupTimestamp = 0.0;
	float SimulationAccumulator = 0.0f;
	int32 SimulationTickCount = 0;

	bool bIsActive = false;
	bool bIsSimulating = false;
};
