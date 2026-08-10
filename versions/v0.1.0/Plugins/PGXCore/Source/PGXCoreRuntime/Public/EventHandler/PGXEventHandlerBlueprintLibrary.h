// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "InstancedStruct.h"
#include "EventHandler/PGXEventHandlerTypes.h"
#include "PGXEventHandlerBlueprintLibrary.generated.h"

class UPGXEventHandlerBase;
class UDataTable;

/**
 * EN: Blueprint function library for the PGX Event Handler System.
 *     Sole BP entry point — subsystem methods are C++ only.
 * ES: Libreria de funciones Blueprint para el Sistema de Event Handler PGX.
 *     Unico punto de entrada BP — metodos del subsistema son solo C++.
 */
UCLASS()
class PGXCORERUNTIME_API UPGXEventHandlerBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ============================================================
	// Core (PGX|EventHandler) — CustomThunk for morphing Payload pin
	// ============================================================

	/**
	 * EN: Execute an event handler with a typed struct payload (morphing pin).
	 * ES: Ejecutar un event handler con payload struct tipado (pin morfico).
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "PGX|EventHandler",
		meta = (WorldContext = "WorldContextObject", CustomStructureParam = "Payload",
			AllowAbstract = "false", DisplayName = "Execute PGX Event"))
	static EPGXEventResult K2_ExecuteEvent(const UObject* WorldContextObject,
		FGameplayTag EventTag, UObject* Instigator, const int32& Payload);
	DECLARE_FUNCTION(execK2_ExecuteEvent);

	/**
	 * EN: Quick-execute with no payload or context (fire-and-forget).
	 * ES: Ejecucion rapida sin payload ni contexto (fire-and-forget).
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|EventHandler",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Quick Execute PGX Event"))
	static EPGXEventResult QuickExecute(const UObject* WorldContextObject, FGameplayTag EventTag);

	// ============================================================
	// Advanced (PGX|EventHandler|Advanced)
	// ============================================================

	/**
	 * EN: Execute an event handler with full context and typed struct payload (morphing pin).
	 * ES: Ejecutar un event handler con contexto completo y payload struct tipado (pin morfico).
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "PGX|EventHandler|Advanced",
		meta = (WorldContext = "WorldContextObject", CustomStructureParam = "Payload",
			AllowAbstract = "false", DisplayName = "Execute PGX Event (Advanced)"))
	static EPGXEventResult K2_ExecuteEventAdvanced(const UObject* WorldContextObject,
		FGameplayTag EventTag, const FPGXEventContext& Context, const int32& Payload);
	DECLARE_FUNCTION(execK2_ExecuteEventAdvanced);

	/** EN: Register all handlers from a DataTable / ES: Registrar todos los handlers desde un DataTable */
	UFUNCTION(BlueprintCallable, Category = "PGX|EventHandler|Advanced",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Register Handler Table"))
	static void RegisterHandlerTable(const UObject* WorldContextObject, UDataTable* InTable);

	/** EN: Unregister all handlers from a DataTable / ES: Desregistrar todos los handlers de un DataTable */
	UFUNCTION(BlueprintCallable, Category = "PGX|EventHandler|Advanced",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Unregister Handler Table"))
	static void UnregisterHandlerTable(const UObject* WorldContextObject, UDataTable* InTable);

	/** EN: Register a single handler programmatically / ES: Registrar un handler individual programaticamente */
	UFUNCTION(BlueprintCallable, Category = "PGX|EventHandler|Advanced",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Register Handler"))
	static void RegisterHandler(const UObject* WorldContextObject, FGameplayTag EventTag,
		TSubclassOf<UPGXEventHandlerBase> HandlerClass,
		EPGXHandlerLifecycle Lifecycle = EPGXHandlerLifecycle::Cached, FGameplayTag CategoryTag = FGameplayTag());

	/** EN: Unregister a handler / ES: Desregistrar un handler */
	UFUNCTION(BlueprintCallable, Category = "PGX|EventHandler|Advanced",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Unregister Handler"))
	static void UnregisterHandler(const UObject* WorldContextObject, FGameplayTag EventTag);

	/** EN: Invalidate the entire handler cache / ES: Invalidar toda la cache de handlers */
	UFUNCTION(BlueprintCallable, Category = "PGX|EventHandler|Advanced",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Invalidate Handler Cache"))
	static void InvalidateHandlerCache(const UObject* WorldContextObject);

	/** EN: Evict a specific handler from cache / ES: Desalojar un handler especifico de la cache */
	UFUNCTION(BlueprintCallable, Category = "PGX|EventHandler|Advanced",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Evict Handler"))
	static void EvictHandler(const UObject* WorldContextObject, FGameplayTag EventTag);

	/** EN: Execute a sequence of events / ES: Ejecutar una secuencia de eventos */
	UFUNCTION(BlueprintCallable, Category = "PGX|EventHandler|Advanced",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Execute Event Sequence"))
	static EPGXEventResult ExecuteEventSequence(const UObject* WorldContextObject,
		const TArray<FGameplayTag>& EventTags, const FPGXEventContext& Context, bool bStopOnFailure = true);

	/** EN: Validate conditions for a set of event tags / ES: Validar condiciones para un conjunto de tags */
	UFUNCTION(BlueprintPure, Category = "PGX|EventHandler|Advanced",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Validate Event Conditions"))
	static bool ValidateEventConditions(const UObject* WorldContextObject,
		const TArray<FGameplayTag>& EventTags, const FPGXEventContext& Context);

	// ============================================================
	// Query (PGX|EventHandler|Query)
	// ============================================================

	/** EN: Check if a handler can execute for the given event tag / ES: Verificar si un handler puede ejecutar */
	UFUNCTION(BlueprintPure, Category = "PGX|EventHandler|Query",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Can Execute Event"))
	static bool CanExecuteEvent(const UObject* WorldContextObject, FGameplayTag EventTag, const FPGXEventContext& Context);

	/** EN: Check if a handler is registered for the given tag / ES: Verificar si hay un handler registrado */
	UFUNCTION(BlueprintPure, Category = "PGX|EventHandler|Query",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Is Event Registered"))
	static bool IsEventRegistered(const UObject* WorldContextObject, FGameplayTag EventTag);

	/** EN: Get handler info for an event tag / ES: Obtener info del handler para un tag de evento */
	UFUNCTION(BlueprintPure, Category = "PGX|EventHandler|Query",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Get Handler Info"))
	static FPGXEventHandlerInfo GetHandlerInfo(const UObject* WorldContextObject, FGameplayTag EventTag);

	/** EN: Get all handlers in a category / ES: Obtener todos los handlers de una categoria */
	UFUNCTION(BlueprintCallable, Category = "PGX|EventHandler|Query",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Get Handlers By Category"))
	static TArray<FPGXEventHandlerInfo> GetHandlersByCategory(const UObject* WorldContextObject, FGameplayTag CategoryTag);

	/** EN: Get all registered event tags / ES: Obtener todos los tags de eventos registrados */
	UFUNCTION(BlueprintCallable, Category = "PGX|EventHandler|Query",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Get All Registered Tags"))
	static TArray<FGameplayTag> GetAllRegisteredTags(const UObject* WorldContextObject);

	/** EN: Get all handler categories / ES: Obtener todas las categorias de handlers */
	UFUNCTION(BlueprintCallable, Category = "PGX|EventHandler|Query",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Get All Categories"))
	static TArray<FGameplayTag> GetAllCategories(const UObject* WorldContextObject);

	/** EN: Get cache statistics / ES: Obtener estadisticas de cache */
	UFUNCTION(BlueprintPure, Category = "PGX|EventHandler|Query",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Get Handler Cache Stats"))
	static FPGXHandlerCacheStats GetHandlerCacheStats(const UObject* WorldContextObject);

	/** EN: Create an event context / ES: Crear un contexto de evento */
	UFUNCTION(BlueprintPure, Category = "PGX|EventHandler|Query",
		meta = (DisplayName = "Make Event Context"))
	static FPGXEventContext MakeEventContext(UObject* Instigator, UObject* Target);

	/** EN: Check if an event result represents success / ES: Verificar si un resultado es exitoso */
	UFUNCTION(BlueprintPure, Category = "PGX|EventHandler|Query",
		meta = (DisplayName = "Was Successful (Event Result)"))
	static bool WasSuccessful(EPGXEventResult Result);

	/** EN: Get a human-readable string for an event result / ES: Obtener string legible */
	UFUNCTION(BlueprintPure, Category = "PGX|EventHandler|Query",
		meta = (DisplayName = "Get Result String"))
	static FString GetResultString(EPGXEventResult Result);

	// ============================================================
	// Debug (PGX|EventHandler|Debug)
	// ============================================================

	/** EN: Get telemetry for a specific handler / ES: Obtener telemetria para un handler especifico */
	UFUNCTION(BlueprintPure, Category = "PGX|EventHandler|Debug",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Get Handler Telemetry"))
	static FPGXHandlerTelemetry GetHandlerTelemetry(const UObject* WorldContextObject, FGameplayTag EventTag);

	/** EN: Get telemetry for all handlers / ES: Obtener telemetria de todos los handlers */
	UFUNCTION(BlueprintCallable, Category = "PGX|EventHandler|Debug",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Get All Handler Telemetry"))
	static TArray<FPGXHandlerTelemetry> GetAllHandlerTelemetry(const UObject* WorldContextObject);

	/** EN: Reset all telemetry counters / ES: Resetear todos los contadores de telemetria */
	UFUNCTION(BlueprintCallable, Category = "PGX|EventHandler|Debug",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Reset Handler Telemetry"))
	static void ResetHandlerTelemetry(const UObject* WorldContextObject);

	/** EN: Get blackbox entries (copy for BP safety) / ES: Obtener entries del blackbox (copia para seguridad BP) */
	UFUNCTION(BlueprintCallable, Category = "PGX|EventHandler|Debug",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Get Blackbox Entries"))
	static TArray<FPGXBlackboxEntry> GetBlackboxEntries(const UObject* WorldContextObject);

	/** EN: Dump blackbox to a formatted string / ES: Volcar blackbox a string formateado */
	UFUNCTION(BlueprintPure, Category = "PGX|EventHandler|Debug",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Dump Blackbox To String"))
	static FString DumpBlackboxToString(const UObject* WorldContextObject);

	/** EN: Export a full handler report / ES: Exportar reporte completo de handlers */
	UFUNCTION(BlueprintCallable, Category = "PGX|EventHandler|Debug",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Export Handler Report"))
	static FString ExportReport(const UObject* WorldContextObject);

	// ============================================================
	// C++ only (used by test utilities and native integrations — no UFUNCTION)
	// ============================================================

	static EPGXEventResult ExecuteEvent(const UObject* WorldContextObject, FGameplayTag EventTag,
		UObject* Instigator, const FInstancedStruct& Payload);

	static EPGXEventResult ExecuteEventAdvanced(const UObject* WorldContextObject, FGameplayTag EventTag,
		const FPGXEventContext& Context, const FInstancedStruct& Payload);
};
