// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "PGXSaveTypes.h"
#include "PGXSaveSerializer.generated.h"

class UPGXSaveGame;
class UPGXSaveConfig;

/**
 * EN: Static utility class for save data serialization, compression, and integrity.
 *     Converts between UPGXSaveGame objects and raw byte arrays.
 *     Handles optional zlib compression and CRC32 checksum generation/validation.
 *     Also resolves file paths based on save mode and context configuration.
 *
 *     Pipeline responsibilities (the Serializer handles WHAT, the Provider handles WHERE):
 *     - Serializer: SaveGame <-> bytes, compress, checksum, path resolution
 *     - Provider:   bytes <-> disk (platform-specific I/O)
 *
 * ES: Clase utilitaria estatica para serializacion, compresion e integridad de datos de guardado.
 *     Convierte entre objetos UPGXSaveGame y arrays de bytes crudos.
 *     Maneja compresion zlib opcional y generacion/validacion de checksum CRC32.
 *     Tambien resuelve paths de archivo basados en modo de guardado y configuracion de contexto.
 *
 *     Responsabilidades del pipeline (el Serializer maneja QUE, el Provider maneja DONDE):
 *     - Serializer: SaveGame <-> bytes, comprimir, checksum, resolucion de paths
 *     - Provider:   bytes <-> disco (I/O especifico de plataforma)
 */
UCLASS()
class PGXSAVERUNTIME_API UPGXSaveSerializer : public UObject
{
	GENERATED_BODY()

public:
	// ========================================================================
	// EN: Serialization (SaveGame <-> bytes)
	// ES: Serializacion (SaveGame <-> bytes)
	// ========================================================================

	/**
	 * EN: Serialize a SaveGame object to a raw byte array.
	 *     Uses UGameplayStatics::SaveGameToMemory internally.
	 * ES: Serializar un objeto SaveGame a un array de bytes crudos.
	 *     Usa UGameplayStatics::SaveGameToMemory internamente.
	 *
	 * @param SaveGame  EN: The SaveGame to serialize / ES: El SaveGame a serializar
	 * @param OutBytes  EN: Output byte array / ES: Array de bytes de salida
	 * @return          EN: True if serialization succeeded / ES: True si la serializacion fue exitosa
	 */
	static bool SerializeToMemory(UPGXSaveGame* SaveGame, TArray<uint8>& OutBytes);

	/**
	 * EN: Deserialize a raw byte array back into a SaveGame object.
	 *     Uses UGameplayStatics::LoadGameFromMemory internally.
	 * ES: Deserializar un array de bytes crudos de vuelta a un objeto SaveGame.
	 *     Usa UGameplayStatics::LoadGameFromMemory internamente.
	 *
	 * @param Bytes  EN: Input byte data / ES: Datos de bytes de entrada
	 * @return       EN: Reconstructed SaveGame (null on failure) / ES: SaveGame reconstruido (null si falla)
	 */
	static UPGXSaveGame* DeserializeFromMemory(const TArray<uint8>& Bytes);

	// ========================================================================
	// EN: Compression (optional zlib)
	// ES: Compresion (zlib opcional)
	// ========================================================================

	/**
	 * EN: Compress a byte array using zlib. Prepends uncompressed size header (4 bytes).
	 * ES: Comprimir un array de bytes usando zlib. Antepone header de tamano sin comprimir (4 bytes).
	 *
	 * @param InBytes   EN: Uncompressed data / ES: Datos sin comprimir
	 * @param OutBytes  EN: Compressed output / ES: Salida comprimida
	 * @return          EN: True if compression succeeded / ES: True si la compresion fue exitosa
	 */
	static bool Compress(const TArray<uint8>& InBytes, TArray<uint8>& OutBytes);

	/**
	 * EN: Decompress a previously compressed byte array.
	 * ES: Descomprimir un array de bytes previamente comprimido.
	 *
	 * @param InBytes   EN: Compressed data (with size header) / ES: Datos comprimidos (con header de tamano)
	 * @param OutBytes  EN: Decompressed output / ES: Salida descomprimida
	 * @return          EN: True if decompression succeeded / ES: True si la descompresion fue exitosa
	 */
	static bool Decompress(const TArray<uint8>& InBytes, TArray<uint8>& OutBytes);

	// ========================================================================
	// EN: Integrity (CRC32 checksum)
	// ES: Integridad (checksum CRC32)
	// ========================================================================

	/**
	 * EN: Generate a CRC32 checksum string from raw byte data.
	 * ES: Generar un string de checksum CRC32 desde datos de bytes crudos.
	 */
	static FString GenerateChecksum(const TArray<uint8>& Data);

	/**
	 * EN: Validate that a byte array matches the expected checksum.
	 * ES: Validar que un array de bytes coincida con el checksum esperado.
	 */
	static bool ValidateChecksum(const TArray<uint8>& Data, const FString& ExpectedChecksum);

	// ========================================================================
	// EN: Path resolution (mode-aware)
	// ES: Resolucion de paths (consciente del modo)
	// ========================================================================

	/**
	 * EN: Resolve the full file path for a domain save file.
	 *     Path depends on SaveMode:
	 *       SingleSlot:   {Base}/{Context}/{Domain}.sav
	 *       MultiSlot:    {Base}/{Context}/{SlotName}/{Domain}.sav
	 *       SessionBased: {Base}/{Context}/{SlotName}/{Domain}.sav
	 *
	 * ES: Resolver el path completo de archivo para un archivo de guardado de dominio.
	 *     El path depende del SaveMode:
	 *       SingleSlot:   {Base}/{Context}/{Domain}.sav
	 *       MultiSlot:    {Base}/{Context}/{SlotName}/{Domain}.sav
	 *       SessionBased: {Base}/{Context}/{SlotName}/{Domain}.sav
	 *
	 * @param Config       EN: The SaveConfig DA / ES: El DA de SaveConfig
	 * @param BasePath     EN: Provider base directory / ES: Directorio base del provider
	 * @param SlotName     EN: Slot name (ignored for SingleSlot) / ES: Nombre del slot (ignorado para SingleSlot)
	 * @param DomainTag    EN: Domain identifier / ES: Identificador de dominio
	 * @return             EN: Full absolute file path / ES: Path absoluto completo del archivo
	 */
	static FString ResolveSavePath(const UPGXSaveConfig* Config, const FString& BasePath,
		const FString& SlotName, FGameplayTag DomainTag);

	/**
	 * EN: Resolve the path for the master index file of a context.
	 * ES: Resolver el path para el archivo de indice maestro de un contexto.
	 *
	 * @param Config    EN: The SaveConfig DA / ES: El DA de SaveConfig
	 * @param BasePath  EN: Provider base directory / ES: Directorio base del provider
	 * @return          EN: Full path to _Index.sav / ES: Path completo a _Index.sav
	 */
	static FString ResolveIndexPath(const UPGXSaveConfig* Config, const FString& BasePath);

	/**
	 * EN: Resolve the directory path for a specific slot.
	 * ES: Resolver el path de directorio para un slot especifico.
	 */
	static FString ResolveSlotDirectory(const UPGXSaveConfig* Config, const FString& BasePath,
		const FString& SlotName);

	/**
	 * EN: Generate a session-based slot name using current timestamp.
	 * ES: Generar un nombre de slot basado en sesion usando el timestamp actual.
	 */
	static FString GenerateSessionSlotName();
};
