// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PGXUITestScene.generated.h"

/**
 * EN: Test scene actor for UI validation. Provides a controlled environment
 *     to test UI widgets, screen flows, and interaction patterns without
 *     requiring a full game setup.
 *
 * ES: Actor de escena de test para validacion de UI. Proporciona un entorno controlado
 *     para probar widgets UI, flujos de pantalla, y patrones de interaccion sin
 *     requerir un setup completo de juego.
 */
UCLASS()
class PGXUIRUNTIME_API APGXUITestScene : public AActor
{
	GENERATED_BODY()

public:
	APGXUITestScene();
};
