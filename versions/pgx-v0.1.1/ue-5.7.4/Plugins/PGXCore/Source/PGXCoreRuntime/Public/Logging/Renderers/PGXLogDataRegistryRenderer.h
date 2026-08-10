// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Logging/PGXLogDomainRendererBase.h"
#include "PGXLogDataRegistryRenderer.generated.h"

UCLASS()
class PGXCORERUNTIME_API UPGXLogDataRegistryRenderer : public UPGXLogDomainRendererBase
{
	GENERATED_BODY()
public:
	FLinearColor GetDomainColor_Implementation() const override;
	FText GetDomainDisplayName_Implementation() const override;
	TArray<FPGXLogColumnDef> GetColumnDefinitions_Implementation() const override;
};
