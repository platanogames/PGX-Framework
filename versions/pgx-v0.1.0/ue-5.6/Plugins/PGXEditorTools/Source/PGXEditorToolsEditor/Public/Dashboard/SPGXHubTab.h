// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Containers/Ticker.h"

/**
 * EN: Main PGX Hub widget. Central dashboard showing framework KPIs,
 *     plugin cards in SWrapBox grid, quick links, and PIE status footer.
 *     Auto-refreshes KPIs via FTSTicker every 5 seconds.
 * ES: Widget principal del PGX Hub. Dashboard central que muestra KPIs del framework,
 *     cards de plugins en grid SWrapBox, enlaces rapidos y footer de estado PIE.
 *     Auto-refresh de KPIs via FTSTicker cada 5 segundos.
 */
class PGXEDITORTOOLSEDITOR_API SPGXHubTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXHubTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SPGXHubTab() override;

private:
	// EN: Build sections / ES: Construir secciones
	TSharedRef<SWidget> BuildStatusSection();
	TSharedRef<SWidget> BuildPluginDashboard();
	TSharedRef<SWidget> BuildQuickLinksSection();

	// EN: Helpers / ES: Helpers
	int32 CountLoadedPlugins() const;
	void RefreshKPIs();

	// EN: PIE lifecycle / ES: Ciclo de vida PIE
	void OnPIEStarted(bool bIsSimulating);
	void OnPIEEnded(bool bIsSimulating);

	// EN: KPI text blocks for refresh / ES: Bloques de texto KPI para refresh
	TSharedPtr<STextBlock> PluginCountText;
	TSharedPtr<STextBlock> SystemCountText;
	TSharedPtr<STextBlock> PIEStatusText;

	// EN: Delegate handles / ES: Handles de delegates
	FDelegateHandle PIEStartedHandle;
	FDelegateHandle PIEEndedHandle;
	FTSTicker::FDelegateHandle RefreshTickerHandle;
};
