// Copyright PGX Framework. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PGXAbilityTypes.h"
#include "PGXEffectFacade.generated.h"

class UGameplayEffect;

/**
 * EN: Clean API for applying, removing, and querying gameplay effects.
 *     Wraps `UGameplayEffect` operations with a simplified PGX interface.
 *     Handles effect stacking, duration, and removal without exposing
 *     GAS internals to other PGX systems.
 *     Supports Blueprint exposure for common effect operations (apply, remove, query).
 *
 * ES: API limpia para aplicar, remover y consultar gameplay effects.
 *     Wrappea operaciones de `UGameplayEffect` con una interfaz PGX simplificada.
 *     Soporta exposicion a Blueprints para operaciones comunes de efectos (aplicar, remover, consultar).
 */
UCLASS()
class PGXABILITYRUNTIME_API UPGXEffectFacade : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * EN: Constructor. Sets default values.
	 * ES: Constructor. Establece valores por defecto.
	 */
	UPGXEffectFacade();

	/**
	 * EN: Apply a gameplay effect by class. Same Tag-via-class-CDO rationale as
	 *     `UPGXAbilityFacade::GrantAbility` — GAS has no Tag->Class resolver.
	 * ES: Aplicar un gameplay effect por clase. Misma razon que GrantAbility — GAS no
	 *     tiene resolver Tag->Class.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Ability|Effect")
	FPGXAbilityResult ApplyEffect(TSubclassOf<UGameplayEffect> EffectClass, float Level, FPGXEffectHandle& OutHandle);

	UFUNCTION(BlueprintCallable, Category = "PGX|Ability|Effect")
	FPGXAbilityResult RemoveEffect(const FPGXEffectHandle& Handle);

	UFUNCTION(BlueprintCallable, Category = "PGX|Ability|Effect")
	int32 RemoveEffectsByTag(FGameplayTag EffectTag);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Ability|Effect")
	bool HasEffect(FGameplayTag EffectTag) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Ability|Effect")
	TArray<FPGXEffectSnapshot> GetActiveEffects() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Ability|Effect")
	float GetEffectRemainingDuration(const FPGXEffectHandle& Handle) const;

private:
	class UAbilitySystemComponent* ResolveASC() const;
};
