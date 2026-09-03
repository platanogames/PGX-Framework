// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PGXAbilityTypes.h"
#include "PGXAttributeFacade.generated.h"

/**
 * EN: Clean API for reading and modifying GAS attributes.
 *     Provides a simplified interface over `UAttributeSet` without hiding the underlying GAS data.
 *     Other PGX systems query attributes through this facade.
 *     Supports Blueprint exposure for common attribute queries (get, set, clamp).
 *
 *     Resolves attribute tags to `FGameplayAttribute` via `UPGXAttributeBridge` (reflection-based,
 *     leaf-tag-name == UPROPERTY-name convention — see PGXAttributeBridge.h for the full rule).
 *
 * ES: API limpia para leer y modificar atributos GAS.
 *     Proporciona una interfaz simplificada sobre `UAttributeSet` sin ocultar los datos GAS subyacentes.
 *     Otros sistemas PGX consultan atributos a traves de esta facade.
 *     Soporta exposicion a Blueprints para consultas comunes de atributos (get, set, clamp).
 */
UCLASS()
class PGXABILITYRUNTIME_API UPGXAttributeFacade : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * EN: Constructor. Sets default values.
	 * ES: Constructor. Establece valores por defecto.
	 */
	UPGXAttributeFacade();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Ability|Attribute")
	float GetAttributeValue(FGameplayTag AttributeTag) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Ability|Attribute")
	float GetAttributeBaseValue(FGameplayTag AttributeTag) const;

	UFUNCTION(BlueprintCallable, Category = "PGX|Ability|Attribute")
	FPGXAbilityResult SetAttributeBaseValue(FGameplayTag AttributeTag, float NewValue);

	UFUNCTION(BlueprintCallable, Category = "PGX|Ability|Attribute")
	FPGXAbilityResult ClampAttributeValue(FGameplayTag AttributeTag, float Min, float Max);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Ability|Attribute")
	bool HasAttribute(FGameplayTag AttributeTag) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Ability|Attribute")
	TArray<FPGXAttributeSnapshot> GetAttributeSnapshot() const;

private:
	class UAbilitySystemComponent* ResolveASC() const;
};
