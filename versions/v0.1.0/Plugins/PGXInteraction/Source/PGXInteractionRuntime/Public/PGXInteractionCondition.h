// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PGXInteractionCondition.generated.h"

/**
 * EN: Base class for interaction conditions (requires key, level, game state, etc.).
 *     Subclass to define custom conditions for interaction availability.
 *
 * ES: Clase base para condiciones de interaccion (requiere llave, nivel, estado de juego, etc.).
 *     Crear subclase para definir condiciones personalizadas de disponibilidad de interaccion.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew)
class PGXINTERACTIONRUNTIME_API UPGXInteractionCondition : public UObject
{
	GENERATED_BODY()

public:
	/** EN: Evaluate whether the condition is met / ES: Evaluar si la condicion se cumple */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|Interaction")
	bool EvaluateCondition(AActor* Interactor) const;
	virtual bool EvaluateCondition_Implementation(AActor* Interactor) const;
};
