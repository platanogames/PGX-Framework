// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "PGXPSOTypes.h"

class UPGXPSOWarmUpConfig;

/**
 * EN: PSO Auto-Populator tool. Editor tab that extracts materials from
 *     Content Browser selections and populates UPGXPSOWarmUpConfig DAs.
 * ES: Herramienta Auto-Populator de PSO. Tab del editor que extrae materiales
 *     de selecciones del Content Browser y puebla DAs UPGXPSOWarmUpConfig.
 */
class PGXEDITORTOOLSEDITOR_API SPGXPSOAutoPopulatorTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXPSOAutoPopulatorTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	// ========================================================================
	// EN: UI Build / ES: Construccion de UI
	// ========================================================================

	TSharedRef<SWidget> BuildTargetDASection();
	TSharedRef<SWidget> BuildDefaultsSection();
	TSharedRef<SWidget> BuildPreviewSection();
	TSharedRef<SWidget> BuildActionsSection();

	// EN: Footer attribute getters / ES: Getters de atributos del footer
	FText GetFooterStatusText() const;
	FLinearColor GetFooterStatusColor() const;

	// ========================================================================
	// EN: SListView / ES: SListView
	// ========================================================================

	TSharedRef<ITableRow> OnGeneratePreviewRow(
		TSharedPtr<FPGXPSOEntry> Item,
		const TSharedRef<STableViewBase>& OwnerTable);

	// ========================================================================
	// EN: Actions / ES: Acciones
	// ========================================================================

	FReply OnImportFromSelection();
	FReply OnPopulateDA();
	FReply OnClearPreview();

	// ========================================================================
	// EN: Material Extraction / ES: Extraccion de Materiales
	// ========================================================================

	void ExtractMaterialsFromAsset(UObject* Asset, TArray<TSharedPtr<FPGXPSOEntry>>& OutEntries, TSet<FSoftObjectPath>& DeduplicatedPaths);

	// ========================================================================
	// EN: Target DA / ES: DA Objetivo
	// ========================================================================

	FString OnGetTargetDAPath() const;
	void OnTargetDAChanged(const FAssetData& AssetData);

	// ========================================================================
	// EN: State / ES: Estado
	// ========================================================================

	/** EN: Target WarmUpConfig DA to populate / ES: DA WarmUpConfig objetivo a poblar */
	TWeakObjectPtr<UPGXPSOWarmUpConfig> TargetConfig;

	/** EN: Preview entries before committing to DA / ES: Entradas de preview antes de escribir al DA */
	TArray<TSharedPtr<FPGXPSOEntry>> PreviewEntries;

	/** EN: List view for preview entries / ES: List view para entradas de preview */
	TSharedPtr<SListView<TSharedPtr<FPGXPSOEntry>>> PreviewListView;

	/** EN: Cached footer status text / ES: Texto de estado del footer cacheado */
	FText CachedFooterText;
	FLinearColor CachedFooterColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

	// ── Defaults / Defaults ──

	EPGXVertexFactoryType DefaultVertexFactory = EPGXVertexFactoryType::StaticMesh;
	FGameplayTag DefaultContextTag;
	bool bDefaultRenderInMainPass = true;
	bool bDefaultRenderInDepthPass = true;
	bool bDefaultCastShadow = true;
	TEnumAsByte<EComponentMobility::Type> DefaultMobility = EComponentMobility::Static;
};
