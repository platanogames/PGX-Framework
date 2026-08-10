// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXDocTypes.h"

class UPGXDocsConfig;

/**
 * EN: Metadata for a documentation root directory.
 * ES: Metadata para un directorio raiz de documentacion.
 */
struct FDocRootInfo
{
	/** EN: Absolute filesystem path / ES: Ruta absoluta en filesystem */
	FString Path;

	/** EN: Lowercase ID used as DocId prefix (e.g. "pgxaudio") / ES: ID en minusculas usado como prefijo de DocId (e.g. "pgxaudio") */
	FString ModuleId;

	/** EN: Display name for tree UI (e.g. "PGXAudio") / ES: Nombre para mostrar en el arbol UI (e.g. "PGXAudio") */
	FString DisplayName;
};

/**
 * EN: Scans documentation roots, builds the navigation tree, and maintains
 *     bidirectional DocId <-> AbsPath mapping. Supports optional manifests.
 *
 * ES: Escanea raices de documentacion, construye el arbol de navegacion,
 *     y mantiene mapeo bidireccional DocId <-> AbsPath. Soporta manifests opcionales.
 */
class FDocRegistry
{
public:
	/**
	 * EN: Initialize the registry — scan all configured roots.
	 * ES: Inicializar el registry — escanear todas las raices configuradas.
	 */
	void Initialize(const UPGXDocsConfig* Config);

	/**
	 * EN: Rebuild tree from scratch. Called when roots change or on explicit reload.
	 * ES: Reconstruir arbol desde cero. Llamado cuando cambian raices o en reload explicito.
	 */
	void Rebuild();

	/** EN: Get the root navigation tree / ES: Obtener el arbol de navegacion raiz */
	const TArray<TSharedPtr<FDocNode>>& GetRootTree() const { return RootNodes; }

	/** EN: Resolve a DocId to absolute file path / ES: Resolver un DocId a ruta absoluta */
	FString ResolveDocId(const FString& DocId) const;

	/** EN: Get DocId from absolute file path / ES: Obtener DocId de ruta absoluta */
	FString GetDocIdForPath(const FString& AbsPath) const;

	/** EN: Get all registered DocIds / ES: Obtener todos los DocIds registrados */
	TArray<FString> GetAllDocIds() const;

	/** EN: Reload a specific document by path / ES: Recargar un documento especifico por ruta */
	void ReloadDocument(const FString& AbsPath);

	/** EN: Get number of registered documents / ES: Obtener numero de documentos registrados */
	int32 GetDocCount() const { return DocIdToPath.Num(); }

	/** EN: Get all doc root directory paths (for file watcher) / ES: Obtener todas las rutas de directorios raiz (para file watcher) */
	TArray<FString> GetDocRootPaths() const
	{
		TArray<FString> Paths;
		Paths.Reserve(DocRoots.Num());
		for (const FDocRootInfo& Root : DocRoots)
		{
			Paths.Add(Root.Path);
		}
		return Paths;
	}

private:
	// EN: Scan a single root directory for .md files / ES: Escanear un directorio raiz para archivos .md
	void ScanRoot(const FDocRootInfo& RootInfo);

	// EN: Build tree nodes from scanned file list / ES: Construir nodos del arbol desde lista de archivos escaneados
	TSharedPtr<FDocNode> BuildTreeFromPath(const FString& RelativePath, const FString& DocId);

	// EN: Parse a module.manifest.json file / ES: Parsear un archivo module.manifest.json
	void ParseManifest(const FString& ManifestPath, const FString& RootPath);

	// EN: Check if a file should be excluded by config patterns / ES: Verificar si un archivo debe excluirse por patrones de config
	bool ShouldExclude(const FString& FilePath) const;

	// EN: Sort children of all nodes by order then alphabetically / ES: Ordenar hijos de todos los nodos por orden y luego alfabeticamente
	void SortTree(TArray<TSharedPtr<FDocNode>>& Nodes);

	/** EN: All configured root directories with metadata / ES: Todos los directorios raiz configurados con metadata */
	TArray<FDocRootInfo> DocRoots;

	/** EN: DocId -> Absolute path / ES: DocId -> Ruta absoluta */
	TMap<FString, FString> DocIdToPath;

	/** EN: Absolute path -> DocId / ES: Ruta absoluta -> DocId */
	TMap<FString, FString> PathToDocId;

	/** EN: Root tree nodes / ES: Nodos raiz del arbol */
	TArray<TSharedPtr<FDocNode>> RootNodes;

	/** EN: Cached config reference / ES: Referencia de config cacheada */
	const UPGXDocsConfig* CachedConfig = nullptr;

	/** EN: Exclude patterns from config / ES: Patrones de exclusion de config */
	TArray<FString> ExcludePatterns;
};
