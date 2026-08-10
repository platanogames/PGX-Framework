// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "LogViewer/SPGXLogViewerTab.h"
#include "Logging/PGXLogTagHelper.h"
#include "Logging/PGXLogDomainRendererBase.h"
#include "Subsystems/PGXLogSubsystem.h"
#include "Logging/PGXLogSerializer.h"
#include "Logging/PGXLogMacros.h"

#include "Editor.h"
#include "DesktopPlatformModule.h"
#include "Misc/FileHelper.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/SWindow.h"
#include "Styling/AppStyle.h"
#include "Utils/PGXEditorUtils.h"
#include "Style/PGXVisualTokens.h"
#include "Widgets/SPGXPremiumShell.h"
#include "Widgets/SPGXSectionDivider.h"
#include "Style/PGXEditorStyle.h"
#include "Framework/Application/SlateApplication.h"

#define LOCTEXT_NAMESPACE "PGXLogViewer"
DEFINE_LOG_CATEGORY_STATIC(LogPGXLogViewer, Log, All);

// EN: Log system color (Blue-Grey) — via PGX Visual Tokens / ES: Color del sistema Log (Azul-Gris) — via PGX Visual Tokens
static const FLinearColor GLogColor = PGX::System::Log;

// ═══════════════════════════════════════════════════════════════
// Construct / Destructor
// ═══════════════════════════════════════════════════════════════

void SPGXLogViewerTab::Construct(const FArguments& /*InArgs*/)
{
	PGX_LOG_INFO(LogPGXLogViewer, TEXT("[LogViewer] Construct v3.0 — Polymorphic Observability"));

	// EN: Initialize verbosity filters (all enabled by default)
	// ES: Inicializar filtros de verbosity (todos habilitados por defecto)
	VerbosityFilters.Add(EPGXLogVerbosity::Verbose, true);
	VerbosityFilters.Add(EPGXLogVerbosity::Debug, true);
	VerbosityFilters.Add(EPGXLogVerbosity::Info, true);
	VerbosityFilters.Add(EPGXLogVerbosity::Warning, true);
	VerbosityFilters.Add(EPGXLogVerbosity::Error, true);
	VerbosityFilters.Add(EPGXLogVerbosity::Fatal, true);

	// EN: Initialize StatusText before shell (referenced in footer)
	// ES: Inicializar StatusText antes del shell (referenciado en footer)
	SAssignNew(StatusText, STextBlock)
		.Text(LOCTEXT("StatusEmpty", "Entries: 0 shown (of 0 total)"))
		.Font(PGX::Font::BodySmall())
		.ColorAndOpacity(FSlateColor(PGX::Text::Muted));

	ChildSlot
	[
		SNew(SPGXPremiumShell)
		.SystemColor(PGX::System::Log)
		.Title(LOCTEXT("PanelTitle", "PGX LOG VIEWER"))
		.Subtitle(LOCTEXT("PanelSubtitle", "Polymorphic Observability v3.0"))
		.Icon(FPGXEditorStyle::Get().GetBrush("PGXEditor.Icon.LogViewer"))
		.StatusText(LOCTEXT("FooterVersion", "PGX Log v3.0"))
		.FooterLeftContent()
		[
			StatusText.ToSharedRef()
		]
		.FooterRightContent()
		[
			SNew(SCheckBox)
			.IsChecked_Lambda([this]() { return bAutoScroll ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
			.OnCheckStateChanged_Lambda([this](ECheckBoxState S) { bAutoScroll = (S == ECheckBoxState::Checked); })
			[
				SNew(STextBlock)
				.Text(LOCTEXT("AutoScroll", "Auto-scroll"))
				.Font(PGX::Font::BodySmall())
			]
		]
		.Content()
		[
			SAssignNew(MainContent, SVerticalBox)

			// EN: Toolbar row / ES: Fila de toolbar
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4.0f)
			[
				BuildToolbar()
			]

			// EN: Verbosity filter row / ES: Fila de filtros de verbosity
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4.0f, 0.0f)
			[
				BuildVerbosityFilters()
			]

			// EN: Domain filter bar / ES: Barra de filtros de dominio
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4.0f, 2.0f)
			[
				BuildDomainFilterBar()
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSeparator)
			]

			// EN: Session info bar / ES: Barra de info de sesion
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4.0f, 2.0f)
			[
				BuildSessionInfoBar()
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSeparator)
			]

			// EN: Column headers / ES: Encabezados de columna
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4.0f, 2.0f)
			[
				BuildColumnHeaders()
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSeparator)
			]

			// EN: Polymorphic list view (main content) / ES: List view polimorfico (contenido principal)
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SAssignNew(ListView, SListView<TSharedPtr<FPGXLogViewItem>>)
				.ListItemsSource(&FilteredItems)
				.OnGenerateRow(this, &SPGXLogViewerTab::OnGenerateRow)
				.OnMouseButtonDoubleClick(this, &SPGXLogViewerTab::OnRowDoubleClicked)
				.SelectionMode(ESelectionMode::Single)
			]
		]
	];

	// EN: Bind to PIE lifecycle events / ES: Enlazar a eventos de ciclo de vida PIE
	BindPIEDelegates();

	// EN: Try to bind immediately if PIE is already running
	// ES: Intentar enlazar inmediatamente si PIE ya esta corriendo
	BindToSubsystem();

	UpdateStatusBar();
	UpdateSessionInfo();
}

SPGXLogViewerTab::~SPGXLogViewerTab()
{
	PGX_LOG_INFO(LogPGXLogViewer, TEXT("[LogViewer] Destructor — cleaning up delegates and detail windows"));

	// EN: Safe cleanup — unbind everything regardless of state
	// ES: Limpieza segura — desenlazar todo sin importar el estado
	UnbindFromSubsystem();
	CleanupDetailWindows();

	if (GEditor)
	{
		FEditorDelegates::PostPIEStarted.Remove(PIEStartedHandle);
		FEditorDelegates::EndPIE.Remove(PIEEndedHandle);
	}
}

// ═══════════════════════════════════════════════════════════════
// UI Build / Construccion de UI
// ═══════════════════════════════════════════════════════════════

TSharedRef<SWidget> SPGXLogViewerTab::BuildToolbar()
{
	return SNew(SHorizontalBox)

		// EN: Search box (fills remaining space) / ES: Caja de busqueda
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(0.0f, 0.0f, 8.0f, 0.0f)
		[
			SNew(SSearchBox)
			.HintText(LOCTEXT("SearchHint", "Search logs... (use tag:PGX.Log.xxx for tag filter)"))
			.OnTextChanged(this, &SPGXLogViewerTab::ParseSearchInput)
		]

		// EN: Refresh mode toggle / ES: Toggle de modo de refresco
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.ToolTipText(LOCTEXT("ModeTooltip", "Toggle between Live and Manual refresh modes"))
			.OnClicked_Lambda([this]() -> FReply { ToggleRefreshMode(); return FReply::Handled(); })
			[
				SAssignNew(ModeBadgeText, STextBlock)
				.Text(LOCTEXT("ModeLive", "LIVE"))
				.Font(PGX::Font::Badge())
				.ColorAndOpacity(FSlateColor(PGX::Semantic::Good))
			]
		]

		// EN: Delta badge (Manual mode — clickable to flush) / ES: Badge delta (modo Manual — click para flush)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.ToolTipText(LOCTEXT("FlushTooltip", "Flush pending entries"))
			.Visibility_Lambda([this]() -> EVisibility
			{
				return (RefreshMode == EPGXLogRefreshMode::Manual && DeltaBuffer.Num() > 0)
					? EVisibility::Visible : EVisibility::Collapsed;
			})
			.OnClicked_Lambda([this]() -> FReply { FlushDeltaBuffer(); return FReply::Handled(); })
			[
				SAssignNew(DeltaBadgeText, STextBlock)
				.Font(PGX::Font::Badge())
				.ColorAndOpacity(FSlateColor(PGX::Semantic::Warn))
			]
		]

		// EN: Import button / ES: Boton importar
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Import", "Import"))
			.ToolTipText(LOCTEXT("ImportTooltip", "Import a previously exported PGX JSON log file"))
			.OnClicked(this, &SPGXLogViewerTab::OnImportClicked)
		]

		// EN: Export button / ES: Boton exportar
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Export", "Export"))
			.ToolTipText(LOCTEXT("ExportTooltip", "Export current logs to JSON in Saved/Logs/PGX/"))
			.OnClicked(this, &SPGXLogViewerTab::OnExportClicked)
		]

		// EN: Clear button / ES: Boton limpiar
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Clear", "Clear"))
			.ToolTipText(LOCTEXT("ClearTooltip", "Clear all log entries from viewer"))
			.OnClicked(this, &SPGXLogViewerTab::OnClearClicked)
		];
}

TSharedRef<SWidget> SPGXLogViewerTab::BuildVerbosityFilters()
{
	TSharedRef<SHorizontalBox> Box = SNew(SHorizontalBox);

	// EN: "All" toggle / ES: Toggle "Todos"
	Box->AddSlot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SCheckBox)
			.IsChecked_Lambda([this]() -> ECheckBoxState
			{
				bool bAllOn = true;
				for (const auto& Pair : VerbosityFilters)
				{
					if (!Pair.Value) { bAllOn = false; break; }
				}
				return bAllOn ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			})
			.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
			{
				const bool bEnable = (NewState == ECheckBoxState::Checked);
				for (auto& Pair : VerbosityFilters)
				{
					Pair.Value = bEnable;
				}
				RebuildFilteredList();
			})
			[
				SNew(STextBlock)
				.Text(LOCTEXT("FilterAll", "All"))
				.Font(PGX::Font::Badge())
			]
		];

	// EN: Per-verbosity checkboxes / ES: Checkboxes por verbosity
	struct FVerbEntry { EPGXLogVerbosity Level; FText Label; };
	const TArray<FVerbEntry> Entries = {
		{ EPGXLogVerbosity::Verbose, LOCTEXT("FilterVerbose", "Verbose") },
		{ EPGXLogVerbosity::Debug,   LOCTEXT("FilterDebug", "Debug") },
		{ EPGXLogVerbosity::Info,    LOCTEXT("FilterInfo", "Info") },
		{ EPGXLogVerbosity::Warning, LOCTEXT("FilterWarning", "Warning") },
		{ EPGXLogVerbosity::Error,   LOCTEXT("FilterError", "Error") },
		{ EPGXLogVerbosity::Fatal,   LOCTEXT("FilterFatal", "Fatal") },
	};

	for (const FVerbEntry& VE : Entries)
	{
		const FLinearColor Color = FPGXLogTagHelper::GetVerbosityColor(VE.Level);

		Box->AddSlot()
			.AutoWidth()
			.Padding(6.0f, 0.0f, 2.0f, 0.0f)
			[
				SNew(SCheckBox)
				.IsChecked_Lambda([this, Level = VE.Level]() -> ECheckBoxState
				{
					const bool* Val = VerbosityFilters.Find(Level);
					return (Val && *Val) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				})
				.OnCheckStateChanged_Lambda([this, Level = VE.Level](ECheckBoxState NewState)
				{
					VerbosityFilters.FindOrAdd(Level) = (NewState == ECheckBoxState::Checked);
					RebuildFilteredList();
				})
				[
					SNew(STextBlock)
					.Text(VE.Label)
					.ColorAndOpacity(FSlateColor(Color))
					.Font(PGX::Font::BodySmall())
				]
			];
	}

	// EN: Category filter dropdown / ES: Dropdown de filtro por categoria
	Box->AddSlot()
		.AutoWidth()
		.Padding(12.0f, 0.0f, 2.0f, 0.0f)
		[
			SNew(SBox)
			.WidthOverride(150.0f)
			[
				SAssignNew(CategoryComboBox, SComboBox<TSharedPtr<FName>>)
				.OptionsSource(&CategoryOptions)
				.OnGenerateWidget(this, &SPGXLogViewerTab::GenerateCategoryComboItem)
				.OnSelectionChanged(this, &SPGXLogViewerTab::OnCategorySelected)
				[
					SNew(STextBlock)
					.Text(this, &SPGXLogViewerTab::GetCurrentCategoryText)
					.Font(PGX::Font::BodySmall())
				]
			]
		];

	// EN: Initialize with "All Categories" option / ES: Inicializar con opcion "Todas las Categorias"
	RebuildCategoryOptions();

	return Box;
}

TSharedRef<SWidget> SPGXLogViewerTab::BuildDomainFilterBar()
{
	// EN: Create the container — populated dynamically when domains are discovered
	// ES: Crear el contenedor — populado dinamicamente cuando se descubren dominios
	return SAssignNew(DomainFilterBox, SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("DomainHint", "Domains: Start PIE to discover"))
			.Font(PGX::Font::Hint())
			.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
		];
}

TSharedRef<SWidget> SPGXLogViewerTab::BuildSessionInfoBar()
{
	return SAssignNew(SessionInfoText, STextBlock)
		.Text(LOCTEXT("SessionNone", "Session: -- | Start PIE to view logs"))
		.Font(PGX::Font::Hint())
		.ColorAndOpacity(FSlateColor(PGX::Text::Secondary));
}

// EN: BuildStatusBar() — Now handled by SPGXPremiumShell footer slots in Construct()
// ES: BuildStatusBar() — Ahora manejado por los slots del footer de SPGXPremiumShell en Construct()
TSharedRef<SWidget> SPGXLogViewerTab::BuildStatusBar()
{
	return SNullWidget::NullWidget;
}

TSharedRef<SWidget> SPGXLogViewerTab::BuildColumnHeaders()
{
	// EN: Column widths match UPGXLogDomainRendererBase::BuildRowWidget() layout
	// ES: Anchos de columna coinciden con el layout de BuildRowWidget() del renderer base
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(70.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("HeaderTime", "TIME"))
				.Font(PGX::Font::CaptionBold())
				.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SBox)
			.WidthOverride(40.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("HeaderVerb", "VERB"))
				.Font(PGX::Font::CaptionBold())
				.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(4.0f, 0.0f, 2.0f, 0.0f)
		[
			SNew(SBox)
			.WidthOverride(8.0f)
			[
				SNullWidget::NullWidget
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SBox)
			.WidthOverride(100.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("HeaderCat", "CATEGORY"))
				.Font(PGX::Font::CaptionBold())
				.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
			]
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(4.0f, 0.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("HeaderMsg", "MESSAGE"))
			.Font(PGX::Font::CaptionBold())
			.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
		];
}

// ═══════════════════════════════════════════════════════════════
// SListView / SListView
// ═══════════════════════════════════════════════════════════════

TSharedRef<ITableRow> SPGXLogViewerTab::OnGenerateRow(
	TSharedPtr<FPGXLogViewItem> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	TSharedRef<SWidget> RowContent = SNullWidget::NullWidget;

	if (Item.IsValid() && Item->Entry.IsValid())
	{
		UPGXLogDomainRendererBase* Renderer = Item->Renderer.Get();
		if (Renderer)
		{
			// EN: Polymorphic rendering via domain renderer
			// ES: Rendering polimorfico via renderer de dominio
			RowContent = Renderer->BuildRowWidget(*Item->Entry);
		}
		else
		{
			// EN: Fallback for imported mode (no subsystem available)
			// ES: Fallback para modo importado (sin subsistema disponible)
			const FPGXLogEntry& Entry = *Item->Entry;
			RowContent = SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(70.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(Entry.Timestamp.ToString(TEXT("%H:%M:%S"))))
						.Font(PGX::Font::Mono())
						.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(2.0f, 0.0f)
				[
					SNew(SBox)
					.WidthOverride(100.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromName(Entry.Category))
						.Font(PGX::Font::Mono())
						.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				.Padding(4.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Entry.Message))
					.Font(PGX::Font::BodySmall())
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				];
		}
	}

	return SNew(STableRow<TSharedPtr<FPGXLogViewItem>>, OwnerTable)
		[
			RowContent
		];
}

void SPGXLogViewerTab::OnRowDoubleClicked(TSharedPtr<FPGXLogViewItem> Item)
{
	OpenDetailWindow(Item);
}

// ═══════════════════════════════════════════════════════════════
// Search & Filter / Busqueda y Filtrado
// ═══════════════════════════════════════════════════════════════

void SPGXLogViewerTab::ParseSearchInput(const FText& NewText)
{
	const FString& Input = NewText.ToString();
	SearchText.Empty();
	SearchTag = FGameplayTag();

	// EN: Extract "tag:XXX" tokens, rest is free text
	// ES: Extraer tokens "tag:XXX", el resto es texto libre
	FString TagToken;
	if (Input.Split(TEXT("tag:"), &SearchText, &TagToken))
	{
		TagToken.TrimStartAndEndInline();
		FString TagName = TagToken;
		int32 SpaceIdx;
		if (TagName.FindChar(TEXT(' '), SpaceIdx))
		{
			TagName.LeftInline(SpaceIdx);
		}
		SearchTag = FGameplayTag::RequestGameplayTag(FName(*TagName), false);
		SearchText.TrimStartAndEndInline();
	}
	else
	{
		SearchText = Input.TrimStartAndEnd();
	}

	RebuildFilteredList();
}

bool SPGXLogViewerTab::MatchesTextSearch(const FPGXLogEntry& Entry) const
{
	if (SearchText.IsEmpty())
	{
		return true;
	}

	if (Entry.Message.Contains(SearchText, ESearchCase::IgnoreCase))
	{
		return true;
	}

	if (Entry.Category.ToString().Contains(SearchText, ESearchCase::IgnoreCase))
	{
		return true;
	}

	for (const FPGXLogParam& Param : Entry.Params)
	{
		if (Param.Key.Contains(SearchText, ESearchCase::IgnoreCase) ||
			Param.Value.Contains(SearchText, ESearchCase::IgnoreCase))
		{
			return true;
		}
	}

	return false;
}

bool SPGXLogViewerTab::PassesFilter(const FPGXLogViewItem& Item) const
{
	if (!Item.Entry.IsValid())
	{
		return false;
	}

	const FPGXLogEntry& Entry = *Item.Entry;

	// EN: 1. Verbosity checkbox / ES: 1. Checkbox de verbosity
	const bool* bVerbEnabled = VerbosityFilters.Find(Entry.Verbosity);
	if (bVerbEnabled && !(*bVerbEnabled))
	{
		return false;
	}

	// EN: 2. Category dropdown / ES: 2. Dropdown de categoria
	if (ActiveCategoryFilter != NAME_None && Entry.Category != ActiveCategoryFilter)
	{
		return false;
	}

	// EN: 3. Domain visibility / ES: 3. Visibilidad de dominio
	if (Entry.DomainTag.IsValid())
	{
		const bool* bDomainVisible = DomainVisibility.Find(Entry.DomainTag);
		if (bDomainVisible && !(*bDomainVisible))
		{
			return false;
		}
	}

	// EN: 4. Text search (Message + Category + Params) / ES: 4. Busqueda de texto
	if (!MatchesTextSearch(Entry))
	{
		return false;
	}

	// EN: 5. Tag search (GameplayTag hierarchy match) / ES: 5. Busqueda de tag
	if (SearchTag.IsValid() && !Entry.Tags.HasTag(SearchTag))
	{
		return false;
	}

	return true;
}

void SPGXLogViewerTab::RebuildFilteredList()
{
	FilteredItems.Reset();
	for (const auto& Item : AllItems)
	{
		if (PassesFilter(*Item))
		{
			FilteredItems.Add(Item);
		}
	}

	if (ListView.IsValid())
	{
		ListView->RequestListRefresh();
	}
	UpdateStatusBar();
}

void SPGXLogViewerTab::UpdateStatusBar()
{
	if (StatusText.IsValid())
	{
		const int32 PendingCount = DeltaBuffer.Num();
		if (PendingCount > 0)
		{
			StatusText->SetText(FText::Format(
				LOCTEXT("StatusFmtPending", "Entries: {0} shown (of {1} total) | {2} pending"),
				FText::AsNumber(FilteredItems.Num()),
				FText::AsNumber(AllItems.Num()),
				FText::AsNumber(PendingCount)
			));
		}
		else
		{
			StatusText->SetText(FText::Format(
				LOCTEXT("StatusFmt", "Entries: {0} shown (of {1} total)"),
				FText::AsNumber(FilteredItems.Num()),
				FText::AsNumber(AllItems.Num())
			));
		}
	}
}

void SPGXLogViewerTab::UpdateSessionInfo()
{
	if (!SessionInfoText.IsValid())
	{
		return;
	}

	// EN: Imported mode — show imported session metadata
	// ES: Modo importado — mostrar metadatos de sesion importada
	if (bViewingImported)
	{
		const FString StartStr = ImportedSession.StartTime.ToString(TEXT("%H:%M"));
		SessionInfoText->SetText(FText::Format(
			LOCTEXT("SessionImportedFmt", "[IMPORTED] Session: #{0} | Started: {1} | Entries: {2} | W:{3} E:{4}"),
			FText::AsNumber(ImportedSession.SessionId),
			FText::FromString(StartStr),
			FText::AsNumber(ImportedSession.EntryCount),
			FText::AsNumber(ImportedSession.WarningCount),
			FText::AsNumber(ImportedSession.ErrorCount)
		));
		return;
	}

	// EN: Live mode / ES: Modo live
	if (!BoundSubsystem.IsValid())
	{
		SessionInfoText->SetText(LOCTEXT("SessionNoPIE", "Session: -- | Start PIE or Import a JSON log file"));
		return;
	}

	const FPGXLogSession Session = BoundSubsystem->GetCurrentSession();
	const FString StartStr = Session.StartTime.ToString(TEXT("%H:%M"));
	SessionInfoText->SetText(FText::Format(
		LOCTEXT("SessionInfoFmt", "Session: #{0} | Started: {1} | Entries: {2} | W:{3} E:{4} | Domains: {5}"),
		FText::AsNumber(Session.SessionId),
		FText::FromString(StartStr),
		FText::AsNumber(Session.EntryCount),
		FText::AsNumber(Session.WarningCount),
		FText::AsNumber(Session.ErrorCount),
		FText::AsNumber(DomainVisibility.Num())
	));
}

void SPGXLogViewerTab::RebuildDomainCounts()
{
	// EN: Recount entries per domain from AllItems
	// ES: Recontar entradas por dominio desde AllItems
	DomainCounts.Reset();
	for (const auto& Item : AllItems)
	{
		if (Item.IsValid() && Item->Entry.IsValid())
		{
			DomainCounts.FindOrAdd(Item->Entry->DomainTag)++;
		}
	}

	// EN: Rebuild the filter bar UI
	// ES: Reconstruir la UI de la barra de filtros
	if (!DomainFilterBox.IsValid())
	{
		return;
	}

	DomainFilterBox->ClearChildren();

	if (DomainVisibility.Num() == 0)
	{
		DomainFilterBox->AddSlot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("NoDomains", "Domains: none discovered"))
				.Font(PGX::Font::Hint())
				.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
			];
		return;
	}

	// EN: "All" toggle / ES: Toggle "Todos"
	DomainFilterBox->AddSlot()
		.AutoWidth()
		.Padding(0.0f, 0.0f, 4.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("DomAll", "All"))
			.ToolTipText(LOCTEXT("DomAllTip", "Show all domains"))
			.OnClicked_Lambda([this]() -> FReply
			{
				for (auto& Pair : DomainVisibility) { Pair.Value = true; }
				RebuildFilteredList();
				return FReply::Handled();
			})
		];

	// EN: "None" toggle / ES: Toggle "Ninguno"
	DomainFilterBox->AddSlot()
		.AutoWidth()
		.Padding(0.0f, 0.0f, 8.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("DomNone", "None"))
			.ToolTipText(LOCTEXT("DomNoneTip", "Hide all domains"))
			.OnClicked_Lambda([this]() -> FReply
			{
				for (auto& Pair : DomainVisibility) { Pair.Value = false; }
				RebuildFilteredList();
				return FReply::Handled();
			})
		];

	// EN: Per-domain toggle buttons / ES: Botones toggle por dominio
	for (const auto& Pair : DomainVisibility)
	{
		const FGameplayTag DomainTag = Pair.Key;
		const int32* CountPtr = DomainCounts.Find(DomainTag);
		const int32 Count = CountPtr ? *CountPtr : 0;

		// EN: Get domain color and name from renderer
		// ES: Obtener color y nombre del dominio desde el renderer
		FLinearColor DomainColor = PGX::Semantic::Neutral;
		FText DomainName = FText::FromString(TEXT("Generic"));

		if (BoundSubsystem.IsValid())
		{
			if (UPGXLogDomainRendererBase* Renderer = BoundSubsystem->GetRendererForDomain(DomainTag))
			{
				DomainColor = Renderer->GetDomainColor();
				DomainName = Renderer->GetDomainDisplayName();
			}
		}
		else if (DomainTag.IsValid())
		{
			// EN: Imported mode fallback: use tag leaf as name
			// ES: Fallback modo importado: usar hoja del tag como nombre
			DomainName = FText::FromString(DomainTag.GetTagName().ToString());
		}

		DomainFilterBox->AddSlot()
			.AutoWidth()
			.Padding(2.0f, 0.0f)
			[
				SNew(SCheckBox)
				.IsChecked_Lambda([this, DomainTag]() -> ECheckBoxState
				{
					const bool* bVisible = DomainVisibility.Find(DomainTag);
					return (bVisible && *bVisible) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				})
				.OnCheckStateChanged_Lambda([this, DomainTag](ECheckBoxState NewState)
				{
					if (bool* bVisible = DomainVisibility.Find(DomainTag))
					{
						*bVisible = (NewState == ECheckBoxState::Checked);
					}
					RebuildFilteredList();
				})
				[
					SNew(SHorizontalBox)

					// EN: Color dot / ES: Punto de color
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.0f, 0.0f, 3.0f, 0.0f)
					[
						SNew(SBorder)
						.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
						.BorderBackgroundColor(DomainColor)
						.Padding(0.0f)
						[
							SNew(SBox)
							.WidthOverride(8.0f)
							.HeightOverride(8.0f)
						]
					]

					// EN: Domain name + count / ES: Nombre de dominio + conteo
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::Format(LOCTEXT("DomainLabel", "{0} ({1})"), DomainName, FText::AsNumber(Count)))
						.Font(PGX::Font::Caption())
					]
				]
			];
	}
}

// ═══════════════════════════════════════════════════════════════
// Refresh Mode / Modo de Refresco
// ═══════════════════════════════════════════════════════════════

void SPGXLogViewerTab::FlushDeltaBuffer()
{
	if (DeltaBuffer.Num() == 0)
	{
		return;
	}

	PGX_LOG_INFO(LogPGXLogViewer, TEXT("[LogViewer] Flushing %d buffered entries"), DeltaBuffer.Num());

	for (const auto& Item : DeltaBuffer)
	{
		if (Item.IsValid() && Item->Entry.IsValid())
		{
			AllItems.Add(Item);
			KnownCategories.Add(Item->Entry->Category);
		}
	}
	DeltaBuffer.Reset();

	RebuildCategoryOptions();
	RebuildDomainCounts();
	RebuildFilteredList();

	if (ListView.IsValid() && bAutoScroll)
	{
		ListView->ScrollToBottom();
	}

	UpdateSessionInfo();
}

void SPGXLogViewerTab::ToggleRefreshMode()
{
	if (RefreshMode == EPGXLogRefreshMode::Live)
	{
		RefreshMode = EPGXLogRefreshMode::Manual;
		if (ModeBadgeText.IsValid())
		{
			ModeBadgeText->SetText(LOCTEXT("ModeManual", "MANUAL"));
			ModeBadgeText->SetColorAndOpacity(FSlateColor(PGX::Text::Secondary));
		}
		PGX_LOG_INFO(LogPGXLogViewer, TEXT("[LogViewer] Switched to Manual mode"));
	}
	else
	{
		RefreshMode = EPGXLogRefreshMode::Live;
		if (ModeBadgeText.IsValid())
		{
			ModeBadgeText->SetText(LOCTEXT("ModeLive", "LIVE"));
			ModeBadgeText->SetColorAndOpacity(FSlateColor(PGX::Semantic::Good));
		}
		PGX_LOG_INFO(LogPGXLogViewer, TEXT("[LogViewer] Switched to Live mode — flushing buffer"));
		FlushDeltaBuffer();
	}
}

// ═══════════════════════════════════════════════════════════════
// Detail Windows / Ventanas de Detalle
// ═══════════════════════════════════════════════════════════════

void SPGXLogViewerTab::OpenDetailWindow(TSharedPtr<FPGXLogViewItem> Item)
{
	if (!Item.IsValid() || !Item->Entry.IsValid())
	{
		return;
	}

	// EN: Clean up expired window refs / ES: Limpiar refs de ventanas expiradas
	OpenDetailWindows.RemoveAll([](const TWeakPtr<SWindow>& W) { return !W.IsValid(); });

	const FPGXLogEntry& Entry = *Item->Entry;

	// EN: Build window title / ES: Construir titulo de ventana
	FString DomainName = TEXT("Generic");
	if (UPGXLogDomainRendererBase* Renderer = Item->Renderer.Get())
	{
		DomainName = Renderer->GetDomainDisplayName().ToString();
	}

	const FString TimeStr = Entry.Timestamp.ToString(TEXT("%H:%M:%S"));
	FString WindowTitle = FString::Printf(TEXT("[%s] Entry — %s"), *DomainName, *TimeStr);
	if (!bPIEActive)
	{
		WindowTitle = FString::Printf(TEXT("[STALE] %s"), *WindowTitle);
	}

	// EN: Build detail content via renderer / ES: Construir contenido de detalle via renderer
	TSharedRef<SWidget> DetailContent = SNullWidget::NullWidget;
	if (UPGXLogDomainRendererBase* Renderer = Item->Renderer.Get())
	{
		DetailContent = Renderer->BuildDetailWidget(Entry);
	}
	else
	{
		// EN: Fallback for imported mode / ES: Fallback para modo importado
		DetailContent = SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(8.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Entry.Message))
				.Font(PGX::Font::SectionHeader())
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(8.0f, 0.0f, 8.0f, 4.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("%s | %s"),
					*TimeStr, *Entry.Category.ToString())))
				.Font(PGX::Font::Mono())
				.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
			];
	}

	// EN: Create and display the window / ES: Crear y mostrar la ventana
	TSharedRef<SWindow> NewWindow = SNew(SWindow)
		.Title(FText::FromString(WindowTitle))
		.ClientSize(FVector2D(650, 500))
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.SupportsMaximize(true)
		.SupportsMinimize(true);

	NewWindow->SetContent(
		SNew(SScrollBox)
		+ SScrollBox::Slot()
		.Padding(4.0f)
		[
			DetailContent
		]
	);

	FSlateApplication::Get().AddWindow(NewWindow);
	OpenDetailWindows.Add(NewWindow);

	PGX_LOG_INFO(LogPGXLogViewer, TEXT("[LogViewer] Opened detail window: %s"), *WindowTitle);
}

void SPGXLogViewerTab::CleanupDetailWindows()
{
	for (const TWeakPtr<SWindow>& WeakWindow : OpenDetailWindows)
	{
		if (TSharedPtr<SWindow> Window = WeakWindow.Pin())
		{
			Window->RequestDestroyWindow();
		}
	}
	OpenDetailWindows.Reset();
}

void SPGXLogViewerTab::MarkDetailWindowsStale()
{
	for (int32 i = OpenDetailWindows.Num() - 1; i >= 0; --i)
	{
		if (TSharedPtr<SWindow> Window = OpenDetailWindows[i].Pin())
		{
			const FString CurrentTitle = Window->GetTitle().ToString();
			if (!CurrentTitle.StartsWith(TEXT("[STALE]")))
			{
				Window->SetTitle(FText::FromString(FString::Printf(TEXT("[STALE] %s"), *CurrentTitle)));
			}
		}
		else
		{
			OpenDetailWindows.RemoveAt(i);
		}
	}
}

// ═══════════════════════════════════════════════════════════════
// PIE Lifecycle / Ciclo de Vida PIE
// ═══════════════════════════════════════════════════════════════

void SPGXLogViewerTab::BindPIEDelegates()
{
	if (!GEditor)
	{
		return;
	}

	PIEStartedHandle = FEditorDelegates::PostPIEStarted.AddSP(SharedThis(this), &SPGXLogViewerTab::OnPIEStarted);
	PIEEndedHandle = FEditorDelegates::EndPIE.AddSP(SharedThis(this), &SPGXLogViewerTab::OnPIEEnded);
	PGX_LOG_INFO(LogPGXLogViewer, TEXT("[LogViewer] PIE delegates bound"));
}

void SPGXLogViewerTab::OnPIEStarted(bool bIsSimulating)
{
	PGX_LOG_INFO(LogPGXLogViewer, TEXT("[LogViewer] PIE started (simulating=%d)"), bIsSimulating);

	bPIEActive = true;

	// EN: Switch from imported mode to live mode when PIE starts
	// ES: Cambiar de modo importado a modo live cuando PIE inicia
	bViewingImported = false;
	ImportedSession = FPGXLogSession();

	BindToSubsystem();
}

void SPGXLogViewerTab::OnPIEEnded(bool bIsSimulating)
{
	PGX_LOG_INFO(LogPGXLogViewer, TEXT("[LogViewer] PIE ended (simulating=%d)"), bIsSimulating);

	bPIEActive = false;
	UnbindFromSubsystem();

	// EN: Mark open detail windows as stale (data snapshot, not live)
	// ES: Marcar ventanas de detalle como stale (snapshot de datos, no live)
	MarkDetailWindowsStale();

	// EN: Retain entries (keep last snapshot visible) but update session info
	// ES: Retener entradas pero actualizar info de sesion
	UpdateSessionInfo();
}

void SPGXLogViewerTab::BindToSubsystem()
{
	if (BoundSubsystem.IsValid())
	{
		return;
	}

	if (!GEditor)
	{
		return;
	}

	// EN: Find subsystem via PIE world context
	// ES: Encontrar subsistema via contexto de mundo PIE
	UPGXLogSubsystem* LogSub = nullptr;

	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game)
		{
			if (UWorld* World = Context.World())
			{
				if (UGameInstance* GI = World->GetGameInstance())
				{
					LogSub = GI->GetSubsystem<UPGXLogSubsystem>();
					if (LogSub)
					{
						break;
					}
				}
			}
		}
	}

	if (!LogSub)
	{
		return;
	}

	BoundSubsystem = LogSub;

	// EN: Load existing entries snapshot + resolve renderers
	// ES: Cargar snapshot de entradas existentes + resolver renderers
	TArray<FPGXLogEntry> Existing = LogSub->GetCurrentSessionEntries();
	AllItems.Reset();
	KnownCategories.Reset();
	DomainVisibility.Reset();
	DomainCounts.Reset();
	DeltaBuffer.Reset();

	for (FPGXLogEntry& E : Existing)
	{
		KnownCategories.Add(E.Category);

		auto Item = MakeShared<FPGXLogViewItem>();
		Item->Entry = MakeShared<FPGXLogEntry>(MoveTemp(E));
		Item->Renderer = ResolveRenderer(*Item->Entry);
		AllItems.Add(Item);

		// EN: Track domain visibility (default visible)
		// ES: Rastrear visibilidad de dominio (visible por defecto)
		if (Item->Entry->DomainTag.IsValid() && !DomainVisibility.Contains(Item->Entry->DomainTag))
		{
			DomainVisibility.Add(Item->Entry->DomainTag, true);
		}
	}

	// EN: Initialize domain visibility from subsystem registry (domains without entries yet)
	// ES: Inicializar visibilidad de dominio desde registro del subsistema
	for (const auto& DomPair : LogSub->GetAllDomains())
	{
		if (!DomainVisibility.Contains(DomPair.Key))
		{
			DomainVisibility.Add(DomPair.Key, true);
		}
	}

	// EN: Subscribe to new entries / ES: Suscribirse a nuevas entradas
	LogEntryHandle = LogSub->OnLogEntryAddedNative.AddSP(SharedThis(this), &SPGXLogViewerTab::OnNewLogEntry);

	PGX_LOG_INFO(LogPGXLogViewer, TEXT("[LogViewer] Bound to subsystem — %d entries, %d categories, %d domains"),
		AllItems.Num(), KnownCategories.Num(), DomainVisibility.Num());

	RebuildCategoryOptions();
	RebuildDomainCounts();
	RebuildFilteredList();
	UpdateSessionInfo();
}

void SPGXLogViewerTab::UnbindFromSubsystem()
{
	if (LogEntryHandle.IsValid() && BoundSubsystem.IsValid())
	{
		BoundSubsystem->OnLogEntryAddedNative.Remove(LogEntryHandle);
	}
	LogEntryHandle.Reset();
	BoundSubsystem.Reset();
}

// ═══════════════════════════════════════════════════════════════
// Log Entry Callback / Callback de Entrada de Log
// ═══════════════════════════════════════════════════════════════

void SPGXLogViewerTab::OnNewLogEntry(const FPGXLogEntry& Entry)
{
	if (!BoundSubsystem.IsValid())
	{
		return;
	}

	// EN: Create view item with resolved renderer
	// ES: Crear item de vista con renderer resuelto
	auto NewItem = MakeShared<FPGXLogViewItem>();
	NewItem->Entry = MakeShared<FPGXLogEntry>(Entry);
	NewItem->Renderer = ResolveRenderer(Entry);

	// EN: Track new domain if unknown / ES: Rastrear nuevo dominio si desconocido
	bool bNewDomain = false;
	if (Entry.DomainTag.IsValid() && !DomainVisibility.Contains(Entry.DomainTag))
	{
		DomainVisibility.Add(Entry.DomainTag, true);
		bNewDomain = true;
	}

	KnownCategories.Add(Entry.Category);

	if (RefreshMode == EPGXLogRefreshMode::Live)
	{
		// EN: Live mode: add immediately / ES: Modo live: agregar inmediatamente
		AllItems.Add(NewItem);
		DomainCounts.FindOrAdd(Entry.DomainTag)++;

		if (PassesFilter(*NewItem))
		{
			FilteredItems.Add(NewItem);
			if (ListView.IsValid())
			{
				ListView->RequestListRefresh();
				if (bAutoScroll)
				{
					ListView->ScrollToBottom();
				}
			}
		}

		RebuildCategoryOptions();

		if (bNewDomain)
		{
			RebuildDomainCounts();
		}

		UpdateStatusBar();
		UpdateSessionInfo();
	}
	else
	{
		// EN: Manual mode: buffer the entry / ES: Modo manual: almacenar la entrada
		DeltaBuffer.Add(NewItem);

		if (DeltaBadgeText.IsValid())
		{
			DeltaBadgeText->SetText(FText::Format(
				LOCTEXT("DeltaFmt", "{0} new"),
				FText::AsNumber(DeltaBuffer.Num())
			));
		}

		UpdateStatusBar();
	}
}

// ═══════════════════════════════════════════════════════════════
// Actions / Acciones
// ═══════════════════════════════════════════════════════════════

FReply SPGXLogViewerTab::OnExportClicked()
{
	if (BoundSubsystem.IsValid())
	{
		BoundSubsystem->ExportToJsonAsync();
		PGX_LOG_INFO(LogPGXLogViewer, TEXT("[LogViewer] Export initiated — %d entries"), AllItems.Num());
		UpdateActionFeedback(true, FString::Printf(TEXT("Exporting %d logs to Saved/Logs/PGX/..."), AllItems.Num()));
	}
	else if (bViewingImported)
	{
		UpdateActionFeedback(false, TEXT("Cannot export imported sessions"));
	}
	else
	{
		UpdateActionFeedback(false, TEXT("No active session to export"));
	}
	return FReply::Handled();
}

FReply SPGXLogViewerTab::OnImportClicked()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
	{
		return FReply::Handled();
	}

	const FString DefaultPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Logs/PGX"));
	TArray<FString> SelectedFiles;

	const bool bOpened = DesktopPlatform->OpenFileDialog(
		FSlateApplication::Get().FindBestParentWindowHandleForDialogs(AsShared()),
		TEXT("Import PGX Log File"),
		DefaultPath,
		TEXT(""),
		TEXT("PGX Log JSON (*.json)|*.json"),
		EFileDialogFlags::None,
		SelectedFiles
	);

	if (!bOpened || SelectedFiles.Num() == 0)
	{
		return FReply::Handled();
	}

	const FString CleanFilename = FPaths::GetCleanFilename(SelectedFiles[0]);
	FString JsonContent;
	if (!FFileHelper::LoadFileToString(JsonContent, *SelectedFiles[0]))
	{
		PGX_LOG_WARNING(LogPGXLogViewer, TEXT("[LogViewer] Failed to read file: %s"), *SelectedFiles[0]);
		UpdateActionFeedback(false, FString::Printf(TEXT("Failed to read: %s"), *CleanFilename));
		return FReply::Handled();
	}

	TArray<FPGXLogEntry> ImportedEntries;
	FPGXLogSession ParsedSession;
	if (!FPGXLogSerializer::FromJson(JsonContent, ImportedEntries, ParsedSession))
	{
		PGX_LOG_WARNING(LogPGXLogViewer, TEXT("[LogViewer] Failed to parse JSON from: %s"), *SelectedFiles[0]);
		UpdateActionFeedback(false, FString::Printf(TEXT("Invalid JSON format: %s"), *CleanFilename));
		return FReply::Handled();
	}

	// EN: Switch to imported mode — disconnect from live PIE if connected
	// ES: Cambiar a modo importado — desconectar del PIE live si esta conectado
	UnbindFromSubsystem();

	bViewingImported = true;
	bPIEActive = false;
	ImportedSession = ParsedSession;

	AllItems.Reset();
	KnownCategories.Reset();
	DomainVisibility.Reset();
	DomainCounts.Reset();
	DeltaBuffer.Reset();

	for (FPGXLogEntry& E : ImportedEntries)
	{
		KnownCategories.Add(E.Category);

		auto Item = MakeShared<FPGXLogViewItem>();
		Item->Entry = MakeShared<FPGXLogEntry>(MoveTemp(E));
		// EN: Renderer is nullptr in imported mode (no subsystem)
		// ES: Renderer es nullptr en modo importado (sin subsistema)
		AllItems.Add(Item);

		if (Item->Entry->DomainTag.IsValid() && !DomainVisibility.Contains(Item->Entry->DomainTag))
		{
			DomainVisibility.Add(Item->Entry->DomainTag, true);
		}
	}

	RebuildCategoryOptions();
	RebuildDomainCounts();
	RebuildFilteredList();
	UpdateSessionInfo();

	PGX_LOG_INFO(LogPGXLogViewer, TEXT("[LogViewer] Imported %d entries from %s"), AllItems.Num(), *CleanFilename);
	UpdateActionFeedback(true, FString::Printf(TEXT("Imported %d entries from %s"), AllItems.Num(), *CleanFilename));

	return FReply::Handled();
}

FReply SPGXLogViewerTab::OnClearClicked()
{
	PGX_LOG_INFO(LogPGXLogViewer, TEXT("[LogViewer] Clear — removing %d entries"), AllItems.Num());

	AllItems.Reset();
	FilteredItems.Reset();
	DeltaBuffer.Reset();
	KnownCategories.Reset();
	DomainVisibility.Reset();
	DomainCounts.Reset();
	ActiveCategoryFilter = NAME_None;
	LastCategoryCount = 0;
	bViewingImported = false;
	ImportedSession = FPGXLogSession();

	RebuildCategoryOptions();
	RebuildDomainCounts();

	if (ListView.IsValid())
	{
		ListView->RequestListRefresh();
	}
	UpdateStatusBar();
	UpdateSessionInfo();
	return FReply::Handled();
}

// ═══════════════════════════════════════════════════════════════
// Category Filter / Filtro de Categoria
// ═══════════════════════════════════════════════════════════════

void SPGXLogViewerTab::RebuildCategoryOptions()
{
	const int32 CurrentCount = KnownCategories.Num();
	if (CurrentCount == LastCategoryCount && CategoryOptions.Num() > 0)
	{
		return;
	}
	LastCategoryCount = CurrentCount;

	CategoryOptions.Reset();
	CategoryOptions.Add(MakeShared<FName>(NAME_None));
	for (const FName& Cat : KnownCategories)
	{
		CategoryOptions.Add(MakeShared<FName>(Cat));
	}

	CategoryOptions.Sort([](const TSharedPtr<FName>& A, const TSharedPtr<FName>& B)
	{
		if (*A == NAME_None) return true;
		if (*B == NAME_None) return false;
		return A->ToString() < B->ToString();
	});

	if (CategoryComboBox.IsValid())
	{
		CategoryComboBox->RefreshOptions();
	}
}

TSharedRef<SWidget> SPGXLogViewerTab::GenerateCategoryComboItem(TSharedPtr<FName> Item)
{
	const FText Label = (*Item == NAME_None)
		? LOCTEXT("AllCategories", "All Categories")
		: FText::FromName(*Item);

	return SNew(STextBlock)
		.Text(Label)
		.Font(PGX::Font::BodySmall());
}

void SPGXLogViewerTab::OnCategorySelected(TSharedPtr<FName> Item, ESelectInfo::Type /*SelectType*/)
{
	ActiveCategoryFilter = Item.IsValid() ? *Item : NAME_None;
	PGX_LOG_INFO(LogPGXLogViewer, TEXT("[LogViewer] Category filter changed to: %s"),
		ActiveCategoryFilter == NAME_None ? TEXT("All") : *ActiveCategoryFilter.ToString());
	RebuildFilteredList();
}

FText SPGXLogViewerTab::GetCurrentCategoryText() const
{
	return (ActiveCategoryFilter == NAME_None)
		? LOCTEXT("AllCats", "All Categories")
		: FText::FromName(ActiveCategoryFilter);
}

// ═══════════════════════════════════════════════════════════════
// Action Feedback / Feedback de Accion
// ═══════════════════════════════════════════════════════════════

void SPGXLogViewerTab::UpdateActionFeedback(bool bSuccess, const FString& Detail)
{
	if (StatusText.IsValid())
	{
		StatusText->SetText(FText::FromString(Detail));
		StatusText->SetColorAndOpacity(bSuccess
			? FSlateColor(PGX::Semantic::Good)
			: FSlateColor(PGX::Semantic::Error));
	}
}

// ═══════════════════════════════════════════════════════════════
// Renderer Resolution / Resolucion de Renderer
// ═══════════════════════════════════════════════════════════════

UPGXLogDomainRendererBase* SPGXLogViewerTab::ResolveRenderer(const FPGXLogEntry& Entry)
{
	if (!BoundSubsystem.IsValid())
	{
		return nullptr;
	}

	if (Entry.DomainTag.IsValid())
	{
		return BoundSubsystem->GetRendererForDomain(Entry.DomainTag);
	}

	// EN: Fallback: generic renderer for entries without domain tag
	// ES: Fallback: renderer generico para entradas sin tag de dominio
	return BoundSubsystem->GetGenericRenderer();
}

#undef LOCTEXT_NAMESPACE
