// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXDocTypes.h"
#include "PGXDocRegistry.h"
#include "PGXDocSearchIndex.h"
#include "PGXDocValidator.h"
#include "PGXDocLinkResolver.h"

class UPGXDocsConfig;

/**
 * EN: Central orchestrator for the PGX Documentation system.
 *     Owns all services: Registry, SearchIndex, Validator, LinkResolver.
 *     Provides the main API consumed by the Editor UI.
 *
 * ES: Orquestador central del sistema de Documentacion PGX.
 *     Posee todos los servicios: Registry, SearchIndex, Validator, LinkResolver.
 *     Proporciona la API principal consumida por la UI del Editor.
 */
class PGXDOCS_API FDocSystem
{
public:
	/** EN: Get the singleton instance / ES: Obtener la instancia singleton */
	static FDocSystem& Get();

	/** EN: Initialize with optional config DA / ES: Inicializar con config DA opcional */
	void Initialize(const UPGXDocsConfig* Config = nullptr);

	/** EN: Shutdown and clean up / ES: Apagar y limpiar */
	void Shutdown();

	/** EN: Whether the system is initialized / ES: Si el sistema esta inicializado */
	bool IsInitialized() const { return bIsInitialized; }

	// ============================================================
	// Registry API
	// ============================================================

	/** EN: Get the root navigation tree / ES: Obtener el arbol de navegacion raiz */
	const TArray<TSharedPtr<FDocNode>>& GetRootTree() const { return Registry.GetRootTree(); }

	/** EN: Get total document count / ES: Obtener conteo total de documentos */
	int32 GetDocCount() const { return Registry.GetDocCount(); }

	/** EN: Rebuild the registry (full rescan) / ES: Reconstruir el registry (rescan completo) */
	void RebuildRegistry();

	// ============================================================
	// Document Loading
	// ============================================================

	/**
	 * EN: Load a document by DocId. Returns cached if available, otherwise reads from disk.
	 * ES: Cargar un documento por DocId. Retorna cacheado si disponible, si no lee de disco.
	 */
	TSharedPtr<FDocDocument> LoadDocument(const FString& DocId);

	// ============================================================
	// Search API
	// ============================================================

	/** EN: Search documents / ES: Buscar documentos */
	TArray<FDocSearchResult> Search(const FString& Query, const FDocSearchFilters& Filters) const;

	/** EN: Rebuild the search index from all docs / ES: Reconstruir el indice de busqueda de todos los docs */
	void RebuildSearchIndex();

	// ============================================================
	// Markdown Rendering API
	// ============================================================

	/**
	 * EN: Render Markdown to HTML using md4c. Wrapper around the embedded C library.
	 *     Called from PGXDocsEditor to avoid cross-DLL md4c symbol resolution.
	 * ES: Renderizar Markdown a HTML usando md4c. Wrapper alrededor de la libreria C embebida.
	 *     Llamado desde PGXDocsEditor para evitar resolucion de simbolos md4c cross-DLL.
	 */
	static FString RenderMarkdownToHtml(const FString& Markdown);

	/**
	 * EN: Parse Markdown to FDocElement tree using md4c callback API.
	 *     Returns an array of root-level block elements. Each element may contain
	 *     inline text runs (FDocTextRun) and/or nested child elements.
	 *     This is the cross-DLL bridge: md4c lives in PGXDocs, rendering in PGXDocsEditor.
	 * ES: Parsear Markdown a arbol FDocElement usando la API de callbacks md4c.
	 *     Retorna array de elementos bloque de nivel raiz. Cada elemento puede contener
	 *     runs de texto inline (FDocTextRun) y/o elementos hijos anidados.
	 *     Este es el bridge cross-DLL: md4c vive en PGXDocs, rendering en PGXDocsEditor.
	 */
	static TArray<FDocElement> ParseMarkdownToTree(const FString& Markdown);

	// ============================================================
	// Validation API
	// ============================================================

	/** EN: Validate all documentation / ES: Validar toda la documentacion */
	FDocValidationReport ValidateAll() const;

	// ============================================================
	// Link Resolver API
	// ============================================================

	/** EN: Get the link resolver / ES: Obtener el resolutor de links */
	const FDocLinkResolver& GetLinkResolver() const { return LinkResolver; }

	/** EN: Get the registry / ES: Obtener el registry */
	const FDocRegistry& GetRegistry() const { return Registry; }

	// ============================================================
	// File Change Notification
	// ============================================================

	/** EN: Called when files change on disk / ES: Llamado cuando cambian archivos en disco */
	void OnFilesChanged(const TArray<FString>& ChangedPaths);

	/** EN: Delegate broadcast when docs change / ES: Delegado broadcast cuando cambian docs */
	DECLARE_MULTICAST_DELEGATE(FOnDocsChanged);
	FOnDocsChanged OnDocsChanged;

private:
	FDocSystem() = default;

	// EN: Parse YAML frontmatter from raw markdown / ES: Parsear frontmatter YAML del markdown crudo
	void ParseFrontMatter(FDocDocument& Document) const;

	// EN: Extract title from first H1 heading / ES: Extraer titulo del primer heading H1
	FString ExtractTitle(const FString& Markdown) const;

	bool bIsInitialized = false;
	const UPGXDocsConfig* CachedConfig = nullptr;

	/** EN: Cached at Initialize() — safe to use during Shutdown() when UObjects are gone / ES: Cacheado en Initialize() — seguro usar durante Shutdown() cuando UObjects ya no existen */
	FString CachedIndexCachePath;

	FDocRegistry Registry;
	FDocSearchIndex SearchIndex;
	FDocValidator Validator;
	FDocLinkResolver LinkResolver;

	/** EN: LRU document cache / ES: Cache LRU de documentos */
	TMap<FString, TSharedPtr<FDocDocument>> DocumentCache;
	static constexpr int32 MaxCacheSize = 50;
};
