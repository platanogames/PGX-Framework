// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXSaveGame.h"

// EN: Base save game class implementation
// ES: Implementacion de la clase base de save game

UPGXSaveGame::UPGXSaveGame()
	: SaveFormatVersion(1)
	, SaveTimestamp(FDateTime::MinValue())
{
}

// ============================================================================
// EN: Key-Value Read API
// ES: API de lectura Key-Value
// ============================================================================

FString UPGXSaveGame::ReadString(FName Key, const FString& Default) const
{
	const FString* Found = StringData.Find(Key);
	return Found ? *Found : Default;
}

int32 UPGXSaveGame::ReadInt(FName Key, int32 Default) const
{
	const int32* Found = IntData.Find(Key);
	return Found ? *Found : Default;
}

float UPGXSaveGame::ReadFloat(FName Key, float Default) const
{
	const float* Found = FloatData.Find(Key);
	return Found ? *Found : Default;
}

bool UPGXSaveGame::ReadBool(FName Key, bool Default) const
{
	const bool* Found = BoolData.Find(Key);
	return Found ? *Found : Default;
}

FVector UPGXSaveGame::ReadVector(FName Key, const FVector& Default) const
{
	const FVector* Found = VectorData.Find(Key);
	return Found ? *Found : Default;
}

FRotator UPGXSaveGame::ReadRotator(FName Key, const FRotator& Default) const
{
	const FRotator* Found = RotatorData.Find(Key);
	return Found ? *Found : Default;
}

FTransform UPGXSaveGame::ReadTransform(FName Key, const FTransform& Default) const
{
	const FTransform* Found = TransformData.Find(Key);
	return Found ? *Found : Default;
}

FGameplayTag UPGXSaveGame::ReadTag(FName Key) const
{
	const FGameplayTag* Found = TagData.Find(Key);
	return Found ? *Found : FGameplayTag();
}

bool UPGXSaveGame::HasKey(FName Key) const
{
	return StringData.Contains(Key)
		|| IntData.Contains(Key)
		|| FloatData.Contains(Key)
		|| BoolData.Contains(Key)
		|| VectorData.Contains(Key)
		|| RotatorData.Contains(Key)
		|| TransformData.Contains(Key)
		|| TagData.Contains(Key);
}

int32 UPGXSaveGame::GetKeyValueCount() const
{
	return StringData.Num()
		+ IntData.Num()
		+ FloatData.Num()
		+ BoolData.Num()
		+ VectorData.Num()
		+ RotatorData.Num()
		+ TransformData.Num()
		+ TagData.Num();
}

void UPGXSaveGame::ClearAllKeyValueData()
{
	StringData.Empty();
	IntData.Empty();
	FloatData.Empty();
	BoolData.Empty();
	VectorData.Empty();
	RotatorData.Empty();
	TransformData.Empty();
	TagData.Empty();
}
