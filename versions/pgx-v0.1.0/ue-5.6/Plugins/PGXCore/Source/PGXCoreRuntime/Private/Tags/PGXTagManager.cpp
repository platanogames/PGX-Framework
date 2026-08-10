// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Tags/PGXTagManager.h"

#include "Logging/PGXLogCategories.h"

// EN: Extended Gameplay Tag management implementation
// ES: Implementacion de la gestion extendida de Gameplay Tags

void UPGXTagManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// EN: Reverse-lookup map and primary/category indexes start empty; no
	//     state to seed. Stale-prune happens lazily on Register / Query.
	// ES: Map de reverse-lookup e indices primario/categoria arrancan vacios;
	//     no hay estado que sembrar. Stale-prune es lazy en Register / Query.
}

void UPGXTagManager::Deinitialize()
{
	// EN: Clear all entries (weak pointers release automatically).
	// ES: Limpia todas las entries (los weak pointers se liberan auto).
	PrimaryIndex.Empty();
	CategoryIndex.Empty();
	EntryReverseLookup.Empty();

	Super::Deinitialize();
}

void UPGXTagManager::RegisterByTag(FGameplayTag Tag, UObject* Entry)
{
	if (!Tag.IsValid() || !Entry)
	{
		UE_LOG(LogPGXSettings, Warning, TEXT("[UPGXTagManager] RegisterByTag skipped: Tag=%s Entry=%s (invalid)"),
			*Tag.ToString(), Entry ? *Entry->GetName() : TEXT("null"));
		return;
	}

	// EN: Prune stale weak pointers in the target tag's set before insertion.
	// ES: Purgar stale weak pointers en el set del tag antes de insertar.
	TSet<TWeakObjectPtr<UObject>>& TagSet = PrimaryIndex.FindOrAdd(Tag);
	for (auto It = TagSet.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrent();
		}
	}
	TagSet.Add(TWeakObjectPtr<UObject>(Entry));

	// EN: Track the registration in the reverse-lookup map (no category).
	// ES: Trackear la registracion en el map de reverse-lookup (sin categoria).
	EntryReverseLookup.FindOrAdd(TWeakObjectPtr<UObject>(Entry)).Add(TPair<FGameplayTag, FName>(Tag, NAME_None));
}

void UPGXTagManager::RegisterByTagAndCategory(FName Category, FGameplayTag Tag, UObject* Entry)
{
	if (!Tag.IsValid() || !Entry || Category.IsNone())
	{
		UE_LOG(LogPGXSettings, Warning, TEXT("[UPGXTagManager] RegisterByTagAndCategory skipped: Category=%s Tag=%s Entry=%s (invalid)"),
			*Category.ToString(), *Tag.ToString(), Entry ? *Entry->GetName() : TEXT("null"));
		return;
	}

	// EN: Prune stale entries in both indexes before insertion.
	// ES: Purgar entries stale en ambos indices antes de insertar.
	TSet<TWeakObjectPtr<UObject>>& TagSet = PrimaryIndex.FindOrAdd(Tag);
	for (auto It = TagSet.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrent();
		}
	}
	TagSet.Add(TWeakObjectPtr<UObject>(Entry));

	TSet<TWeakObjectPtr<UObject>>& CatSet = CategoryIndex.FindOrAdd(Category);
	for (auto It = CatSet.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrent();
		}
	}
	CatSet.Add(TWeakObjectPtr<UObject>(Entry));

	// EN: Track the (tag, category) pair in the reverse-lookup map so a later
	//     UnregisterEntry can clean both indexes in one pass.
	// ES: Trackear el par (tag, categoria) en el map de reverse-lookup.
	EntryReverseLookup.FindOrAdd(TWeakObjectPtr<UObject>(Entry)).Add(TPair<FGameplayTag, FName>(Tag, Category));
}

void UPGXTagManager::UnregisterEntry(FGameplayTag Tag, UObject* Entry)
{
	if (!Entry)
	{
		return;
	}

	// EN: Use the reverse-lookup to find all (tag, category) pairs the entry
	//     was registered under. Walk the list and remove from the corresponding
	//     tag-set and category-set.
	// ES: Usar el reverse-lookup para encontrar todos los pares (tag, categoria)
	//     bajo los que la entry estaba registrada.
	TWeakObjectPtr<UObject> EntryKey(Entry);
	if (TSet<TPair<FGameplayTag, FName>>* LookupPairSet = EntryReverseLookup.Find(EntryKey))
	{
		for (const TPair<FGameplayTag, FName>& Pair : *LookupPairSet)
		{
			const FGameplayTag& RegisteredTag = Pair.Key;
			const FName& RegisteredCategory = Pair.Value;

			// EN: Remove from primary tag index.
			// ES: Remover del indice primario de tag.
			if (TSet<TWeakObjectPtr<UObject>>* TagSet = PrimaryIndex.Find(RegisteredTag))
			{
				TagSet->Remove(EntryKey);
				if (TagSet->Num() == 0)
				{
					PrimaryIndex.Remove(RegisteredTag);
				}
			}

			// EN: Remove from secondary category index (no-op if registered
			//     without category, since RegisteredCategory == NAME_None).
			// ES: Remover del indice secundario de categoria.
			if (!RegisteredCategory.IsNone())
			{
				if (TSet<TWeakObjectPtr<UObject>>* CatSet = CategoryIndex.Find(RegisteredCategory))
				{
					CatSet->Remove(EntryKey);
					if (CatSet->Num() == 0)
					{
						CategoryIndex.Remove(RegisteredCategory);
					}
				}
			}
		}

		EntryReverseLookup.Remove(EntryKey);
	}

	// EN: Also walk the per-tag signature: the caller may have passed a Tag
	//     that does not match any reverse-lookup pair (e.g., registered
	//     under a different tag). Defensive removal from the named Tag's set.
	// ES: Tambien walk la firma per-tag: el caller puede haber pasado un Tag
	//     que no matchea ningun par del reverse-lookup. Remocion defensiva
	//     del set del Tag nombrado.
	if (Tag.IsValid())
	{
		if (TSet<TWeakObjectPtr<UObject>>* TagSet = PrimaryIndex.Find(Tag))
		{
			TagSet->Remove(EntryKey);
			if (TagSet->Num() == 0)
			{
				PrimaryIndex.Remove(Tag);
			}
		}
	}
}

void UPGXTagManager::QueryByTag(FGameplayTag Tag, TArray<UObject*>& OutEntries) const
{
	OutEntries.Reset();
	if (!Tag.IsValid())
	{
		return;
	}

	const TSet<TWeakObjectPtr<UObject>>* TagSet = PrimaryIndex.Find(Tag);
	if (!TagSet)
	{
		return;
	}

	// EN: Stale-prune opportunistically during the lookup, then copy live
	//     entries to OutEntries. The prune copy is local; the underlying
	//     set is not modified (const method).
	// ES: Stale-prune oportunistamente durante el lookup, luego copiar las
	//     entries live a OutEntries. El prune copy es local; el set subyacente
	//     no se modifica (metodo const).
	for (const TWeakObjectPtr<UObject>& Weak : *TagSet)
	{
		if (UObject* Live = Weak.Get())
		{
			OutEntries.Add(Live);
		}
	}
}

void UPGXTagManager::QueryByCategory(FName Category, TArray<UObject*>& OutEntries) const
{
	OutEntries.Reset();
	if (Category.IsNone())
	{
		return;
	}

	const TSet<TWeakObjectPtr<UObject>>* CatSet = CategoryIndex.Find(Category);
	if (!CatSet)
	{
		return;
	}

	for (const TWeakObjectPtr<UObject>& Weak : *CatSet)
	{
		if (UObject* Live = Weak.Get())
		{
			OutEntries.Add(Live);
		}
	}
}

int32 UPGXTagManager::GetTotalEntryCount() const
{
	// EN: Count unique live entries across the primary index. O(N) with
	//     N = total registrations. Stale entries are pruned during counting.
	// ES: Contar entries live unicas en el indice primario. O(N) con
	//     N = total de registraciones. Entries stale se purgan al contar.
	TSet<TWeakObjectPtr<UObject>> Unique;
	for (const auto& Pair : PrimaryIndex)
	{
		for (const TWeakObjectPtr<UObject>& Weak : Pair.Value)
		{
			if (Weak.IsValid())
			{
				Unique.Add(Weak);
			}
		}
	}
	return Unique.Num();
}
