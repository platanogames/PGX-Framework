// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXAssetValidator.generated.h"

/**
 * EN: Base class for PGX asset validation rules.
 *     Provides a framework for custom validation that runs on save,
 *     on demand, or in CI pipelines.
 *
 * ES: Clase base para reglas de validacion de assets PGX.
 *     Proporciona un framework de validacion custom que se ejecuta al guardar,
 *     bajo demanda, o en pipelines de CI.
 */
UCLASS(Abstract, Blueprintable)
class PGXCOREEDITOR_API UPGXAssetValidator : public UObject
{
	GENERATED_BODY()

public:
	/** EN: Validate the given asset / ES: Validar el asset dado */
	UFUNCTION(BlueprintNativeEvent)
	bool ValidateAsset(UObject* Asset, TArray<FText>& OutErrors) const;
	virtual bool ValidateAsset_Implementation(UObject* Asset, TArray<FText>& OutErrors) const;
};
