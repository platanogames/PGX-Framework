// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "Core/PGXScaffoldTypes.h"

class FPGXProjectAnalyzer;
class FPGXHierarchyResolver;
class FPGXScaffoldValidator;
class FPGXBuildPlanGenerator;
class FPGXScaffoldExecutor;
class FPGXScaffoldLogger;

/**
 * EN: Row data for the plan review SListView
 * ES: Datos de fila para el SListView de revision de plan
 */
struct FPGXScaffoldPlanStepRow
{
	FPGXScaffoldPlanStep Step;

	/** EN: True if asset/folder already exists (will be skipped) / ES: True si el asset/carpeta ya existe (se saltara) */
	bool bWillSkip = false;
};

/**
 * EN: PGX Scaffold Panel — 3-phase UI for template selection, validation, and execution.
 *     Phase A: Template selection
 *     Phase B: Configuration + validation
 *     Phase C: Plan review + execution
 *
 * ES: Panel PGX Scaffold — flujo de 3 pasos para seleccion de template, validacion y ejecucion.
 *     Paso A: Seleccion de template
 *     Paso B: Configuracion + validacion
 *     Paso C: Revision de plan + ejecucion
 */
class SPGXScaffoldTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXScaffoldTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SPGXScaffoldTab() override;

private:
	// ─── Phase builders ───
	TSharedRef<SWidget> BuildPhaseA();
	TSharedRef<SWidget> BuildPhaseB();
	TSharedRef<SWidget> BuildPhaseC();

	// ─── Phase A: Template selection ───
	TSharedRef<SWidget> BuildCategoryList();
	TSharedRef<SWidget> BuildTemplateCards();
	void OnCategorySelected(FName Category);
	void OnTemplateSelected(const FPGXScaffoldTemplate* Template);

	// ─── Phase B: Configuration ───
	TSharedRef<SWidget> BuildVariableFields();
	TSharedRef<SWidget> BuildHierarchyTree();
	TSharedRef<SWidget> BuildValidationResults();
	void OnValidateClicked();

	// ─── Phase C: Review ───
	TSharedRef<SWidget> BuildPlanReview();
	TSharedRef<SWidget> BuildKPIBar();
	void OnExportJsonClicked();
	void OnExecuteClicked();

	/** EN: SListView row generation for plan review / ES: Generacion de fila SListView para revision de plan */
	TSharedRef<ITableRow> OnGeneratePlanRow(
		TSharedPtr<FPGXScaffoldPlanStepRow> Item,
		const TSharedRef<STableViewBase>& OwnerTable);

	/** EN: Check Live Coding state before future C++ generation / ES: Verificar Live Coding antes de la futura generacion C++ */
	bool CheckLiveCodingWarning() const;

	// ─── Navigation ───
	void GoToPhase(int32 Phase);

	// ─── State ───
	int32 CurrentPhase = 0;
	TSharedPtr<class SWidgetSwitcher> PhaseSwitcher;
	TSharedPtr<class STextBlock> FooterText;

	// EN: Selected template / ES: Template seleccionado
	const FPGXScaffoldTemplate* SelectedTemplate = nullptr;
	FName SelectedCategory;

	// EN: Variable values (user input) / ES: Valores de variables (input del usuario)
	TMap<FString, FString> VariableValues;

	// EN: Tree state / ES: Estado del arbol
	TArray<TSharedPtr<FPGXScaffoldTreeNode>> TreeRootNodes;

	// EN: Validation results / ES: Resultados de validacion
	TArray<FPGXScaffoldValidationResult> ValidationResults;
	bool bValidationPassed = false;

	// EN: Build plan / ES: Plan de build
	TSharedPtr<FPGXScaffoldBuildPlan> CurrentPlan;

	// EN: Project info / ES: Info del proyecto
	FPGXScaffoldProjectInfo ProjectInfo;

	// EN: Pipeline components / ES: Componentes del pipeline
	TSharedPtr<FPGXProjectAnalyzer> Analyzer;
	TSharedPtr<FPGXHierarchyResolver> Resolver;
	TSharedPtr<FPGXScaffoldValidator> Validator;
	TSharedPtr<FPGXBuildPlanGenerator> PlanGenerator;
	TSharedPtr<FPGXScaffoldExecutor> Executor;
	TSharedPtr<FPGXScaffoldLogger> Logger;

	// EN: Cached widgets for dynamic updates / ES: Widgets cacheados para actualizacion dinamica
	TSharedPtr<SVerticalBox> TemplateCardsBox;
	TSharedPtr<SVerticalBox> VariableFieldsBox;
	TSharedPtr<SVerticalBox> ValidationBox;
	TSharedPtr<SVerticalBox> TreeBox;
	TSharedPtr<class SButton> NextFromBButton;
	TSharedPtr<class STextBlock> KPIFoldersText;
	TSharedPtr<class STextBlock> KPIAssetsText;
	TSharedPtr<class STextBlock> KPITotalText;

	// EN: SListView for plan review / ES: SListView para revision del plan
	TSharedPtr<SListView<TSharedPtr<FPGXScaffoldPlanStepRow>>> PlanStepsListView;
	TArray<TSharedPtr<FPGXScaffoldPlanStepRow>> PlanStepRows;

	void RefreshTemplateCards();
	void RefreshVariableFields();
	void RefreshTree();
	void RefreshValidation();
	void RefreshPlanReview();
	void UpdateFooter();
};
