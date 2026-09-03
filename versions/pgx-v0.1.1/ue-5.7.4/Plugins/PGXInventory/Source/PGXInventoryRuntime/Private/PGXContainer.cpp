// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXContainer.h"
#include "PGXInventoryComponent.h"

// EN: Container actor implementation - world-placed item storage
// ES: Implementacion del actor contenedor - almacenamiento de items colocado en el mundo

APGXContainer::APGXContainer()
{
	// EN: Create inventory subobject for this container / ES: Crear subobjeto de inventario para este contenedor
	Inventory = CreateDefaultSubobject<UPGXInventoryComponent>(TEXT("Inventory"));
}
