// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Profile/PGXProfileTypes.h"

class UPGXPlatformConfig;
struct FPGXResolvedProfile;

/**
 * EN: PGX Platform Health Dashboard — Shows configured platform budgets per system.
 *     Displays budget limits from UPGXPlatformConfig for all PGX systems,
 *     with color-coded indicators and a platform selector for comparison.
 *     Reads Profile subsystem during PIE; shows config data in editor.
 *
 * ES: Dashboard de Salud de Plataforma PGX — Muestra presupuestos de plataforma por sistema.
 *     Muestra limites de presupuesto desde UPGXPlatformConfig para todos los sistemas PGX,
 *     con indicadores coloreados y selector de plataforma para comparacion.
 *     Lee el subsistema Profile durante PIE; muestra datos de config en editor.
 */
class PGXEDITORTOOLSEDITOR_API SPGXPlatformHealthTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXPlatformHealthTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SPGXPlatformHealthTab() override;

private:
	// ── UI Build ─────────────────────────────────────────────────────────

	// EN: BuildToolbar() removed — integrated into SPGXPanelHeader
	TSharedRef<SWidget> BuildGlobalBudgetsSection();
	TSharedRef<SWidget> BuildSystemBudgetsGrid();
	TSharedRef<SWidget> BuildTraitsGrid(const UPGXPlatformConfig* Config);
	TSharedRef<SWidget> BuildSystemCard(const FText& SystemName, const FLinearColor& SystemColor, const TArray<TPair<FText, int32>>& BudgetValues);
	TSharedRef<SWidget> BuildBudgetRow(const FText& Label, int32 Value);

	// ── Data ─────────────────────────────────────────────────────────────

	void RefreshData();
	const UPGXPlatformConfig* GetSelectedPlatformConfig() const;

	// ── Actions ──────────────────────────────────────────────────────────

	FReply OnRefreshClicked();
	FReply OnSimulatePlatformClicked();
	void OnPlatformSelected(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);

	// ── PIE Lifecycle ────────────────────────────────────────────────────

	void BindPIEDelegates();
	void OnPIEStarted(bool bIsSimulating);
	void OnPIEEnded(bool bIsSimulating);

	// ── Delegate Callbacks ───────────────────────────────────────────────

	void OnProfileChanged(const FPGXResolvedProfile& OldProfile, const FPGXResolvedProfile& NewProfile);

	// ── State ────────────────────────────────────────────────────────────

	TArray<TSharedPtr<FString>> PlatformOptions;
	TSharedPtr<FString> SelectedPlatform;
	TSharedPtr<SVerticalBox> ContentArea;
	TSharedPtr<STextBlock> StatusText;

	bool bIsPIEActive = false;
	FDelegateHandle PIEStartedHandle;
	FDelegateHandle PIEEndedHandle;
	FDelegateHandle ProfileChangedHandle;
};
