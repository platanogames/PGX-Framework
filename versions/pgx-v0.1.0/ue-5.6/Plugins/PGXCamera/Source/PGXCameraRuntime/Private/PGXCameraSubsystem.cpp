// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXCameraSubsystem.h"

#include "PGXCameraMode.h"

// EN: Camera mode manager implementation
// ES: Implementacion del manager de modos de camara

void UPGXCameraSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UPGXCameraSubsystem::Deinitialize()
{
	ClearCameraMode();
	Super::Deinitialize();
}

// Thin Blueprint surface; camera component transition logic is outside this subsystem.
bool UPGXCameraSubsystem::SetCameraMode(UPGXCameraMode* NewMode)
{
	if (!IsValid(NewMode))
	{
		return false;
	}

	ActiveCameraMode = NewMode;
	return true;
}

void UPGXCameraSubsystem::ClearCameraMode()
{
	ActiveCameraMode = nullptr;
}

UPGXCameraMode* UPGXCameraSubsystem::GetActiveCameraMode() const
{
	return ActiveCameraMode.Get();
}

FName UPGXCameraSubsystem::GetActiveCameraModeName() const
{
	return ActiveCameraMode ? ActiveCameraMode->ModeName : NAME_None;
}
