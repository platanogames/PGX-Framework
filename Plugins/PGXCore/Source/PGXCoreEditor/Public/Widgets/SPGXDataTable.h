// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Standardized header row for data tables — renders column labels with token styling.
// ES: Fila de encabezado estandarizada para tablas de datos — renderiza labels de columna con tokens.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Style/PGXVisualTokens.h"

/**
 * EN: Column definition for PGX data tables.
 * ES: Definicion de columna para tablas de datos PGX.
 */
struct PGXCOREEDITOR_API FPGXColumnDef
{
	FName Id;
	FText Label;
	float Width = 0.0f;  // 0 = auto
	EHorizontalAlignment HAlign = HAlign_Left;
};

/**
 * EN: Data table header row — renders column definitions with consistent typography.
 *     Pair with SListView below for the data rows.
 * ES: Fila de encabezado de tabla de datos — renderiza definiciones de columna con tipografia consistente.
 *     Combinar con SListView debajo para las filas de datos.
 */
class PGXCOREEDITOR_API SPGXDataTableHeader : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXDataTableHeader) {}
		SLATE_ARGUMENT(TArray<FPGXColumnDef>, Columns)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};
