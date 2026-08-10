// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXPSOSubsystem.h"
#include "PGXPSOWarmUpConfig.h"
#include "PGXPSOSettings.h"
#include "Utils/PGXConfigResolution.h"
#include "Tags/PGXPSOTags.h"
#include "Logging/PGXLogCategories.h"
#include "Logging/PGXLogMacros.h"
#include "Base/PGXBaseMessaging.h"
#include "Messages/PGXBridgeMessages.h"
#include "Messages/PGXMessage.h"
#include "Messages/Tags/PGXBridgeTags.h"

// EN: UE Engine headers for PSO pipeline
// ES: Headers del motor UE para pipeline PSO
#include "Materials/MaterialInterface.h"
#include "VertexFactory.h"
#include "PSOPrecache.h"
#include "ShaderPipelineCache.h"
#include "AssetRegistry/AssetRegistryModule.h"

#include "Trace/PGXTraceHelper.h"
#include "Trace/PGXTraceTags.h"
#include "Profile/PGXProfileSubsystem.h"
#include "Profile/PGXPlatformConfig.h"

// EN: JSON support for recording export
// ES: JSON para exportacion de grabacion
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"

namespace PGXPSOLoadingBridge
{
	static FGameplayTag GetStateTag()
	{
		return FGameplayTag::RequestGameplayTag(FName(TEXT("PGX.Loading.PSO.State")), false);
	}

	static FGameplayTag GetProgressTag()
	{
		return FGameplayTag::RequestGameplayTag(FName(TEXT("PGX.Loading.PSO.Progress")), false);
	}

	static FGameplayTag GetCompleteTag()
	{
		return FGameplayTag::RequestGameplayTag(FName(TEXT("PGX.Loading.PSO.Complete")), false);
	}

	static bool IsWarmUpActive(EPGXPSOWarmUpState State)
	{
		return State != EPGXPSOWarmUpState::Idle && State != EPGXPSOWarmUpState::Complete;
	}
}

// ============================================================================
// EN: Lifecycle
// ES: Ciclo de vida
// ============================================================================

void UPGXPSOSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// EN: Add Global context by default
	// ES: Agregar contexto Global por defecto
	ActiveContexts.Add(TAG_PGX_PSO_Context_Global);

	// EN: Bind Loading bridge query listener through PGX messages (no Loading dependency)
	// ES: Enlazar listener de consulta del puente Loading via mensajes PGX (sin dependencia Loading)
	BindLoadingBridgeMessages();

	// EN: Discover configs via AssetRegistry
	// ES: Descubrir configs via AssetRegistry
	DiscoverConfigs();

	// EN: Bind to GameFlow if any config uses OnGameFlowTag
	// ES: Enlazar a GameFlow si algun config usa OnGameFlowTag
	bool bNeedGameFlowBinding = false;
	for (const TObjectPtr<UPGXPSOWarmUpConfig>& Config : DiscoveredConfigs)
	{
		if (Config && Config->ActivationMode == EPGXPSOActivationMode::OnGameFlowTag)
		{
			bNeedGameFlowBinding = true;
			break;
		}
	}

	if (bNeedGameFlowBinding)
	{
		BindToGameFlowBridgeMessages();
	}

	// EN: Process each config's activation mode
	// ES: Procesar el modo de activacion de cada config
	for (const TObjectPtr<UPGXPSOWarmUpConfig>& Config : DiscoveredConfigs)
	{
		if (Config)
		{
			ProcessActivationMode(Config);
		}
	}

	// EN: Register console commands for runtime control
	// ES: Registrar comandos de consola para control en runtime
	RegisterConsoleCommands();

	// EN: Register trace config from project settings
	// ES: Registrar config de traza desde project settings
	{
		const UPGXPSOSettings* Settings = GetDefault<UPGXPSOSettings>();
		FPGXTraceConfig PSOTraceConfig;
		if (Settings)
		{
			PSOTraceConfig = Settings->TraceConfig;
		}
		FPGXTraceHelper::RegisterSystemTraceConfig(TAG_PGX_System_PSO, PSOTraceConfig);
	}

	// EN: Apply project profile constraints if available
	// ES: Aplicar restricciones del profile de proyecto si esta disponible
	if (auto* ProfileSS = GetGameInstance()->GetSubsystem<UPGXProfileSubsystem>())
	{
		if (ProfileSS->IsProfileResolved())
		{
			ApplyProfileConstraints(ProfileSS->GetResolvedProfile());
		}
		ProfileSS->OnProfileChangedNative.AddUObject(this, &ThisClass::HandleProfileChanged);
	}

	// EN: Compute Effective budgets after Profile setup (single source of truth).
	// ES: Computar presupuestos efectivos tras setup de Profile (fuente unica de verdad).
	RecomputeEffectiveBudgets();

	PGX_LOG_INFO(LogPGXPSO, TEXT("UPGXPSOSubsystem: Initialized. Discovered %d configs. Active contexts: %d. Effective budgets: MaxEntries=%d TimeBudgetMs=%.1f MaxShaders=%d"),
		DiscoveredConfigs.Num(), ActiveContexts.Num(),
		EffectiveMaxEntries, EffectiveTimeBudgetMs, EffectiveMaxShaders);
}

void UPGXPSOSubsystem::Deinitialize()
{
	// EN: Cleanup Profile delegate subscription / ES: Limpiar suscripcion a delegate de Profile
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UPGXProfileSubsystem* Profile = GI->GetSubsystem<UPGXProfileSubsystem>())
		{
			Profile->OnProfileChangedNative.RemoveAll(this);
		}
	}

	// EN: Stop any active warm-up
	// ES: Detener cualquier warm-up activo
	if (CurrentState != EPGXPSOWarmUpState::Idle && CurrentState != EPGXPSOWarmUpState::Complete)
	{
		CancelWarmUp();
	}

	// EN: Unbind GameFlow bridge message listeners
	// ES: Desenlazar listeners de mensajes del puente GameFlow
	UnbindGameFlowBridgeMessages();

	// EN: Unbind Loading bridge message listeners
	// ES: Desenlazar listeners de mensajes del puente Loading
	UnbindLoadingBridgeMessages();

	// EN: Auto-save cache on shutdown if configured
	// ES: Auto-guardar cache al cerrar si esta configurado
	const UPGXPSOSettings* Settings = GetDefault<UPGXPSOSettings>();
	if (Settings && Settings->bAutoSaveCacheOnShutdown && bCacheDirty)
	{
		SaveCacheToDisk();
	}

	// EN: Unregister console commands
	// ES: Desregistrar comandos de consola
	UnregisterConsoleCommands();
	FPGXTraceHelper::UnregisterSystemTraceConfig(TAG_PGX_System_PSO);

	// EN: Clear state
	// ES: Limpiar estado
	LoadedMaterials.Empty();
	DiscoveredConfigs.Empty();
	SubmittedKeys.Empty();
	PendingBatchEntries.Empty();
	ActiveContexts.Empty();
	ActiveGraphEvents.Empty();
	ActiveWarmUpConfig = nullptr;

	PGX_LOG_INFO(LogPGXPSO, TEXT("UPGXPSOSubsystem: Deinitialized"));

	Super::Deinitialize();
}

void UPGXPSOSubsystem::BindLoadingBridgeMessages()
{
	UnbindLoadingBridgeMessages();

	TWeakObjectPtr<ThisClass> WeakThis(this);
	LoadingBridgeMessageHandles.Add(PGXBaseMessaging::Listen<FPGXMessage>(
		this,
		TAG_PGX_PSO_Loading_QueryState.GetTag(),
		[WeakThis](FGameplayTag Channel, const FPGXMessage& Payload)
		{
			if (ThisClass* StrongThis = WeakThis.Get())
			{
				StrongThis->OnLoadingPSOQuery(Channel, Payload);
			}
		}));
}

void UPGXPSOSubsystem::UnbindLoadingBridgeMessages()
{
	PGXBaseMessaging::UnregisterAll(LoadingBridgeMessageHandles);
}

void UPGXPSOSubsystem::OnLoadingPSOQuery(FGameplayTag /*Channel*/, const FPGXMessage& /*Payload*/)
{
	BroadcastLoadingBridgeState();
}

void UPGXPSOSubsystem::BroadcastLoadingBridgeState()
{
	const FGameplayTag StateTag = PGXPSOLoadingBridge::GetStateTag();
	if (!StateTag.IsValid())
	{
		return;
	}

	FPGXBridgeLoadingState State;
	State.bIsLoading = PGXPSOLoadingBridge::IsWarmUpActive(CurrentState);
	State.Progress = CurrentState == EPGXPSOWarmUpState::Complete
		? 1.0f
		: FMath::Clamp(CachedProgress.PercentComplete, 0.0f, 1.0f);
	State.Timestamp = FPlatformTime::Seconds();

	PGXBaseMessaging::Broadcast<FPGXBridgeLoadingState>(this, StateTag, State);
}

void UPGXPSOSubsystem::BroadcastLoadingBridgeProgress()
{
	const FGameplayTag ProgressTag = PGXPSOLoadingBridge::GetProgressTag();
	if (!ProgressTag.IsValid())
	{
		return;
	}

	FPGXBridgeLoadingState Progress;
	Progress.bIsLoading = true;
	Progress.Progress = FMath::Clamp(CachedProgress.PercentComplete, 0.0f, 1.0f);
	Progress.Timestamp = FPlatformTime::Seconds();

	PGXBaseMessaging::Broadcast<FPGXBridgeLoadingState>(this, ProgressTag, Progress);
}

void UPGXPSOSubsystem::BroadcastLoadingBridgeComplete()
{
	const FGameplayTag CompleteTag = PGXPSOLoadingBridge::GetCompleteTag();
	if (!CompleteTag.IsValid())
	{
		return;
	}

	FPGXBridgeLoadingState Complete;
	Complete.bIsLoading = false;
	Complete.Progress = 1.0f;
	Complete.Timestamp = FPlatformTime::Seconds();

	PGXBaseMessaging::Broadcast<FPGXBridgeLoadingState>(this, CompleteTag, Complete);
}

// ============================================================================
// EN: Public API — Warm-Up Control
// ES: API Publica — Control de Warm-Up
// ============================================================================

bool UPGXPSOSubsystem::RequestWarmUp(FGameplayTag ContextTag)
{
	if (!ContextTag.IsValid())
	{
		PGX_LOG_WARNING(LogPGXPSO, TEXT("UPGXPSOSubsystem::RequestWarmUp — Invalid ContextTag"));
		return false;
	}

	// EN: Merge entries filtered by context tag
	// ES: Fusionar entradas filtradas por context tag
	TArray<FPGXPSOEntry> FilteredEntries;
	MergeEntries(ContextTag, FilteredEntries);

	if (FilteredEntries.IsEmpty())
	{
		PGX_LOG_WARNING(LogPGXPSO, TEXT("UPGXPSOSubsystem::RequestWarmUp — No entries found for context '%s'"),
			*ContextTag.ToString());
		return false;
	}

	// EN: Apply Profile-derived hard cap on entry count. EffectiveMaxEntries==0 means
	//     unbounded (no platform-side clamp). Positive values truncate the merged entries with
	//     a warning so production divergence stays visible.
	// ES: Aplicar tope duro por entry count derivado del Profile. 0 = sin clamp.
	if (EffectiveMaxEntries > 0 && FilteredEntries.Num() > EffectiveMaxEntries)
	{
		PGX_LOG_WARNING(LogPGXPSO,
			TEXT("UPGXPSOSubsystem::RequestWarmUp — Profile clamp: %d entries truncated to EffectiveMaxEntries=%d (context '%s')"),
			FilteredEntries.Num(), EffectiveMaxEntries, *ContextTag.ToString());
		FilteredEntries.SetNum(EffectiveMaxEntries);
	}

	PGX_LOG_INFO(LogPGXPSO, TEXT("UPGXPSOSubsystem::RequestWarmUp — Context: %s, %d entries"),
		*ContextTag.ToString(), FilteredEntries.Num());

	// EN: Find a config that has this context to use its settings
	// ES: Buscar un config que tenga este contexto para usar sus settings
	const UPGXPSOWarmUpConfig* SourceConfig = nullptr;
	for (const TObjectPtr<UPGXPSOWarmUpConfig>& Config : DiscoveredConfigs)
	{
		if (!Config) continue;
		for (const FPGXPSOEntry& Entry : Config->Entries)
		{
			if (Entry.ContextTag == ContextTag)
			{
				SourceConfig = Config;
				break;
			}
		}
		if (SourceConfig) break;
	}

	return StartOrMergePipeline(FilteredEntries, SourceConfig);
}

bool UPGXPSOSubsystem::RequestWarmUpAll()
{
	// EN: Merge all entries (no context filter)
	// ES: Fusionar todas las entradas (sin filtro de contexto)
	TArray<FPGXPSOEntry> AllEntries;
	MergeEntries(FGameplayTag(), AllEntries);

	if (AllEntries.IsEmpty())
	{
		PGX_LOG_WARNING(LogPGXPSO, TEXT("UPGXPSOSubsystem::RequestWarmUpAll — No entries found"));
		return false;
	}

	// EN: Apply Profile-derived hard cap on entry count. EffectiveMaxEntries==0 means
	//     unbounded.
	// ES: Aplicar tope duro por entry count derivado del Profile. 0 = sin clamp.
	if (EffectiveMaxEntries > 0 && AllEntries.Num() > EffectiveMaxEntries)
	{
		PGX_LOG_WARNING(LogPGXPSO,
			TEXT("UPGXPSOSubsystem::RequestWarmUpAll — Profile clamp: %d entries truncated to EffectiveMaxEntries=%d"),
			AllEntries.Num(), EffectiveMaxEntries);
		AllEntries.SetNum(EffectiveMaxEntries);
	}

	PGX_LOG_INFO(LogPGXPSO, TEXT("UPGXPSOSubsystem::RequestWarmUpAll — %d entries"), AllEntries.Num());

	// EN: Use first discovered config for settings (or nullptr for defaults)
	// ES: Usar primer config descubierto para settings (o nullptr para defaults)
	const UPGXPSOWarmUpConfig* SourceConfig = DiscoveredConfigs.IsEmpty() ? nullptr : DiscoveredConfigs[0];
	return StartOrMergePipeline(AllEntries, SourceConfig);
}

void UPGXPSOSubsystem::PauseWarmUp()
{
	if (CurrentState != EPGXPSOWarmUpState::Loading && CurrentState != EPGXPSOWarmUpState::Compiling)
	{
		PGX_LOG_WARNING(LogPGXPSO, TEXT("UPGXPSOSubsystem::PauseWarmUp — Cannot pause in state %d"),
			static_cast<int32>(CurrentState));
		return;
	}

	// EN: Pause batch timer
	// ES: Pausar timer de lotes
	if (BatchTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(BatchTickerHandle);
		BatchTickerHandle.Reset();
	}

	// EN: Pause progress check timer
	// ES: Pausar timer de chequeo de progreso
	if (ProgressTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(ProgressTickerHandle);
		ProgressTickerHandle.Reset();
	}

	// EN: Pause UE shader pipeline batching
	// ES: Pausar batching del pipeline de shaders de UE
	FShaderPipelineCache::PauseBatching();

	SetState(EPGXPSOWarmUpState::Paused);
	PGX_LOG_INFO(LogPGXPSO, TEXT("UPGXPSOSubsystem: Warm-up paused"));
}

void UPGXPSOSubsystem::ResumeWarmUp()
{
	if (CurrentState != EPGXPSOWarmUpState::Paused)
	{
		PGX_LOG_WARNING(LogPGXPSO, TEXT("UPGXPSOSubsystem::ResumeWarmUp — Not paused (state: %d)"),
			static_cast<int32>(CurrentState));
		return;
	}

	// EN: Resume UE shader pipeline batching
	// ES: Reanudar batching del pipeline de shaders de UE
	FShaderPipelineCache::ResumeBatching();

	// EN: If there are pending batch entries, restart batch timer
	// ES: Si hay entradas de lote pendientes, reiniciar timer de lotes
	if (!PendingBatchEntries.IsEmpty() && CurrentBatchSize > 0)
	{
		SetState(EPGXPSOWarmUpState::Loading);
		BatchTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateUObject(this, &UPGXPSOSubsystem::OnBatchTimerTick),
			CurrentBatchDelay);
	}
	else
	{
		// EN: Resume in Compiling state — restart progress check
		// ES: Reanudar en estado Compiling — reiniciar chequeo de progreso
		SetState(EPGXPSOWarmUpState::Compiling);
	}

	// EN: Restart progress check timer
	// ES: Reiniciar timer de chequeo de progreso
	ProgressTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UPGXPSOSubsystem::OnProgressCheckTick),
		0.1f);

	PGX_LOG_INFO(LogPGXPSO, TEXT("UPGXPSOSubsystem: Warm-up resumed"));
}

void UPGXPSOSubsystem::CancelWarmUp()
{
	if (CurrentState == EPGXPSOWarmUpState::Idle)
	{
		return;
	}

	PGX_LOG_INFO(LogPGXPSO, TEXT("UPGXPSOSubsystem: Cancelling warm-up (state: %d, submitted: %d)"),
		static_cast<int32>(CurrentState), TotalSubmitted);

	// EN: Stop timers
	// ES: Detener timers
	if (BatchTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(BatchTickerHandle);
		BatchTickerHandle.Reset();
	}
	if (ProgressTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(ProgressTickerHandle);
		ProgressTickerHandle.Reset();
	}

	// EN: Resume batching if we had paused it
	// ES: Reanudar batching si lo habiamos pausado
	if (FShaderPipelineCache::IsBatchingPaused())
	{
		FShaderPipelineCache::ResumeBatching();
	}

	// EN: Clear pending entries (already-submitted compilations will finish harmlessly)
	// ES: Limpiar entradas pendientes (compilaciones ya enviadas terminaran sin problemas)
	PendingBatchEntries.Empty();
	ActiveGraphEvents.Empty();

	// EN: Release loaded materials (allow GC)
	// ES: Liberar materiales cargados (permitir GC)
	LoadedMaterials.Empty();
	CurrentLoadedMaterialCount = 0;

	// EN: Reset state
	// ES: Resetear estado
	SetState(EPGXPSOWarmUpState::Idle);
	ActiveWarmUpConfig = nullptr;
}

void UPGXPSOSubsystem::AddPSOContext(FGameplayTag ContextTag)
{
	if (ContextTag.IsValid())
	{
		ActiveContexts.Add(ContextTag);
		PGX_LOG_VERBOSE(LogPGXPSO, TEXT("UPGXPSOSubsystem: Added context %s"), *ContextTag.ToString());
	}
}

void UPGXPSOSubsystem::RemovePSOContext(FGameplayTag ContextTag)
{
	// EN: Guard: cannot remove Global context
	// ES: Guard: no se puede remover el contexto Global
	if (ContextTag == TAG_PGX_PSO_Context_Global)
	{
		PGX_LOG_WARNING(LogPGXPSO, TEXT("UPGXPSOSubsystem: Cannot remove Global context"));
		return;
	}

	if (ContextTag.IsValid())
	{
		ActiveContexts.Remove(ContextTag);
		PGX_LOG_VERBOSE(LogPGXPSO, TEXT("UPGXPSOSubsystem: Removed context %s"), *ContextTag.ToString());
	}
}

TArray<FGameplayTag> UPGXPSOSubsystem::GetActiveContexts() const
{
	return ActiveContexts.Array();
}

void UPGXPSOSubsystem::SaveCacheToDisk()
{
	FShaderPipelineCache::SavePipelineFileCache(FPipelineFileCacheManager::SaveMode::Incremental);
	bCacheDirty = false;
	PGX_LOG_INFO(LogPGXPSO, TEXT("UPGXPSOSubsystem: PSO cache saved to disk (incremental)"));
}

// ============================================================================
// EN: Discovery
// ES: Descubrimiento
// ============================================================================

void UPGXPSOSubsystem::DiscoverConfigs()
{
	const UPGXPSOSettings* Settings = GetDefault<UPGXPSOSettings>();

	if (!Settings->PSOConfigTable.IsNull())
	{
		// EN: Load from DataTable (deterministic)
		// ES: Cargar desde DataTable (deterministico)
		UDataTable* Table = Settings->PSOConfigTable.LoadSynchronous();
		if (IsValid(Table))
		{
			TArray<FPGXPSOConfigRow*> Rows;
			Table->GetAllRows<FPGXPSOConfigRow>(TEXT("PSODiscovery"), Rows);

			for (const FPGXPSOConfigRow* Row : Rows)
			{
				if (!Row || Row->ConfigRef.IsNull()) { continue; }
				UPGXPSOWarmUpConfig* Config = Row->ConfigRef.LoadSynchronous();
				if (!IsValid(Config) || DiscoveredConfigs.Contains(Config)) { continue; }

				DiscoveredConfigs.Add(Config);
				PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO Discovery] Config from DataTable: '%s' — %d entries, activation: %d"),
					*Config->GetName(),
					Config->Entries.Num(),
					static_cast<int32>(Config->ActivationMode));
			}

			PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO] %d configs resolved from DataTable."), DiscoveredConfigs.Num());
			return;
		}
		PGX_LOG_WARNING(LogPGXPSO, TEXT("[PSO] DataTable assigned in Settings but failed to load: %s"),
			*Settings->PSOConfigTable.ToString());
	}

	// EN: AssetRegistry fallback (deprecated)
	// ES: Fallback AssetRegistry (deprecated)
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	const IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	FARFilter Filter;
	Filter.ClassPaths.Add(UPGXPSOWarmUpConfig::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true;

	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssets(Filter, AssetDataList);

	for (const FAssetData& AssetData : AssetDataList)
	{
		UPGXPSOWarmUpConfig* Config = Cast<UPGXPSOWarmUpConfig>(AssetData.GetAsset());
		if (!Config)
		{
			PGX_LOG_WARNING(LogPGXPSO, TEXT("[PSO Discovery] Failed to load WarmUpConfig: %s"),
				*AssetData.GetObjectPathString());
			continue;
		}

		if (DiscoveredConfigs.Contains(Config))
		{
			continue;
		}

		DiscoveredConfigs.Add(Config);

		PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO Discovery] Registered config '%s' — %d entries, activation: %d"),
			*Config->GetName(),
			Config->Entries.Num(),
			static_cast<int32>(Config->ActivationMode));
	}

	if (DiscoveredConfigs.Num() > 0)
	{
		PGX_LOG_WARNING(LogPGXPSO, TEXT("[PSO] %d configs auto-discovered from AssetRegistry. "
			"Configure a DataTable in Project Settings > PGX > PSO System to remove this warning. "
			"Auto-discovery is deprecated and will be removed in v0.6.0."),
			DiscoveredConfigs.Num());
	}
}

void UPGXPSOSubsystem::MergeEntries(const FGameplayTag& ContextFilter, TArray<FPGXPSOEntry>& OutEntries)
{
	// EN: Temporary dedup set (local to this merge, doesn't affect SubmittedKeys)
	// ES: Set de dedup temporal (local a esta fusion, no afecta SubmittedKeys)
	TSet<FPGXPSOKey> LocalDedup;

	for (const TObjectPtr<UPGXPSOWarmUpConfig>& Config : DiscoveredConfigs)
	{
		if (!Config) continue;

		for (const FPGXPSOEntry& Entry : Config->Entries)
		{
			// EN: Context filter: if provided, entry must match. If not provided, accept all.
			//     Entries with no ContextTag are accepted if they match any active context or no filter is set.
			// ES: Filtro de contexto: si se proporciona, la entrada debe coincidir. Si no, aceptar todo.
			if (ContextFilter.IsValid())
			{
				if (Entry.ContextTag.IsValid() && Entry.ContextTag != ContextFilter)
				{
					continue;
				}
			}

			// EN: Deduplicate by FPGXPSOKey
			// ES: Deduplicar por FPGXPSOKey
			const FPGXPSOKey Key = Entry.MakeKey();
			if (LocalDedup.Contains(Key))
			{
				continue;
			}
			LocalDedup.Add(Key);

			OutEntries.Add(Entry);
		}
	}
}

// ============================================================================
// EN: Pipeline Core
// ES: Pipeline Core
// ============================================================================

void UPGXPSOSubsystem::ProcessActivationMode(const UPGXPSOWarmUpConfig* Config)
{
	if (!Config) return;

	switch (Config->ActivationMode)
	{
	case EPGXPSOActivationMode::OnSubsystemInit:
	{
		// EN: Collect this config's entries and start warm-up
		// ES: Recoger las entradas de este config e iniciar warm-up
		if (Config->Entries.Num() > 0)
		{
			StartOrMergePipeline(Config->Entries, Config);
		}
		break;
	}
	case EPGXPSOActivationMode::OnGameFlowTag:
		// EN: Handled via GameFlow bridge message binding (done in Initialize)
		// ES: Manejado via enlace al mensaje bridge de GameFlow (hecho en Initialize)
		break;
	case EPGXPSOActivationMode::OnLevelLoad:
		// EN: OnLevelLoad not yet implemented — warn and skip warm-up for this config
		// ES: OnLevelLoad aun no implementado — advertir y omitir warm-up para esta config
		PGX_LOG_WARNING(LogPGXPSO, TEXT("[PSO] Config '%s' uses OnLevelLoad — mode not yet implemented, warm-up skipped"),
			*Config->GetName());
		break;
	case EPGXPSOActivationMode::OnExplicitCall:
		// EN: No-op — user will call RequestWarmUp() manually
		// ES: No-op — el usuario llamara RequestWarmUp() manualmente
		break;
	}
}

bool UPGXPSOSubsystem::StartOrMergePipeline(const TArray<FPGXPSOEntry>& NewEntries, const UPGXPSOWarmUpConfig* SourceConfig)
{
	if (NewEntries.IsEmpty())
	{
		return false;
	}

	// EN: Terminal states (Idle, Complete, Failed) → start fresh
	// ES: Estados terminales (Idle, Complete, Failed) → inicio limpio
	if (CurrentState == EPGXPSOWarmUpState::Idle
		|| CurrentState == EPGXPSOWarmUpState::Complete
		|| CurrentState == EPGXPSOWarmUpState::Failed)
	{
		ResetPipelineState();
		ActiveWarmUpConfig = SourceConfig;

		// EN: Configure batch parameters from source config
		// ES: Configurar parametros de lote desde config fuente
		CurrentBatchSize = SourceConfig ? SourceConfig->BatchSize : 0;
		CurrentBatchDelay = SourceConfig ? SourceConfig->BatchDelaySeconds : 0.016f;
		MaxSimultaneousLoads = SourceConfig ? SourceConfig->MaxSimultaneousLoads : 20;

		// EN: Track total entries for progress
		// ES: Rastrear total de entradas para progreso
		CachedProgress.TotalEntries = NewEntries.Num();

		// EN: Transition to Loading state
		// ES: Transicionar a estado Loading
		WarmUpStartTime = FPlatformTime::Seconds();
		SetState(EPGXPSOWarmUpState::Loading);
		OnWarmUpBegin.Broadcast();
		OnWarmUpBeginNative.Broadcast();

		// EN: Submit entries
		// ES: Enviar entradas
		SubmitEntries(NewEntries);
		return true;
	}

	// EN: Active state → apply concurrency policy
	// ES: Estado activo → aplicar politica de concurrencia
	const EPGXPSOConcurrencyPolicy Policy = ActiveWarmUpConfig
		? ActiveWarmUpConfig->ConcurrencyPolicy
		: EPGXPSOConcurrencyPolicy::MergeAndContinue;

	switch (Policy)
	{
	case EPGXPSOConcurrencyPolicy::RejectNew:
	{
		PGX_LOG_WARNING(LogPGXPSO, TEXT("UPGXPSOSubsystem: Warm-up request rejected (policy: RejectNew, state: %d)"),
			static_cast<int32>(CurrentState));
		OnWarmUpFailed.Broadcast(TEXT("Warm-up request rejected: already active (RejectNew policy)"));
		return false;
	}
	case EPGXPSOConcurrencyPolicy::CancelAndRestart:
	{
		PGX_LOG_INFO(LogPGXPSO, TEXT("UPGXPSOSubsystem: Cancelling current warm-up to restart (CancelAndRestart policy)"));
		CancelWarmUp();
		// EN: Recurse after cancel (now in Idle state)
		// ES: Recursion tras cancelar (ahora en estado Idle)
		return StartOrMergePipeline(NewEntries, SourceConfig);
	}
	case EPGXPSOConcurrencyPolicy::MergeAndContinue:
	{
		// EN: Append new entries to pending batch
		// ES: Agregar nuevas entradas al lote pendiente
		CachedProgress.TotalEntries += NewEntries.Num();
		PendingBatchEntries.Append(NewEntries);

		if (CurrentState == EPGXPSOWarmUpState::Paused)
		{
			PGX_LOG_INFO(LogPGXPSO, TEXT("UPGXPSOSubsystem: Merged %d entries (paused — will process on resume)"),
				NewEntries.Num());
		}
		else
		{
			PGX_LOG_INFO(LogPGXPSO, TEXT("UPGXPSOSubsystem: Merged %d entries into active pipeline"),
				NewEntries.Num());

			// EN: If not batching, submit immediately
			// ES: Si no hay batching, enviar inmediatamente
			if (CurrentBatchSize == 0)
			{
				SubmitEntries(NewEntries);
			}
			// EN: If batching, entries already in PendingBatchEntries, timer will pick them up
			// ES: Si hay batching, las entradas ya estan en PendingBatchEntries, el timer las procesara
		}
		return true;
	}
	}

	return false;
}

void UPGXPSOSubsystem::SubmitEntries(const TArray<FPGXPSOEntry>& EntriesToSubmit)
{
	if (CurrentBatchSize == 0)
	{
		// EN: All at once — submit every entry immediately
		// ES: Todas a la vez — enviar cada entrada inmediatamente
		for (const FPGXPSOEntry& Entry : EntriesToSubmit)
		{
			SubmitSingleEntry(Entry);
		}

		// EN: All submitted — transition to Compiling and start progress check
		// ES: Todas enviadas — transicionar a Compiling e iniciar chequeo de progreso
		if (TotalSubmitted > 0)
		{
			SetState(EPGXPSOWarmUpState::Compiling);
			ProgressTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
				FTickerDelegate::CreateUObject(this, &UPGXPSOSubsystem::OnProgressCheckTick),
				0.1f);
		}
		else
		{
			// EN: No entries actually submitted (all deduplicated or failed)
			// ES: Ninguna entrada realmente enviada (todas deduplicadas o fallidas)
			OnAllCompilationsComplete();
		}
	}
	else
	{
		// EN: Batched — add to pending and start batch timer
		// ES: Por lotes — agregar a pendientes e iniciar timer de lotes
		PendingBatchEntries.Append(EntriesToSubmit);

		if (!BatchTickerHandle.IsValid())
		{
			BatchTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
				FTickerDelegate::CreateUObject(this, &UPGXPSOSubsystem::OnBatchTimerTick),
				CurrentBatchDelay);
		}
	}
}

void UPGXPSOSubsystem::SubmitSingleEntry(const FPGXPSOEntry& Entry)
{
	// EN: Skip non-Material entries; RawCache is not handled by this path.
	// ES: Omitir entradas no-Material (RawCache se maneja en fases futuras)
	if (Entry.EntryType != EPGXPSOEntryType::Material)
	{
		PGX_LOG_VERBOSE(LogPGXPSO, TEXT("UPGXPSOSubsystem: Skipping non-Material entry '%s' (type: RawCache)"),
			*Entry.Label);
		return;
	}

	// EN: Deduplicate via FPGXPSOKey
	// ES: Deduplicar via FPGXPSOKey
	const FPGXPSOKey Key = Entry.MakeKey();
	if (SubmittedKeys.Contains(Key))
	{
		CachedProgress.DeduplicatedEntries++;
		return;
	}
	SubmittedKeys.Add(Key);

	// EN: Load material synchronously
	// ES: Cargar material sincronicamente
	UMaterialInterface* Mat = Entry.Material.LoadSynchronous();
	if (!Mat)
	{
		PGX_LOG_WARNING(LogPGXPSO, TEXT("UPGXPSOSubsystem: Failed to load material '%s' for entry '%s' — skipped"),
			*Entry.Material.GetAssetName(), *Entry.Label);
		CachedProgress.FailedEntries++;
		return;
	}

	// EN: Keep reference to prevent GC during warm-up
	// ES: Mantener referencia para prevenir GC durante warm-up
	if (Entry.bKeepLoadedAfterPrecache)
	{
		LoadedMaterials.Emplace(Mat);
	}

	CurrentLoadedMaterialCount++;
	PeakLoadedMaterialCount = FMath::Max(PeakLoadedMaterialCount, CurrentLoadedMaterialCount);

	// EN: Resolve vertex factory type
	// ES: Resolver tipo de vertex factory
	const FVertexFactoryType* VF = ResolveVertexFactory(Entry.VertexFactory, Entry.CustomVertexFactoryName);
	if (!VF)
	{
		PGX_LOG_WARNING(LogPGXPSO, TEXT("UPGXPSOSubsystem: Could not resolve VF '%s' for entry '%s' — skipped"),
			*Entry.GetResolvedVertexFactoryName().ToString(), *Entry.Label);
		CachedProgress.FailedEntries++;
		CurrentLoadedMaterialCount--;
		return;
	}

	// EN: Build precache parameters from entry render settings
	// ES: Construir parametros de precache desde settings de render de la entrada
	FPSOPrecacheParams Params = BuildPrecacheParams(Entry);

	// EN: Submit to UE PrecachePSOs API
	// ES: Enviar a API PrecachePSOs de UE
	FGraphEventArray EntryEvents = Mat->PrecachePSOs(VF, Params);

	// EN: Track graph events for completion monitoring
	// ES: Rastrear graph events para monitoreo de completitud
	for (const FGraphEventRef& Event : EntryEvents)
	{
		if (Event.IsValid())
		{
			ActiveGraphEvents.Add(Event);
		}
	}

	TotalSubmitted++;
	CachedProgress.TotalPrecacheRequests++;

	// EN: Broadcast per-entry event
	// ES: Emitir evento por entrada
	OnEntryCompiled.Broadcast(Entry);

#if WITH_EDITORONLY_DATA
	// EN: Record entry if recording is active
	// ES: Grabar entrada si la grabacion esta activa
	if (bIsRecording)
	{
		FPGXPSORecordedEntry Recorded;
		Recorded.MaterialPath = Entry.Material.ToSoftObjectPath().ToString();
		Recorded.VertexFactoryName = Entry.GetResolvedVertexFactoryName();
		Recorded.MeshPassName = StaticEnum<EPGXPSOMeshPassHint>()->GetNameStringByValue(static_cast<int64>(Entry.PassHint));
		Recorded.Timestamp = FDateTime::Now();
		Recorded.FrameNumber = GFrameNumber;
		Recorded.PrecacheResult = FString::Printf(TEXT("%d events"), EntryEvents.Num());

		// EN: Compilation time measured as overhead of SubmitSingleEntry
		// ES: Tiempo de compilacion medido como overhead de SubmitSingleEntry
		Recorded.CompilationTimeMs = 0.0f; // EN: Actual time measured post-completion / ES: Tiempo real medido post-completitud

		const UPGXPSOSettings* Settings = GetDefault<UPGXPSOSettings>();
		if (Settings)
		{
			Recorded.bCausedHitch = (Recorded.CompilationTimeMs > Settings->HitchThresholdMs);
		}

		RecordedEntries.Add(Recorded);
		OnRecordingUpdate.Broadcast(Recorded);
	}
#endif

	PGX_LOG_VERBOSE(LogPGXPSO, TEXT("UPGXPSOSubsystem: Submitted '%s' (VF: %s, %d events)"),
		*Entry.Label,
		*Entry.GetResolvedVertexFactoryName().ToString(),
		EntryEvents.Num());
}

bool UPGXPSOSubsystem::OnBatchTimerTick(float /*DeltaTime*/)
{
	if (PendingBatchEntries.IsEmpty())
	{
		// EN: All batches submitted — stop batch timer, move to Compiling
		// ES: Todos los lotes enviados — detener timer de lotes, mover a Compiling
		BatchTickerHandle.Reset();

		if (TotalSubmitted > 0)
		{
			SetState(EPGXPSOWarmUpState::Compiling);

			// EN: Start progress check if not already running
			// ES: Iniciar chequeo de progreso si no esta corriendo
			if (!ProgressTickerHandle.IsValid())
			{
				ProgressTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
					FTickerDelegate::CreateUObject(this, &UPGXPSOSubsystem::OnProgressCheckTick),
					0.1f);
			}
		}
		else
		{
			OnAllCompilationsComplete();
		}

		return false; // EN: Remove ticker / ES: Remover ticker
	}

	// EN: Submit next batch
	// ES: Enviar siguiente lote
	const int32 Count = FMath::Min(CurrentBatchSize, PendingBatchEntries.Num());
	for (int32 i = 0; i < Count; ++i)
	{
		SubmitSingleEntry(PendingBatchEntries[i]);
	}

	// EN: Remove processed entries from pending
	// ES: Remover entradas procesadas de pendientes
	PendingBatchEntries.RemoveAt(0, Count);

	// EN: Update progress
	// ES: Actualizar progreso
	UpdateProgress();

	return true; // EN: Keep ticking / ES: Seguir con ticks
}

bool UPGXPSOSubsystem::OnProgressCheckTick(float /*DeltaTime*/)
{
	if (CurrentState != EPGXPSOWarmUpState::Compiling)
	{
		// EN: Not in compiling state — stop ticker
		// ES: No en estado compiling — detener ticker
		ProgressTickerHandle.Reset();
		return false;
	}

	// EN: Count completed graph events
	// ES: Contar graph events completados
	int32 CompletedCount = 0;
	for (const FGraphEventRef& Event : ActiveGraphEvents)
	{
		if (!Event.IsValid() || Event->IsComplete())
		{
			CompletedCount++;
		}
	}

	// EN: Update progress data
	// ES: Actualizar datos de progreso
	CachedProgress.CompletedEntries = CompletedCount;
	CachedProgress.ElapsedTimeSeconds = static_cast<float>(FPlatformTime::Seconds() - WarmUpStartTime);

	if (ActiveGraphEvents.Num() > 0)
	{
		CachedProgress.PercentComplete = static_cast<float>(CompletedCount) / static_cast<float>(ActiveGraphEvents.Num());
	}
	else
	{
		CachedProgress.PercentComplete = 1.0f;
	}

	// EN: Broadcast progress
	// ES: Emitir progreso
	OnWarmUpProgressDelegate.Broadcast(CompletedCount, ActiveGraphEvents.Num(), CachedProgress.PercentComplete);
	OnWarmUpProgressNative.Broadcast(CompletedCount, ActiveGraphEvents.Num(), CachedProgress.PercentComplete);
	BroadcastLoadingBridgeProgress();

	// EN: Check if all compilations are complete
	// ES: Verificar si todas las compilaciones estan completas
	if (CompletedCount >= ActiveGraphEvents.Num())
	{
		ProgressTickerHandle.Reset();
		OnAllCompilationsComplete();
		return false; // EN: Remove ticker / ES: Remover ticker
	}

	return true; // EN: Keep ticking / ES: Seguir con ticks
}

void UPGXPSOSubsystem::OnAllCompilationsComplete()
{
	const double ElapsedTime = FPlatformTime::Seconds() - WarmUpStartTime;

	// EN: Final progress update
	// ES: Actualizacion final de progreso
	CachedProgress.PercentComplete = 1.0f;
	CachedProgress.ElapsedTimeSeconds = static_cast<float>(ElapsedTime);
	CachedProgress.PeakLoadedMaterials = PeakLoadedMaterialCount;

	SetState(EPGXPSOWarmUpState::Complete);

	// EN: Broadcast completion
	// ES: Emitir completitud
	OnWarmUpComplete.Broadcast();
	OnWarmUpCompleteNative.Broadcast();
	BroadcastLoadingBridgeComplete();

	PGX_LOG_INFO(LogPGXPSO, TEXT("UPGXPSOSubsystem: Warm-up COMPLETE — %d entries, %d submitted, %d failed, %d dedup, %.2fs"),
		CachedProgress.TotalEntries,
		TotalSubmitted,
		CachedProgress.FailedEntries,
		CachedProgress.DeduplicatedEntries,
		ElapsedTime);

	// EN: Auto-save cache if configured
	// ES: Auto-guardar cache si esta configurado
	if (ActiveWarmUpConfig && ActiveWarmUpConfig->bSaveCacheAfterWarmUp
		&& ActiveWarmUpConfig->SavePolicy == EPGXPSOSavePolicy::OnCompleteOnly)
	{
		bCacheDirty = true;
		SaveCacheToDisk();
	}
	else if (TotalSubmitted > 0)
	{
		bCacheDirty = true;
	}

	// EN: Release non-persistent loaded materials (allow GC)
	// ES: Liberar materiales cargados no-persistentes (permitir GC)
	// Note: Materials with bKeepLoadedAfterPrecache remain in LoadedMaterials
	CurrentLoadedMaterialCount = LoadedMaterials.Num();
}

const FVertexFactoryType* UPGXPSOSubsystem::ResolveVertexFactory(EPGXVertexFactoryType Type, FName CustomName) const
{
	const FName VFName = PGXPSOUtils::ResolveVertexFactoryType(Type, CustomName);
	const FHashedName HashedName(VFName);
	return FVertexFactoryType::GetVFByName(HashedName);
}

FPSOPrecacheParams UPGXPSOSubsystem::BuildPrecacheParams(const FPGXPSOEntry& Entry) const
{
	FPSOPrecacheParams Params;
	Params.bRenderInMainPass = Entry.bRenderInMainPass;
	Params.bRenderInDepthPass = Entry.bRenderInDepthPass;
	Params.bCastShadow = Entry.bCastShadow;
	Params.bSkinnedMesh = Entry.bSkinnedMesh;
	Params.bRenderCustomDepth = Entry.bRenderCustomDepth;
	Params.SetMobility(Entry.Mobility.GetValue());

	// EN: Set spline mesh flag based on vertex factory type
	// ES: Establecer flag de spline mesh basado en tipo de vertex factory
	Params.bSplineMesh = (Entry.VertexFactory == EPGXVertexFactoryType::SplineMesh);

	return Params;
}

void UPGXPSOSubsystem::ResetPipelineState()
{
	// EN: Stop any running timers
	// ES: Detener cualquier timer corriendo
	if (BatchTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(BatchTickerHandle);
		BatchTickerHandle.Reset();
	}
	if (ProgressTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(ProgressTickerHandle);
		ProgressTickerHandle.Reset();
	}

	// EN: Clear pipeline tracking state
	// ES: Limpiar estado de tracking del pipeline
	ActiveGraphEvents.Empty();
	PendingBatchEntries.Empty();
	SubmittedKeys.Empty();
	TotalSubmitted = 0;
	CurrentLoadedMaterialCount = 0;
	PeakLoadedMaterialCount = 0;
	WarmUpStartTime = 0.0;

	// EN: Reset progress
	// ES: Resetear progreso
	CachedProgress = FPGXPSOWarmUpProgress();
}

void UPGXPSOSubsystem::UpdateProgress()
{
	CachedProgress.ElapsedTimeSeconds = static_cast<float>(FPlatformTime::Seconds() - WarmUpStartTime);

	// EN: Count completed graph events
	// ES: Contar graph events completados
	int32 CompletedCount = 0;
	for (const FGraphEventRef& Event : ActiveGraphEvents)
	{
		if (!Event.IsValid() || Event->IsComplete())
		{
			CompletedCount++;
		}
	}
	CachedProgress.CompletedEntries = CompletedCount;

	if (CachedProgress.TotalEntries > 0)
	{
		CachedProgress.PercentComplete = static_cast<float>(TotalSubmitted) / static_cast<float>(CachedProgress.TotalEntries);
	}

	CachedProgress.PeakLoadedMaterials = PeakLoadedMaterialCount;
}

// ============================================================================
// EN: GameFlow Bridge Messaging
// ES: Mensajeria de puente GameFlow
// ============================================================================

void UPGXPSOSubsystem::BindToGameFlowBridgeMessages()
{
	if (bGameFlowBridgeBound) return;

	const FGameplayTag GameFlowBridgeTag = TAG_PGX_Bridge_GameFlow_StateChanged.GetTag();
	if (!GameFlowBridgeTag.IsValid())
	{
		PGX_LOG_WARNING(LogPGXPSO, TEXT("UPGXPSOSubsystem: GameFlow bridge tag unavailable — OnGameFlowTag configs will not auto-trigger"));
		return;
	}

	TWeakObjectPtr<ThisClass> WeakThis(this);
	GameFlowBridgeMessageHandles.Add(PGXBaseMessaging::Listen<FPGXBridgeGameFlowChanged>(
		this,
		GameFlowBridgeTag,
		[WeakThis](FGameplayTag Channel, const FPGXBridgeGameFlowChanged& Payload)
		{
			if (ThisClass* StrongThis = WeakThis.Get())
			{
				StrongThis->OnGameFlowBridgeChanged(Channel, Payload);
			}
		}));

	if (!GameFlowBridgeMessageHandles.Last().IsValid())
	{
		GameFlowBridgeMessageHandles.RemoveAt(GameFlowBridgeMessageHandles.Num() - 1, 1, EAllowShrinking::No);
		PGX_LOG_WARNING(LogPGXPSO, TEXT("UPGXPSOSubsystem: GameFlow bridge listener registration failed — OnGameFlowTag configs will not auto-trigger"));
		return;
	}

	bGameFlowBridgeBound = true;
	PGX_LOG_VERBOSE(LogPGXPSO, TEXT("UPGXPSOSubsystem: Bound to GameFlow bridge state-change messages"));
}

void UPGXPSOSubsystem::UnbindGameFlowBridgeMessages()
{
	PGXBaseMessaging::UnregisterAll(GameFlowBridgeMessageHandles);
	bGameFlowBridgeBound = false;
}

void UPGXPSOSubsystem::OnGameFlowBridgeChanged(FGameplayTag /*Channel*/, const FPGXBridgeGameFlowChanged& Payload)
{
	const FGameplayTag FlowTag = Payload.NewState;
	if (!FlowTag.IsValid())
	{
		PGX_LOG_WARNING(LogPGXPSO, TEXT("UPGXPSOSubsystem: Ignoring GameFlow bridge payload without valid NewState tag"));
		return;
	}

	for (const TObjectPtr<UPGXPSOWarmUpConfig>& Config : DiscoveredConfigs)
	{
		if (!Config) continue;
		if (Config->ActivationMode != EPGXPSOActivationMode::OnGameFlowTag) continue;
		if (!Config->TriggerGameFlowTag.IsValid()) continue;
		if (Config->TriggerGameFlowTag != FlowTag) continue;

		PGX_LOG_INFO(LogPGXPSO, TEXT("UPGXPSOSubsystem: GameFlow bridge tag '%s' triggered warm-up for config '%s'"),
			*FlowTag.ToString(), *Config->GetName());

		StartOrMergePipeline(Config->Entries, Config);
	}
}

#if WITH_DEV_AUTOMATION_TESTS
void UPGXPSOSubsystem::RebindGameFlowBridgeForTesting()
{
	UnbindGameFlowBridgeMessages();

	for (const TObjectPtr<UPGXPSOWarmUpConfig>& Config : DiscoveredConfigs)
	{
		if (Config && Config->ActivationMode == EPGXPSOActivationMode::OnGameFlowTag)
		{
			BindToGameFlowBridgeMessages();
			return;
		}
	}
}
#endif

#if WITH_DEV_AUTOMATION_TESTS
void UPGXPSOSubsystem::InjectTestConfigForTesting(UPGXPSOWarmUpConfig* Config)
{
	if (!IsValid(Config))
	{
		PGX_LOG_WARNING(LogPGXPSO, TEXT("[TestHarness] InjectTestConfigForTesting — invalid Config"));
		return;
	}

	DiscoveredConfigs.AddUnique(Config);
	PGX_LOG_INFO(LogPGXPSO, TEXT("[TestHarness] Injected test PSOWarmUpConfig: %s"), *Config->GetName());
}

void UPGXPSOSubsystem::ClearTestConfigsForTesting()
{
	const int32 Before = DiscoveredConfigs.Num();
	DiscoveredConfigs.RemoveAll([](const TObjectPtr<UPGXPSOWarmUpConfig>& Cfg)
	{
		return IsValid(Cfg) && Cfg->HasAnyFlags(RF_Transient);
	});
	PGX_LOG_INFO(LogPGXPSO, TEXT("[TestHarness] ClearTestConfigsForTesting — removed %d transient configs"),
		Before - DiscoveredConfigs.Num());
}
#endif

// ============================================================================
// EN: Console Commands
// ES: Comandos de Consola
// ============================================================================

void UPGXPSOSubsystem::RegisterConsoleCommands()
{
	IConsoleManager& CM = IConsoleManager::Get();

	// ── 1. pgx.pso.status ──
	if (IConsoleObject* Cmd = CM.RegisterConsoleCommand(
		TEXT("pgx.pso.status"),
		TEXT("Show PSO subsystem status: state, contexts, configs, cache"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			const TCHAR* StateNames[] = { TEXT("Idle"), TEXT("Loading"), TEXT("Compiling"), TEXT("Complete"), TEXT("Failed"), TEXT("Paused") };
			const int32 StateIdx = static_cast<int32>(CurrentState);
			PGX_LOG_INFO(LogPGXPSO, TEXT("=== PGX PSO Status ==="));
			PGX_LOG_INFO(LogPGXPSO, TEXT("  State: %s"), (StateIdx >= 0 && StateIdx < 6) ? StateNames[StateIdx] : TEXT("Unknown"));
			PGX_LOG_INFO(LogPGXPSO, TEXT("  Discovered Configs: %d"), DiscoveredConfigs.Num());
			PGX_LOG_INFO(LogPGXPSO, TEXT("  Active Contexts: %d"), ActiveContexts.Num());
			for (const FGameplayTag& Ctx : ActiveContexts)
			{
				PGX_LOG_INFO(LogPGXPSO, TEXT("    - %s"), *Ctx.ToString());
			}
			PGX_LOG_INFO(LogPGXPSO, TEXT("  Cache Dirty: %s"), bCacheDirty ? TEXT("YES") : TEXT("NO"));
			PGX_LOG_INFO(LogPGXPSO, TEXT("  GameFlow Bridge Bound: %s"), bGameFlowBridgeBound ? TEXT("YES") : TEXT("NO"));
		}), ECVF_Default))
	{
		RegisteredCommands.Add(Cmd);
	}

	// ── 2. pgx.pso.warmup [ContextTag] ──
	if (IConsoleObject* Cmd = CM.RegisterConsoleCommand(
		TEXT("pgx.pso.warmup"),
		TEXT("Start warm-up: pgx.pso.warmup [ContextTag] (no args = all)"),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& Args)
		{
			if (Args.Num() == 0)
			{
				const bool bResult = RequestWarmUpAll();
				PGX_LOG_INFO(LogPGXPSO, TEXT("pgx.pso.warmup (all): %s"), bResult ? TEXT("STARTED") : TEXT("FAILED/NO ENTRIES"));
			}
			else
			{
				const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*Args[0]), false);
				if (!Tag.IsValid())
				{
					PGX_LOG_WARNING(LogPGXPSO, TEXT("pgx.pso.warmup: Invalid tag '%s'"), *Args[0]);
					return;
				}
				const bool bResult = RequestWarmUp(Tag);
				PGX_LOG_INFO(LogPGXPSO, TEXT("pgx.pso.warmup [%s]: %s"), *Tag.ToString(), bResult ? TEXT("STARTED") : TEXT("FAILED/NO ENTRIES"));
			}
		}), ECVF_Default))
	{
		RegisteredCommands.Add(Cmd);
	}

	// ── 3. pgx.pso.pause ──
	if (IConsoleObject* Cmd = CM.RegisterConsoleCommand(
		TEXT("pgx.pso.pause"),
		TEXT("Pause active PSO warm-up"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			PauseWarmUp();
			PGX_LOG_INFO(LogPGXPSO, TEXT("pgx.pso.pause: State is now %d"), static_cast<int32>(CurrentState));
		}), ECVF_Default))
	{
		RegisteredCommands.Add(Cmd);
	}

	// ── 4. pgx.pso.resume ──
	if (IConsoleObject* Cmd = CM.RegisterConsoleCommand(
		TEXT("pgx.pso.resume"),
		TEXT("Resume paused PSO warm-up"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			ResumeWarmUp();
			PGX_LOG_INFO(LogPGXPSO, TEXT("pgx.pso.resume: State is now %d"), static_cast<int32>(CurrentState));
		}), ECVF_Default))
	{
		RegisteredCommands.Add(Cmd);
	}

	// ── 5. pgx.pso.cancel ──
	if (IConsoleObject* Cmd = CM.RegisterConsoleCommand(
		TEXT("pgx.pso.cancel"),
		TEXT("Cancel active PSO warm-up"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			CancelWarmUp();
			PGX_LOG_INFO(LogPGXPSO, TEXT("pgx.pso.cancel: Warm-up cancelled"));
		}), ECVF_Default))
	{
		RegisteredCommands.Add(Cmd);
	}

	// ── 6. pgx.pso.progress ──
	if (IConsoleObject* Cmd = CM.RegisterConsoleCommand(
		TEXT("pgx.pso.progress"),
		TEXT("Show PSO warm-up progress details"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			PGX_LOG_INFO(LogPGXPSO, TEXT("=== PGX PSO Progress ==="));
			PGX_LOG_INFO(LogPGXPSO, TEXT("  Total Entries: %d"), CachedProgress.TotalEntries);
			PGX_LOG_INFO(LogPGXPSO, TEXT("  Completed: %d"), CachedProgress.CompletedEntries);
			PGX_LOG_INFO(LogPGXPSO, TEXT("  Failed: %d"), CachedProgress.FailedEntries);
			PGX_LOG_INFO(LogPGXPSO, TEXT("  Deduplicated: %d"), CachedProgress.DeduplicatedEntries);
			PGX_LOG_INFO(LogPGXPSO, TEXT("  Percent: %.1f%%"), CachedProgress.PercentComplete * 100.0f);
			PGX_LOG_INFO(LogPGXPSO, TEXT("  Elapsed: %.2fs"), CachedProgress.ElapsedTimeSeconds);
			PGX_LOG_INFO(LogPGXPSO, TEXT("  Peak Materials Loaded: %d"), CachedProgress.PeakLoadedMaterials);
			PGX_LOG_INFO(LogPGXPSO, TEXT("  Precache Requests: %d"), CachedProgress.TotalPrecacheRequests);
		}), ECVF_Default))
	{
		RegisteredCommands.Add(Cmd);
	}

	// ── 7. pgx.pso.save ──
	if (IConsoleObject* Cmd = CM.RegisterConsoleCommand(
		TEXT("pgx.pso.save"),
		TEXT("Save PSO cache to disk"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			SaveCacheToDisk();
			PGX_LOG_INFO(LogPGXPSO, TEXT("pgx.pso.save: Cache saved to disk"));
		}), ECVF_Default))
	{
		RegisteredCommands.Add(Cmd);
	}

	// ── 8. pgx.pso.configs ──
	if (IConsoleObject* Cmd = CM.RegisterConsoleCommand(
		TEXT("pgx.pso.configs"),
		TEXT("List all discovered PSO WarmUpConfig DataAssets"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			const TCHAR* ModeNames[] = { TEXT("OnSubsystemInit"), TEXT("OnLevelLoad"), TEXT("OnGameFlowTag"), TEXT("OnExplicitCall") };
			PGX_LOG_INFO(LogPGXPSO, TEXT("=== PGX PSO Configs (%d) ==="), DiscoveredConfigs.Num());
			for (int32 i = 0; i < DiscoveredConfigs.Num(); ++i)
			{
				const UPGXPSOWarmUpConfig* Config = DiscoveredConfigs[i];
				if (!Config) continue;
				const int32 ModeIdx = static_cast<int32>(Config->ActivationMode);
				PGX_LOG_INFO(LogPGXPSO, TEXT("  [%d] '%s' — %d entries, activation: %s, batch: %d, concurrency: %d"),
					i,
					*Config->GetName(),
					Config->Entries.Num(),
					(ModeIdx >= 0 && ModeIdx < 4) ? ModeNames[ModeIdx] : TEXT("Unknown"),
					Config->BatchSize,
					static_cast<int32>(Config->ConcurrencyPolicy));
				if (Config->ActivationMode == EPGXPSOActivationMode::OnGameFlowTag && Config->TriggerGameFlowTag.IsValid())
				{
					PGX_LOG_INFO(LogPGXPSO, TEXT("        Trigger Tag: %s"), *Config->TriggerGameFlowTag.ToString());
				}
			}
		}), ECVF_Default))
	{
		RegisteredCommands.Add(Cmd);
	}

	// ── 9. pgx.pso.validate [ConfigName] ──
	if (IConsoleObject* Cmd = CM.RegisterConsoleCommand(
		TEXT("pgx.pso.validate"),
		TEXT("Validate PSO config entries: pgx.pso.validate [ConfigName] (no args = all)"),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& Args)
		{
			TArray<const UPGXPSOWarmUpConfig*> ConfigsToValidate;

			if (Args.Num() > 0)
			{
				// EN: Find config by name / ES: Buscar config por nombre
				for (const TObjectPtr<UPGXPSOWarmUpConfig>& Config : DiscoveredConfigs)
				{
					if (Config && Config->GetName().Contains(Args[0]))
					{
						ConfigsToValidate.Add(Config);
					}
				}
				if (ConfigsToValidate.IsEmpty())
				{
					PGX_LOG_WARNING(LogPGXPSO, TEXT("pgx.pso.validate: No config matching '%s'"), *Args[0]);
					return;
				}
			}
			else
			{
				for (const TObjectPtr<UPGXPSOWarmUpConfig>& Config : DiscoveredConfigs)
				{
					if (Config) ConfigsToValidate.Add(Config);
				}
			}

			int32 TotalErrors = 0;
			int32 TotalEntries = 0;

			for (const UPGXPSOWarmUpConfig* Config : ConfigsToValidate)
			{
				PGX_LOG_INFO(LogPGXPSO, TEXT("--- Validating '%s' (%d entries) ---"), *Config->GetName(), Config->Entries.Num());

				for (int32 i = 0; i < Config->Entries.Num(); ++i)
				{
					const FPGXPSOEntry& Entry = Config->Entries[i];
					TotalEntries++;

					// EN: Check material soft ref / ES: Verificar soft ref de material
					if (Entry.EntryType == EPGXPSOEntryType::Material && Entry.Material.IsNull())
					{
						PGX_LOG_WARNING(LogPGXPSO, TEXT("  [%d] '%s': Material is NULL"), i, *Entry.Label);
						TotalErrors++;
					}
					else if (Entry.EntryType == EPGXPSOEntryType::Material && !Entry.Material.IsNull())
					{
						// EN: Try to resolve soft ref path (does not load) / ES: Intentar resolver ruta de soft ref (no carga)
						const FSoftObjectPath& Path = Entry.Material.ToSoftObjectPath();
						if (!Path.IsValid())
						{
							PGX_LOG_WARNING(LogPGXPSO, TEXT("  [%d] '%s': Material path invalid: %s"), i, *Entry.Label, *Path.ToString());
							TotalErrors++;
						}
					}

					// EN: Check vertex factory / ES: Verificar vertex factory
					if (Entry.VertexFactory == EPGXVertexFactoryType::Custom && Entry.CustomVertexFactoryName.IsNone())
					{
						PGX_LOG_WARNING(LogPGXPSO, TEXT("  [%d] '%s': Custom VF selected but CustomVertexFactoryName is empty"), i, *Entry.Label);
						TotalErrors++;
					}

					// EN: Check VF resolvability / ES: Verificar resolubilidad de VF
					const FVertexFactoryType* VF = ResolveVertexFactory(Entry.VertexFactory, Entry.CustomVertexFactoryName);
					if (!VF)
					{
						PGX_LOG_WARNING(LogPGXPSO, TEXT("  [%d] '%s': VF type '%s' could not be resolved"),
							i, *Entry.Label, *Entry.GetResolvedVertexFactoryName().ToString());
						TotalErrors++;
					}
				}
			}

			PGX_LOG_INFO(LogPGXPSO, TEXT("=== Validation Complete: %d entries, %d errors ==="), TotalEntries, TotalErrors);
		}), ECVF_Default))
	{
		RegisteredCommands.Add(Cmd);
	}

	// ── 10. pgx.pso.record.start [SessionName] ──
	if (IConsoleObject* Cmd = CM.RegisterConsoleCommand(
		TEXT("pgx.pso.record.start"),
		TEXT("Start PSO recording session: pgx.pso.record.start [SessionName]"),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& Args)
		{
#if WITH_EDITOR
			const FString SessionName = Args.Num() > 0 ? Args[0] : TEXT("");
			StartRecording(SessionName);
#else
			PGX_LOG_WARNING(LogPGXPSO, TEXT("pgx.pso.record.start: Recording is editor-only"));
#endif
		}), ECVF_Default))
	{
		RegisteredCommands.Add(Cmd);
	}

	// ── 11. pgx.pso.record.stop ──
	if (IConsoleObject* Cmd = CM.RegisterConsoleCommand(
		TEXT("pgx.pso.record.stop"),
		TEXT("Stop PSO recording session"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
#if WITH_EDITOR
			StopRecording();
#else
			PGX_LOG_WARNING(LogPGXPSO, TEXT("pgx.pso.record.stop: Recording is editor-only"));
#endif
		}), ECVF_Default))
	{
		RegisteredCommands.Add(Cmd);
	}

	// ── 12. pgx.pso.record.export [Path] ──
	if (IConsoleObject* Cmd = CM.RegisterConsoleCommand(
		TEXT("pgx.pso.record.export"),
		TEXT("Export recorded PSO data to JSON: pgx.pso.record.export [Path]"),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& Args)
		{
#if WITH_EDITOR
			const FString Path = Args.Num() > 0 ? Args[0] : TEXT("");
			ExportRecordingToJson(Path);
#else
			PGX_LOG_WARNING(LogPGXPSO, TEXT("pgx.pso.record.export: Recording is editor-only"));
#endif
		}), ECVF_Default))
	{
		RegisteredCommands.Add(Cmd);
	}

	// ── 13. pgx.pso.record.clear ──
	if (IConsoleObject* Cmd = CM.RegisterConsoleCommand(
		TEXT("pgx.pso.record.clear"),
		TEXT("Clear recorded PSO data"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
#if WITH_EDITOR
			ClearRecording();
#else
			PGX_LOG_WARNING(LogPGXPSO, TEXT("pgx.pso.record.clear: Recording is editor-only"));
#endif
		}), ECVF_Default))
	{
		RegisteredCommands.Add(Cmd);
	}

	// ── 14. pgx.pso.stats ──
	if (IConsoleObject* Cmd = CM.RegisterConsoleCommand(
		TEXT("pgx.pso.stats"),
		TEXT("Show UE native PSO pipeline cache statistics"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			PGX_LOG_INFO(LogPGXPSO, TEXT("=== UE Native PSO Stats ==="));
			const int32 Remaining = static_cast<int32>(FShaderPipelineCache::NumPrecompilesRemaining());
			PGX_LOG_INFO(LogPGXPSO, TEXT("  Precompiles Remaining: %d"), Remaining);
			PGX_LOG_INFO(LogPGXPSO, TEXT("  Batching Paused: %s"), FShaderPipelineCache::IsBatchingPaused() ? TEXT("YES") : TEXT("NO"));
			PGX_LOG_INFO(LogPGXPSO, TEXT("  PGX Submitted: %d"), TotalSubmitted);
			PGX_LOG_INFO(LogPGXPSO, TEXT("  PGX Active Graph Events: %d"), ActiveGraphEvents.Num());
		}), ECVF_Default))
	{
		RegisteredCommands.Add(Cmd);
	}
}

void UPGXPSOSubsystem::UnregisterConsoleCommands()
{
	for (IConsoleObject* Cmd : RegisteredCommands)
	{
		if (Cmd)
		{
			IConsoleManager::Get().UnregisterConsoleObject(Cmd);
		}
	}
	RegisteredCommands.Empty();
}

// ============================================================================
// EN: Recording API (Editor-only)
// ES: API de Grabacion (Solo-editor)
// ============================================================================

#if WITH_EDITOR

void UPGXPSOSubsystem::StartRecording(const FString& SessionName)
{
	if (bIsRecording)
	{
		PGX_LOG_WARNING(LogPGXPSO, TEXT("UPGXPSOSubsystem::StartRecording — Already recording session '%s'"), *RecordingSessionName);
		return;
	}

	RecordingSessionName = SessionName.IsEmpty() ? FDateTime::Now().ToString() : SessionName;
	RecordingStartTime = FPlatformTime::Seconds();
	RecordedEntries.Empty();
	bIsRecording = true;

	PGX_LOG_INFO(LogPGXPSO, TEXT("UPGXPSOSubsystem: Recording started — session '%s'"), *RecordingSessionName);
}

void UPGXPSOSubsystem::StopRecording()
{
	if (!bIsRecording)
	{
		PGX_LOG_WARNING(LogPGXPSO, TEXT("UPGXPSOSubsystem::StopRecording — Not recording"));
		return;
	}

	bIsRecording = false;
	const double ElapsedTime = FPlatformTime::Seconds() - RecordingStartTime;

	PGX_LOG_INFO(LogPGXPSO, TEXT("UPGXPSOSubsystem: Recording stopped — session '%s', %d entries, %.2fs"),
		*RecordingSessionName, RecordedEntries.Num(), ElapsedTime);
}

void UPGXPSOSubsystem::ClearRecording()
{
	if (bIsRecording)
	{
		StopRecording();
	}
	RecordedEntries.Empty();
	RecordingSessionName.Empty();
	PGX_LOG_INFO(LogPGXPSO, TEXT("UPGXPSOSubsystem: Recording cleared"));
}

bool UPGXPSOSubsystem::ExportRecordingToJson(const FString& FilePath)
{
	if (RecordedEntries.IsEmpty())
	{
		PGX_LOG_WARNING(LogPGXPSO, TEXT("UPGXPSOSubsystem::ExportRecordingToJson — No recorded entries to export"));
		return false;
	}

	// EN: Build JSON array
	// ES: Construir array JSON
	TArray<TSharedPtr<FJsonValue>> JsonEntries;
	for (const FPGXPSORecordedEntry& Entry : RecordedEntries)
	{
		TSharedPtr<FJsonObject> EntryObj = MakeShared<FJsonObject>();
		EntryObj->SetStringField(TEXT("MaterialPath"), Entry.MaterialPath);
		EntryObj->SetStringField(TEXT("VertexFactory"), Entry.VertexFactoryName.ToString());
		EntryObj->SetStringField(TEXT("MeshPass"), Entry.MeshPassName);
		EntryObj->SetStringField(TEXT("Timestamp"), Entry.Timestamp.ToString());
		EntryObj->SetNumberField(TEXT("CompilationTimeMs"), Entry.CompilationTimeMs);
		EntryObj->SetBoolField(TEXT("CausedHitch"), Entry.bCausedHitch);
		EntryObj->SetNumberField(TEXT("FrameNumber"), static_cast<double>(Entry.FrameNumber));
		EntryObj->SetStringField(TEXT("PrecacheResult"), Entry.PrecacheResult);
		JsonEntries.Add(MakeShared<FJsonValueObject>(EntryObj));
	}

	// EN: Build root object
	// ES: Construir objeto raiz
	TSharedPtr<FJsonObject> RootObj = MakeShared<FJsonObject>();
	RootObj->SetStringField(TEXT("SessionName"), RecordingSessionName);
	RootObj->SetStringField(TEXT("ExportDate"), FDateTime::Now().ToString());
	RootObj->SetNumberField(TEXT("EntryCount"), RecordedEntries.Num());
	RootObj->SetArrayField(TEXT("Entries"), JsonEntries);

	// EN: Serialize to string
	// ES: Serializar a string
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(RootObj.ToSharedRef(), Writer);

	// EN: Resolve output path
	// ES: Resolver ruta de salida
	FString ResolvedPath = FilePath;
	if (ResolvedPath.IsEmpty())
	{
		ResolvedPath = FPaths::ProjectSavedDir() / TEXT("PGX") / TEXT("PSO") /
			FString::Printf(TEXT("PSORecording_%s.json"), *RecordingSessionName);
	}

	// EN: Ensure directory exists
	// ES: Asegurar que el directorio exista
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(ResolvedPath), true);

	if (FFileHelper::SaveStringToFile(OutputString, *ResolvedPath))
	{
		PGX_LOG_INFO(LogPGXPSO, TEXT("UPGXPSOSubsystem: Recording exported to '%s' (%d entries)"),
			*ResolvedPath, RecordedEntries.Num());
		return true;
	}

	PGX_LOG_ERROR(LogPGXPSO, TEXT("UPGXPSOSubsystem::ExportRecordingToJson — Failed to write file '%s'"), *ResolvedPath);
	return false;
}

TArray<FPGXPSOEntry> UPGXPSOSubsystem::ConvertRecordingToCatalogEntries(FGameplayTag DefaultContext) const
{
	TArray<FPGXPSOEntry> Result;
	TSet<FString> SeenMaterials;

	for (const FPGXPSORecordedEntry& Recorded : RecordedEntries)
	{
		// EN: Deduplicate by material path
		// ES: Deduplicar por ruta de material
		if (SeenMaterials.Contains(Recorded.MaterialPath))
		{
			continue;
		}
		SeenMaterials.Add(Recorded.MaterialPath);

		FPGXPSOEntry NewEntry;
		NewEntry.Material = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(Recorded.MaterialPath));
		NewEntry.ContextTag = DefaultContext;
		NewEntry.Label = FPaths::GetBaseFilename(Recorded.MaterialPath);

		// EN: Resolve VF name back to enum
		// ES: Resolver nombre VF de vuelta a enum
		const FName VFName = Recorded.VertexFactoryName;
		if (VFName == TEXT("FLocalVertexFactory"))                      NewEntry.VertexFactory = EPGXVertexFactoryType::StaticMesh;
		else if (VFName == TEXT("FGPUSkinVertexFactory"))               NewEntry.VertexFactory = EPGXVertexFactoryType::SkeletalMesh;
		else if (VFName == TEXT("FInstancedStaticMeshVertexFactory"))   NewEntry.VertexFactory = EPGXVertexFactoryType::InstancedMesh;
		else if (VFName == TEXT("FSplineMeshVertexFactory"))            NewEntry.VertexFactory = EPGXVertexFactoryType::SplineMesh;
		else if (VFName == TEXT("FLandscapeVertexFactory"))             NewEntry.VertexFactory = EPGXVertexFactoryType::Landscape;
		else if (VFName == TEXT("FNiagaraSpriteVertexFactory"))         NewEntry.VertexFactory = EPGXVertexFactoryType::NiagaraSprite;
		else if (VFName == TEXT("FNiagaraMeshVertexFactory"))           NewEntry.VertexFactory = EPGXVertexFactoryType::NiagaraMesh;
		else
		{
			NewEntry.VertexFactory = EPGXVertexFactoryType::Custom;
			NewEntry.CustomVertexFactoryName = VFName;
		}

		Result.Add(NewEntry);
	}

	return Result;
}

TArray<FString> UPGXPSOSubsystem::ValidateConfig(const UPGXPSOWarmUpConfig* Config) const
{
	TArray<FString> Errors;

	if (!Config)
	{
		Errors.Add(TEXT("Config is null"));
		return Errors;
	}

	for (int32 i = 0; i < Config->Entries.Num(); ++i)
	{
		const FPGXPSOEntry& Entry = Config->Entries[i];

		// EN: Check material ref / ES: Verificar ref de material
		if (Entry.EntryType == EPGXPSOEntryType::Material && Entry.Material.IsNull())
		{
			Errors.Add(FString::Printf(TEXT("[%d] '%s': Material is NULL"), i, *Entry.Label));
		}

		// EN: Check custom VF name / ES: Verificar nombre VF custom
		if (Entry.VertexFactory == EPGXVertexFactoryType::Custom && Entry.CustomVertexFactoryName.IsNone())
		{
			Errors.Add(FString::Printf(TEXT("[%d] '%s': Custom VF selected but name is empty"), i, *Entry.Label));
		}

		// EN: Check VF resolvability / ES: Verificar resolubilidad de VF
		const FVertexFactoryType* VF = ResolveVertexFactory(Entry.VertexFactory, Entry.CustomVertexFactoryName);
		if (!VF)
		{
			Errors.Add(FString::Printf(TEXT("[%d] '%s': VF '%s' could not be resolved"),
				i, *Entry.Label, *Entry.GetResolvedVertexFactoryName().ToString()));
		}
	}

	return Errors;
}

#endif // WITH_EDITOR

// ============================================================================
// EN: State Management
// ES: Gestion de Estado
// ============================================================================

void UPGXPSOSubsystem::SetState(EPGXPSOWarmUpState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	const EPGXPSOWarmUpState OldState = CurrentState;
	CurrentState = NewState;
	CachedProgress.State = NewState;

	PGX_LOG_INFO(LogPGXPSO, TEXT("UPGXPSOSubsystem: State %d -> %d"),
		static_cast<int32>(OldState), static_cast<int32>(NewState));

	// EN: Broadcast native state change
	// ES: Emitir cambio de estado nativo
	OnStateChangedNative.Broadcast(NewState);
	BroadcastLoadingBridgeState();
}

// ============================================================================
// Profile Integration
// ============================================================================

void UPGXPSOSubsystem::ApplyProfileConstraints(const FPGXResolvedProfile& Profile)
{
	// EN: Profile clamp materializes via RecomputeEffectiveBudgets (single source of truth).
	//     Budgets resolve into EffectiveMaxEntries/TimeBudgetMs/MaxShaders members, and
	//     RequestWarmUp/All apply them as hard caps using the runtime request path's effective-cap rule.
	// ES: El clamp por Profile se materializa via RecomputeEffectiveBudgets (fuente unica de verdad).
	//     Los presupuestos resuelven a EffectiveXxx y RequestWarmUp/All
	//     los aplican como tope duro.
	RecomputeEffectiveBudgets();

	PGX_LOG_INFO(LogPGXPSO, TEXT("[PSOSubsystem] Profile constraints applied via RecomputeEffectiveBudgets. Global ShaderBudget=%d, PSOBudget=%d"),
		Profile.Budgets.ShaderBudget, Profile.Budgets.PSOBudget);
}

void UPGXPSOSubsystem::HandleProfileChanged(const FPGXResolvedProfile& /*OldProfile*/, const FPGXResolvedProfile& NewProfile)
{
	ApplyProfileConstraints(NewProfile);
}

void UPGXPSOSubsystem::RecomputeEffectiveBudgets()
{
	// EN: Resolve effective budgets from active PlatformConfig.PSOBudgets. Profile-resolved values
	//     of 0 mean "unbounded" (no platform-side clamp); positive values are hard caps.
	//     Logs transition when any value changes so production divergence stays visible.
	// ES: Resolver presupuestos efectivos desde PlatformConfig.PSOBudgets activo. Valor 0 = sin
	//     clamp; positivos = tope duro. Loguea transiciones cuando cambian valores.
	int32 NewMaxEntries = 0;
	float NewTimeBudgetMs = 0.f;
	int32 NewMaxShaders = 0;

	if (UGameInstance* GI = GetGameInstance())
	{
		if (auto* ProfileSS = GI->GetSubsystem<UPGXProfileSubsystem>())
		{
			if (const UPGXPlatformConfig* PlatformCfg = ProfileSS->GetActivePlatformConfig())
			{
				const auto& B = PlatformCfg->PSOBudgets;
				NewMaxEntries = B.MaxWarmUpEntries;
				NewTimeBudgetMs = B.WarmUpTimeBudgetMs;
				NewMaxShaders = B.MaxShaderPermutations;
			}
		}
	}

	const bool bChanged = (NewMaxEntries != EffectiveMaxEntries)
		|| (!FMath::IsNearlyEqual(NewTimeBudgetMs, EffectiveTimeBudgetMs))
		|| (NewMaxShaders != EffectiveMaxShaders);

	if (bChanged)
	{
		PGX_LOG_INFO(LogPGXPSO,
			TEXT("[PSOSubsystem] EffectiveBudgets: MaxEntries %d->%d, TimeBudgetMs %.1f->%.1f, MaxShaders %d->%d"),
			EffectiveMaxEntries, NewMaxEntries,
			EffectiveTimeBudgetMs, NewTimeBudgetMs,
			EffectiveMaxShaders, NewMaxShaders);

		EffectiveMaxEntries = NewMaxEntries;
		EffectiveTimeBudgetMs = NewTimeBudgetMs;
		EffectiveMaxShaders = NewMaxShaders;
	}
}

// ============================================================================
// EN: Test injection API (editor only)
// ES: API de inyeccion de test (solo editor)
// ============================================================================

#if WITH_EDITOR
void UPGXPSOSubsystem::InjectTestConfig(UPGXPSOWarmUpConfig* Config)
{
	if (!IsValid(Config))
	{
		PGX_LOG_WARNING(LogPGXPSO, TEXT("[TestHarness] InjectTestConfig — invalid Config"));
		return;
	}

	DiscoveredConfigs.AddUnique(Config);
	PGX_LOG_INFO(LogPGXPSO, TEXT("[TestHarness] Injected test PSOWarmUpConfig: %s"), *Config->GetName());
}

void UPGXPSOSubsystem::ClearTestConfigs()
{
	const int32 Before = DiscoveredConfigs.Num();
	DiscoveredConfigs.RemoveAll([](const TObjectPtr<UPGXPSOWarmUpConfig>& Cfg)
	{
		return IsValid(Cfg) && Cfg->HasAnyFlags(RF_Transient);
	});
	PGX_LOG_INFO(LogPGXPSO, TEXT("[TestHarness] ClearTestConfigs — removed %d transient configs"),
		Before - DiscoveredConfigs.Num());
}
#endif
