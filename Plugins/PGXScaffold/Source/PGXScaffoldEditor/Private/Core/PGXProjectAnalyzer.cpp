// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Core/PGXProjectAnalyzer.h"
#include "PGXScaffoldEditor.h"
#include "Logging/PGXLogMacros.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/App.h"

FPGXScaffoldProjectInfo FPGXProjectAnalyzer::Analyze()
{
	if (bCacheValid)
	{
		return CachedInfo;
	}

	PGX_LOG_INFO(LogPGXScaffold, TEXT("FPGXProjectAnalyzer: Analyzing project..."));

	FPGXScaffoldProjectInfo Info;
	Info.ProjectName = FApp::GetProjectName();
	Info.ContentDir = FPaths::ProjectContentDir();

	DetectProjectType(Info);
	CollectModules(Info);
	CollectPlugins(Info);
	CollectExistingStructure(Info);

	CachedInfo = Info;
	bCacheValid = true;

	PGX_LOG_INFO(LogPGXScaffold, TEXT("FPGXProjectAnalyzer: Analysis complete — Type=%d, Modules=%d, Plugins=%d, Folders=%d"),
		static_cast<int32>(Info.ProjectType), Info.Modules.Num(), Info.Plugins.Num(), Info.ExistingFolders.Num());

	return Info;
}

void FPGXProjectAnalyzer::DetectProjectType(FPGXScaffoldProjectInfo& OutInfo)
{
	// EN: Check if Source/ directory exists with .cpp files → C++ project
	// ES: Verificar si existe directorio Source/ con archivos .cpp → proyecto C++
	const FString SourceDir = FPaths::ProjectDir() / TEXT("Source");
	if (IFileManager::Get().DirectoryExists(*SourceDir))
	{
		TArray<FString> CppFiles;
		IFileManager::Get().FindFilesRecursive(CppFiles, *SourceDir, TEXT("*.cpp"), true, false);
		OutInfo.ProjectType = CppFiles.Num() > 0 ? EPGXProjectType::CppProject : EPGXProjectType::BlueprintOnly;
	}
	else
	{
		OutInfo.ProjectType = EPGXProjectType::BlueprintOnly;
	}
}

void FPGXProjectAnalyzer::CollectModules(FPGXScaffoldProjectInfo& OutInfo)
{
	// EN: Scan Source/ for module directories (contain .Build.cs)
	// ES: Escanear Source/ buscando directorios de modulos (contienen .Build.cs)
	const FString SourceDir = FPaths::ProjectDir() / TEXT("Source");
	if (!IFileManager::Get().DirectoryExists(*SourceDir))
	{
		return;
	}

	TArray<FString> BuildFiles;
	IFileManager::Get().FindFilesRecursive(BuildFiles, *SourceDir, TEXT("*.Build.cs"), true, false);
	for (const FString& BuildFile : BuildFiles)
	{
		OutInfo.Modules.Add(FPaths::GetBaseFilename(BuildFile).Replace(TEXT(".Build"), TEXT("")));
	}
}

void FPGXProjectAnalyzer::CollectPlugins(FPGXScaffoldProjectInfo& OutInfo)
{
	// EN: List enabled plugins / ES: Listar plugins habilitados
	TArray<TSharedRef<IPlugin>> Plugins = IPluginManager::Get().GetEnabledPlugins();
	for (const TSharedRef<IPlugin>& Plugin : Plugins)
	{
		OutInfo.Plugins.Add(Plugin->GetName());
	}
}

void FPGXProjectAnalyzer::CollectExistingStructure(FPGXScaffoldProjectInfo& OutInfo)
{
	// EN: Recursively collect all folders under Content/
	// ES: Recolectar recursivamente todas las carpetas bajo Content/
	const FString ContentDir = FPaths::ProjectContentDir();
	TArray<FString> Dirs;
	IFileManager::Get().FindFilesRecursive(Dirs, *ContentDir, TEXT("*"), false, true);

	for (const FString& Dir : Dirs)
	{
		FString RelDir = Dir;
		FPaths::MakePathRelativeTo(RelDir, *ContentDir);
		OutInfo.ExistingFolders.Add(RelDir);
	}

	// EN: Collect existing asset paths for duplicate detection
	// ES: Recolectar rutas de assets existentes para deteccion de duplicados
	TArray<FString> AssetFiles;
	IFileManager::Get().FindFilesRecursive(AssetFiles, *ContentDir, TEXT("*.uasset"), true, false);
	for (const FString& AssetFile : AssetFiles)
	{
		FString RelPath = AssetFile;
		FPaths::MakePathRelativeTo(RelPath, *ContentDir);
		OutInfo.ExistingAssets.Add(FPaths::GetBaseFilename(RelPath));
	}
}
