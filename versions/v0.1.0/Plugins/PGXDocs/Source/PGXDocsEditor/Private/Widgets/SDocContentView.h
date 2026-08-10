// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SDocSlateRenderer;

/**
 * EN: Right content panel that renders documentation using native Slate widgets.
 *     Delegates to SDocSlateRenderer for all visual rendering.
 *     Handles document loading, link navigation, and welcome page display.
 *
 * ES: Panel de contenido derecho que renderiza documentacion usando widgets Slate nativos.
 *     Delega a SDocSlateRenderer para todo el rendering visual.
 *     Maneja carga de documentos, navegacion de links y display de welcome page.
 */
class SDocContentView : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDocContentView) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** EN: Load and display a document by DocId / ES: Cargar y mostrar un documento por DocId */
	void LoadDocument(const FString& DocId);

	/** EN: Clear the content view / ES: Limpiar la vista de contenido */
	void Clear();

	/** EN: Get the Slate renderer for scroll position access / ES: Obtener el renderer Slate para acceso a posicion de scroll */
	TSharedPtr<SDocSlateRenderer> GetRenderer() const { return SlateRenderer; }

private:
	// EN: Handle link clicks from the renderer / ES: Manejar clicks de links del renderer
	void HandleLinkClicked(const FString& Url);

	TSharedPtr<SDocSlateRenderer> SlateRenderer;
	FString CurrentDocId;
};
