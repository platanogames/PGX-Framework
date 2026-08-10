// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SDocTreeView;
class SDocContentView;
class SDocTopBar;
class SMenuAnchor;

/**
 * EN: Main PGX Documentation Viewer window widget.
 *     Layout: TopBar + HSplitter(TreeView | ContentView) + StatusBar.
 *     Orchestrates navigation between tree and content panels.
 *
 * ES: Widget principal de la ventana del Visor de Documentacion PGX.
 *     Layout: TopBar + HSplitter(TreeView | ContentView) + StatusBar.
 *     Orquesta la navegacion entre paneles de arbol y contenido.
 */
class SDocWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDocWindow) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SDocWindow() override;

	/** EN: Navigate to a specific document / ES: Navegar a un documento especifico */
	void NavigateToDoc(const FString& DocId);

	/** EN: Reload the current document / ES: Recargar el documento actual */
	void ReloadCurrentDoc();

private:
	// EN: Build the top toolbar section / ES: Construir la seccion de toolbar superior
	TSharedRef<SWidget> BuildTopBar();

	// EN: Build the status bar / ES: Construir la barra de estado
	TSharedRef<SWidget> BuildStatusBar();

	// EN: Callback when tree selection changes / ES: Callback cuando cambia la seleccion del arbol
	void OnTreeSelectionChanged(const FString& DocId);

	// EN: Callback when search result is clicked / ES: Callback cuando se hace click en resultado de busqueda
	void OnSearchResultClicked(const FString& DocId);

	// EN: Callback when docs system changes / ES: Callback cuando cambia el sistema de docs
	void OnDocsChanged();

	// EN: Refresh status bar text / ES: Refrescar texto de la barra de estado
	void UpdateStatusBar();

	// EN: Handle reload button / ES: Manejar boton de recargar
	FReply OnReloadClicked();

	// EN: Handle validate button / ES: Manejar boton de validar
	FReply OnValidateClicked();

	// -- Bookmarks --

	/** EN: Toggle bookmark for current document / ES: Toggle bookmark del documento actual */
	FReply OnToggleBookmark();

	/** EN: Check if the current document is bookmarked / ES: Verificar si el documento actual esta en bookmarks */
	bool IsCurrentDocBookmarked() const;

	/** EN: Update the bookmark toggle button visual state / ES: Actualizar estado visual del boton toggle bookmark */
	void UpdateBookmarkButton();

	/** EN: Build the bookmark dropdown content / ES: Construir el contenido del dropdown de bookmarks */
	TSharedRef<SWidget> BuildBookmarkDropdownContent();

	TSharedPtr<SDocTreeView> TreeView;
	TSharedPtr<SDocContentView> ContentView;
	TSharedPtr<STextBlock> StatusBarText;

	FString CurrentDocId;
	FDelegateHandle DocsChangedHandle;

	/** EN: Re-entrancy guard — prevents NavigateToDoc recursion loop / ES: Guard de re-entrancia — previene loop de recursion en NavigateToDoc */
	bool bIsNavigating = false;

	/** EN: Scroll position memory per document / ES: Memoria de posicion de scroll por documento */
	TMap<FString, float> DocScrollPositions;

	/** EN: Session bookmarks (document IDs) / ES: Bookmarks de sesion (IDs de documentos) */
	TSet<FString> Bookmarks;

	/** EN: Bookmark toggle button — updated when navigation or toggle happens / ES: Boton toggle bookmark — actualizado al navegar o hacer toggle */
	TSharedPtr<STextBlock> BookmarkToggleLabel;

	/** EN: Menu anchor for bookmark dropdown / ES: Menu anchor para dropdown de bookmarks */
	TSharedPtr<SMenuAnchor> BookmarkMenuAnchor;

	/** EN: Debounce active timer handle for search / ES: Handle de active timer debounce para busqueda */
	TSharedPtr<FActiveTimerHandle> SearchDebounceHandle;

	/** EN: Pending search query / ES: Consulta de busqueda pendiente */
	FString PendingSearchQuery;

	/** EN: Active timer callback for search debounce / ES: Callback de active timer para debounce de busqueda */
	EActiveTimerReturnType OnSearchDebounce(double InCurrentTime, float InDeltaTime);
};
