// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "GameplayTagContainer.h"
#include "Messages/PGXMessage.h"

class UPGXMessageSubsystem;

/**
 * EN: Per-channel summary row used by the Channels panel SListView.
 * ES: Fila de resumen por canal usada por el SListView del panel Channels.
 */
struct FPGXMessageChannelEntry
{
	FGameplayTag Channel;
	int32 ListenerCount = 0;
};

/**
 * EN: Listener lifecycle event inferred from polled listener-count deltas.
 *     Type encodes whether the delta was positive (Registered) or negative
 *     (Unregistered). Workaround for OnListenerRegistered being DYNAMIC-only.
 * ES: Evento de listener lifecycle inferido desde deltas polled del count de
 *     listeners. Type codifica si el delta fue positivo (Registered) o negativo
 *     (Unregistered). Workaround porque OnListenerRegistered es DYNAMIC-only.
 */
enum class EPGXMessageLifecycleEventType : uint8
{
	Registered,
	Unregistered,
};

struct FPGXMessageLifecycleEvent
{
	FGameplayTag Channel;
	EPGXMessageLifecycleEventType Type = EPGXMessageLifecycleEventType::Registered;
	int32 Delta = 0;          // EN: count change (signed) / ES: cambio de count (con signo)
	int32 NewCount = 0;       // EN: count after the delta / ES: count despues del delta
	double Timestamp = 0.0;   // EN: GameThread monotonic seconds / ES: segundos GameThread monotonic
};

/**
 * EN: PGX Message Inspector tab widget. Read-only diagnostics surface for the
 *     UPGXMessageSubsystem covering seven diagnostic sections.
 *
 *     Provides History and Listener Lifecycle panels in addition to the
 *     scaffold, Overview, and Channels sections.
 *
 * ES: Widget tab del PGX Message Inspector. Surface read-only de diagnostics
 *     para el UPGXMessageSubsystem, con siete secciones de diagnostico.
 *     Incluye History y Listener Lifecycle ademas de Overview y Channels.
 */
class PGXCOREEDITOR_API SPGXMessageInspectorTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXMessageInspectorTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual ~SPGXMessageInspectorTab() override;

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	// EN: PIE lifecycle entry points. The bool parameter matches the
	//     UE 5.6 FEditorDelegates::PostPIEStarted / EndPIE multicast
	//     signature `void(bool bIsSimulating)`; default-arg permits
	//     direct call sites (Tick refresh, Construct path) to invoke
	//     without supplying the flag while retaining delegate signature compatibility.
	// ES: Entry points del ciclo de vida PIE. El parametro bool empareja
	//     la signature multicast `void(bool bIsSimulating)` de
	//     FEditorDelegates::PostPIEStarted / EndPIE en UE 5.6; el
	//     default-arg permite que call sites directos (refresh Tick,
	//     path Construct) invoquen sin suministrar el flag y mantengan la
	//     compatibilidad con la signature del delegate.
	void BindToSubsystem(bool bIsSimulating = false);
	void UnbindFromSubsystem(bool bIsSimulating = false);

	UPGXMessageSubsystem* ResolvePIESubsystem() const;

	void HandleBroadcast(FGameplayTag Channel, const UScriptStruct* StructType, double Timestamp);

	// EN: Refresh polled snapshot. Order:
	//     1. GetStats / GetAllActiveChannels / GetListenerCount → CachedStats + ChannelEntries.
	//     2. Diff per-channel listener counts vs LastSeenListenerCounts → append
	//        FPGXMessageLifecycleEvent rows to LifecycleEvents (capped).
	//     3. If a channel is selected, refresh HistoryEntries via GetMessageHistory.
	//     4. RequestListRefresh on all three SListViews.
	// ES: Refrescar snapshot polled. Orden:
	//     1. GetStats / GetAllActiveChannels / GetListenerCount → CachedStats + ChannelEntries.
	//     2. Diff de counts de listeners por canal vs LastSeenListenerCounts → append
	//        de filas FPGXMessageLifecycleEvent a LifecycleEvents (cap).
	//     3. Si hay un canal seleccionado, refresh de HistoryEntries via GetMessageHistory.
	//     4. RequestListRefresh en los tres SListViews.
	void RefreshSnapshot();

	// EN: SListView row generators.
	// ES: Generadores de filas SListView.
	TSharedRef<class ITableRow> OnGenerateChannelRow(
		TSharedPtr<FPGXMessageChannelEntry> Entry,
		const TSharedRef<class STableViewBase>& OwnerTable);
	TSharedRef<class ITableRow> OnGenerateHistoryRow(
		TSharedPtr<FPGXMessageRecord> Entry,
		const TSharedRef<class STableViewBase>& OwnerTable);
	TSharedRef<class ITableRow> OnGenerateLifecycleRow(
		TSharedPtr<FPGXMessageLifecycleEvent> Entry,
		const TSharedRef<class STableViewBase>& OwnerTable);

	// EN: Channels SListView selection callback — drives History panel filter.
	// ES: Callback de seleccion del SListView Channels — guia el filtro del History.
	void OnChannelSelectionChanged(
		TSharedPtr<FPGXMessageChannelEntry> NewSelection,
		ESelectInfo::Type SelectInfo);

	// EN: Slate text getters bound via TAttribute lambdas.
	// ES: Getters de texto Slate bindeados via TAttribute lambdas.
	FText GetOverviewSummaryText() const;
	FText GetEmptyStateText() const;
	FText GetHistoryHeaderText() const;
	FText GetConfigSummaryText() const;
	FText GetWarningsSummaryText() const;

	// EN: Subsystem ref — null until PIE starts and resolution succeeds.
	// ES: Ref al subsistema — null hasta que PIE inicie y la resolucion tenga exito.
	TWeakObjectPtr<UPGXMessageSubsystem> WeakSubsystem;

	// EN: Cached snapshot from last polled Refresh.
	// ES: Snapshot cacheada del ultimo Refresh polled.
	FPGXMessageStats CachedStats;

	// EN: Channels panel data + view.
	// ES: Data + view del panel Channels.
	TArray<TSharedPtr<FPGXMessageChannelEntry>> ChannelEntries;
	TSharedPtr<SListView<TSharedPtr<FPGXMessageChannelEntry>>> ChannelListView;

	// EN: History panel data + view + selected channel filter (empty tag = no filter).
	// ES: Data + view del panel History + filtro de canal seleccionado (tag vacio = sin filtro).
	TArray<TSharedPtr<FPGXMessageRecord>> HistoryEntries;
	TSharedPtr<SListView<TSharedPtr<FPGXMessageRecord>>> HistoryListView;
	FGameplayTag SelectedChannel;

	// EN: Listener Lifecycle panel — capped event log inferred from delta polling.
	// ES: Panel Listener Lifecycle — log de eventos cap inferidos desde polling de delta.
	TArray<TSharedPtr<FPGXMessageLifecycleEvent>> LifecycleEvents;
	TSharedPtr<SListView<TSharedPtr<FPGXMessageLifecycleEvent>>> LifecycleListView;
	TMap<FGameplayTag, int32> LastSeenListenerCounts;

	// EN: Delegate handles.
	// ES: Handles de delegate.
	FDelegateHandle BroadcastNativeHandle;
	FDelegateHandle PIEStartedHandle;
	FDelegateHandle PIEEndedHandle;

	// EN: Last-broadcast info touched by HandleBroadcast — GameThread-only per
	//     GameThread-only message-bus invariant; FString / double / int32 are NOT atomic in
	//     the C++ memory model, so any future background dispatch must marshal
	//     to GameThread before invoking the callback. HandleBroadcast asserts
	//     IsInGameThread() to enforce the invariant.
	// ES: Info de ultimo broadcast tocada por HandleBroadcast — GameThread-only
	//     por el invariante GameThread-only del bus de mensajes; FString / double / int32 NO son
	//     atomic en el modelo de memoria C++, asi que cualquier dispatch
	//     futuro en background debe marshal a GameThread antes de invocar el
	//     callback. HandleBroadcast assertea IsInGameThread() para enforcear
	//     el invariante.
	FString LastBroadcastChannelName;
	double LastBroadcastTimestamp = 0.0;
	int32 ObservedBroadcastCount = 0;

	double LastRefreshSeconds = 0.0;
};
