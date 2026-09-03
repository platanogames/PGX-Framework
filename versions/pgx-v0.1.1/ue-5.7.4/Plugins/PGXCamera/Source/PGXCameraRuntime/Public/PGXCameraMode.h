// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXDataAsset.h"
#include "PGXCameraMode.generated.h"

/**
 * EN: Data asset defining a camera mode configuration.
 *     Contains FOV, offset, blend time, and mode identification.
 *
 * ES: Data asset que define la configuracion de un modo de camara.
 *     Contiene FOV, offset, tiempo de blend e identificacion de modo.
 */
UCLASS(BlueprintType)
class PGXCAMERARUNTIME_API UPGXCameraMode : public UPGXDataAsset
{
	GENERATED_BODY()

public:
	/** EN: Unique name identifying this camera mode / ES: Nombre unico que identifica este modo de camara */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	FName ModeName;

	/** EN: Field of view in degrees / ES: Campo de vision en grados */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	float FieldOfView = 90.0f;

	/** EN: Camera offset from target / ES: Offset de la camara respecto al objetivo */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	FVector CameraOffset = FVector::ZeroVector;

	/** EN: Time in seconds to blend into this mode / ES: Tiempo en segundos para transicionar a este modo */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	float BlendTime = 0.5f;
};
