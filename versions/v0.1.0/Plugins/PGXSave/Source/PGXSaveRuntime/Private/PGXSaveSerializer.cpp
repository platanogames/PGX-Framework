// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXSaveSerializer.h"
#include "PGXSaveGame.h"
#include "PGXSaveConfig.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Compression.h"
#include "Misc/CRC.h"
#include "Misc/Paths.h"
#include "Logging/PGXLogCategories.h"
#include "Logging/PGXLogMacros.h"

// EN: Save data serializer implementation
// ES: Implementacion del serializador de datos de guardado

// ============================================================================
// EN: Serialization
// ES: Serializacion
// ============================================================================

bool UPGXSaveSerializer::SerializeToMemory(UPGXSaveGame* SaveGame, TArray<uint8>& OutBytes)
{
	if (!SaveGame)
	{
		PGX_LOG_ERROR(LogPGXSave, TEXT("[Serializer] SerializeToMemory: SaveGame is null"));
		return false;
	}

	OutBytes.Empty();

	// EN: Use UGameplayStatics which handles the full UPROPERTY serialization chain
	// ES: Usar UGameplayStatics que maneja la cadena completa de serializacion de UPROPERTYs
	if (!UGameplayStatics::SaveGameToMemory(SaveGame, OutBytes))
	{
		PGX_LOG_ERROR(LogPGXSave, TEXT("[Serializer] SerializeToMemory failed for class: %s"),
			*SaveGame->GetClass()->GetName());
		return false;
	}

	return true;
}

UPGXSaveGame* UPGXSaveSerializer::DeserializeFromMemory(const TArray<uint8>& Bytes)
{
	if (Bytes.Num() == 0)
	{
		PGX_LOG_ERROR(LogPGXSave, TEXT("[Serializer] DeserializeFromMemory: empty byte array"));
		return nullptr;
	}

	// EN: UGameplayStatics::LoadGameFromMemory reconstructs the object from bytes
	// ES: UGameplayStatics::LoadGameFromMemory reconstruye el objeto desde bytes
	USaveGame* LoadedGame = UGameplayStatics::LoadGameFromMemory(Bytes);

	if (!LoadedGame)
	{
		PGX_LOG_ERROR(LogPGXSave, TEXT("[Serializer] DeserializeFromMemory: LoadGameFromMemory returned null"));
		return nullptr;
	}

	UPGXSaveGame* PGXSaveGame = Cast<UPGXSaveGame>(LoadedGame);
	if (!PGXSaveGame)
	{
		PGX_LOG_ERROR(LogPGXSave, TEXT("[Serializer] DeserializeFromMemory: loaded object is not UPGXSaveGame (class: %s)"),
			*LoadedGame->GetClass()->GetName());
		return nullptr;
	}

	return PGXSaveGame;
}

// ============================================================================
// EN: Compression
// ES: Compresion
// ============================================================================

bool UPGXSaveSerializer::Compress(const TArray<uint8>& InBytes, TArray<uint8>& OutBytes)
{
	if (InBytes.Num() == 0)
	{
		return false;
	}

	// EN: Store uncompressed size as 4-byte header so Decompress knows the buffer size
	// ES: Guardar tamano sin comprimir como header de 4 bytes para que Decompress sepa el tamano del buffer
	const int32 UncompressedSize = InBytes.Num();

	// EN: Allocate worst-case output: header (4 bytes) + compressed data (could be larger than input in edge cases)
	// ES: Asignar salida de peor caso: header (4 bytes) + datos comprimidos (podria ser mas grande que input en casos extremos)
	const int32 MaxCompressedSize = FCompression::CompressMemoryBound(NAME_Zlib, UncompressedSize);
	OutBytes.SetNumUninitialized(static_cast<int32>(sizeof(int32)) + MaxCompressedSize);

	// EN: Write uncompressed size header
	// ES: Escribir header de tamano sin comprimir
	FMemory::Memcpy(OutBytes.GetData(), &UncompressedSize, sizeof(int32));

	// EN: Compress into buffer after header
	// ES: Comprimir en buffer despues del header
	int32 CompressedSize = MaxCompressedSize;
	if (!FCompression::CompressMemory(NAME_Zlib,
		OutBytes.GetData() + sizeof(int32), CompressedSize,
		InBytes.GetData(), UncompressedSize))
	{
		PGX_LOG_ERROR(LogPGXSave, TEXT("[Serializer] Compression failed (input: %d bytes)"), UncompressedSize);
		OutBytes.Empty();
		return false;
	}

	// EN: Trim to actual size: header + compressed data
	// ES: Recortar al tamano real: header + datos comprimidos
	OutBytes.SetNum(static_cast<int32>(sizeof(int32)) + CompressedSize);
	return true;
}

bool UPGXSaveSerializer::Decompress(const TArray<uint8>& InBytes, TArray<uint8>& OutBytes)
{
	// EN: Minimum: 4-byte header + at least 1 byte of data
	// ES: Minimo: header de 4 bytes + al menos 1 byte de datos
	if (InBytes.Num() <= static_cast<int32>(sizeof(int32)))
	{
		PGX_LOG_ERROR(LogPGXSave, TEXT("[Serializer] Decompress: data too small (%d bytes)"), InBytes.Num());
		return false;
	}

	// EN: Read uncompressed size from header
	// ES: Leer tamano sin comprimir desde header
	int32 UncompressedSize = 0;
	FMemory::Memcpy(&UncompressedSize, InBytes.GetData(), sizeof(int32));

	if (UncompressedSize <= 0 || UncompressedSize > 256 * 1024 * 1024) // 256 MB sanity check
	{
		PGX_LOG_ERROR(LogPGXSave, TEXT("[Serializer] Decompress: invalid uncompressed size: %d"), UncompressedSize);
		return false;
	}

	OutBytes.SetNumUninitialized(UncompressedSize);

	const int32 CompressedSize = InBytes.Num() - static_cast<int32>(sizeof(int32));
	if (!FCompression::UncompressMemory(NAME_Zlib,
		OutBytes.GetData(), UncompressedSize,
		InBytes.GetData() + sizeof(int32), CompressedSize))
	{
		PGX_LOG_ERROR(LogPGXSave, TEXT("[Serializer] Decompression failed (compressed: %d, expected: %d)"),
			CompressedSize, UncompressedSize);
		OutBytes.Empty();
		return false;
	}

	return true;
}

// ============================================================================
// EN: Integrity
// ES: Integridad
// ============================================================================

FString UPGXSaveSerializer::GenerateChecksum(const TArray<uint8>& Data)
{
	if (Data.Num() == 0)
	{
		return TEXT("");
	}

	const uint32 CRC = FCrc::MemCrc32(Data.GetData(), Data.Num());
	return FString::Printf(TEXT("%08X"), CRC);
}

bool UPGXSaveSerializer::ValidateChecksum(const TArray<uint8>& Data, const FString& ExpectedChecksum)
{
	if (ExpectedChecksum.IsEmpty())
	{
		return true; // EN: No checksum to validate / ES: No hay checksum que validar
	}

	const FString ActualChecksum = GenerateChecksum(Data);
	return ActualChecksum.Equals(ExpectedChecksum, ESearchCase::IgnoreCase);
}

// ============================================================================
// EN: Path resolution
// ES: Resolucion de paths
// ============================================================================

FString UPGXSaveSerializer::ResolveSavePath(const UPGXSaveConfig* Config, const FString& BasePath,
	const FString& SlotName, FGameplayTag DomainTag)
{
	if (!Config)
	{
		PGX_LOG_ERROR(LogPGXSave, TEXT("[Serializer] ResolveSavePath: Config is null"));
		return TEXT("");
	}

	// EN: Context subdirectory from the ContextTag
	// ES: Subdirectorio de contexto desde el ContextTag
	const FString ContextDir = Config->ContextTag.IsValid()
		? Config->ContextTag.ToString().Replace(TEXT("."), TEXT("/"))
		: TEXT("Default");

	// EN: Domain filename from the DomainTag
	// ES: Nombre de archivo de dominio desde el DomainTag
	const FString DomainFilename = DomainTag.IsValid()
		? DomainTag.GetTagName().ToString().Replace(TEXT("."), TEXT("_")) + Config->SaveFileExtension
		: TEXT("Unknown") + Config->SaveFileExtension;

	switch (Config->SaveMode)
	{
	case EPGXSaveMode::SingleSlot:
		// EN: {Base}/{Context}/{Domain}.sav — no slot subdirectory
		// ES: {Base}/{Context}/{Domain}.sav — sin subdirectorio de slot
		return FPaths::Combine(BasePath, Config->BaseDirectory, ContextDir, DomainFilename);

	case EPGXSaveMode::MultiSlot:
	case EPGXSaveMode::SessionBased:
		// EN: {Base}/{Context}/{SlotName}/{Domain}.sav
		// ES: {Base}/{Context}/{SlotName}/{Domain}.sav
		return FPaths::Combine(BasePath, Config->BaseDirectory, ContextDir, SlotName, DomainFilename);

	default:
		return FPaths::Combine(BasePath, Config->BaseDirectory, ContextDir, SlotName, DomainFilename);
	}
}

FString UPGXSaveSerializer::ResolveIndexPath(const UPGXSaveConfig* Config, const FString& BasePath)
{
	if (!Config)
	{
		return TEXT("");
	}

	const FString ContextDir = Config->ContextTag.IsValid()
		? Config->ContextTag.ToString().Replace(TEXT("."), TEXT("/"))
		: TEXT("Default");

	return FPaths::Combine(BasePath, Config->BaseDirectory, ContextDir, TEXT("_Index.sav"));
}

FString UPGXSaveSerializer::ResolveSlotDirectory(const UPGXSaveConfig* Config, const FString& BasePath,
	const FString& SlotName)
{
	if (!Config)
	{
		return TEXT("");
	}

	const FString ContextDir = Config->ContextTag.IsValid()
		? Config->ContextTag.ToString().Replace(TEXT("."), TEXT("/"))
		: TEXT("Default");

	if (Config->SaveMode == EPGXSaveMode::SingleSlot)
	{
		// EN: SingleSlot has no slot subdirectory
		// ES: SingleSlot no tiene subdirectorio de slot
		return FPaths::Combine(BasePath, Config->BaseDirectory, ContextDir);
	}

	return FPaths::Combine(BasePath, Config->BaseDirectory, ContextDir, SlotName);
}

FString UPGXSaveSerializer::GenerateSessionSlotName()
{
	const FDateTime Now = FDateTime::Now();
	return FString::Printf(TEXT("Session_%04d%02d%02d_%02d%02d%02d"),
		Now.GetYear(), Now.GetMonth(), Now.GetDay(),
		Now.GetHour(), Now.GetMinute(), Now.GetSecond());
}
