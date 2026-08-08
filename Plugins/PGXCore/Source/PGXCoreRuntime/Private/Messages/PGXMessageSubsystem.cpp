// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Messages/PGXMessageSubsystem.h"
#include "Messages/PGXMessageConfig.h"
#include "Messages/PGXMessageSettings.h"
#include "Messages/PGXMessageLog.h"
#include "Profile/PGXProfileSubsystem.h"
#include "Profile/PGXPlatformConfig.h"
#include "Utils/PGXConfigResolution.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/ScopeExit.h"
#include "UObject/ScriptMacros.h"
#include "UObject/Stack.h"

// EN: PGX Message Subsystem — Lyra pattern reimplementation with PGX extensions
// ES: Subsistema de Mensajes PGX — Reimplementacion del patron Lyra con extensiones PGX

// ============================================================
// FPGXMessageListenerHandle
// ============================================================

FPGXMessageListenerHandle::FPGXMessageListenerHandle(UPGXMessageSubsystem* InSubsystem, FGameplayTag InChannel, int32 InID)
	: Subsystem(static_cast<UObject*>(InSubsystem)), Channel(InChannel), ID(InID)
{
}

void FPGXMessageListenerHandle::Unregister()
{
	if (UPGXMessageSubsystem* StrongSubsystem = Cast<UPGXMessageSubsystem>(Subsystem.Get()))
	{
		StrongSubsystem->UnregisterListener(*this);
	}
	// EN: Always reset local state — handle should never appear valid after Unregister
	// ES: Siempre resetear estado local — handle nunca debe parecer valido tras Unregister
	Subsystem.Reset();
	Channel = FGameplayTag();
	ID = 0;
}

// ============================================================
// Static Accessor
// ============================================================

UPGXMessageSubsystem* UPGXMessageSubsystem::Get(const UObject* WorldContextObject)
{
	if (!IsValid(WorldContextObject))
	{
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!IsValid(World) || !IsValid(World->GetGameInstance()))
	{
		return nullptr;
	}

	return UGameInstance::GetSubsystem<UPGXMessageSubsystem>(World->GetGameInstance());
}

// ============================================================
// Lifecycle
// ============================================================

void UPGXMessageSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// EN: NOTE: This subsystem runs on game thread only. No synchronization required.
	// ES: NOTA: Este subsistema corre solo en game thread. No requiere sincronizacion.
	Super::Initialize(Collection);

	DiscoverAndLoadConfig();
	RegisterConsoleCommands();

	// ── Profile Integration ──
	if (UGameInstance* GI = GetGameInstance())
	{
		if (auto* ProfileSS = GI->GetSubsystem<UPGXProfileSubsystem>())
		{
			if (ProfileSS->IsProfileResolved())
			{
				ApplyProfileConstraints(ProfileSS->GetResolvedProfile());
			}
			ProfileSS->OnProfileChangedNative.AddUObject(this, &ThisClass::HandleProfileChanged);
		}
	}

	// EN: Compute EffectiveMaxHistory after Config + Profile resolution (the platform-profile clamp policy).
	// ES: Computar EffectiveMaxHistory tras resolucion de Config + Profile (the platform-profile clamp policy).
	RecomputeEffectiveMaxHistory();

	UE_LOG(LogPGXMessage, Log, TEXT("PGX MessageSubsystem initialized. Config: %s, EffectiveMaxHistory: %d"),
		IsValid(CachedConfig) ? *CachedConfig->GetName() : TEXT("Default"), EffectiveMaxHistory);
}

void UPGXMessageSubsystem::Deinitialize()
{
	// ── Profile Unbind ──
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UPGXProfileSubsystem* ProfileSS = GI->GetSubsystem<UPGXProfileSubsystem>())
		{
			ProfileSS->OnProfileChangedNative.RemoveAll(this);
		}
	}

	UnregisterConsoleCommands();

	ListenerMap.Reset();
	MessageHistory.Empty();
	PendingBroadcasts.Empty();
	bIsDispatching = false;
	BroadcastDepth = 0;
	Stats = FPGXMessageStats();
	CachedConfig = nullptr;

	UE_LOG(LogPGXMessage, Log, TEXT("PGX MessageSubsystem deinitialized."));

	Super::Deinitialize();
}

// ============================================================
// Config Discovery
// ============================================================

void UPGXMessageSubsystem::DiscoverAndLoadConfig()
{
	// EN: Settings-first resolution with AssetRegistry fallback (deprecated)
	// ES: Resolucion Settings-first con fallback a AssetRegistry (deprecated)
	const UPGXMessageSettings* Settings = GetDefault<UPGXMessageSettings>();
	CachedConfig = PGX::ResolveSingleConfig<UPGXMessageConfig>(Settings->ActiveConfig, TEXT("Message"));

	if (IsValid(CachedConfig))
	{
		UE_LOG(LogPGXMessage, Log, TEXT("PGXMessageConfig loaded: %s (History=%d, LogBroadcasts=%d, PartialMatch=%d)"),
			*CachedConfig->GetName(), CachedConfig->MaxMessageHistory,
			CachedConfig->bLogBroadcasts, CachedConfig->bEnablePartialMatching);
	}
}

// ============================================================
// Core: Broadcast (Internal)
// ============================================================

void UPGXMessageSubsystem::BroadcastMessageInternal(FGameplayTag Channel, const UScriptStruct* StructType, const void* MessageBytes, bool bIsTestOrigin)
{
	// EN: Orchestrator. same-frame deferred-dispatch ordering: when called during a parent dispatch,
	//     enqueue and return; the parent drains the queue FIFO after its fan-out completes.
	//     Top-level call performs DispatchOne directly then drains any pending broadcasts.
	// ES: Orquestador. same-frame deferred-dispatch ordering: si se llama durante un dispatch padre,
	//     encola y retorna; el padre drena la cola FIFO tras su fan-out. Top-level dispatchea
	//     directamente y luego drena la cola.

	// Early sanity (cheap rejects before any state mutation).
	if (!Channel.IsValid())
	{
		UE_LOG(LogPGXMessage, Warning, TEXT("BroadcastMessage called with invalid channel tag."));
		return;
	}

	if (!StructType)
	{
		UE_LOG(LogPGXMessage, Warning, TEXT("BroadcastMessage called with null StructType on channel %s."), *Channel.ToString());
		return;
	}

	// the deferred-dispatch ordering invariant deferred-dispatch: queue if a parent dispatch is in flight.
	if (bIsDispatching)
	{
		const int32 MaxQueue = IsValid(CachedConfig) ? CachedConfig->MaxBroadcastQueueDepth : 64;
		if (PendingBroadcasts.Num() >= MaxQueue)
		{
			UE_LOG(LogPGXMessage, Error,
				TEXT("Broadcast queue cap reached (%d) on channel %s. Dropping nested broadcast to prevent unbounded growth."),
				MaxQueue, *Channel.ToString());
			return;
		}

		FPGXPendingBroadcast Pending;
		Pending.Channel = Channel;
		Pending.Payload.InitializeAs(StructType, reinterpret_cast<const uint8*>(MessageBytes));
		Pending.bIsTestOrigin = bIsTestOrigin;
		PendingBroadcasts.Emplace(MoveTemp(Pending));
		Stats.MaxQueueDepth = FMath::Max(Stats.MaxQueueDepth, PendingBroadcasts.Num());
		return;
	}

	// Top-level dispatch.
	bIsDispatching = true;
	ON_SCOPE_EXIT { bIsDispatching = false; };

	DispatchOne(Channel, StructType, MessageBytes, bIsTestOrigin);

	// Drain queue FIFO. Each drained dispatch may enqueue more; they go to the back. Loop ends
	// when the queue is empty (or a future drained dispatch hits the queue-cap drop path).
	while (PendingBroadcasts.Num() > 0)
	{
		FPGXPendingBroadcast Next = MoveTemp(PendingBroadcasts[0]);
		PendingBroadcasts.RemoveAt(0);
		DispatchOne(Next.Channel, Next.Payload.GetScriptStruct(), Next.Payload.GetMemory(), Next.bIsTestOrigin);
	}
}

void UPGXMessageSubsystem::DispatchOne(FGameplayTag Channel, const UScriptStruct* StructType, const void* MessageBytes, bool bIsTestOrigin)
{
	// EN: Single-broadcast worker. Recursion guard semantics — with MaxDepth=N, dispatches at
	//     depths 1..N execute normally; the (N+1)th nesting attempt is rejected at entry while
	//     current depth == N (before the increment that would push to N+1). With deferred
	//     dispatch in BroadcastMessageInternal, depth normally stays at 1; the guard remains
	//     as a sanity backstop should DispatchOne ever be reached via a non-orchestrator path.
	//     Resolution chain mirrors EmergencyHistoryFallback (Step 2): CachedConfig →
	//     Settings.EmergencyMaxBroadcastRecursionDepth → 0 (fail-open guard, queue cap still bounds).
	// ES: Worker de un solo broadcast. Con MaxDepth=N: depths 1..N ejecutan; el intento (N+1)
	//     es rechazado al entry mientras depth == N. Resolution: CachedConfig →
	//     Settings.EmergencyMaxBroadcastRecursionDepth → 0 (fail-open).
	int32 MaxDepth = 0;
	if (IsValid(CachedConfig))
	{
		MaxDepth = CachedConfig->MaxBroadcastRecursionDepth;
	}
	else
	{
		const UPGXMessageSettings* Settings = GetDefault<UPGXMessageSettings>();
		MaxDepth = Settings ? Settings->EmergencyMaxBroadcastRecursionDepth : 0;
		if (MaxDepth > 0)
		{
			UE_LOG(LogPGXMessage, Warning,
				TEXT("DispatchOne: CachedConfig invalid; using Settings.EmergencyMaxBroadcastRecursionDepth=%d"),
				MaxDepth);
		}
	}

	if (MaxDepth > 0 && BroadcastDepth >= MaxDepth)
	{
		UE_LOG(LogPGXMessage, Error,
			TEXT("Broadcast recursion guard tripped on channel %s (current_depth=%d, max_levels=%d). Attempted (N+1)th nesting rejected."),
			*Channel.ToString(), BroadcastDepth, MaxDepth);
		return;
	}

	++BroadcastDepth;
	Stats.MaxBroadcastDepth = FMath::Max(Stats.MaxBroadcastDepth, BroadcastDepth);
	ON_SCOPE_EXIT { --BroadcastDepth; };

	const bool bShouldLog = IsValid(CachedConfig) ? CachedConfig->bLogBroadcasts : false;

	if (bShouldLog)
	{
		FString HumanReadable;
		if (StructType)
		{
			StructType->ExportText(HumanReadable, MessageBytes, nullptr, nullptr, PPF_None, nullptr);
		}
		UE_LOG(LogPGXMessage, Log, TEXT("BroadcastMessage: Channel=%s Type=%s Data=%s"),
			*Channel.ToString(), StructType ? *StructType->GetName() : TEXT("None"), *HumanReadable);
	}

	// EN: Global partial-matching gate (GAP-C). When CachedConfig disables partial
	//     matching, we skip parent-tag iteration entirely — only listeners directly
	//     subscribed to the broadcast channel fire, regardless of their per-listener
	//     MatchType. This honors the AAA invariant that broad fan-out is opt-in at both
	//     global and per-listener levels.
	// ES: Compuerta global de matching parcial (GAP-C). Cuando CachedConfig
	//     deshabilita el matching parcial, saltamos la iteracion por tags padre — solo
	//     listeners suscritos directamente al canal del broadcast disparan,
	//     independientemente de su MatchType.
	const bool bGlobalPartialAllowed = IsValid(CachedConfig) ? CachedConfig->bEnablePartialMatching : true;

	int32 ListenersNotified = 0;
	bool bOnInitialTag = true;

	for (FGameplayTag CtxTag = Channel; CtxTag.IsValid(); CtxTag = CtxTag.RequestDirectParent())
	{
		// EN: When global partial matching is disabled, stop after the initial (exact) tag.
		// ES: Cuando el matching parcial global esta deshabilitado, detenemos tras el tag inicial (exacto).
		if (!bOnInitialTag && !bGlobalPartialAllowed)
		{
			break;
		}

		if (const FChannelListenerList* pList = ListenerMap.Find(CtxTag))
		{
			// EN: Copy in case there are removals while handling callbacks
			// ES: Copia por si hay desregistros durante los callbacks
			TArray<FPGXMessageListenerData> ListenerArray(pList->Listeners);

			for (const FPGXMessageListenerData& Listener : ListenerArray)
			{
				if (bOnInitialTag || (Listener.MatchType == EPGXMessageMatch::PartialMatch))
				{
					if (Listener.bHadValidType && !Listener.ListenerStructType.IsValid())
					{
						UE_LOG(LogPGXMessage, Warning, TEXT("Listener struct type invalidated on channel %s. Removing."), *Channel.ToString());
						UnregisterListenerInternal(CtxTag, Listener.HandleID);
						continue;
					}

					if (!Listener.bHadValidType || StructType->IsChildOf(Listener.ListenerStructType.Get()))
					{
						Listener.ReceivedCallback(Channel, StructType, MessageBytes);
						++ListenersNotified;
					}
					else
					{
						UE_LOG(LogPGXMessage, Error, TEXT("Struct type mismatch on channel %s (broadcast: %s, listener expected: %s)"),
							*Channel.ToString(),
							*StructType->GetPathName(),
							*Listener.ListenerStructType->GetPathName());
					}
				}
			}
		}
		bOnInitialTag = false;
	}

	// EN: Update stats and history. MaxFanOutOnSingleBroadcast tracks broadest dispatch
	//     for inspector visibility (fan-out diagnostics).
	// ES: Actualizar estadisticas e historial. MaxFanOutOnSingleBroadcast registra el dispatch
	//     mas amplio para visibilidad del inspector.
	Stats.TotalBroadcasts++;
	Stats.TotalListenersNotified += ListenersNotified;
	Stats.MaxFanOutOnSingleBroadcast = FMath::Max(Stats.MaxFanOutOnSingleBroadcast, ListenersNotified);
	AddToHistory(Channel, StructType, ListenersNotified, bIsTestOrigin);

	// EN: Fire delegates / ES: Disparar delegates
	const FString TypeName = StructType ? StructType->GetName() : TEXT("None");
	const double Timestamp = FPlatformTime::Seconds();
	OnMessageBroadcast.Broadcast(Channel, TypeName);
	OnMessageBroadcastNative.Broadcast(Channel, StructType, Timestamp);
}

// ============================================================
// Blueprint API
// ============================================================

void UPGXMessageSubsystem::BroadcastMessageStruct(FGameplayTag Channel, const FInstancedStruct& Payload)
{
	if (!Channel.IsValid()) { return; }

	const UScriptStruct* StructType = Payload.GetScriptStruct();
	const void* PayloadPtr = Payload.GetMemory();

	if (StructType && PayloadPtr)
	{
		BroadcastMessageInternal(Channel, StructType, PayloadPtr);
	}
	else
	{
		// EN: Broadcast with no payload — still notify listeners (some only care about the tag)
		// ES: Broadcast sin payload — aun notifica listeners (algunos solo les importa el tag)
		BroadcastMessageInternal(Channel, nullptr, nullptr);
	}
}

// ============================================================
// Blueprint CustomThunk (Legacy)
// ============================================================

void UPGXMessageSubsystem::K2_BroadcastMessage(FGameplayTag /*Channel*/, const int32& /*Message*/)
{
	// EN: Never called directly, the exec version below will be hit instead
	// ES: Nunca se llama directamente, la version exec de abajo se ejecuta
	checkNoEntry();
}

DEFINE_FUNCTION(UPGXMessageSubsystem::execK2_BroadcastMessage)
{
	P_GET_STRUCT(FGameplayTag, Channel);

	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* MessagePtr = Stack.MostRecentPropertyAddress;
	FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);

	P_FINISH;

	if (ensure(StructProp && StructProp->Struct && MessagePtr))
	{
		P_THIS->BroadcastMessageInternal(Channel, StructProp->Struct, MessagePtr);
	}
}

// ============================================================
// Core: Register / Unregister
// ============================================================

FPGXMessageListenerHandle UPGXMessageSubsystem::RegisterListenerInternal(
	FGameplayTag Channel,
	TFunction<void(FGameplayTag, const UScriptStruct*, const void*)>&& Callback,
	const UScriptStruct* StructType,
	EPGXMessageMatch MatchType)
{
	FChannelListenerList& List = ListenerMap.FindOrAdd(Channel);

	FPGXMessageListenerData& Entry = List.Listeners.AddDefaulted_GetRef();
	Entry.ReceivedCallback = MoveTemp(Callback);
	Entry.ListenerStructType = StructType;
	Entry.bHadValidType = StructType != nullptr;
	Entry.HandleID = ++List.HandleID;
	Entry.MatchType = MatchType;

	const bool bShouldLog = IsValid(CachedConfig) ? CachedConfig->bLogRegistrations : false;
	if (bShouldLog)
	{
		UE_LOG(LogPGXMessage, Log, TEXT("Listener registered: Channel=%s Type=%s HandleID=%d"),
			*Channel.ToString(), StructType ? *StructType->GetName() : TEXT("Any"), Entry.HandleID);
	}

	// EN: Update stats / ES: Actualizar estadisticas
	Stats.ActiveChannels = ListenerMap.Num();
	Stats.ActiveListeners++;

	OnListenerRegistered.Broadcast(Channel);

	return FPGXMessageListenerHandle(this, Channel, Entry.HandleID);
}

void UPGXMessageSubsystem::UnregisterListener(FPGXMessageListenerHandle Handle)
{
	if (Handle.IsValid())
	{
		if (!ensureMsgf(Handle.Subsystem == this, TEXT("PGX MessageSubsystem: Handle belongs to a different subsystem instance")))
		{
			return;
		}
		UnregisterListenerInternal(Handle.Channel, Handle.ID);
	}
	else
	{
		UE_LOG(LogPGXMessage, Warning, TEXT("Trying to unregister an invalid handle."));
	}
}

void UPGXMessageSubsystem::UnregisterListenerInternal(FGameplayTag Channel, int32 HandleID)
{
	if (FChannelListenerList* pList = ListenerMap.Find(Channel))
	{
		const int32 MatchIndex = pList->Listeners.IndexOfByPredicate(
			[ID = HandleID](const FPGXMessageListenerData& Other) { return Other.HandleID == ID; });

		if (MatchIndex != INDEX_NONE)
		{
			pList->Listeners.RemoveAtSwap(MatchIndex);
			Stats.ActiveListeners = FMath::Max(0, Stats.ActiveListeners - 1);

			const bool bShouldLog = IsValid(CachedConfig) ? CachedConfig->bLogRegistrations : false;
			if (bShouldLog)
			{
				UE_LOG(LogPGXMessage, Log, TEXT("Listener unregistered: Channel=%s HandleID=%d"),
					*Channel.ToString(), HandleID);
			}

			OnListenerUnregistered.Broadcast(Channel);
		}

		if (pList->Listeners.Num() == 0)
		{
			ListenerMap.Remove(Channel);
		}

		Stats.ActiveChannels = ListenerMap.Num();
	}
}

// ============================================================
// PGX Extensions: Query API
// ============================================================

TArray<FPGXMessageRecord> UPGXMessageSubsystem::GetMessageHistory(FGameplayTag Channel, int32 MaxResults) const
{
	TArray<FPGXMessageRecord> Result;
	const int32 Limit = FMath::Max(1, MaxResults);

	for (int32 i = MessageHistory.Num() - 1; i >= 0 && Result.Num() < Limit; --i)
	{
		if (!Channel.IsValid() || MessageHistory[i].Channel.MatchesTag(Channel))
		{
			Result.Add(MessageHistory[i]);
		}
	}

	return Result;
}

void UPGXMessageSubsystem::ClearHistory()
{
	MessageHistory.Empty();
	Stats.HistorySize = 0;
	UE_LOG(LogPGXMessage, Log, TEXT("Message history cleared"));
}

int32 UPGXMessageSubsystem::GetListenerCount(FGameplayTag Channel) const
{
	if (const FChannelListenerList* pList = ListenerMap.Find(Channel))
	{
		return pList->Listeners.Num();
	}
	return 0;
}

int32 UPGXMessageSubsystem::GetTotalListenerCount() const
{
	return Stats.ActiveListeners;
}

TArray<FGameplayTag> UPGXMessageSubsystem::GetAllActiveChannels() const
{
	TArray<FGameplayTag> Channels;
	ListenerMap.GetKeys(Channels);
	return Channels;
}

bool UPGXMessageSubsystem::IsChannelActive(FGameplayTag Channel) const
{
	return ListenerMap.Contains(Channel);
}

FPGXMessageStats UPGXMessageSubsystem::GetStats() const
{
	FPGXMessageStats CurrentStats = Stats;
	CurrentStats.HistorySize = MessageHistory.Num();
	return CurrentStats;
}

// ============================================================
// History Management
// ============================================================

void UPGXMessageSubsystem::AddToHistory(FGameplayTag Channel, const UScriptStruct* StructType, int32 ListenersNotified, bool bIsTestOrigin)
{
	// EN: EffectiveMaxHistory is the single source of truth (resolved by RecomputeEffectiveMaxHistory).
	//     Min(Config.MaxMessageHistory or Settings.EmergencyHistoryFallback, Profile.MessageBudgets.MaxMessageHistory).
	//     0 means history disabled (graceful degradation when no config + emergency fallback==0).
	// ES: EffectiveMaxHistory es la fuente unica de verdad (resuelto por RecomputeEffectiveMaxHistory).
	//     0 significa historial deshabilitado.
	if (EffectiveMaxHistory <= 0)
	{
		return;
	}

	FPGXMessageRecord Record;
	Record.Channel = Channel;
	Record.PayloadTypeName = StructType ? StructType->GetName() : TEXT("None");
	Record.Timestamp = FPlatformTime::Seconds();
	Record.ListenersNotified = ListenersNotified;
	Record.bIsTestOrigin = bIsTestOrigin;

	MessageHistory.Add(MoveTemp(Record));

	// EN: Trim history if over limit / ES: Recortar historial si excede el limite
	while (MessageHistory.Num() > EffectiveMaxHistory)
	{
		MessageHistory.RemoveAt(0);
	}
}

// ============================================================
// EffectiveMaxHistory recomputation (platform-profile history clamping)
// ============================================================

void UPGXMessageSubsystem::RecomputeEffectiveMaxHistory()
{
	// EN: Resolve config-side bound deterministically: CachedConfig → Settings.EmergencyHistoryFallback → 0.
	// ES: Resolver el limite del lado config: CachedConfig → Settings.EmergencyHistoryFallback → 0.
	int32 ConfigVal = 0;
	if (IsValid(CachedConfig))
	{
		ConfigVal = CachedConfig->MaxMessageHistory;
	}
	else
	{
		const UPGXMessageSettings* Settings = GetDefault<UPGXMessageSettings>();
		ConfigVal = Settings ? Settings->EmergencyHistoryFallback : 0;
		if (ConfigVal > 0)
		{
			UE_LOG(LogPGXMessage, Warning,
				TEXT("RecomputeEffectiveMaxHistory: CachedConfig invalid; using Settings.EmergencyHistoryFallback=%d"),
				ConfigVal);
		}
	}

	// EN: Profile clamp (the platform-profile clamp policy). Profile budget > 0 means active clamp; 0 means no platform-side limit.
	// ES: Clamp por Profile (the platform-profile clamp policy). Budget > 0 = clamp activo; 0 = sin limite de plataforma.
	int32 ProfileVal = MAX_int32;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (auto* ProfileSS = GI->GetSubsystem<UPGXProfileSubsystem>())
		{
			if (const UPGXPlatformConfig* PlatformCfg = ProfileSS->GetActivePlatformConfig())
			{
				const int32 Budget = PlatformCfg->MessageBudgets.MaxMessageHistory;
				if (Budget > 0)
				{
					ProfileVal = Budget;
				}
			}
		}
	}

	// EN: Effective = Min(Config, Profile). 0 means history disabled entirely.
	// ES: Efectivo = Min(Config, Profile). 0 = historial deshabilitado.
	const int32 NewEffective = FMath::Min(ConfigVal, ProfileVal);

	if (NewEffective != EffectiveMaxHistory)
	{
		UE_LOG(LogPGXMessage, Log,
			TEXT("EffectiveMaxHistory: %d -> %d (Config=%d, Profile=%s)"),
			EffectiveMaxHistory, NewEffective, ConfigVal,
			ProfileVal == MAX_int32 ? TEXT("unbounded") : *FString::Printf(TEXT("%d"), ProfileVal));

		EffectiveMaxHistory = NewEffective;

		// EN: Trim existing history if new bound shrinks it. Profile clamp trims hot.
		// ES: Recortar historial existente si el nuevo limite lo reduce. Profile clamp recorta en caliente.
		while (MessageHistory.Num() > EffectiveMaxHistory && MessageHistory.Num() > 0)
		{
			MessageHistory.RemoveAt(0);
		}
	}
}

// ============================================================
// Console Commands
// ============================================================

void UPGXMessageSubsystem::RegisterConsoleCommands()
{
	ConsoleCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.message.status"),
		TEXT("Display PGX Message System status"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			const FPGXMessageStats S = GetStats();
			UE_LOG(LogPGXMessage, Log, TEXT("=== PGX Message System Status ==="));
			UE_LOG(LogPGXMessage, Log, TEXT("  Active Channels: %d"), S.ActiveChannels);
			UE_LOG(LogPGXMessage, Log, TEXT("  Active Listeners: %d"), S.ActiveListeners);
			UE_LOG(LogPGXMessage, Log, TEXT("  Total Broadcasts: %d"), S.TotalBroadcasts);
			UE_LOG(LogPGXMessage, Log, TEXT("  Total Listeners Notified: %d"), S.TotalListenersNotified);
			UE_LOG(LogPGXMessage, Log, TEXT("  History Size: %d"), S.HistorySize);
		}),
		ECVF_Default));

	ConsoleCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.message.channels"),
		TEXT("List all active message channels"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			const TArray<FGameplayTag> Channels = GetAllActiveChannels();
			UE_LOG(LogPGXMessage, Log, TEXT("=== Active Message Channels (%d) ==="), Channels.Num());
			for (const FGameplayTag& Ch : Channels)
			{
				UE_LOG(LogPGXMessage, Log, TEXT("  %s (%d listeners)"), *Ch.ToString(), GetListenerCount(Ch));
			}
		}),
		ECVF_Default));

	ConsoleCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.message.history"),
		TEXT("Show recent message history"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			const TArray<FPGXMessageRecord> History = GetMessageHistory(FGameplayTag(), 20);
			UE_LOG(LogPGXMessage, Log, TEXT("=== Message History (last %d) ==="), History.Num());
			for (const FPGXMessageRecord& Record : History)
			{
				UE_LOG(LogPGXMessage, Log, TEXT("  [%.2f] %s (%s) -> %d listeners"),
					Record.Timestamp, *Record.Channel.ToString(), *Record.PayloadTypeName, Record.ListenersNotified);
			}
		}),
		ECVF_Default));

	// EN: pgx.message.broadcast — gated at compile time to dev/editor/debug builds .
	//     Shipping builds never register this command. Within dev builds, runtime gate via
	//     Config.bAllowTestBroadcasts allows projects to disable per-asset.
	//     All console-originated broadcasts mark Record.bIsTestOrigin=true so inspector
	//     can filter test pollution out of production telemetry views.
	// ES: pgx.message.broadcast — gated en compile time a builds dev/editor/debug .
	//     Builds shipping nunca registran este comando. Dentro de builds dev, gate de runtime
	//     via Config.bAllowTestBroadcasts permite a proyectos deshabilitarlo per-asset.
	//     Todos los broadcasts originados en consola marcan Record.bIsTestOrigin=true para que
	//     el inspector pueda filtrar la polucion de tests fuera de las vistas de produccion.
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG
	ConsoleCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.message.broadcast"),
		TEXT("Broadcast a test message on a tag (dev/editor only; record marked bIsTestOrigin)"),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& Args)
		{
			// EN: Runtime gate via Config DA. When invalid Config, default to true (dev build conservative).
			// ES: Gate de runtime via Config DA. Si Config invalido, default a true (conservador en dev).
			const bool bAllowed = IsValid(CachedConfig) ? CachedConfig->bAllowTestBroadcasts : true;
			if (!bAllowed)
			{
				UE_LOG(LogPGXMessage, Warning,
					TEXT("pgx.message.broadcast disabled by Config.bAllowTestBroadcasts=false"));
				return;
			}

			if (Args.Num() < 1)
			{
				UE_LOG(LogPGXMessage, Warning, TEXT("Usage: pgx.message.broadcast <GameplayTag>"));
				return;
			}
			const FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(FName(*Args[0]), false);
			if (!TestTag.IsValid())
			{
				UE_LOG(LogPGXMessage, Warning, TEXT("Invalid tag: %s"), *Args[0]);
				return;
			}
			FPGXMessage TestMsg;
			TestMsg.MessageTag = TestTag;
			TestMsg.Timestamp = FPlatformTime::Seconds();

			// EN: Direct call to internal with bIsTestOrigin=true. Cannot use BroadcastMessage<T>
			//     template because that path passes bIsTestOrigin=false (production semantic).
			// ES: Llamada directa a internal con bIsTestOrigin=true. No podemos usar el template
			//     BroadcastMessage<T> porque ese path pasa bIsTestOrigin=false (semantica de produccion).
			const UScriptStruct* MsgStructType = TBaseStructure<FPGXMessage>::Get();
			BroadcastMessageInternal(TestTag, MsgStructType, &TestMsg, /*bIsTestOrigin=*/true);

			UE_LOG(LogPGXMessage, Log,
				TEXT("Test broadcast sent on channel: %s (bIsTestOrigin=true)"), *TestTag.ToString());
		}),
		ECVF_Default));
#endif // WITH_EDITOR || UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG
}

void UPGXMessageSubsystem::UnregisterConsoleCommands()
{
	for (IConsoleCommand* Cmd : ConsoleCommands)
	{
		if (Cmd)
		{
			IConsoleManager::Get().UnregisterConsoleObject(Cmd);
		}
	}
	ConsoleCommands.Empty();
}

// ============================================================
// Profile Integration
// ============================================================

void UPGXMessageSubsystem::ApplyProfileConstraints(const FPGXResolvedProfile& /*Profile*/)
{
	// EN: Profile clamp materializes via RecomputeEffectiveMaxHistory (single source of truth).
	//     platform-profile history clamping now actually applies (was no-op log before).
	// ES: El clamp por Profile se materializa via RecomputeEffectiveMaxHistory (fuente unica de verdad).
	//     platform-profile history clamping ahora aplica realmente (era no-op log antes).
	RecomputeEffectiveMaxHistory();
}

void UPGXMessageSubsystem::HandleProfileChanged(const FPGXResolvedProfile& /*OldProfile*/, const FPGXResolvedProfile& NewProfile)
{
	ApplyProfileConstraints(NewProfile);
}

// ============================================================
// Test Harness Support
// ============================================================

#if WITH_EDITOR
void UPGXMessageSubsystem::InjectTestConfig(UPGXMessageConfig* TestConfig)
{
	CachedConfig = TestConfig;
	RecomputeEffectiveMaxHistory();
	UE_LOG(LogPGXMessage, Log, TEXT("Test config injected (MaxHistory=%d, EffectiveMaxHistory=%d)"),
		IsValid(CachedConfig) ? CachedConfig->MaxMessageHistory : 0, EffectiveMaxHistory);
}

void UPGXMessageSubsystem::ClearTestConfigs()
{
	CachedConfig = nullptr;
	DiscoverAndLoadConfig();
	RecomputeEffectiveMaxHistory();
	UE_LOG(LogPGXMessage, Log, TEXT("Test config cleared, re-discovered from AssetRegistry (EffectiveMaxHistory=%d)"),
		EffectiveMaxHistory);
}
#endif
