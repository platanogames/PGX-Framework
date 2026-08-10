// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Base/PGXGameInstanceSubsystem.h"
#include "EventHandler/PGXEventHandlerTypes.h"
#include "EventHandler/PGXEventHandlerDelegates.h"
#include "Interfaces/PGXTaggedRegistry.h"
#include "InstancedStruct.h"
#include "PGXEventHandlerSubsystem.generated.h"

class UPGXEventHandlerBase;
class UPGXEventHandlerConfig;
class UDataTable;
struct IConsoleCommand;

/**
 * EN: PGX Event Handler Subsystem — data-driven behavior resolution bus.
 *     Resolves handler objects by GameplayTag, executes them with typed payloads,
 *     and manages lifecycle (Singleton/Cached/Ephemeral).
 *     Depends on UPGXMessageSubsystem for optional message broadcasting.
 * ES: Subsistema Event Handler PGX — bus de resolucion de comportamiento data-driven.
 *     Resuelve objetos handler por GameplayTag, los ejecuta con payloads tipados,
 *     y gestiona su ciclo de vida (Singleton/Cached/Ephemeral).
 *     Depende de UPGXMessageSubsystem para broadcasting opcional de mensajes.
 */
UCLASS()
class PGXCORERUNTIME_API UPGXEventHandlerSubsystem : public UPGXGameInstanceSubsystem, public IPGXTaggedRegistry
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface

	/** EN: Get the subsystem from a world context / ES: Obtener el subsistema desde un contexto de mundo */
	static UPGXEventHandlerSubsystem* Get(const UObject* WorldContextObject);

	// ============================================================
	// Core Execution API
	// ============================================================

	/**
	 * EN: Resolve and execute a handler — simplified (auto-builds minimal context from Instigator).
	 * ES: Resolver y ejecutar un handler — simplificado (auto-construye contexto minimo desde Instigator).
	 */
	EPGXEventResult ResolveAndExecute(FGameplayTag EventTag, UObject* Instigator, const FInstancedStruct& Payload);

	/**
	 * EN: Resolve and execute a handler with full context (Target, SourceTags).
	 * ES: Resolver y ejecutar un handler con contexto completo (Target, SourceTags).
	 */
	EPGXEventResult ResolveAndExecuteWithContext(FGameplayTag EventTag, const FPGXEventContext& Context, const FInstancedStruct& Payload);

	/** EN: Execute a sequence of handlers with per-event payloads / ES: Ejecutar una secuencia de handlers con payloads por evento */
	EPGXEventResult ExecuteSequence(const TArray<FGameplayTag>& EventTags, const FPGXEventContext& Context, const TArray<FInstancedStruct>& Payloads, bool bStopOnFailure = true);

	/** EN: Validate conditions for a set of event tags / ES: Validar condiciones para un conjunto de tags */
	bool ValidateConditions(const TArray<FGameplayTag>& EventTags, const FPGXEventContext& Context);

private:
	/** EN: Internal core execution — all public paths funnel here / ES: Ejecucion interna — todos los paths publicos convergen aqui */
	EPGXEventResult ResolveAndExecuteInternal(FGameplayTag EventTag, const FPGXEventContext& Context, const FInstancedStruct& Payload);
public:

	// ============================================================
	// Registration API
	// ============================================================

	/** EN: Register all handlers from a DataTable / ES: Registrar todos los handlers desde un DataTable */
	void RegisterHandlerTable(UDataTable* InTable);

	/** EN: Unregister all handlers from a DataTable / ES: Desregistrar todos los handlers de un DataTable */
	void UnregisterHandlerTable(UDataTable* InTable);

	/** EN: Register a single handler programmatically / ES: Registrar un handler individual programaticamente */
	void RegisterHandler(FGameplayTag EventTag, TSubclassOf<UPGXEventHandlerBase> HandlerClass,
		EPGXHandlerLifecycle Lifecycle = EPGXHandlerLifecycle::Cached, FGameplayTag CategoryTag = FGameplayTag());

	/** EN: Unregister a handler / ES: Desregistrar un handler */
	void UnregisterHandler(FGameplayTag EventTag);

	// ============================================================
	// Query API
	// ============================================================

	bool IsHandlerRegistered(FGameplayTag EventTag) const;

	FPGXEventHandlerInfo GetHandlerInfo(FGameplayTag EventTag) const;
	TArray<FPGXEventHandlerInfo> GetHandlersByCategory(FGameplayTag CategoryTag) const;
	TArray<FGameplayTag> GetAllRegisteredTags() const;

	// EN: IPGXTaggedRegistry adoption — handler registry facade.
	// ES: Adopcion IPGXTaggedRegistry — fachada del registry de handlers.
	bool HasEntryByTag(FGameplayTag Tag) const override;
	int32 GetCount() const override;
	void GetSnapshot(TArray<FGameplayTag>& OutTags) const override;

	TArray<FGameplayTag> GetAllCategories() const;
	FPGXHandlerCacheStats GetCacheStats() const;

	// ============================================================
	// Cache Management
	// ============================================================

	void InvalidateCache();
	void EvictHandler(FGameplayTag EventTag);

	// ============================================================
	// Telemetry & Observability
	// ============================================================

	FPGXHandlerTelemetry GetHandlerTelemetry(FGameplayTag EventTag) const;
	TArray<FPGXHandlerTelemetry> GetAllTelemetry() const;
	void ResetTelemetry();

	FString DumpBlackboxToString() const;

	/** EN: Get direct access to blackbox entries for structured display / ES: Acceso directo a entries del blackbox para display estructurado */
	const TArray<FPGXBlackboxEntry>& GetBlackboxEntries() const;

	FString ExportReport() const;

	// ============================================================
	// Delegates
	// ============================================================

	UPROPERTY(BlueprintAssignable, Category = "PGX|EventHandler|Events")
	FOnPGXHandlerExecuted OnHandlerExecuted;

	FOnPGXHandlerExecutedNative OnHandlerExecutedNative;

	UPROPERTY(BlueprintAssignable, Category = "PGX|EventHandler|Events")
	FOnPGXHandlerNotFound OnHandlerNotFound;

	UPROPERTY(BlueprintAssignable, Category = "PGX|EventHandler|Events")
	FOnPGXHandlerCacheChanged OnHandlerCacheChanged;

#if WITH_EDITOR
	/** EN: Inject transient test config (harness use only) / ES: Inyectar config test transitorio */
	void InjectTestConfig(UPGXEventHandlerConfig* TestConfig);

	/** EN: Clear injected test config, restore discovery / ES: Limpiar config test inyectado */
	void ClearTestConfigs();
#endif

private:
	// EN: Config discovery / ES: Descubrimiento de configuracion
	void DiscoverAndLoadConfig();
	void LoadHandlerTables();
	void RegisterConsoleCommands();
	void UnregisterConsoleCommands();

	// EN: Handler lifecycle management / ES: Gestion del ciclo de vida de handlers
	UPGXEventHandlerBase* AcquireHandler(const FPGXEventHandlerRow& Row, const FPGXEventContext& Context);
	void ReleaseHandler(FGameplayTag EventTag, UPGXEventHandlerBase* Handler, EPGXHandlerLifecycle Lifecycle);
	void EvictLRU();

	// EN: Registration/execution validation helpers / ES: Helpers de validacion de registro/ejecucion
	bool ValidateHandlerRow(const FPGXEventHandlerRow& Row, const FString& SourceName, FString& OutIssue) const;
	bool ValidatePayloadForRow(const FPGXEventHandlerRow& Row, const FInstancedStruct& Payload, FString& OutIssue) const;
	void RemoveCategoryIndexEntry(FGameplayTag CategoryTag, FGameplayTag EventTag);

	// EN: Blackbox recording / ES: Grabacion de blackbox
	void RecordBlackboxEntry(FGameplayTag EventTag, EPGXEventResult Result, const FString& HandlerClassName, double ExecutionTimeMs, const FPGXEventContext& Context, FName FailureReasonName = NAME_None, const FInstancedStruct* Payload = nullptr);

	// EN: Telemetry update / ES: Actualizacion de telemetria
	void UpdateTelemetry(FGameplayTag EventTag, EPGXEventResult Result, double ExecutionTimeMs);

private:
	// EN: Handler registry (tag -> row data) / ES: Registro de handlers (tag -> datos de fila)
	TMap<FGameplayTag, FPGXEventHandlerRow> HandlerRegistry;

	// EN: Handler instance cache / ES: Cache de instancias de handlers
	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UPGXEventHandlerBase>> HandlerCache;

	// EN: Category index for fast lookups / ES: Indice de categoria para busquedas rapidas
	TMap<FGameplayTag, TArray<FGameplayTag>> CategoryIndex;

	// EN: LRU order for cache eviction / ES: Orden LRU para eviccion de cache
	TArray<FGameplayTag> CacheLRUOrder;

	// EN: Telemetry / ES: Telemetria
	TMap<FGameplayTag, FPGXHandlerTelemetry> TelemetryMap;

	// EN: Blackbox / ES: Blackbox
	TArray<FPGXBlackboxEntry> BlackboxBuffer;

	// EN: Recursion safety / ES: Seguridad de recursion
	int32 CurrentExecutionDepth = 0;

	// EN: Active execution stack for cycle diagnostics / ES: Stack activo para diagnostico de ciclos
	TArray<FGameplayTag> ExecutionStack;

	// EN: Cache stats / ES: Estadisticas de cache
	FPGXHandlerCacheStats CacheStats;

	// EN: Config / ES: Configuracion
	UPROPERTY()
	TObjectPtr<UPGXEventHandlerConfig> CachedConfig = nullptr;

	// EN: Registered DataTables / ES: DataTables registrados
	UPROPERTY()
	TArray<TObjectPtr<UDataTable>> RegisteredTables;

	// EN: Console command handles — owned by IConsoleManager, not GC'd. Raw pointers safe per UE console API contract.
	// ES: Handles de comandos de consola — propiedad de IConsoleManager, no GC. Punteros raw seguros.
	TArray<IConsoleCommand*> ConsoleCommands;

	// ── Profile Integration ──

	void ApplyProfileConstraints(const struct FPGXResolvedProfile& Profile);
	void HandleProfileChanged(const struct FPGXResolvedProfile& OldProfile, const struct FPGXResolvedProfile& NewProfile);
};
