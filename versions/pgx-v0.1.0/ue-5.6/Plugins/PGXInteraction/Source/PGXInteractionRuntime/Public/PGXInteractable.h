// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PGXInteractable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UPGXInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * EN: Interface for objects that can be interacted with.
 *     Implement on any actor that should respond to player interaction.
 *
 * ES: Interfaz para objetos con los que se puede interactuar.
 *     Implementar en cualquier actor que deba responder a interaccion del jugador.
 */
class PGXINTERACTIONRUNTIME_API IPGXInteractable
{
	GENERATED_BODY()

public:
	/** EN: Called when interaction starts / ES: Se llama cuando comienza la interaccion */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PGX|Interaction")
	void OnInteract(AActor* Interactor);

	/** EN: Get the interaction prompt text / ES: Obtener el texto de prompt de interaccion */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PGX|Interaction")
	FText GetInteractionPromptText() const;

	/** EN: Check if interaction is currently available / ES: Verificar si la interaccion esta disponible */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PGX|Interaction")
	bool CanInteract(AActor* Interactor) const;
};
