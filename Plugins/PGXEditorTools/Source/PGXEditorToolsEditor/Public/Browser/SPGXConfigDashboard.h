// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

struct FAssetData;

/**
 * EN: Dashboard showing Config resolution status across the framework.
 *     Primary view: Settings Slots — shows which systems have configs assigned in Project Settings.
 *     Secondary view: All DAs — shows all Config DataAssets with CONFIGURED/UNREFERENCED badges.
 *     Toggle between views with a button. Auto-refreshes when AssetRegistry changes.
 * ES: Dashboard mostrando el estado de resolucion de Config del framework.
 *     Vista primaria: Settings Slots — muestra que sistemas tienen configs asignados en Project Settings.
 *     Vista secundaria: All DAs — muestra todos los Config DataAssets con badges CONFIGURED/UNREFERENCED.
 *     Alterna entre vistas con un boton. Auto-refresca cuando cambia el AssetRegistry.
 */
class PGXEDITORTOOLSEDITOR_API SPGXConfigDashboard : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXConfigDashboard) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SPGXConfigDashboard() override;

private:
	// ── Enums ────────────────────────────────────────────────────────────
	enum class EValidationResult : uint8
	{
		Unknown,
		OK,
		Warning,
		Error
	};

	enum class EDashboardView : uint8
	{
		SettingsSlots,
		AllDAs
	};

	// ── Structs ──────────────────────────────────────────────────────────

	/** EN: Settings slot entry / ES: Entrada de slot de settings */
	struct FSettingsSlotEntry
	{
		FString SystemName;
		FLinearColor SystemColor;
		bool bConfigSet = false;
		FString ConfigDAName;
		bool bTableSet = false;
		int32 TableRowCount = 0;
		FString TableDAName;
	};

	/** EN: Internal config entry (All DAs view) / ES: Entrada de config interna (vista All DAs) */
	struct FConfigEntry
	{
		FString ClassName;
		FString AssetName;
		FString SystemGroup;
		FAssetData AssetData;
		EValidationResult Validation = EValidationResult::Unknown;
		FString ValidationMessage;
		bool bReferencedBySettings = false;
	};

	// ── UI Build ─────────────────────────────────────────────────────────
	void RebuildView();
	void BuildSettingsSlotsView();
	void BuildAllDAsView();

	// ── Footer Attribute Getters ─────────────────────────────────────────
	FText GetFooterStatusText() const;
	FLinearColor GetFooterStatusColor() const;

	// ── Actions ──────────────────────────────────────────────────────────
	FReply OnRefreshClicked();
	FReply OnValidateAllClicked();
	FReply OnToggleViewClicked();

	// ── Data ─────────────────────────────────────────────────────────────
	void ScanConfigAssets();
	void ScanSettingsSlots();
	FString InferSystemGroup(const FString& ClassName) const;
	static FLinearColor GetSystemGroupColor(const FString& Group);
	void ValidateEntry(FConfigEntry& Entry);
	void MarkReferencedDAs();

	// ── Auto-Refresh ─────────────────────────────────────────────────────
	void BindAssetRegistryDelegates();
	void UnbindAssetRegistryDelegates();
	void OnAssetAdded(const FAssetData& AssetData);
	void OnAssetRemoved(const FAssetData& AssetData);
	void OnAssetRenamed(const FAssetData& AssetData, const FString& OldPath);
	bool IsConfigAsset(const FAssetData& AssetData) const;

	// ── State ────────────────────────────────────────────────────────────
	EDashboardView CurrentView = EDashboardView::SettingsSlots;
	TArray<FSettingsSlotEntry> SettingsSlots;
	TArray<FConfigEntry> ConfigEntries;
	TSharedPtr<SVerticalBox> ContentBox;
	int32 WarningCount = 0;
	int32 ErrorCount = 0;

	// ── Delegate Handles ─────────────────────────────────────────────────
	FDelegateHandle OnAssetAddedHandle;
	FDelegateHandle OnAssetRemovedHandle;
	FDelegateHandle OnAssetRenamedHandle;
};
