// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "GameplayTagContainer.h"
#include "PGXGameFlowTypes.h"
#include "GameFlowInspector/SPGXGameFlowChannelRow.h"

class UPGXGameFlowSubsystem;
class SPGXTelemetryGraph;
class SPGXSparkline;

/**
 * EN: Enriched transition record captured by the inspector's delegate callback.
 *     Extends FPGXFlowHistoryEntry with channel, previous tag, and source name.
 * ES: Registro de transicion enriquecido capturado por el callback de delegado del inspector.
 *     Extiende FPGXFlowHistoryEntry con canal, tag anterior y nombre de fuente.
 */
struct FPGXFlowTransitionRecord
{
	EPGXFlowChannel Channel = EPGXFlowChannel::Global;
	FGameplayTag NewTag;
	FGameplayTag PreviousTag;
	FDateTime Timestamp;
	FString SourceName;
	FString ChannelName;
};

/**
 * EN: Main PGX GameFlow Inspector widget. Provides real-time visualization of the
 *     GameFlow subsystem state with 3 panels:
 *     - Current States: 8 channel rows (expandable) with live state + rule details
 *     - Transition History: Filtered by selected channel or "all"
 *     - Channel Detail: Text dump of selected channel's full state
 *
 *     Binds to PIE lifecycle to connect/disconnect from UPGXGameFlowSubsystem.
 *     Listens to OnFlowStateChangedNative for live updates.
 *
 * ES: Widget principal del PGX GameFlow Inspector. Provee visualizacion en tiempo real
 *     del estado del subsistema GameFlow con 3 paneles:
 *     - Estados Actuales: 8 filas de canal (expandibles) con estado en vivo + detalles de regla
 *     - Historial de Transiciones: Filtrado por canal seleccionado o "todos"
 *     - Detalle de Canal: Volcado de texto del estado completo del canal seleccionado
 *
 *     Se bindea al ciclo de vida PIE para conectar/desconectar de UPGXGameFlowSubsystem.
 *     Escucha OnFlowStateChangedNative para actualizaciones en vivo.
 */
class PGXEDITORTOOLSEDITOR_API SPGXGameFlowInspectorTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXGameFlowInspectorTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SPGXGameFlowInspectorTab() override;
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	// ========================================================================
	// EN: UI Build
	// ES: Construccion de UI
	// ========================================================================

	TSharedRef<SWidget> BuildToolbar();
	TSharedRef<SWidget> BuildCurrentStatesPanel();
	TSharedRef<SWidget> BuildTransitionHistoryPanel();
	TSharedRef<SWidget> BuildChannelDetailPanel();
	TSharedRef<SWidget> BuildTelemetrySection();

	// ========================================================================
	// EN: SListView row generators
	// ES: Generadores de filas SListView
	// ========================================================================

	TSharedRef<ITableRow> OnGenerateChannelRow(
		TSharedPtr<FPGXFlowChannelSummary> Item,
		const TSharedRef<STableViewBase>& OwnerTable);

	TSharedRef<ITableRow> OnGenerateHistoryRow(
		TSharedPtr<FPGXFlowTransitionRecord> Item,
		const TSharedRef<STableViewBase>& OwnerTable);

	// ========================================================================
	// EN: Channel selection
	// ES: Seleccion de canal
	// ========================================================================

	void OnChannelSelected(TSharedPtr<FPGXFlowChannelSummary> Item, ESelectInfo::Type SelectInfo);

	// ========================================================================
	// EN: Refresh methods
	// ES: Metodos de refresh
	// ========================================================================

	void RefreshChannelStates();
	void RefreshHistoryForChannel();
	void RefreshChannelDetail();
	void LoadExistingHistory();

	// ========================================================================
	// EN: PIE Lifecycle
	// ES: Ciclo de Vida PIE
	// ========================================================================

	void BindPIEDelegates();
	void OnPIEStarted(bool bIsSimulating);
	void OnPIEEnded(bool bIsSimulating);
	void BindToSubsystem();
	void UnbindFromSubsystem();

	// ========================================================================
	// EN: Delegate callback (live from subsystem)
	// ES: Callback de delegado (en vivo desde subsistema)
	// ========================================================================

	void OnFlowStateChanged(EPGXFlowChannel Channel, FGameplayTag FlowTag, UObject* Source);

	// ========================================================================
	// EN: Actions
	// ES: Acciones
	// ========================================================================

	FReply OnRefreshClicked();
	FReply OnClearClicked();
	FReply OnRevertClicked();

	void RefreshKPIChips();

	// ========================================================================
	// EN: Helpers
	// ES: Helpers
	// ========================================================================

	static FLinearColor GetChannelColor(EPGXFlowChannel Channel);

	// ========================================================================
	// EN: State
	// ES: Estado
	// ========================================================================

	// EN: Channel rows (Current States panel) / ES: Filas de canal (panel Estados Actuales)
	TArray<TSharedPtr<FPGXFlowChannelSummary>> ChannelSummaries;
	TSharedPtr<SListView<TSharedPtr<FPGXFlowChannelSummary>>> ChannelListView;
	TMap<EPGXFlowChannel, TSharedPtr<SPGXGameFlowChannelRow>> ChannelRowWidgets;

	// EN: Transition history / ES: Historial de transiciones
	TArray<TSharedPtr<FPGXFlowTransitionRecord>> AllHistoryEntries;
	TArray<TSharedPtr<FPGXFlowTransitionRecord>> FilteredHistoryEntries;
	TSharedPtr<SListView<TSharedPtr<FPGXFlowTransitionRecord>>> HistoryListView;

	// EN: KPI chips in toolbar / ES: Chips KPI en toolbar
	TSharedPtr<STextBlock> KPIActiveChannels;
	TSharedPtr<STextBlock> KPITransitionCount;

	// EN: Channel detail structured widgets / ES: Widgets estructurados de detalle de canal
	TSharedPtr<STextBlock> DetailCurrentText;
	TSharedPtr<STextBlock> DetailPreviousText;
	TSharedPtr<SBorder> DetailCanRevertBadge;
	TSharedPtr<STextBlock> DetailCanRevertText;
	TSharedPtr<STextBlock> DetailHistoryDepthText;
	TSharedPtr<STextBlock> DetailRuleNameText;
	TSharedPtr<STextBlock> DetailRuleDescText;
	TSharedPtr<STextBlock> DetailRuleRevertText;
	TSharedPtr<STextBlock> DetailRuleErrorText;
	TSharedPtr<SVerticalBox> DetailAllowedBox;
	TSharedPtr<SVerticalBox> DetailDisallowedBox;
	TSharedPtr<SWidget> DetailRuleSection;
	TSharedPtr<SWidget> DetailStateSection;
	TSharedPtr<STextBlock> DetailEmptyText;
	TSharedPtr<SButton> RevertButton;

	// EN: History header / ES: Header de historial
	TSharedPtr<STextBlock> HistoryHeaderText;

	// EN: Selected channel filter (MAX = all channels) / ES: Filtro de canal seleccionado (MAX = todos)
	EPGXFlowChannel SelectedChannel = EPGXFlowChannel::MAX;

	// EN: Auto-scroll / ES: Auto-scroll
	bool bAutoScroll = true;

	// EN: Telemetry graph / ES: Grafico de telemetria
	TSharedPtr<SPGXTelemetryGraph> TransitionRateGraph;
	TMap<EPGXFlowChannel, int32> TransitionCounters;
	TMap<EPGXFlowChannel, TSharedPtr<SPGXSparkline>> ChannelSparklines;
	float SparklineAccumulator = 0.0f;

	// EN: PIE state / ES: Estado PIE
	TWeakObjectPtr<UPGXGameFlowSubsystem> BoundSubsystem;
	FDelegateHandle PIEStartedHandle;
	FDelegateHandle PIEEndedHandle;
	FDelegateHandle FlowStateChangedHandle;

	// EN: Status bar / ES: Barra de estado
	TSharedPtr<STextBlock> StatusText;
};
