// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PGXTradeTypes.h"
#include "PGXTradeBlueprintLibrary.generated.h"

class UPGXTradeSubsystem;

/** EN: Thin Blueprint access surface for PGXTrade baseline APIs. */
UCLASS()
class PGXTRADERUNTIME_API UPGXTradeBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "PGX|Trade", meta = (WorldContext = "WorldContextObject"))
	static UPGXTradeSubsystem* GetTradeSubsystem(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "PGX|Trade", meta = (WorldContext = "WorldContextObject"))
	static FPGXTradeResult RegisterTradeActor(const UObject* WorldContextObject, const FPGXTradeActorRecord& Actor);

	UFUNCTION(BlueprintCallable, Category = "PGX|Trade", meta = (WorldContext = "WorldContextObject"))
	static FPGXTradeResult CreateTradeOffer(const UObject* WorldContextObject, const FPGXTradeOfferRequest& Request, FPGXTradeOffer& OutOffer);

	UFUNCTION(BlueprintCallable, Category = "PGX|Trade", meta = (WorldContext = "WorldContextObject"))
	static FPGXTradeResult AcceptTradeOffer(const UObject* WorldContextObject, FPGXTradeOfferId OfferId, FGameplayTag ReasonTag);
};
