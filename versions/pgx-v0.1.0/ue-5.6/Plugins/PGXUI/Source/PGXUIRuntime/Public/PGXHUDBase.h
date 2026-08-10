// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PGXHUDBase.generated.h"

/**
 * EN: Base HUD class for PGX projects. Provides modular HUD structure with
 *     layer management, widget registration, and standardized update patterns.
 *     Manages HUD widget lifecycle and coordinates with PGXUISubsystem.
 *
 * ES: Clase HUD base para proyectos PGX. Proporciona estructura de HUD modular con
 *     gestion de capas, registro de widgets, y patrones de actualizacion estandarizados.
 *     Gestiona el ciclo de vida de widgets HUD y coordina con PGXUISubsystem.
 */
UCLASS()
class PGXUIRUNTIME_API APGXHUDBase : public AHUD
{
	GENERATED_BODY()

public:
	APGXHUDBase();
};
