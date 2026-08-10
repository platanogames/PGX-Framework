// Copyright PGX Framework. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PGXAbilityComponent.generated.h"

class UAbilitySystemComponent;
class UPGXAbilityFacade;
class UPGXAttributeFacade;
class UPGXEffectFacade;

/**
 * EN: PGX wrapper component over `UAbilitySystemComponent`.
 *     This is the primary interface for PGX systems to interact with GAS.
 *     Other PGX plugins should use this component, never `UAbilitySystemComponent` directly.
 *     On `BeginPlay`, finds an existing `UAbilitySystemComponent` on the owner, or creates
 *     one lazily (`NewObject` + `RegisterComponent`) if the owner does not already have one —
 *     so a project actor only needs to add `UPGXAbilityComponent`, never both components
 *     manually. Owns (resolves) one instance each of the three facades and registers itself
 *     with `UPGXAbilitySubsystem` for the Inspector registry (architecture design section 6.1).
 *
 * ES: Componente wrapper PGX sobre `UAbilitySystemComponent`.
 *     Esta es la interfaz principal para que los sistemas PGX interactuen con GAS.
 *     Otros plugins PGX deben usar este componente, nunca `UAbilitySystemComponent` directamente.
 *     En `BeginPlay`, busca un `UAbilitySystemComponent` existente en el owner, o crea uno
 *     perezosamente si el owner no tiene uno ya. Resuelve una instancia de cada una de las
 *     tres facades y se registra con `UPGXAbilitySubsystem`.
 */
UCLASS(ClassGroup=(PGX), meta=(BlueprintSpawnableComponent))
class PGXABILITYRUNTIME_API UPGXAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UPGXAbilityComponent();

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End UActorComponent Interface

	/** EN: Returns the owned ability facade, resolving lazily if not yet created. / ES: Retorna la facade de ability, resolviendo perezosamente si aun no existe. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Ability")
	UPGXAbilityFacade* GetAbilityFacade();

	/** EN: Returns the owned attribute facade, resolving lazily if not yet created. / ES: Retorna la facade de atributos. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Ability")
	UPGXAttributeFacade* GetAttributeFacade();

	/** EN: Returns the owned effect facade, resolving lazily if not yet created. / ES: Retorna la facade de efectos. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Ability")
	UPGXEffectFacade* GetEffectFacade();

	/** EN: Whether the internal `UAbilitySystemComponent` is resolved and ready. / ES: Si el `UAbilitySystemComponent` interno esta resuelto y listo. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Ability")
	bool IsAbilitySystemReady() const;

	/** EN: Internal accessor for the three facades — never returns raw GAS types outside this plugin. / ES: Accessor interno del ASC real — solo para uso dentro de este plugin (facades/bridge). */
	UAbilitySystemComponent* GetAbilitySystemComponentInternal() const { return AbilitySystemComponent; }

	/** EN: Number of currently-active abilities owned by this component (consumed by `UPGXAbilitySubsystem::GetActiveAbilityCount`). / ES: Numero de abilities actualmente activas de este componente. */
	int32 GetActiveAbilityCount() const;

private:
	/** EN: Finds an existing ASC on the owner, or creates one. Idempotent. / ES: Busca un ASC existente en el owner, o crea uno. Idempotente. */
	void ResolveAbilitySystemComponent();

	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(Transient)
	TObjectPtr<UPGXAbilityFacade> AbilityFacade;

	UPROPERTY(Transient)
	TObjectPtr<UPGXAttributeFacade> AttributeFacade;

	UPROPERTY(Transient)
	TObjectPtr<UPGXEffectFacade> EffectFacade;
};
