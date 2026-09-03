// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PGXAbilityTypes.h"
#include "PGXAbilityFacade.generated.h"

class UGameplayAbility;

/**
 * EN: Clean API for granting, activating, and managing gameplay abilities.
 *     Encapsulates `UGameplayAbility` lifecycle behind a PGX-friendly interface.
 *     Supports Blueprint exposure for common ability operations.
 *     Handles grant, revoke, activate, cancel, and query without exposing
 *     raw GAS handles or spec types to the rest of the PGX framework.
 *
 *     Resolves its owning `UPGXAbilityComponent` via `GetTypedOuter` (this facade is
 *     always `NewObject`'d with the component as Outer — architecture design section 7, never a
 *     free-floating singleton).
 *
 * ES: API limpia para conceder, activar y gestionar gameplay abilities.
 *     Encapsula el ciclo de vida de `UGameplayAbility` detras de una interfaz PGX-friendly.
 *     Soporta exposicion a Blueprints para operaciones comunes de abilities.
 *     Maneja concesion, revocacion, activacion, cancelacion y consulta sin exponer
 *     handles raw de GAS o tipos spec al resto del framework PGX.
 */
UCLASS()
class PGXABILITYRUNTIME_API UPGXAbilityFacade : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * EN: Constructor. Sets default values.
	 * ES: Constructor. Establece valores por defecto.
	 */
	UPGXAbilityFacade();

	/**
	 * EN: Grant an ability by class. The ability's identity tag is read from the class CDO's
	 *     `AbilityTags` (project content authors this on the `UGameplayAbility` Blueprint/class,
	 *     not passed separately here — GAS has no built-in Tag->Class resolver, so the class is
	 *     the canonical input; the tag is what other PGX systems and Blueprint then query by).
	 * ES: Conceder una ability por clase. El tag de identidad se lee de `AbilityTags` del CDO
	 *     de la clase.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Ability")
	FPGXAbilityResult GrantAbility(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level, FPGXAbilityHandle& OutHandle);

	UFUNCTION(BlueprintCallable, Category = "PGX|Ability")
	FPGXAbilityResult RevokeAbility(const FPGXAbilityHandle& Handle);

	UFUNCTION(BlueprintCallable, Category = "PGX|Ability")
	FPGXAbilityResult ActivateAbilityByTag(FGameplayTag AbilityTag);

	UFUNCTION(BlueprintCallable, Category = "PGX|Ability")
	FPGXAbilityResult CancelAbility(const FPGXAbilityHandle& Handle);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Ability")
	bool IsAbilityGranted(FGameplayTag AbilityTag) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Ability")
	bool IsAbilityActive(const FPGXAbilityHandle& Handle) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Ability")
	TArray<FPGXAbilitySnapshot> GetGrantedAbilities() const;

	/** EN: Cooldown remaining, read from the ability's CooldownGameplayEffect active instance. 0 if not on cooldown or ability not found. / ES: Cooldown restante. 0 si no aplica o no se encuentra. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Ability")
	float GetCooldownRemaining(FGameplayTag AbilityTag) const;

private:
	class UAbilitySystemComponent* ResolveASC() const;
};
