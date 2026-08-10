// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Subsystems/PGXCoreTaggedRegistrySubsystem.h"

#include "Logging/PGXLogCategories.h"
#include "Utils/FPGXSubsystemResolver.h"

void UPGXCoreTaggedRegistrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	EntriesByTag.Empty();
	TagsByEntry.Empty();
}

void UPGXCoreTaggedRegistrySubsystem::Deinitialize()
{
	ClearRegistry();
	Super::Deinitialize();
}

UPGXCoreTaggedRegistrySubsystem* UPGXCoreTaggedRegistrySubsystem::Get(const UObject* WorldContextObject)
{
	return FPGXSubsystemResolver::GetFromContext<UPGXCoreTaggedRegistrySubsystem>(WorldContextObject);
}

void UPGXCoreTaggedRegistrySubsystem::RegisterEntryByTag(FGameplayTag Tag, UObject* Entry)
{
	if (!Tag.IsValid() || !Entry)
	{
		UE_LOG(LogPGXTags, Warning, TEXT("[PGXCoreTaggedRegistry] Register skipped: Tag=%s Entry=%s"),
			*Tag.ToString(), Entry ? *Entry->GetName() : TEXT("null"));
		return;
	}

	PruneStaleForTag(Tag);

	const TWeakObjectPtr<UObject> EntryWeak(Entry);
	EntriesByTag.FindOrAdd(Tag).Add(EntryWeak);
	TagsByEntry.FindOrAdd(EntryWeak).Add(Tag);
}

void UPGXCoreTaggedRegistrySubsystem::UnregisterEntryByTag(FGameplayTag Tag, UObject* Entry)
{
	if (!Tag.IsValid() || !Entry)
	{
		return;
	}

	const TWeakObjectPtr<UObject> EntryWeak(Entry);
	if (TSet<TWeakObjectPtr<UObject>>* Entries = EntriesByTag.Find(Tag))
	{
		Entries->Remove(EntryWeak);
		for (auto It = Entries->CreateIterator(); It; ++It)
		{
			if (!It->IsValid())
			{
				It.RemoveCurrent();
			}
		}

		if (Entries->Num() == 0)
		{
			EntriesByTag.Remove(Tag);
		}
	}

	if (TSet<FGameplayTag>* Tags = TagsByEntry.Find(EntryWeak))
	{
		Tags->Remove(Tag);
		if (Tags->Num() == 0)
		{
			TagsByEntry.Remove(EntryWeak);
		}
	}
}

void UPGXCoreTaggedRegistrySubsystem::UnregisterEntry(UObject* Entry)
{
	if (!Entry)
	{
		return;
	}

	const TWeakObjectPtr<UObject> EntryWeak(Entry);
	TSet<FGameplayTag> Tags;
	if (const TSet<FGameplayTag>* FoundTags = TagsByEntry.Find(EntryWeak))
	{
		Tags = *FoundTags;
	}

	for (const FGameplayTag& Tag : Tags)
	{
		UnregisterEntryByTag(Tag, Entry);
	}

	TagsByEntry.Remove(EntryWeak);
}

void UPGXCoreTaggedRegistrySubsystem::GetEntriesByTag(FGameplayTag Tag, TArray<UObject*>& OutEntries) const
{
	OutEntries.Reset();
	if (!Tag.IsValid())
	{
		return;
	}

	const TSet<TWeakObjectPtr<UObject>>* Entries = EntriesByTag.Find(Tag);
	if (!Entries)
	{
		return;
	}

	for (const TWeakObjectPtr<UObject>& EntryWeak : *Entries)
	{
		if (UObject* Entry = EntryWeak.Get())
		{
			OutEntries.Add(Entry);
		}
	}
}

void UPGXCoreTaggedRegistrySubsystem::ClearRegistry()
{
	EntriesByTag.Empty();
	TagsByEntry.Empty();
}

bool UPGXCoreTaggedRegistrySubsystem::HasEntryByTag(FGameplayTag Tag) const
{
	if (!Tag.IsValid())
	{
		return false;
	}

	if (const TSet<TWeakObjectPtr<UObject>>* Entries = EntriesByTag.Find(Tag))
	{
		return HasLiveEntry(*Entries);
	}
	return false;
}

int32 UPGXCoreTaggedRegistrySubsystem::GetCount() const
{
	TSet<TWeakObjectPtr<UObject>> UniqueEntries;
	for (const TPair<FGameplayTag, TSet<TWeakObjectPtr<UObject>>>& Pair : EntriesByTag)
	{
		for (const TWeakObjectPtr<UObject>& EntryWeak : Pair.Value)
		{
			if (EntryWeak.IsValid())
			{
				UniqueEntries.Add(EntryWeak);
			}
		}
	}
	return UniqueEntries.Num();
}

void UPGXCoreTaggedRegistrySubsystem::GetSnapshot(TArray<FGameplayTag>& OutTags) const
{
	OutTags.Reset();
	for (const TPair<FGameplayTag, TSet<TWeakObjectPtr<UObject>>>& Pair : EntriesByTag)
	{
		if (Pair.Key.IsValid() && HasLiveEntry(Pair.Value))
		{
			OutTags.Add(Pair.Key);
		}
	}
}

void UPGXCoreTaggedRegistrySubsystem::PruneStaleForTag(FGameplayTag Tag)
{
	if (TSet<TWeakObjectPtr<UObject>>* Entries = EntriesByTag.Find(Tag))
	{
		for (auto It = Entries->CreateIterator(); It; ++It)
		{
			if (!It->IsValid())
			{
				It.RemoveCurrent();
			}
		}

		if (Entries->Num() == 0)
		{
			EntriesByTag.Remove(Tag);
		}
	}
}

bool UPGXCoreTaggedRegistrySubsystem::HasLiveEntry(const TSet<TWeakObjectPtr<UObject>>& Entries)
{
	for (const TWeakObjectPtr<UObject>& EntryWeak : Entries)
	{
		if (EntryWeak.IsValid())
		{
			return true;
		}
	}
	return false;
}
