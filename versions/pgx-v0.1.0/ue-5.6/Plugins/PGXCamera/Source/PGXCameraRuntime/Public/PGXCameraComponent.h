// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "PGXCameraComponent.generated.h"

/**
 * EN: PGX camera component with mode support and integrated transitions.
 *     Drop-in replacement for UCameraComponent with PGX camera mode integration.
 *
 * ES: Componente de camara PGX con soporte de modos y transiciones integradas.
 *     Reemplazo directo de UCameraComponent con integracion de modos de camara PGX.
 */
UCLASS(ClassGroup=(PGX), BlueprintType, meta=(BlueprintSpawnableComponent))
class PGXCAMERARUNTIME_API UPGXCameraComponent : public UCameraComponent
{
	GENERATED_BODY()
};
