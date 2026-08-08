// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "PGXDocTypes.h"

class UPGXDocsConfig;
class SScrollBox;
class FSlateStyleSet;

/**
 * EN: Native Slate renderer for parsed Markdown documents.
 *     Receives an FDocElement tree (parsed by FDocSystem::ParseMarkdownToTree)
 *     and builds a scrollable widget hierarchy using SRichTextBlock, STextBlock,
 *     SBorder, SGridPanel, etc.
 *
 * ES: Renderer Slate nativo para documentos Markdown parseados.
 *     Recibe un arbol FDocElement (parseado por FDocSystem::ParseMarkdownToTree)
 *     y construye una jerarquia de widgets scrollable usando SRichTextBlock,
 *     STextBlock, SBorder, SGridPanel, etc.
 */
class SDocSlateRenderer : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDocSlateRenderer) {}
		SLATE_ARGUMENT(const UPGXDocsConfig*, Config)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** EN: Set the content to render / ES: Establecer el contenido a renderizar */
	void SetContent(const TArray<FDocElement>& Elements, const FString& DocTitle);

	/** EN: Clear all rendered content / ES: Limpiar todo el contenido renderizado */
	void Clear();

	/** EN: Scroll to the top of the document / ES: Scrollear al inicio del documento */
	void ScrollToTop();

	/** EN: Get current scroll offset for position memory / ES: Obtener offset de scroll actual para memoria de posicion */
	float GetScrollOffset() const;

	/** EN: Restore scroll offset / ES: Restaurar offset de scroll */
	void SetScrollOffset(float Offset);

	/** EN: Extract TOC from the current document {heading text, level} / ES: Extraer TOC del documento actual {texto heading, nivel} */
	TArray<TPair<FString, int32>> GetTOC() const;

	// EN: Delegate fired when a doc link is clicked / ES: Delegado disparado cuando se hace click en un link de doc
	DECLARE_DELEGATE_OneParam(FOnDocLinkClicked, const FString& /*Url*/);
	FOnDocLinkClicked OnLinkClicked;

private:
	// EN: Build all widgets from the element tree / ES: Construir todos los widgets del arbol de elementos
	void BuildWidgets(const TArray<FDocElement>& Elements);

	// EN: Build a single block element widget / ES: Construir un widget de un solo elemento bloque
	TSharedRef<SWidget> BuildBlock(const FDocElement& Element);

	// EN: Per-type builders / ES: Builders por tipo
	TSharedRef<SWidget> BuildParagraph(const FDocElement& Element);
	TSharedRef<SWidget> BuildHeading(const FDocElement& Element);
	TSharedRef<SWidget> BuildCodeBlock(const FDocElement& Element);
	TSharedRef<SWidget> BuildBlockquote(const FDocElement& Element);
	TSharedRef<SWidget> BuildHorizontalRule();
	TSharedRef<SWidget> BuildList(const FDocElement& Element);
	TSharedRef<SWidget> BuildListItem(const FDocElement& Element, bool bOrdered, int32 Index);
	TSharedRef<SWidget> BuildTable(const FDocElement& Element);

	// EN: Convert inline runs to SRichTextBlock-compatible decorated text
	// ES: Convertir runs inline a texto decorado compatible con SRichTextBlock
	FString RunsToRichText(const TArray<FDocTextRun>& Runs) const;

	// EN: Build a simple text widget from runs (for cells, list items with simple text)
	// ES: Construir un widget de texto simple desde runs (para celdas, items de lista con texto simple)
	TSharedRef<SWidget> BuildInlineText(const TArray<FDocTextRun>& Runs) const;

	// EN: Get the configured or default font size / ES: Obtener el tamano de fuente configurado o por defecto
	int32 GetBaseFontSize() const;

	// EN: Handle hyperlink click / ES: Manejar click de hyperlink
	void HandleHyperlinkClick(const TMap<FString, FString>& Metadata) const;

	// EN: Initialize the rich text style set / ES: Inicializar el style set de rich text
	void InitStyleSet();

	TSharedPtr<SScrollBox> ScrollBox;
	const UPGXDocsConfig* CachedConfig = nullptr;
	TSharedPtr<FSlateStyleSet> DocStyleSet;

	// EN: Cached headings for TOC extraction / ES: Headings cacheados para extraccion TOC
	TArray<TPair<FString, int32>> CachedHeadings;
};
