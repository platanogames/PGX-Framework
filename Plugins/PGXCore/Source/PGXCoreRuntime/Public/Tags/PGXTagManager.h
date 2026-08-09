// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "UObject/WeakObjectPtr.h"
#include "PGXTagManager.generated.h"

class UObject;

/**
 * EN: Extended Gameplay Tag management.
 *     Provides complex tag queries, per-system tag categories,
 *     and tag validation rules.
 *
 *     RegisterByTag / UnregisterEntry / QueryByTag use composite indexes
 *     (primary tag index + secondary category index). Updating both indexes
 *     naive Register/Unregister that only touches a single index
 *     reintroduces the 'stale indexes' bug already documented in PGXCore
 *     diagnostics. Composite-index updates on every Register/Unregister keep
 *     the two views consistent.
 *
 * ES: Gestion extendida de Gameplay Tags.
 *     Proporciona queries complejas de tags, categorias por sistema
 *     y reglas de validacion.
 *
 *     RegisterByTag / UnregisterEntry / QueryByTag usan indices
 *     compuestos (indice primario de tag + indice secundario de categoria).
 */
UCLASS()
class PGXCORERUNTIME_API UPGXTagManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface

	/**
	 * EN: Register an entry under a tag (no category). Inserts into the
	 *     primary index only. Stale-weak-pointer entries are silently pruned
	 *     before insertion.
	 *
	 * ES: Registra una entrada bajo un tag (sin categoria). Inserta solo
	 *     en el indice primario. Entries stale-weak-pointer se purgan antes
	 *     de la insercion.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|TagManager")
	void RegisterByTag(FGameplayTag Tag, UObject* Entry);

	/**
	 * EN: Register an entry under a tag AND a category. Inserts into both
	 *     the primary tag index and the secondary category index. Both views
	 *     stay consistent because the same Insert op touches both.
	 *
	 * ES: Registra una entrada bajo un tag Y una categoria. Inserta en el
	 *     indice primario de tag y en el indice secundario de categoria.
	 *     Ambas vistas se mantienen consistentes.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|TagManager")
	void RegisterByTagAndCategory(FName Category, FGameplayTag Tag, UObject* Entry);

	/**
	 * EN: Remove an entry from ALL indexes where it appears (every tag +
	 *     category it was registered under, resolved via the reverse-lookup).
	 *     The Tag parameter is a defensive extra: it also removes the entry
	 *     from that specific tag's set even if the reverse-lookup missed it.
	 *     Stale entries are removed opportunistically. Safe on a
	 *     never-registered entry.
	 *
	 *     The name UnregisterEntry makes the full-entry behavior explicit;
	 *     a single-tag name would imply partial
	 *     removal, but the behavior is a full-entry unregister across all its
	 *     tags. No callers existed at rename time. To remove from a single
	 *     tag only; this API does not provide an exact-tag-only removal method.
	 *
	 * ES: Remueve una entrada de TODOS los indices donde aparece (cada tag +
	 *     categoria bajo los que se registro, via reverse-lookup). El
	 *     parametro Tag es un extra defensivo. Renombrada de UnregisterByTag
	 *     (el nombre viejo implicaba remocion de un solo tag).
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|TagManager")
	void UnregisterEntry(FGameplayTag Tag, UObject* Entry);

	/**
	 * EN: Lookup all live entries registered under a tag. Stale weak
	 *     pointers are SKIPPED (not copied to OutEntries) but the index is
	 *     NOT mutated — this method is const; actual index pruning happens
	 *     on the next Register. OutEntries is cleared before population.
	 *
	 * ES: Lookup de todas las entries live registradas bajo un tag. Los weak
	 *     pointers stale se OMITEN (no se copian a OutEntries) pero el indice
	 *     NO se muta — metodo const; el prune real ocurre en el siguiente
	 *     Register. OutEntries se limpia antes de poblar.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|TagManager")
	void QueryByTag(FGameplayTag Tag, TArray<UObject*>& OutEntries) const;

	/**
	 * EN: Lookup all live entries registered under a category. Same
	 *     skip-stale-without-mutating semantics as QueryByTag (const method).
	 *     OutEntries is cleared before population.
	 *
	 * ES: Lookup de todas las entries live bajo una categoria. Misma
	 *     semantica de omitir-stale-sin-mutar que QueryByTag (metodo const).
	 *     OutEntries se limpia antes de poblar.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|TagManager")
	void QueryByCategory(FName Category, TArray<UObject*>& OutEntries) const;

	/**
	 * EN: Total number of unique LIVE entries across all tags (stale weak
	 *     pointers are skipped in the count, NOT pruned from the index —
	 *     const method). Useful for inspector UI. O(N), N = total entries.
	 *
	 * ES: Numero total de entries LIVE unicas a traves de todos los tags
	 *     (los weak pointers stale se omiten en el conteo, no se purgan del
	 *     indice — metodo const). Util para UI de inspector.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|TagManager")
	int32 GetTotalEntryCount() const;

private:
	/**
	 * EN: Primary index keyed by tag. Values are sets of weak pointers so
	 *     GC-collected entries don't dangle.
	 *
	 * ES: Indice primario keyed by tag. Values son sets de weak pointers
	 *     para que entries GC-collected no queden dangling.
	 */
	TMap<FGameplayTag, TSet<TWeakObjectPtr<UObject>>> PrimaryIndex;

	/**
	 * EN: Secondary index keyed by category (FName). Lets consumers ask
	 *     'give me everything registered under category=Audio' without
	 *     enumerating all tags. Same weak-pointer semantics.
	 *
	 * ES: Indice secundario keyed por categoria (FName). Permite
	 *     'dame todo lo registrado bajo categoria=Audio' sin enumerar
	 *     todos los tags. Misma semantica de weak-pointer.
	 */
	TMap<FName, TSet<TWeakObjectPtr<UObject>>> CategoryIndex;

	/**
	 * EN: Reverse lookup: entry -> set of (tag, category) pairs the entry
	 *     is registered under. Lets Unregister clean up both indexes in one
	 *     pass. Empty for entries registered via the no-category overload.
	 *
	 * ES: Lookup inverso: entry -> set de pares (tag, categoria) bajo
	 *     los que la entry esta registrada. Permite a Unregister limpiar
	 *     ambos indices en una pasada.
	 */
	TMap<TWeakObjectPtr<UObject>, TSet<TPair<FGameplayTag, FName>>> EntryReverseLookup;
};
