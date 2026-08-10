// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Containers/Ticker.h"

class FPGXVisualHarness;

/**
 * EN: PSPH Control Panel — NomadTab with DEMO and HARNESS modes.
 *     DEMO: Ordered checklist of demo DAs with create/open buttons.
 *     HARNESS: Rich data injection, live simulation, status dashboard, quick actions, panel launcher.
 *
 * ES: Panel de Control PSPH — NomadTab con modos DEMO y HARNESS.
 *     DEMO: Checklist ordenado de DAs demo con botones crear/abrir.
 *     HARNESS: Inyeccion de datos ricos, simulacion en vivo, dashboard de estado, acciones rapidas, launcher de paneles.
 */
class PGXSIMHARNESSEDITOR_API SPGXSimHarnessTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXSimHarnessTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SPGXSimHarnessTab() override;

private:
	/** EN: Build the DEMO mode panel / ES: Construir el panel de modo DEMO */
	TSharedRef<SWidget> BuildDemoPanel();

	/** EN: Build the HARNESS mode panel / ES: Construir el panel de modo HARNESS */
	TSharedRef<SWidget> BuildHarnessPanel();

	/** EN: Build the mode toggle bar / ES: Construir la barra de toggle de modo */
	TSharedRef<SWidget> BuildModeToggle();

	/** EN: Build harness control bar / ES: Construir barra de control del harness */
	TSharedRef<SWidget> BuildControlBar();

	/** EN: Build status dashboard / ES: Construir dashboard de estado */
	TSharedRef<SWidget> BuildStatusDashboard();

	/** EN: Build system status grid / ES: Construir grid de estado de sistemas */
	TSharedRef<SWidget> BuildSystemGrid();

	/** EN: Build coverage matrix / ES: Construir matriz de cobertura */
	TSharedRef<SWidget> BuildCoverageMatrix();

	/** EN: Build quick actions section / ES: Construir seccion de acciones rapidas */
	TSharedRef<SWidget> BuildQuickActions();

	/** EN: Build panel launcher section / ES: Construir seccion de lanzador de paneles */
	TSharedRef<SWidget> BuildPanelLauncher();

	/** EN: Switch between DEMO and HARNESS / ES: Cambiar entre DEMO y HARNESS */
	void SetMode(bool bNewDemoMode);

	/** EN: Refresh HARNESS panel UI state / ES: Refrescar estado del UI del panel HARNESS */
	void RefreshHarnessUI();

	/** EN: UI refresh ticker callback / ES: Callback del ticker de refresco de UI */
	bool OnUIRefreshTick(float DeltaTime);

	// ─── State ───

	bool bIsDemoMode = true;

	TSharedPtr<class SWidgetSwitcher> ModeSwitcher;

	/** EN: Persistent harness instance / ES: Instancia persistente del harness */
	TSharedPtr<FPGXVisualHarness> Harness;

	/** EN: UI refresh ticker (1s interval) / ES: Ticker de refresco de UI (intervalo 1s) */
	FTSTicker::FDelegateHandle UIRefreshTickerHandle;

	// EN: Cached text blocks for dynamic updates / ES: Text blocks cacheados para actualizacion dinamica
	TSharedPtr<class STextBlock> StatusKPI;
	TSharedPtr<class STextBlock> SimKPI;
	TSharedPtr<class STextBlock> ElapsedKPI;
	TSharedPtr<class STextBlock> ObjectsKPI;
	TSharedPtr<class STextBlock> FooterText;
	TSharedPtr<class SVerticalBox> SystemGridBox;

	/** EN: Map raw FName panel IDs to display names / ES: Mapear FName IDs de panel a nombres legibles */
	static FString GetPanelDisplayName(const FName& PanelId);
};
