// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PGXProfileAware.generated.h"

struct FPGXResolvedProfile;

/**
 * EN: Optional interface for subsystems that respond to profile changes.
 *     Implementing subsystems receive the resolved profile on init and
 *     get notified when the profile changes (e.g., during editor simulation).
 *
 * ES: Interfaz opcional para subsistemas que responden a cambios de profile.
 *     Los subsistemas que la implementan reciben el profile resuelto al init y
 *     son notificados cuando el profile cambia (e.g., durante simulacion en editor).
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UPGXProfileAware : public UInterface
{
	GENERATED_BODY()
};

class PGXCORERUNTIME_API IPGXProfileAware
{
	GENERATED_BODY()

public:
	/**
	 * EN: Called when the profile is applied (on init or on change).
	 *     Implement to read capabilities, budgets, features, and adapt behavior.
	 * ES: Se llama cuando el profile se aplica (al init o al cambiar).
	 *     Implementar para leer capabilities, budgets, features, y adaptar comportamiento.
	 */
	virtual void OnProfileApplied(const FPGXResolvedProfile& Profile) = 0;

	/**
	 * EN: Called before a profile change to check if this subsystem can accept it.
	 *     Return false with reasons to block the change.
	 *     Default implementation always accepts.
	 * ES: Se llama antes de un cambio de profile para verificar si este subsistema puede aceptarlo.
	 *     Retornar false con razones para bloquear el cambio.
	 *     Implementacion por defecto siempre acepta.
	 */
	virtual bool CanAcceptProfileChange(const FPGXResolvedProfile& NewProfile, TArray<FText>& OutReasons) const
	{
		return true;
	}
};
