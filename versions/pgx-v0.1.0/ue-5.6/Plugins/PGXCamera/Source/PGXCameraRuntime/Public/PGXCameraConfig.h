// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Data/PGXConfigDataAsset.h"
#include "PGXCameraConfig.generated.h"

/**
 * EN: Config DataAsset for the PGX Camera system.
 *     Defines default FOV, blend times, collision, and lag settings.
 *
 * ES: Config DataAsset para el sistema de camara PGX.
 *     Define FOV por defecto, tiempos de blend, colision y ajustes de lag.
 */
UCLASS(BlueprintType)
class PGXCAMERARUNTIME_API UPGXCameraConfig : public UPGXConfigDataAsset
{
	GENERATED_BODY()

public:
	/** EN: Default field of view in degrees / ES: Campo de vision por defecto en grados */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Camera", meta = (ClampMin = "30.0", ClampMax = "170.0"))
	float DefaultFieldOfView = 90.0f;

	/** EN: Default blend time when switching camera modes in seconds / ES: Tiempo de blend por defecto al cambiar modos de camara en segundos */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Camera", meta = (ClampMin = "0.0"))
	float DefaultBlendTime = 0.5f;

	/** EN: Enable camera collision to prevent clipping through geometry / ES: Habilitar colision de camara para evitar atravesar geometria */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Camera")
	bool bEnableCameraCollision = true;

	/** EN: Size of the collision probe sphere / ES: Tamano de la esfera de sonda de colision */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Camera", meta = (EditCondition = "bEnableCameraCollision", ClampMin = "1.0"))
	float CollisionProbeSize = 12.0f;

	/** EN: Camera lag speed (0 = no lag, higher = snappier) / ES: Velocidad de lag de camara (0 = sin lag, mayor = mas responsivo) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Camera", meta = (ClampMin = "0.0"))
	float CameraLagSpeed = 10.0f;

	/** EN: Maximum allowed lag distance before the camera snaps / ES: Distancia maxima de lag antes de que la camara se enganche */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Camera", meta = (ClampMin = "0.0"))
	float MaxLagDistance = 200.0f;
};
