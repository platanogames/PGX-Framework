// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "Validation/PGXRegistryValidationTypes.h"

class UDataTable;
class FPGXRegistryValidationService;
class FPGXRegistryFixService;

/**
 * EN: PGX Registry Validation Tab — Three-pane editor for DataTable validation.
 *     Left:   DataTable list with status badges (OK, WARN, ERROR)
 *     Center: Issue grid (severity, rule id, table, row, key, message)
 *     Right:  Issue detail + quick actions (apply fix, export, navigate)
 *
 *     Top action bar: Validate All, Validate Selected, Apply Safe Fixes,
 *     Export Report, Rebuild Runtime Preview Index.
 *
 * ES: Tab de Validacion del PGX Registry — Editor de tres paneles para validacion de DataTables.
 *     Izq:    Lista de DataTables con badges de estado (OK, WARN, ERROR)
 *     Centro: Grid de problemas (severidad, id de regla, tabla, fila, clave, mensaje)
 *     Der:    Detalle de problema + acciones rapidas (aplicar fix, exportar, navegar)
 *
 *     Barra de acciones: Validar Todo, Validar Seleccion, Aplicar Fixes Seguros,
 *     Exportar Reporte, Reconstruir Indice de Vista Previa Runtime.
 */
class PGXREGISTRYEDITOR_API SPGXRegistryValidationTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXRegistryValidationTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SPGXRegistryValidationTab() override;

private:
	// ═══════════════════════════════════════════════════════════════
	// EN: UI Build / ES: Construccion de UI
	// ═══════════════════════════════════════════════════════════════

	TSharedRef<SWidget> BuildToolbar();
	TSharedRef<SWidget> BuildLeftPanel();
	TSharedRef<SWidget> BuildCenterPanel();
	TSharedRef<SWidget> BuildRightPanel();

	// ═══════════════════════════════════════════════════════════════
	// EN: SListView Row Generators / ES: Generadores de Filas SListView
	// ═══════════════════════════════════════════════════════════════

	/** EN: Row for DataTable list (left panel) / ES: Fila para lista de DataTables (panel izquierdo) */
	TSharedRef<ITableRow> OnGenerateTableRow(
		TSharedPtr<FString> Item,
		const TSharedRef<STableViewBase>& OwnerTable);

	/** EN: Row for issue grid (center panel) / ES: Fila para grid de issues (panel central) */
	TSharedRef<ITableRow> OnGenerateIssueRow(
		TSharedPtr<FPGXRegistryValidationIssue> Item,
		const TSharedRef<STableViewBase>& OwnerTable);

	// ═══════════════════════════════════════════════════════════════
	// EN: Actions / ES: Acciones
	// ═══════════════════════════════════════════════════════════════

	FReply OnValidateAllClicked();
	FReply OnValidateSelectedClicked();
	FReply OnApplySafeFixesClicked();
	FReply OnExportReportClicked();
	FReply OnRebuildIndexClicked();
	FReply OnBrowseToAssetClicked();

	// ═══════════════════════════════════════════════════════════════
	// EN: Severity Filters / ES: Filtros de Severidad
	// ═══════════════════════════════════════════════════════════════

	void OnErrorFilterChanged(ECheckBoxState NewState);
	void OnWarningFilterChanged(ECheckBoxState NewState);
	void OnInfoFilterChanged(ECheckBoxState NewState);

	// ═══════════════════════════════════════════════════════════════
	// EN: Selection / ES: Seleccion
	// ═══════════════════════════════════════════════════════════════

	void OnTableSelected(TSharedPtr<FString> Item, ESelectInfo::Type SelectInfo);
	void OnIssueSelected(TSharedPtr<FPGXRegistryValidationIssue> Item, ESelectInfo::Type SelectInfo);

	// ═══════════════════════════════════════════════════════════════
	// EN: Refresh / ES: Actualizar
	// ═══════════════════════════════════════════════════════════════

	/** EN: Discover all PGXDataTableAssets and populate left panel / ES: Descubrir PGXDataTableAssets y llenar panel izquierdo */
	void RefreshTableList();

	/** EN: Filter issues by selected table / ES: Filtrar issues por tabla seleccionada */
	void RefreshIssueList();

	/** EN: Update right panel with selected issue / ES: Actualizar panel derecho con issue seleccionado */
	void RefreshDetailPanel();

	/** EN: Update status bar / ES: Actualizar barra de estado */
	void UpdateStatusBar();

	// ═══════════════════════════════════════════════════════════════
	// EN: Helpers / ES: Helpers
	// ═══════════════════════════════════════════════════════════════

	/** EN: Get status badge text for a table / ES: Obtener texto de badge de estado para una tabla */
	FString GetTableStatus(const FString& TableName) const;

	/** EN: Get severity color / ES: Obtener color de severidad */
	static FLinearColor GetSeverityColor(EPGXRegistryValidationSeverity Severity);

	// ═══════════════════════════════════════════════════════════════
	// EN: State / ES: Estado
	// ═══════════════════════════════════════════════════════════════

	// EN: Services / ES: Servicios
	TSharedPtr<FPGXRegistryValidationService> ValidationService;
	TSharedPtr<FPGXRegistryFixService> FixService;

	// EN: All validation issues (full set) / ES: Todos los issues de validacion (set completo)
	TArray<FPGXRegistryValidationIssue> AllIssues;

	// EN: Left panel — DataTable list / ES: Panel izquierdo — lista de DataTables
	TArray<TSharedPtr<FString>> TableDisplayNames;
	TSharedPtr<SListView<TSharedPtr<FString>>> TableListView;
	TSharedPtr<FString> SelectedTable;

	// EN: Center panel — filtered issues / ES: Panel central — issues filtrados
	TArray<TSharedPtr<FPGXRegistryValidationIssue>> FilteredIssues;
	TSharedPtr<SListView<TSharedPtr<FPGXRegistryValidationIssue>>> IssueListView;
	TSharedPtr<FPGXRegistryValidationIssue> SelectedIssue;

	// EN: Right panel — detail widgets / ES: Panel derecho — widgets de detalle
	TSharedPtr<STextBlock> DetailRuleText;
	TSharedPtr<STextBlock> DetailSeverityText;
	TSharedPtr<STextBlock> DetailTableText;
	TSharedPtr<STextBlock> DetailRowText;
	TSharedPtr<STextBlock> DetailKeyText;
	TSharedPtr<STextBlock> DetailAssetText;
	TSharedPtr<STextBlock> DetailMessageText;
	TSharedPtr<STextBlock> DetailFixableText;

	// EN: Status bar / ES: Barra de estado
	TSharedPtr<STextBlock> StatusText;

	// EN: Severity filter state / ES: Estado de filtros de severidad
	bool bShowErrors = true;
	bool bShowWarnings = true;
	bool bShowInfo = true;
};
