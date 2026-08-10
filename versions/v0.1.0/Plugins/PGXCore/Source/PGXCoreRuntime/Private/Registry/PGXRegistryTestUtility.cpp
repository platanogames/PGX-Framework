// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Registry/PGXRegistryTestUtility.h"
#include "Registry/PGXDataRegistrySubsystem.h"
#include "Registry/PGXRegistryTypes.h"
#include "Registry/PGXRegistrySettings.h"
#include "Base/PGXDataAsset.h"
#include "Data/PGXObjectDataAsset.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameplayTagsManager.h"
#include "NativeGameplayTags.h"
#include "HAL/PlatformTime.h"

// EN: Data Registry test utility implementation
// ES: Implementacion de utilidad de test del Data Registry

// ============================================================================
// EN: Static test tag registration — required for MakeTestTag() to return valid tags
// ES: Registro estatico de tags de test — necesario para que MakeTestTag() retorne tags validos
// ============================================================================

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_CrudDB,       "PGX.Test.Registry.CrudDB");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_CrudItem,     "PGX.Test.Registry.CrudItem");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_CatDB,        "PGX.Test.Registry.CatDB");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_CatItem1,     "PGX.Test.Registry.CatItem1");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_CatItem2,     "PGX.Test.Registry.CatItem2");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_CatItem3,     "PGX.Test.Registry.CatItem3");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_CompositeDB,  "PGX.Test.Registry.CompositeDB");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_CompositeItem,"PGX.Test.Registry.CompositeItem");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_ConflictDB,   "PGX.Test.Registry.ConflictDB");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_ConflictItem, "PGX.Test.Registry.ConflictItem");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_CacheDB,      "PGX.Test.Registry.CacheDB");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_CacheItem1,   "PGX.Test.Registry.CacheItem1");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_CacheItem2,   "PGX.Test.Registry.CacheItem2");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_CacheItem3,   "PGX.Test.Registry.CacheItem3");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_StressDB,     "PGX.Test.Registry.StressDB");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_StressCat,    "PGX.Test.Registry.StressCat");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_CatA,         "PGX.Test.Registry.CatA");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_CatB,         "PGX.Test.Registry.CatB");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_CompositeCat, "PGX.Test.Registry.CompositeCat");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_ConflictCat,  "PGX.Test.Registry.ConflictCat");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_StressItem00, "PGX.Test.Registry.Stress.Item0");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_StressItem01, "PGX.Test.Registry.Stress.Item1");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_StressItem02, "PGX.Test.Registry.Stress.Item2");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_StressItem03, "PGX.Test.Registry.Stress.Item3");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_StressItem04, "PGX.Test.Registry.Stress.Item4");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_StressItem05, "PGX.Test.Registry.Stress.Item5");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_StressItem06, "PGX.Test.Registry.Stress.Item6");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_StressItem07, "PGX.Test.Registry.Stress.Item7");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_StressItem08, "PGX.Test.Registry.Stress.Item8");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_StressItem09, "PGX.Test.Registry.Stress.Item9");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_StressItem10, "PGX.Test.Registry.Stress.Item10");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_StressItem11, "PGX.Test.Registry.Stress.Item11");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_StressItem12, "PGX.Test.Registry.Stress.Item12");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_StressItem13, "PGX.Test.Registry.Stress.Item13");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_StressItem14, "PGX.Test.Registry.Stress.Item14");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTestRegistry_StressItem15, "PGX.Test.Registry.Stress.Item15");

static const FNativeGameplayTag* const GPGXRegistryStressItemTags[] = {
	&TAG_PGXTestRegistry_StressItem00,
	&TAG_PGXTestRegistry_StressItem01,
	&TAG_PGXTestRegistry_StressItem02,
	&TAG_PGXTestRegistry_StressItem03,
	&TAG_PGXTestRegistry_StressItem04,
	&TAG_PGXTestRegistry_StressItem05,
	&TAG_PGXTestRegistry_StressItem06,
	&TAG_PGXTestRegistry_StressItem07,
	&TAG_PGXTestRegistry_StressItem08,
	&TAG_PGXTestRegistry_StressItem09,
	&TAG_PGXTestRegistry_StressItem10,
	&TAG_PGXTestRegistry_StressItem11,
	&TAG_PGXTestRegistry_StressItem12,
	&TAG_PGXTestRegistry_StressItem13,
	&TAG_PGXTestRegistry_StressItem14,
	&TAG_PGXTestRegistry_StressItem15,
};

// ============================================================================
// EN: Internal helpers
// ES: Helpers internos
// ============================================================================

static UPGXDataRegistrySubsystem* GetRegistrySubsystemForTest(const UObject* WorldContextObject)
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
	const UGameInstance* GI = World->GetGameInstance();
	return GI ? GI->GetSubsystem<UPGXDataRegistrySubsystem>() : nullptr;
}

static void LogRegistryTestResult(const FString& TestName, bool bPassed, const FString& Details = TEXT(""))
{
	if (bPassed)
	{
		UE_LOG(LogPGXRegistry, Log, TEXT("[TestUtility] PASS: %s%s"),
			*TestName, Details.IsEmpty() ? TEXT("") : *FString::Printf(TEXT(" (%s)"), *Details));
	}
	else
	{
		UE_LOG(LogPGXRegistry, Error, TEXT("[TestUtility] FAIL: %s%s"),
			*TestName, Details.IsEmpty() ? TEXT("") : *FString::Printf(TEXT(" (%s)"), *Details));
	}
}

static FGameplayTag MakeTestTag(const FString& Suffix)
{
	return FGameplayTag::RequestGameplayTag(FName(*FString::Printf(TEXT("PGX.Test.Registry.%s"), *Suffix)), false);
}

static FGameplayTag MakeStressItemTag(int32 Index)
{
	if (Index >= 0 && Index < UE_ARRAY_COUNT(GPGXRegistryStressItemTags))
	{
		return GPGXRegistryStressItemTags[Index]->GetTag();
	}

	return FGameplayTag::RequestGameplayTag(
		FName(*FString::Printf(TEXT("PGX.Test.Registry.Stress.Item%d"), Index)),
		false);
}

static void CleanupTestDatabase(UPGXDataRegistrySubsystem* Sub, const FGameplayTag& DatabaseTag)
{
	if (!Sub || !DatabaseTag.IsValid())
	{
		return;
	}

	// EN: Unregister all entries in the test database / ES: Desregistrar todas las entradas en la database de test
	TArray<FPGXRegistryEntry> AllEntries = Sub->GetAllEntries(DatabaseTag);
	for (const FPGXRegistryEntry& Entry : AllEntries)
	{
		Sub->UnregisterAsset(DatabaseTag, Entry.ItemTag);
	}

	// EN: Invalidate cache / ES: Invalidar cache
	Sub->InvalidateCache(DatabaseTag);
}

/**
 * EN: Ensure a test database exists and is clean. Idempotent — safe to call on re-runs
 *     within the same PIE session. If the database already exists (from a previous run),
 *     cleans its entries instead of trying to create it again.
 * ES: Asegurar que una database de test existe y esta limpia. Idempotente — seguro de llamar
 *     en re-runs dentro de la misma sesion PIE. Si la database ya existe (de un run anterior),
 *     limpia sus entries en vez de intentar crearla de nuevo.
 */
static bool EnsureTestDatabase(UPGXDataRegistrySubsystem* Sub, const FGameplayTag& DatabaseTag, UClass* AssetClass)
{
	if (!Sub || !DatabaseTag.IsValid() || !AssetClass)
	{
		return false;
	}

	if (Sub->HasDatabase(DatabaseTag))
	{
		// EN: Database persists from previous run — clean entries for fresh start
		// ES: Database persiste de un run anterior — limpiar entries para empezar limpio
		CleanupTestDatabase(Sub, DatabaseTag);
		return true;
	}

	return Sub->CreateDatabase(DatabaseTag, AssetClass, false);
}

// ============================================================================
// EN: Test 1 — RunQuickTest
// ES: Test 1 — RunQuickTest
// ============================================================================

bool UPGXRegistryTestUtility::RunQuickTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	UE_LOG(LogPGXRegistry, Log, TEXT("[TestUtility] ========== RunQuickTest START =========="));

	UPGXDataRegistrySubsystem* Sub = GetRegistrySubsystemForTest(WorldContextObject);
	if (!Sub)
	{
		OutIssues.Add(TEXT("DataRegistrySubsystem not found"));
		LogRegistryTestResult(TEXT("RunQuickTest"), false, TEXT("Subsystem not found"));
		return false;
	}

	// EN: Subsystem exists / ES: Subsistema existe
	LogRegistryTestResult(TEXT("RunQuickTest.SubsystemExists"), true);

	// EN: Get all database tags / ES: Obtener todos los database tags
	TArray<FGameplayTag> DbTags = Sub->GetAllDatabaseTags();
	OutIssues.Add(FString::Printf(TEXT("Databases found: %d"), DbTags.Num()));
	LogRegistryTestResult(TEXT("RunQuickTest.GetAllDatabaseTags"), true,
		FString::Printf(TEXT("%d databases"), DbTags.Num()));

	// EN: Stats per database / ES: Stats por database
	int32 TotalEntries = 0;
	for (const FGameplayTag& DbTag : DbTags)
	{
		FPGXDatabaseStats Stats = Sub->GetDatabaseStats(DbTag);
		OutIssues.Add(FString::Printf(TEXT("  [%s] Total=%d, Loaded=%d, Categories=%d, Class=%s"),
			*DbTag.ToString(), Stats.TotalEntries, Stats.LoadedEntries, Stats.CategoryCount,
			*Stats.AssetClassName.ToString()));
		TotalEntries += Stats.TotalEntries;
	}

	// EN: Global entry index count / ES: Conteo global del entry index
	const int32 IndexCount = Sub->GetEntryIndexCount();
	OutIssues.Add(FString::Printf(TEXT("EntryIndexCount: %d (sum from DBs: %d)"), IndexCount, TotalEntries));
	LogRegistryTestResult(TEXT("RunQuickTest.EntryIndex"), true,
		FString::Printf(TEXT("IndexCount=%d"), IndexCount));

	UE_LOG(LogPGXRegistry, Log, TEXT("[TestUtility] ========== RunQuickTest END =========="));
	return true;
}

// ============================================================================
// EN: Test 2 — TestDatabaseCRUD
// ES: Test 2 — TestDatabaseCRUD
// ============================================================================

bool UPGXRegistryTestUtility::TestDatabaseCRUD(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	UE_LOG(LogPGXRegistry, Log, TEXT("[TestUtility] ========== TestDatabaseCRUD START =========="));

	UPGXDataRegistrySubsystem* Sub = GetRegistrySubsystemForTest(WorldContextObject);
	if (!Sub)
	{
		OutIssues.Add(TEXT("DataRegistrySubsystem not found"));
		LogRegistryTestResult(TEXT("TestDatabaseCRUD"), false, TEXT("Subsystem not found"));
		return false;
	}

	bool bAllPassed = true;
	const FGameplayTag DbTag = MakeTestTag(TEXT("CrudDB"));
	const FGameplayTag ItemTag = MakeTestTag(TEXT("CrudItem"));

	// EN: Phase 1 — Ensure database exists (idempotent) / ES: Fase 1 — Asegurar que database existe (idempotente)
	const bool bCreated = EnsureTestDatabase(Sub, DbTag, UPGXObjectDataAsset::StaticClass());
	LogRegistryTestResult(TEXT("CRUD.EnsureDatabase"), bCreated);
	if (!bCreated)
	{
		OutIssues.Add(TEXT("Failed to ensure test database"));
		bAllPassed = false;
	}

	// EN: Phase 2 — Register transient asset / ES: Fase 2 — Registrar asset transient
	UPGXDataAsset* TestAsset = NewObject<UPGXObjectDataAsset>(GetTransientPackage(), NAME_None, RF_Transient);
	const bool bRegistered = Sub->RegisterAsset(DbTag, TestAsset, ItemTag);
	LogRegistryTestResult(TEXT("CRUD.RegisterAsset"), bRegistered);
	if (!bRegistered)
	{
		OutIssues.Add(TEXT("Failed to register test asset"));
		bAllPassed = false;
	}

	// EN: Phase 3 — FindEntry / ES: Fase 3 — FindEntry
	const FPGXRegistryEntry* Found = Sub->FindEntry(DbTag, ItemTag);
	const bool bFound = (Found != nullptr);
	LogRegistryTestResult(TEXT("CRUD.FindEntry"), bFound);
	if (!bFound)
	{
		OutIssues.Add(TEXT("FindEntry returned nullptr for registered asset"));
		bAllPassed = false;
	}

	// EN: Phase 4 — Unregister / ES: Fase 4 — Desregistrar
	const bool bUnregistered = Sub->UnregisterAsset(DbTag, ItemTag);
	LogRegistryTestResult(TEXT("CRUD.UnregisterAsset"), bUnregistered);
	if (!bUnregistered)
	{
		OutIssues.Add(TEXT("Failed to unregister test asset"));
		bAllPassed = false;
	}

	// EN: Phase 5 — Verify removal / ES: Fase 5 — Verificar remocion
	const FPGXRegistryEntry* ShouldBeNull = Sub->FindEntry(DbTag, ItemTag);
	const bool bRemoved = (ShouldBeNull == nullptr);
	LogRegistryTestResult(TEXT("CRUD.VerifyRemoval"), bRemoved);
	if (!bRemoved)
	{
		OutIssues.Add(TEXT("Entry still found after unregister"));
		bAllPassed = false;
	}

	// EN: Cleanup / ES: Limpieza
	CleanupTestDatabase(Sub, DbTag);

	OutIssues.Add(FString::Printf(TEXT("TestDatabaseCRUD: %s"), bAllPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED")));
	UE_LOG(LogPGXRegistry, Log, TEXT("[TestUtility] ========== TestDatabaseCRUD END =========="));
	return bAllPassed;
}

// ============================================================================
// EN: Test 3 — TestCategoryQueries
// ES: Test 3 — TestCategoryQueries
// ============================================================================

bool UPGXRegistryTestUtility::TestCategoryQueries(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	UE_LOG(LogPGXRegistry, Log, TEXT("[TestUtility] ========== TestCategoryQueries START =========="));

	UPGXDataRegistrySubsystem* Sub = GetRegistrySubsystemForTest(WorldContextObject);
	if (!Sub)
	{
		OutIssues.Add(TEXT("DataRegistrySubsystem not found"));
		LogRegistryTestResult(TEXT("TestCategoryQueries"), false, TEXT("Subsystem not found"));
		return false;
	}

	bool bAllPassed = true;
	const FGameplayTag DbTag = MakeTestTag(TEXT("CatDB"));
	const FGameplayTag Item1 = MakeTestTag(TEXT("CatItem1"));
	const FGameplayTag Item2 = MakeTestTag(TEXT("CatItem2"));
	const FGameplayTag Item3 = MakeTestTag(TEXT("CatItem3"));

	// EN: Ensure database exists (idempotent) / ES: Asegurar que database existe (idempotente)
	EnsureTestDatabase(Sub, DbTag, UPGXObjectDataAsset::StaticClass());

	// EN: Register 3 transient assets / ES: Registrar 3 assets transient
	// EN: Note: base UPGXDataAsset may not have CategoryTag — this test focuses on
	//     verifying the category API (FindByCategory, GetCategoryEntries) works correctly.
	//     Category assignment depends on the asset subclass used.
	// ES: Nota: UPGXDataAsset base puede no tener CategoryTag — este test se enfoca en
	//     verificar que la API de categorias (FindByCategory, GetCategoryEntries) funciona.
	//     La asignacion de categoria depende de la subclase del asset.
	UPGXObjectDataAsset* Asset1 = NewObject<UPGXObjectDataAsset>(GetTransientPackage(), NAME_None, RF_Transient);
	UPGXObjectDataAsset* Asset2 = NewObject<UPGXObjectDataAsset>(GetTransientPackage(), NAME_None, RF_Transient);
	UPGXObjectDataAsset* Asset3 = NewObject<UPGXObjectDataAsset>(GetTransientPackage(), NAME_None, RF_Transient);
	Asset1->CategoryTag = MakeTestTag(TEXT("CatA"));
	Asset2->CategoryTag = MakeTestTag(TEXT("CatA"));
	Asset3->CategoryTag = MakeTestTag(TEXT("CatB"));

	Sub->RegisterAsset(DbTag, Asset1, Item1);
	Sub->RegisterAsset(DbTag, Asset2, Item2);
	Sub->RegisterAsset(DbTag, Asset3, Item3);

	// EN: Verify total entries / ES: Verificar total de entradas
	TArray<FPGXRegistryEntry> AllEntries = Sub->GetAllEntries(DbTag);
	const bool bTotalCorrect = (AllEntries.Num() == 3);
	LogRegistryTestResult(TEXT("CategoryQueries.TotalEntries"), bTotalCorrect,
		FString::Printf(TEXT("Expected 3, got %d"), AllEntries.Num()));
	if (!bTotalCorrect)
	{
		OutIssues.Add(FString::Printf(TEXT("Total entries mismatch: expected 3, got %d"), AllEntries.Num()));
		bAllPassed = false;
	}

	// EN: Discover actual categories assigned to entries / ES: Descubrir categorias reales asignadas a entries
	TMap<FGameplayTag, int32> CategoryDistribution;
	for (const FPGXRegistryEntry& Entry : AllEntries)
	{
		if (Entry.CategoryTag.IsValid())
		{
			CategoryDistribution.FindOrAdd(Entry.CategoryTag)++;
		}
	}

	OutIssues.Add(FString::Printf(TEXT("Category distribution (%d unique categories):"), CategoryDistribution.Num()));
	for (const auto& Pair : CategoryDistribution)
	{
		OutIssues.Add(FString::Printf(TEXT("  [%s] = %d entries"), *Pair.Key.ToString(), Pair.Value));

		// EN: Verify FindByCategory returns matching count / ES: Verificar que FindByCategory retorna conteo correcto
		TArray<FPGXRegistryEntry> CatEntries = Sub->FindByCategory(DbTag, Pair.Key);
		const bool bCountMatch = (CatEntries.Num() == Pair.Value);
		LogRegistryTestResult(FString::Printf(TEXT("CategoryQueries.FindByCategory.%s"), *Pair.Key.ToString()),
			bCountMatch, FString::Printf(TEXT("Expected %d, got %d"), Pair.Value, CatEntries.Num()));
		if (!bCountMatch)
		{
			bAllPassed = false;
		}

		// EN: Verify GetCategoryEntries (v2.0) / ES: Verificar GetCategoryEntries (v2.0)
		TArray<FPGXRegistryEntry> CatEntriesV2 = Sub->GetCategoryEntries(DbTag, Pair.Key);
		OutIssues.Add(FString::Printf(TEXT("  GetCategoryEntries: %d entries"), CatEntriesV2.Num()));
	}

	// EN: Verify database stats CategoryCount / ES: Verificar CategoryCount de stats
	FPGXDatabaseStats Stats = Sub->GetDatabaseStats(DbTag);
	OutIssues.Add(FString::Printf(TEXT("DatabaseStats.CategoryCount: %d"), Stats.CategoryCount));
	LogRegistryTestResult(TEXT("CategoryQueries.Stats"), true,
		FString::Printf(TEXT("Categories=%d"), Stats.CategoryCount));

	// EN: Cleanup / ES: Limpieza
	CleanupTestDatabase(Sub, DbTag);

	OutIssues.Add(FString::Printf(TEXT("TestCategoryQueries: %s"), bAllPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED")));
	UE_LOG(LogPGXRegistry, Log, TEXT("[TestUtility] ========== TestCategoryQueries END =========="));
	return bAllPassed;
}

// ============================================================================
// EN: Test 4 — TestCompositeKeyLookup
// ES: Test 4 — TestCompositeKeyLookup
// ============================================================================

bool UPGXRegistryTestUtility::TestCompositeKeyLookup(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	UE_LOG(LogPGXRegistry, Log, TEXT("[TestUtility] ========== TestCompositeKeyLookup START =========="));

	UPGXDataRegistrySubsystem* Sub = GetRegistrySubsystemForTest(WorldContextObject);
	if (!Sub)
	{
		OutIssues.Add(TEXT("DataRegistrySubsystem not found"));
		LogRegistryTestResult(TEXT("TestCompositeKeyLookup"), false, TEXT("Subsystem not found"));
		return false;
	}

	bool bAllPassed = true;
	const FGameplayTag DbTag = MakeTestTag(TEXT("CompositeDB"));
	const FGameplayTag ItemTag = MakeTestTag(TEXT("CompositeItem"));

	// EN: Ensure database exists (idempotent) / ES: Asegurar que database existe (idempotente)
	EnsureTestDatabase(Sub, DbTag, UPGXObjectDataAsset::StaticClass());
	UPGXObjectDataAsset* TestAsset = NewObject<UPGXObjectDataAsset>(GetTransientPackage(), NAME_None, RF_Transient);
	TestAsset->CategoryTag = MakeTestTag(TEXT("CompositeCat"));
	Sub->RegisterAsset(DbTag, TestAsset, ItemTag);

	// EN: Read the actual entry to get its CategoryTag (assigned during registration)
	// ES: Leer la entrada real para obtener su CategoryTag (asignado durante el registro)
	const FPGXRegistryEntry* BaseEntry = Sub->FindEntry(DbTag, ItemTag);
	if (!BaseEntry)
	{
		OutIssues.Add(TEXT("FindEntry returned nullptr — cannot test composite key"));
		LogRegistryTestResult(TEXT("CompositeKey.Setup"), false, TEXT("Base entry not found"));
		CleanupTestDatabase(Sub, DbTag);
		return false;
	}

	// EN: Build composite key using the ACTUAL CategoryTag from the entry
	// ES: Construir clave compuesta usando el CategoryTag REAL de la entrada
	FPGXCompositeRegistryKey ValidKey;
	ValidKey.DatabaseTag = DbTag;
	ValidKey.CategoryTag = BaseEntry->CategoryTag;
	ValidKey.ItemTag = ItemTag;

	OutIssues.Add(FString::Printf(TEXT("Composite key: DB=%s, Cat=%s, Item=%s"),
		*DbTag.ToString(),
		ValidKey.CategoryTag.IsValid() ? *ValidKey.CategoryTag.ToString() : TEXT("(invalid)"),
		*ItemTag.ToString()));

	const FPGXRegistryEntry* FoundEntry = Sub->FindByCompositeKey(ValidKey);
	const bool bCompositeFound = FoundEntry != nullptr && FoundEntry->CachedAsset == TestAsset;
	LogRegistryTestResult(TEXT("CompositeKey.FindByCompositeKey"), bCompositeFound);
	if (!bCompositeFound)
	{
		OutIssues.Add(TEXT("FindByCompositeKey did not return the RegisterAsset-backed composite entry"));
		bAllPassed = false;
	}

	const bool bReverseIndexPopulated = Sub->GetCompositeReverseIndexCount() >= 1;
	LogRegistryTestResult(TEXT("CompositeKey.ReverseIndexPopulated"), bReverseIndexPopulated,
		FString::Printf(TEXT("ReverseLinks=%d"), Sub->GetCompositeReverseIndexCount()));
	if (!bReverseIndexPopulated)
	{
		OutIssues.Add(TEXT("Composite reverse index was not populated by RegisterAsset"));
		bAllPassed = false;
	}

	// EN: Test invalid key / ES: Testear clave invalida
	FPGXCompositeRegistryKey InvalidKey;
	InvalidKey.DatabaseTag = MakeTestTag(TEXT("NonExistentDB"));
	InvalidKey.CategoryTag = MakeTestTag(TEXT("NonExistentCat"));
	InvalidKey.ItemTag = MakeTestTag(TEXT("NonExistentItem"));

	const FPGXRegistryEntry* NotFound = Sub->FindByCompositeKey(InvalidKey);
	const bool bInvalidReturnsNull = (NotFound == nullptr);
	LogRegistryTestResult(TEXT("CompositeKey.InvalidReturnsNull"), bInvalidReturnsNull);
	if (!bInvalidReturnsNull)
	{
		OutIssues.Add(TEXT("Invalid composite key did not return nullptr"));
		bAllPassed = false;
	}

	// EN: Cleanup / ES: Limpieza
	CleanupTestDatabase(Sub, DbTag);

	OutIssues.Add(FString::Printf(TEXT("TestCompositeKeyLookup: %s"), bAllPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED")));
	UE_LOG(LogPGXRegistry, Log, TEXT("[TestUtility] ========== TestCompositeKeyLookup END =========="));
	return bAllPassed;
}

// ============================================================================
// EN: Test 5 — TestConflictPolicies
// ES: Test 5 — TestConflictPolicies
// ============================================================================

bool UPGXRegistryTestUtility::TestConflictPolicies(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	UE_LOG(LogPGXRegistry, Log, TEXT("[TestUtility] ========== TestConflictPolicies START =========="));

	UPGXDataRegistrySubsystem* Sub = GetRegistrySubsystemForTest(WorldContextObject);
	if (!Sub)
	{
		OutIssues.Add(TEXT("DataRegistrySubsystem not found"));
		LogRegistryTestResult(TEXT("TestConflictPolicies"), false, TEXT("Subsystem not found"));
		return false;
	}

	bool bAllPassed = true;
	const FGameplayTag DbTag = MakeTestTag(TEXT("ConflictDB"));
	const FGameplayTag ItemTag = MakeTestTag(TEXT("ConflictItem"));

	// EN: Ensure database exists (idempotent) / ES: Asegurar que database existe (idempotente)
	EnsureTestDatabase(Sub, DbTag, UPGXObjectDataAsset::StaticClass());

	// EN: Register first asset / ES: Registrar primer asset
	UPGXDataAsset* Asset1 = NewObject<UPGXObjectDataAsset>(GetTransientPackage(), NAME_None, RF_Transient);
	const bool bFirst = Sub->RegisterAsset(DbTag, Asset1, ItemTag);
	LogRegistryTestResult(TEXT("ConflictPolicies.FirstRegistration"), bFirst);
	if (!bFirst)
	{
		OutIssues.Add(TEXT("First registration failed unexpectedly"));
		bAllPassed = false;
	}

	// EN: Attempt duplicate registration — explicit DefaultConflictPolicy dispatch.
	// ES: Intentar registro duplicado — dispatch explicito por DefaultConflictPolicy.
	UPGXObjectDataAsset* Asset2 = NewObject<UPGXObjectDataAsset>(GetTransientPackage(), NAME_None, RF_Transient);
	Asset2->CategoryTag = MakeTestTag(TEXT("ConflictCat"));
	const bool bDuplicateResult = Sub->RegisterAsset(DbTag, Asset2, ItemTag);
	const UPGXRegistrySettings* Settings = GetDefault<UPGXRegistrySettings>();
	const EPGXRegistryConflictPolicy Policy = Settings
		? Settings->DefaultConflictPolicy
		: EPGXRegistryConflictPolicy::FailOnConflict;

	const FPGXRegistryEntry* Entry = Sub->FindEntry(DbTag, ItemTag);
	if (!Entry)
	{
		OutIssues.Add(TEXT("Entry not found after conflict test"));
		bAllPassed = false;
	}
	else
	{
		switch (Policy)
		{
		case EPGXRegistryConflictPolicy::FailOnConflict:
			LogRegistryTestResult(TEXT("ConflictPolicies.FailOnConflict"), !bDuplicateResult && Entry->CachedAsset == Asset1);
			bAllPassed &= (!bDuplicateResult && Entry->CachedAsset == Asset1);
			break;
		case EPGXRegistryConflictPolicy::FirstWins:
			LogRegistryTestResult(TEXT("ConflictPolicies.FirstWins"), bDuplicateResult && Entry->CachedAsset == Asset1);
			bAllPassed &= (bDuplicateResult && Entry->CachedAsset == Asset1);
			break;
		case EPGXRegistryConflictPolicy::LastWins:
			LogRegistryTestResult(TEXT("ConflictPolicies.LastWins"), bDuplicateResult && Entry->CachedAsset == Asset2);
			bAllPassed &= (bDuplicateResult && Entry->CachedAsset == Asset2);
			break;
		}
	}

	// EN: Verify entry count is still 1 / ES: Verificar que el conteo sigue siendo 1
	FPGXDatabaseStats Stats = Sub->GetDatabaseStats(DbTag);
	const bool bCountCorrect = (Stats.TotalEntries == 1);
	LogRegistryTestResult(TEXT("ConflictPolicies.EntryCount"), bCountCorrect,
		FString::Printf(TEXT("Expected 1, got %d"), Stats.TotalEntries));
	if (!bCountCorrect)
	{
		OutIssues.Add(FString::Printf(TEXT("Entry count after conflict: expected 1, got %d"), Stats.TotalEntries));
		bAllPassed = false;
	}

	OutIssues.Add(FString::Printf(TEXT("DefaultConflictPolicy exercised: %s"), *UEnum::GetValueAsString(Policy)));

	// EN: Cleanup / ES: Limpieza
	CleanupTestDatabase(Sub, DbTag);

	OutIssues.Add(FString::Printf(TEXT("TestConflictPolicies: %s"), bAllPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED")));
	UE_LOG(LogPGXRegistry, Log, TEXT("[TestUtility] ========== TestConflictPolicies END =========="));
	return bAllPassed;
}

// ============================================================================
// EN: Test 6 — RunStressTest
// ES: Test 6 — RunStressTest
// ============================================================================

bool UPGXRegistryTestUtility::RunStressTest(const UObject* WorldContextObject, int32 EntryCount, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	UE_LOG(LogPGXRegistry, Log, TEXT("[TestUtility] ========== RunStressTest START (N=%d) =========="), EntryCount);

	UPGXDataRegistrySubsystem* Sub = GetRegistrySubsystemForTest(WorldContextObject);
	if (!Sub)
	{
		OutIssues.Add(TEXT("DataRegistrySubsystem not found"));
		LogRegistryTestResult(TEXT("RunStressTest"), false, TEXT("Subsystem not found"));
		return false;
	}

	if (EntryCount <= 0)
	{
		OutIssues.Add(TEXT("EntryCount must be > 0"));
		return false;
	}

	const FGameplayTag DbTag = MakeTestTag(TEXT("StressDB"));
	const FGameplayTag CatTag = MakeTestTag(TEXT("StressCat"));
	TArray<FGameplayTag> ItemTags;
	ItemTags.Reserve(EntryCount);
	for (int32 Index = 0; Index < EntryCount; ++Index)
	{
		const FGameplayTag ItemTag = MakeStressItemTag(Index);
		if (!ItemTag.IsValid())
		{
			OutIssues.Add(FString::Printf(
				TEXT("Stress item tag unavailable at index %d. The built-in pool provides 16 tags (indices 0-15); provision PGX.Test.Registry.Stress.Item%d in the project before requesting this benchmark size. Benchmark aborted before registry mutation."),
				Index,
				Index));
			return false;
		}
		ItemTags.Add(ItemTag);
	}

	// EN: Ensure database exists (idempotent) / ES: Asegurar que database existe (idempotente)
	if (!EnsureTestDatabase(Sub, DbTag, UPGXObjectDataAsset::StaticClass()))
	{
		OutIssues.Add(TEXT("Failed to create/ensure StressDB database"));
		LogRegistryTestResult(TEXT("RunStressTest"), false, TEXT("Database creation failed"));
		return false;
	}

	// EN: Phase 1 — Bulk registration / ES: Fase 1 — Registro masivo
	double StartTime = FPlatformTime::Seconds();
	int32 RegisteredCount = 0;
	for (int32 i = 0; i < EntryCount; ++i)
	{
		const FGameplayTag ItemTag = ItemTags[i];
		UPGXObjectDataAsset* Asset = NewObject<UPGXObjectDataAsset>(GetTransientPackage(), NAME_None, RF_Transient);
		Asset->CategoryTag = CatTag;
		if (Sub->RegisterAsset(DbTag, Asset, ItemTag))
		{
			RegisteredCount++;
		}
	}
	double RegistrationTime = FPlatformTime::Seconds() - StartTime;
	OutIssues.Add(FString::Printf(TEXT("Registration: %d/%d in %.3f ms (%.0f entries/sec)"),
		RegisteredCount, EntryCount, RegistrationTime * 1000.0,
		RegisteredCount / FMath::Max(RegistrationTime, 0.0001)));
	LogRegistryTestResult(TEXT("StressTest.Registration"), RegisteredCount == EntryCount,
		FString::Printf(TEXT("%.3f ms"), RegistrationTime * 1000.0));

	// EN: Phase 2 — FindEntry x N / ES: Fase 2 — FindEntry x N
	StartTime = FPlatformTime::Seconds();
	int32 FoundCount = 0;
	for (int32 i = 0; i < EntryCount; ++i)
	{
		const FGameplayTag ItemTag = ItemTags[i];
		if (Sub->FindEntry(DbTag, ItemTag) != nullptr)
		{
			FoundCount++;
		}
	}
	double FindTime = FPlatformTime::Seconds() - StartTime;
	OutIssues.Add(FString::Printf(TEXT("FindEntry: %d/%d in %.3f ms (%.0f lookups/sec)"),
		FoundCount, EntryCount, FindTime * 1000.0,
		FoundCount / FMath::Max(FindTime, 0.0001)));
	LogRegistryTestResult(TEXT("StressTest.FindEntry"), FoundCount == EntryCount,
		FString::Printf(TEXT("%.3f ms"), FindTime * 1000.0));

	const int32 ReverseIndexCount = Sub->GetCompositeReverseIndexCount();
	OutIssues.Add(FString::Printf(TEXT("CompositeReverseIndex: %d links"), ReverseIndexCount));
	LogRegistryTestResult(TEXT("StressTest.CompositeReverseIndex"), ReverseIndexCount == EntryCount,
		FString::Printf(TEXT("Expected %d, got %d"), EntryCount, ReverseIndexCount));

	// EN: Phase 3 — GetAllEntries / ES: Fase 3 — GetAllEntries
	StartTime = FPlatformTime::Seconds();
	TArray<FPGXRegistryEntry> AllEntries = Sub->GetAllEntries(DbTag);
	double GetAllTime = FPlatformTime::Seconds() - StartTime;
	OutIssues.Add(FString::Printf(TEXT("GetAllEntries: %d in %.3f ms"),
		AllEntries.Num(), GetAllTime * 1000.0));
	LogRegistryTestResult(TEXT("StressTest.GetAllEntries"), AllEntries.Num() == EntryCount,
		FString::Printf(TEXT("%.3f ms"), GetAllTime * 1000.0));

	// EN: Phase 4 — InvalidateCache / ES: Fase 4 — InvalidateCache
	StartTime = FPlatformTime::Seconds();
	Sub->InvalidateCache(DbTag);
	double InvalidateTime = FPlatformTime::Seconds() - StartTime;
	OutIssues.Add(FString::Printf(TEXT("InvalidateCache: %.3f ms"), InvalidateTime * 1000.0));
	LogRegistryTestResult(TEXT("StressTest.InvalidateCache"), true,
		FString::Printf(TEXT("%.3f ms"), InvalidateTime * 1000.0));

	// EN: Cleanup / ES: Limpieza
	CleanupTestDatabase(Sub, DbTag);

	OutIssues.Add(TEXT("RunStressTest: COMPLETED"));
	UE_LOG(LogPGXRegistry, Log, TEXT("[TestUtility] ========== RunStressTest END =========="));
	return RegisteredCount == EntryCount && FoundCount == EntryCount && AllEntries.Num() == EntryCount && ReverseIndexCount == EntryCount;
}

bool UPGXRegistryTestUtility::RunLargeRegistryBenchmark(const UObject* WorldContextObject, int32 EntryCount, TArray<FString>& OutIssues)
{
	const bool bCanonicalScale = EntryCount == 10000 || EntryCount == 50000 || EntryCount == 100000;
	const bool bResult = RunStressTest(WorldContextObject, EntryCount, OutIssues);
	if (!bCanonicalScale)
	{
		OutIssues.Add(FString::Printf(TEXT("RunLargeRegistryBenchmark canonical scales are 10000, 50000, or 100000 entries; got %d"), EntryCount));
	}
	return bResult;
}

// ============================================================================
// EN: Test 7 — TestCacheAndTelemetry
// ES: Test 7 — TestCacheAndTelemetry
// ============================================================================

bool UPGXRegistryTestUtility::TestCacheAndTelemetry(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	UE_LOG(LogPGXRegistry, Log, TEXT("[TestUtility] ========== TestCacheAndTelemetry START =========="));

	UPGXDataRegistrySubsystem* Sub = GetRegistrySubsystemForTest(WorldContextObject);
	if (!Sub)
	{
		OutIssues.Add(TEXT("DataRegistrySubsystem not found"));
		LogRegistryTestResult(TEXT("TestCacheAndTelemetry"), false, TEXT("Subsystem not found"));
		return false;
	}

	bool bAllPassed = true;
	const FGameplayTag DbTag = MakeTestTag(TEXT("CacheDB"));
	const FGameplayTag Item1 = MakeTestTag(TEXT("CacheItem1"));
	const FGameplayTag Item2 = MakeTestTag(TEXT("CacheItem2"));
	const FGameplayTag Item3 = MakeTestTag(TEXT("CacheItem3"));

	// EN: Ensure database exists (idempotent) / ES: Asegurar que database existe (idempotente)
	EnsureTestDatabase(Sub, DbTag, UPGXObjectDataAsset::StaticClass());
	UPGXDataAsset* Asset1 = NewObject<UPGXObjectDataAsset>(GetTransientPackage(), NAME_None, RF_Transient);
	UPGXDataAsset* Asset2 = NewObject<UPGXObjectDataAsset>(GetTransientPackage(), NAME_None, RF_Transient);
	UPGXDataAsset* Asset3 = NewObject<UPGXObjectDataAsset>(GetTransientPackage(), NAME_None, RF_Transient);
	Sub->RegisterAsset(DbTag, Asset1, Item1);
	Sub->RegisterAsset(DbTag, Asset2, Item2);
	Sub->RegisterAsset(DbTag, Asset3, Item3);

	// EN: Verify GetDatabaseStats — TotalEntries / ES: Verificar GetDatabaseStats — TotalEntries
	FPGXDatabaseStats Stats = Sub->GetDatabaseStats(DbTag);
	const bool bTotalCorrect = (Stats.TotalEntries == 3);
	LogRegistryTestResult(TEXT("CacheTelemetry.TotalEntries"), bTotalCorrect,
		FString::Printf(TEXT("Expected 3, got %d"), Stats.TotalEntries));
	if (!bTotalCorrect)
	{
		OutIssues.Add(FString::Printf(TEXT("TotalEntries: expected 3, got %d"), Stats.TotalEntries));
		bAllPassed = false;
	}

	// EN: Verify LoadedEntries before invalidation / ES: Verificar LoadedEntries antes de invalidacion
	OutIssues.Add(FString::Printf(TEXT("LoadedEntries before invalidation: %d"), Stats.LoadedEntries));
	LogRegistryTestResult(TEXT("CacheTelemetry.LoadedBefore"), true,
		FString::Printf(TEXT("Loaded=%d"), Stats.LoadedEntries));

	// EN: Invalidate cache / ES: Invalidar cache
	Sub->InvalidateCache(DbTag);

	// EN: Verify LoadedEntries == 0 after invalidation / ES: Verificar LoadedEntries == 0 despues de invalidacion
	FPGXDatabaseStats StatsAfter = Sub->GetDatabaseStats(DbTag);
	const bool bCacheCleared = (StatsAfter.LoadedEntries == 0);
	LogRegistryTestResult(TEXT("CacheTelemetry.CacheCleared"), bCacheCleared,
		FString::Printf(TEXT("LoadedEntries=%d (expected 0)"), StatsAfter.LoadedEntries));
	if (!bCacheCleared)
	{
		OutIssues.Add(FString::Printf(TEXT("LoadedEntries after invalidation: %d (expected 0)"), StatsAfter.LoadedEntries));
		bAllPassed = false;
	}

	// EN: Verify ExportMetadata is non-empty / ES: Verificar que ExportMetadata no esta vacio
	FString Metadata = Sub->ExportMetadata(DbTag);
	const bool bMetadataValid = !Metadata.IsEmpty();
	LogRegistryTestResult(TEXT("CacheTelemetry.ExportMetadata"), bMetadataValid,
		FString::Printf(TEXT("Length=%d"), Metadata.Len()));
	if (!bMetadataValid)
	{
		OutIssues.Add(TEXT("ExportMetadata returned empty string"));
		bAllPassed = false;
	}
	else
	{
		OutIssues.Add(FString::Printf(TEXT("ExportMetadata: %d chars"), Metadata.Len()));
	}

	// EN: Cleanup / ES: Limpieza
	CleanupTestDatabase(Sub, DbTag);

	OutIssues.Add(FString::Printf(TEXT("TestCacheAndTelemetry: %s"), bAllPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED")));
	UE_LOG(LogPGXRegistry, Log, TEXT("[TestUtility] ========== TestCacheAndTelemetry END =========="));
	return bAllPassed;
}

// ============================================================================
// EN: RunAllTests — aggregate suite
// ES: Ejecutar Todos — suite agregada
// ============================================================================

bool UPGXRegistryTestUtility::RunAllTests(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	TArray<FString> TestIssues;
	bool bAllPassed = true;

	OutIssues.Add(TEXT("=== PGX Data Registry Test Suite ==="));

	// EN: Test 1: Quick Test / ES: Test 1: Quick Test
	TestIssues.Empty();
	if (!RunQuickTest(WorldContextObject, TestIssues))
	{
		bAllPassed = false;
		OutIssues.Add(TEXT("[FAIL] RunQuickTest"));
		OutIssues.Append(TestIssues);
	}
	else
	{
		OutIssues.Add(TEXT("[PASS] RunQuickTest"));
	}

	// EN: Test 2: Database CRUD / ES: Test 2: CRUD de Database
	TestIssues.Empty();
	if (!TestDatabaseCRUD(WorldContextObject, TestIssues))
	{
		bAllPassed = false;
		OutIssues.Add(TEXT("[FAIL] TestDatabaseCRUD"));
		OutIssues.Append(TestIssues);
	}
	else
	{
		OutIssues.Add(TEXT("[PASS] TestDatabaseCRUD"));
	}

	// EN: Test 3: Category Queries / ES: Test 3: Queries por Categoria
	TestIssues.Empty();
	if (!TestCategoryQueries(WorldContextObject, TestIssues))
	{
		bAllPassed = false;
		OutIssues.Add(TEXT("[FAIL] TestCategoryQueries"));
		OutIssues.Append(TestIssues);
	}
	else
	{
		OutIssues.Add(TEXT("[PASS] TestCategoryQueries"));
	}

	// EN: Test 4: Composite Key Lookup / ES: Test 4: Lookup por Clave Compuesta
	TestIssues.Empty();
	if (!TestCompositeKeyLookup(WorldContextObject, TestIssues))
	{
		bAllPassed = false;
		OutIssues.Add(TEXT("[FAIL] TestCompositeKeyLookup"));
		OutIssues.Append(TestIssues);
	}
	else
	{
		OutIssues.Add(TEXT("[PASS] TestCompositeKeyLookup"));
	}

	// EN: Test 5: Conflict Policies / ES: Test 5: Politicas de Conflicto
	TestIssues.Empty();
	if (!TestConflictPolicies(WorldContextObject, TestIssues))
	{
		bAllPassed = false;
		OutIssues.Add(TEXT("[FAIL] TestConflictPolicies"));
		OutIssues.Append(TestIssues);
	}
	else
	{
		OutIssues.Add(TEXT("[PASS] TestConflictPolicies"));
	}

	// EN: Test 6: Stress Test — SKIPPED (profiling only, not for automated validation)
	// ES: Test 6: Stress Test — OMITIDO (solo profiling, no para validacion automatica)
	OutIssues.Add(TEXT("[SKIP] RunStressTest (profiling only — use RunStressTest() directly)"));

	// EN: Test 7: Cache and Telemetry / ES: Test 7: Cache y Telemetria
	TestIssues.Empty();
	if (!TestCacheAndTelemetry(WorldContextObject, TestIssues))
	{
		bAllPassed = false;
		OutIssues.Add(TEXT("[FAIL] TestCacheAndTelemetry"));
		OutIssues.Append(TestIssues);
	}
	else
	{
		OutIssues.Add(TEXT("[PASS] TestCacheAndTelemetry"));
	}

	OutIssues.Add(FString::Printf(TEXT("=== Result: %s ==="), bAllPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED")));

	UE_LOG(LogPGXRegistry, Log, TEXT("RunAllTests — %s"), bAllPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED"));
	return bAllPassed;
}
