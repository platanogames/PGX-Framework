// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PGXConfigurable.generated.h"

// EN: Forward declaration
// ES: Declaracion adelantada
class UPGXDataAsset;

/**
 * EN: Contract for objects configured via Data Assets.
 * ES: Contrato para objetos que se configuran via Data Assets.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UPGXConfigurable : public UInterface
{
	GENERATED_BODY()
};

class PGXCORERUNTIME_API IPGXConfigurable
{
	GENERATED_BODY()

public:
	/** EN: Apply configuration from a data asset / ES: Aplicar configuracion desde un data asset */
	virtual void ApplyConfiguration(const UPGXDataAsset* Config) = 0;
};
