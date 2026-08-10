// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PGXAITestUtility.generated.h"

class UPGXAISubsystem;

/**
 * EN: Test utility for the PGX AI baseline. 5 BPL helpers exercise the Development Preview
 *     primitives (subsystem lifecycle, agent registry idempotency / snapshot, Settings
 *     access, native tag registration). Canonical contract — used consistently per
 *     Configuration established validation pattern:
 *
 *         static bool TestX(WCO, TArray<FString>& OutIssues, ...)
 *
 *     Returns true on full pass; populates OutIssues with [PASS]/[FAIL]/[INFO] lines so
 *     callers (Automation wrappers, Test Dashboard) propagate failure correctly.
 *
 * ES: Utilidad de pruebas para el baseline PGXAI. 5 BPL helpers ejercitan las primitivas
 *     Runtime (lifecycle del subsistema, idempotencia/snapshot del registro, acceso a
 *     Settings, registro de native tags). Contrato canonico bool+OutIssues internalizado
 *     desde el baseline.
 */
UCLASS()
class PGXAIRUNTIME_API UPGXAITestUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** EN: Subsystem accessible from world; initial registry empty; lifecycle counters reset. */
	UFUNCTION(BlueprintCallable, Category = "PGX|AI|Test", meta = (WorldContext = "WorldContextObject"))
	static bool SubsystemInitializeTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: RegisterAgent / UnregisterAgent contract — assigns id, idempotent re-register, NotFound on miss, InvalidInput on null/zero handle. */
	UFUNCTION(BlueprintCallable, Category = "PGX|AI|Test", meta = (WorldContext = "WorldContextObject"))
	static bool AgentRegisterUnregisterTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: GetAgentSnapshot reports the live registry; stale weak pointers filtered at snapshot time. */
	UFUNCTION(BlueprintCallable, Category = "PGX|AI|Test", meta = (WorldContext = "WorldContextObject"))
	static bool AgentRegistrySnapshotTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: UPGXAISettings reachable via GetDefault; ActiveConfig accessor + default DiscoveryMode shape. */
	UFUNCTION(BlueprintCallable, Category = "PGX|AI|Test", meta = (WorldContext = "WorldContextObject"))
	static bool ConfigResolutionTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: All 16 native gameplay tags resolve and are valid post module load (runtime-safety: no name lookups). */
	UFUNCTION(BlueprintCallable, Category = "PGX|AI|Test", meta = (WorldContext = "WorldContextObject"))
	static bool NativeTagsRegisteredTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

private:
	/** EN: Resolve UPGXAISubsystem from a world context. */
	static UPGXAISubsystem* GetSubsystem(const UObject* WorldContextObject);

	/** EN: Append [PASS]/[FAIL]/[INFO] line to OutIssues + UE_LOG mirror for live debug visibility. */
	static void RecordResult(TArray<FString>& OutIssues, const FString& TestName, bool bPassed, const FString& Details = TEXT(""));
};
