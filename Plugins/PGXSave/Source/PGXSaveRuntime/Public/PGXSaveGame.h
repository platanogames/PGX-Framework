// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GameplayTagContainer.h"
#include "PGXSaveGame.generated.h"

/**
 * EN: Base save game class for PGX. Provides versioning, metadata, and a
 *     generic key-value store for Blueprint users who don't need a subclass.
 *     Users can derive from this class (BP or C++) to add custom UPROPERTYs
 *     that UE serializes automatically.
 *
 *     Two paths for storing data:
 *     1. Key-Value: WriteSaveDataInt/Float/etc → stored in typed TMaps below
 *     2. Subclass:  Create BP/C++ subclass with your own UPROPERTYs
 *     Both paths coexist without conflict.
 *
 * ES: Clase base de save game para PGX. Proporciona versionado, metadata, y un
 *     almacen key-value generico para usuarios Blueprint que no necesitan subclass.
 *     Los usuarios pueden derivar de esta clase (BP o C++) para anadir UPROPERTYs
 *     custom que UE serializa automaticamente.
 *
 *     Dos caminos para almacenar datos:
 *     1. Key-Value: WriteSaveDataInt/Float/etc → almacenado en TMaps tipados abajo
 *     2. Subclass:  Crear subclass BP/C++ con tus propios UPROPERTYs
 *     Ambos caminos coexisten sin conflicto.
 */
UCLASS(Blueprintable, BlueprintType)
class PGXSAVERUNTIME_API UPGXSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPGXSaveGame();

	// ========================================================================
	// EN: Metadata (always present in every save)
	// ES: Metadata (siempre presente en cada guardado)
	// ========================================================================

	/** EN: Save data format version for migration support / ES: Version del formato de datos para soporte de migracion */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Metadata")
	int32 SaveFormatVersion;

	/** EN: Timestamp when the save was created / ES: Timestamp de cuando se creo el guardado */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Metadata")
	FDateTime SaveTimestamp;

	/** EN: Domain tag this SaveGame belongs to / ES: Tag de dominio al que pertenece este SaveGame */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Metadata")
	FGameplayTag DomainTag;

	/** EN: Integrity checksum (populated by serializer) / ES: Checksum de integridad (poblado por el serializador) */
	UPROPERTY()
	FString Checksum;

	// ========================================================================
	// EN: Key-Value store (for WriteSaveData/ReadSaveData BP nodes)
	// ES: Almacen Key-Value (para nodos BP WriteSaveData/ReadSaveData)
	// ========================================================================

	/** @internal EN: String key-value pairs / ES: Pares key-value de tipo String */
	UPROPERTY()
	TMap<FName, FString> StringData;

	/** @internal EN: Integer key-value pairs / ES: Pares key-value de tipo Integer */
	UPROPERTY()
	TMap<FName, int32> IntData;

	/** @internal EN: Float key-value pairs / ES: Pares key-value de tipo Float */
	UPROPERTY()
	TMap<FName, float> FloatData;

	/** @internal EN: Boolean key-value pairs / ES: Pares key-value de tipo Boolean */
	UPROPERTY()
	TMap<FName, bool> BoolData;

	/** @internal EN: Vector key-value pairs / ES: Pares key-value de tipo Vector */
	UPROPERTY()
	TMap<FName, FVector> VectorData;

	/** @internal EN: Rotator key-value pairs / ES: Pares key-value de tipo Rotator */
	UPROPERTY()
	TMap<FName, FRotator> RotatorData;

	/** @internal EN: Transform key-value pairs / ES: Pares key-value de tipo Transform */
	UPROPERTY()
	TMap<FName, FTransform> TransformData;

	/** @internal EN: GameplayTag key-value pairs / ES: Pares key-value de tipo GameplayTag */
	UPROPERTY()
	TMap<FName, FGameplayTag> TagData;

	// ========================================================================
	// EN: Key-Value API (C++ convenience, BP uses BlueprintLibrary)
	// ES: API Key-Value (conveniencia C++, BP usa BlueprintLibrary)
	// ========================================================================

	/** EN: Write a string value / ES: Escribir un valor string */
	void WriteString(FName Key, const FString& Value) { StringData.Add(Key, Value); }
	/** EN: Write an integer value / ES: Escribir un valor entero */
	void WriteInt(FName Key, int32 Value) { IntData.Add(Key, Value); }
	/** EN: Write a float value / ES: Escribir un valor float */
	void WriteFloat(FName Key, float Value) { FloatData.Add(Key, Value); }
	/** EN: Write a boolean value / ES: Escribir un valor booleano */
	void WriteBool(FName Key, bool Value) { BoolData.Add(Key, Value); }
	/** EN: Write a vector value / ES: Escribir un valor vector */
	void WriteVector(FName Key, const FVector& Value) { VectorData.Add(Key, Value); }
	/** EN: Write a rotator value / ES: Escribir un valor rotator */
	void WriteRotator(FName Key, const FRotator& Value) { RotatorData.Add(Key, Value); }
	/** EN: Write a transform value / ES: Escribir un valor transform */
	void WriteTransform(FName Key, const FTransform& Value) { TransformData.Add(Key, Value); }
	/** EN: Write a gameplay tag value / ES: Escribir un valor gameplay tag */
	void WriteTag(FName Key, const FGameplayTag& Value) { TagData.Add(Key, Value); }

	/** EN: Read a string value with default / ES: Leer un valor string con default */
	FString ReadString(FName Key, const FString& Default = TEXT("")) const;
	/** EN: Read an integer value with default / ES: Leer un valor entero con default */
	int32 ReadInt(FName Key, int32 Default = 0) const;
	/** EN: Read a float value with default / ES: Leer un valor float con default */
	float ReadFloat(FName Key, float Default = 0.0f) const;
	/** EN: Read a boolean value with default / ES: Leer un valor booleano con default */
	bool ReadBool(FName Key, bool Default = false) const;
	/** EN: Read a vector value with default / ES: Leer un valor vector con default */
	FVector ReadVector(FName Key, const FVector& Default = FVector::ZeroVector) const;
	/** EN: Read a rotator value with default / ES: Leer un valor rotator con default */
	FRotator ReadRotator(FName Key, const FRotator& Default = FRotator::ZeroRotator) const;
	/** EN: Read a transform value with default / ES: Leer un valor transform con default */
	FTransform ReadTransform(FName Key, const FTransform& Default = FTransform::Identity) const;
	/** EN: Read a gameplay tag value / ES: Leer un valor gameplay tag */
	FGameplayTag ReadTag(FName Key) const;

	/** EN: Check if a key exists in any typed map / ES: Comprobar si una key existe en algun mapa tipado */
	bool HasKey(FName Key) const;

	/** EN: Get total number of key-value entries across all maps / ES: Obtener numero total de entradas key-value en todos los mapas */
	int32 GetKeyValueCount() const;

	/** EN: Clear all key-value data / ES: Limpiar todos los datos key-value */
	void ClearAllKeyValueData();
};
