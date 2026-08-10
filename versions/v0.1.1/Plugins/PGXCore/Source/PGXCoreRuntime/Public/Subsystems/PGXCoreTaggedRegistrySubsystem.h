// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Interfaces/PGXTaggedRegistry.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/WeakObjectPtr.h"
#include "PGXCoreTaggedRegistrySubsystem.generated.h"

/**
 * EN: Core reusable tagged registry implementation. Stores weak UObject entries
 *     behind GameplayTags, keeps a reverse lookup for deterministic cleanup,
 *     and exposes the IPGXTaggedRegistry read contract.
 *
 *     This is the core API layer only. Plugin-specific adoption should wrap or
 *     delegate their existing tag registries to this subsystem in separate
 *     commits so behavior changes stay reviewable.
 *
 * ES: Implementacion reusable core de registry tagged. Guarda entries UObject
 *     weak por GameplayTag, mantiene lookup inverso para cleanup determinista,
 *     y expone el contrato de lectura IPGXTaggedRegistry.
 *
 *     Esta es solo la capa API core. La adopcion especifica por plugin debe
 *     envolver o delegar sus registries tagged existentes en commits separados.
 */
UCLASS()
class PGXCORERUNTIME_API UPGXCoreTaggedRegistrySubsystem : public UGameInstanceSubsystem, public IPGXTaggedRegistry
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface

	/**
	 * EN: Resolve the subsystem from any UObject world context. Returns nullptr
	 *     if context, world, game instance, or subsystem resolution fails.
	 * ES: Resuelve el subsistema desde cualquier UObject world context. Retorna
	 *     nullptr si falla context, world, game instance o subsystem.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|TaggedRegistry", meta = (WorldContext = "WorldContextObject"))
	static UPGXCoreTaggedRegistrySubsystem* Get(const UObject* WorldContextObject);

	/**
	 * EN: Register an entry under a tag. Null entries and invalid tags are ignored.
	 *     Stale weak pointers for that tag are pruned before insertion.
	 * ES: Registra una entry bajo un tag. Entries null y tags invalidos se ignoran.
	 *     Weak pointers stale de ese tag se purgan antes de insertar.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|TaggedRegistry")
	void RegisterEntryByTag(FGameplayTag Tag, UObject* Entry);

	/**
	 * EN: Remove an entry from a specific tag. Safe for null, invalid, and
	 *     never-registered inputs.
	 * ES: Remueve una entry de un tag especifico. Seguro para null, invalidos y
	 *     entradas nunca registradas.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|TaggedRegistry")
	void UnregisterEntryByTag(FGameplayTag Tag, UObject* Entry);

	/**
	 * EN: Remove an entry from every tag it was registered under.
	 * ES: Remueve una entry de todos los tags donde fue registrada.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|TaggedRegistry")
	void UnregisterEntry(UObject* Entry);

	/**
	 * EN: Return live entries for a tag. OutEntries is reset before population;
	 *     stale weak pointers are skipped without mutating this const query.
	 * ES: Retorna entries vivas de un tag. OutEntries se resetea antes de poblar;
	 *     weak pointers stale se omiten sin mutar esta query const.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|TaggedRegistry")
	void GetEntriesByTag(FGameplayTag Tag, TArray<UObject*>& OutEntries) const;

	/** EN: Clear all registry state / ES: Limpia todo el estado del registry. */
	UFUNCTION(BlueprintCallable, Category = "PGX|TaggedRegistry")
	void ClearRegistry();

	//~ Begin IPGXTaggedRegistry Interface
	bool HasEntryByTag(FGameplayTag Tag) const override;
	int32 GetCount() const override;
	void GetSnapshot(TArray<FGameplayTag>& OutTags) const override;
	//~ End IPGXTaggedRegistry Interface

private:
	void PruneStaleForTag(FGameplayTag Tag);
	static bool HasLiveEntry(const TSet<TWeakObjectPtr<UObject>>& Entries);

	/** EN: Primary index tag -> entries / ES: Indice primario tag -> entries. */
	TMap<FGameplayTag, TSet<TWeakObjectPtr<UObject>>> EntriesByTag;

	/** EN: Reverse lookup entry -> tags for deterministic unregister.
	 *  ES: Lookup inverso entry -> tags para unregister determinista. */
	TMap<TWeakObjectPtr<UObject>, TSet<FGameplayTag>> TagsByEntry;
};
