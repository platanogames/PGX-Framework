// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Observer/PGXObserverTypes.h"

class SPGXTelemetryGraph;
class SPGXSparkline;

/**
 * EN: PGX System Observer Panel — Live dashboard of framework health.
 *     Displays 5 sections: Core Systems, Subsystems, Active Instances, Plugins, Data Registry.
 *     Auto-refreshes during PIE at 1s intervals. Retains last snapshot after PIE ends.
 *     Changes (PLAN_02): STUB badges, system color bars, styled instance rows,
 *     leaf tag names, KPI toolbar, SHeaderRow for subsystem list.
 *
 * ES: Panel Observer del Sistema PGX — Dashboard live de salud del framework.
 *     Muestra 5 secciones: Sistemas Core, Subsistemas, Instancias Activas, Plugins, Data Registry.
 *     Auto-refresco durante PIE a intervalos de 1s. Retiene ultimo snapshot despues de PIE.
 *     Cambios (PLAN_02): Badges STUB, barras de color por sistema, filas de instancias estilizadas,
 *     nombres leaf de tags, KPI en toolbar, SHeaderRow para lista de subsistemas.
 */
class PGXEDITORTOOLSEDITOR_API SPGXSystemObserverTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXSystemObserverTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SPGXSystemObserverTab() override;

private:
	// ========================================================================
	// EN: UI Build / ES: Construccion de UI
	// ========================================================================

	TSharedRef<SWidget> BuildToolbar();
	TSharedRef<SWidget> BuildCoreSystemsSection();
	TSharedRef<SWidget> BuildSubsystemsSection();
	TSharedRef<SWidget> BuildTelemetryOverviewSection();
	TSharedRef<SWidget> BuildActiveInstancesSection();
	TSharedRef<SWidget> BuildPluginGridSection();
	TSharedRef<SWidget> BuildDataRegistrySection();

	// ========================================================================
	// EN: SListView row generator / ES: Generador de filas SListView
	// ========================================================================

	TSharedRef<ITableRow> OnGenerateSubsystemRow(
		TSharedPtr<FPGXSubsystemSnapshot> Item,
		const TSharedRef<STableViewBase>& OwnerTable);

	// ========================================================================
	// EN: Data Collection / ES: Recoleccion de Datos
	// ========================================================================

	void CollectCoreClasses();
	void CollectSubsystems();
	void CollectActiveInstances();
	void CollectPlugins();
	void CollectDataRegistry();
	void RefreshAllData();

	// ========================================================================
	// EN: PIE Lifecycle / ES: Ciclo de Vida PIE
	// ========================================================================

	void BindPIEDelegates();
	void OnPIEStarted(bool bIsSimulating);
	void OnPIEEnded(bool bIsSimulating);
	void StartPolling();
	void StopPolling();
	bool OnPollingTick(float DeltaTime);

	// ========================================================================
	// EN: Helpers / ES: Helpers
	// ========================================================================

	FReply OnRefreshClicked();

	/** EN: Get system color by subsystem name / ES: Obtener color de sistema por nombre */
	static FLinearColor GetSystemColorByName(FName Name);

	// ========================================================================
	// EN: State / ES: Estado
	// ========================================================================

	// EN: Core class snapshots / ES: Snapshots de clases core
	TArray<FPGXCoreClassSnapshot> CoreSnapshots;

	// EN: Subsystem snapshots for SListView / ES: Snapshots de subsistemas para SListView
	TArray<TSharedPtr<FPGXSubsystemSnapshot>> SubsystemSnapshots;
	TSharedPtr<SListView<TSharedPtr<FPGXSubsystemSnapshot>>> SubsystemListView;

	// EN: Active instance detail box (styled rows) / ES: Box de detalle de instancias activas (filas estilizadas)
	TSharedPtr<SVerticalBox> InstanceDetailBox;

	// EN: Plugin snapshots / ES: Snapshots de plugins
	TArray<FPGXPluginSnapshot> PluginSnapshots;

	// EN: Core classes display / ES: Display de clases core
	TSharedPtr<SVerticalBox> CoreClassesBox;

	// EN: Plugin grid display / ES: Display de grid de plugins
	TSharedPtr<SWrapBox> PluginGrid;

	// EN: Data Registry display / ES: Display del Data Registry
	TSharedPtr<SVerticalBox> DataRegistryBox;

	// EN: Status bar / ES: Barra de estado
	TSharedPtr<STextBlock> StatusText;

	// EN: KPI chip in toolbar / ES: KPI chip en toolbar
	TSharedPtr<SWidget> SubsystemChip;
	FText CachedSubsystemCount;
	FText CachedFooterText;

	// EN: Telemetry graphs / ES: Graficos de telemetria
	TSharedPtr<SPGXTelemetryGraph> ActiveSubsystemGraph;
	TMap<FName, TSharedPtr<SPGXSparkline>> PerSubsystemSparklines;

	// EN: PIE state / ES: Estado PIE
	bool bIsPIEActive = false;
	FDelegateHandle PIEStartedHandle;
	FDelegateHandle PIEEndedHandle;

	// EN: Polling timer / ES: Timer de polling
	FTSTicker::FDelegateHandle PollingHandle;
};
