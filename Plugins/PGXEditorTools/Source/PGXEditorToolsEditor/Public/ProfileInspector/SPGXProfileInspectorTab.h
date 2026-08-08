// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Profile/PGXProfileTypes.h"

class UPGXProfileSubsystem;

/**
 * EN: PGX Profile Inspector Panel — Dashboard for the project profile system.
 *     Changes (PLAN_03): KPI toolbar, capabilities grid, budget progress bars,
 *     feature policy badges with color, structured policies table.
 *
 * ES: Panel Inspector Profile de PGX — Dashboard para el sistema de profile del proyecto.
 *     Cambios (PLAN_03): KPI toolbar, grid de capacidades, barras de progreso de budgets,
 *     badges de policy de features con color, tabla de politicas estructurada.
 */
class PGXEDITORTOOLSEDITOR_API SPGXProfileInspectorTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXProfileInspectorTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SPGXProfileInspectorTab() override;

private:
	// ========================================================================
	// EN: UI Build / ES: Construccion de UI
	// ========================================================================

	TSharedRef<SWidget> BuildIdentityPanel();
	TSharedRef<SWidget> BuildCapabilitiesPanel();
	TSharedRef<SWidget> BuildPoliciesPanel();
	TSharedRef<SWidget> BuildBudgetsPanel();
	TSharedRef<SWidget> BuildFeaturesPanel();
	TSharedRef<SWidget> BuildSimulationPanel();

	// ========================================================================
	// EN: Refresh / ES: Refrescar
	// ========================================================================

	void RefreshAllData();
	void RefreshFromProfile(const FPGXResolvedProfile& Profile);

	// ========================================================================
	// EN: PIE Lifecycle / ES: Ciclo de Vida PIE
	// ========================================================================

	void BindPIEDelegates();
	void OnPIEStarted(bool bIsSimulating);
	void OnPIEEnded(bool bIsSimulating);
	void BindToSubsystem();
	void UnbindFromSubsystem();

	// ========================================================================
	// EN: Delegate callbacks / ES: Callbacks de delegados
	// ========================================================================

	void HandleProfileChanged(const FPGXResolvedProfile& OldProfile, const FPGXResolvedProfile& NewProfile);

	// ========================================================================
	// EN: Actions / ES: Acciones
	// ========================================================================

	FReply OnRefreshClicked();
	FReply OnSimulatePlatformClicked();
	FReply OnSimulateBuildClicked();
	FReply OnClearSimulationClicked();

	// ========================================================================
	// EN: Helpers / ES: Helpers
	// ========================================================================

	static FString GetModeName(EPGXProjectMode Mode);
	static FString GetPlatformName(EPGXTargetPlatform Platform);
	static FString GetBuildName(EPGXBuildContext Context);
	static FString GetRestrictionName(EPGXRestrictionLevel Level);
	static FString GetStateName(EPGXProfileState State);
	static FString GetBackendName(EPGXPersistenceBackend Backend);
	static FString GetFeaturePolicyName(EPGXFeaturePolicy Policy);
	static FLinearColor GetStateColor(EPGXProfileState State);
	static FLinearColor GetFeaturePolicyColor(EPGXFeaturePolicy Policy);

	// ========================================================================
	// EN: State / ES: Estado
	// ========================================================================

	// EN: Identity panel widgets / ES: Widgets del panel de identidad
	TSharedPtr<STextBlock> ModeText;
	TSharedPtr<STextBlock> TargetsText;
	TSharedPtr<STextBlock> BuildText;
	TSharedPtr<STextBlock> RestrictionText;

	// EN: KPI chips in toolbar / ES: Chips KPI en toolbar
	TSharedPtr<STextBlock> KPIModeChip;
	TSharedPtr<STextBlock> KPIStateChip;
	TSharedPtr<SBorder> KPIStateBadge;
	TSharedPtr<STextBlock> KPIPlatformChip;

	// EN: Capabilities grid (10 badges) / ES: Grid de capacidades (10 badges)
	static constexpr int32 NumCapabilities = 10;
	TSharedPtr<SBorder> CapBadges[NumCapabilities];

	// EN: Budgets panel (9 rows) / ES: Panel de budgets (9 filas)
	TSharedPtr<SVerticalBox> BudgetsBox;

	// EN: Features panel (7 rows with policy badges) / ES: Panel de features (7 filas con badges de policy)
	static constexpr int32 NumFeatures = 7;
	TSharedPtr<SBorder> FeaturePolicyBadges[NumFeatures];
	TSharedPtr<STextBlock> FeaturePolicyTexts[NumFeatures];
	TSharedPtr<STextBlock> FeatureRestartTexts[NumFeatures];

	// EN: Policies panel fields / ES: Campos del panel de politicas
	TSharedPtr<STextBlock> PolicyConfigBackend;
	TSharedPtr<STextBlock> PolicyLogBackend;
	TSharedPtr<STextBlock> PolicySaveBackend;
	TSharedPtr<STextBlock> PolicyBasePath;
	TSharedPtr<STextBlock> PolicyNaming;
	TSharedPtr<SBorder> PolicyEncryptionBadge;
	TSharedPtr<SBorder> PolicyCompressionBadge;
	TSharedPtr<SBorder> PolicySensitiveBadge;
	TSharedPtr<SBorder> PolicyStripDebugBadge;
	TSharedPtr<STextBlock> PolicyRestartText;

	// EN: Simulation panel / ES: Panel de simulacion
	TSharedPtr<STextBlock> SimulationStatusText;

	// EN: Status bar / ES: Barra de estado
	TSharedPtr<STextBlock> StatusBarText;

	// EN: Cached profile for display / ES: Profile cacheado para display
	FPGXResolvedProfile CachedProfile;

	// EN: PIE state / ES: Estado PIE
	TWeakObjectPtr<UPGXProfileSubsystem> BoundSubsystem;
	FDelegateHandle PIEStartedHandle;
	FDelegateHandle PIEEndedHandle;
	FDelegateHandle ProfileChangedHandle;
	bool bIsPIEActive = false;
};
