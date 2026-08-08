// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Inspector/SPGXRegistryValidationTab.h"
#include "PGXRegistryEditorModule.h"
#include "Validation/PGXRegistryValidationService.h"
#include "Validation/PGXRegistryFixService.h"
#include "Export/PGXRegistryReportExporter.h"
#include "Registry/PGXDataRegistrySubsystem.h"
#include "Registry/PGXRegistryDefinition.h"

#include "Engine/DataTable.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/STableRow.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "Style/PGXVisualTokens.h"
// EN: Cannot use SPGXPanelHeader/SectionDivider/FooterBar/StatusBadge — circular dep PGXCoreEditor <-> PGXRegistryEditor
// ES: No puede usar SPGXPanelHeader/SectionDivider/FooterBar/StatusBadge — dep circular PGXCoreEditor <-> PGXRegistryEditor
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Widgets/Input/SCheckBox.h"

#define LOCTEXT_NAMESPACE "PGXRegistryValidation"

// EN: Registry Validation uses the canonical Data Registry system color.
// ES: Registry Validation usa el color canonico del sistema Data Registry.
namespace { static const FLinearColor GValidationColor = PGX::System::DataRegistry; }

// ============================================================================
// EN: Construction / Destruction
// ES: Construccion / Destruccion
// ============================================================================

void SPGXRegistryValidationTab::Construct(const FArguments& /*InArgs*/)
{
	ValidationService = MakeShared<FPGXRegistryValidationService>();
	FixService = MakeShared<FPGXRegistryFixService>();

	UE_LOG(LogPGXRegistryEditor, Log, TEXT("[RegistryValidation] Construct"));

	ChildSlot
	[
		SNew(SVerticalBox)

		// EN: Manual panel header (circular dep prevents SPGXPanelHeader)
		// ES: Header manual (dep circular impide SPGXPanelHeader)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FSlateColor(PGX::System::DataRegistry))
			.Padding(0)
			[
				SNew(SBox)
				.HeightOverride(PGX::Height::AccentBar)
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(PGX::Spacing::MD, PGX::Spacing::SM)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("PanelTitle", "REGISTRY VALIDATION"))
					.Font(PGX::Font::SubHeader())
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("PanelSubtitle", "Validate registry DataTables against 18 rules"))
					.Font(PGX::Font::Caption())
					.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
				]
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNullWidget::NullWidget
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				BuildToolbar()
			]
		]

		// EN: Severity quick-filter chips / ES: Chips de filtro rapido de severidad
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f, 2.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 8, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("FilterLabel", "Show:"))
				.Font(PGX::Font::Badge())
			]

			// EN: Error filter / ES: Filtro de errores
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0)
			[
				SNew(SCheckBox)
				.IsChecked(ECheckBoxState::Checked)
				.OnCheckStateChanged(this, &SPGXRegistryValidationTab::OnErrorFilterChanged)
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("FilterError", "Errors"))
				.Font(PGX::Font::Badge())
				.ColorAndOpacity(FSlateColor(PGX::Semantic::Error))
			]

			// EN: Warning filter / ES: Filtro de warnings
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8, 0, 4, 0)
			[
				SNew(SCheckBox)
				.IsChecked(ECheckBoxState::Checked)
				.OnCheckStateChanged(this, &SPGXRegistryValidationTab::OnWarningFilterChanged)
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("FilterWarning", "Warnings"))
				.Font(PGX::Font::Badge())
				.ColorAndOpacity(FSlateColor(PGX::Semantic::Warn))
			]

			// EN: Info filter / ES: Filtro de info
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8, 0, 4, 0)
			[
				SNew(SCheckBox)
				.IsChecked(ECheckBoxState::Checked)
				.OnCheckStateChanged(this, &SPGXRegistryValidationTab::OnInfoFilterChanged)
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("FilterInfo", "Info"))
				.Font(PGX::Font::Badge())
				.ColorAndOpacity(FSlateColor(PGX::Semantic::Info))
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SSeparator)
		]

		// EN: Three-pane content / ES: Contenido de tres paneles
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SSplitter)
			.Orientation(Orient_Horizontal)

			// EN: Left panel — DataTable list / ES: Panel izquierdo — lista de DataTables
			+ SSplitter::Slot()
			.Value(0.2f)
			[
				BuildLeftPanel()
			]

			// EN: Center panel — Issue grid / ES: Panel central — grid de issues
			+ SSplitter::Slot()
			.Value(0.5f)
			[
				BuildCenterPanel()
			]

			// EN: Right panel — Issue detail / ES: Panel derecho — detalle de issue
			+ SSplitter::Slot()
			.Value(0.3f)
			[
				BuildRightPanel()
			]
		]

		// EN: Footer bar (manual — circular dep) / ES: Barra de footer (manual — dep circular)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(FMargin(PGX::Spacing::MD, PGX::Spacing::SM))
			[
				SAssignNew(StatusText, STextBlock)
				.Text(LOCTEXT("StatusReady", "Ready - Click 'Validate All' to scan registry DataTables"))
				.Font(PGX::Font::BodySmall())
				.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
			]
		]
	];

	// EN: Initial table discovery / ES: Descubrimiento inicial de tablas
	RefreshTableList();
}

SPGXRegistryValidationTab::~SPGXRegistryValidationTab()
{
	UE_LOG(LogPGXRegistryEditor, Log, TEXT("[RegistryValidation] Destructor — cleanup"));
	ValidationService.Reset();
	FixService.Reset();
}

// ============================================================================
// EN: UI Build
// ES: Construccion de UI
// ============================================================================

TSharedRef<SWidget> SPGXRegistryValidationTab::BuildToolbar()
{
	// EN: Returns only action buttons — title/badge now in SPGXPanelHeader
	// ES: Retorna solo botones de accion — titulo/badge ahora en SPGXPanelHeader
	return SNew(SHorizontalBox)

		// EN: Validate All / ES: Validar Todo
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("ValidateAll", "Validate All"))
			.ToolTipText(LOCTEXT("ValidateAllTip", "Run all 18 validation rules on every discovered DataTable"))
			.OnClicked(this, &SPGXRegistryValidationTab::OnValidateAllClicked)
		]

		// EN: Validate Selected / ES: Validar Seleccion
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("ValidateSelected", "Validate Selected"))
			.ToolTipText(LOCTEXT("ValidateSelectedTip", "Run validation on the selected DataTable only"))
			.OnClicked(this, &SPGXRegistryValidationTab::OnValidateSelectedClicked)
		]

		// EN: Apply Safe Fixes / ES: Aplicar Fixes Seguros
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("ApplyFixes", "Apply Safe Fixes"))
			.ToolTipText(LOCTEXT("ApplyFixesTip", "Apply safe autofix for RDT030 (null removal) and RDT033 (redirector guidance)"))
			.OnClicked(this, &SPGXRegistryValidationTab::OnApplySafeFixesClicked)
		]

		// EN: Export Report / ES: Exportar Reporte
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("ExportReport", "Export Report"))
			.ToolTipText(LOCTEXT("ExportReportTip", "Export validation results to JSON and CSV in Saved/PGXRegistry/"))
			.OnClicked(this, &SPGXRegistryValidationTab::OnExportReportClicked)
		]

		// EN: Rebuild Index / ES: Reconstruir Indice
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("RebuildIndex", "Rebuild Index"))
			.ToolTipText(LOCTEXT("RebuildIndexTip", "Re-ingest all registry definitions to rebuild the runtime index"))
			.OnClicked(this, &SPGXRegistryValidationTab::OnRebuildIndexClicked)
		];
}

TSharedRef<SWidget> SPGXRegistryValidationTab::BuildLeftPanel()
{
	return SNew(SVerticalBox)

		// EN: Section header (manual — circular dep) / ES: Cabecera de seccion (manual — dep circular)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, PGX::Spacing::XL, 0, PGX::Spacing::SM)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, PGX::Spacing::SM, 0)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FSlateColor(PGX::System::DataRegistry))
				.Padding(0)
				[
					SNew(SBox).WidthOverride(3.0f).HeightOverride(14.0f)
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SecDataTables", "DATATABLES"))
				.Font(PGX::Font::CaptionBold())
				.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
			]
		]

		// EN: Table list / ES: Lista de tablas
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SAssignNew(TableListView, SListView<TSharedPtr<FString>>)
			.ListItemsSource(&TableDisplayNames)
			.OnGenerateRow(this, &SPGXRegistryValidationTab::OnGenerateTableRow)
			.OnSelectionChanged(this, &SPGXRegistryValidationTab::OnTableSelected)
			.SelectionMode(ESelectionMode::Single)
		];
}

TSharedRef<SWidget> SPGXRegistryValidationTab::BuildCenterPanel()
{
	return SNew(SVerticalBox)

		// EN: Section header (manual — circular dep) / ES: Cabecera de seccion (manual — dep circular)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, PGX::Spacing::XL, 0, PGX::Spacing::SM)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, PGX::Spacing::SM, 0)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FSlateColor(PGX::System::DataRegistry))
				.Padding(0)
				[
					SNew(SBox).WidthOverride(3.0f).HeightOverride(14.0f)
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SecIssues", "VALIDATION ISSUES"))
				.Font(PGX::Font::CaptionBold())
				.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
			]
		]

		// EN: Issue list with real SHeaderRow / ES: Lista de issues con SHeaderRow real
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SAssignNew(IssueListView, SListView<TSharedPtr<FPGXRegistryValidationIssue>>)
			.ListItemsSource(&FilteredIssues)
			.OnGenerateRow(this, &SPGXRegistryValidationTab::OnGenerateIssueRow)
			.OnSelectionChanged(this, &SPGXRegistryValidationTab::OnIssueSelected)
			.SelectionMode(ESelectionMode::Single)
			.HeaderRow(
				SNew(SHeaderRow)
				+ SHeaderRow::Column("Severity").DefaultLabel(LOCTEXT("ColSev", "Sev")).FillWidth(0.08f)
				+ SHeaderRow::Column("Rule").DefaultLabel(LOCTEXT("ColRule2", "Rule")).FillWidth(0.1f)
				+ SHeaderRow::Column("Table").DefaultLabel(LOCTEXT("ColTable2", "Table")).FillWidth(0.2f)
				+ SHeaderRow::Column("Row").DefaultLabel(LOCTEXT("ColRow2", "Row")).FillWidth(0.15f)
				+ SHeaderRow::Column("Message").DefaultLabel(LOCTEXT("ColMsg", "Message")).FillWidth(0.47f)
			)
		];
}

TSharedRef<SWidget> SPGXRegistryValidationTab::BuildRightPanel()
{
	return SNew(SVerticalBox)

		// EN: Section header (manual — circular dep) / ES: Cabecera de seccion (manual — dep circular)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, PGX::Spacing::XL, 0, PGX::Spacing::SM)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, PGX::Spacing::SM, 0)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FSlateColor(PGX::System::DataRegistry))
				.Padding(0)
				[
					SNew(SBox).WidthOverride(3.0f).HeightOverride(14.0f)
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SecDetail", "ISSUE DETAIL"))
				.Font(PGX::Font::CaptionBold())
				.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
			]
		]

		// EN: Detail fields / ES: Campos de detalle
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(4.0f)
		[
			SNew(SScrollBox)

			+ SScrollBox::Slot()
			.Padding(0.0f, 2.0f)
			[
				SNew(SVerticalBox)

				// Rule ID
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 2.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("LblRule", "Rule: "))
						.Font(PGX::Font::Badge())
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SAssignNew(DetailRuleText, STextBlock)
						.Text(LOCTEXT("NoSelection", "-"))
						.Font(PGX::Font::Mono())
					]
				]

				// Severity
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 2.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("LblSeverity", "Severity: "))
						.Font(PGX::Font::Badge())
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SAssignNew(DetailSeverityText, STextBlock)
						.Text(LOCTEXT("NoSelection2", "-"))
						.Font(PGX::Font::Mono())
					]
				]

				// Table
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 2.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("LblTable", "Table: "))
						.Font(PGX::Font::Badge())
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SAssignNew(DetailTableText, STextBlock)
						.Text(LOCTEXT("NoSelection3", "-"))
						.Font(PGX::Font::Mono())
					]
				]

				// Row
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 2.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("LblRow", "Row: "))
						.Font(PGX::Font::Badge())
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SAssignNew(DetailRowText, STextBlock)
						.Text(LOCTEXT("NoSelection4", "-"))
						.Font(PGX::Font::Mono())
					]
				]

				// Key Context
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 2.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("LblKey", "Key: "))
						.Font(PGX::Font::Badge())
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SAssignNew(DetailKeyText, STextBlock)
						.Text(LOCTEXT("NoSelection5", "-"))
						.Font(PGX::Font::Mono())
					]
				]

				// Asset Path
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 2.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("LblAsset", "Asset: "))
						.Font(PGX::Font::Badge())
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SAssignNew(DetailAssetText, STextBlock)
						.Text(LOCTEXT("NoSelection6", "-"))
						.Font(PGX::Font::Mono())
						.AutoWrapText(true)
					]
				]

				// Auto-Fixable
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 2.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("LblFixable", "Auto-Fixable: "))
						.Font(PGX::Font::Badge())
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SAssignNew(DetailFixableText, STextBlock)
						.Text(LOCTEXT("NoSelection7", "-"))
						.Font(PGX::Font::Mono())
					]
				]

				// EN: Browse To Asset button / ES: Boton para navegar al asset
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 6.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("BrowseToAsset", "Browse To Asset"))
					.ToolTipText(LOCTEXT("BrowseToAssetTip", "Navigate to the related asset in Content Browser"))
					.OnClicked(this, &SPGXRegistryValidationTab::OnBrowseToAssetClicked)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 8.0f, 0.0f, 4.0f)
				[
					SNew(SSeparator)
				]

				// Message (full, wrapped)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 2.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("LblMessage", "Message:"))
					.Font(PGX::Font::Badge())
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(4.0f, 2.0f)
				[
					SAssignNew(DetailMessageText, STextBlock)
					.Text(LOCTEXT("NoSelection8", "Select an issue to view details"))
					.Font(PGX::Font::BodySmall())
					.AutoWrapText(true)
					.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
				]
			]
		];
}

// ============================================================================
// EN: SListView Row Generators
// ES: Generadores de Filas SListView
// ============================================================================

TSharedRef<ITableRow> SPGXRegistryValidationTab::OnGenerateTableRow(
	TSharedPtr<FString> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	const FString DisplayName = Item.IsValid() ? *Item : TEXT("(null)");
	const FString Status = GetTableStatus(DisplayName);

	FLinearColor BadgeColor = FLinearColor(0.1f, 0.8f, 0.2f); // Green = OK
	if (Status == TEXT("ERROR"))
	{
		BadgeColor = FLinearColor(1.0f, 0.2f, 0.2f);
	}
	else if (Status == TEXT("WARN"))
	{
		BadgeColor = FLinearColor(1.0f, 0.8f, 0.0f);
	}

	return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
		.Padding(FMargin(4.0f, 1.0f))
		[
			SNew(SHorizontalBox)

			// EN: Status badge (manual — circular dep) / ES: Badge de estado (manual — dep circular)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FSlateColor(BadgeColor))
				.Padding(FMargin(6.0f, 1.0f))
				[
					SNew(STextBlock)
					.Text(FText::FromString(Status))
					.Font(PGX::Font::Badge())
					.ColorAndOpacity(FSlateColor(PGX::Text::OnColor))
				]
			]

			// EN: Table name / ES: Nombre de tabla
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(DisplayName))
				.Font(PGX::Font::BodySmall())
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			]
		];
}

TSharedRef<ITableRow> SPGXRegistryValidationTab::OnGenerateIssueRow(
	TSharedPtr<FPGXRegistryValidationIssue> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	if (!Item.IsValid())
	{
		return SNew(STableRow<TSharedPtr<FPGXRegistryValidationIssue>>, OwnerTable)
			[
				SNew(STextBlock).Text(LOCTEXT("NullIssue", "(null)"))
			];
	}

	const FLinearColor SevColor = GetSeverityColor(Item->Severity);

	return SNew(STableRow<TSharedPtr<FPGXRegistryValidationIssue>>, OwnerTable)
		.Padding(FMargin(4.0f, 1.0f))
		[
			SNew(SHorizontalBox)

			// Severity
			+ SHorizontalBox::Slot()
			.FillWidth(0.08f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->GetSeverityString()))
				.Font(PGX::Font::CaptionBold())
				.ColorAndOpacity(FSlateColor(SevColor))
			]

			// Rule ID
			+ SHorizontalBox::Slot()
			.FillWidth(0.1f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->GetRuleIdString()))
				.Font(PGX::Font::Mono())
			]

			// Table
			+ SHorizontalBox::Slot()
			.FillWidth(0.2f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->TableName))
				.Font(PGX::Font::Caption())
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			]

			// Row
			+ SHorizontalBox::Slot()
			.FillWidth(0.15f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->RowName.ToString()))
				.Font(PGX::Font::Mono())
				.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
			]

			// Message
			+ SHorizontalBox::Slot()
			.FillWidth(0.47f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->Message))
				.Font(PGX::Font::Caption())
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			]
		];
}

// ============================================================================
// EN: Actions
// ES: Acciones
// ============================================================================

FReply SPGXRegistryValidationTab::OnValidateAllClicked()
{
	UE_LOG(LogPGXRegistryEditor, Log, TEXT("[RegistryValidation] Validate All clicked"));

	if (!ValidationService.IsValid())
	{
		return FReply::Handled();
	}

	AllIssues.Empty();
	ValidationService->ValidateAll(AllIssues);
	UE_LOG(LogPGXRegistryEditor, Log, TEXT("[RegistryValidation] Validation complete: %d issues found"), AllIssues.Num());

	RefreshIssueList();
	UpdateStatusBar();

	return FReply::Handled();
}

FReply SPGXRegistryValidationTab::OnValidateSelectedClicked()
{
	if (!ValidationService.IsValid() || !SelectedTable.IsValid())
	{
		FNotificationInfo Info(LOCTEXT("NoTableSelected", "Select a DataTable first"));
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
		return FReply::Handled();
	}

	// EN: Find the DataTable by name / ES: Encontrar el DataTable por nombre
	UDataTable* Table = nullptr;
	for (TObjectIterator<UDataTable> It; It; ++It)
	{
		if (It->GetName() == *SelectedTable)
		{
			Table = *It;
			break;
		}
	}

	if (!Table)
	{
		FNotificationInfo Info(FText::Format(
			LOCTEXT("TableNotFound", "DataTable '{0}' not found in memory"),
			FText::FromString(*SelectedTable)));
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
		return FReply::Handled();
	}

	AllIssues.Empty();
	ValidationService->ValidateTable(Table, nullptr, AllIssues);

	RefreshIssueList();
	UpdateStatusBar();

	return FReply::Handled();
}

FReply SPGXRegistryValidationTab::OnApplySafeFixesClicked()
{
	if (!FixService.IsValid() || AllIssues.IsEmpty())
	{
		FNotificationInfo Info(LOCTEXT("NoIssues", "No issues to fix. Run validation first."));
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
		return FReply::Handled();
	}

	TArray<FString> FixLog;
	const int32 FixedCount = FixService->ApplySafeFixes(AllIssues, FixLog);

	FNotificationInfo Info(FText::Format(
		LOCTEXT("FixResult", "Applied {0} safe fix(es). Re-validate to confirm."),
		FText::AsNumber(FixedCount)));
	Info.ExpireDuration = 5.0f;
	FSlateNotificationManager::Get().AddNotification(Info);

	if (StatusText.IsValid())
	{
		StatusText->SetText(FText::Format(
			LOCTEXT("StatusAfterFix", "Applied {0} fix(es) — re-validate to confirm changes"),
			FText::AsNumber(FixedCount)));
		StatusText->SetColorAndOpacity(FSlateColor(PGX::Semantic::Good));
	}

	return FReply::Handled();
}

FReply SPGXRegistryValidationTab::OnExportReportClicked()
{
	if (AllIssues.IsEmpty())
	{
		FNotificationInfo Info(LOCTEXT("NoExportData", "No validation results to export. Run validation first."));
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
		return FReply::Handled();
	}

	const FString JSONPath = FPGXRegistryReportExporter::ExportToJSON(AllIssues);
	const FString CSVPath = FPGXRegistryReportExporter::ExportToCSV(AllIssues);

	FString NotifyMsg = TEXT("Exported:");
	if (!JSONPath.IsEmpty()) { NotifyMsg += FString::Printf(TEXT("\nJSON: %s"), *JSONPath); }
	if (!CSVPath.IsEmpty()) { NotifyMsg += FString::Printf(TEXT("\nCSV: %s"), *CSVPath); }

	FNotificationInfo Info(FText::FromString(NotifyMsg));
	Info.ExpireDuration = 6.0f;
	FSlateNotificationManager::Get().AddNotification(Info);

	if (StatusText.IsValid())
	{
		StatusText->SetText(FText::FromString(FString::Printf(TEXT("Report exported to %s"),
			*FPGXRegistryReportExporter::GetExportDirectory())));
		StatusText->SetColorAndOpacity(FSlateColor(PGX::Semantic::Good));
	}

	return FReply::Handled();
}

FReply SPGXRegistryValidationTab::OnRebuildIndexClicked()
{
	// EN: Access the subsystem if PIE is running, otherwise show guidance
	// ES: Acceder al subsistema si PIE esta corriendo, sino mostrar guia
	UPGXDataRegistrySubsystem* Sub = nullptr;
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::PIE && Context.World())
			{
				UGameInstance* GI = Context.World()->GetGameInstance();
				if (GI)
				{
					Sub = GI->GetSubsystem<UPGXDataRegistrySubsystem>();
					if (Sub) break;
				}
			}
		}
	}

	if (Sub)
	{
		Sub->IngestAllDefinitions();

		FNotificationInfo Info(LOCTEXT("IndexRebuilt", "Runtime index rebuilt from definitions"));
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
	}
	else
	{
		FNotificationInfo Info(LOCTEXT("NoPIE", "Start PIE to rebuild runtime index. Use 'pgx.registry.ingest' in PIE console."));
		Info.ExpireDuration = 5.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
	}

	return FReply::Handled();
}

// ============================================================================
// EN: Selection
// ES: Seleccion
// ============================================================================

void SPGXRegistryValidationTab::OnTableSelected(TSharedPtr<FString> Item, ESelectInfo::Type /*SelectInfo*/)
{
	SelectedTable = Item;
	RefreshIssueList();
}

void SPGXRegistryValidationTab::OnIssueSelected(TSharedPtr<FPGXRegistryValidationIssue> Item, ESelectInfo::Type /*SelectInfo*/)
{
	SelectedIssue = Item;
	RefreshDetailPanel();
}

// ============================================================================
// EN: Refresh
// ES: Actualizar
// ============================================================================

void SPGXRegistryValidationTab::RefreshTableList()
{
	TableDisplayNames.Empty();

	// EN: Discover DataTables with FPGXRegistryCategoryRow via AssetRegistry
	// ES: Descubrir DataTables con FPGXRegistryCategoryRow via AssetRegistry
	const IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

	TArray<FAssetData> Assets;
	AssetRegistry.GetAssetsByClass(UDataTable::StaticClass()->GetClassPathName(), Assets, true);

	for (const FAssetData& Asset : Assets)
	{
		TableDisplayNames.Add(MakeShared<FString>(Asset.AssetName.ToString()));
	}

	// EN: Sort alphabetically / ES: Ordenar alfabeticamente
	TableDisplayNames.Sort([](const TSharedPtr<FString>& A, const TSharedPtr<FString>& B)
	{
		return *A < *B;
	});

	if (TableListView.IsValid())
	{
		TableListView->RequestListRefresh();
	}
}

void SPGXRegistryValidationTab::RefreshIssueList()
{
	FilteredIssues.Empty();

	for (const FPGXRegistryValidationIssue& Issue : AllIssues)
	{
		// EN: Apply table filter / ES: Aplicar filtro de tabla
		if (SelectedTable.IsValid() && !SelectedTable->IsEmpty())
		{
			if (Issue.TableName != *SelectedTable)
			{
				continue;
			}
		}

		// EN: Apply severity filter / ES: Aplicar filtro de severidad
		if (Issue.Severity == EPGXRegistryValidationSeverity::Error && !bShowErrors) continue;
		if (Issue.Severity == EPGXRegistryValidationSeverity::Warning && !bShowWarnings) continue;
		if (Issue.Severity != EPGXRegistryValidationSeverity::Error &&
			Issue.Severity != EPGXRegistryValidationSeverity::Warning && !bShowInfo) continue;

		FilteredIssues.Add(MakeShared<FPGXRegistryValidationIssue>(Issue));
	}

	if (IssueListView.IsValid())
	{
		IssueListView->RequestListRefresh();
	}

	// EN: Clear detail on filter change / ES: Limpiar detalle al cambiar filtro
	SelectedIssue.Reset();
	RefreshDetailPanel();
}

void SPGXRegistryValidationTab::RefreshDetailPanel()
{
	if (!SelectedIssue.IsValid())
	{
		if (DetailRuleText.IsValid())     { DetailRuleText->SetText(FText::FromString(TEXT("-"))); }
		if (DetailSeverityText.IsValid())  { DetailSeverityText->SetText(FText::FromString(TEXT("-"))); }
		if (DetailTableText.IsValid())     { DetailTableText->SetText(FText::FromString(TEXT("-"))); }
		if (DetailRowText.IsValid())       { DetailRowText->SetText(FText::FromString(TEXT("-"))); }
		if (DetailKeyText.IsValid())       { DetailKeyText->SetText(FText::FromString(TEXT("-"))); }
		if (DetailAssetText.IsValid())     { DetailAssetText->SetText(FText::FromString(TEXT("-"))); }
		if (DetailFixableText.IsValid())   { DetailFixableText->SetText(FText::FromString(TEXT("-"))); }
		if (DetailMessageText.IsValid())
		{
			DetailMessageText->SetText(LOCTEXT("SelectIssue", "Select an issue to view details"));
			DetailMessageText->SetColorAndOpacity(FSlateColor(PGX::Text::Secondary));
		}
		return;
	}

	const FPGXRegistryValidationIssue& Issue = *SelectedIssue;

	if (DetailRuleText.IsValid())
	{
		DetailRuleText->SetText(FText::FromString(Issue.GetRuleIdString()));
	}
	if (DetailSeverityText.IsValid())
	{
		DetailSeverityText->SetText(FText::FromString(Issue.GetSeverityString()));
		DetailSeverityText->SetColorAndOpacity(FSlateColor(GetSeverityColor(Issue.Severity)));
	}
	if (DetailTableText.IsValid())
	{
		DetailTableText->SetText(FText::FromString(Issue.TableName));
	}
	if (DetailRowText.IsValid())
	{
		DetailRowText->SetText(FText::FromString(Issue.RowName.ToString()));
	}
	if (DetailKeyText.IsValid())
	{
		DetailKeyText->SetText(FText::FromString(Issue.KeyContext.IsEmpty() ? TEXT("-") : Issue.KeyContext));
	}
	if (DetailAssetText.IsValid())
	{
		DetailAssetText->SetText(FText::FromString(Issue.AssetPath.IsEmpty() ? TEXT("-") : Issue.AssetPath));
	}
	if (DetailFixableText.IsValid())
	{
		DetailFixableText->SetText(FText::FromString(Issue.bAutoFixable ? TEXT("Yes") : TEXT("No")));
		DetailFixableText->SetColorAndOpacity(FSlateColor(
			Issue.bAutoFixable ? PGX::Semantic::Good : PGX::Text::Muted));
	}
	if (DetailMessageText.IsValid())
	{
		DetailMessageText->SetText(FText::FromString(Issue.Message));
		DetailMessageText->SetColorAndOpacity(FSlateColor(PGX::Text::Primary));
	}
}

void SPGXRegistryValidationTab::UpdateStatusBar()
{
	if (!StatusText.IsValid())
	{
		return;
	}

	const int32 ErrorCount = FPGXRegistryValidationService::CountErrors(AllIssues);
	const int32 WarningCount = FPGXRegistryValidationService::CountWarnings(AllIssues);
	const int32 InfoCount = AllIssues.Num() - ErrorCount - WarningCount;

	if (AllIssues.IsEmpty())
	{
		StatusText->SetText(LOCTEXT("StatusClean", "Validation complete - No issues found"));
		StatusText->SetColorAndOpacity(FSlateColor(PGX::Semantic::Good));
	}
	else
	{
		StatusText->SetText(FText::Format(
			LOCTEXT("StatusSummary", "Found {0} issue(s): {1} error(s), {2} warning(s), {3} info"),
			FText::AsNumber(AllIssues.Num()),
			FText::AsNumber(ErrorCount),
			FText::AsNumber(WarningCount),
			FText::AsNumber(InfoCount)));

		StatusText->SetColorAndOpacity(FSlateColor(
			ErrorCount > 0 ? PGX::Semantic::Error : PGX::Semantic::Warn));
	}
}

// ============================================================================
// EN: Helpers
// ES: Helpers
// ============================================================================

FString SPGXRegistryValidationTab::GetTableStatus(const FString& TableName) const
{
	bool bHasError = false;
	bool bHasWarning = false;

	for (const FPGXRegistryValidationIssue& Issue : AllIssues)
	{
		if (Issue.TableName == TableName)
		{
			if (Issue.Severity == EPGXRegistryValidationSeverity::Error)
			{
				bHasError = true;
				break; // EN: Error is the worst — no need to check further / ES: Error es lo peor — no seguir
			}
			if (Issue.Severity == EPGXRegistryValidationSeverity::Warning)
			{
				bHasWarning = true;
			}
		}
	}

	if (bHasError) return TEXT("ERROR");
	if (bHasWarning) return TEXT("WARN");
	return TEXT("OK");
}

FLinearColor SPGXRegistryValidationTab::GetSeverityColor(EPGXRegistryValidationSeverity Severity)
{
	switch (Severity)
	{
	case EPGXRegistryValidationSeverity::Error:   return PGX::Semantic::Error;
	case EPGXRegistryValidationSeverity::Warning: return PGX::Semantic::Warn;
	case EPGXRegistryValidationSeverity::Info:    return PGX::Semantic::Info;
	default:                              return PGX::Text::Muted;
	}
}

// ============================================================================
// EN: Severity Filter Handlers
// ES: Handlers de Filtros de Severidad
// ============================================================================

void SPGXRegistryValidationTab::OnErrorFilterChanged(ECheckBoxState NewState)
{
	bShowErrors = (NewState == ECheckBoxState::Checked);
	UE_LOG(LogPGXRegistryEditor, Log, TEXT("[RegistryValidation] Error filter: %s"), bShowErrors ? TEXT("ON") : TEXT("OFF"));
	RefreshIssueList();
}

void SPGXRegistryValidationTab::OnWarningFilterChanged(ECheckBoxState NewState)
{
	bShowWarnings = (NewState == ECheckBoxState::Checked);
	UE_LOG(LogPGXRegistryEditor, Log, TEXT("[RegistryValidation] Warning filter: %s"), bShowWarnings ? TEXT("ON") : TEXT("OFF"));
	RefreshIssueList();
}

void SPGXRegistryValidationTab::OnInfoFilterChanged(ECheckBoxState NewState)
{
	bShowInfo = (NewState == ECheckBoxState::Checked);
	UE_LOG(LogPGXRegistryEditor, Log, TEXT("[RegistryValidation] Info filter: %s"), bShowInfo ? TEXT("ON") : TEXT("OFF"));
	RefreshIssueList();
}

// ============================================================================
// EN: Browse To Asset
// ES: Navegar al Asset
// ============================================================================

FReply SPGXRegistryValidationTab::OnBrowseToAssetClicked()
{
	if (!SelectedIssue.IsValid() || SelectedIssue->AssetPath.IsEmpty())
	{
		FNotificationInfo Info(LOCTEXT("NoAssetPath", "No asset path available for this issue"));
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
		return FReply::Handled();
	}

	UE_LOG(LogPGXRegistryEditor, Log, TEXT("[RegistryValidation] Browse to asset: %s"), *SelectedIssue->AssetPath);

	const IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
	const FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(FSoftObjectPath(SelectedIssue->AssetPath));

	if (AssetData.IsValid())
	{
		FContentBrowserModule& CB = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		TArray<FAssetData> SyncAssets = { AssetData };
		CB.Get().SyncBrowserToAssets(SyncAssets);
	}
	else
	{
		UE_LOG(LogPGXRegistryEditor, Warning, TEXT("[RegistryValidation] Asset not found in registry: %s"), *SelectedIssue->AssetPath);
		FNotificationInfo Info(FText::Format(
			LOCTEXT("AssetNotFound", "Asset not found: {0}"),
			FText::FromString(SelectedIssue->AssetPath)));
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
