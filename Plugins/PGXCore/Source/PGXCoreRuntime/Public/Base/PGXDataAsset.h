// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PGXDataAsset.generated.h"

/**
 * EN: Base data asset for all PGX data-driven types.
 *     Provides validation, versioning, and metadata support.
 * ES: Data asset base para todos los tipos data-driven de PGX.
 *     Proporciona validacion, versionado y soporte de metadata.
 */
UCLASS(Abstract, BlueprintType)
class PGXCORERUNTIME_API UPGXDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** EN: Unique identifier for this asset / ES: Identificador unico del asset */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX", meta = (AssetRegistrySearchable))
	FName AssetId;

	/** EN: Version number for save compatibility / ES: Numero de version para compatibilidad de guardado */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX")
	int32 Version = 1;
};
