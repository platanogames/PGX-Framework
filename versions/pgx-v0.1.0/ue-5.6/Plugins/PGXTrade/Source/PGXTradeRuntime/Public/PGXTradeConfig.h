// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Observability/PGXObservable.h"
#include "PGXTradeConfig.generated.h"

/** EN: Runtime policy box for the generic PGXTrade baseline. */
UCLASS(BlueprintType)
class PGXTRADERUNTIME_API UPGXTradeConfig : public UDataAsset, public IPGXObservable
{
	GENERATED_BODY()

public:
	static const FName SchemaVersion;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Trade|Policy", meta = (ClampMin = "0.0"))
	float FairTradeTolerance = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Trade|Policy", meta = (ClampMin = "0.0"))
	float DefaultOfferExpirationSeconds = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Trade|Policy", meta = (ClampMin = "-100.0", ClampMax = "100.0"))
	float MinReputation = -100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Trade|Policy", meta = (ClampMin = "-100.0", ClampMax = "100.0"))
	float MaxReputation = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Trade|Policy", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DefaultInformationFreshness = 1.0f;

	bool IsPolicyValid() const;

	//~ Begin IPGXObservable
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;
	//~ End IPGXObservable
};
