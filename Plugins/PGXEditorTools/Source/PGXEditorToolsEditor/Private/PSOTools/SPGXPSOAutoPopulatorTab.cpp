// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PSOTools/SPGXPSOAutoPopulatorTab.h"
#include "PGXPSOWarmUpConfig.h"
#include "Logging/PGXLogMacros.h"

// EN: Slate / UI
// ES: Slate / UI
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "Styling/CoreStyle.h"

// EN: Asset picker / ES: Selector de assets
#include "PropertyCustomizationHelpers.h"

// EN: Content Browser selection / ES: Seleccion del Content Browser
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

// EN: Asset types for material extraction / ES: Tipos de asset para extraccion de materiales
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"

// EN: Undo/Redo support / ES: Soporte Undo/Redo
#include "ScopedTransaction.h"

// EN: PGX Editor infrastructure / ES: Infraestructura de editor PGX
#include "Utils/PGXEditorUtils.h"
#include "Style/PGXVisualTokens.h"
#include "Style/PGXEditorStyle.h"
#include "Widgets/SPGXPremiumShell.h"
#include "Widgets/SPGXSectionDivider.h"
#include "Widgets/SPGXFooterBar.h"
#include "PGXEditorToolsEditor.h"

// EN: GameplayTags for context tag input / ES: GameplayTags para input de tag de contexto
#include "Widgets/Input/SEditableTextBox.h"
#include "GameplayTagContainer.h"

#define LOCTEXT_NAMESPACE "PGXPSOAutoPopulator"

// EN: PSO accent color (Cyan) / ES: Color de acento PSO (Cyan)
namespace { static const FLinearColor GPSOPopulatorColor = PGX::System::PSO; }

// ============================================================================
// EN: Construct / ES: Construir
// ============================================================================

void SPGXPSOAutoPopulatorTab::Construct(const FArguments& /*InArgs*/)
{
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[PSOTool] Construct"));

	CachedFooterText = LOCTEXT("StatusReady", "Ready — Select a target DA and import assets");
	CachedFooterColor = PGX::Text::Muted;

	ChildSlot
	[
		SNew(SPGXPremiumShell)
		.SystemColor(GPSOPopulatorColor)
		.Title(LOCTEXT("PanelTitle", "PSO AUTO-POPULATOR"))
		.Subtitle(LOCTEXT("PanelSubtitle", "Material extraction tool"))
		.Icon(FPGXEditorStyle::Get().GetBrush("PGXEditor.Icon.PSO"))
		.bShowFooter(true)
		.StatusText(this, &SPGXPSOAutoPopulatorTab::GetFooterStatusText)
		.StatusColor(this, &SPGXPSOAutoPopulatorTab::GetFooterStatusColor)
		.TitleRightContent()
		[
			SNew(SButton)
			.Text(LOCTEXT("Clear", "Clear"))
			.ToolTipText(LOCTEXT("ClearTooltip", "Clear all preview entries"))
			.OnClicked(this, &SPGXPSOAutoPopulatorTab::OnClearPreview)
		]
		.Content()
		[
			SNew(SScrollBox)

			// EN: Target DA picker / ES: Selector de DA objetivo
			+ SScrollBox::Slot()
			.Padding(8.0f, 4.0f)
			[
				BuildTargetDASection()
			]

			// EN: Defaults section / ES: Seccion de defaults
			+ SScrollBox::Slot()
			.Padding(8.0f, 4.0f)
			[
				BuildDefaultsSection()
			]

			// EN: Preview list / ES: Lista de preview
			+ SScrollBox::Slot()
			.Padding(8.0f, 4.0f)
			[
				BuildPreviewSection()
			]

			// EN: Action buttons / ES: Botones de accion
			+ SScrollBox::Slot()
			.Padding(8.0f, 4.0f)
			[
				BuildActionsSection()
			]
		]
	];
}

// ============================================================================
// EN: Footer Attribute Getters / ES: Getters de Atributos del Footer
// ============================================================================

FText SPGXPSOAutoPopulatorTab::GetFooterStatusText() const
{
	return CachedFooterText;
}

FLinearColor SPGXPSOAutoPopulatorTab::GetFooterStatusColor() const
{
	return CachedFooterColor;
}

// ============================================================================
// EN: UI Build / ES: Construccion de UI
// ============================================================================

TSharedRef<SWidget> SPGXPSOAutoPopulatorTab::BuildTargetDASection()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 2.0f)
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("SectionTarget", "TARGET WARMUPCONFIG"))
			.AccentColor(GPSOPopulatorColor)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 2.0f)
		[
			SNew(SObjectPropertyEntryBox)
			.AllowedClass(UPGXPSOWarmUpConfig::StaticClass())
			.ObjectPath(this, &SPGXPSOAutoPopulatorTab::OnGetTargetDAPath)
			.OnObjectChanged(this, &SPGXPSOAutoPopulatorTab::OnTargetDAChanged)
			.AllowClear(true)
			.DisplayThumbnail(false)
		];
}

TSharedRef<SWidget> SPGXPSOAutoPopulatorTab::BuildDefaultsSection()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 2.0f)
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("SectionDefaults", "ENTRY DEFAULTS"))
			.AccentColor(GPSOPopulatorColor)
		]

		// EN: Vertex Factory / ES: Vertex Factory
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 2.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("VFLabel", "Vertex Factory:"))
				.MinDesiredWidth(120.0f)
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("VFAutoDetect", "Auto-detect from asset type (StaticMesh / SkeletalMesh)"))
				.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
			]
		]

		// EN: Render checkboxes / ES: Checkboxes de render
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 4.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 12.0f, 0.0f)
			[
				SNew(SCheckBox)
				.IsChecked_Lambda([this]() { return bDefaultRenderInMainPass ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
				.OnCheckStateChanged_Lambda([this](ECheckBoxState State) { bDefaultRenderInMainPass = (State == ECheckBoxState::Checked); })
				[
					SNew(STextBlock).Text(LOCTEXT("MainPass", "Main Pass"))
				]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 12.0f, 0.0f)
			[
				SNew(SCheckBox)
				.IsChecked_Lambda([this]() { return bDefaultRenderInDepthPass ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
				.OnCheckStateChanged_Lambda([this](ECheckBoxState State) { bDefaultRenderInDepthPass = (State == ECheckBoxState::Checked); })
				[
					SNew(STextBlock).Text(LOCTEXT("DepthPass", "Depth Pass"))
				]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SCheckBox)
				.IsChecked_Lambda([this]() { return bDefaultCastShadow ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
				.OnCheckStateChanged_Lambda([this](ECheckBoxState State) { bDefaultCastShadow = (State == ECheckBoxState::Checked); })
				[
					SNew(STextBlock).Text(LOCTEXT("CastShadow", "Cast Shadow"))
				]
			]
		]

		// EN: Mobility note / ES: Nota de movilidad
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 2.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("MobilityLabel", "Mobility:"))
				.MinDesiredWidth(120.0f)
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("MobilityDefault", "Static (default)"))
				.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
			]
		]

		// EN: Context Tag input / ES: Input de Context Tag
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 4.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ContextTagLabel", "Context Tag:"))
				.MinDesiredWidth(120.0f)
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SEditableTextBox)
				.HintText(LOCTEXT("ContextTagHint", "e.g. PGX.PSO.Context.Default"))
				.Text_Lambda([this]()
				{
					return DefaultContextTag.IsValid()
						? FText::FromString(DefaultContextTag.ToString())
						: FText::GetEmpty();
				})
				.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type /*CommitType*/)
				{
					if (Text.IsEmpty())
					{
						DefaultContextTag = FGameplayTag();
						PGX_LOG_INFO(LogPGXEditorTools, TEXT("[PSOTool] Context tag cleared"));
					}
					else
					{
						const FGameplayTag Resolved = FGameplayTag::RequestGameplayTag(FName(*Text.ToString()), false);
						if (Resolved.IsValid())
						{
							DefaultContextTag = Resolved;
							PGX_LOG_INFO(LogPGXEditorTools, TEXT("[PSOTool] Context tag set: %s"), *Resolved.ToString());
						}
						else
						{
							PGX_LOG_WARNING(LogPGXEditorTools, TEXT("[PSOTool] Invalid tag: '%s' — not registered in GameplayTagsManager"), *Text.ToString());
							CachedFooterText = FText::Format(
								LOCTEXT("StatusInvalidTag", "Warning: Tag '{0}' not found. Check tag registry."),
								Text);
							CachedFooterColor = FLinearColor(1.0f, 0.6f, 0.2f);
						}
					}
				})
			]
		];
}

TSharedRef<SWidget> SPGXPSOAutoPopulatorTab::BuildPreviewSection()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 2.0f)
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("SectionPreview", "PREVIEW ENTRIES"))
			.AccentColor(GPSOPopulatorColor)
			.RightContent()
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					// EN: Count duplicates against target DA / ES: Contar duplicados contra DA objetivo
					int32 DupCount = 0;
					if (TargetConfig.IsValid())
					{
						for (const TSharedPtr<FPGXPSOEntry>& Entry : PreviewEntries)
						{
							if (!Entry.IsValid()) continue;
							const FPGXPSOKey Key = Entry->MakeKey();
							for (const FPGXPSOEntry& Existing : TargetConfig->Entries)
							{
								if (Existing.MakeKey() == Key)
								{
									DupCount++;
									break;
								}
							}
						}
					}

					if (DupCount > 0)
					{
						return FText::Format(LOCTEXT("EntryCountDup", "{0} entries ({1} duplicates)"),
							FText::AsNumber(PreviewEntries.Num()),
							FText::AsNumber(DupCount));
					}
					return FText::Format(LOCTEXT("EntryCountFmt", "{0} entries"),
						FText::AsNumber(PreviewEntries.Num()));
				})
				.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.MaxHeight(300.0f)
		.Padding(0.0f, 4.0f)
		[
			SAssignNew(PreviewListView, SListView<TSharedPtr<FPGXPSOEntry>>)
			.ListItemsSource(&PreviewEntries)
			.OnGenerateRow(this, &SPGXPSOAutoPopulatorTab::OnGeneratePreviewRow)
			.SelectionMode(ESelectionMode::None)
			.HeaderRow(
				SNew(SHeaderRow)
				+ SHeaderRow::Column("Material")
				.DefaultLabel(LOCTEXT("ColMaterial", "Material"))
				.FillWidth(0.4f)
				+ SHeaderRow::Column("VFType")
				.DefaultLabel(LOCTEXT("ColVF", "VF Type"))
				.FillWidth(0.2f)
				+ SHeaderRow::Column("Context")
				.DefaultLabel(LOCTEXT("ColContext", "Context"))
				.FillWidth(0.2f)
				+ SHeaderRow::Column("Label")
				.DefaultLabel(LOCTEXT("ColLabel", "Label"))
				.FillWidth(0.2f)
			)
		];
}

TSharedRef<SWidget> SPGXPSOAutoPopulatorTab::BuildActionsSection()
{
	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.0f, 0.0f, 8.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("ImportBtn", "Import from Selection"))
			.ToolTipText(LOCTEXT("ImportBtnTooltip", "Extract materials from selected Content Browser assets (meshes, materials)"))
			.OnClicked(this, &SPGXPSOAutoPopulatorTab::OnImportFromSelection)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("PopulateBtn", "Populate DA"))
			.ToolTipText(LOCTEXT("PopulateBtnTooltip", "Write preview entries to the target WarmUpConfig DA (supports Ctrl+Z undo)"))
			.IsEnabled_Lambda([this]() { return TargetConfig.IsValid() && PreviewEntries.Num() > 0; })
			.OnClicked(this, &SPGXPSOAutoPopulatorTab::OnPopulateDA)
		];
}

// ============================================================================
// EN: SListView Row / ES: Fila SListView
// ============================================================================

TSharedRef<ITableRow> SPGXPSOAutoPopulatorTab::OnGeneratePreviewRow(
	TSharedPtr<FPGXPSOEntry> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	const TCHAR* VFNames[] = {
		TEXT("StaticMesh"), TEXT("SkeletalMesh"), TEXT("InstancedMesh"), TEXT("SplineMesh"),
		TEXT("Landscape"), TEXT("NiagaraSprite"), TEXT("NiagaraMesh"), TEXT("Custom")
	};
	const int32 VFIdx = static_cast<int32>(Item->VertexFactory);
	const FString VFDisplay = (VFIdx >= 0 && VFIdx < 8) ? FString(VFNames[VFIdx]) : TEXT("Unknown");

	// EN: Detect duplicate against target DA / ES: Detectar duplicado contra DA objetivo
	bool bIsDuplicate = false;
	if (TargetConfig.IsValid() && Item.IsValid())
	{
		const FPGXPSOKey NewKey = Item->MakeKey();
		for (const FPGXPSOEntry& Existing : TargetConfig->Entries)
		{
			if (Existing.MakeKey() == NewKey)
			{
				bIsDuplicate = true;
				break;
			}
		}
	}

	return SNew(STableRow<TSharedPtr<FPGXPSOEntry>>, OwnerTable)
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.FillWidth(0.4f)
		.Padding(4.0f, 2.0f)
		[
			SNew(SHorizontalBox)

			// EN: Duplicate badge / ES: Badge de duplicado
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				bIsDuplicate
				? StaticCastSharedRef<SWidget>(
					SNew(SBorder)
					.BorderBackgroundColor(FLinearColor(1.0f, 0.8f, 0.0f, 0.3f))
					.Padding(FMargin(2.0f, 0.0f))
					[
						SNew(STextBlock)
						.Text(LOCTEXT("DupBadge", "DUP"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 7))
						.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.8f, 0.0f)))
					])
				: StaticCastSharedRef<SWidget>(SNullWidget::NullWidget)
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->Material.GetAssetName()))
				.ToolTipText(FText::FromString(Item->Material.ToString()))
			]
		]

		+ SHorizontalBox::Slot()
		.FillWidth(0.2f)
		.Padding(4.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(VFDisplay))
		]

		+ SHorizontalBox::Slot()
		.FillWidth(0.2f)
		.Padding(4.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(Item->ContextTag.IsValid() ? FText::FromString(Item->ContextTag.ToString()) : LOCTEXT("NoContext", "(none)"))
		]

		+ SHorizontalBox::Slot()
		.FillWidth(0.2f)
		.Padding(4.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Item->Label))
		]
	];
}

// ============================================================================
// EN: Target DA / ES: DA Objetivo
// ============================================================================

FString SPGXPSOAutoPopulatorTab::OnGetTargetDAPath() const
{
	if (TargetConfig.IsValid())
	{
		return TargetConfig->GetPathName();
	}
	return FString();
}

void SPGXPSOAutoPopulatorTab::OnTargetDAChanged(const FAssetData& AssetData)
{
	if (AssetData.IsValid())
	{
		TargetConfig = Cast<UPGXPSOWarmUpConfig>(AssetData.GetAsset());
		PGX_LOG_INFO(LogPGXEditorTools, TEXT("[PSOTool] Target DA set: %s (%d existing entries)"),
			TargetConfig.IsValid() ? *TargetConfig->GetName() : TEXT("null"),
			TargetConfig.IsValid() ? TargetConfig->Entries.Num() : 0);
	}
	else
	{
		TargetConfig = nullptr;
		PGX_LOG_INFO(LogPGXEditorTools, TEXT("[PSOTool] Target DA cleared"));
	}

	if (TargetConfig.IsValid())
	{
		CachedFooterText = FText::Format(
			LOCTEXT("StatusTargetSet", "Target: {0} ({1} existing entries)"),
			FText::FromString(TargetConfig->GetName()),
			FText::AsNumber(TargetConfig->Entries.Num()));
	}
	else
	{
		CachedFooterText = LOCTEXT("StatusNoTarget", "No target DA selected");
	}
}

// ============================================================================
// EN: Actions / ES: Acciones
// ============================================================================

FReply SPGXPSOAutoPopulatorTab::OnImportFromSelection()
{
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[PSOTool] Import from Selection clicked"));

	// EN: Get selected assets from Content Browser
	// ES: Obtener assets seleccionados del Content Browser
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FAssetData> SelectedAssets;
	ContentBrowserModule.Get().GetSelectedAssets(SelectedAssets);

	if (SelectedAssets.IsEmpty())
	{
		PGX_LOG_WARNING(LogPGXEditorTools, TEXT("[PSOTool] No assets selected in Content Browser"));
		CachedFooterText = LOCTEXT("StatusNoSelection", "No assets selected in Content Browser");
		CachedFooterColor = FLinearColor(1.0f, 0.6f, 0.2f);
		return FReply::Handled();
	}

	// EN: Extract materials, deduplicating by soft object path
	// ES: Extraer materiales, deduplicando por ruta de soft object
	TSet<FSoftObjectPath> DeduplicatedPaths;

	// EN: Also deduplicate against existing preview entries
	// ES: Tambien deduplicar contra entradas de preview existentes
	for (const TSharedPtr<FPGXPSOEntry>& Existing : PreviewEntries)
	{
		if (Existing.IsValid() && !Existing->Material.IsNull())
		{
			DeduplicatedPaths.Add(Existing->Material.ToSoftObjectPath());
		}
	}

	int32 NewCount = 0;
	for (const FAssetData& AssetData : SelectedAssets)
	{
		UObject* Asset = AssetData.GetAsset();
		if (!Asset) continue;

		const int32 Before = PreviewEntries.Num();
		ExtractMaterialsFromAsset(Asset, PreviewEntries, DeduplicatedPaths);
		NewCount += PreviewEntries.Num() - Before;
	}

	if (PreviewListView.IsValid())
	{
		PreviewListView->RequestListRefresh();
	}

	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[PSOTool] Imported %d new entries from %d assets (%d total preview)"),
		NewCount, SelectedAssets.Num(), PreviewEntries.Num());

	CachedFooterText = FText::Format(
		LOCTEXT("StatusImported", "Imported {0} new entries from {1} assets ({2} total)"),
		FText::AsNumber(NewCount),
		FText::AsNumber(SelectedAssets.Num()),
		FText::AsNumber(PreviewEntries.Num()));
	CachedFooterColor = FLinearColor(0.3f, 0.8f, 0.3f);

	return FReply::Handled();
}

FReply SPGXPSOAutoPopulatorTab::OnPopulateDA()
{
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[PSOTool] Populate DA clicked"));

	if (!TargetConfig.IsValid())
	{
		PGX_LOG_WARNING(LogPGXEditorTools, TEXT("[PSOTool] No target DA selected"));
		CachedFooterText = LOCTEXT("StatusNoTargetForPopulate", "Cannot populate: No target DA selected");
		CachedFooterColor = FLinearColor(1.0f, 0.3f, 0.3f);
		return FReply::Handled();
	}

	if (PreviewEntries.IsEmpty())
	{
		PGX_LOG_WARNING(LogPGXEditorTools, TEXT("[PSOTool] No preview entries to populate"));
		CachedFooterText = LOCTEXT("StatusNoEntriesToPopulate", "Cannot populate: No preview entries");
		CachedFooterColor = FLinearColor(1.0f, 0.6f, 0.2f);
		return FReply::Handled();
	}

	// EN: Count duplicates before writing / ES: Contar duplicados antes de escribir
	UPGXPSOWarmUpConfig* Config = TargetConfig.Get();
	int32 DupCount = 0;
	for (const TSharedPtr<FPGXPSOEntry>& EntryPtr : PreviewEntries)
	{
		if (EntryPtr.IsValid())
		{
			const FPGXPSOKey Key = EntryPtr->MakeKey();
			for (const FPGXPSOEntry& Existing : Config->Entries)
			{
				if (Existing.MakeKey() == Key)
				{
					DupCount++;
					break;
				}
			}
		}
	}

	// EN: FScopedTransaction for undo support
	// ES: FScopedTransaction para soporte de undo
	FScopedTransaction Transaction(LOCTEXT("PopulatePSOConfig", "PGX: Populate PSO WarmUpConfig"));

	Config->Modify();

	int32 AddedCount = 0;
	for (const TSharedPtr<FPGXPSOEntry>& EntryPtr : PreviewEntries)
	{
		if (EntryPtr.IsValid())
		{
			Config->Entries.Add(*EntryPtr);
			AddedCount++;
		}
	}

	Config->MarkPackageDirty();

	// EN: Clear preview after populating
	// ES: Limpiar preview tras poblar
	PreviewEntries.Empty();
	if (PreviewListView.IsValid())
	{
		PreviewListView->RequestListRefresh();
	}

	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[PSOTool] Populated %d entries to '%s' (total: %d, duplicates: %d)"),
		AddedCount, *Config->GetName(), Config->Entries.Num(), DupCount);

	if (DupCount > 0)
	{
		CachedFooterText = FText::Format(
			LOCTEXT("StatusPopulatedDup", "Added {0} entries ({1} duplicates) to '{2}' (total: {3}). Ctrl+Z to undo."),
			FText::AsNumber(AddedCount),
			FText::AsNumber(DupCount),
			FText::FromString(Config->GetName()),
			FText::AsNumber(Config->Entries.Num()));
		CachedFooterColor = FLinearColor(1.0f, 0.8f, 0.0f);
	}
	else
	{
		CachedFooterText = FText::Format(
			LOCTEXT("StatusPopulated", "Added {0} entries to '{1}' (total: {2}). Ctrl+Z to undo."),
			FText::AsNumber(AddedCount),
			FText::FromString(Config->GetName()),
			FText::AsNumber(Config->Entries.Num()));
		CachedFooterColor = FLinearColor(0.3f, 0.8f, 0.3f);
	}

	return FReply::Handled();
}

FReply SPGXPSOAutoPopulatorTab::OnClearPreview()
{
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[PSOTool] Clear preview (%d entries removed)"), PreviewEntries.Num());
	PreviewEntries.Empty();
	if (PreviewListView.IsValid())
	{
		PreviewListView->RequestListRefresh();
	}
	CachedFooterText = LOCTEXT("StatusCleared", "Preview cleared");
	CachedFooterColor = PGX::Text::Muted;
	return FReply::Handled();
}

// ============================================================================
// EN: Material Extraction / ES: Extraccion de Materiales
// ============================================================================

void SPGXPSOAutoPopulatorTab::ExtractMaterialsFromAsset(
	UObject* Asset,
	TArray<TSharedPtr<FPGXPSOEntry>>& OutEntries,
	TSet<FSoftObjectPath>& DeduplicatedPaths)
{
	auto MakeEntry = [this](UMaterialInterface* Mat, EPGXVertexFactoryType VF, const FString& InLabel) -> TSharedPtr<FPGXPSOEntry>
	{
		TSharedPtr<FPGXPSOEntry> Entry = MakeShared<FPGXPSOEntry>();
		Entry->Material = Mat;
		Entry->VertexFactory = VF;
		Entry->ContextTag = DefaultContextTag;
		Entry->Label = InLabel;
		Entry->bRenderInMainPass = bDefaultRenderInMainPass;
		Entry->bRenderInDepthPass = bDefaultRenderInDepthPass;
		Entry->bCastShadow = bDefaultCastShadow;
		Entry->Mobility = DefaultMobility;
		Entry->bSkinnedMesh = (VF == EPGXVertexFactoryType::SkeletalMesh);
		Entry->EntryType = EPGXPSOEntryType::Material;
		return Entry;
	};

	// EN: Direct material selection / ES: Seleccion directa de material
	if (UMaterialInterface* Mat = Cast<UMaterialInterface>(Asset))
	{
		const FSoftObjectPath Path(Mat);
		if (!DeduplicatedPaths.Contains(Path))
		{
			DeduplicatedPaths.Add(Path);
			OutEntries.Add(MakeEntry(Mat, DefaultVertexFactory, Mat->GetName()));
		}
		return;
	}

	// EN: Static Mesh — extract materials from slots / ES: Static Mesh — extraer materiales de slots
	if (UStaticMesh* SM = Cast<UStaticMesh>(Asset))
	{
		const TArray<FStaticMaterial>& Materials = SM->GetStaticMaterials();
		for (const FStaticMaterial& SlotMat : Materials)
		{
			UMaterialInterface* Mat = SlotMat.MaterialInterface;
			if (!Mat) continue;

			const FSoftObjectPath Path(Mat);
			if (DeduplicatedPaths.Contains(Path)) continue;
			DeduplicatedPaths.Add(Path);

			const FString EntryLabel = FString::Printf(TEXT("%s_%s"),
				*SM->GetName(),
				*SlotMat.MaterialSlotName.ToString());
			OutEntries.Add(MakeEntry(Mat, EPGXVertexFactoryType::StaticMesh, EntryLabel));
		}
		return;
	}

	// EN: Skeletal Mesh — extract materials from slots / ES: Skeletal Mesh — extraer materiales de slots
	if (USkeletalMesh* SK = Cast<USkeletalMesh>(Asset))
	{
		const TArray<FSkeletalMaterial>& Materials = SK->GetMaterials();
		for (const FSkeletalMaterial& SlotMat : Materials)
		{
			UMaterialInterface* Mat = SlotMat.MaterialInterface;
			if (!Mat) continue;

			const FSoftObjectPath Path(Mat);
			if (DeduplicatedPaths.Contains(Path)) continue;
			DeduplicatedPaths.Add(Path);

			const FString EntryLabel = FString::Printf(TEXT("%s_%s"),
				*SK->GetName(),
				*SlotMat.MaterialSlotName.ToString());
			OutEntries.Add(MakeEntry(Mat, EPGXVertexFactoryType::SkeletalMesh, EntryLabel));
		}
		return;
	}
}

#undef LOCTEXT_NAMESPACE
