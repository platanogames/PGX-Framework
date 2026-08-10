// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "PGXAsyncAction.generated.h"

/**
 * EN: Base class for PGX async Blueprint actions (latent nodes).
 * ES: Clase base para acciones async de Blueprint de PGX (latent nodes).
 */
UCLASS(Abstract)
class PGXCORERUNTIME_API UPGXAsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	//~ Begin UBlueprintAsyncActionBase Interface
	void Activate() override;
	//~ End UBlueprintAsyncActionBase Interface
};
