// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PGXColonyTestUtility.generated.h"

class UPGXColonySubsystem;

/**
 * EN: Test utility for the PGX Colony baseline. 5 BPL helpers exercise the Development Preview
 *     primitives (subsystem lifecycle, survivor registry idempotency / snapshot, Settings
 *     access, native tag registration). Canonical contract — used consistently (no
 *     adapter layer; the same result contract is used by: PGXMessage, PGXBridge, PGXPSO,
 *     PGXAI, PGXInput):
 *
 *         static bool TestX(WCO, TArray<FString>& OutIssues, ...)
 *
 *     Returns true on full pass; populates OutIssues with [PASS]/[FAIL] lines so callers
 *     (Automation wrappers, Test Dashboard) propagate failure correctly.
 *
 * ES: Utilidad de pruebas para el baseline PGXColony. 5 BPL helpers ejercitan las primitivas
 *     Behavior (lifecycle del subsistema, idempotencia/snapshot del registro, acceso a Settings,
 *     registro de native tags). Contrato canonico bool+OutIssues internalizado desde el baseline.
 */
UCLASS()
class PGXCOLONYRUNTIME_API UPGXColonyTestUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** EN: Subsystem accessible from world's GameInstance; initial registry empty; lifecycle counters reset. */
	UFUNCTION(BlueprintCallable, Category = "PGX|Colony|Test", meta = (WorldContext = "WorldContextObject"))
	static bool SubsystemInitializeTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/**
	 * EN: RegisterSurvivor / UnregisterSurvivor contract — assigns fresh stable id, registry
	 *     count tracks register/unregister, NotFound on miss, InvalidInput on zero handle, no
	 *     crash on idempotent re-unregister.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Colony|Test", meta = (WorldContext = "WorldContextObject"))
	static bool SurvivorRegisterUnregisterTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: GetSurvivorSnapshot reports the live registry; FindSurvivor resolves ids; ids are unique. */
	UFUNCTION(BlueprintCallable, Category = "PGX|Colony|Test", meta = (WorldContext = "WorldContextObject"))
	static bool SurvivorRegistrySnapshotTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: UPGXColonySettings reachable via GetDefault; ActiveConfig accessor + default DiscoveryMode shape; Verbose default false. */
	UFUNCTION(BlueprintCallable, Category = "PGX|Colony|Test", meta = (WorldContext = "WorldContextObject"))
	static bool ConfigResolutionTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: All 16 native gameplay tags resolve and are valid post module load (runtime-safety: no name lookups via RequestGameplayTag). */
	UFUNCTION(BlueprintCallable, Category = "PGX|Colony|Test", meta = (WorldContext = "WorldContextObject"))
	static bool NativeTagsRegisteredTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

private:
	/** EN: Resolve UPGXColonySubsystem via WorldContextObject -> UWorld -> UGameInstance. */
	static UPGXColonySubsystem* GetSubsystem(const UObject* WorldContextObject);

	/** EN: Append [PASS]/[FAIL] line to OutIssues + UE_LOG mirror. */
	static void RecordResult(TArray<FString>& OutIssues, const FString& TestName, bool bPassed, const FString& Details = TEXT(""));
};
