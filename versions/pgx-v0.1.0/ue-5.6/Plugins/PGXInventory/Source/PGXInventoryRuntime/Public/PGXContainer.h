// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PGXContainer.generated.h"

class UPGXInventoryComponent;

/**
 * EN: Interactable container actor (chests, shops, loot).
 *     Holds an inventory component for item storage.
 *
 * ES: Actor contenedor interactuable (cofres, tiendas, loot).
 *     Contiene un componente de inventario para almacenamiento de items.
 */
UCLASS(BlueprintType, Blueprintable)
class PGXINVENTORYRUNTIME_API APGXContainer : public AActor
{
	GENERATED_BODY()

public:
	APGXContainer();

	/** EN: Inventory component holding this container's items / ES: Componente de inventario que contiene los items de este contenedor */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Container")
	TObjectPtr<UPGXInventoryComponent> Inventory;
};
