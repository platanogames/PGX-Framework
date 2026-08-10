// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXSaveProvider_Default.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformMisc.h"
#include "Misc/Paths.h"
#include "Logging/PGXLogCategories.h"
#include "Logging/PGXLogMacros.h"

// EN: Default platform save provider implementation using standard UE file I/O
// ES: Implementacion del provider de guardado por defecto usando I/O de archivos estandar de UE

bool UPGXSaveProvider_Default::SaveBytes(const FString& FilePath, const TArray<uint8>& Bytes)
{
	if (!FFileHelper::SaveArrayToFile(Bytes, *FilePath))
	{
		PGX_LOG_ERROR(LogPGXSave, TEXT("[Provider_Default] Failed to write %d bytes to: %s"),
			Bytes.Num(), *FilePath);
		return false;
	}
	return true;
}

bool UPGXSaveProvider_Default::LoadBytes(const FString& FilePath, TArray<uint8>& OutBytes)
{
	OutBytes.Empty();

	if (!FFileHelper::LoadFileToArray(OutBytes, *FilePath))
	{
		PGX_LOG_ERROR(LogPGXSave, TEXT("[Provider_Default] Failed to read from: %s"), *FilePath);
		return false;
	}
	return true;
}

bool UPGXSaveProvider_Default::DeleteFile(const FString& FilePath)
{
	if (!IFileManager::Get().Delete(*FilePath, /*bRequireExists=*/ false, /*bEvenReadOnly=*/ true))
	{
		PGX_LOG_WARNING(LogPGXSave, TEXT("[Provider_Default] Failed to delete: %s"), *FilePath);
		return false;
	}
	return true;
}

bool UPGXSaveProvider_Default::DoesFileExist(const FString& FilePath) const
{
	return IFileManager::Get().FileExists(*FilePath);
}

TArray<FString> UPGXSaveProvider_Default::GetFilesInDirectory(const FString& DirectoryPath, const FString& Extension) const
{
	TArray<FString> FoundFiles;

	const FString SearchPattern = Extension.IsEmpty()
		? FPaths::Combine(DirectoryPath, TEXT("*"))
		: FPaths::Combine(DirectoryPath, TEXT("*") + Extension);

	IFileManager::Get().FindFiles(FoundFiles, *SearchPattern, /*bFiles=*/ true, /*bDirectories=*/ false);

	// EN: FindFiles returns relative names — convert to full paths
	// ES: FindFiles retorna nombres relativos — convertir a paths completos
	for (FString& File : FoundFiles)
	{
		File = FPaths::Combine(DirectoryPath, File);
	}

	return FoundFiles;
}

bool UPGXSaveProvider_Default::EnsureDirectoryExists(const FString& DirectoryPath)
{
	if (!IFileManager::Get().DirectoryExists(*DirectoryPath))
	{
		return IFileManager::Get().MakeDirectory(*DirectoryPath, /*bTree=*/ true);
	}
	return true;
}

int64 UPGXSaveProvider_Default::GetDiskSpaceAvailable() const
{
	uint64 TotalBytes = 0;
	uint64 FreeBytes = 0;

	if (FPlatformMisc::GetDiskTotalAndFreeSpace(FPaths::ProjectSavedDir(), TotalBytes, FreeBytes))
	{
		return static_cast<int64>(FreeBytes);
	}

	return -1;
}

FString UPGXSaveProvider_Default::GetBaseSaveDirectory() const
{
	return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("SaveGames"));
}
