// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXDocRegistry.h"
#include "Logging/PGXLogMacros.h"
#include "PGXDocsModule.h"
#include "PGXDocsConfig.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

void FDocRegistry::Initialize(const UPGXDocsConfig* Config)
{
	CachedConfig = Config;

	// EN: Collect exclude patterns from config / ES: Recolectar patrones de exclusion de config
	ExcludePatterns.Empty();
	if (Config)
	{
		ExcludePatterns = Config->ExcludePatterns;
	}

	Rebuild();
}

void FDocRegistry::Rebuild()
{
	DocIdToPath.Empty();
	PathToDocId.Empty();
	RootNodes.Empty();
	DocRoots.Empty();

	const FString ProjectDir = FPaths::ProjectDir();

	// EN: Add default roots based on config (or safe defaults if no config)
	// ES: Agregar raices por defecto basadas en config (o defaults seguros si no hay config)
	bool bIncludeDocs = true;
	bool bIncludePluginDocs = true;

	if (CachedConfig)
	{
		bIncludeDocs = CachedConfig->bIncludeProjectDocs;
		bIncludePluginDocs = CachedConfig->bIncludePluginDocs;
	}

	if (bIncludeDocs)
	{
		const FString DocsRoot = FPaths::Combine(ProjectDir, TEXT("Docs"));
		if (FPaths::DirectoryExists(DocsRoot))
		{
			FDocRootInfo Info;
			Info.Path = DocsRoot;
			Info.ModuleId = TEXT("docs");
			Info.DisplayName = TEXT("Docs");
			DocRoots.Add(Info);
		}
	}


	if (bIncludePluginDocs)
	{
		// EN: Scan plugin directories for Docs/ subfolders — use plugin folder name as display name
		// ES: Escanear directorios de plugins para subcarpetas Docs/ — usar nombre de carpeta del plugin como display name
		const FString PluginsRoot = FPaths::Combine(ProjectDir, TEXT("Plugins"));
		TArray<FString> PluginDirs;
		IFileManager::Get().FindFiles(PluginDirs, *FPaths::Combine(PluginsRoot, TEXT("*")), false, true);
		for (const FString& PluginDir : PluginDirs)
		{
			const FString PluginDocsPath = FPaths::Combine(PluginsRoot, PluginDir, TEXT("Docs"));
			if (FPaths::DirectoryExists(PluginDocsPath))
			{
				FDocRootInfo Info;
				Info.Path = PluginDocsPath;
				Info.ModuleId = PluginDir.ToLower();
				Info.DisplayName = PluginDir;
				DocRoots.Add(Info);
			}
		}
	}


	// EN: Add additional roots from config / ES: Agregar raices adicionales de config
	if (CachedConfig)
	{
		for (const FDirectoryPath& AdditionalRoot : CachedConfig->AdditionalDocRoots)
		{
			if (!AdditionalRoot.Path.IsEmpty() && FPaths::DirectoryExists(AdditionalRoot.Path))
			{
				FDocRootInfo Info;
				Info.Path = AdditionalRoot.Path;
				Info.ModuleId = FPaths::GetCleanFilename(AdditionalRoot.Path).ToLower();
				Info.DisplayName = FPaths::GetCleanFilename(AdditionalRoot.Path);
				DocRoots.Add(Info);
			}
		}
	}

	// EN: Scan each root / ES: Escanear cada raiz
	for (const FDocRootInfo& RootInfo : DocRoots)
	{
		// EN: Check for manifest / ES: Verificar manifest
		const FString ManifestPath = FPaths::Combine(RootInfo.Path, TEXT("module.manifest.json"));
		if (FPaths::FileExists(ManifestPath))
		{
			ParseManifest(ManifestPath, RootInfo.Path);
		}

		ScanRoot(RootInfo);
	}

	// EN: Sort the tree / ES: Ordenar el arbol
	SortTree(RootNodes);

	PGX_LOG_INFO(LogPGXDocs, TEXT("PGXDocs: Registry rebuilt — %d documents across %d roots"), DocIdToPath.Num(), DocRoots.Num());
}

void FDocRegistry::ScanRoot(const FDocRootInfo& RootInfo)
{
	const FString& RootPath = RootInfo.Path;
	const FString& ModuleId = RootInfo.ModuleId;

	// EN: Create root node for this directory — use DisplayName instead of folder name
	// ES: Crear nodo raiz para este directorio — usar DisplayName en vez del nombre de carpeta
	TSharedPtr<FDocNode> RootNode = MakeShared<FDocNode>();
	RootNode->Id = ModuleId;
	RootNode->Title = RootInfo.DisplayName;
	RootNode->bIsExpanded = true;

	// EN: Recursively scan for .md files / ES: Escanear recursivamente archivos .md
	TArray<FString> FoundFiles;
	IFileManager::Get().FindFilesRecursive(FoundFiles, *RootPath, TEXT("*.md"), true, false);

	// EN: Map to hold intermediate folder nodes / ES: Map para nodos intermedios de carpetas
	TMap<FString, TSharedPtr<FDocNode>> FolderNodes;

	for (const FString& FilePath : FoundFiles)
	{
		if (ShouldExclude(FilePath))
		{
			continue;
		}

		// EN: Compute relative path and DocId / ES: Calcular ruta relativa y DocId
		FString RelativePath = FilePath;
		FPaths::MakePathRelativeTo(RelativePath, *FPaths::Combine(RootPath, TEXT("")));
		RelativePath.ReplaceInline(TEXT("\\"), TEXT("/"));

		// EN: Use full relative path without .md extension to preserve directory hierarchy
		// ES: Usar ruta relativa completa sin extension .md para preservar jerarquia de directorios
		FString DocIdPath = RelativePath;
		if (DocIdPath.EndsWith(TEXT(".md")))
		{
			DocIdPath.LeftChopInline(3);
		}
		const FString DocId = FString::Printf(TEXT("%s/%s"), *ModuleId, *DocIdPath);

		// EN: Skip if already registered (manifest may have added it) / ES: Saltar si ya esta registrado
		if (DocIdToPath.Contains(DocId))
		{
			continue;
		}

		// EN: Register DocId <-> Path / ES: Registrar DocId <-> Path
		const FString NormalizedPath = FPaths::ConvertRelativePathToFull(FilePath);
		DocIdToPath.Add(DocId, NormalizedPath);
		PathToDocId.Add(NormalizedPath, DocId);

		// EN: Build tree hierarchy from relative path / ES: Construir jerarquia de arbol desde ruta relativa
		TArray<FString> PathParts;
		RelativePath.ParseIntoArray(PathParts, TEXT("/"));

		TSharedPtr<FDocNode> ParentNode = RootNode;

		// EN: Create intermediate folder nodes / ES: Crear nodos intermedios de carpetas
		FString CurrentPath = ModuleId;
		for (int32 i = 0; i < PathParts.Num() - 1; ++i)
		{
			CurrentPath = FString::Printf(TEXT("%s/%s"), *CurrentPath, *PathParts[i]);

			TSharedPtr<FDocNode>* ExistingFolder = FolderNodes.Find(CurrentPath);
			if (ExistingFolder)
			{
				ParentNode = *ExistingFolder;
			}
			else
			{
				TSharedPtr<FDocNode> FolderNode = MakeShared<FDocNode>();
				FolderNode->Id = CurrentPath;
				FolderNode->Title = PathParts[i];
				ParentNode->Children.Add(FolderNode);
				FolderNodes.Add(CurrentPath, FolderNode);
				ParentNode = FolderNode;
			}
		}

		// EN: Create leaf node for the .md file / ES: Crear nodo hoja para el archivo .md
		TSharedPtr<FDocNode> LeafNode = MakeShared<FDocNode>();
		LeafNode->Id = DocId;
		LeafNode->DocId = DocId;

		// EN: Extract display title from filename (strip numeric prefix)
		// ES: Extraer titulo para mostrar del nombre de archivo (quitar prefijo numerico)
		FString FileName = FPaths::GetBaseFilename(FilePath);
		// EN: Strip "00_", "01_" style prefixes / ES: Quitar prefijos estilo "00_", "01_"
		if (FileName.Len() > 3 && FChar::IsDigit(FileName[0]) && FChar::IsDigit(FileName[1]) && FileName[2] == TEXT('_'))
		{
			// EN: Extract order from prefix / ES: Extraer orden del prefijo
			LeafNode->Order = FCString::Atoi(*FileName.Left(2));
			FileName = FileName.Mid(3);
		}
		LeafNode->Title = FileName.Replace(TEXT("_"), TEXT(" "));

		ParentNode->Children.Add(LeafNode);
	}

	RootNodes.Add(RootNode);
}

void FDocRegistry::ParseManifest(const FString& ManifestPath, const FString& RootPath)
{
	FString JsonStr;
	if (!FFileHelper::LoadFileToString(JsonStr, *ManifestPath))
	{
		return;
	}

	TSharedPtr<FJsonObject> JsonObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonStr);
	if (!FJsonSerializer::Deserialize(Reader, JsonObj) || !JsonObj.IsValid())
	{
		PGX_LOG_WARNING(LogPGXDocs, TEXT("PGXDocs: Failed to parse manifest: %s"), *ManifestPath);
		return;
	}

	const FString ManifestModuleId = JsonObj->GetStringField(TEXT("module_id"));
	const TArray<TSharedPtr<FJsonValue>>* Chapters = nullptr;

	if (JsonObj->TryGetArrayField(TEXT("chapters"), Chapters))
	{
		for (const TSharedPtr<FJsonValue>& ChapterVal : *Chapters)
		{
			const TSharedPtr<FJsonObject>& Chapter = ChapterVal->AsObject();
			if (!Chapter.IsValid()) continue;

			const FString ChapterId = Chapter->GetStringField(TEXT("id"));
			const FString ChapterFile = Chapter->GetStringField(TEXT("file"));
			const FString AbsFilePath = FPaths::ConvertRelativePathToFull(FPaths::Combine(RootPath, ChapterFile));

			if (FPaths::FileExists(AbsFilePath))
			{
				const FString DocId = FString::Printf(TEXT("%s/%s"), *ManifestModuleId, *ChapterId);
				DocIdToPath.Add(DocId, AbsFilePath);
				PathToDocId.Add(AbsFilePath, DocId);
			}
		}
	}
}

FString FDocRegistry::ResolveDocId(const FString& DocId) const
{
	const FString* Path = DocIdToPath.Find(DocId);
	return Path ? *Path : FString();
}

FString FDocRegistry::GetDocIdForPath(const FString& AbsPath) const
{
	const FString NormalizedPath = FPaths::ConvertRelativePathToFull(AbsPath);
	const FString* DocId = PathToDocId.Find(NormalizedPath);
	return DocId ? *DocId : FString();
}

TArray<FString> FDocRegistry::GetAllDocIds() const
{
	TArray<FString> Result;
	DocIdToPath.GetKeys(Result);
	return Result;
}

void FDocRegistry::ReloadDocument(const FString& AbsPath)
{
	// EN: Re-scan the document — the caller should also update the search index
	// ES: Re-escanear el documento — el llamante tambien debe actualizar el indice de busqueda
	const FString NormalizedPath = FPaths::ConvertRelativePathToFull(AbsPath);
	if (!PathToDocId.Contains(NormalizedPath))
	{
		// EN: New file — trigger full rebuild / ES: Archivo nuevo — triggear rebuild completo
		Rebuild();
	}
}

bool FDocRegistry::ShouldExclude(const FString& FilePath) const
{
	const FString FileName = FPaths::GetCleanFilename(FilePath);

	for (const FString& Pattern : ExcludePatterns)
	{
		if (FileName.MatchesWildcard(Pattern))
		{
			return true;
		}
	}

	// EN: Check excluded folders / ES: Verificar carpetas excluidas
	if (CachedConfig)
	{
		for (const FDirectoryPath& ExcludedFolder : CachedConfig->ExcludedFolders)
		{
			if (!ExcludedFolder.Path.IsEmpty() && FilePath.Contains(ExcludedFolder.Path))
			{
				return true;
			}
		}
	}

	return false;
}

void FDocRegistry::SortTree(TArray<TSharedPtr<FDocNode>>& Nodes)
{
	Nodes.Sort([](const TSharedPtr<FDocNode>& A, const TSharedPtr<FDocNode>& B)
	{
		// EN: Folders before files / ES: Carpetas antes que archivos
		const bool bAIsFolder = A->DocId.IsEmpty();
		const bool bBIsFolder = B->DocId.IsEmpty();
		if (bAIsFolder != bBIsFolder)
		{
			return bAIsFolder;
		}
		// EN: By order, then alphabetical / ES: Por orden, luego alfabetico
		if (A->Order != B->Order)
		{
			return A->Order < B->Order;
		}
		return A->Title < B->Title;
	});

	for (TSharedPtr<FDocNode>& Node : Nodes)
	{
		if (Node->Children.Num() > 0)
		{
			SortTree(Node->Children);
		}
	}
}
