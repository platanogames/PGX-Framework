// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Widgets/SDocWindow.h"
#include "Widgets/SDocTreeView.h"
#include "Widgets/SDocContentView.h"
// NOTE: SDocSearchPanel is [Reserved] — not integrated in main UI flow
#include "Render/SDocSlateRenderer.h"
#include "PGXDocsModule.h"
#include "Logging/PGXLogMacros.h"
#include "PGXDocSystem.h"
#include "PGXDocsSettings.h"
#include "PGXDocsConfig.h"
#include "Widgets/SPGXPremiumShell.h"
#include "Style/PGXVisualTokens.h"
#include "Style/PGXEditorStyle.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SMenuAnchor.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/AppStyle.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "JsonObjectConverter.h"

#define LOCTEXT_NAMESPACE "PGXDocs"

SDocWindow::~SDocWindow()
{
	// EN: Unsubscribe from doc system changes / ES: Desuscribirse de cambios del sistema de docs
	if (DocsChangedHandle.IsValid())
	{
		FDocSystem::Get().OnDocsChanged.Remove(DocsChangedHandle);
	}
}

void SDocWindow::Construct(const FArguments& /*InArgs*/)
{
	// EN: Initialize the documentation system / ES: Inicializar el sistema de documentacion
	FDocSystem& DocSystem = FDocSystem::Get();
	if (!DocSystem.IsInitialized())
	{
		DocSystem.Initialize();
	}

	// EN: Subscribe to doc changes / ES: Suscribirse a cambios de docs
	DocsChangedHandle = DocSystem.OnDocsChanged.AddSP(SharedThis(this), &SDocWindow::OnDocsChanged);

	ChildSlot
	[
		SNew(SPGXPremiumShell)
		.SystemColor(PGX::System::Documentation)
		.Title(LOCTEXT("ShellTitle", "DOCUMENTATION"))
		.Subtitle(LOCTEXT("ShellSubtitle", "PGX integrated docs browser"))
		.Icon(FPGXEditorStyle::Get().GetBrush("PGXEditor.Icon.Docs"))
		.bShowFooter(true)
		.StatusText_Lambda([this]()
		{
			const FDocSystem& DocSys = FDocSystem::Get();
			return FText::FromString(FString::Printf(TEXT("%d docs indexed | PGX v0.4.0"), DocSys.GetDocCount()));
		})
		.TitleRightContent()
		[
			BuildTopBar()
		]
		.Content()
		[
			SNew(SSplitter)
			.Orientation(Orient_Horizontal)

			+ SSplitter::Slot()
			.Value(0.25f)
			[
				SAssignNew(TreeView, SDocTreeView)
				.OnDocSelected_Lambda([this](const FString& DocId)
				{
					OnTreeSelectionChanged(DocId);
				})
			]

			+ SSplitter::Slot()
			.Value(0.75f)
			[
				SAssignNew(ContentView, SDocContentView)
			]
		]
	];

	UpdateStatusBar();
}

TSharedRef<SWidget> SDocWindow::BuildTopBar()
{
	return SNew(SHorizontalBox)

		// EN: Search box / ES: Caja de busqueda
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(4.0f, 2.0f)
		[
			SNew(SSearchBox)
			.HintText(LOCTEXT("SearchHint", "Search documentation..."))
			.OnTextChanged_Lambda([this](const FText& Text)
			{
				// EN: Debounce search — 250ms delay to avoid per-keystroke queries
				// ES: Debounce de busqueda — 250ms de retraso para evitar consultas por tecla
				PendingSearchQuery = Text.ToString();

				if (SearchDebounceHandle.IsValid())
				{
					UnRegisterActiveTimer(SearchDebounceHandle.ToSharedRef());
					SearchDebounceHandle.Reset();
				}

				if (PendingSearchQuery.Len() >= 2)
				{
					SearchDebounceHandle = RegisterActiveTimer(0.25f,
						FWidgetActiveTimerDelegate::CreateSP(this, &SDocWindow::OnSearchDebounce));
				}
			})
		]

		// EN: Bookmark toggle button / ES: Boton toggle de bookmark
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f)
		[
			SNew(SButton)
			.ButtonStyle(FAppStyle::Get(), "SimpleButton")
			.ToolTipText(LOCTEXT("BookmarkToggleTooltip", "Toggle bookmark for current document"))
			.OnClicked(this, &SDocWindow::OnToggleBookmark)
			[
				SAssignNew(BookmarkToggleLabel, STextBlock)
				.Text(FText::FromString(TEXT("\u2606")))  // empty star
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
			]
		]

		// EN: Bookmarks dropdown button / ES: Boton dropdown de bookmarks
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f)
		[
			SAssignNew(BookmarkMenuAnchor, SMenuAnchor)
			.OnGetMenuContent(this, &SDocWindow::BuildBookmarkDropdownContent)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.ToolTipText(LOCTEXT("BookmarksDropdownTooltip", "Show bookmarked documents"))
				.OnClicked_Lambda([this]()
				{
					if (BookmarkMenuAnchor.IsValid())
					{
						BookmarkMenuAnchor->SetIsOpen(!BookmarkMenuAnchor->IsOpen());
					}
					return FReply::Handled();
				})
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("\u2630")))  // trigram / list icon
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
				]
			]
		]

		// EN: Reload button / ES: Boton de recargar
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f)
		[
			SNew(SButton)
			.ButtonStyle(FAppStyle::Get(), "SimpleButton")
			.ToolTipText(LOCTEXT("ReloadTooltip", "Reload all documentation"))
			.OnClicked(this, &SDocWindow::OnReloadClicked)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("\u21BB")))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
			]
		]

		// EN: Validate button / ES: Boton de validar
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f)
		[
			SNew(SButton)
			.ButtonStyle(FAppStyle::Get(), "SimpleButton")
			.ToolTipText(LOCTEXT("ValidateTooltip", "Validate all documentation links"))
			.OnClicked(this, &SDocWindow::OnValidateClicked)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("\u2713")))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
			]
		];
}

TSharedRef<SWidget> SDocWindow::BuildStatusBar()
{
	return SNew(SBox)
		.Padding(FMargin(8.0f, 2.0f))
		[
			SAssignNew(StatusBarText, STextBlock)
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)))
		];
}

void SDocWindow::NavigateToDoc(const FString& DocId)
{
	// EN: Re-entrancy guard — breaks the recursion loop:
	//     SelectDocument → SetSelection → OnSelectionChanged → NavigateToDoc → return
	// ES: Guard de re-entrancia — rompe el loop de recursion:
	//     SelectDocument → SetSelection → OnSelectionChanged → NavigateToDoc → return
	if (bIsNavigating)
	{
		PGX_LOG_VERBOSE(LogPGXDocs, TEXT("PGXDocs: NavigateToDoc SKIPPED (re-entrancy guard) — DocId='%s'"), *DocId);
		return;
	}
	TGuardValue<bool> NavigationGuard(bIsNavigating, true);

	PGX_LOG_INFO(LogPGXDocs, TEXT("PGXDocs: NavigateToDoc — DocId='%s'"), *DocId);

	// EN: Save scroll position of current document before navigating away
	// ES: Guardar posicion de scroll del documento actual antes de navegar
	if (!CurrentDocId.IsEmpty() && ContentView.IsValid())
	{
		TSharedPtr<SDocSlateRenderer> Renderer = ContentView->GetRenderer();
		if (Renderer.IsValid())
		{
			DocScrollPositions.Add(CurrentDocId, Renderer->GetScrollOffset());
		}
	}

	CurrentDocId = DocId;

	if (ContentView.IsValid())
	{
		ContentView->LoadDocument(DocId);

		// EN: Restore scroll position if we've visited this doc before
		// ES: Restaurar posicion de scroll si ya visitamos este doc antes
		if (const float* SavedScroll = DocScrollPositions.Find(DocId))
		{
			TSharedPtr<SDocSlateRenderer> Renderer = ContentView->GetRenderer();
			if (Renderer.IsValid())
			{
				Renderer->SetScrollOffset(*SavedScroll);
			}
		}
	}

	if (TreeView.IsValid())
	{
		TreeView->SelectDocument(DocId);
	}

	UpdateBookmarkButton();
	PGX_LOG_INFO(LogPGXDocs, TEXT("PGXDocs: NavigateToDoc DONE — DocId='%s'"), *DocId);
}

void SDocWindow::ReloadCurrentDoc()
{
	if (!CurrentDocId.IsEmpty() && ContentView.IsValid())
	{
		ContentView->LoadDocument(CurrentDocId);
	}
}

void SDocWindow::OnTreeSelectionChanged(const FString& DocId)
{
	NavigateToDoc(DocId);
}

void SDocWindow::OnSearchResultClicked(const FString& DocId)
{
	NavigateToDoc(DocId);
}

void SDocWindow::OnDocsChanged()
{
	// EN: Refresh tree and current document / ES: Refrescar arbol y documento actual
	if (TreeView.IsValid())
	{
		TreeView->RefreshTree();
	}

	if (!CurrentDocId.IsEmpty())
	{
		ReloadCurrentDoc();
	}

	UpdateStatusBar();
}

void SDocWindow::UpdateStatusBar()
{
	if (StatusBarText.IsValid())
	{
		const FDocSystem& DocSystem = FDocSystem::Get();
		const UPGXDocsSettings* Settings = UPGXDocsSettings::Get();

		FString LiveStatus = Settings->bEnableLiveReload ? TEXT("Live") : TEXT("Manual");
		FString BookmarkInfo = Bookmarks.Num() > 0
			? FString::Printf(TEXT(" | %d bookmarks"), Bookmarks.Num())
			: TEXT("");
		FString StatusText = FString::Printf(TEXT("%d docs indexed | %s%s | PGX v0.4.0"),
			DocSystem.GetDocCount(), *LiveStatus, *BookmarkInfo);

		StatusBarText->SetText(FText::FromString(StatusText));
	}
}

FReply SDocWindow::OnReloadClicked()
{
	FDocSystem::Get().RebuildRegistry();
	UpdateStatusBar();
	return FReply::Handled();
}

FReply SDocWindow::OnValidateClicked()
{
	FDocValidationReport Report = FDocSystem::Get().ValidateAll();

	FString Message = FString::Printf(TEXT("Validation: %d docs, %d errors, %d warnings"),
		Report.TotalDocs, Report.Errors.Num(), Report.Warnings.Num());

	PGX_LOG_INFO(LogPGXDocs, TEXT("PGXDocs: %s"), *Message);

	// EN: Log individual errors / ES: Loguear errores individuales
	for (const FDocValidationEntry& Error : Report.Errors)
	{
		PGX_LOG_WARNING(LogPGXDocs, TEXT("PGXDocs [%s]: %s — %s"), *Error.Type, *Error.DocId, *Error.Message);
	}

	// EN: Export JSON report if ValidationReportPath is configured
	// ES: Exportar reporte JSON si ValidationReportPath esta configurado
	const UPGXDocsConfig* Config = GetDefault<UPGXDocsConfig>();
	if (Config && !Config->ValidationReportPath.IsEmpty())
	{
		// EN: Serialize struct to JSON using UE5 standard converter
		// ES: Serializar struct a JSON usando convertidor estandar de UE5
		FString JsonOutput;
		if (FJsonObjectConverter::UStructToJsonObjectString(FDocValidationReport::StaticStruct(), &Report, JsonOutput, 0, 0))
		{
			FString FullPath = FPaths::ProjectDir() / Config->ValidationReportPath;
			if (FFileHelper::SaveStringToFile(JsonOutput, *FullPath))
			{
				PGX_LOG_INFO(LogPGXDocs, TEXT("PGXDocs: Validation report exported to %s"), *FullPath);
			}
			else
			{
				PGX_LOG_ERROR(LogPGXDocs, TEXT("PGXDocs: Failed to export validation report to %s"), *FullPath);
			}
		}
		else
		{
			PGX_LOG_ERROR(LogPGXDocs, TEXT("PGXDocs: Failed to serialize validation report to JSON"));
		}
	}

	return FReply::Handled();
}

// ============================================================================
// Bookmarks
// ============================================================================

FReply SDocWindow::OnToggleBookmark()
{
	if (CurrentDocId.IsEmpty())
	{
		return FReply::Handled();
	}

	if (Bookmarks.Contains(CurrentDocId))
	{
		Bookmarks.Remove(CurrentDocId);
	}
	else
	{
		Bookmarks.Add(CurrentDocId);
	}

	UpdateBookmarkButton();
	UpdateStatusBar();
	return FReply::Handled();
}

bool SDocWindow::IsCurrentDocBookmarked() const
{
	return !CurrentDocId.IsEmpty() && Bookmarks.Contains(CurrentDocId);
}

void SDocWindow::UpdateBookmarkButton()
{
	if (BookmarkToggleLabel.IsValid())
	{
		// EN: Filled star when bookmarked, empty star when not / ES: Estrella llena cuando bookmarked, vacia cuando no
		const FString StarText = IsCurrentDocBookmarked() ? TEXT("\u2605") : TEXT("\u2606");
		BookmarkToggleLabel->SetText(FText::FromString(StarText));
	}
}

TSharedRef<SWidget> SDocWindow::BuildBookmarkDropdownContent()
{
	if (Bookmarks.Num() == 0)
	{
		return SNew(SBox)
			.Padding(FMargin(12.0f, 8.0f))
			[
				SNew(STextBlock)
				.Text(LOCTEXT("NoBookmarks", "No bookmarked documents"))
				.Font(FCoreStyle::GetDefaultFontStyle("Italic", 10))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)))
			];
	}

	// EN: Build a vertical list of bookmark entries / ES: Construir una lista vertical de entradas de bookmark
	TSharedRef<SVerticalBox> ListBox = SNew(SVerticalBox);

	for (const FString& DocId : Bookmarks)
	{
		// EN: Use the last path component as display name / ES: Usar el ultimo componente del path como nombre de display
		FString DisplayName = FPaths::GetBaseFilename(DocId);
		if (DisplayName.IsEmpty())
		{
			DisplayName = DocId;
		}

		FString CapturedDocId = DocId;
		ListBox->AddSlot()
		.AutoHeight()
		[
			SNew(SButton)
			.ButtonStyle(FAppStyle::Get(), "SimpleButton")
			.OnClicked_Lambda([this, CapturedDocId]()
			{
				if (BookmarkMenuAnchor.IsValid())
				{
					BookmarkMenuAnchor->SetIsOpen(false);
				}
				NavigateToDoc(CapturedDocId);
				return FReply::Handled();
			})
			[
				SNew(SBox)
				.Padding(FMargin(8.0f, 4.0f))
				[
					SNew(STextBlock)
					.Text(FText::FromString(DisplayName))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
				]
			]
		];
	}

	return SNew(SBox)
		.MinDesiredWidth(200.0f)
		.MaxDesiredHeight(300.0f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				ListBox
			]
		];
}

EActiveTimerReturnType SDocWindow::OnSearchDebounce(double /*InCurrentTime*/, float /*InDeltaTime*/)
{
	if (PendingSearchQuery.Len() >= 2)
	{
		FDocSearchFilters Filters;
		TArray<FDocSearchResult> Results = FDocSystem::Get().Search(PendingSearchQuery, Filters);

		// EN: Do NOT auto-navigate — let user click explicitly
		// ES: NO auto-navegar — dejar que el usuario haga click explicitamente
		// NOTE: Results are displayed in search panel, navigation happens on user click
		// if (Results.Num() > 0)
		// {
		// 	NavigateToDoc(Results[0].DocId);
		// }
	}

	SearchDebounceHandle.Reset();
	return EActiveTimerReturnType::Stop;
}

#undef LOCTEXT_NAMESPACE
