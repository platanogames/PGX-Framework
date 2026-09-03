// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PGXCameraSubsystem.generated.h"

class UPGXCameraMode;

/**
 * EN: Camera mode manager.
 *     Manages the selected camera mode and active mode state per world.
 *     Blend/transition execution is outside the current runtime contract.
 *
 * ES: Manager de modos de camara.
 *     Gestiona el modo de camara seleccionado y el estado de modo activo por mundo.
 *     La ejecucion de blend/transicion sigue siendo una capa de runtime futura.
 */
UCLASS(BlueprintType)
class PGXCAMERARUNTIME_API UPGXCameraSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface

	/** EN: Select the active camera mode for the current world (sets selected-mode state only; blend/transition execution is outside the current runtime contract) / ES: Seleccionar el modo de camara activo para el mundo actual (solo fija el estado de modo seleccionado; la ejecucion de blend/transicion queda fuera del contrato runtime actual) */
	UFUNCTION(BlueprintCallable, Category = "PGX|Camera")
	bool SetCameraMode(UPGXCameraMode* NewMode);

	/** EN: Clear the active camera mode / ES: Limpiar el modo de camara activo */
	UFUNCTION(BlueprintCallable, Category = "PGX|Camera")
	void ClearCameraMode();

	/** EN: Get the active camera mode asset / ES: Obtener el asset de modo de camara activo */
	UFUNCTION(BlueprintPure, Category = "PGX|Camera")
	UPGXCameraMode* GetActiveCameraMode() const;

	/** EN: Get the active camera mode name / ES: Obtener el nombre del modo de camara activo */
	UFUNCTION(BlueprintPure, Category = "PGX|Camera")
	FName GetActiveCameraModeName() const;

private:
	/** EN: Current active mode selected through the subsystem / ES: Modo activo actual seleccionado por el subsistema */
	UPROPERTY(Transient)
	TObjectPtr<UPGXCameraMode> ActiveCameraMode;
};
