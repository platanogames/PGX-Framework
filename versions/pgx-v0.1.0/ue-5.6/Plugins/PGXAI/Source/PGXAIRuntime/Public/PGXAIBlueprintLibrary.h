// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PGXAITypes.h"
#include "PGXAIBlueprintLibrary.generated.h"

class AAIController;
class UBehaviorTree;
class UPGXAISubsystem;

/**
 * EN: Blueprint access surface for PGXAI Development Preview APIs. `UPGXAISubsystem` stays
 *     `UCLASS()` (not `BlueprintType`) per architecture decision
 *     (the public API policy public API section + public API policy):
 *     mutators are not exposed directly on the subsystem. This library is the sole
 *     Blueprint entry point, mirroring `UPGXTradeBlueprintLibrary`. One static wrapper
 *     per existing Runtime subsystem method; no new subsystem behavior is introduced here.
 *
 * ES: Superficie de acceso Blueprint para las APIs baseline Runtime de PGXAI.
 *     `UPGXAISubsystem` permanece `UCLASS()` (no `BlueprintType`) per decision de
 *     arquitectura -- los mutators no se exponen directamente en el subsistema. Esta
 *     libreria es el unico punto de entrada Blueprint, igual patron que
 *     `UPGXTradeBlueprintLibrary`. Un wrapper estatico por metodo Runtime existente; no
 *     se introduce comportamiento nuevo en el subsistema.
 */
UCLASS()
class PGXAIRUNTIME_API UPGXAIBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** EN: Resolve the world-scoped PGXAI subsystem. / ES: Resolver el subsistema PGXAI por mundo. */
	UFUNCTION(BlueprintPure, Category = "PGX|AI", meta = (WorldContext = "WorldContextObject"))
	static UPGXAISubsystem* GetAISubsystem(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|AI|Query", meta = (WorldContext = "WorldContextObject"))
	static FPGXAIResult ValidateControllerForRegistration(const UObject* WorldContextObject, AAIController* Controller);

	UFUNCTION(BlueprintCallable, Category = "PGX|AI", meta = (WorldContext = "WorldContextObject"))
	static FPGXAIAgentHandle RegisterAgent(const UObject* WorldContextObject, AAIController* Controller, FPGXAIResult& OutResult);

	UFUNCTION(BlueprintCallable, Category = "PGX|AI", meta = (WorldContext = "WorldContextObject"))
	static FPGXAIResult UnregisterAgent(const UObject* WorldContextObject, const FPGXAIAgentHandle& Handle);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|AI|Query", meta = (WorldContext = "WorldContextObject"))
	static TArray<FPGXAIAgentHandle> GetAgentSnapshot(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|AI|Query", meta = (WorldContext = "WorldContextObject"))
	static bool FindAgent(const UObject* WorldContextObject, int32 AgentId, FPGXAIAgentHandle& OutHandle);

	UFUNCTION(BlueprintCallable, Category = "PGX|AI", meta = (WorldContext = "WorldContextObject"))
	static FPGXAIResult TryRunBehaviorTreeForAgent(const UObject* WorldContextObject, const FPGXAIAgentHandle& Handle, UBehaviorTree* BehaviorTree);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|AI|Query", meta = (WorldContext = "WorldContextObject"))
	static bool GetBehaviorTreeRunStatus(const UObject* WorldContextObject, const FPGXAIAgentHandle& Handle, FPGXAIBehaviorTreeRunStatus& OutStatus);

	UFUNCTION(BlueprintCallable, Category = "PGX|AI|Advanced", meta = (WorldContext = "WorldContextObject"))
	static int32 CleanupStaleAgents(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|AI|Query", meta = (WorldContext = "WorldContextObject"))
	static int32 GetRegisteredAgentCount(const UObject* WorldContextObject);
};
