// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PGXSaveProvider.generated.h"

/**
 * EN: Abstract base class for platform save I/O abstraction.
 *     Works with raw bytes and file paths — the subsystem hands off
 *     already-serialized data. Override for platform-specific storage
 *     (cloud, console, etc.). Default implementation uses FFileHelper.
 *
 *     The subsystem creates one instance via NewObject using the class
 *     specified in UPGXSaveConfig::SaveProviderClass (or Default if null).
 *
 * ES: Clase base abstracta para abstraccion de I/O de guardado por plataforma.
 *     Trabaja con bytes crudos y paths de archivo — el subsistema entrega
 *     datos ya serializados. Override para almacenamiento especifico de
 *     plataforma (cloud, consola, etc.). Implementacion default usa FFileHelper.
 *
 *     El subsistema crea una instancia via NewObject usando la clase
 *     especificada en UPGXSaveConfig::SaveProviderClass (o Default si null).
 */
UCLASS(Abstract, BlueprintType)
class PGXSAVERUNTIME_API UPGXSaveProvider : public UObject
{
	GENERATED_BODY()

public:
	// ========================================================================
	// EN: File operations
	// ES: Operaciones de archivo
	// ========================================================================

	/**
	 * EN: Save raw bytes to the specified file path.
	 * ES: Guardar bytes crudos en el path de archivo especificado.
	 *
	 * @param FilePath  EN: Full absolute path to the target file / ES: Path absoluto completo al archivo destino
	 * @param Bytes     EN: Data to write / ES: Datos a escribir
	 * @return          EN: True if write succeeded / ES: True si la escritura fue exitosa
	 */
	virtual bool SaveBytes(const FString& /*FilePath*/, const TArray<uint8>& /*Bytes*/) PURE_VIRTUAL(UPGXSaveProvider::SaveBytes, return false;);

	/**
	 * EN: Load raw bytes from the specified file path.
	 * ES: Cargar bytes crudos desde el path de archivo especificado.
	 *
	 * @param FilePath  EN: Full absolute path to the source file / ES: Path absoluto completo al archivo fuente
	 * @param OutBytes  EN: Loaded data (cleared before load) / ES: Datos cargados (limpiados antes de cargar)
	 * @return          EN: True if read succeeded / ES: True si la lectura fue exitosa
	 */
	virtual bool LoadBytes(const FString& /*FilePath*/, TArray<uint8>& /*OutBytes*/) PURE_VIRTUAL(UPGXSaveProvider::LoadBytes, return false;);

	/**
	 * EN: Delete a file at the specified path.
	 * ES: Eliminar un archivo en el path especificado.
	 */
	virtual bool DeleteFile(const FString& /*FilePath*/) PURE_VIRTUAL(UPGXSaveProvider::DeleteFile, return false;);

	/**
	 * EN: Check if a file exists at the specified path.
	 * ES: Comprobar si un archivo existe en el path especificado.
	 */
	virtual bool DoesFileExist(const FString& /*FilePath*/) const PURE_VIRTUAL(UPGXSaveProvider::DoesFileExist, return false;);

	// ========================================================================
	// EN: Directory operations
	// ES: Operaciones de directorio
	// ========================================================================

	/**
	 * EN: Get all files matching a pattern in a directory.
	 * ES: Obtener todos los archivos que coincidan con un patron en un directorio.
	 *
	 * @param DirectoryPath  EN: Directory to search / ES: Directorio donde buscar
	 * @param Extension      EN: File extension filter (e.g. ".sav") / ES: Filtro de extension (ej. ".sav")
	 * @return               EN: Array of full file paths / ES: Array de paths completos de archivos
	 */
	virtual TArray<FString> GetFilesInDirectory(const FString& /*DirectoryPath*/, const FString& /*Extension*/ = TEXT("")) const
		PURE_VIRTUAL(UPGXSaveProvider::GetFilesInDirectory, return {};);

	/**
	 * EN: Create a directory tree if it doesn't exist.
	 * ES: Crear un arbol de directorios si no existe.
	 */
	virtual bool EnsureDirectoryExists(const FString& /*DirectoryPath*/) PURE_VIRTUAL(UPGXSaveProvider::EnsureDirectoryExists, return false;);

	// ========================================================================
	// EN: Platform info
	// ES: Informacion de plataforma
	// ========================================================================

	/**
	 * EN: Get available disk space in bytes.
	 * ES: Obtener espacio en disco disponible en bytes.
	 */
	virtual int64 GetDiskSpaceAvailable() const PURE_VIRTUAL(UPGXSaveProvider::GetDiskSpaceAvailable, return -1;);

	/**
	 * EN: Get the base save directory for this platform.
	 * ES: Obtener el directorio base de guardado para esta plataforma.
	 */
	virtual FString GetBaseSaveDirectory() const PURE_VIRTUAL(UPGXSaveProvider::GetBaseSaveDirectory, return TEXT(""););

	// ========================================================================
	// EN: Cloud sync (default = not supported)
	// ES: Sincronizacion cloud (default = no soportado)
	// ========================================================================

	/** EN: Whether this provider supports cloud sync / ES: Si este provider soporta sincronizacion cloud */
	virtual bool SupportsCloudSync() const { return false; }
};
