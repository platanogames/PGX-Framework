// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Base/PGXGameInstanceSubsystem.h"
#include "InstancedStruct.h"
#include "Messages/PGXMessage.h"
#include "Messages/PGXMessageDelegates.h"
#include "PGXMessageSubsystem.generated.h"

class UPGXMessageConfig;
struct IConsoleCommand;

/**
 * EN: PGX Message Subsystem — pub/sub message bus for cross-system communication.
 *     Reimplements Lyra's GameplayMessageSubsystem pattern with PGX extensions:
 *     config DA, message history, console commands, telemetry, and inspector support.
 *     All L2 plugins communicate through this bus (Architecture Invariant #2).
 * ES: Subsistema de Mensajes PGX — bus pub/sub para comunicacion entre sistemas.
 *     Reimplementa el patron GameplayMessageSubsystem de Lyra con extensiones PGX:
 *     config DA, historial de mensajes, comandos de consola, telemetria y soporte inspector.
 *     Todos los plugins L2 se comunican a traves de este bus (Invariante de Arquitectura #2).
 */
UCLASS()
class PGXCORERUNTIME_API UPGXMessageSubsystem : public UPGXGameInstanceSubsystem
{
	GENERATED_BODY()

	friend struct FPGXMessageListenerHandle;
	friend class UPGXAsyncListenForMessage;

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface

	/** EN: Get the subsystem from a world context / ES: Obtener el subsistema desde un contexto de mundo */
	static UPGXMessageSubsystem* Get(const UObject* WorldContextObject);

	// ============================================================
	// Template C++ API (Lyra pattern)
	// ============================================================

	/**
	 * EN: Broadcast a typed message on the specified channel.
	 * ES: Emitir un mensaje tipado en el canal especificado.
	 */
	template <typename FMessageStructType>
	void BroadcastMessage(FGameplayTag Channel, const FMessageStructType& Message)
	{
		const UScriptStruct* StructType = TBaseStructure<FMessageStructType>::Get();
		BroadcastMessageInternal(Channel, StructType, &Message);
	}

	/**
	 * EN: Register a listener with a TFunction callback.
	 * ES: Registrar un listener con un callback TFunction.
	 */
	template <typename FMessageStructType>
	FPGXMessageListenerHandle RegisterListener(FGameplayTag Channel,
		TFunction<void(FGameplayTag, const FMessageStructType&)>&& Callback,
		EPGXMessageMatch MatchType = EPGXMessageMatch::ExactMatch)
	{
		auto ThunkCallback = [InnerCallback = MoveTemp(Callback)](FGameplayTag ActualTag, const UScriptStruct* /*SenderStructType*/, const void* SenderPayload)
		{
			InnerCallback(ActualTag, *reinterpret_cast<const FMessageStructType*>(SenderPayload));
		};
		const UScriptStruct* StructType = TBaseStructure<FMessageStructType>::Get();
		return RegisterListenerInternal(Channel, MoveTemp(ThunkCallback), StructType, MatchType);
	}

	/**
	 * EN: Register a listener with a member function (weak object check).
	 * ES: Registrar un listener con una funcion miembro (con verificacion weak object).
	 */
	template <typename FMessageStructType, typename TOwner = UObject>
	FPGXMessageListenerHandle RegisterListener(FGameplayTag Channel, TOwner* Object,
		void(TOwner::* Function)(FGameplayTag, const FMessageStructType&),
		EPGXMessageMatch MatchType = EPGXMessageMatch::ExactMatch)
	{
		TWeakObjectPtr<TOwner> WeakObject(Object);
		return RegisterListener<FMessageStructType>(Channel,
			[WeakObject, Function](FGameplayTag ActualChannel, const FMessageStructType& Payload)
			{
				if (TOwner* StrongObject = WeakObject.Get())
				{
					(StrongObject->*Function)(ActualChannel, Payload);
				}
			}, MatchType);
	}

	/**
	 * EN: Unregister a previously registered listener.
	 * ES: Desregistrar un listener previamente registrado.
	 */
	void UnregisterListener(FPGXMessageListenerHandle Handle);

	// ============================================================
	// Blueprint API
	// ============================================================

	/**
	 * EN: Broadcast a message with FInstancedStruct payload (C++ use only, not exposed to BP palette).
	 * ES: Emitir un mensaje con payload FInstancedStruct (uso C++ solamente, no expuesto a palette BP).
	 */
	void BroadcastMessageStruct(FGameplayTag Channel, const FInstancedStruct& Payload);

	/**
	 * EN: Broadcast a typed message (wildcard struct pin). Same pattern as Lyra's K2_BroadcastMessage.
	 *     The Message pin morphs to match any USTRUCT you connect.
	 *     For simple Channel+Sender with no struct, use BroadcastPGXMessage in PGXMessageBlueprintLibrary.
	 * ES: Emitir un mensaje tipado (pin struct wildcard). Mismo patron que K2_BroadcastMessage de Lyra.
	 *     El pin Message se adapta a cualquier USTRUCT que conectes.
	 *     Para Channel+Sender simple sin struct, usar BroadcastPGXMessage en PGXMessageBlueprintLibrary.
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "PGX|Messages",
		meta = (CustomStructureParam = "Message", AllowAbstract = "false",
			DisplayName = "Broadcast PGX Message (Struct)"))
	void K2_BroadcastMessage(FGameplayTag Channel, const int32& Message);
	DECLARE_FUNCTION(execK2_BroadcastMessage);

	// ============================================================
	// PGX Extensions (Query API)
	// ============================================================

	// EN: Query API — C++ accessible. BP uses UPGXMessageBlueprintLibrary (sole BP entry point per S4).
	// ES: API de consulta — accesible desde C++. BP usa UPGXMessageBlueprintLibrary (unico punto de entrada BP segun S4).

	/** EN: Get recent message history / ES: Obtener historial reciente de mensajes */
	TArray<FPGXMessageRecord> GetMessageHistory(FGameplayTag Channel, int32 MaxResults = 10) const;

	/** EN: Clear all message history records / ES: Limpiar todos los registros del historial de mensajes */
	void ClearHistory();

	/** EN: Get the number of active listeners on a channel / ES: Obtener numero de listeners activos en un canal */
	int32 GetListenerCount(FGameplayTag Channel) const;

	/** EN: Get the total number of active listeners across all channels / ES: Obtener total de listeners activos */
	int32 GetTotalListenerCount() const;

	/** EN: Get all channels that have active listeners / ES: Obtener todos los canales con listeners activos */
	TArray<FGameplayTag> GetAllActiveChannels() const;

	/** EN: Check if a channel has any listeners / ES: Verificar si un canal tiene listeners */
	bool IsChannelActive(FGameplayTag Channel) const;

	/** EN: Get aggregate message stats / ES: Obtener estadisticas agregadas de mensajes */
	FPGXMessageStats GetStats() const;

	// ============================================================
	// Delegates
	// ============================================================

	/** EN: Fires on every broadcast (Blueprint) / ES: Se dispara en cada broadcast */
	UPROPERTY(BlueprintAssignable, Category = "PGX|Messages|Events")
	FOnPGXMessageBroadcast OnMessageBroadcast;

	/** EN: Fires on every broadcast (Native) / ES: Se dispara en cada broadcast (Nativo) */
	FOnPGXMessageBroadcastNative OnMessageBroadcastNative;

	/** EN: Fires when a listener is registered / ES: Se dispara al registrar un listener */
	UPROPERTY(BlueprintAssignable, Category = "PGX|Messages|Events")
	FOnPGXListenerRegistered OnListenerRegistered;

	/** EN: Fires when a listener is unregistered / ES: Se dispara al desregistrar un listener */
	UPROPERTY(BlueprintAssignable, Category = "PGX|Messages|Events")
	FOnPGXListenerUnregistered OnListenerUnregistered;

#if WITH_EDITOR
	/** EN: Inject transient test config (harness use only) / ES: Inyectar config test transitorio */
	void InjectTestConfig(UPGXMessageConfig* TestConfig);

	/** EN: Clear injected test config, restore discovery / ES: Limpiar config test inyectado */
	void ClearTestConfigs();
#endif

protected:
	// EN: Public-facing broadcast orchestrator. Honors same-frame deferred-dispatch ordering —
	//     when called during a parent dispatch (bIsDispatching==true), enqueues to
	//     PendingBroadcasts and returns; the parent dispatch drains the queue FIFO after its
	//     fan-out completes. Top-level callers run DispatchOne directly. bIsTestOrigin marks
	//     the resulting history record.
	// ES: Orquestador publico de broadcast. Honra same-frame deferred-dispatch ordering — cuando
	//     se llama durante un dispatch padre (bIsDispatching==true), encola a PendingBroadcasts y
	//     retorna; el dispatch padre drena la cola FIFO tras completar su fan-out.
	void BroadcastMessageInternal(FGameplayTag Channel, const UScriptStruct* StructType, const void* MessageBytes, bool bIsTestOrigin = false);

	// EN: Single-broadcast worker. Contains recursion guard + listener fan-out + stats + history.
	//     Called from BroadcastMessageInternal for both top-level dispatch and queued drain.
	//     Does NOT consult bIsDispatching — that gate lives in the orchestrator.
	// ES: Worker de un solo broadcast. Contiene recursion guard + listener fan-out + stats + history.
	void DispatchOne(FGameplayTag Channel, const UScriptStruct* StructType, const void* MessageBytes, bool bIsTestOrigin);

	// Internal listener registration
	FPGXMessageListenerHandle RegisterListenerInternal(FGameplayTag Channel,
		TFunction<void(FGameplayTag, const UScriptStruct*, const void*)>&& Callback,
		const UScriptStruct* StructType, EPGXMessageMatch MatchType);

	void UnregisterListenerInternal(FGameplayTag Channel, int32 HandleID);

private:
	// EN: Discover and load config DA from AssetRegistry / ES: Descubrir y cargar config DA desde AssetRegistry
	void DiscoverAndLoadConfig();

	// EN: Register console commands / ES: Registrar comandos de consola
	void RegisterConsoleCommands();

	// EN: Unregister console commands / ES: Desregistrar comandos de consola
	void UnregisterConsoleCommands();

	// EN: Add a record to history. bIsTestOrigin propagates from BroadcastMessageInternal.
	// ES: Agregar un registro al historial. bIsTestOrigin se propaga desde BroadcastMessageInternal.
	void AddToHistory(FGameplayTag Channel, const UScriptStruct* StructType, int32 ListenersNotified, bool bIsTestOrigin = false);

	// EN: Recompute EffectiveMaxHistory from Config DA + Profile + Settings (single source of truth).
	//     Called on Initialize, ApplyProfileConstraints, HandleProfileChanged, InjectTestConfig, ClearTestConfigs.
	//     Trims MessageHistory if the new bound is smaller than current size.
	// ES: Recomputar EffectiveMaxHistory desde Config DA + Profile + Settings (fuente unica de verdad).
	//     Llamado en Initialize, ApplyProfileConstraints, HandleProfileChanged, InjectTestConfig, ClearTestConfigs.
	//     Recorta MessageHistory si el nuevo limite es menor que el tamano actual.
	void RecomputeEffectiveMaxHistory();

private:
	// EN: Listener storage (Lyra pattern) / ES: Almacenamiento de listeners (patron Lyra)
	struct FChannelListenerList
	{
		TArray<FPGXMessageListenerData> Listeners;
		int32 HandleID = 0;
	};

	TMap<FGameplayTag, FChannelListenerList> ListenerMap;

	// EN: Message history (PGX extension) / ES: Historial de mensajes (extension PGX)
	TArray<FPGXMessageRecord> MessageHistory;

	// EN: Statistics / ES: Estadisticas
	FPGXMessageStats Stats;

	// EN: Effective history retention bound resolved from Config DA + Profile + Settings fallback.
	//     Single source of truth for AddToHistory. Recomputed on init + profile change.
	//     Min(Config.MaxMessageHistory or Settings.EmergencyHistoryFallback, Profile.MessageBudgets.MaxMessageHistory).
	//     The resolved value enforces platform-profile history clamping.
	// ES: Limite efectivo de retencion de historial resuelto desde Config DA + Profile + Settings fallback.
	//     Fuente unica de verdad para AddToHistory. Recomputado en init + cambio de profile.
	int32 EffectiveMaxHistory = 0;

	// EN: Current broadcast nesting depth. Incremented at DispatchOne entry (post guard check),
	//     decremented at exit (RAII via ON_SCOPE_EXIT). Cap semantics — with
	//     Config.MaxBroadcastRecursionDepth=N: depths 1..N execute normally; the (N+1)th
	//     nesting attempt is rejected at entry while depth==N (before increment to N+1).
	//     With deferred-dispatch this depth normally stays at 1 since nested
	//     broadcasts queue rather than recurse; the guard remains as a sanity backstop should
	//     DispatchOne be invoked through a future direct path.
	//     Stats.MaxBroadcastDepth tracks high-water mark for inspector telemetry.
	// ES: Profundidad actual de anidamiento. Con MaxBroadcastRecursionDepth=N: depths 1..N
	//     ejecutan; el intento (N+1) es rechazado al entry mientras depth==N (antes del
	//     increment a N+1). Con deferred-dispatch normalmente queda en 1.
	int32 BroadcastDepth = 0;

	// EN: True while a top-level dispatch is in flight (BroadcastMessageInternal owns the dispatch
	//     scope). Nested calls during this window enqueue to PendingBroadcasts. Cleared via
	//     ON_SCOPE_EXIT at orchestrator return. same-frame deferred-dispatch ordering invariant.
	// ES: True mientras un dispatch top-level esta en vuelo. Nested calls durante esta ventana
	//     encolan a PendingBroadcasts. Cleared via ON_SCOPE_EXIT al retorno del orchestrator.
	bool bIsDispatching = false;

	// EN: Broadcasts queued while a parent dispatch is in flight. Drained FIFO after the parent
	//     fan-out completes — guarantees same-frame deferred-dispatch ordering. Queue cap is
	//     Config.MaxBroadcastQueueDepth; oversize broadcasts are dropped with LogError.
	//     UPROPERTY(Transient) keeps GC tracking for FInstancedStruct payloads but excludes from
	//     save serialization. Stats.MaxQueueDepth tracks high-water mark.
	// ES: Broadcasts encolados mientras un dispatch padre esta en vuelo. Drenados FIFO tras
	//     completar el fan-out padre — garantiza same-frame deferred-dispatch ordering.
	UPROPERTY(Transient)
	TArray<FPGXPendingBroadcast> PendingBroadcasts;

	// EN: Configuration / ES: Configuracion
	UPROPERTY()
	TObjectPtr<UPGXMessageConfig> CachedConfig = nullptr;

	// EN: Console command handles — owned by IConsoleManager, not GC'd. Raw pointers safe per UE console API contract.
	// ES: Handles de comandos de consola — propiedad de IConsoleManager, no GC. Punteros raw seguros.
	TArray<IConsoleCommand*> ConsoleCommands;

	// ── Profile Integration ──

	void ApplyProfileConstraints(const struct FPGXResolvedProfile& Profile);
	void HandleProfileChanged(const struct FPGXResolvedProfile& OldProfile, const struct FPGXResolvedProfile& NewProfile);
};
