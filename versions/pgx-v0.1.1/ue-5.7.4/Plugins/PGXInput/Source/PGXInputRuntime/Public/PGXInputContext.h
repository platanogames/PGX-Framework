// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXDataAsset.h"
#include "GameplayTagContainer.h"
#include "InputMappingContext.h"
#include "PGXInputTypes.h"
#include "PGXInputContext.generated.h"

/**
 * EN: Data asset defining an input context.
 *     Maps to UE Enhanced Input Mapping Contexts with priority and activation rules.
 *
 * ES: Data asset que define un contexto de input.
 *     Mapea a Input Mapping Contexts de Enhanced Input de UE con prioridad y reglas de activacion.
 */
UCLASS(BlueprintType)
class PGXINPUTRUNTIME_API UPGXInputContext : public UPGXDataAsset
{
	GENERATED_BODY()

public:
	/** EN: GameplayTag identity for this context / ES: Identidad GameplayTag de este contexto */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Input", meta = (Categories = "PGX.Input.Context"))
	FGameplayTag ContextTag;

	/** EN: Enhanced Input mapping context resolved by this PGX context / ES: Mapping context de Enhanced Input resuelto por este contexto PGX */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Input")
	TSoftObjectPtr<UInputMappingContext> MappingContext;

	/** EN: Internal name retained for migration/readability / ES: Nombre interno conservado para migracion/legibilidad */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Input")
	FName ContextName;

	/** EN: Priority for context stacking (higher = takes precedence) / ES: Prioridad para apilado de contextos (mayor = tiene precedencia) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Input")
	int32 Priority = 0;

	/** EN: Stack activation policy / ES: Politica de activacion en el stack */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Input")
	EPGXInputContextActivationMode ActivationMode = EPGXInputContextActivationMode::Additive;

	/** EN: Whether config bootstrap may activate this context / ES: Si el bootstrap de config puede activar este contexto */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Input")
	bool bActivateOnStart = false;
};
