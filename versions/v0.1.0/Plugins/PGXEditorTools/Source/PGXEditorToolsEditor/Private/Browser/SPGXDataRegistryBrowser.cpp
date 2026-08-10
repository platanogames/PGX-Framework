// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Browser/SPGXDataRegistryBrowser.h"
#include "PGXEditorToolsEditor.h"
#include "Logging/PGXLogMacros.h"
#include "Registry/PGXDataRegistrySubsystem.h"

#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "Utils/PGXEditorUtils.h"

// EN: Editor / Content Browser / ES: Editor / Content Browser
#include "Editor.h"
#include "Style/PGXVisualTokens.h"
#include "Widgets/SPGXPremiumShell.h"
#include "Widgets/SPGXSectionDivider.h"
#include "Style/PGXEditorStyle.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetRegistry/AssetRegistryModule.h"

#define LOCTEXT_NAMESPACE "PGXDataRegistryBrowser"

// EN: Data Registry accent color (Cyan) / ES: Color de acento de Data Registry (Cyan)
namespace { static const FLinearColor GRegistryColor = PGX::System::DataRegistry; }

// ============================================================================
// EN: Construct / Destruct
// ES: Construccion / Destruccion
// ============================================================================

void SPGXDataRegistryBrowser::Construct(const FArguments& /*InArgs*/)
{
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[RegistryBrowser] Construct"));

	// EN: Bind PIE lifecycle / ES: Enlazar ciclo de vida PIE
	BindPIEDelegates();

	// EN: Check if PIE is already running / ES: Verificar si PIE ya esta corriendo
	if (GEditor && GEditor->PlayWorld)
	{
		OnPIEStarted(false);
		PGX_LOG_INFO(LogPGXEditorTools, TEXT("[RegistryBrowser] PIE already running at Construct"));
	}

	// EN: Initialize StatusText before shell (referenced in footer)
	// ES: Inicializar StatusText antes del shell (referenciado en footer)
	SAssignNew(StatusText, STextBlock)
		.Text(LOCTEXT("StatusEmpty", "No databases loaded — start PIE or register databases to browse"))
		.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
		.Font(PGX::Font::BodySmall());

	ChildSlot
	[
		SNew(SPGXPremiumShell)
		.SystemColor(PGX::System::DataRegistry)
		.Title(LOCTEXT("PanelTitle", "DATA REGISTRY BROWSER"))
		.Subtitle(LOCTEXT("PanelSubtitle", "Browse and inspect registered databases"))
		.Icon(FPGXEditorStyle::Get().GetBrush("PGXEditor.Icon.DataRegistry"))
		.TitleRightContent()
		[
			SNew(SButton)
			.Text(LOCTEXT("Refresh", "Refresh"))
			.OnClicked(this, &SPGXDataRegistryBrowser::OnRefreshClicked)
		]
		.FooterLeftContent()
		[
			StatusText.ToSharedRef()
		]
		.FooterRightContent()
		[
			// EN: PIE state indicator / ES: Indicador de estado PIE
			SNew(STextBlock)
			.Text_Lambda([this]() { return bIsPIEActive ? LOCTEXT("PIEActive", "PIE Active") : LOCTEXT("PIEOffline", "Editor Only"); })
			.Font(PGX::Font::CaptionBold())
			.ColorAndOpacity_Lambda([this]() -> FSlateColor { return bIsPIEActive ? FSlateColor(PGX::Semantic::Good) : FSlateColor(PGX::Text::Muted); })
		]
		.Content()
		[
			// EN: Main content — splitter with database list + entries
			// ES: Contenido principal — splitter con lista de databases + entries
			SNew(SSplitter)
			.Orientation(Orient_Horizontal)

			+ SSplitter::Slot()
			.Value(0.25f)
			[
				BuildDatabaseListPanel()
			]

			+ SSplitter::Slot()
			.Value(0.75f)
			[
				BuildEntriesPanel()
			]
		]
	];

	RefreshDatabases();
}

SPGXDataRegistryBrowser::~SPGXDataRegistryBrowser()
{
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[RegistryBrowser] Destructor — cleaning up"));

	UnbindFromSubsystem();

	if (PIEStartedHandle.IsValid())
	{
		FEditorDelegates::PostPIEStarted.Remove(PIEStartedHandle);
	}
	if (PIEEndedHandle.IsValid())
	{
		FEditorDelegates::EndPIE.Remove(PIEEndedHandle);
	}

	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[RegistryBrowser] Unbound all delegates"));
}

// ============================================================================
// EN: Panel Builders / ES: Constructores de Paneles
// ============================================================================

TSharedRef<SWidget> SPGXDataRegistryBrowser::BuildDatabaseListPanel()
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(4.0f)
		[
			SNew(SVerticalBox)

			// EN: Section header / ES: Cabecera de seccion
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SPGXSectionDivider)
				.Title(LOCTEXT("SecDatabases", "DATABASES"))
				.AccentColor(PGX::System::DataRegistry)
			]

			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SAssignNew(DatabaseListView, SListView<TSharedPtr<FGameplayTag>>)
				.ListItemsSource(&DatabaseTags)
				.OnGenerateRow(this, &SPGXDataRegistryBrowser::OnGenerateDatabaseRow)
				.OnSelectionChanged(this, &SPGXDataRegistryBrowser::OnDatabaseSelected)
				.SelectionMode(ESelectionMode::Single)
			]
		];
}

TSharedRef<SWidget> SPGXDataRegistryBrowser::BuildEntriesPanel()
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(4.0f)
		[
			SNew(SVerticalBox)

			// EN: Search bar / ES: Barra de busqueda
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4.0f, 2.0f)
			[
				SNew(SSearchBox)
				.HintText(LOCTEXT("SearchHint", "Search entries by tag, name, or category..."))
				.OnTextChanged(this, &SPGXDataRegistryBrowser::OnSearchTextChanged)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSeparator)
			]

			// EN: Entry list with real SHeaderRow / ES: Lista de entries con SHeaderRow real
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SAssignNew(EntryListView, SListView<TSharedPtr<FPGXRegistryEntry>>)
				.ListItemsSource(&FilteredEntries)
				.OnGenerateRow(this, &SPGXDataRegistryBrowser::OnGenerateEntryRow)
				.SelectionMode(ESelectionMode::Single)
				.HeaderRow(
					SNew(SHeaderRow)
					+ SHeaderRow::Column("Tag")
					.DefaultLabel(LOCTEXT("ColTag", "Tag"))
					.FillWidth(0.3f)
					+ SHeaderRow::Column("Name")
					.DefaultLabel(LOCTEXT("ColName", "Name"))
					.FillWidth(0.2f)
					+ SHeaderRow::Column("Ver")
					.DefaultLabel(LOCTEXT("ColVer", "Ver"))
					.FillWidth(0.08f)
					+ SHeaderRow::Column("Category")
					.DefaultLabel(LOCTEXT("ColCat", "Category"))
					.FillWidth(0.2f)
					+ SHeaderRow::Column("Status")
					.DefaultLabel(LOCTEXT("ColStat", "Status"))
					.FillWidth(0.12f)
					+ SHeaderRow::Column("Actions")
					.DefaultLabel(FText::GetEmpty())
					.FillWidth(0.1f)
				)
			]
		];
}

// EN: BuildStatusBar() — Now handled by SPGXPremiumShell footer slots in Construct()
// ES: BuildStatusBar() — Ahora manejado por los slots del footer de SPGXPremiumShell en Construct()
TSharedRef<SWidget> SPGXDataRegistryBrowser::BuildStatusBar()
{
	return SNullWidget::NullWidget;
}

// ============================================================================
// EN: Row Generators / ES: Generadores de Filas
// ============================================================================

TSharedRef<ITableRow> SPGXDataRegistryBrowser::OnGenerateDatabaseRow(
	TSharedPtr<FGameplayTag> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	const bool bIsSelected = Item.IsValid() && SelectedDatabase == *Item;

	// EN: Get stats for this database / ES: Obtener stats de esta DB
	int32 EntryCount = 0;
	if (UPGXDataRegistrySubsystem* Registry = UPGXDataRegistrySubsystem::GetCached())
	{
		FPGXDatabaseStats Stats = Registry->GetDatabaseStats(*Item);
		EntryCount = Stats.TotalEntries;
	}

	return SNew(STableRow<TSharedPtr<FGameplayTag>>, OwnerTable)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(6.0f, 3.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item.IsValid() ? PGXEditorUtils::TagToLeafName(*Item) : TEXT("???")))
				.ToolTipText(FText::FromString(Item.IsValid() ? Item->ToString() : TEXT("???")))
				.Font(bIsSelected ? PGX::Font::SubHeader() : PGX::Font::Body())
				.ColorAndOpacity(GRegistryColor)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.0f, 3.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::Format(LOCTEXT("DbStats", "({0})"), FText::AsNumber(EntryCount)))
				.Font(PGX::Font::Caption())
				.ColorAndOpacity(PGX::Text::Muted)
			]
		];
}

TSharedRef<ITableRow> SPGXDataRegistryBrowser::OnGenerateEntryRow(
	TSharedPtr<FPGXRegistryEntry> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	const FString TagLeaf = Item->ItemTag.IsValid() ? PGXEditorUtils::TagToLeafName(Item->ItemTag) : TEXT("---");
	const FString TagFull = Item->ItemTag.IsValid() ? Item->ItemTag.ToString() : TEXT("---");
	const FString NameStr = Item->DisplayName.IsEmpty() ? Item->AssetId.ToString() : Item->DisplayName.ToString();
	const FString VerStr = FString::Printf(TEXT("v%d"), Item->Version);
	const FString CatLeaf = Item->CategoryTag.IsValid() ? PGXEditorUtils::TagToLeafName(Item->CategoryTag) : TEXT("---");
	const FString CatFull = Item->CategoryTag.IsValid() ? Item->CategoryTag.ToString() : TEXT("---");
	const FString StatusStr = Item->IsLoaded() ? TEXT("Cached") : TEXT("Unloaded");
	const FLinearColor StatusColor = Item->IsLoaded()
		? PGX::Semantic::Good
		: PGX::Text::Muted;

	// EN: Capture asset path for Browse button lambda / ES: Capturar path para lambda del boton Browse
	const FSoftObjectPath CapturedPath = Item->AssetPath;

	return SNew(STableRow<TSharedPtr<FPGXRegistryEntry>>, OwnerTable)
		[
			SNew(SHorizontalBox)

			// EN: Tag (leaf with tooltip) / ES: Tag (hoja con tooltip)
			+ SHorizontalBox::Slot()
			.FillWidth(0.3f)
			.VAlign(VAlign_Center)
			.Padding(4.0f, 2.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TagLeaf))
				.ToolTipText(FText::FromString(TagFull))
				.Font(PGX::Font::BodySmall())
			]

			// EN: Name / ES: Nombre
			+ SHorizontalBox::Slot()
			.FillWidth(0.2f)
			.VAlign(VAlign_Center)
			.Padding(4.0f, 2.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(NameStr))
			]

			// EN: Version / ES: Version
			+ SHorizontalBox::Slot()
			.FillWidth(0.08f)
			.VAlign(VAlign_Center)
			.Padding(4.0f, 2.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(VerStr))
				.ColorAndOpacity(PGX::Text::Secondary)
			]

			// EN: Category (leaf with tooltip) / ES: Categoria (hoja con tooltip)
			+ SHorizontalBox::Slot()
			.FillWidth(0.2f)
			.VAlign(VAlign_Center)
			.Padding(4.0f, 2.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(CatLeaf))
				.ToolTipText(FText::FromString(CatFull))
				.ColorAndOpacity(PGX::Text::Secondary)
			]

			// EN: Status / ES: Estado
			+ SHorizontalBox::Slot()
			.FillWidth(0.12f)
			.VAlign(VAlign_Center)
			.Padding(4.0f, 2.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(StatusStr))
				.ColorAndOpacity(StatusColor)
				.Font(PGX::Font::CaptionBold())
			]

			// EN: Browse button / ES: Boton Browse
			+ SHorizontalBox::Slot()
			.FillWidth(0.1f)
			.VAlign(VAlign_Center)
			.Padding(2.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("Browse", "Browse"))
				.ContentPadding(FMargin(4.0f, 1.0f))
				.IsEnabled(!CapturedPath.IsNull())
				.OnClicked_Lambda([CapturedPath]()
				{
					IAssetRegistry& AR = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
					FAssetData AssetData = AR.GetAssetByObjectPath(CapturedPath);
					if (AssetData.IsValid())
					{
						TArray<FAssetData> Assets;
						Assets.Add(AssetData);
						FContentBrowserModule& CBModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
						CBModule.Get().SyncBrowserToAssets(Assets);
					}
					return FReply::Handled();
				})
			]
		];
}

// ============================================================================
// EN: Actions / ES: Acciones
// ============================================================================

void SPGXDataRegistryBrowser::OnDatabaseSelected(TSharedPtr<FGameplayTag> Item, ESelectInfo::Type /*SelectInfo*/)
{
	if (Item.IsValid())
	{
		SelectedDatabase = *Item;
		PGX_LOG_INFO(LogPGXEditorTools, TEXT("[RegistryBrowser] Database selected: %s"), *SelectedDatabase.ToString());
		RefreshEntries();
	}
}

void SPGXDataRegistryBrowser::OnSearchTextChanged(const FText& NewText)
{
	SearchText = NewText.ToString();
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[RegistryBrowser] Search filter: '%s'"), *SearchText);
	ApplySearchFilter();
}

FReply SPGXDataRegistryBrowser::OnRefreshClicked()
{
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[RegistryBrowser] Manual refresh"));
	RefreshDatabases();
	RefreshEntries();
	return FReply::Handled();
}

// ============================================================================
// EN: PIE Lifecycle / ES: Ciclo de vida PIE
// ============================================================================

void SPGXDataRegistryBrowser::BindPIEDelegates()
{
	PIEStartedHandle = FEditorDelegates::PostPIEStarted.AddSP(SharedThis(this), &SPGXDataRegistryBrowser::OnPIEStarted);
	PIEEndedHandle = FEditorDelegates::EndPIE.AddSP(SharedThis(this), &SPGXDataRegistryBrowser::OnPIEEnded);
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[RegistryBrowser] PIE delegates bound"));
}

void SPGXDataRegistryBrowser::OnPIEStarted(bool bIsSimulating)
{
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[RegistryBrowser] PIE started (simulating=%s)"), bIsSimulating ? TEXT("true") : TEXT("false"));
	bIsPIEActive = true;
	BindToSubsystem();
	RefreshDatabases();
	RefreshEntries();
}

void SPGXDataRegistryBrowser::OnPIEEnded(bool /*bIsSimulating*/)
{
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[RegistryBrowser] PIE ended"));
	bIsPIEActive = false;
	UnbindFromSubsystem();
	RefreshDatabases();
}

// ============================================================================
// EN: Subsystem Delegates / ES: Delegados del Subsistema
// ============================================================================

void SPGXDataRegistryBrowser::BindToSubsystem()
{
	UnbindFromSubsystem();

	UPGXDataRegistrySubsystem* Registry = UPGXDataRegistrySubsystem::GetCached();
	if (!Registry)
	{
		return;
	}

	BoundSubsystem = Registry;

	OnAssetRegisteredHandle = Registry->OnAssetRegistered.AddSP(SharedThis(this), &SPGXDataRegistryBrowser::OnAssetRegistered);
	OnAssetUnregisteredHandle = Registry->OnAssetUnregistered.AddSP(SharedThis(this), &SPGXDataRegistryBrowser::OnAssetUnregistered);
	OnDatabaseCreatedHandle = Registry->OnDatabaseCreated.AddSP(SharedThis(this), &SPGXDataRegistryBrowser::OnDatabaseCreated);
	OnCacheInvalidatedHandle = Registry->OnCacheInvalidated.AddSP(SharedThis(this), &SPGXDataRegistryBrowser::OnCacheInvalidated);
	OnTablesIngestedHandle = Registry->OnTablesIngested.AddSP(SharedThis(this), &SPGXDataRegistryBrowser::OnTablesIngested);

	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[RegistryBrowser] Bound 5 subsystem delegates"));
}

void SPGXDataRegistryBrowser::UnbindFromSubsystem()
{
	if (BoundSubsystem.IsValid())
	{
		UPGXDataRegistrySubsystem* Registry = BoundSubsystem.Get();
		Registry->OnAssetRegistered.Remove(OnAssetRegisteredHandle);
		Registry->OnAssetUnregistered.Remove(OnAssetUnregisteredHandle);
		Registry->OnDatabaseCreated.Remove(OnDatabaseCreatedHandle);
		Registry->OnCacheInvalidated.Remove(OnCacheInvalidatedHandle);
		Registry->OnTablesIngested.Remove(OnTablesIngestedHandle);

		OnAssetRegisteredHandle.Reset();
		OnAssetUnregisteredHandle.Reset();
		OnDatabaseCreatedHandle.Reset();
		OnCacheInvalidatedHandle.Reset();
		OnTablesIngestedHandle.Reset();

		BoundSubsystem = nullptr;
		PGX_LOG_INFO(LogPGXEditorTools, TEXT("[RegistryBrowser] Unbound subsystem delegates"));
	}
}

void SPGXDataRegistryBrowser::OnAssetRegistered(const FGameplayTag& DbTag, const FGameplayTag& ItemTag)
{
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[RegistryBrowser] Asset registered: %s in %s — auto-refreshing"), *ItemTag.ToString(), *DbTag.ToString());
	RefreshEntries();
}

void SPGXDataRegistryBrowser::OnAssetUnregistered(const FGameplayTag& DbTag, const FGameplayTag& ItemTag)
{
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[RegistryBrowser] Asset unregistered: %s in %s — auto-refreshing"), *ItemTag.ToString(), *DbTag.ToString());
	RefreshEntries();
}

void SPGXDataRegistryBrowser::OnDatabaseCreated(const FGameplayTag& DbTag)
{
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[RegistryBrowser] Database created: %s — auto-refreshing"), *DbTag.ToString());
	RefreshDatabases();
}

void SPGXDataRegistryBrowser::OnCacheInvalidated(const FGameplayTag& DbTag)
{
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[RegistryBrowser] Cache invalidated: %s — auto-refreshing"), *DbTag.ToString());
	RefreshEntries();
}

void SPGXDataRegistryBrowser::OnTablesIngested(const FGameplayTag& DbTag, int32 EntryCount)
{
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("[RegistryBrowser] Tables ingested: %s (%d entries) — auto-refreshing"), *DbTag.ToString(), EntryCount);
	RefreshDatabases();
	RefreshEntries();
}

// ============================================================================
// EN: Data Refresh / ES: Refresco de Datos
// ============================================================================

void SPGXDataRegistryBrowser::RefreshDatabases()
{
	DatabaseTags.Empty();

	UPGXDataRegistrySubsystem* Registry = UPGXDataRegistrySubsystem::GetCached();
	if (!Registry)
	{
		if (StatusText.IsValid())
		{
			StatusText->SetText(LOCTEXT("NoSubsystem", "Data Registry subsystem not available — start PIE"));
		}
		if (DatabaseListView.IsValid())
		{
			DatabaseListView->RequestListRefresh();
		}
		return;
	}

	TArray<FGameplayTag> Tags = Registry->GetAllDatabaseTags();
	for (const FGameplayTag& DbTag : Tags)
	{
		DatabaseTags.Add(MakeShared<FGameplayTag>(DbTag));
	}

	if (DatabaseListView.IsValid())
	{
		DatabaseListView->RequestListRefresh();
	}

	// EN: Auto-select first database if none selected
	// ES: Auto-seleccionar primera database si ninguna seleccionada
	if (!SelectedDatabase.IsValid() && DatabaseTags.Num() > 0)
	{
		SelectedDatabase = *DatabaseTags[0];
		if (DatabaseListView.IsValid())
		{
			DatabaseListView->SetSelection(DatabaseTags[0]);
		}
		RefreshEntries();
	}

	// EN: Update status with total database count
	// ES: Actualizar estado con conteo total de databases
	if (StatusText.IsValid())
	{
		int32 TotalEntries = 0;
		int32 TotalCached = 0;
		for (const TSharedPtr<FGameplayTag>& DbTag : DatabaseTags)
		{
			FPGXDatabaseStats Stats = Registry->GetDatabaseStats(*DbTag);
			TotalEntries += Stats.TotalEntries;
			TotalCached += Stats.LoadedEntries;
		}

		StatusText->SetText(FText::Format(
			LOCTEXT("StatusFmt", "{0} databases | {1} entries | {2} cached"),
			FText::AsNumber(DatabaseTags.Num()),
			FText::AsNumber(TotalEntries),
			FText::AsNumber(TotalCached)));
	}
}

void SPGXDataRegistryBrowser::RefreshEntries()
{
	AllEntries.Empty();
	FilteredEntries.Empty();

	if (!SelectedDatabase.IsValid())
	{
		if (EntryListView.IsValid())
		{
			EntryListView->RequestListRefresh();
		}
		return;
	}

	UPGXDataRegistrySubsystem* Registry = UPGXDataRegistrySubsystem::GetCached();
	if (!Registry)
	{
		if (EntryListView.IsValid())
		{
			EntryListView->RequestListRefresh();
		}
		return;
	}

	TArray<FPGXRegistryEntry> Entries = Registry->GetAllEntries(SelectedDatabase);
	for (const FPGXRegistryEntry& Entry : Entries)
	{
		AllEntries.Add(MakeShared<FPGXRegistryEntry>(Entry));
	}

	ApplySearchFilter();
}

void SPGXDataRegistryBrowser::ApplySearchFilter()
{
	FilteredEntries.Empty();

	if (SearchText.IsEmpty())
	{
		FilteredEntries = AllEntries;
	}
	else
	{
		for (const TSharedPtr<FPGXRegistryEntry>& Entry : AllEntries)
		{
			const FString TagStr = Entry->ItemTag.ToString();
			const FString NameStr = Entry->AssetId.ToString();
			const FString DisplayStr = Entry->DisplayName.ToString();
			const FString CatStr = Entry->CategoryTag.ToString();

			if (TagStr.Contains(SearchText, ESearchCase::IgnoreCase) ||
				NameStr.Contains(SearchText, ESearchCase::IgnoreCase) ||
				DisplayStr.Contains(SearchText, ESearchCase::IgnoreCase) ||
				CatStr.Contains(SearchText, ESearchCase::IgnoreCase))
			{
				FilteredEntries.Add(Entry);
			}
		}
	}

	if (EntryListView.IsValid())
	{
		EntryListView->RequestListRefresh();
	}
}

#undef LOCTEXT_NAMESPACE
