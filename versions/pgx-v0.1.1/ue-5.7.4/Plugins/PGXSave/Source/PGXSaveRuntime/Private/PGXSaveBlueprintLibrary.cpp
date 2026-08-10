// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXSaveBlueprintLibrary.h"
#include "PGXSaveSubsystem.h"
#include "PGXSaveGame.h"
#include "PGXSaveConfig.h"
#include "PGXSaveVersioning.h"
#include "Engine/GameInstance.h"

// EN: Blueprint function library implementation for PGX Save
// ES: Implementacion de la biblioteca de funciones Blueprint para PGX Save

// ============================================================================
// EN: Internal helper
// ES: Helper interno
// ============================================================================

UPGXSaveSubsystem* UPGXSaveBlueprintLibrary::GetSaveSubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return nullptr;
	}

	UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		return nullptr;
	}

	return GI->GetSubsystem<UPGXSaveSubsystem>();
}

// ============================================================================
// EN: Core operations (auto-resolve slot from config)
// ES: Operaciones core (auto-resuelven slot desde config)
// ============================================================================

EPGXSaveResult UPGXSaveBlueprintLibrary::Save(const UObject* WorldContextObject, FGameplayTag ContextTag,
	FName /*CallerName*/)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	return Sub ? Sub->QuickSave(ContextTag) : EPGXSaveResult::Failed;
}

EPGXSaveResult UPGXSaveBlueprintLibrary::Load(const UObject* WorldContextObject, FGameplayTag ContextTag,
	FName /*CallerName*/)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	return Sub ? Sub->QuickLoad(ContextTag) : EPGXSaveResult::Failed;
}

// ============================================================================
// EN: Slot operations (explicit slot name)
// ES: Operaciones de slot (nombre de slot explicito)
// ============================================================================

EPGXSaveResult UPGXSaveBlueprintLibrary::SaveToSlot(const UObject* WorldContextObject,
	FGameplayTag ContextTag, const FString& SlotName, FName /*CallerName*/)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	return Sub ? Sub->SaveContext(ContextTag, SlotName) : EPGXSaveResult::Failed;
}

EPGXSaveResult UPGXSaveBlueprintLibrary::LoadFromSlot(const UObject* WorldContextObject,
	FGameplayTag ContextTag, const FString& SlotName, FName /*CallerName*/)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	return Sub ? Sub->LoadContext(ContextTag, SlotName) : EPGXSaveResult::Failed;
}

EPGXSaveResult UPGXSaveBlueprintLibrary::DeleteSlot(const UObject* WorldContextObject,
	FGameplayTag ContextTag, const FString& SlotName, FName /*CallerName*/)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	return Sub ? Sub->DeleteSlot(ContextTag, SlotName) : EPGXSaveResult::Failed;
}

EPGXSaveResult UPGXSaveBlueprintLibrary::CopySlot(const UObject* WorldContextObject,
	FGameplayTag ContextTag, const FString& SourceSlotName, const FString& DestSlotName)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	return Sub ? Sub->CopySlot(ContextTag, SourceSlotName, DestSlotName) : EPGXSaveResult::Failed;
}

// ============================================================================
// EN: Slot queries
// ES: Consultas de slots
// ============================================================================

TArray<FPGXSaveSlotInfo> UPGXSaveBlueprintLibrary::GetAllSlots(const UObject* WorldContextObject, FGameplayTag ContextTag)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	return Sub ? Sub->GetAllSlots(ContextTag) : TArray<FPGXSaveSlotInfo>();
}

FPGXSaveSlotInfo UPGXSaveBlueprintLibrary::GetSlotInfo(const UObject* WorldContextObject,
	FGameplayTag ContextTag, const FString& SlotName)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	return Sub ? Sub->GetSlotInfo(ContextTag, SlotName) : FPGXSaveSlotInfo();
}

bool UPGXSaveBlueprintLibrary::DoesSlotExist(const UObject* WorldContextObject,
	FGameplayTag ContextTag, const FString& SlotName)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	return Sub ? Sub->DoesSlotExist(ContextTag, SlotName) : false;
}

FString UPGXSaveBlueprintLibrary::GetNextAvailableSlotName(const UObject* WorldContextObject, FGameplayTag ContextTag)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	return Sub ? Sub->GetNextAvailableSlotName(ContextTag) : TEXT("");
}

// ============================================================================
// EN: Data write
// ES: Escritura de datos
// ============================================================================

void UPGXSaveBlueprintLibrary::WriteSaveDataString(const UObject* WorldContextObject,
	FGameplayTag DomainTag, FName Key, const FString& Value)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	if (!Sub) return;
	UPGXSaveGame* SG = Sub->GetSaveGame(DomainTag);
	if (SG) SG->WriteString(Key, Value);
}

void UPGXSaveBlueprintLibrary::WriteSaveDataInt(const UObject* WorldContextObject,
	FGameplayTag DomainTag, FName Key, int32 Value)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	if (!Sub) return;
	UPGXSaveGame* SG = Sub->GetSaveGame(DomainTag);
	if (SG) SG->WriteInt(Key, Value);
}

void UPGXSaveBlueprintLibrary::WriteSaveDataFloat(const UObject* WorldContextObject,
	FGameplayTag DomainTag, FName Key, float Value)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	if (!Sub) return;
	UPGXSaveGame* SG = Sub->GetSaveGame(DomainTag);
	if (SG) SG->WriteFloat(Key, Value);
}

void UPGXSaveBlueprintLibrary::WriteSaveDataBool(const UObject* WorldContextObject,
	FGameplayTag DomainTag, FName Key, bool Value)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	if (!Sub) return;
	UPGXSaveGame* SG = Sub->GetSaveGame(DomainTag);
	if (SG) SG->WriteBool(Key, Value);
}

void UPGXSaveBlueprintLibrary::WriteSaveDataVector(const UObject* WorldContextObject,
	FGameplayTag DomainTag, FName Key, FVector Value)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	if (!Sub) return;
	UPGXSaveGame* SG = Sub->GetSaveGame(DomainTag);
	if (SG) SG->WriteVector(Key, Value);
}

void UPGXSaveBlueprintLibrary::WriteSaveDataRotator(const UObject* WorldContextObject,
	FGameplayTag DomainTag, FName Key, FRotator Value)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	if (!Sub) return;
	UPGXSaveGame* SG = Sub->GetSaveGame(DomainTag);
	if (SG) SG->WriteRotator(Key, Value);
}

void UPGXSaveBlueprintLibrary::WriteSaveDataTransform(const UObject* WorldContextObject,
	FGameplayTag DomainTag, FName Key, const FTransform& Value)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	if (!Sub) return;
	UPGXSaveGame* SG = Sub->GetSaveGame(DomainTag);
	if (SG) SG->WriteTransform(Key, Value);
}

void UPGXSaveBlueprintLibrary::WriteSaveDataTag(const UObject* WorldContextObject,
	FGameplayTag DomainTag, FName Key, FGameplayTag Value)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	if (!Sub) return;
	UPGXSaveGame* SG = Sub->GetSaveGame(DomainTag);
	if (SG) SG->WriteTag(Key, Value);
}

// ============================================================================
// EN: Data read
// ES: Lectura de datos
// ============================================================================

FString UPGXSaveBlueprintLibrary::ReadSaveDataString(const UObject* WorldContextObject,
	FGameplayTag DomainTag, FName Key, const FString& DefaultValue)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	if (!Sub) return DefaultValue;
	UPGXSaveGame* SG = Sub->GetSaveGame(DomainTag);
	return SG ? SG->ReadString(Key, DefaultValue) : DefaultValue;
}

int32 UPGXSaveBlueprintLibrary::ReadSaveDataInt(const UObject* WorldContextObject,
	FGameplayTag DomainTag, FName Key, int32 DefaultValue)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	if (!Sub) return DefaultValue;
	UPGXSaveGame* SG = Sub->GetSaveGame(DomainTag);
	return SG ? SG->ReadInt(Key, DefaultValue) : DefaultValue;
}

float UPGXSaveBlueprintLibrary::ReadSaveDataFloat(const UObject* WorldContextObject,
	FGameplayTag DomainTag, FName Key, float DefaultValue)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	if (!Sub) return DefaultValue;
	UPGXSaveGame* SG = Sub->GetSaveGame(DomainTag);
	return SG ? SG->ReadFloat(Key, DefaultValue) : DefaultValue;
}

bool UPGXSaveBlueprintLibrary::ReadSaveDataBool(const UObject* WorldContextObject,
	FGameplayTag DomainTag, FName Key, bool DefaultValue)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	if (!Sub) return DefaultValue;
	UPGXSaveGame* SG = Sub->GetSaveGame(DomainTag);
	return SG ? SG->ReadBool(Key, DefaultValue) : DefaultValue;
}

FVector UPGXSaveBlueprintLibrary::ReadSaveDataVector(const UObject* WorldContextObject,
	FGameplayTag DomainTag, FName Key)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	if (!Sub) return FVector::ZeroVector;
	UPGXSaveGame* SG = Sub->GetSaveGame(DomainTag);
	return SG ? SG->ReadVector(Key) : FVector::ZeroVector;
}

FRotator UPGXSaveBlueprintLibrary::ReadSaveDataRotator(const UObject* WorldContextObject,
	FGameplayTag DomainTag, FName Key)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	if (!Sub) return FRotator::ZeroRotator;
	UPGXSaveGame* SG = Sub->GetSaveGame(DomainTag);
	return SG ? SG->ReadRotator(Key) : FRotator::ZeroRotator;
}

FTransform UPGXSaveBlueprintLibrary::ReadSaveDataTransform(const UObject* WorldContextObject,
	FGameplayTag DomainTag, FName Key)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	if (!Sub) return FTransform::Identity;
	UPGXSaveGame* SG = Sub->GetSaveGame(DomainTag);
	return SG ? SG->ReadTransform(Key) : FTransform::Identity;
}

FGameplayTag UPGXSaveBlueprintLibrary::ReadSaveDataTag(const UObject* WorldContextObject,
	FGameplayTag DomainTag, FName Key)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	if (!Sub) return FGameplayTag();
	UPGXSaveGame* SG = Sub->GetSaveGame(DomainTag);
	return SG ? SG->ReadTag(Key) : FGameplayTag();
}

// ============================================================================
// EN: Data utility
// ES: Utilidad de datos
// ============================================================================

bool UPGXSaveBlueprintLibrary::HasSaveData(const UObject* WorldContextObject,
	FGameplayTag DomainTag, FName Key)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	return Sub ? Sub->HasData(DomainTag, Key) : false;
}

void UPGXSaveBlueprintLibrary::ClearDomainData(const UObject* WorldContextObject, FGameplayTag DomainTag)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	if (Sub) Sub->ClearDomain(DomainTag);
}

UPGXSaveGame* UPGXSaveBlueprintLibrary::GetSaveGame(const UObject* WorldContextObject, FGameplayTag DomainTag)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	return Sub ? Sub->GetSaveGame(DomainTag) : nullptr;
}

UPGXSaveGame* UPGXSaveBlueprintLibrary::GetSaveGameTyped(const UObject* WorldContextObject,
	FGameplayTag DomainTag, TSubclassOf<UPGXSaveGame> ExpectedClass)
{
	// EN: DeterminesOutputType handles the BP node pin type — runtime cast verifies
	// ES: DeterminesOutputType maneja el tipo del pin del nodo BP — cast en runtime verifica
	UPGXSaveGame* SaveGame = GetSaveGame(WorldContextObject, DomainTag);
	if (SaveGame && ExpectedClass && !SaveGame->IsA(ExpectedClass))
	{
		return nullptr;
	}
	return SaveGame;
}

// ============================================================================
// EN: Auto-save control
// ES: Control de auto-guardado
// ============================================================================

void UPGXSaveBlueprintLibrary::SetAutoSaveEnabled(const UObject* WorldContextObject,
	FGameplayTag ContextTag, bool bEnabled)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	if (Sub) Sub->SetAutoSaveEnabled(ContextTag, bEnabled);
}

void UPGXSaveBlueprintLibrary::TriggerAutoSave(const UObject* WorldContextObject, FGameplayTag ContextTag)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	if (Sub) Sub->TriggerAutoSave(ContextTag);
}

bool UPGXSaveBlueprintLibrary::IsAutoSaveActive(const UObject* WorldContextObject, FGameplayTag ContextTag)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	return Sub ? Sub->IsAutoSaveActive(ContextTag) : false;
}

// ============================================================================
// EN: Active state
// ES: Estado activo
// ============================================================================

FString UPGXSaveBlueprintLibrary::GetActiveSlotName(const UObject* WorldContextObject, FGameplayTag ContextTag)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	return Sub ? Sub->GetActiveSlotName(ContextTag) : TEXT("");
}

void UPGXSaveBlueprintLibrary::SetActiveSlot(const UObject* WorldContextObject,
	FGameplayTag ContextTag, const FString& SlotName)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	if (Sub) Sub->SetActiveSlot(ContextTag, SlotName);
}

bool UPGXSaveBlueprintLibrary::IsSaveInProgress(const UObject* WorldContextObject)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	return Sub ? Sub->IsSaveInProgress() : false;
}

bool UPGXSaveBlueprintLibrary::IsLoadInProgress(const UObject* WorldContextObject)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	return Sub ? Sub->IsLoadInProgress() : false;
}

// ============================================================================
// EN: Context queries
// ES: Consultas de contexto
// ============================================================================

TArray<FGameplayTag> UPGXSaveBlueprintLibrary::GetAllContextTags(const UObject* WorldContextObject)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	return Sub ? Sub->GetAllContextTags() : TArray<FGameplayTag>();
}

int32 UPGXSaveBlueprintLibrary::GetContextCount(const UObject* WorldContextObject)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	return Sub ? Sub->GetContextCount() : 0;
}

// ============================================================================
// EN: Versioning queries
// ES: Consultas de versionado
// ============================================================================

int32 UPGXSaveBlueprintLibrary::GetSaveFormatVersion(const UObject* WorldContextObject, FGameplayTag DomainTag)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	if (!Sub) return 0;
	UPGXSaveGame* SG = Sub->GetSaveGame(DomainTag);
	return SG ? SG->SaveFormatVersion : 0;
}

int32 UPGXSaveBlueprintLibrary::GetCurrentSaveVersion(const UObject* WorldContextObject, FGameplayTag ContextTag)
{
	UPGXSaveSubsystem* Sub = GetSaveSubsystem(WorldContextObject);
	if (!Sub) return 0;
	const UPGXSaveConfig* Config = Sub->GetContextConfig(ContextTag);
	return Config ? Config->CurrentSaveVersion : 0;
}

bool UPGXSaveBlueprintLibrary::HasMigrationPath(int32 FromVersion, int32 ToVersion)
{
	return FPGXSaveVersioning::HasMigrationPath(FromVersion, ToVersion);
}

int32 UPGXSaveBlueprintLibrary::GetMigrationCount()
{
	return FPGXSaveVersioning::GetRegisteredMigrationCount();
}
