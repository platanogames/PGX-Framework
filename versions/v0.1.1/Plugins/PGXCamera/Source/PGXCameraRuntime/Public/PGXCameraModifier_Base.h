// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraModifier.h"
#include "PGXCameraModifier_Base.generated.h"

/**
 * EN: Base class for PGX camera modifiers (shake, FOV pulse, offset).
 *     Subclass to create specific camera effects that integrate with the PGX camera system.
 *
 * ES: Clase base para modifiers de camara PGX (shake, pulso de FOV, offset).
 *     Derivar para crear efectos de camara especificos que integren con el sistema de camara PGX.
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class PGXCAMERARUNTIME_API UPGXCameraModifier_Base : public UCameraModifier
{
	GENERATED_BODY()
};
