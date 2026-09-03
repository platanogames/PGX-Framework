// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PGXTradeTestUtility.generated.h"

class UPGXTradeSubsystem;

/**
 * EN: Path B test utility for PGXTrade greenfield baseline. Helpers return bool and
 *     populate OutIssues with [PASS]/[FAIL] lines so automation wrappers propagate failures.
 * ES: Utilidad Path B para baseline PGXTrade con contrato bool+OutIssues.
 */
UCLASS()
class PGXTRADERUNTIME_API UPGXTradeTestUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "PGX|Trade|Test", meta = (WorldContext = "WorldContextObject"))
	static bool SubsystemInitializeTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	UFUNCTION(BlueprintCallable, Category = "PGX|Trade|Test", meta = (WorldContext = "WorldContextObject"))
	static bool ConfigDefaultsTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	UFUNCTION(BlueprintCallable, Category = "PGX|Trade|Test", meta = (WorldContext = "WorldContextObject"))
	static bool ActorRegistrationTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	UFUNCTION(BlueprintCallable, Category = "PGX|Trade|Test", meta = (WorldContext = "WorldContextObject"))
	static bool OfferLifecycleTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	UFUNCTION(BlueprintCallable, Category = "PGX|Trade|Test", meta = (WorldContext = "WorldContextObject"))
	static bool ReputationReasonGuardTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	UFUNCTION(BlueprintCallable, Category = "PGX|Trade|Test", meta = (WorldContext = "WorldContextObject"))
	static bool InformationFreshnessTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	UFUNCTION(BlueprintCallable, Category = "PGX|Trade|Test", meta = (WorldContext = "WorldContextObject"))
	static bool NativeTagsRegisteredTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	UFUNCTION(BlueprintCallable, Category = "PGX|Trade|Test", meta = (WorldContext = "WorldContextObject"))
	static bool ObservableConfigSchemaTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	UFUNCTION(BlueprintCallable, Category = "PGX|Trade|Test", meta = (WorldContext = "WorldContextObject"))
	static bool ValidateAll(const UObject* WorldContextObject, TArray<FString>& OutIssues);

private:
	static UPGXTradeSubsystem* GetSubsystem(const UObject* WorldContextObject);
	static void RecordResult(TArray<FString>& OutIssues, const FString& TestName, bool bPassed, const FString& Details = TEXT(""));
};
