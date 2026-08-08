// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Layout/SWidgetSwitcher.h"

class UPGXMessageSubsystem;

/**
 * EN: Row data for Active Channels table.
 * ES: Datos de fila para tabla de Canales Activos.
 */
struct FPGXChannelRowData
{
	FString LeafTag;
	FString FullTag;
	int32 ListenerCount = 0;
	int32 BroadcastCount = 0;
	FGameplayTag ChannelTag;
};

/**
 * EN: Row data for Message History table.
 * ES: Datos de fila para tabla de Historial de Mensajes.
 */
struct FPGXMessageHistoryRowData
{
	double Timestamp = 0.0;
	FString LeafChannel;
	FString FullChannel;
	FString PayloadType;
	int32 ListenersNotified = 0;
};

/**
 * EN: PGX Message Inspector — NomadTab for inspecting the Message System.
 *     Shows active channels (SListView, selectable for filtering), message history (SListView, filtered),
 *     stats, and test/clear actions. Delegate-driven via OnMessageBroadcastNative.
 *     Universal shell: 2px color bar + KPI chips + footer PIE state.
 * ES: PGX Message Inspector — NomadTab para inspeccionar el Sistema de Mensajes.
 *     Muestra canales activos (SListView, seleccionable para filtrar), historial de mensajes (SListView, filtrado),
 *     estadisticas y acciones test/clear. Delegate-driven via OnMessageBroadcastNative.
 */
class SPGXMessageInspectorTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXMessageInspectorTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SPGXMessageInspectorTab() override;
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	TSharedRef<SWidget> BuildChannelsSection();
	TSharedRef<SWidget> BuildHistorySection();
	TSharedRef<SWidget> BuildActionsSection();

	UPGXMessageSubsystem* GetMessageSubsystem() const;
	void RefreshData();
	void RefreshHistorySection();
	void UpdateChannelFilterLabel();

	// EN: PIE lifecycle / ES: Ciclo de vida PIE
	void BindPIEDelegates();
	void OnPIEStarted(bool bIsSimulating);
	void OnPIEEnded(bool bIsSimulating);

	// EN: Subsystem delegate binding / ES: Binding de delegates del subsistema
	void BindToSubsystem();
	void UnbindFromSubsystem();

	// EN: Delegate callbacks / ES: Callbacks de delegates
	void OnMessageBroadcast(FGameplayTag Channel, const UScriptStruct* StructType, double Timestamp);

	// EN: Channel selection for history filter / ES: Seleccion de canal para filtro de historial
	void OnChannelSelected(TSharedPtr<FPGXChannelRowData> Item, ESelectInfo::Type SelectInfo);

	// EN: Row generation / ES: Generacion de filas
	TSharedRef<ITableRow> OnGenerateChannelRow(TSharedPtr<FPGXChannelRowData> Item, const TSharedRef<STableViewBase>& Owner);
	TSharedRef<ITableRow> OnGenerateHistoryRow(TSharedPtr<FPGXMessageHistoryRowData> Item, const TSharedRef<STableViewBase>& Owner);

	// EN: Content switcher (0 = empty state, 1 = live content) / ES: Switcher de contenido
	TSharedPtr<SWidgetSwitcher> ContentSwitcher;
	TSharedPtr<STextBlock> FooterStatusText;

	// EN: PIE delegate handles / ES: Handles de delegates PIE
	FDelegateHandle PIEStartedHandle;
	FDelegateHandle PIEEndedHandle;

	// EN: Subsystem delegate handles / ES: Handles de delegates del subsistema
	FDelegateHandle BroadcastHandle;
	TWeakObjectPtr<UPGXMessageSubsystem> BoundSubsystem;

	// EN: Channel filter / ES: Filtro de canal
	FGameplayTag SelectedChannelFilter;
	TSharedPtr<STextBlock> ChannelFilterLabel;

	// EN: Overview metrics / ES: Metricas del resumen
	TSharedPtr<STextBlock> StatusText;
	TSharedPtr<STextBlock> ChannelsText;
	TSharedPtr<STextBlock> ListenersText;
	TSharedPtr<STextBlock> BroadcastsText;

	// EN: Channels table / ES: Tabla de canales
	TArray<TSharedPtr<FPGXChannelRowData>> ChannelItems;
	TSharedPtr<SListView<TSharedPtr<FPGXChannelRowData>>> ChannelListView;

	// EN: History table / ES: Tabla de historial
	TArray<TSharedPtr<FPGXMessageHistoryRowData>> HistoryItems;
	TSharedPtr<SListView<TSharedPtr<FPGXMessageHistoryRowData>>> HistoryListView;

	float RefreshAccumulator = 0.0f;
	static constexpr float RefreshInterval = 0.5f;
};
