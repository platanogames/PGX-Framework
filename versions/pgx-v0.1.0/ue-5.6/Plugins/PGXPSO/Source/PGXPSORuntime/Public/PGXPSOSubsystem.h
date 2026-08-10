// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXGameInstanceSubsystem.h"
#include "Messages/PGXMessage.h"
#include "PGXPSOTypes.h"
#include "PGXPSODelegates.h"
#include "GameplayTagContainer.h"
#include "Containers/Ticker.h"
#include "Async/TaskGraphInterfaces.h"
#include "PGXPSOSubsystem.generated.h"

class UPGXPSOWarmUpConfig;
class FVertexFactoryType;
struct FPSOPrecacheParams;

/**
 * EN: Central PSO warm-up subsystem.
 *     Discovers Config DAs via AssetRegistry, manages the warm-up pipeline
 *     (batched loading -> PrecachePSOs -> compilation tracking -> GC release),
 *     and provides recording for runtime capture workflows.
 *     One instance per GameInstance (UPGXGameInstanceSubsystem).
 * ES: Subsistema central de warm-up de PSO.
 *     Descubre Config DAs via AssetRegistry, gestiona el pipeline de warm-up
 *     (carga por lotes -> PrecachePSOs -> seguimiento de compilacion -> liberacion GC),
 *     y proporciona grabacion para flujos de captura runtime.
 *     Una instancia por GameInstance (UPGXGameInstanceSubsystem).
 */
UCLASS(BlueprintType)
class PGXPSORUNTIME_API UPGXPSOSubsystem : public UPGXGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ========================================================================
	// EN: Lifecycle
	// ES: Ciclo de vida
	// ========================================================================

	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	// ========================================================================
	// EN: Warm-Up Control
	// ES: Control de Warm-Up
	// ========================================================================

	/** EN: Request warm-up for entries matching ContextTag / ES: Solicitar warm-up para entradas que coincidan con ContextTag */
	bool RequestWarmUp(FGameplayTag ContextTag);

	/** EN: Request warm-up for all discovered configs / ES: Solicitar warm-up para todas las configs descubiertas */
	bool RequestWarmUpAll();

	/** EN: Pause active warm-up / ES: Pausar warm-up activo */
	void PauseWarmUp();

	/** EN: Resume paused warm-up / ES: Reanudar warm-up pausado */
	void ResumeWarmUp();

	/** EN: Cancel active warm-up / ES: Cancelar warm-up activo */
	void CancelWarmUp();

	/** EN: Get current warm-up state / ES: Obtener estado actual de warm-up */
	EPGXPSOWarmUpState GetWarmUpState() const { return CurrentState; }

	/** EN: Get current warm-up progress snapshot / ES: Obtener snapshot de progreso de warm-up actual */
	FPGXPSOWarmUpProgress GetWarmUpProgress() const { return CachedProgress; }

	/** EN: Add a context tag for filtering / ES: Agregar un tag de contexto para filtrado */
	void AddPSOContext(FGameplayTag ContextTag);

	/** EN: Remove a context tag / ES: Remover un tag de contexto */
	void RemovePSOContext(FGameplayTag ContextTag);

	/** EN: Get active context tags / ES: Obtener tags de contexto activos */
	TArray<FGameplayTag> GetActiveContexts() const;

	/** EN: Get number of discovered configs / ES: Obtener numero de configs descubiertas */
	int32 GetDiscoveredConfigCount() const { return DiscoveredConfigs.Num(); }

	/** EN: Save PSO cache to disk / ES: Guardar cache PSO a disco */
	void SaveCacheToDisk();

	/** EN: Whether the cache has unsaved compiled PSOs / ES: Si el cache tiene PSOs compilados sin guardar */
	bool IsCacheDirty() const { return bCacheDirty; }

	// ========================================================================
	// EN: Recording API (Editor-only)
	// ES: API de Grabacion (Solo-editor)
	// ========================================================================

#if WITH_EDITOR
	/** EN: Start recording PSO entries during warm-up / ES: Comenzar a grabar entradas PSO durante warm-up */
	void StartRecording(const FString& SessionName);

	/** EN: Stop recording / ES: Detener grabacion */
	void StopRecording();

	/** EN: Whether recording is active / ES: Si la grabacion esta activa */
	bool IsRecording() const { return bIsRecording; }

	/** EN: Get recorded entries / ES: Obtener entradas grabadas */
	const TArray<FPGXPSORecordedEntry>& GetRecordedEntries() const { return RecordedEntries; }

	/** EN: Clear all recorded entries / ES: Limpiar todas las entradas grabadas */
	void ClearRecording();

	/** EN: Export recorded entries to JSON file / ES: Exportar entradas grabadas a archivo JSON */
	bool ExportRecordingToJson(const FString& FilePath);

	/** EN: Convert recorded entries to catalog entries for a new config / ES: Convertir entradas grabadas a entradas de catalogo para un nuevo config */
	TArray<FPGXPSOEntry> ConvertRecordingToCatalogEntries(FGameplayTag DefaultContext) const;

	/** EN: Validate a config's entries (material refs, VF types) / ES: Validar las entradas de un config (refs de material, tipos VF) */
	TArray<FString> ValidateConfig(const UPGXPSOWarmUpConfig* Config) const;

	/** EN: Inject a test config into discovered configs (test harness) / ES: Inyectar un config de test en configs descubiertos (harness de test) */
	void InjectTestConfig(UPGXPSOWarmUpConfig* Config);

	/** EN: Remove all transient (test-injected) configs / ES: Remover todos los configs transient (inyectados por test) */
	void ClearTestConfigs();
#endif

#if WITH_DEV_AUTOMATION_TESTS
	/** EN: Test harness only — inject transient config without touching project assets. */
	void InjectTestConfigForTesting(UPGXPSOWarmUpConfig* Config);

	/** EN: Test harness only — clear transient configs injected by automation. */
	void ClearTestConfigsForTesting();

	/** EN: Test harness only — rebind GameFlow bridge listener after injecting configs. */
	void RebindGameFlowBridgeForTesting();
#endif

	// ========================================================================
	// EN: Delegates — Dynamic (Blueprint)
	// ES: Delegados — Dinamicos (Blueprint)
	// ========================================================================

	UPROPERTY(BlueprintAssignable, Category = "PGX|PSO|Events")
	FOnPGXPSOWarmUpBegin OnWarmUpBegin;

	UPROPERTY(BlueprintAssignable, Category = "PGX|PSO|Events")
	FOnPGXPSOWarmUpProgress OnWarmUpProgressDelegate;

	UPROPERTY(BlueprintAssignable, Category = "PGX|PSO|Events")
	FOnPGXPSOWarmUpComplete OnWarmUpComplete;

	UPROPERTY(BlueprintAssignable, Category = "PGX|PSO|Events")
	FOnPGXPSOWarmUpFailed OnWarmUpFailed;

	UPROPERTY(BlueprintAssignable, Category = "PGX|PSO|Events")
	FOnPGXPSOEntryCompiled OnEntryCompiled;

	UPROPERTY(BlueprintAssignable, Category = "PGX|PSO|Events")
	FOnPGXPSORecordingUpdate OnRecordingUpdate;

	// ========================================================================
	// EN: Delegates — Native (C++ / Slate)
	// ES: Delegados — Nativos (C++ / Slate)
	// ========================================================================

	FOnPGXPSOWarmUpBeginNative OnWarmUpBeginNative;
	FOnPGXPSOWarmUpProgressNative OnWarmUpProgressNative;
	FOnPGXPSOWarmUpCompleteNative OnWarmUpCompleteNative;
	FOnPGXPSOStateChangedNative OnStateChangedNative;

private:
	// ========================================================================
	// EN: Profile Integration
	// ES: Integracion con Profile
	// ========================================================================

	void ApplyProfileConstraints(const struct FPGXResolvedProfile& Profile);
	void HandleProfileChanged(const struct FPGXResolvedProfile& OldProfile, const struct FPGXResolvedProfile& NewProfile);

	// ========================================================================
	// EN: State
	// ES: Estado
	// ========================================================================

	/** EN: Current warm-up pipeline state / ES: Estado actual del pipeline de warm-up */
	EPGXPSOWarmUpState CurrentState = EPGXPSOWarmUpState::Idle;

	/** EN: Discovered config DataAssets / ES: Config DataAssets descubiertos */
	UPROPERTY()
	TArray<TObjectPtr<UPGXPSOWarmUpConfig>> DiscoveredConfigs;

	/** EN: Set of already-submitted keys for deduplication / ES: Set de claves ya enviadas para deduplicacion */
	TSet<FPGXPSOKey> SubmittedKeys;

	/** EN: Entries pending batch submission / ES: Entradas pendientes de envio por lotes */
	TArray<FPGXPSOEntry> PendingBatchEntries;

	/** EN: Materials kept loaded during warm-up (prevent GC) / ES: Materiales mantenidos cargados durante warm-up (prevenir GC) */
	TArray<TStrongObjectPtr<UMaterialInterface>> LoadedMaterials;

	/** EN: Cached progress snapshot / ES: Snapshot de progreso en cache */
	FPGXPSOWarmUpProgress CachedProgress;

	/** EN: Active context tags / ES: Tags de contexto activos */
	TSet<FGameplayTag> ActiveContexts;

	/** EN: Whether compiled PSOs haven't been saved yet / ES: Si PSOs compilados aun no se han guardado */
	bool bCacheDirty = false;

	/** EN: Registered console commands / ES: Comandos de consola registrados */
	TArray<IConsoleObject*> RegisteredCommands;

	// ========================================================================
	// EN: Discovery
	// ES: Descubrimiento
	// ========================================================================

	/** EN: Scan AssetRegistry for UPGXPSOWarmUpConfig DAs / ES: Escanear AssetRegistry buscando DAs UPGXPSOWarmUpConfig */
	void DiscoverConfigs();

	/** EN: Collect entries from configs, filtering by context tag and deduplicating / ES: Recoger entradas de configs, filtrando por context tag y deduplicando */
	void MergeEntries(const FGameplayTag& ContextFilter, TArray<FPGXPSOEntry>& OutEntries);

	// ========================================================================
	// EN: Pipeline Core
	// ES: Pipeline Core
	// ========================================================================

	/** EN: Process a config's activation mode during init / ES: Procesar modo de activacion de un config durante init */
	void ProcessActivationMode(const UPGXPSOWarmUpConfig* Config);

	/** EN: Start or merge entries into the warm-up pipeline (handles concurrency policy) / ES: Iniciar o fusionar entradas en el pipeline de warm-up (maneja politica de concurrencia) */
	bool StartOrMergePipeline(const TArray<FPGXPSOEntry>& NewEntries, const UPGXPSOWarmUpConfig* SourceConfig);

	/** EN: Submit entries (all-at-once or batched) / ES: Enviar entradas (todas a la vez o por lotes) */
	void SubmitEntries(const TArray<FPGXPSOEntry>& EntriesToSubmit);

	/** EN: Submit a single entry to PrecachePSOs / ES: Enviar una sola entrada a PrecachePSOs */
	void SubmitSingleEntry(const FPGXPSOEntry& Entry);

	/** EN: Batch timer callback (FTSTicker) — returns true to keep ticking / ES: Callback del timer de lotes (FTSTicker) — retorna true para seguir */
	bool OnBatchTimerTick(float DeltaTime);

	/** EN: Progress check callback (FTSTicker, 100ms) — returns true to keep ticking / ES: Callback de chequeo de progreso (FTSTicker, 100ms) */
	bool OnProgressCheckTick(float DeltaTime);

	/** EN: Called when all PSO compilations are complete / ES: Llamado cuando todas las compilaciones PSO terminan */
	void OnAllCompilationsComplete();

	/** EN: Resolve enum + custom name to FVertexFactoryType* / ES: Resolver enum + nombre custom a FVertexFactoryType* */
	const FVertexFactoryType* ResolveVertexFactory(EPGXVertexFactoryType Type, FName CustomName) const;

	/** EN: Build FPSOPrecacheParams from a PSO entry / ES: Construir FPSOPrecacheParams desde una entrada PSO */
	FPSOPrecacheParams BuildPrecacheParams(const FPGXPSOEntry& Entry) const;

	/** EN: Reset all pipeline counters and state for a fresh warm-up / ES: Resetear todos los contadores y estado del pipeline para un warm-up nuevo */
	void ResetPipelineState();

	/** EN: Update CachedProgress and broadcast progress delegates / ES: Actualizar CachedProgress y emitir delegados de progreso */
	void UpdateProgress();

	// ========================================================================
	// EN: GameFlow Bridge Messaging
	// ES: Mensajeria de puente GameFlow
	// ========================================================================

	/** EN: Bind to GameFlow bridge messages for OnGameFlowTag configs / ES: Enlazar mensajes bridge de GameFlow para configs OnGameFlowTag */
	void BindToGameFlowBridgeMessages();

	/** EN: Unbind from GameFlow bridge messages / ES: Desenlazar mensajes bridge de GameFlow */
	void UnbindGameFlowBridgeMessages();

	/** EN: Handle GameFlow bridge state-change payloads / ES: Manejar payloads bridge de cambio de estado GameFlow */
	void OnGameFlowBridgeChanged(FGameplayTag Channel, const struct FPGXBridgeGameFlowChanged& Payload);

	// ========================================================================
	// EN: Loading Bridge Messaging
	// ES: Mensajeria de puente con Loading
	// ========================================================================

	void BindLoadingBridgeMessages();
	void UnbindLoadingBridgeMessages();
	void OnLoadingPSOQuery(FGameplayTag Channel, const FPGXMessage& Payload);
	void BroadcastLoadingBridgeState();
	void BroadcastLoadingBridgeProgress();
	void BroadcastLoadingBridgeComplete();

	// ========================================================================
	// EN: Console commands
	// ES: Comandos de consola
	// ========================================================================

	void RegisterConsoleCommands();
	void UnregisterConsoleCommands();

	// ========================================================================
	// EN: State Management
	// ES: Gestion de Estado
	// ========================================================================

	void SetState(EPGXPSOWarmUpState NewState);

	// ========================================================================
	// EN: Pipeline State
	// ES: Estado del Pipeline
	// ========================================================================

	/** EN: Time when the current warm-up started / ES: Tiempo cuando el warm-up actual inicio */
	double WarmUpStartTime = 0.0;

	/** EN: Graph events from PrecachePSOs for per-entry completion tracking / ES: Graph events de PrecachePSOs para tracking de completitud por entrada */
	TArray<FGraphEventRef> ActiveGraphEvents;

	/** EN: Total entries submitted to PrecachePSOs / ES: Total de entradas enviadas a PrecachePSOs */
	int32 TotalSubmitted = 0;

	/** EN: Batch ticker handle / ES: Handle del ticker de lotes */
	FTSTicker::FDelegateHandle BatchTickerHandle;

	/** EN: Progress check ticker handle / ES: Handle del ticker de chequeo de progreso */
	FTSTicker::FDelegateHandle ProgressTickerHandle;

	/** EN: Current batch size from active config (0 = all at once) / ES: Tamano de lote actual del config activo (0 = todas a la vez) */
	int32 CurrentBatchSize = 0;

	/** EN: Batch delay from active config / ES: Delay de lote del config activo */
	float CurrentBatchDelay = 0.016f;

	/** EN: Message listener handles for GameFlow bridge / ES: Handles de listeners del puente GameFlow */
	TArray<FPGXMessageListenerHandle> GameFlowBridgeMessageHandles;

	/** EN: Message listener handles for Loading bridge / ES: Handles de listeners del puente Loading */
	TArray<FPGXMessageListenerHandle> LoadingBridgeMessageHandles;

	/** EN: Current loaded material count (for memory management) / ES: Cuenta actual de materiales cargados (para gestion de memoria) */
	int32 CurrentLoadedMaterialCount = 0;

	/** EN: Peak loaded material count (metric) / ES: Pico de materiales cargados (metrica) */
	int32 PeakLoadedMaterialCount = 0;

	// ========================================================================
	// EN: Effective Profile Budgets (single source of truth)
	//     Resolved from Profile.PSOBudgets via RecomputeEffectiveBudgets() on Init
	//     and on profile change. 0 means "no clamp" (no platform-side limit
	//     declared); positive values are hard caps applied by RequestWarmUp/
	//     RequestWarmUpAll. This keeps one effective-cap rule for all request paths.
	// ES: Presupuestos efectivos del Profile (fuente unica de verdad). 0 = sin
	//     clamp; positivos son tope duro aplicado por RequestWarmUp/All.
	// ========================================================================

	/** EN: Effective max warm-up entries (clamps RequestWarmUp/All; 0 = unbounded). */
	int32 EffectiveMaxEntries = 0;

	/** EN: Effective time budget (ms) for warm-up; 0 = unbounded. Telemetry/observability for now. */
	float EffectiveTimeBudgetMs = 0.f;

	/** EN: Effective max shader permutations; 0 = unbounded. Telemetry/observability for now. */
	int32 EffectiveMaxShaders = 0;

	/**
	 * EN: Recompute EffectiveMaxEntries / EffectiveTimeBudgetMs / EffectiveMaxShaders from
	 *     the active PlatformConfig.PSOBudgets. Single source of truth for clamps.
	 *     Called on Initialize after profile setup, on ApplyProfileConstraints, and on
	 *     HandleProfileChanged. Logs transitions when values change so production
	 *     divergence stays visible.
	 * ES: Recomputar los presupuestos efectivos desde PlatformConfig.PSOBudgets activo.
	 *     Llamado en Initialize, ApplyProfileConstraints y HandleProfileChanged.
	 */
	void RecomputeEffectiveBudgets();

	/** EN: Max simultaneous loads from active config / ES: Max cargas simultaneas del config activo */
	int32 MaxSimultaneousLoads = 20;

	/** EN: Active config driving current warm-up settings / ES: Config activo que controla las settings del warm-up actual */
	const UPGXPSOWarmUpConfig* ActiveWarmUpConfig = nullptr;

	/** EN: Whether PSO listens to GameFlow bridge messages / ES: Si PSO escucha mensajes bridge de GameFlow */
	bool bGameFlowBridgeBound = false;

	// ========================================================================
	// EN: Recording State (Editor-only)
	// ES: Estado de Grabacion (Solo-editor)
	// ========================================================================

#if WITH_EDITORONLY_DATA
	/** EN: Whether recording is active / ES: Si la grabacion esta activa */
	bool bIsRecording = false;

	/** EN: Name of the current recording session / ES: Nombre de la sesion de grabacion actual */
	FString RecordingSessionName;

	/** EN: Time when recording started / ES: Tiempo cuando inicio la grabacion */
	double RecordingStartTime = 0.0;

	/** EN: Recorded entries / ES: Entradas grabadas */
	TArray<FPGXPSORecordedEntry> RecordedEntries;
#endif
};
