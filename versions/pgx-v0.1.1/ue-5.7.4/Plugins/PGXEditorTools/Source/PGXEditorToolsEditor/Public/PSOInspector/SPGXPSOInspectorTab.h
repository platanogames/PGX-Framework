// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "PGXPSOTypes.h"
#include "Containers/Ticker.h"

class UPGXPSOSubsystem;
class UPGXPSOWarmUpConfig;

/**
 * EN: PGX PSO Inspector Panel — Real-time visualization of PSO warm-up pipeline.
 *     Displays 3 panels:
 *     - Warm-Up Status: State badge, progress %, stats grid, active contexts
 *     - Discovered Configs: List of configs with entry count, activation mode, validate button
 *     - Recording Session: List of recorded entries, export/convert/clear buttons
 *
 *     Binds to PIE lifecycle for live updates via OnStateChangedNative + 1s polling.
 *     Retains last snapshot after PIE ends.
 *
 * ES: Panel Inspector PSO de PGX — Visualizacion en tiempo real del pipeline de warm-up PSO.
 *     Muestra 3 paneles:
 *     - Estado de Warm-Up: Badge de estado, % de progreso, grid de stats, contextos activos
 *     - Configs Descubiertos: Lista de configs con conteo de entradas, modo de activacion, boton validar
 *     - Sesion de Grabacion: Lista de entradas grabadas, botones exportar/convertir/limpiar
 */
class PGXEDITORTOOLSEDITOR_API SPGXPSOInspectorTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXPSOInspectorTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SPGXPSOInspectorTab() override;

private:
	// ========================================================================
	// EN: UI Build / ES: Construccion de UI
	// ========================================================================

	TSharedRef<SWidget> BuildWarmUpStatusPanel();
	TSharedRef<SWidget> BuildDiscoveredConfigsPanel();
	TSharedRef<SWidget> BuildRecordingPanel();

	// ========================================================================
	// EN: SListView row generators / ES: Generadores de filas SListView
	// ========================================================================

	TSharedRef<ITableRow> OnGenerateConfigRow(
		TSharedPtr<FString> Item,
		const TSharedRef<STableViewBase>& OwnerTable);

	TSharedRef<ITableRow> OnGenerateRecordingRow(
		TSharedPtr<FPGXPSORecordedEntry> Item,
		const TSharedRef<STableViewBase>& OwnerTable);

	// ========================================================================
	// EN: Refresh methods / ES: Metodos de refresh
	// ========================================================================

	void RefreshAllData();
	void RefreshStatusPanel();
	void RefreshConfigsList();
	void RefreshRecordingList();

	// ========================================================================
	// EN: PIE Lifecycle / ES: Ciclo de Vida PIE
	// ========================================================================

	void BindPIEDelegates();
	void OnPIEStarted(bool bIsSimulating);
	void OnPIEEnded(bool bIsSimulating);
	void BindToSubsystem();
	void UnbindFromSubsystem();
	void StartPolling();
	void StopPolling();

	// ========================================================================
	// EN: Delegate callbacks / ES: Callbacks de delegados
	// ========================================================================

	void OnPSOStateChanged(EPGXPSOWarmUpState NewState);
	void OnWarmUpBegin();
	void OnWarmUpProgress(int32 Completed, int32 Total, float Percent);
	void OnWarmUpComplete();

	// ========================================================================
	// EN: Actions / ES: Acciones
	// ========================================================================

	FReply OnRefreshClicked();
	FReply OnRecordToggleClicked();
	FReply OnExportJsonClicked();
	FReply OnClearRecordingClicked();
	FReply OnForceWarmUpClicked();
	FReply OnPauseResumeClicked();
	FReply OnSaveCacheClicked();
	FReply OnConvertRecordingClicked();

	void RefreshKPIChips();

	// ========================================================================
	// EN: Helpers / ES: Helpers
	// ========================================================================

	static FLinearColor GetStateColor(EPGXPSOWarmUpState State);
	static FString GetStateName(EPGXPSOWarmUpState State);

	// ========================================================================
	// EN: State / ES: Estado
	// ========================================================================

	// EN: KPI chips in toolbar / ES: Chips KPI en toolbar
	TSharedPtr<STextBlock> KPIStateChip;
	TSharedPtr<STextBlock> KPIConfigsChip;
	TSharedPtr<SBorder> KPICacheBadge;
	TSharedPtr<STextBlock> KPICacheChip;

	// EN: Status panel widgets / ES: Widgets del panel de estado
	TSharedPtr<STextBlock> StateBadgeText;
	TSharedPtr<STextBlock> ProgressText;
	TSharedPtr<STextBlock> StatsText;
	TSharedPtr<STextBlock> ContextsText;
	TSharedPtr<STextBlock> RecordButtonText;

	// EN: Discovered configs list / ES: Lista de configs descubiertos
	TArray<TSharedPtr<FString>> ConfigDisplayNames;
	TSharedPtr<SListView<TSharedPtr<FString>>> ConfigListView;

	// EN: Recording entries / ES: Entradas de grabacion
	TArray<TSharedPtr<FPGXPSORecordedEntry>> RecordingEntries;
	TSharedPtr<SListView<TSharedPtr<FPGXPSORecordedEntry>>> RecordingListView;

	// EN: Status bar / ES: Barra de estado
	TSharedPtr<STextBlock> StatusText;

	// EN: PIE state / ES: Estado PIE
	TWeakObjectPtr<UPGXPSOSubsystem> BoundSubsystem;
	FDelegateHandle PIEStartedHandle;
	FDelegateHandle PIEEndedHandle;
	FDelegateHandle StateChangedHandle;
	FDelegateHandle WarmUpBeginHandle;
	FDelegateHandle WarmUpProgressHandle;
	FDelegateHandle WarmUpCompleteHandle;

	// EN: Polling timer / ES: Timer de polling
	FTSTicker::FDelegateHandle PollingHandle;
	bool bIsPIEActive = false;
};
