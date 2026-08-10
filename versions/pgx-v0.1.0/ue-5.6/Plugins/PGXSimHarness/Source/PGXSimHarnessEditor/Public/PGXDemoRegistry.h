// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * EN: A single demo DataAsset entry with metadata for ordered, categorized creation.
 * ES: Una entrada demo DataAsset con metadata para creacion ordenada y categorizada.
 */
struct PGXSIMHARNESSEDITOR_API FPGXDemoEntry
{
	/** EN: Display name shown in menu / ES: Nombre mostrado en menu */
	FString DisplayName;

	/** EN: UClass path for NewObject creation / ES: Path de UClass para creacion NewObject */
	FString ClassPath;

	/** EN: Default asset name suggestion / ES: Nombre de asset sugerido por defecto */
	FString SuggestedName;

	/** EN: Numbered category ("1. Foundation", "2. Game State", etc.) / ES: Categoria numerada */
	FString Category;

	/** EN: Sort order within category / ES: Orden dentro de la categoria */
	int32 OrderInCategory = 0;

	/** EN: Tooltip describing the purpose / ES: Tooltip describiendo el proposito */
	FString Tooltip;

	/** EN: Icon style name (PGX or UE built-in) / ES: Nombre de estilo de icono */
	FString IconStyleName;
};

/**
 * EN: Registry of demo DataAsset entries organized in ordered logical categories.
 *     Each entry represents a pre-configured DA that can be created with educational example values.
 *
 * ES: Registro de entradas demo DataAsset organizadas en categorias logicas ordenadas.
 *     Cada entrada representa un DA pre-configurado que se puede crear con valores ejemplo educativos.
 */
class PGXSIMHARNESSEDITOR_API FPGXDemoRegistry
{
public:
	/** EN: Get all demo entries / ES: Obtener todas las entradas demo */
	static const TArray<FPGXDemoEntry>& GetDemoEntries();

	/** EN: Get ordered list of category names / ES: Obtener lista ordenada de nombres de categoria */
	static TArray<FString> GetDemoCategories();

	/** EN: Get entries for a specific category / ES: Obtener entradas para una categoria especifica */
	static TArray<FPGXDemoEntry> GetEntriesForCategory(const FString& Category);
};
