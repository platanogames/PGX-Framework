// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Registry/PGXDataRegistrySubsystem.h"
#include "Registry/PGXDataDatabase.h"
#include "Registry/PGXRegistryDefinition.h"
#include "Registry/PGXRegistrySettings.h"
#include "Tables/PGXTableTypes.h"
#include "Base/PGXDataAsset.h"
#include "Data/PGXObjectDataAsset.h"
#include "Data/PGXConfigDataAsset.h"
#include "Profile/PGXProfileSubsystem.h"
#include "Profile/PGXPlatformConfig.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/DataTable.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "GameplayTagsManager.h"

DEFINE_LOG_CATEGORY(LogPGXRegistry);

// ═══════════════════════════════════════════════════════════════
// Static Cache / Cache Estatico
// ═══════════════════════════════════════════════════════════════

TWeakObjectPtr<UPGXDataRegistrySubsystem> UPGXDataRegistrySubsystem::CachedInstance;

UPGXDataRegistrySubsystem* UPGXDataRegistrySubsystem::GetCached()
{
	return CachedInstance.Get();
}

// ═══════════════════════════════════════════════════════════════
// Lifecycle / Ciclo de Vida
// ═══════════════════════════════════════════════════════════════

void UPGXDataRegistrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CachedInstance = this;
	// EN: v2.0 — Auto-ingest definitions if settings allow
	// ES: v2.0 — Auto-ingestar definiciones si los settings lo permiten
	const UPGXRegistrySettings* Settings = GetDefault<UPGXRegistrySettings>();
	if (Settings && Settings->bAutoIngestOnInit)
	{
		IngestAllDefinitions();
	}

	// ── Profile Integration ──
	if (UGameInstance* GI = GetGameInstance())
	{
		if (auto* ProfileSS = GI->GetSubsystem<UPGXProfileSubsystem>())
		{
			ProfileSS->OnProfileChangedNative.AddUObject(this, &ThisClass::HandleProfileChanged);
		}
	}

	UE_LOG(LogPGXRegistry, Log, TEXT("PGX Data Registry initialized | v2.0 EntryIndex=%d"),
		EntryIndex.Num());
}

void UPGXDataRegistrySubsystem::Deinitialize()
{
	// ── Profile Unbind ──
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UPGXProfileSubsystem* ProfileSS = GI->GetSubsystem<UPGXProfileSubsystem>())
		{
			ProfileSS->OnProfileChangedNative.RemoveAll(this);
		}
	}

	UE_LOG(LogPGXRegistry, Log, TEXT("PGX Data Registry deinitializing | Databases=%d EntryIndex=%d"),
		Databases.Num(), EntryIndex.Num());
	Databases.Empty();
	EntryIndex.Empty();
	CompositeReverseIndex.Empty();
	V2CategoryIndex.Empty();
	ActiveDefinitions.Empty();
	CachedInstance.Reset();

	Super::Deinitialize();
}

// ═══════════════════════════════════════════════════════════════
// Registration API / API de Registro
// ═══════════════════════════════════════════════════════════════

bool UPGXDataRegistrySubsystem::CreateDatabase(FGameplayTag DatabaseTag, UClass* AssetClass, bool bAutoDiscover)
{
	// EN: v2.0 Migration telemetry — warn when legacy path is used
	// ES: v2.0 Telemetria de migracion — advertir cuando se usa el path legacy
	UE_LOG(LogPGXRegistry, Warning,
		TEXT("Legacy CreateDatabase() path used for [%s]. "
			 "Consider migrating to UPGXRegistryDefinition DataAssets for DataTable-first authoring."),
		*DatabaseTag.ToString());

	if (!DatabaseTag.IsValid())
	{
		UE_LOG(LogPGXRegistry, Warning, TEXT("CreateDatabase: Invalid DatabaseTag"));
		return false;
	}

	if (!AssetClass)
	{
		UE_LOG(LogPGXRegistry, Warning, TEXT("CreateDatabase: Null AssetClass for [%s]"), *DatabaseTag.ToString());
		return false;
	}

	if (Databases.Contains(DatabaseTag))
	{
		UE_LOG(LogPGXRegistry, Warning, TEXT("CreateDatabase: Database [%s] already exists"), *DatabaseTag.ToString());
		return false;
	}

	TUniquePtr<FPGXDataDatabase> NewDB = MakeUnique<FPGXDataDatabase>();
	NewDB->DatabaseTag = DatabaseTag;
	NewDB->AssetClass = AssetClass;
	NewDB->bAutoDiscovered = bAutoDiscover;

	FPGXDataDatabase& DBRef = *NewDB;
	Databases.Add(DatabaseTag, MoveTemp(NewDB));

	if (bAutoDiscover)
	{
		AutoDiscoverAssets(DBRef);
	}

	OnDatabaseCreated.Broadcast(DatabaseTag);

	UE_LOG(LogPGXRegistry, Log, TEXT("Database created: [%s] Class=%s AutoDiscover=%s Entries=%d"),
		*DatabaseTag.ToString(),
		*AssetClass->GetName(),
		bAutoDiscover ? TEXT("true") : TEXT("false"),
		DBRef.Entries.Num());

	return true;
}

bool UPGXDataRegistrySubsystem::RegisterAsset(FGameplayTag DatabaseTag, UPGXDataAsset* Asset, FGameplayTag ItemTag)
{
	if (!Asset)
	{
		UE_LOG(LogPGXRegistry, Warning, TEXT("RegisterAsset: Null asset for database [%s]"), *DatabaseTag.ToString());
		return false;
	}

	if (!ItemTag.IsValid())
	{
		UE_LOG(LogPGXRegistry, Warning, TEXT("RegisterAsset: Invalid ItemTag for asset [%s]"), *Asset->GetName());
		return false;
	}

	TUniquePtr<FPGXDataDatabase>* FoundDB = Databases.Find(DatabaseTag);
	if (!FoundDB)
	{
		UE_LOG(LogPGXRegistry, Warning, TEXT("RegisterAsset: Database [%s] not found"), *DatabaseTag.ToString());
		return false;
	}

	FPGXDataDatabase& DB = **FoundDB;

	// EN: Type safety check / ES: Verificacion de seguridad de tipo
	if (DB.AssetClass && !Asset->IsA(DB.AssetClass))
	{
		UE_LOG(LogPGXRegistry, Warning, TEXT("RegisterAsset: Asset [%s] is not of expected class [%s] for database [%s]"),
			*Asset->GetName(), *DB.AssetClass->GetName(), *DatabaseTag.ToString());
		return false;
	}

	if (const FPGXRegistryEntry* ExistingEntry = DB.Entries.Find(ItemTag))
	{
		const UPGXRegistrySettings* Settings = GetDefault<UPGXRegistrySettings>();
		const EPGXRegistryConflictPolicy Policy = Settings
			? Settings->DefaultConflictPolicy
			: EPGXRegistryConflictPolicy::FailOnConflict;

		switch (Policy)
		{
		case EPGXRegistryConflictPolicy::FailOnConflict:
			UE_LOG(LogPGXRegistry, Error,
				TEXT("RegisterAsset: Conflict for [%s] in database [%s]. Policy=FailOnConflict, rejecting replacement."),
				*ItemTag.ToString(), *DatabaseTag.ToString());
			return false;

		case EPGXRegistryConflictPolicy::FirstWins:
			UE_LOG(LogPGXRegistry, Verbose,
				TEXT("RegisterAsset: Conflict for [%s] in database [%s]. Policy=FirstWins, keeping existing [%s]."),
				*ItemTag.ToString(), *DatabaseTag.ToString(), *ExistingEntry->AssetPath.ToString());
			return true;

		case EPGXRegistryConflictPolicy::LastWins:
			UE_LOG(LogPGXRegistry, Verbose,
				TEXT("RegisterAsset: Conflict for [%s] in database [%s]. Policy=LastWins, replacing."),
				*ItemTag.ToString(), *DatabaseTag.ToString());
			if (ExistingEntry->CategoryTag.IsValid())
			{
				DB.CategoryIndex.RemoveSingle(ExistingEntry->CategoryTag, ItemTag);
			}
			RemoveCompositeIndexesForItem(DatabaseTag, ItemTag);
			break;
		}
	}
	else
	{
		// EN: Defensive cleanup for partially-ingested stale links; O(K) via reverse index.
		// ES: Limpieza defensiva de enlaces obsoletos; O(K) via indice inverso.
		RemoveCompositeIndexesForItem(DatabaseTag, ItemTag);
	}

	FPGXRegistryEntry Entry = BuildEntryFromAsset(Asset, ItemTag);
	Entry.CachedAsset = Asset; // EN: Already loaded, cache it / ES: Ya cargado, cachearlo

	// EN: Update category index / ES: Actualizar indice de categoria
	if (Entry.CategoryTag.IsValid())
	{
		DB.CategoryIndex.RemoveSingle(Entry.CategoryTag, ItemTag);
		DB.CategoryIndex.Add(Entry.CategoryTag, ItemTag);
	}

	if (Entry.CategoryTag.IsValid())
	{
		FPGXCompositeRegistryKey CompositeKey;
		CompositeKey.DatabaseTag = DatabaseTag;
		CompositeKey.CategoryTag = Entry.CategoryTag;
		CompositeKey.ItemTag = ItemTag;
		AddCompositeIndexEntry(CompositeKey, Entry);
	}

	DB.Entries.Add(ItemTag, MoveTemp(Entry));
	OnAssetRegistered.Broadcast(DatabaseTag, ItemTag);

	return true;
}

bool UPGXDataRegistrySubsystem::UnregisterAsset(FGameplayTag DatabaseTag, FGameplayTag ItemTag)
{
	TUniquePtr<FPGXDataDatabase>* FoundDB = Databases.Find(DatabaseTag);
	if (!FoundDB)
	{
		return false;
	}

	FPGXDataDatabase& DB = **FoundDB;
	const FPGXRegistryEntry* Entry = DB.Entries.Find(ItemTag);
	if (!Entry)
	{
		return false;
	}

	// EN: Remove from category indexes / ES: Eliminar de indices de categoria
	if (Entry->CategoryTag.IsValid())
	{
		DB.CategoryIndex.RemoveSingle(Entry->CategoryTag, ItemTag);
	}
	RemoveCompositeIndexesForItem(DatabaseTag, ItemTag);

	DB.Entries.Remove(ItemTag);
	OnAssetUnregistered.Broadcast(DatabaseTag, ItemTag);

	UE_LOG(LogPGXRegistry, Verbose, TEXT("Unregistered [%s] from database [%s]"),
		*ItemTag.ToString(), *DatabaseTag.ToString());

	return true;
}

bool UPGXDataRegistrySubsystem::HasDatabase(FGameplayTag DatabaseTag) const
{
	return Databases.Contains(DatabaseTag);
}

// ═══════════════════════════════════════════════════════════════
// Query API / API de Consulta
// ═══════════════════════════════════════════════════════════════

const FPGXRegistryEntry* UPGXDataRegistrySubsystem::FindEntry(FGameplayTag DatabaseTag, FGameplayTag ItemTag) const
{
	const TUniquePtr<FPGXDataDatabase>* FoundDB = Databases.Find(DatabaseTag);
	if (!FoundDB)
	{
		return nullptr;
	}
	return (*FoundDB)->Entries.Find(ItemTag);
}

TArray<FPGXRegistryEntry> UPGXDataRegistrySubsystem::FindByCategory(FGameplayTag DatabaseTag, FGameplayTag CategoryTag) const
{
	TArray<FPGXRegistryEntry> Result;

	const TUniquePtr<FPGXDataDatabase>* FoundDB = Databases.Find(DatabaseTag);
	if (!FoundDB)
	{
		return Result;
	}

	const FPGXDataDatabase& DB = **FoundDB;
	TArray<FGameplayTag> ItemTags;
	DB.CategoryIndex.MultiFind(CategoryTag, ItemTags);

	Result.Reserve(ItemTags.Num());
	for (const FGameplayTag& Tag : ItemTags)
	{
		if (const FPGXRegistryEntry* Entry = DB.Entries.Find(Tag))
		{
			Result.Add(*Entry);
		}
	}

	return Result;
}

TArray<FPGXRegistryEntry> UPGXDataRegistrySubsystem::GetAllEntries(FGameplayTag DatabaseTag) const
{
	TArray<FPGXRegistryEntry> Result;

	const TUniquePtr<FPGXDataDatabase>* FoundDB = Databases.Find(DatabaseTag);
	if (!FoundDB)
	{
		return Result;
	}

	const FPGXDataDatabase& DB = **FoundDB;
	Result.Reserve(DB.Entries.Num());
	for (const auto& Pair : DB.Entries)
	{
		Result.Add(Pair.Value);
	}

	return Result;
}

FSoftObjectPath UPGXDataRegistrySubsystem::GetSoftReference(FGameplayTag DatabaseTag, FGameplayTag ItemTag) const
{
	const FPGXRegistryEntry* Entry = FindEntry(DatabaseTag, ItemTag);
	return Entry ? Entry->AssetPath : FSoftObjectPath();
}

UPGXDataAsset* UPGXDataRegistrySubsystem::ResolveAsset(FGameplayTag DatabaseTag, FGameplayTag ItemTag)
{
	TUniquePtr<FPGXDataDatabase>* FoundDB = Databases.Find(DatabaseTag);
	if (!FoundDB)
	{
		return nullptr;
	}

	FPGXDataDatabase& DB = **FoundDB;
	FPGXRegistryEntry* Entry = DB.Entries.Find(ItemTag);
	if (!Entry)
	{
		return nullptr;
	}

	// EN: Return cached if available / ES: Retornar cacheado si disponible
	if (Entry->CachedAsset)
	{
		return Entry->CachedAsset;
	}

	// EN: Synchronous load / ES: Carga sincrona
	if (!Entry->AssetPath.IsNull())
	{
		UObject* Loaded = Entry->AssetPath.TryLoad();
		if (UPGXDataAsset* AsDA = Cast<UPGXDataAsset>(Loaded))
		{
			Entry->CachedAsset = AsDA;
			SyncCompositeCachedAsset(DatabaseTag, ItemTag, AsDA);
			return AsDA;
		}
	}

	return nullptr;
}

TArray<FGameplayTag> UPGXDataRegistrySubsystem::GetAllDatabaseTags() const
{
	TArray<FGameplayTag> Result;
	Databases.GetKeys(Result);
	return Result;
}

bool UPGXDataRegistrySubsystem::HasEntryByTag(FGameplayTag Tag) const
{
	return HasDatabase(Tag);
}

int32 UPGXDataRegistrySubsystem::GetCount() const
{
	return Databases.Num();
}

void UPGXDataRegistrySubsystem::GetSnapshot(TArray<FGameplayTag>& OutTags) const
{
	Databases.GetKeys(OutTags);
}

// ═══════════════════════════════════════════════════════════════
// Cache / Cache
// ═══════════════════════════════════════════════════════════════

void UPGXDataRegistrySubsystem::RequestLoad(FGameplayTag DatabaseTag, FGameplayTag ItemTag, FOnPGXAssetLoaded Callback)
{
	TUniquePtr<FPGXDataDatabase>* FoundDB = Databases.Find(DatabaseTag);
	if (!FoundDB)
	{
		Callback.ExecuteIfBound(ItemTag, nullptr);
		return;
	}

	FPGXDataDatabase& DB = **FoundDB;
	FPGXRegistryEntry* Entry = DB.Entries.Find(ItemTag);
	if (!Entry)
	{
		Callback.ExecuteIfBound(ItemTag, nullptr);
		return;
	}

	// EN: Already cached / ES: Ya cacheado
	if (Entry->CachedAsset)
	{
		Callback.ExecuteIfBound(ItemTag, Entry->CachedAsset);
		return;
	}

	if (Entry->AssetPath.IsNull())
	{
		Callback.ExecuteIfBound(ItemTag, nullptr);
		return;
	}

	// EN: Async load via StreamableManager / ES: Carga asincrona via StreamableManager
	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
	FGameplayTag CapturedDB = DatabaseTag;
	FGameplayTag CapturedItem = ItemTag;

	StreamableManager.RequestAsyncLoad(
		Entry->AssetPath,
		FStreamableDelegate::CreateWeakLambda(this,
			[this, CapturedDB, CapturedItem, Callback]()
			{
				TUniquePtr<FPGXDataDatabase>* DB2 = Databases.Find(CapturedDB);
				if (!DB2)
				{
					Callback.ExecuteIfBound(CapturedItem, nullptr);
					return;
				}

				FPGXRegistryEntry* Entry2 = (*DB2)->Entries.Find(CapturedItem);
				if (!Entry2)
				{
					Callback.ExecuteIfBound(CapturedItem, nullptr);
					return;
				}

				UObject* Loaded = Entry2->AssetPath.ResolveObject();
				UPGXDataAsset* AsDA = Cast<UPGXDataAsset>(Loaded);
				if (AsDA)
				{
					Entry2->CachedAsset = AsDA;
					SyncCompositeCachedAsset(CapturedDB, CapturedItem, AsDA);
				}
				Callback.ExecuteIfBound(CapturedItem, AsDA);
			}
		)
	);
}

void UPGXDataRegistrySubsystem::InvalidateCache(FGameplayTag DatabaseTag)
{
	TUniquePtr<FPGXDataDatabase>* FoundDB = Databases.Find(DatabaseTag);
	if (!FoundDB)
	{
		return;
	}

	FPGXDataDatabase& DB = **FoundDB;
	for (auto& Pair : DB.Entries)
	{
		Pair.Value.CachedAsset = nullptr;
	}

	OnCacheInvalidated.Broadcast(DatabaseTag);

	UE_LOG(LogPGXRegistry, Log, TEXT("Cache invalidated for database [%s]"), *DatabaseTag.ToString());
}

UPGXDataAsset* UPGXDataRegistrySubsystem::GetCachedAsset(FGameplayTag DatabaseTag, FGameplayTag ItemTag) const
{
	const FPGXRegistryEntry* Entry = FindEntry(DatabaseTag, ItemTag);
	return Entry ? Entry->CachedAsset : nullptr;
}

// ═══════════════════════════════════════════════════════════════
// Telemetry / Telemetria
// ═══════════════════════════════════════════════════════════════

FPGXDatabaseStats UPGXDataRegistrySubsystem::GetDatabaseStats(FGameplayTag DatabaseTag) const
{
	const TUniquePtr<FPGXDataDatabase>* FoundDB = Databases.Find(DatabaseTag);
	if (!FoundDB)
	{
		return FPGXDatabaseStats();
	}
	return (*FoundDB)->GetStats();
}

FString UPGXDataRegistrySubsystem::ExportMetadata(FGameplayTag DatabaseTag) const
{
	const TUniquePtr<FPGXDataDatabase>* FoundDB = Databases.Find(DatabaseTag);
	if (!FoundDB)
	{
		return TEXT("{}");
	}

	const FPGXDataDatabase& DB = **FoundDB;
	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("database"), DatabaseTag.ToString());
	Root->SetStringField(TEXT("assetClass"), DB.AssetClass ? DB.AssetClass->GetName() : TEXT("None"));
	Root->SetBoolField(TEXT("autoDiscovered"), DB.bAutoDiscovered);
	Root->SetNumberField(TEXT("totalEntries"), DB.Entries.Num());
	Root->SetNumberField(TEXT("loadedEntries"), DB.GetLoadedCount());
	Root->SetNumberField(TEXT("categoryCount"), DB.GetCategoryCount());

	TArray<TSharedPtr<FJsonValue>> EntriesArray;
	for (const auto& Pair : DB.Entries)
	{
		TSharedRef<FJsonObject> EntryObj = MakeShared<FJsonObject>();
		EntryObj->SetStringField(TEXT("itemTag"), Pair.Value.ItemTag.ToString());
		EntryObj->SetStringField(TEXT("categoryTag"), Pair.Value.CategoryTag.ToString());
		EntryObj->SetStringField(TEXT("assetId"), Pair.Value.AssetId.ToString());
		EntryObj->SetNumberField(TEXT("version"), Pair.Value.Version);
		EntryObj->SetStringField(TEXT("displayName"), Pair.Value.DisplayName.ToString());
		EntryObj->SetStringField(TEXT("assetPath"), Pair.Value.AssetPath.ToString());
		EntryObj->SetBoolField(TEXT("isLoaded"), Pair.Value.CachedAsset != nullptr);
		EntriesArray.Add(MakeShared<FJsonValueObject>(EntryObj));
	}
	Root->SetArrayField(TEXT("entries"), EntriesArray);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(Root, Writer);
	return OutputString;
}

FString UPGXDataRegistrySubsystem::ExportAllMetadata() const
{
	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetNumberField(TEXT("databaseCount"), Databases.Num());

	TArray<TSharedPtr<FJsonValue>> DatabasesArray;
	for (const auto& Pair : Databases)
	{
		FString DBJson = ExportMetadata(Pair.Key);
		TSharedPtr<FJsonObject> DBObj;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(DBJson);
		if (FJsonSerializer::Deserialize(Reader, DBObj) && DBObj.IsValid())
		{
			DatabasesArray.Add(MakeShared<FJsonValueObject>(DBObj.ToSharedRef()));
		}
	}
	Root->SetArrayField(TEXT("databases"), DatabasesArray);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(Root, Writer);
	return OutputString;
}

// ═══════════════════════════════════════════════════════════════
// Internal / Interno
// ═══════════════════════════════════════════════════════════════

void UPGXDataRegistrySubsystem::AutoDiscoverAssets(FPGXDataDatabase& Database)
{
	if (!Database.AssetClass)
	{
		return;
	}

	IAssetRegistry& AssetRegistry = UAssetManager::Get().GetAssetRegistry();

	TArray<FAssetData> FoundAssets;
	AssetRegistry.GetAssetsByClass(Database.AssetClass->GetClassPathName(), FoundAssets, true);

	UE_LOG(LogPGXRegistry, Log, TEXT("AutoDiscover: Scanning for [%s] — found %d assets"),
		*Database.AssetClass->GetName(), FoundAssets.Num());

	for (const FAssetData& AssetData : FoundAssets)
	{
		// EN: Try to extract AssetId from AssetRegistry metadata (AssetRegistrySearchable)
		// ES: Intentar extraer AssetId de metadata del AssetRegistry
		FName ExtractedAssetId;
		AssetData.GetTagValue(GET_MEMBER_NAME_CHECKED(UPGXDataAsset, AssetId), ExtractedAssetId);

		// EN: Build a tag from the asset name if no AssetId found
		// ES: Construir un tag del nombre del asset si no se encuentra AssetId
		FString TagString;
		if (ExtractedAssetId.IsNone() || ExtractedAssetId == NAME_None)
		{
			TagString = FString::Printf(TEXT("PGX.Registry.%s.%s"),
				*Database.DatabaseTag.GetTagName().ToString(),
				*AssetData.AssetName.ToString());
		}
		else
		{
			TagString = FString::Printf(TEXT("PGX.Registry.%s.%s"),
				*Database.DatabaseTag.GetTagName().ToString(),
				*ExtractedAssetId.ToString());
		}

		FGameplayTag ItemTag = FGameplayTag::RequestGameplayTag(FName(*TagString), false);
		if (!ItemTag.IsValid())
		{
			// EN: Create dynamic tag / ES: Crear tag dinamico
			ItemTag = FGameplayTag::RequestGameplayTag(FName(*TagString), false);
			if (!ItemTag.IsValid())
			{
				// EN: Tag doesn't exist in the tag table, use AddNativeGameplayTag approach
				// ES: El tag no existe en la tabla, usar un enfoque directo
				UGameplayTagsManager& TagManager = UGameplayTagsManager::Get();
				ItemTag = TagManager.RequestGameplayTag(FName(*TagString), false);
				if (!ItemTag.IsValid())
				{
					// EN: Fallback: register a runtime tag
					// ES: Fallback: registrar un tag de runtime
					ItemTag = FGameplayTag::RequestGameplayTag(FName(*TagString), false);
				}
			}
		}

		// EN: Build entry from metadata without forcing a load
		// ES: Construir entrada desde metadata sin forzar carga
		FPGXRegistryEntry Entry;
		Entry.ItemTag = ItemTag.IsValid() ? ItemTag : FGameplayTag();
		Entry.AssetId = ExtractedAssetId;
		Entry.AssetPath = AssetData.GetSoftObjectPath();
		Entry.Version = 0; // EN: Cannot extract without loading / ES: No se puede extraer sin cargar

		// EN: Try to extract category and display name from metadata
		// ES: Intentar extraer categoria y display name de metadata
		FString CategoryStr;
		if (AssetData.GetTagValue(FName(TEXT("CategoryTag")), CategoryStr) && !CategoryStr.IsEmpty())
		{
			Entry.CategoryTag = FGameplayTag::RequestGameplayTag(FName(*CategoryStr), false);
		}

		// EN: Use asset name as display name fallback
		// ES: Usar nombre del asset como display name fallback
		Entry.DisplayName = FText::FromName(AssetData.AssetName);

		if (Entry.ItemTag.IsValid())
		{
			// EN: Update category index / ES: Actualizar indice de categoria
			if (Entry.CategoryTag.IsValid())
			{
				Database.CategoryIndex.Add(Entry.CategoryTag, Entry.ItemTag);
			}
			Database.Entries.Add(Entry.ItemTag, MoveTemp(Entry));
		}
		else
		{
			UE_LOG(LogPGXRegistry, Verbose, TEXT("AutoDiscover: Skipped [%s] — could not create valid tag"),
				*AssetData.AssetName.ToString());
		}
	}
}

FPGXRegistryEntry UPGXDataRegistrySubsystem::BuildEntryFromAsset(UPGXDataAsset* Asset, FGameplayTag ItemTag) const
{
	FPGXRegistryEntry Entry;
	Entry.ItemTag = ItemTag;
	Entry.AssetId = Asset->AssetId;
	Entry.Version = Asset->Version;
	Entry.AssetPath = FSoftObjectPath(Asset);

	// EN: Extract category/display from object DA if applicable
	// ES: Extraer categoria/display del object DA si aplica
	if (const UPGXObjectDataAsset* ObjectDA = Cast<UPGXObjectDataAsset>(Asset))
	{
		Entry.CategoryTag = ObjectDA->CategoryTag;
		Entry.DisplayName = ObjectDA->DisplayName;
	}
	else if (const UPGXConfigDataAsset* ConfigDA = Cast<UPGXConfigDataAsset>(Asset))
	{
		Entry.DisplayName = ConfigDA->ConfigDisplayName;
	}
	else
	{
		// EN: Fallback to asset name / ES: Fallback al nombre del asset
		Entry.DisplayName = FText::FromString(Asset->GetName());
	}

	return Entry;
}

// ═══════════════════════════════════════════════════════════════
// v2.0 DataTable Ingest / Ingesta de DataTable v2.0
// ═══════════════════════════════════════════════════════════════

void UPGXDataRegistrySubsystem::IngestAllDefinitions()
{
	IAssetRegistry& AssetRegistry = UAssetManager::Get().GetAssetRegistry();

	TArray<FAssetData> FoundDefinitions;
	AssetRegistry.GetAssetsByClass(UPGXRegistryDefinition::StaticClass()->GetClassPathName(), FoundDefinitions, true);

	const UPGXRegistrySettings* Settings = GetDefault<UPGXRegistrySettings>();

	// EN: Filter by scan roots if configured / ES: Filtrar por raices de escaneo si configurado
	if (Settings && Settings->ScanRoots.Num() > 0)
	{
		FoundDefinitions.RemoveAll([&Settings](const FAssetData& AssetData)
		{
			const FString AssetPath = AssetData.GetObjectPathString();
			for (const FDirectoryPath& Root : Settings->ScanRoots)
			{
				if (AssetPath.StartsWith(Root.Path))
				{
					return false; // EN: Keep it / ES: Mantenerlo
				}
			}
			return true; // EN: Remove if not in any scan root / ES: Remover si no esta en ninguna raiz
		});
	}

	UE_LOG(LogPGXRegistry, Log, TEXT("v2.0 IngestAllDefinitions: Found %d definition(s)"), FoundDefinitions.Num());

	for (const FAssetData& AssetData : FoundDefinitions)
	{
		UPGXRegistryDefinition* Definition = Cast<UPGXRegistryDefinition>(AssetData.GetAsset());
		if (Definition && Definition->IsValidForIngestion())
		{
			IngestDefinition(Definition);
		}
		else
		{
			UE_LOG(LogPGXRegistry, Warning, TEXT("IngestAllDefinitions: Skipped invalid definition [%s]"),
				*AssetData.AssetName.ToString());
		}
	}
}

bool UPGXDataRegistrySubsystem::IngestDefinition(const UPGXRegistryDefinition* Definition)
{
	if (!Definition || !Definition->IsValidForIngestion())
	{
		UE_LOG(LogPGXRegistry, Warning, TEXT("IngestDefinition: Invalid or null definition"));
		return false;
	}

	const FGameplayTag& DBTag = Definition->DatabaseTag;

	// EN: Create the v1.0 database container if it doesn't exist yet
	// ES: Crear el contenedor de BD v1.0 si no existe aun
	if (!Databases.Contains(DBTag))
	{
		TUniquePtr<FPGXDataDatabase> NewDB = MakeUnique<FPGXDataDatabase>();
		NewDB->DatabaseTag = DBTag;
		NewDB->AssetClass = Definition->ExpectedAssetClass ? Definition->ExpectedAssetClass.Get() : UPGXDataAsset::StaticClass();
		NewDB->bAutoDiscovered = false;
		Databases.Add(DBTag, MoveTemp(NewDB));
		OnDatabaseCreated.Broadcast(DBTag);
	}

	int32 TotalIngested = 0;

	for (const TSoftObjectPtr<UDataTable>& TableRef : Definition->DataTables)
	{
		UDataTable* Table = TableRef.LoadSynchronous();
		if (!Table)
		{
			UE_LOG(LogPGXRegistry, Warning, TEXT("IngestDefinition [%s]: Failed to load DataTable [%s]"),
				*DBTag.ToString(), *TableRef.ToString());
			continue;
		}

		const int32 BeforeCount = EntryIndex.Num();
		IngestDataTable(DBTag, Table, Definition->ConflictPolicy);
		TotalIngested += (EntryIndex.Num() - BeforeCount);
	}

	// EN: Track active definition / ES: Rastrear definicion activa
	ActiveDefinitions.Add(TWeakObjectPtr<const UPGXRegistryDefinition>(Definition));

	OnTablesIngested.Broadcast(DBTag, TotalIngested);

	UE_LOG(LogPGXRegistry, Log, TEXT("IngestDefinition [%s]: Ingested %d entries from %d table(s)"),
		*DBTag.ToString(), TotalIngested, Definition->DataTables.Num());

	return TotalIngested > 0;
}

bool UPGXDataRegistrySubsystem::IngestDataTable(FGameplayTag DatabaseTag, const UDataTable* DataTable,
	EPGXRegistryConflictPolicy Policy)
{
	if (!DataTable)
	{
		return false;
	}

	// EN: Verify row struct is FPGXRegistryCategoryRow / ES: Verificar que el row struct sea FPGXRegistryCategoryRow
	if (!DataTable->GetRowStruct() || !DataTable->GetRowStruct()->IsChildOf(FPGXRegistryCategoryRow::StaticStruct()))
	{
		UE_LOG(LogPGXRegistry, Warning,
			TEXT("IngestDataTable [%s]: Table [%s] has incompatible row struct [%s]. Expected FPGXRegistryCategoryRow."),
			*DatabaseTag.ToString(), *DataTable->GetName(),
			DataTable->GetRowStruct() ? *DataTable->GetRowStruct()->GetName() : TEXT("null"));
		return false;
	}

	const UPGXRegistrySettings* Settings = GetDefault<UPGXRegistrySettings>();
	const bool bVerbose = Settings && Settings->bVerboseIngestLogging;

	int32 IngestedCount = 0;
	const TMap<FName, uint8*>& RowMap = DataTable->GetRowMap();

	for (const auto& RowPair : RowMap)
	{
		const FPGXRegistryCategoryRow* Row = reinterpret_cast<const FPGXRegistryCategoryRow*>(RowPair.Value);
		if (!Row)
		{
			continue;
		}

		if (!Row->CategoryTag.IsValid())
		{
			UE_LOG(LogPGXRegistry, Warning, TEXT("IngestDataTable: Row [%s] in [%s] has invalid CategoryTag"),
				*RowPair.Key.ToString(), *DataTable->GetName());
			continue;
		}

		// EN: Process each item in the category / ES: Procesar cada item en la categoria
		for (const auto& ItemPair : Row->ItemsByTag)
		{
			const FGameplayTag& ItemTag = ItemPair.Key;
			const TSoftObjectPtr<UPGXDataAsset>& AssetRef = ItemPair.Value;

			if (!ItemTag.IsValid())
			{
				UE_LOG(LogPGXRegistry, Warning, TEXT("IngestDataTable: Invalid ItemTag in row [%s] of [%s]"),
					*RowPair.Key.ToString(), *DataTable->GetName());
				continue;
			}

			if (AssetRef.IsNull())
			{
				UE_LOG(LogPGXRegistry, Warning, TEXT("IngestDataTable: Null asset reference for ItemTag [%s] in row [%s] of [%s]"),
					*ItemTag.ToString(), *RowPair.Key.ToString(), *DataTable->GetName());
				continue;
			}

			// EN: Build composite key / ES: Construir clave compuesta
			FPGXCompositeRegistryKey CompositeKey;
			CompositeKey.DatabaseTag = DatabaseTag;
			CompositeKey.CategoryTag = Row->CategoryTag;
			CompositeKey.ItemTag = ItemTag;

			// EN: Check for conflict / ES: Verificar conflicto
			if (EntryIndex.Contains(CompositeKey))
			{
				switch (Policy)
				{
				case EPGXRegistryConflictPolicy::FailOnConflict:
					UE_LOG(LogPGXRegistry, Error,
						TEXT("IngestDataTable: Conflict for key [%s] in table [%s]. Policy=FailOnConflict, skipping."),
						*CompositeKey.ToString(), *DataTable->GetName());
					continue;

				case EPGXRegistryConflictPolicy::FirstWins:
					if (bVerbose)
					{
						UE_LOG(LogPGXRegistry, Verbose,
							TEXT("IngestDataTable: Duplicate key [%s] — FirstWins, keeping existing."),
							*CompositeKey.ToString());
					}
					continue;

				case EPGXRegistryConflictPolicy::LastWins:
					if (bVerbose)
					{
						UE_LOG(LogPGXRegistry, Verbose,
							TEXT("IngestDataTable: Duplicate key [%s] — LastWins, overwriting."),
							*CompositeKey.ToString());
					}
					RemoveCompositeIndexEntry(CompositeKey);
					break;
				}
			}

			if (TUniquePtr<FPGXDataDatabase>* FoundDB = Databases.Find(DatabaseTag))
			{
				if (const FPGXRegistryEntry* ExistingDBEntry = (*FoundDB)->Entries.Find(ItemTag))
				{
					const bool bSameEntry = ExistingDBEntry->CategoryTag == Row->CategoryTag
						&& ExistingDBEntry->AssetPath == AssetRef.ToSoftObjectPath();
					if (!bSameEntry)
					{
						switch (Policy)
						{
						case EPGXRegistryConflictPolicy::FailOnConflict:
							UE_LOG(LogPGXRegistry, Error,
								TEXT("IngestDataTable: Conflict for item [%s] in database [%s]. Policy=FailOnConflict, skipping."),
								*ItemTag.ToString(), *DatabaseTag.ToString());
							continue;

						case EPGXRegistryConflictPolicy::FirstWins:
							if (bVerbose)
							{
								UE_LOG(LogPGXRegistry, Verbose,
									TEXT("IngestDataTable: Duplicate item [%s] — FirstWins, keeping existing."),
									*ItemTag.ToString());
							}
							continue;

						case EPGXRegistryConflictPolicy::LastWins:
							if (ExistingDBEntry->CategoryTag.IsValid())
							{
								(*FoundDB)->CategoryIndex.RemoveSingle(ExistingDBEntry->CategoryTag, ItemTag);
							}
							RemoveCompositeIndexesForItem(DatabaseTag, ItemTag);
							break;
						}
					}
				}
			}

			// EN: Build entry / ES: Construir entrada
			FPGXRegistryEntry Entry;
			Entry.ItemTag = ItemTag;
			Entry.CategoryTag = Row->CategoryTag;
			Entry.AssetPath = AssetRef.ToSoftObjectPath();
			Entry.DisplayName = FText::FromString(ItemTag.GetTagName().ToString());
			// EN: Version and AssetId resolved on load / ES: Version y AssetId resueltos al cargar

			// EN: Add to global indexes / ES: Agregar a indices globales
			AddCompositeIndexEntry(CompositeKey, Entry, DataTable);

			// EN: Also populate the v1.0 database for backward compatibility
			// ES: Tambien poblar la BD v1.0 para compatibilidad hacia atras
			if (TUniquePtr<FPGXDataDatabase>* FoundDB = Databases.Find(DatabaseTag))
			{
				FPGXDataDatabase& DB = **FoundDB;
				DB.Entries.Add(ItemTag, Entry);

				if (Entry.CategoryTag.IsValid())
				{
					DB.CategoryIndex.RemoveSingle(Entry.CategoryTag, ItemTag);
					DB.CategoryIndex.Add(Entry.CategoryTag, ItemTag);
				}
			}

			++IngestedCount;

			if (bVerbose)
			{
				UE_LOG(LogPGXRegistry, Verbose, TEXT("  Ingested: %s"), *CompositeKey.ToString());
			}
		}
	}

	UE_LOG(LogPGXRegistry, Log, TEXT("IngestDataTable [%s]: Table [%s] — %d entries ingested"),
		*DatabaseTag.ToString(), *DataTable->GetName(), IngestedCount);

	return IngestedCount > 0;
}

// ═══════════════════════════════════════════════════════════════
// v2.0 Composite Key Query / Consulta por Clave Compuesta v2.0
// ═══════════════════════════════════════════════════════════════

const FPGXRegistryEntry* UPGXDataRegistrySubsystem::FindByCompositeKey(const FPGXCompositeRegistryKey& Key) const
{
	return EntryIndex.Find(Key);
}

TArray<FPGXRegistryEntry> UPGXDataRegistrySubsystem::GetCategoryEntries(FGameplayTag DatabaseTag, FGameplayTag CategoryTag) const
{
	TArray<FPGXRegistryEntry> Result;

	const uint32 CatKey = MakeCategoryKey(DatabaseTag, CategoryTag);
	const FPGXRegistryCategoryBucket* Bucket = V2CategoryIndex.Find(CatKey);
	if (!Bucket)
	{
		return Result;
	}

	Result.Reserve(Bucket->ItemTags.Num());
	for (const FGameplayTag& ItemTag : Bucket->ItemTags)
	{
		FPGXCompositeRegistryKey Key;
		Key.DatabaseTag = DatabaseTag;
		Key.CategoryTag = CategoryTag;
		Key.ItemTag = ItemTag;

		if (const FPGXRegistryEntry* Entry = EntryIndex.Find(Key))
		{
			Result.Add(*Entry);
		}
	}

	return Result;
}

TArray<const UPGXRegistryDefinition*> UPGXDataRegistrySubsystem::GetActiveDefinitions() const
{
	TArray<const UPGXRegistryDefinition*> Result;
	for (const TWeakObjectPtr<const UPGXRegistryDefinition>& WeakDef : ActiveDefinitions)
	{
		if (const UPGXRegistryDefinition* Def = WeakDef.Get())
		{
			Result.Add(Def);
		}
	}
	return Result;
}

uint32 UPGXDataRegistrySubsystem::MakeCategoryKey(const FGameplayTag& DatabaseTag, const FGameplayTag& CategoryTag)
{
	return HashCombine(GetTypeHash(DatabaseTag), GetTypeHash(CategoryTag));
}

FPGXRegistryReverseKey UPGXDataRegistrySubsystem::MakeReverseKey(const FGameplayTag& DatabaseTag, const FGameplayTag& ItemTag)
{
	FPGXRegistryReverseKey ReverseKey;
	ReverseKey.DatabaseTag = DatabaseTag;
	ReverseKey.ItemTag = ItemTag;
	return ReverseKey;
}

void UPGXDataRegistrySubsystem::AddCompositeIndexEntry(const FPGXCompositeRegistryKey& Key, const FPGXRegistryEntry& Entry, const UDataTable* SourceTable)
{
	RemoveCompositeIndexEntry(Key);

	EntryIndex.Add(Key, Entry);
	CompositeReverseIndex.Add(MakeReverseKey(Key.DatabaseTag, Key.ItemTag), Key);

	const uint32 CatKey = MakeCategoryKey(Key.DatabaseTag, Key.CategoryTag);
	FPGXRegistryCategoryBucket& Bucket = V2CategoryIndex.FindOrAdd(CatKey);
	Bucket.ItemTags.AddUnique(Key.ItemTag);
	if (SourceTable && !Bucket.SourceTables.ContainsByPredicate([SourceTable](const TSoftObjectPtr<UDataTable>& Ref)
	{
		return Ref.Get() == SourceTable;
	}))
	{
		Bucket.SourceTables.Add(TSoftObjectPtr<UDataTable>(const_cast<UDataTable*>(SourceTable)));
	}
}

void UPGXDataRegistrySubsystem::RemoveCompositeIndexEntry(const FPGXCompositeRegistryKey& Key)
{
	EntryIndex.Remove(Key);
	CompositeReverseIndex.RemoveSingle(MakeReverseKey(Key.DatabaseTag, Key.ItemTag), Key);

	const uint32 CatKey = MakeCategoryKey(Key.DatabaseTag, Key.CategoryTag);
	if (FPGXRegistryCategoryBucket* Bucket = V2CategoryIndex.Find(CatKey))
	{
		Bucket->ItemTags.Remove(Key.ItemTag);
		if (Bucket->ItemTags.Num() == 0)
		{
			V2CategoryIndex.Remove(CatKey);
		}
	}
}

TArray<FPGXCompositeRegistryKey> UPGXDataRegistrySubsystem::FindCompositeKeysForItem(const FGameplayTag& DatabaseTag, const FGameplayTag& ItemTag) const
{
	TArray<FPGXCompositeRegistryKey> Keys;
	CompositeReverseIndex.MultiFind(MakeReverseKey(DatabaseTag, ItemTag), Keys);
	return Keys;
}

void UPGXDataRegistrySubsystem::RemoveCompositeIndexesForItem(const FGameplayTag& DatabaseTag, const FGameplayTag& ItemTag)
{
	const TArray<FPGXCompositeRegistryKey> Keys = FindCompositeKeysForItem(DatabaseTag, ItemTag);
	for (const FPGXCompositeRegistryKey& Key : Keys)
	{
		RemoveCompositeIndexEntry(Key);
	}
}

void UPGXDataRegistrySubsystem::SyncCompositeCachedAsset(const FGameplayTag& DatabaseTag, const FGameplayTag& ItemTag, UPGXDataAsset* Asset)
{
	const TArray<FPGXCompositeRegistryKey> Keys = FindCompositeKeysForItem(DatabaseTag, ItemTag);
	for (const FPGXCompositeRegistryKey& Key : Keys)
	{
		if (FPGXRegistryEntry* CompositeEntry = EntryIndex.Find(Key))
		{
			CompositeEntry->CachedAsset = Asset;
		}
	}
}

// ═══════════════════════════════════════════════════════════════
// Console Commands / Comandos de Consola
// ═══════════════════════════════════════════════════════════════

void UPGXDataRegistrySubsystem::ExecuteConsoleCommand(const FString& CommandName, const TArray<FString>& Args, UWorld* World)
{
	if (CommandName == TEXT("pgx.registry.definitions"))
	{
		TArray<const UPGXRegistryDefinition*> Defs = GetActiveDefinitions();
		if (Defs.Num() == 0)
		{
			UE_LOG(LogPGXRegistry, Display, TEXT("No active registry definitions"));
			return;
		}
		UE_LOG(LogPGXRegistry, Display, TEXT("═══ Active Registry Definitions: %d ═══"), Defs.Num());
		for (const UPGXRegistryDefinition* Def : Defs)
		{
			UE_LOG(LogPGXRegistry, Display, TEXT("  [%s] Tables=%d Policy=%s Asset=%s"),
				*Def->DatabaseTag.ToString(),
				Def->DataTables.Num(),
				*UEnum::GetValueAsString(Def->ConflictPolicy),
				Def->ExpectedAssetClass ? *Def->ExpectedAssetClass->GetName() : TEXT("Any"));
		}
		return;
	}
	if (CommandName == TEXT("pgx.registry.export"))
	{
		if (Args.Num() < 1)
		{
			UE_LOG(LogPGXRegistry, Display, TEXT("Usage: pgx.registry.export <DatabaseTag>"));
			return;
		}
		FGameplayTag DBTag = FGameplayTag::RequestGameplayTag(FName(*Args[0]), false);
		if (!DBTag.IsValid() || !Databases.Contains(DBTag))
		{
			UE_LOG(LogPGXRegistry, Display, TEXT("Database [%s] not found"), *Args[0]);
			return;
		}
		FString Json = ExportMetadata(DBTag);
		UE_LOG(LogPGXRegistry, Display, TEXT("%s"), *Json);
		return;
	}
	if (CommandName == TEXT("pgx.registry.export.all"))
	{
		FString Json = ExportAllMetadata();
		UE_LOG(LogPGXRegistry, Display, TEXT("%s"), *Json);
		return;
	}
	if (CommandName == TEXT("pgx.registry.ingest"))
	{
		EntryIndex.Empty();
		CompositeReverseIndex.Empty();
		V2CategoryIndex.Empty();
		ActiveDefinitions.Empty();
		IngestAllDefinitions();
		UE_LOG(LogPGXRegistry, Display, TEXT("v2.0 Re-ingest complete | EntryIndex=%d"), EntryIndex.Num());
		return;
	}
	if (CommandName == TEXT("pgx.registry.list"))
	{
		if (Databases.Num() == 0)
		{
			UE_LOG(LogPGXRegistry, Display, TEXT("No databases registered"));
			return;
		}
		UE_LOG(LogPGXRegistry, Display, TEXT("═══ PGX Data Registry: %d Databases ═══"), Databases.Num());
		for (const auto& Pair : Databases)
		{
			const FPGXDataDatabase& DB = *Pair.Value;
			UE_LOG(LogPGXRegistry, Display, TEXT("  [%s] Class=%s Entries=%d Loaded=%d AutoDiscovered=%s"),
				*Pair.Key.ToString(),
				DB.AssetClass ? *DB.AssetClass->GetName() : TEXT("None"),
				DB.Entries.Num(),
				DB.GetLoadedCount(),
				DB.bAutoDiscovered ? TEXT("Yes") : TEXT("No"));
		}
		return;
	}
	if (CommandName == TEXT("pgx.registry.stats"))
	{
		if (Args.Num() < 1)
		{
			UE_LOG(LogPGXRegistry, Display, TEXT("Usage: pgx.registry.stats <DatabaseTag>"));
			return;
		}
		FGameplayTag DBTag = FGameplayTag::RequestGameplayTag(FName(*Args[0]), false);
		if (!DBTag.IsValid() || !Databases.Contains(DBTag))
		{
			UE_LOG(LogPGXRegistry, Display, TEXT("Database [%s] not found"), *Args[0]);
			return;
		}
		FPGXDatabaseStats Stats = GetDatabaseStats(DBTag);
		UE_LOG(LogPGXRegistry, Display, TEXT("═══ Database: %s ═══"), *Stats.DatabaseTag.ToString());
		UE_LOG(LogPGXRegistry, Display, TEXT("  Class: %s"), *Stats.AssetClassName.ToString());
		UE_LOG(LogPGXRegistry, Display, TEXT("  Entries: %d"), Stats.TotalEntries);
		UE_LOG(LogPGXRegistry, Display, TEXT("  Loaded: %d"), Stats.LoadedEntries);
		UE_LOG(LogPGXRegistry, Display, TEXT("  Categories: %d"), Stats.CategoryCount);
		UE_LOG(LogPGXRegistry, Display, TEXT("  AutoDiscovered: %s"), Stats.bAutoDiscovered ? TEXT("Yes") : TEXT("No"));
		return;
	}
	if (CommandName == TEXT("pgx.registry.validate"))
	{
		// EN: Validation is exposed through the Registry editor tools in this preview.
		// ES: La validacion se ofrece mediante las herramientas de editor del registro.
		UE_LOG(LogPGXRegistry, Display, TEXT("pgx.registry.validate: use the Registry editor validation tools"));
		return;
	}
}


// ═══════════════════════════════════════════════════════════════
// Profile Integration
// ═══════════════════════════════════════════════════════════════

void UPGXDataRegistrySubsystem::HandleProfileChanged(const FPGXResolvedProfile& /*OldProfile*/, const FPGXResolvedProfile& /*NewProfile*/)
{
	// EN: DataRegistry has no platform budgets; the binding is notification-only.
	// ES: DataRegistry no tiene budgets de plataforma; el binding solo notifica.
	UE_LOG(LogPGXRegistry, Log, TEXT("[DataRegistry] Profile changed — no platform budgets to enforce."));
}
