// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PGXColonyTypes.h"
#include "PGXColonySubsystem.generated.h"

/**
 * GameInstance-scoped survivor registry with stable handles, lookup, snapshots
 * and teardown-safe cleanup.
 *
 * Settlement simulation, recruitment, policies, roles, needs, morale, remote
 * simulation, persistence and cross-plugin orchestration are not included.
 */
UCLASS()
class PGXCOLONYRUNTIME_API UPGXColonySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface

	// ========================================================================
	// Survivor registry
	// ========================================================================

	/**
	 * EN: Register a survivor with an optional authored definition tag. Always allocates a fresh
	 *     stable id and stores the resulting handle — there is **no auto-merge**: re-registering
	 *     the same `DefinitionTag` (or any tag) yields a different `SurvivorId` on every call.
	 *     Callers are responsible for tracking the handles they receive. Returns the freshly
	 *     allocated handle on success; the result code in `OutResult` carries the failure reason
	 *     on rejection.
	 * ES: Registrar un superviviente con un tag opcional de definicion autorada. Siempre asigna un
	 *     id estable nuevo (NO hay auto-merge): re-registrar el mismo `DefinitionTag` produce un
	 *     `SurvivorId` distinto en cada llamada.
	 */
	FPGXColonySurvivorHandle RegisterSurvivor(FGameplayTag DefinitionTag, FPGXColonyResult& OutResult);

	/**
	 * EN: Unregister a previously-registered handle. **Idempotent and crash-safe in the
	 *     NotFound-on-miss sense**: an invalid handle (`SurvivorId==0`) returns InvalidInput, an
	 *     unknown id returns NotFound, repeated unregister of an already-removed handle returns
	 *     NotFound — never crashes, never asserts. Successful first removal returns Success.
	 * ES: Desregistrar un handle previamente registrado. Idempotente en el sentido NotFound-on-miss
	 *     y crash-safe — handle invalido devuelve InvalidInput, id desconocido devuelve NotFound,
	 *     re-unregister devuelve NotFound. Nunca crashea.
	 */
	FPGXColonyResult UnregisterSurvivor(const FPGXColonySurvivorHandle& Handle);

	/** EN: Snapshot the current survivor registry. Returns a copy of the active handles. */
	TArray<FPGXColonySurvivorHandle> GetSurvivorSnapshot() const;

	/** EN: Lookup a handle by id. Returns true if the id is registered. */
	bool FindSurvivor(int32 SurvivorId, FPGXColonySurvivorHandle& OutHandle) const;

	/** EN: Number of currently-registered survivors. */
	int32 GetRegisteredSurvivorCount() const { return SurvivorRegistry.Num(); }

private:
	/** EN: Monotonic id allocator. Starts at 1; 0 reserved for invalid handles. */
	int32 NextSurvivorId = 1;

	/** EN: Handle storage keyed by SurvivorId. */
	TMap<int32, FPGXColonySurvivorHandle> SurvivorRegistry;
};
