// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXSaveProvider.h"
#include "PGXSaveProvider_Default.generated.h"

/**
 * EN: Default platform save provider using standard UE file I/O.
 *     Uses FFileHelper for read/write and IFileManager for file operations.
 *     Suitable for PC (Windows/Mac/Linux). Override for console-specific
 *     storage APIs or cloud providers.
 *
 * ES: Provider de guardado por defecto usando I/O de archivos estandar de UE.
 *     Usa FFileHelper para lectura/escritura e IFileManager para operaciones de archivo.
 *     Adecuado para PC (Windows/Mac/Linux). Override para APIs de almacenamiento
 *     especificas de consola o proveedores cloud.
 */
UCLASS()
class PGXSAVERUNTIME_API UPGXSaveProvider_Default : public UPGXSaveProvider
{
	GENERATED_BODY()

public:
	//~ Begin UPGXSaveProvider Interface
	bool SaveBytes(const FString& FilePath, const TArray<uint8>& Bytes) override;
	bool LoadBytes(const FString& FilePath, TArray<uint8>& OutBytes) override;
	bool DeleteFile(const FString& FilePath) override;
	bool DoesFileExist(const FString& FilePath) const override;
	TArray<FString> GetFilesInDirectory(const FString& DirectoryPath, const FString& Extension = TEXT("")) const override;
	bool EnsureDirectoryExists(const FString& DirectoryPath) override;
	int64 GetDiskSpaceAvailable() const override;
	FString GetBaseSaveDirectory() const override;
	//~ End UPGXSaveProvider Interface
};
