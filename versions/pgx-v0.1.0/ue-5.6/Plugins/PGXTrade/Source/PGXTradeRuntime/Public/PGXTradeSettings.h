// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PGXTradeSettings.generated.h"

class UPGXTradeConfig;

/** EN: Project Settings surface for PGXTrade baseline policy. */
UCLASS(Config = Game, DefaultConfig, DisplayName = "PGX Trade")
class PGXTRADERUNTIME_API UPGXTradeSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPGXTradeSettings();

	UPROPERTY(EditAnywhere, Config, Category = "PGX|Trade")
	TSoftObjectPtr<UPGXTradeConfig> ActiveConfig;

	UPROPERTY(EditAnywhere, Config, Category = "PGX|Trade")
	bool bVerboseLogging = false;
};
