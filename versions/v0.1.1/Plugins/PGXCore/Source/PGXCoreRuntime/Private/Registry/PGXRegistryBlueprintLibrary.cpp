// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Registry/PGXRegistryBlueprintLibrary.h"
#include "Registry/PGXDataRegistrySubsystem.h"
#include "Base/PGXDataAsset.h"
#include "Engine/GameInstance.h"

// EN: Helper to get the registry subsystem from world context
// ES: Helper para obtener el subsistema de registro desde world context
static UPGXDataRegistrySubsystem* GetRegistrySubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return UPGXDataRegistrySubsystem::GetCached();
	}

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return UPGXDataRegistrySubsystem::GetCached();
	}

	UGameInstance* GI = World->GetGameInstance();
	return GI ? GI->GetSubsystem<UPGXDataRegistrySubsystem>() : nullptr;
}

// ═══════════════════════════════════════════════════════════════
// Query / Consulta
// ═══════════════════════════════════════════════════════════════

FPGXRegistryEntry UPGXRegistryBlueprintLibrary::FindRegistryEntry(const UObject* WorldContextObject, FGameplayTag DatabaseTag, FGameplayTag ItemTag, bool& bFound)
{
	bFound = false;
	UPGXDataRegistrySubsystem* Registry = GetRegistrySubsystem(WorldContextObject);
	if (!Registry)
	{
		return FPGXRegistryEntry();
	}

	const FPGXRegistryEntry* Entry = Registry->FindEntry(DatabaseTag, ItemTag);
	if (Entry)
	{
		bFound = true;
		return *Entry;
	}
	return FPGXRegistryEntry();
}

TArray<FPGXRegistryEntry> UPGXRegistryBlueprintLibrary::FindRegistryEntriesByCategory(const UObject* WorldContextObject, FGameplayTag DatabaseTag, FGameplayTag CategoryTag)
{
	UPGXDataRegistrySubsystem* Registry = GetRegistrySubsystem(WorldContextObject);
	if (!Registry)
	{
		return TArray<FPGXRegistryEntry>();
	}
	return Registry->FindByCategory(DatabaseTag, CategoryTag);
}

TArray<FPGXRegistryEntry> UPGXRegistryBlueprintLibrary::GetAllRegistryEntries(const UObject* WorldContextObject, FGameplayTag DatabaseTag)
{
	UPGXDataRegistrySubsystem* Registry = GetRegistrySubsystem(WorldContextObject);
	if (!Registry)
	{
		return TArray<FPGXRegistryEntry>();
	}
	return Registry->GetAllEntries(DatabaseTag);
}

UPGXDataAsset* UPGXRegistryBlueprintLibrary::ResolveRegistryAsset(const UObject* WorldContextObject, FGameplayTag DatabaseTag, FGameplayTag ItemTag)
{
	UPGXDataRegistrySubsystem* Registry = GetRegistrySubsystem(WorldContextObject);
	if (!Registry)
	{
		return nullptr;
	}
	return Registry->ResolveAsset(DatabaseTag, ItemTag);
}

UPGXDataAsset* UPGXRegistryBlueprintLibrary::ResolveRegistryAssetTyped(const UObject* WorldContextObject,
	FGameplayTag DatabaseTag, FGameplayTag ItemTag, TSubclassOf<UPGXDataAsset> ExpectedClass)
{
	// EN: DeterminesOutputType handles the BP node pin type — runtime cast verifies
	// ES: DeterminesOutputType maneja el tipo del pin del nodo BP — cast en runtime verifica
	UPGXDataAsset* Asset = ResolveRegistryAsset(WorldContextObject, DatabaseTag, ItemTag);
	if (Asset && ExpectedClass && !Asset->IsA(ExpectedClass))
	{
		return nullptr;
	}
	return Asset;
}

UPGXDataAsset* UPGXRegistryBlueprintLibrary::ResolveRegistryAssetByTag(const UObject* WorldContextObject, FGameplayTag FullItemTag)
{
	if (!FullItemTag.IsValid()) { return nullptr; }

	// EN: Parse tag hierarchy — parent is database, full tag is item
	// ES: Parsear jerarquia del tag — padre es database, tag completo es item
	const FGameplayTag ParentTag = FullItemTag.RequestDirectParent();
	if (!ParentTag.IsValid()) { return nullptr; }

	return ResolveRegistryAsset(WorldContextObject, ParentTag, FullItemTag);
}

// ═══════════════════════════════════════════════════════════════
// Telemetry / Telemetria
// ═══════════════════════════════════════════════════════════════

FPGXDatabaseStats UPGXRegistryBlueprintLibrary::GetRegistryDatabaseStats(const UObject* WorldContextObject, FGameplayTag DatabaseTag)
{
	UPGXDataRegistrySubsystem* Registry = GetRegistrySubsystem(WorldContextObject);
	if (!Registry)
	{
		return FPGXDatabaseStats();
	}
	return Registry->GetDatabaseStats(DatabaseTag);
}

bool UPGXRegistryBlueprintLibrary::HasRegistryDatabase(const UObject* WorldContextObject, FGameplayTag DatabaseTag)
{
	UPGXDataRegistrySubsystem* Registry = GetRegistrySubsystem(WorldContextObject);
	if (!Registry)
	{
		return false;
	}
	return Registry->HasDatabase(DatabaseTag);
}

TArray<FGameplayTag> UPGXRegistryBlueprintLibrary::GetAllRegistryDatabaseTags(const UObject* WorldContextObject)
{
	UPGXDataRegistrySubsystem* Registry = GetRegistrySubsystem(WorldContextObject);
	if (!Registry)
	{
		return TArray<FGameplayTag>();
	}
	return Registry->GetAllDatabaseTags();
}

// ═══════════════════════════════════════════════════════════════
// v2.0 Composite Key Query / Consulta por Clave Compuesta v2.0
// ═══════════════════════════════════════════════════════════════

FPGXRegistryEntry UPGXRegistryBlueprintLibrary::FindByCompositeKey(const UObject* WorldContextObject,
	FGameplayTag DatabaseTag, FGameplayTag CategoryTag, FGameplayTag ItemTag, bool& bFound)
{
	bFound = false;
	UPGXDataRegistrySubsystem* Registry = GetRegistrySubsystem(WorldContextObject);
	if (!Registry)
	{
		return FPGXRegistryEntry();
	}

	FPGXCompositeRegistryKey Key;
	Key.DatabaseTag = DatabaseTag;
	Key.CategoryTag = CategoryTag;
	Key.ItemTag = ItemTag;

	const FPGXRegistryEntry* Entry = Registry->FindByCompositeKey(Key);
	if (Entry)
	{
		bFound = true;
		return *Entry;
	}
	return FPGXRegistryEntry();
}

TArray<FPGXRegistryEntry> UPGXRegistryBlueprintLibrary::GetCategoryEntries(const UObject* WorldContextObject,
	FGameplayTag DatabaseTag, FGameplayTag CategoryTag)
{
	UPGXDataRegistrySubsystem* Registry = GetRegistrySubsystem(WorldContextObject);
	if (!Registry)
	{
		return TArray<FPGXRegistryEntry>();
	}
	return Registry->GetCategoryEntries(DatabaseTag, CategoryTag);
}

void UPGXRegistryBlueprintLibrary::ReIngestDefinitions(const UObject* WorldContextObject)
{
	UPGXDataRegistrySubsystem* Registry = GetRegistrySubsystem(WorldContextObject);
	if (Registry)
	{
		Registry->IngestAllDefinitions();
	}
}

int32 UPGXRegistryBlueprintLibrary::GetTotalIndexedEntries(const UObject* WorldContextObject)
{
	UPGXDataRegistrySubsystem* Registry = GetRegistrySubsystem(WorldContextObject);
	if (!Registry)
	{
		return 0;
	}
	return Registry->GetEntryIndexCount();
}
