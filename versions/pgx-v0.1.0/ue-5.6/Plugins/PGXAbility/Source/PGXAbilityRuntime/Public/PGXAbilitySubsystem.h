// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once

#include "CoreMinimal.h"
#include "Base/PGXGameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "PGXAbilitySubsystem.generated.h"

class UPGXAbilityComponent;
class UPGXAbilityConfig;

/**
 * EN: Native multicast delegate fired when any registered `UPGXAbilityComponent`'s
 *     `UPGXAbilityFacade` activates an ability. Fan-in surface for the Inspector
 *     (design section 6.1 of Docs/Architecture/PGXAbility_Architecture.md) — required by
 *     `SPGXAbilityPanel` (read-only editor surface) for reactive refresh.
 * ES: Delegate nativo multicast disparado cuando cualquier `UPGXAbilityComponent`
 *     registrado activa una ability. Superficie fan-in para el Inspector.
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FPGXOnAbilityActivatedNative, UPGXAbilityComponent* /*Component*/, FGameplayTag /*AbilityTag*/);

/** EN: Native multicast delegate fired when a component registers or unregisters with the subsystem (bRegistered=true/false). / ES: Delegate disparado cuando un componente se registra o desregistra. */
DECLARE_MULTICAST_DELEGATE_TwoParams(FPGXOnComponentRegisteredNative, UPGXAbilityComponent* /*Component*/, bool /*bRegistered*/);

/**
 * EN: Central ability system coordinator.
 *     Resolves Config DA (Settings-first), maintains a GameInstance-scoped registry of
 *     active `UPGXAbilityComponent` instances, and exposes aggregate query surfaces for
 *     the Inspector. Per architecture design section 7, this class stays non-`BlueprintType` and
 *     exposes no `UFUNCTION` — Blueprint always reaches ability functionality through
 *     `UPGXAbilityComponent` and its three facades, never through this subsystem
 *     directly (matching the layer separation used by
 *     `UPGXProfileSubsystem`).
 *
 * ES: Coordinador central del sistema de abilities.
 *     Resuelve el Config DA (Settings-first), mantiene un registro a nivel GameInstance
 *     de instancias `UPGXAbilityComponent` activas, y expone superficies de consulta
 *     agregada para el Inspector. Per arquitectura design section 7, esta clase permanece no-
 *     `BlueprintType` y no expone `UFUNCTION` — Blueprint siempre llega a la
 *     funcionalidad de ability via `UPGXAbilityComponent` y sus tres facades.
 */
UCLASS()
class PGXABILITYRUNTIME_API UPGXAbilitySubsystem : public UPGXGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	//~ Begin USubsystem interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem interface

	// ========================================================================
	// EN: Component Registry (consumed by Inspector, not Blueprint — C++ only)
	// ES: Registro de Componentes (consumido por Inspector, no Blueprint — solo C++)
	// ========================================================================

	/** EN: Called by `UPGXAbilityComponent` on creation/BeginPlay. Idempotent. / ES: Llamado por el componente al crearse. Idempotente. */
	void RegisterComponent(UPGXAbilityComponent* Component);

	/** EN: Called by `UPGXAbilityComponent` on EndPlay/teardown. Idempotent — unknown component is a no-op. / ES: Llamado al destruirse el componente. Idempotente. */
	void UnregisterComponent(UPGXAbilityComponent* Component);

	/** EN: Snapshot of currently-registered components, filtering stale (GC'd) weak refs. / ES: Snapshot de componentes registrados, filtrando refs stale. */
	TArray<TWeakObjectPtr<UPGXAbilityComponent>> GetComponentRegistry() const;

	/** EN: Aggregate active-ability count across all registered components (for Inspector overview KPI). / ES: Conteo agregado de abilities activas. */
	int32 GetActiveAbilityCount() const;

	/** EN: Number of currently-registered components (does not filter stale entries). / ES: Cantidad de componentes registrados. */
	int32 GetRegisteredComponentCount() const { return ComponentRegistry.Num(); }

	/** EN: Fan-in delegate for ability-activated events across all components. Components broadcast here from their facade callbacks. / ES: Delegate fan-in para eventos de activacion. */
	FPGXOnAbilityActivatedNative OnAbilityActivatedNative;

	/** EN: Fires on RegisterComponent/UnregisterComponent (Inspector reactive-refresh hook). / ES: Dispara en RegisterComponent/UnregisterComponent. */
	FPGXOnComponentRegisteredNative OnComponentRegisteredNative;

	/** EN: Resolved Config DA, or nullptr if unassigned (subsystem then runs on `UPGXAbilityConfig` field defaults via a transient fallback instance — never a hard failure). / ES: Config DA resuelto, o nullptr si no asignado. */
	const UPGXAbilityConfig* GetActiveConfig() const { return ActiveConfig; }

private:
	void DiscoverConfig();
	void RegisterConsoleCommands();
	void UnregisterConsoleCommands();

	UPROPERTY(Transient)
	TObjectPtr<const UPGXAbilityConfig> ActiveConfig;

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<UPGXAbilityComponent>> ComponentRegistry;

	TArray<IConsoleObject*> RegisteredCommands;
};
