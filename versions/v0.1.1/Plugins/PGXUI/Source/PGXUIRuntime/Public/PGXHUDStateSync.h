// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PGXHUDStateSync.generated.h"

/**
 * EN: Synchronizes HUD elements with PlayerState and GameState data.
 *     Provides automatic binding between game state properties and UI elements,
 *     reducing boilerplate for common HUD updates (health, score, ammo, etc.).
 *
 * ES: Sincroniza elementos del HUD con datos de PlayerState y GameState.
 *     Proporciona binding automatico entre propiedades del estado de juego y elementos UI,
 *     reduciendo boilerplate para actualizaciones comunes del HUD (salud, puntuacion, municion, etc.).
 */
UCLASS()
class PGXUIRUNTIME_API UPGXHUDStateSync : public UObject
{
	GENERATED_BODY()
};
