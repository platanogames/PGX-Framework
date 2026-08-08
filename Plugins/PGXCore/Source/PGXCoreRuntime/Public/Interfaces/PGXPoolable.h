// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PGXPoolable.generated.h"

/**
 * EN: Contract for objects managed by the Object Pool system.
 * ES: Contrato para objetos gestionados por el sistema de Object Pool.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UPGXPoolable : public UInterface
{
	GENERATED_BODY()
};

class PGXCORERUNTIME_API IPGXPoolable
{
	GENERATED_BODY()

public:
	/** EN: Called when the object is retrieved from the pool / ES: Llamado cuando el objeto se obtiene del pool */
	virtual void ActivateFromPool() = 0;

	/** EN: Called when the object is returned to the pool / ES: Llamado cuando el objeto se devuelve al pool */
	virtual void DeactivateToPool() = 0;

	/** EN: Called to fully reset the object state for reuse / ES: Llamado para reiniciar completamente el estado del objeto para reutilizacion */
	virtual void ResetForPool() = 0;
};
