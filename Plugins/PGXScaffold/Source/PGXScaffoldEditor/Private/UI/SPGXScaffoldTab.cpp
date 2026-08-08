// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "UI/SPGXScaffoldTab.h"
#include "PGXScaffoldEditor.h"
#include "Core/PGXProjectAnalyzer.h"
#include "Core/PGXScaffoldTemplateRegistry.h"
#include "Core/PGXHierarchyResolver.h"
#include "Core/PGXScaffoldValidator.h"
#include "Core/PGXBuildPlanGenerator.h"
#include "Core/PGXScaffoldExecutor.h"
#include "Core/PGXScaffoldLogger.h"
#include "Templates/PGXBuiltInTemplates.h"
#include "UI/SPGXScaffoldHierarchyTree.h"
#include "UI/SPGXScaffoldProgressWindow.h"

#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/SBoxPanel.h"
#include "Styling/AppStyle.h"

#include "Style/PGXVisualTokens.h"
#include "Style/PGXEditorStyle.h"
#include "Widgets/SPGXPremiumShell.h"
#include "Widgets/SPGXSectionDivider.h"
#include "Widgets/SPGXKPIChip.h"
#include "Widgets/SPGXFooterBar.h"
#include "Widgets/SPGXStatusBadge.h"
#include "Notifications/PGXEditorNotification.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
// EN: ILiveCodingModule.h is only needed when C++ generation actions are available
// ES: ILiveCodingModule.h solo sera necesario cuando existan acciones de generacion C++
#include "Widgets/Views/SHeaderRow.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/FileManager.h"

#define LOCTEXT_NAMESPACE "PGXScaffoldTab"

void SPGXScaffoldTab::Construct(const FArguments& /*InArgs*/)
{
	// EN: Initialize pipeline components
	// ES: Inicializar componentes del pipeline
	Analyzer = MakeShared<FPGXProjectAnalyzer>();
	Resolver = MakeShared<FPGXHierarchyResolver>();
	Validator = MakeShared<FPGXScaffoldValidator>();
	PlanGenerator = MakeShared<FPGXBuildPlanGenerator>();
	Executor = MakeShared<FPGXScaffoldExecutor>();
	Logger = MakeShared<FPGXScaffoldLogger>();

	// EN: Register built-in templates if not already done
	// ES: Registrar templates built-in si no se ha hecho
	FPGXScaffoldTemplateRegistry& Registry = FPGXScaffoldTemplateRegistry::Get();
	if (Registry.GetAllTemplates().Num() == 0)
	{
		FPGXBuiltInTemplates::RegisterAll(Registry);
	}

	// EN: Analyze project
	// ES: Analizar proyecto
	ProjectInfo = Analyzer->Analyze();

	// EN: Default category selection
	// ES: Seleccion de categoria por defecto
	TArray<FName> Categories = Registry.GetCategories();
	if (Categories.Num() > 0)
	{
		SelectedCategory = Categories[0];
	}

	ChildSlot
	[
		SNew(SPGXPremiumShell)
		.SystemColor(PGX::System::Scaffold)
		.Title(LOCTEXT("PanelTitle", "PGX Scaffold"))
		.Subtitle(LOCTEXT("PanelSubtitle", "Automated Project Scaffolding"))
		.Icon(FPGXEditorStyle::Get().GetBrush("PGXEditor.Icon.Scaffold"))
		.bShowFooter(true)
		.StatusText_Lambda([this]() -> FText
		{
			FString TemplateName = SelectedTemplate ? SelectedTemplate->DisplayName.ToString() : TEXT("None");
			int32 ItemCount = 0;
			if (SelectedTemplate)
			{
				for (const auto& Root : TreeRootNodes)
				{
					if (Root->bChecked) { ItemCount++; }
					for (const auto& Child : Root->Children)
					{
						if (Child->bChecked) { ItemCount++; }
					}
				}
				if (ItemCount == 0) { ItemCount = SelectedTemplate->GetItemCount(); }
			}
			return FText::Format(LOCTEXT("Footer", "PGXScaffold v0.2.0 | Template: {0} | Items: {1}"),
				FText::FromString(TemplateName), FText::AsNumber(ItemCount));
		})
		.Content()
		[
			SAssignNew(PhaseSwitcher, SWidgetSwitcher)
				.WidgetIndex_Lambda([this]() { return CurrentPhase; })

			+ SWidgetSwitcher::Slot() [ BuildPhaseA() ]
			+ SWidgetSwitcher::Slot() [ BuildPhaseB() ]
			+ SWidgetSwitcher::Slot() [ BuildPhaseC() ]
		]
	];

	UpdateFooter();
}

SPGXScaffoldTab::~SPGXScaffoldTab()
{
}

// ============================================================================
// Phase A: Template Selection
// ============================================================================

TSharedRef<SWidget> SPGXScaffoldTab::BuildPhaseA()
{
	TSharedRef<SVerticalBox> PhaseAWidget = SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight().Padding(PGX::Spacing::MD)
		[
			SNew(SPGXSectionDivider)
				.Title(LOCTEXT("PhaseATitle", "Phase A: Select Template"))
		]

		+ SVerticalBox::Slot().FillHeight(1.0f).Padding(PGX::Spacing::MD)
		[
			SNew(SHorizontalBox)

			// EN: Category list (left) / ES: Lista de categorias (izquierda)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SBox).WidthOverride(180.0f)
				[
					BuildCategoryList()
				]
			]

			+ SHorizontalBox::Slot().AutoWidth().Padding(PGX::Spacing::SM, 0.0f)
			[
				SNew(SSpacer)
			]

			// EN: Template cards (right) / ES: Cards de templates (derecha)
			+ SHorizontalBox::Slot().FillWidth(1.0f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SAssignNew(TemplateCardsBox, SVerticalBox)
				]
			]
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(PGX::Spacing::MD).HAlign(HAlign_Right)
		[
			SNew(SButton)
				.Text(LOCTEXT("NextA", "Next >"))
				.IsEnabled_Lambda([this]() { return SelectedTemplate != nullptr; })
				.OnClicked_Lambda([this]() -> FReply
				{
					GoToPhase(1);
					return FReply::Handled();
				})
		];

	// EN: Populate template cards after SAssignNew has resolved
	// ES: Poblar cards de templates despues de que SAssignNew se resuelva
	RefreshTemplateCards();

	return PhaseAWidget;
}

TSharedRef<SWidget> SPGXScaffoldTab::BuildCategoryList()
{
	TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
	TArray<FName> Categories = FPGXScaffoldTemplateRegistry::Get().GetCategories();

	for (const FName& Cat : Categories)
	{
		Box->AddSlot().AutoHeight().Padding(2.0f)
		[
			SNew(SButton)
				.Text(FText::FromName(Cat))
				.OnClicked_Lambda([this, Cat]() -> FReply
				{
					OnCategorySelected(Cat);
					return FReply::Handled();
				})
		];
	}

	return Box;
}

TSharedRef<SWidget> SPGXScaffoldTab::BuildTemplateCards()
{
	// EN: Placeholder — populated dynamically
	// ES: Placeholder — poblado dinamicamente
	return SAssignNew(TemplateCardsBox, SVerticalBox);
}

void SPGXScaffoldTab::OnCategorySelected(FName Category)
{
	SelectedCategory = Category;
	RefreshTemplateCards();
}

void SPGXScaffoldTab::OnTemplateSelected(const FPGXScaffoldTemplate* Template)
{
	SelectedTemplate = Template;

	// EN: Initialize variable values from defaults
	// ES: Inicializar valores de variables desde defaults
	VariableValues.Empty();
	if (SelectedTemplate)
	{
		for (const auto& Pair : SelectedTemplate->DefaultVariables)
		{
			VariableValues.Add(Pair.Key, Pair.Value);
		}

		// EN: Build tree from template
		// ES: Construir arbol desde template
		TreeRootNodes = Resolver->BuildTree(*SelectedTemplate);
	}

	bValidationPassed = false;
	ValidationResults.Empty();
	CurrentPlan.Reset();
}

void SPGXScaffoldTab::RefreshTemplateCards()
{
	if (!TemplateCardsBox.IsValid()) { return; }
	TemplateCardsBox->ClearChildren();

	TArray<const FPGXScaffoldTemplate*> Templates = FPGXScaffoldTemplateRegistry::Get().GetTemplatesByCategory(SelectedCategory);

	for (const FPGXScaffoldTemplate* T : Templates)
	{
		const bool bIsSelected = (SelectedTemplate == T);

		TemplateCardsBox->AddSlot().AutoHeight().Padding(PGX::Spacing::SM)
		[
			SNew(SBorder)
				// Use the shared premium card brush for template entries.
				.BorderImage(FPGXEditorStyle::Get().GetBrush("PGXEditor.Premium.Card"))
				.Padding(PGX::Spacing::MD)
				.BorderBackgroundColor(bIsSelected ? FSlateColor(PGX::System::Scaffold) : FSlateColor(PGX::Surface::Raised))
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.0f)
					[
						SNew(STextBlock)
							.Text(T->DisplayName)
							.Font(PGX::Font::SectionHeader())
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(STextBlock)
							.Text(FText::Format(LOCTEXT("ItemCount", "{0} items"), FText::AsNumber(T->GetItemCount())))
							.Font(PGX::Font::BodySmall())
							.ColorAndOpacity(PGX::Text::Muted)
					]
				]

				+ SVerticalBox::Slot().AutoHeight().Padding(0, PGX::Spacing::SM, 0, 0)
				[
					SNew(STextBlock)
						.Text(T->Description)
						.Font(PGX::Font::Body())
						.AutoWrapText(true)
				]

				+ SVerticalBox::Slot().AutoHeight().Padding(0, PGX::Spacing::MD, 0, 0).HAlign(HAlign_Right)
				[
					SNew(SButton)
						.Text(bIsSelected ? LOCTEXT("Selected", "Selected") : LOCTEXT("Select", "Select"))
						.IsEnabled(!bIsSelected)
						.OnClicked_Lambda([this, T]() -> FReply
						{
							OnTemplateSelected(T);
							RefreshTemplateCards();
							return FReply::Handled();
						})
				]
			]
		];
	}
}

// ============================================================================
// Phase B: Configuration + Validation
// ============================================================================

TSharedRef<SWidget> SPGXScaffoldTab::BuildPhaseB()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight().Padding(PGX::Spacing::MD)
		[
			SNew(SPGXSectionDivider)
				.Title(LOCTEXT("PhaseBTitle", "Phase B: Configure & Validate"))
		]

		// EN: Variables / ES: Variables
		+ SVerticalBox::Slot().AutoHeight().Padding(PGX::Spacing::MD)
		[
			SAssignNew(VariableFieldsBox, SVerticalBox)
		]

		// EN: Hierarchy tree / ES: Arbol de jerarquia
		+ SVerticalBox::Slot().FillHeight(1.0f).Padding(PGX::Spacing::MD)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SAssignNew(TreeBox, SVerticalBox)
			]
		]

		// EN: Validation results / ES: Resultados de validacion
		+ SVerticalBox::Slot().AutoHeight().MaxHeight(150.0f).Padding(PGX::Spacing::MD)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SAssignNew(ValidationBox, SVerticalBox)
			]
		]

		// EN: Navigation buttons / ES: Botones de navegacion
		+ SVerticalBox::Slot().AutoHeight().Padding(PGX::Spacing::MD)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SButton)
					.Text(LOCTEXT("BackB", "< Back"))
					.OnClicked_Lambda([this]() -> FReply
					{
						GoToPhase(0);
						return FReply::Handled();
					})
			]

			+ SHorizontalBox::Slot().FillWidth(1.0f)
			[
				SNew(SSpacer)
			]

			+ SHorizontalBox::Slot().AutoWidth().Padding(PGX::Spacing::SM, 0.0f)
			[
				SNew(SButton)
					.Text(LOCTEXT("Validate", "Validate"))
					.OnClicked_Lambda([this]() -> FReply
					{
						OnValidateClicked();
						return FReply::Handled();
					})
			]

			+ SHorizontalBox::Slot().AutoWidth().Padding(PGX::Spacing::SM, 0.0f)
			[
				SAssignNew(NextFromBButton, SButton)
					.Text(LOCTEXT("NextB", "Next >"))
					.IsEnabled_Lambda([this]() { return bValidationPassed; })
					.OnClicked_Lambda([this]() -> FReply
					{
						// EN: Generate plan / ES: Generar plan
						if (SelectedTemplate && Resolver.IsValid() && PlanGenerator.IsValid())
						{
							TArray<FPGXScaffoldTemplateItem> Items = Resolver->GetSelectedItems(TreeRootNodes);
							FPGXScaffoldBuildPlan Plan = PlanGenerator->Generate(Items, VariableValues, ProjectInfo.ContentDir, SelectedTemplate->TemplateId);
							CurrentPlan = MakeShared<FPGXScaffoldBuildPlan>(MoveTemp(Plan));
							RefreshPlanReview();
						}
						GoToPhase(2);
						return FReply::Handled();
					})
			]
		];
}

TSharedRef<SWidget> SPGXScaffoldTab::BuildVariableFields()
{
	return SAssignNew(VariableFieldsBox, SVerticalBox);
}

TSharedRef<SWidget> SPGXScaffoldTab::BuildHierarchyTree()
{
	return SAssignNew(TreeBox, SVerticalBox);
}

TSharedRef<SWidget> SPGXScaffoldTab::BuildValidationResults()
{
	return SAssignNew(ValidationBox, SVerticalBox);
}

void SPGXScaffoldTab::OnValidateClicked()
{
	if (!SelectedTemplate || !Resolver.IsValid() || !Validator.IsValid())
	{
		return;
	}

	// EN: Refresh project info
	// ES: Refrescar info del proyecto
	Analyzer->InvalidateCache();
	ProjectInfo = Analyzer->Analyze();

	TArray<FPGXScaffoldTemplateItem> Items = Resolver->GetSelectedItems(TreeRootNodes);
	ValidationResults = Validator->Validate(Items, VariableValues, ProjectInfo);
	bValidationPassed = !Validator->HasErrors();

	RefreshValidation();
}

void SPGXScaffoldTab::RefreshVariableFields()
{
	if (!VariableFieldsBox.IsValid() || !SelectedTemplate) { return; }
	VariableFieldsBox->ClearChildren();

	for (const auto& Pair : SelectedTemplate->DefaultVariables)
	{
		const FString VarName = Pair.Key;
		const FText* DisplayName = SelectedTemplate->VariableDisplayNames.Find(VarName);
		FText Label = DisplayName ? *DisplayName : FText::FromString(VarName);

		VariableFieldsBox->AddSlot().AutoHeight().Padding(PGX::Spacing::SM)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(SBox).WidthOverride(120.0f)
				[
					SNew(STextBlock)
						.Text(Label)
						.Font(PGX::Font::Body())
				]
			]

			+ SHorizontalBox::Slot().FillWidth(1.0f)
			[
				SNew(SEditableTextBox)
					.Text(FText::FromString(VariableValues.FindOrAdd(VarName, Pair.Value)))
					.OnTextCommitted_Lambda([this, VarName](const FText& NewText, ETextCommit::Type)
					{
						VariableValues.FindOrAdd(VarName) = NewText.ToString();
						bValidationPassed = false;
					})
			]
		];
	}
}

void SPGXScaffoldTab::RefreshTree()
{
	if (!TreeBox.IsValid()) { return; }
	TreeBox->ClearChildren();

	if (TreeRootNodes.Num() == 0) { return; }

	TreeBox->AddSlot().AutoHeight()
	[
		SNew(SPGXScaffoldHierarchyTree)
			.RootNodes(TreeRootNodes)
			.OnNodeToggled_Lambda([this]()
			{
				bValidationPassed = false;
			})
	];
}

void SPGXScaffoldTab::RefreshValidation()
{
	if (!ValidationBox.IsValid()) { return; }
	ValidationBox->ClearChildren();

	for (const auto& R : ValidationResults)
	{
		FLinearColor Color;
		FString Prefix;
		switch (R.Severity)
		{
		case EPGXScaffoldSeverity::Info:    Color = PGX::Semantic::Info;  Prefix = TEXT("INFO"); break;
		case EPGXScaffoldSeverity::Warning: Color = PGX::Semantic::Warn;  Prefix = TEXT("WARN"); break;
		case EPGXScaffoldSeverity::Error:   Color = PGX::Semantic::Error; Prefix = TEXT("ERROR"); break;
		case EPGXScaffoldSeverity::Fatal:   Color = PGX::Semantic::Error; Prefix = TEXT("FATAL"); break;
		}

		ValidationBox->AddSlot().AutoHeight().Padding(2.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, PGX::Spacing::SM, 0)
			[
				SNew(STextBlock)
					.Text(FText::FromString(FString::Printf(TEXT("[%s]"), *Prefix)))
					.Font(PGX::Font::BodySmall())
					.ColorAndOpacity(Color)
			]

			+ SHorizontalBox::Slot().FillWidth(1.0f)
			[
				SNew(STextBlock)
					.Text(R.Message)
					.Font(PGX::Font::BodySmall())
					.AutoWrapText(true)
			]
		];
	}
}

// ============================================================================
// Phase C: Review + Execute
// ============================================================================

TSharedRef<SWidget> SPGXScaffoldTab::BuildPhaseC()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight().Padding(PGX::Spacing::MD)
		[
			SNew(SPGXSectionDivider)
				.Title(LOCTEXT("PhaseCTitle", "Phase C: Review & Execute"))
		]

		// EN: KPI bar / ES: Barra KPI
		+ SVerticalBox::Slot().AutoHeight().Padding(PGX::Spacing::MD)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot().AutoWidth().Padding(PGX::Spacing::SM)
			[
				SNew(SPGXKPIChip)
					.Label(LOCTEXT("KPIFolders", "Folders"))
					.Value_Lambda([this]() -> FText
					{
						return FText::AsNumber(CurrentPlan.IsValid() ? CurrentPlan->CountByType(EPGXScaffoldActionType::CreateFolder) : 0);
					})
					.AccentColor(PGX::System::Scaffold)
			]

			+ SHorizontalBox::Slot().AutoWidth().Padding(PGX::Spacing::SM)
			[
				SNew(SPGXKPIChip)
					.Label(LOCTEXT("KPIAssets", "Assets"))
					.Value_Lambda([this]() -> FText
					{
						int32 N = 0;
						if (CurrentPlan.IsValid())
						{
							N = CurrentPlan->CountByType(EPGXScaffoldActionType::CreateDataAsset) +
								CurrentPlan->CountByType(EPGXScaffoldActionType::CreateBlueprint);
						}
						return FText::AsNumber(N);
					})
					.AccentColor(PGX::Semantic::Info)
			]

			+ SHorizontalBox::Slot().AutoWidth().Padding(PGX::Spacing::SM)
			[
				SNew(SPGXKPIChip)
					.Label(LOCTEXT("KPITotal", "Total Steps"))
					.Value_Lambda([this]() -> FText
					{
						return FText::AsNumber(CurrentPlan.IsValid() ? CurrentPlan->Steps.Num() : 0);
					})
					.AccentColor(PGX::Semantic::Good)
			]
		]

		// EN: Plan steps (SListView with color-coded badges) / ES: Pasos del plan (SListView con badges de color)
		+ SVerticalBox::Slot().FillHeight(1.0f).Padding(PGX::Spacing::MD)
		[
			SAssignNew(PlanStepsListView, SListView<TSharedPtr<FPGXScaffoldPlanStepRow>>)
				.ListItemsSource(&PlanStepRows)
				.OnGenerateRow(this, &SPGXScaffoldTab::OnGeneratePlanRow)
				.HeaderRow(
					SNew(SHeaderRow)
					+ SHeaderRow::Column("Step").DefaultLabel(LOCTEXT("Col_Step", "#")).FixedWidth(40)
					+ SHeaderRow::Column("Type").DefaultLabel(LOCTEXT("Col_Type", "Type")).FixedWidth(80)
					+ SHeaderRow::Column("Path").DefaultLabel(LOCTEXT("Col_Path", "Path")).FillWidth(1.0f)
					+ SHeaderRow::Column("Status").DefaultLabel(LOCTEXT("Col_Status", "Status")).FixedWidth(80)
				)
		]

		// EN: Navigation buttons / ES: Botones de navegacion
		+ SVerticalBox::Slot().AutoHeight().Padding(PGX::Spacing::MD)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SButton)
					.Text(LOCTEXT("BackC", "< Back"))
					.OnClicked_Lambda([this]() -> FReply
					{
						GoToPhase(1);
						return FReply::Handled();
					})
			]

			+ SHorizontalBox::Slot().FillWidth(1.0f)
			[
				SNew(SSpacer)
			]

			+ SHorizontalBox::Slot().AutoWidth().Padding(PGX::Spacing::SM, 0.0f)
			[
				SNew(SButton)
					.Text(LOCTEXT("ExportJSON", "Export JSON"))
					.IsEnabled_Lambda([this]() { return CurrentPlan.IsValid(); })
					.OnClicked_Lambda([this]() -> FReply
					{
						OnExportJsonClicked();
						return FReply::Handled();
					})
			]

			+ SHorizontalBox::Slot().AutoWidth().Padding(PGX::Spacing::SM, 0.0f)
			[
				SNew(SButton)
					.Text(LOCTEXT("Execute", "Execute"))
					.IsEnabled_Lambda([this]() { return CurrentPlan.IsValid(); })
					.OnClicked_Lambda([this]() -> FReply
					{
						OnExecuteClicked();
						return FReply::Handled();
					})
			]
		];
}

TSharedRef<SWidget> SPGXScaffoldTab::BuildPlanReview()
{
	// EN: Now handled by SListView in BuildPhaseC
	// ES: Ahora manejado por SListView en BuildPhaseC
	return SNullWidget::NullWidget;
}

TSharedRef<SWidget> SPGXScaffoldTab::BuildKPIBar()
{
	return SNullWidget::NullWidget;
}

void SPGXScaffoldTab::RefreshPlanReview()
{
	PlanStepRows.Empty();

	if (!CurrentPlan.IsValid()) { return; }

	for (const auto& Step : CurrentPlan->Steps)
	{
		TSharedPtr<FPGXScaffoldPlanStepRow> Row = MakeShared<FPGXScaffoldPlanStepRow>();
		Row->Step = Step;

		// EN: Check if asset/folder already exists (will be skipped on execute)
		// ES: Verificar si el asset/carpeta ya existe (se saltara al ejecutar)
		if (Step.ActionType == EPGXScaffoldActionType::CreateFolder)
		{
			Row->bWillSkip = IFileManager::Get().DirectoryExists(*Step.AbsolutePath);
		}
		else
		{
			// EN: Check asset registry for DA/BP
			// ES: Verificar asset registry para DA/BP
			FString RelPath = Step.AbsolutePath;
			FString ContentDir = FPaths::ProjectContentDir();
			FPaths::MakePathRelativeTo(RelPath, *ContentDir);
			FString PackagePath = TEXT("/Game/") + FPaths::GetPath(RelPath);
			PackagePath.ReplaceInline(TEXT("\\"), TEXT("/"));
			FString AssetName = FPaths::GetBaseFilename(Step.AbsolutePath);
			FString FullAssetPath = PackagePath / AssetName + TEXT(".") + AssetName;
			FAssetRegistryModule& ARModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
			Row->bWillSkip = ARModule.Get().GetAssetByObjectPath(FSoftObjectPath(FullAssetPath)).IsValid();
		}

		PlanStepRows.Add(Row);
	}

	if (PlanStepsListView.IsValid())
	{
		PlanStepsListView->RequestListRefresh();
	}
}

TSharedRef<ITableRow> SPGXScaffoldTab::OnGeneratePlanRow(
	TSharedPtr<FPGXScaffoldPlanStepRow> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	// EN: Color by action type / ES: Color por tipo de accion
	FLinearColor TypeColor = PGX::Semantic::Neutral;
	FString TypeStr = TEXT("?");
	switch (Item->Step.ActionType)
	{
	case EPGXScaffoldActionType::CreateFolder:    TypeColor = PGX::System::Scaffold; TypeStr = TEXT("FOLDER"); break;
	case EPGXScaffoldActionType::CreateDataAsset: TypeColor = PGX::Semantic::Info;   TypeStr = TEXT("DA"); break;
	case EPGXScaffoldActionType::CreateBlueprint: TypeColor = PGX::Semantic::Good;   TypeStr = TEXT("BP"); break;
	default: break;
	}

	// EN: Status from validation / ES: Estado desde validacion
	FString StatusStr;
	FLinearColor StatusColor;
	if (Item->bWillSkip)
	{
		StatusStr = TEXT("SKIP");
		StatusColor = PGX::Semantic::Warn;
	}
	else
	{
		StatusStr = TEXT("NEW");
		StatusColor = PGX::Semantic::Good;
	}

	return SNew(STableRow<TSharedPtr<FPGXScaffoldPlanStepRow>>, OwnerTable)
	[
		SNew(SHorizontalBox)

		// EN: Step number / ES: Numero de paso
		+ SHorizontalBox::Slot().AutoWidth()
		[
			SNew(SBox).WidthOverride(40.0f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
					.Text(FText::AsNumber(Item->Step.StepIndex + 1))
					.Font(PGX::Font::Mono())
					.ColorAndOpacity(PGX::Text::Muted)
					.Justification(ETextJustify::Right)
			]
		]

		// EN: Type badge / ES: Badge de tipo
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(PGX::Spacing::SM, 0)
		[
			SNew(SPGXStatusBadge)
				.Shape(EPGXBadgeShape::Pill)
				.Label(FText::FromString(TypeStr))
				.Color(TypeColor)
		]

		// EN: Path / ES: Ruta
		+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center).Padding(PGX::Spacing::SM, 0)
		[
			SNew(STextBlock)
				.Text(FText::FromString(Item->Step.AbsolutePath))
				.Font(PGX::Font::Mono())
				.ColorAndOpacity(PGX::Text::Primary)
		]

		// EN: Status badge / ES: Badge de estado
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			SNew(SPGXStatusBadge)
				.Shape(EPGXBadgeShape::Pill)
				.Label(FText::FromString(StatusStr))
				.Color(StatusColor)
		]
	];
}

void SPGXScaffoldTab::OnExportJsonClicked()
{
	if (!CurrentPlan.IsValid()) { return; }

	FString JsonStr = CurrentPlan->ToJsonString();
	FString DefaultPath = FPGXScaffoldLogger::GetLogDirectory();
	FString FileName = FString::Printf(TEXT("scaffold_plan_%s.json"), *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
	FString FilePath = DefaultPath / FileName;

	IFileManager::Get().MakeDirectory(*DefaultPath, true);
	if (FFileHelper::SaveStringToFile(JsonStr, *FilePath))
	{
		UPGXEditorNotification::ShowInfo(
			FText::Format(LOCTEXT("ExportOK", "Plan exported to {0}"), FText::FromString(FilePath)));
	}
	else
	{
		UPGXEditorNotification::ShowError(LOCTEXT("ExportFail", "Failed to export plan"));
	}
}

void SPGXScaffoldTab::OnExecuteClicked()
{
	if (!CurrentPlan.IsValid() || !Executor.IsValid() || !Logger.IsValid()) { return; }

	// EN: Check the Live Coding state for future C++ generation actions
	// ES: Verificar el estado de Live Coding para futuras acciones de generacion C++
	if (!CheckLiveCodingWarning()) { return; }

	// EN: Start logger session / ES: Iniciar sesion del logger
	Logger->BeginSession(*CurrentPlan);

	// EN: Execute synchronously (folder/asset creation is fast)
	// ES: Ejecutar sincronicamente (creacion de carpetas/assets es rapida)
	FPGXScaffoldExecutionResult Result = Executor->Execute(*CurrentPlan);

	// EN: Log steps / ES: Loggear pasos
	for (const auto& Entry : Result.LogEntries)
	{
		Logger->LogStep(Entry);
	}

	// EN: End logger session / ES: Terminar sesion del logger
	FString AuditLogPath = Logger->EndSession(Result);

	// EN: Show result notification / ES: Mostrar notificacion de resultado
	if (Result.Status == EPGXScaffoldOperationStatus::Completed)
	{
		UPGXEditorNotification::ShowInfo(
			FText::Format(LOCTEXT("ExecOK", "Scaffold completed: {0} created, {1} skipped ({2}ms)"),
				FText::AsNumber(Result.CompletedSteps),
				FText::AsNumber(Result.SkippedSteps),
				FText::FromString(FString::Printf(TEXT("%.0f"), Result.TotalDurationMs))));
	}
	else
	{
		UPGXEditorNotification::ShowError(
			FText::Format(LOCTEXT("ExecFail", "Scaffold failed at step {0}. {1} completed, {2} failed. Log: {3}"),
				FText::AsNumber(Result.CompletedSteps + Result.FailedSteps),
				FText::AsNumber(Result.CompletedSteps),
				FText::AsNumber(Result.FailedSteps),
				FText::FromString(AuditLogPath)));
	}
}

// ============================================================================
// Live Coding warning for future C++ generation
// ============================================================================

bool SPGXScaffoldTab::CheckLiveCodingWarning() const
{
	if (!CurrentPlan.IsValid()) { return true; }

	// EN: Check whether the plan contains a C++ generation action
	// ES: Verificar si el plan contiene acciones de generacion C++
	bool bHasCppActions = false;
	for (const auto& Step : CurrentPlan->Steps)
	{
		// EN: No C++ generation action types are currently available
		// ES: Actualmente no existen tipos de accion de generacion C++
		// When CreateCppHeader/CreateCppSource are added to EPGXScaffoldActionType,
		// uncomment the check below:
		// if (Step.ActionType == EPGXScaffoldActionType::CreateCppHeader ||
		//     Step.ActionType == EPGXScaffoldActionType::CreateCppSource)
		// {
		//     bHasCppActions = true;
		//     break;
		// }
	}

	if (!bHasCppActions) { return true; }

	// EN: When C++ generation actions are available, check Live Coding here:
	//   #include "ILiveCodingModule.h" (with WITH_LIVE_CODING guard)
	//   ILiveCodingModule* LC = FModuleManager::GetModulePtr<ILiveCodingModule>(LIVE_CODING_MODULE_NAME);
	//   if (LC && LC->IsEnabledForSession()) { warn user }
	// ES: Cuando existan acciones de generacion C++, comprobar Live Coding aqui

	return true;
}

// ============================================================================
// Navigation
// ============================================================================

void SPGXScaffoldTab::GoToPhase(int32 Phase)
{
	CurrentPhase = FMath::Clamp(Phase, 0, 2);

	if (CurrentPhase == 1)
	{
		RefreshVariableFields();
		RefreshTree();
		RefreshValidation();
	}
	else if (CurrentPhase == 2)
	{
		RefreshPlanReview();
	}

	UpdateFooter();
}

void SPGXScaffoldTab::UpdateFooter()
{
	// EN: Footer updates via lambda binding in Construct
	// ES: Footer se actualiza via binding lambda en Construct
}

#undef LOCTEXT_NAMESPACE
