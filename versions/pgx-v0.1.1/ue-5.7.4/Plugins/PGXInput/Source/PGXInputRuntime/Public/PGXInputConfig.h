// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Data/PGXConfigDataAsset.h"
#include "Observability/PGXObservable.h"
#include "PGXInputTypes.h"
#include "PGXInputConfig.generated.h"

/**
 * EN: Config DataAsset for the PGX Input system.
 *     Defines dead zones, input buffering, device support settings, and default contexts.
 *
 * ES: Config DataAsset para el sistema de input PGX.
 *     Define zonas muertas, buffering de input, configuracion de dispositivos y contextos por defecto.
 */
UCLASS(BlueprintType)
class PGXINPUTRUNTIME_API UPGXInputConfig : public UPGXConfigDataAsset, public IPGXObservable
{
	GENERATED_BODY()

public:
	//~ Begin IPGXObservable
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;
	//~ End IPGXObservable

	/** EN: Default dead zone for analog sticks / ES: Zona muerta por defecto para sticks analogicos */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Input", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DefaultDeadZone = 0.2f;

	/** EN: Show input prompt UI based on active device / ES: Mostrar UI de prompts de input segun dispositivo activo */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Input")
	bool bShowInputPrompts = true;

	/** EN: Input buffer window in seconds for combo/queued inputs / ES: Ventana de buffer de input en segundos para combos/inputs encolados */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Input", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float InputBufferWindowSeconds = 0.15f;

	/** EN: Maximum buffered input records retained / ES: Maximo de registros de input retenidos en el buffer */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Input", meta = (ClampMin = "1", ClampMax = "128"))
	int32 InputBufferCapacity = 16;

	/** EN: Enable gamepad support and detection / ES: Habilitar soporte y deteccion de gamepad */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Input")
	bool bEnableGamepadSupport = true;

	/** EN: Automatically switch input prompts when device changes / ES: Cambiar prompts automaticamente al cambiar de dispositivo */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Input")
	bool bAutoSwitchInputDevice = true;

	/** EN: Default contexts indexed by GameplayTag / ES: Contextos por defecto indexados por GameplayTag */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Input")
	TArray<FPGXInputContextEntry> DefaultContexts;
};
