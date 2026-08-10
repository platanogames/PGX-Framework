// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PGXSettings.generated.h"

/**
 * EN: Base class for PGX system settings.
 *     Derived settings appear in Project Settings under the PGX category.
 * ES: Clase base para settings de sistemas PGX.
 *     Settings derivados aparecen en Project Settings bajo la categoria PGX.
 */
UCLASS(Abstract, config=PGX, defaultconfig)
class PGXCORERUNTIME_API UPGXSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	//~ Begin UDeveloperSettings Interface
	/** EN: Returns the PGX category name / ES: Retorna el nombre de la categoria PGX */
	FName GetCategoryName() const override { return TEXT("PGX"); }
	//~ End UDeveloperSettings Interface
};
