// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "GameplayTagContainer.h"
#include "PGXSaveTypes.h"
#include "PGXSaveSlotInfo.h"

class UPGXSaveSubsystem;
class UPGXSaveConfig;

/**
 * EN: Pipeline operation entry for the Save Inspector log.
 *     Captures save/load/delete/autosave operations as they happen.
 * ES: Entrada de operacion de pipeline para el log del Save Inspector.
 *     Captura operaciones de save/load/delete/autosave a medida que ocurren.
 */
struct FPGXSavePipelineEntry
{
	FDateTime Timestamp;
	FString OperationType; // "Save", "Load", "Delete", "AutoSave"
	FString ContextName;
	FString SlotName;
	EPGXSaveResult Result;
};

/**
 * EN: Context summary row for the Active Contexts panel.
 *     Represents a discovered UPGXSaveConfig with summary data.
 * ES: Fila de resumen de contexto para el panel de Contextos Activos.
 *     Representa un UPGXSaveConfig descubierto con datos de resumen.
 */
struct FPGXSaveContextEntry
{
	FGameplayTag ContextTag;
	FText DisplayName;
	FString SaveModeName;
	int32 DomainCount = 0;
	int32 SlotCount = 0;
	FString ActiveSlotName;
	bool bAutoSaveActive = false;
};

/**
 * EN: Domain detail row for the SListView (replaces text dump).
 * ES: Fila de detalle de dominio para SListView (reemplaza dump de texto).
 */
struct FPGXDomainDetailRow
{
	FString DomainLeafTag;
	FString DomainFullTag;
	FString SaveGameClassName;
	bool bHasInstance = false;
	int32 SaveFormatVersion = 0;
	int32 TotalKeys = 0;
	FString KeyBreakdown; // "Str:5 Int:3 Float:2 Bool:1 Vec:0 Rot:0 Trans:0 Tag:0"
	bool bRequired = false;
};

/**
 * EN: PGX Save Inspector — NomadTab for inspecting the Save System.
 *     Shows active contexts (SListView, selectable), domain detail (SListView),
 *     pipeline log (SListView, live), slot browser (SListView), platform budgets,
 *     and action buttons. Universal shell: 2px Green bar + footer PIE state.
 *     SWidgetSwitcher for empty/live transition.
 * ES: PGX Save Inspector — NomadTab para inspeccionar el Sistema de Guardado.
 *     Muestra contextos activos (SListView, seleccionable), detalle de dominio (SListView),
 *     pipeline log (SListView, live), slot browser (SListView), presupuestos de plataforma,
 *     y botones de accion. Shell universal: 2px barra Verde + footer estado PIE.
 */
class PGXEDITORTOOLSEDITOR_API SPGXSaveInspectorTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXSaveInspectorTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SPGXSaveInspectorTab() override;
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	// ========================================================================
	// EN: UI Build
	// ES: Construccion de UI
	// ========================================================================

	TSharedRef<SWidget> BuildToolbar();
	TSharedRef<SWidget> BuildContextPanel();
	TSharedRef<SWidget> BuildDomainDetailPanel();
	TSharedRef<SWidget> BuildPipelineLogPanel();
	TSharedRef<SWidget> BuildSlotBrowserPanel();
	TSharedRef<SWidget> BuildFooter();

	// ========================================================================
	// EN: SListView row generators
	// ES: Generadores de filas SListView
	// ========================================================================

	TSharedRef<ITableRow> OnGenerateContextRow(TSharedPtr<FPGXSaveContextEntry> Item, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<ITableRow> OnGeneratePipelineRow(TSharedPtr<FPGXSavePipelineEntry> Item, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<ITableRow> OnGenerateSlotRow(TSharedPtr<FPGXSaveSlotInfo> Item, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<ITableRow> OnGenerateDomainDetailRow(TSharedPtr<FPGXDomainDetailRow> Item, const TSharedRef<STableViewBase>& OwnerTable);

	// ========================================================================
	// EN: Context selection
	// ES: Seleccion de contexto
	// ========================================================================

	void OnContextSelected(TSharedPtr<FPGXSaveContextEntry> Item, ESelectInfo::Type SelectInfo);
	void RefreshContextList();
	void RefreshDomainDetail();
	void RefreshSlotBrowser();

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
	// EN: Delegate callbacks (live from subsystem)
	// ES: Callbacks de delegates (en vivo desde subsistema)
	// ========================================================================

	void OnSaveCompleted(const FString& SlotName, EPGXSaveResult Result);
	void OnLoadCompleted(const FString& SlotName, EPGXSaveResult Result, UPGXSaveGame* SaveGame);
	void OnSlotDeleted(const FString& SlotName);
	void OnAutoSaveTriggered(FGameplayTag ContextTag);

	// ========================================================================
	// EN: Actions
	// ES: Acciones
	// ========================================================================

	FReply OnRefreshClicked();

	// ========================================================================
	// EN: State
	// ES: Estado
	// ========================================================================

	// EN: Content switcher (0 = empty state, 1 = live content) / ES: Switcher de contenido
	TSharedPtr<SWidgetSwitcher> ContentSwitcher;

	// EN: Active Contexts data / ES: Datos de Contextos Activos
	TArray<TSharedPtr<FPGXSaveContextEntry>> ContextRows;
	TSharedPtr<SListView<TSharedPtr<FPGXSaveContextEntry>>> ContextListView;

	// EN: Pipeline Log data / ES: Datos del Pipeline Log
	TArray<TSharedPtr<FPGXSavePipelineEntry>> PipelineEntries;
	TSharedPtr<SListView<TSharedPtr<FPGXSavePipelineEntry>>> PipelineListView;

	// EN: Slot Browser data / ES: Datos del Slot Browser
	TArray<TSharedPtr<FPGXSaveSlotInfo>> SlotRows;
	TSharedPtr<SListView<TSharedPtr<FPGXSaveSlotInfo>>> SlotListView;

	// EN: Domain Detail data / ES: Datos de Detalle de Dominio
	TArray<TSharedPtr<FPGXDomainDetailRow>> DomainDetailItems;
	TSharedPtr<SListView<TSharedPtr<FPGXDomainDetailRow>>> DomainListView;
	TSharedPtr<STextBlock> DomainNoSelectionText;

	// EN: Selected context / ES: Contexto seleccionado
	FGameplayTag SelectedContextTag;

	// EN: PIE state / ES: Estado PIE
	TWeakObjectPtr<UPGXSaveSubsystem> BoundSubsystem;
	FDelegateHandle PIEStartedHandle;
	FDelegateHandle PIEEndedHandle;
	FDelegateHandle SaveCompletedHandle;
	FDelegateHandle LoadCompletedHandle;
	FDelegateHandle SlotDeletedHandle;
	FDelegateHandle AutoSaveHandle;

	// EN: Footer / ES: Footer
	TSharedPtr<STextBlock> FooterStatusText;

	// EN: Activity indicator (Save/Load in progress) / ES: Indicador de actividad
	TSharedPtr<STextBlock> ActivityIndicator;
};
