// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "PGXDocTypes.h"

/**
 * EN: Search panel with results list for documentation.
 *     Integrated into the top bar as an expandable panel.
 *
 * ES: Panel de busqueda con lista de resultados para documentacion.
 *     Integrado en la barra superior como panel expandible.
 */
class SDocSearchPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDocSearchPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** EN: Set the search results to display / ES: Establecer los resultados de busqueda a mostrar */
	void SetResults(const TArray<FDocSearchResult>& Results);

	/** EN: Clear results / ES: Limpiar resultados */
	void ClearResults();

private:
	TSharedRef<ITableRow> OnGenerateResultRow(TSharedPtr<FDocSearchResult> Item, const TSharedRef<STableViewBase>& OwnerTable);

	TArray<TSharedPtr<FDocSearchResult>> ResultItems;
	TSharedPtr<SListView<TSharedPtr<FDocSearchResult>>> ResultsList;
};
