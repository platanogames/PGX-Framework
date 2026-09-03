// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PGXInputTypes.h"
#include "PGXInputDeviceManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPGXInputDeviceChangedSignature, EPGXInputDeviceType, PreviousDevice, EPGXInputDeviceType, NewDevice);

/**
 * EN: Input device detection and management subsystem.
 *     Holds the active device snapshot and allows deterministic test/device overrides.
 *
 * ES: Subsistema de deteccion y gestion de dispositivos de input.
 *     Mantiene el snapshot del dispositivo activo y permite overrides deterministas de test/dispositivo.
 */
UCLASS()
class PGXINPUTRUNTIME_API UPGXInputDeviceManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface

	UFUNCTION(BlueprintPure, Category = "PGX|Input")
	EPGXInputDeviceType GetActiveDeviceType() const;

	UFUNCTION(BlueprintCallable, Category = "PGX|Input")
	void SetActiveDeviceType(EPGXInputDeviceType NewDeviceType);

	UFUNCTION(BlueprintCallable, Category = "PGX|Input")
	void ForceDeviceType(EPGXInputDeviceType ForcedDeviceType);

	UFUNCTION(BlueprintCallable, Category = "PGX|Input")
	void ClearDeviceOverride();

	UFUNCTION(BlueprintPure, Category = "PGX|Input")
	bool HasDeviceOverride() const;

	UFUNCTION(BlueprintPure, Category = "PGX|Input")
	bool IsUsingGamepad() const;

	UFUNCTION(BlueprintPure, Category = "PGX|Input")
	bool IsUsingKeyboardMouse() const;

	UPROPERTY(BlueprintAssignable, Category = "PGX|Input")
	FPGXInputDeviceChangedSignature OnDeviceChanged;

private:
	void SetResolvedDeviceType(EPGXInputDeviceType NewDeviceType);

	UPROPERTY(Transient)
	EPGXInputDeviceType ActiveDeviceType = EPGXInputDeviceType::KeyboardMouse;

	UPROPERTY(Transient)
	EPGXInputDeviceType DeviceOverrideType = EPGXInputDeviceType::Unknown;

	UPROPERTY(Transient)
	bool bHasDeviceOverride = false;
};
