// Copyright PGX Framework. All Rights Reserved.

#include "PGXInputDeviceManager.h"

void UPGXInputDeviceManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ActiveDeviceType = EPGXInputDeviceType::KeyboardMouse;
	DeviceOverrideType = EPGXInputDeviceType::Unknown;
	bHasDeviceOverride = false;
}

void UPGXInputDeviceManager::Deinitialize()
{
	bHasDeviceOverride = false;
	DeviceOverrideType = EPGXInputDeviceType::Unknown;
	ActiveDeviceType = EPGXInputDeviceType::KeyboardMouse;
	Super::Deinitialize();
}

EPGXInputDeviceType UPGXInputDeviceManager::GetActiveDeviceType() const
{
	return bHasDeviceOverride ? DeviceOverrideType : ActiveDeviceType;
}

void UPGXInputDeviceManager::SetActiveDeviceType(EPGXInputDeviceType NewDeviceType)
{
	if (bHasDeviceOverride)
	{
		return;
	}
	SetResolvedDeviceType(NewDeviceType);
}

void UPGXInputDeviceManager::ForceDeviceType(EPGXInputDeviceType ForcedDeviceType)
{
	bHasDeviceOverride = ForcedDeviceType != EPGXInputDeviceType::Unknown;
	DeviceOverrideType = ForcedDeviceType;
	if (bHasDeviceOverride)
	{
		SetResolvedDeviceType(ForcedDeviceType);
	}
}

void UPGXInputDeviceManager::ClearDeviceOverride()
{
	bHasDeviceOverride = false;
	DeviceOverrideType = EPGXInputDeviceType::Unknown;
}

bool UPGXInputDeviceManager::HasDeviceOverride() const
{
	return bHasDeviceOverride;
}

bool UPGXInputDeviceManager::IsUsingGamepad() const
{
	return GetActiveDeviceType() == EPGXInputDeviceType::Gamepad;
}

bool UPGXInputDeviceManager::IsUsingKeyboardMouse() const
{
	return GetActiveDeviceType() == EPGXInputDeviceType::KeyboardMouse;
}

void UPGXInputDeviceManager::SetResolvedDeviceType(EPGXInputDeviceType NewDeviceType)
{
	if (NewDeviceType == EPGXInputDeviceType::Unknown || ActiveDeviceType == NewDeviceType)
	{
		return;
	}

	const EPGXInputDeviceType PreviousDeviceType = ActiveDeviceType;
	ActiveDeviceType = NewDeviceType;
	OnDeviceChanged.Broadcast(PreviousDeviceType, ActiveDeviceType);
}
