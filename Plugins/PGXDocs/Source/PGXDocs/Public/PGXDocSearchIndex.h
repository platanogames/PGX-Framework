// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXDocTypes.h"

/**
 * EN: In-memory search index for documentation. Strips markdown to plain text,
 *     extracts headings, and supports ranked search with title/heading/body weighting.
 *     Supports incremental rebuild via ContentHash comparison.
 *
 * ES: Indice de busqueda en memoria para documentacion. Quita markdown a texto plano,
 *     extrae headings, y soporta busqueda rankeada con ponderacion titulo/heading/body.
 *     Soporta rebuild incremental via comparacion de ContentHash.
 */
class FDocSearchIndex
{
public:
	/** EN: Index a single document / ES: Indexar un solo documento */
	void IndexDocument(const FDocDocument& Document);

	/** EN: Remove a document from the index / ES: Remover un documento del indice */
	void RemoveDocument(const FString& DocId);

	/** EN: Clear the entire index / ES: Limpiar todo el indice */
	void Clear();

	/**
	 * EN: Search the index. Returns ranked results.
	 * ES: Buscar en el indice. Retorna resultados rankeados.
	 */
	TArray<FDocSearchResult> Search(const FString& Query, const FDocSearchFilters& Filters, int32 MaxResults = 25) const;

	/** EN: Get number of indexed documents / ES: Obtener numero de documentos indexados */
	int32 GetIndexedCount() const { return IndexEntries.Num(); }

	/** EN: Save index cache to JSON file / ES: Guardar cache de indice a archivo JSON */
	bool SaveCache(const FString& CachePath) const;

	/** EN: Load index cache from JSON file / ES: Cargar cache de indice de archivo JSON */
	bool LoadCache(const FString& CachePath);

private:
	// EN: Internal index entry / ES: Entrada interna de indice
	struct FIndexEntry
	{
		FString DocId;
		FString Title;
		FString ModuleId;
		FString PlainText;
		TArray<FString> Headings;
		TArray<FString> Tags;
		uint64 ContentHash = 0;
		int64 ModifiedTimeUtc = 0;
	};

	/** EN: Strip markdown formatting to plain text / ES: Quitar formato markdown a texto plano */
	static FString StripMarkdown(const FString& Markdown);

	/** EN: Extract headings from markdown / ES: Extraer headings del markdown */
	static TArray<FString> ExtractHeadings(const FString& Markdown);

	/** EN: Compute a hash for content comparison / ES: Computar un hash para comparacion de contenido */
	static uint64 ComputeHash(const FString& Content);

	/** EN: All indexed entries / ES: Todas las entradas indexadas */
	TMap<FString, FIndexEntry> IndexEntries;
};
