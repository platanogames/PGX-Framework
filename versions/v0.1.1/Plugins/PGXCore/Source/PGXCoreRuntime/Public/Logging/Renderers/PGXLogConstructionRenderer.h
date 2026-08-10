// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Logging/PGXLogDomainRendererBase.h"
#include "PGXLogConstructionRenderer.generated.h"

UCLASS()
class PGXCORERUNTIME_API UPGXLogConstructionRenderer : public UPGXLogDomainRendererBase
{
	GENERATED_BODY()
public:
	FLinearColor GetDomainColor_Implementation() const override;
	FText GetDomainDisplayName_Implementation() const override;
	TArray<FPGXLogColumnDef> GetColumnDefinitions_Implementation() const override;
};
