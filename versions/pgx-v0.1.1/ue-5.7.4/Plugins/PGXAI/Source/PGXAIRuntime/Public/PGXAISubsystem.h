// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PGXAITypes.h"
#include "PGXAISubsystem.generated.h"

class AAIController;
class UBehaviorTree;

/**
 * World-scoped AI agent registry with stable handles, stale-entry cleanup,
 * Behavior Tree dispatch and per-agent run status.
 *
 * Full perception, alert-state, state-machine and squad orchestration are not
 * part of this Development Preview.
 */
UCLASS()
class PGXAIRUNTIME_API UPGXAISubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface

	// ========================================================================
	// Agent registry
	// ========================================================================

	/**
	 * EN: Register an AI controller as a managed agent. Assigns a fresh stable id and stores a
	 *     weak pointer back to the controller for inspector and message-broadcast
	 *     perception integration. Returns the assigned handle on success; the result code in
	 *     `OutResult` carries the failure reason on rejection (null controller, world mismatch).
	 *     Idempotent — re-registering the same controller returns the existing handle.
	 * ES: Registrar un controller AI como agente gestionado. Asigna un id estable nuevo y
	 *     almacena un weak pointer al controller. Retorna el handle asignado; el codigo de
	 *     resultado en `OutResult` lleva la razon del fallo en rechazos.
	 */
	FPGXAIResult ValidateControllerForRegistration(AAIController* Controller) const;

	FPGXAIAgentHandle RegisterAgent(AAIController* Controller, FPGXAIResult& OutResult);

	/**
	 * EN: Unregister a previously-registered handle. Idempotent — unregistering an invalid or
	 *     unknown handle is a no-op that returns NotFound rather than crashing. The registry
	 *     index is freed; the controller's weak pointer is dropped.
	 * ES: Desregistrar un handle previamente registrado. Idempotente.
	 */
	FPGXAIResult UnregisterAgent(const FPGXAIAgentHandle& Handle);

	/**
	 * EN: Snapshot the current agent registry. Returns a copy of the active handles for
	 *     inspector / diagnostics consumers. Stale entries (controllers that were GC'd without
	 *     calling Unregister) are filtered out at snapshot time but remain in the registry until
	 *     explicit cleanup; no periodic prune pass is provided.
	 * ES: Snapshot del registro de agentes actual. Filtra entradas stale al momento del snapshot.
	 */
	TArray<FPGXAIAgentHandle> GetAgentSnapshot() const;

	/**
	 * EN: Lookup a handle by id. Returns true if the id is registered and the controller is
	 *     still valid. Out-handle is left default-constructed on miss.
	 * ES: Buscar un handle por id. Retorna true si el id esta registrado.
	 */
	bool FindAgent(int32 AgentId, FPGXAIAgentHandle& OutHandle) const;

	FPGXAIResult TryRunBehaviorTreeForAgent(const FPGXAIAgentHandle& Handle, UBehaviorTree* BehaviorTree);

	bool GetBehaviorTreeRunStatus(const FPGXAIAgentHandle& Handle, FPGXAIBehaviorTreeRunStatus& OutStatus) const;

	int32 CleanupStaleAgents();

	/** EN: Number of currently-registered agents (does not filter stale entries). */
	int32 GetRegisteredAgentCount() const { return AgentRegistry.Num(); }

#if WITH_DEV_AUTOMATION_TESTS
	void SetForceNextBehaviorTreeRunResultForTesting(bool bInForce, bool bInResult);
#endif

private:
	/** EN: Monotonic id allocator. Starts at 1; 0 reserved for invalid handles. */
	int32 NextAgentId = 1;

	/** EN: Handle storage keyed by AgentId. */
	TMap<int32, FPGXAIAgentHandle> AgentRegistry;

	/** EN: Last Behavior Tree run status keyed by AgentId. */
	TMap<int32, FPGXAIBehaviorTreeRunStatus> BehaviorTreeRunRegistry;

#if WITH_DEV_AUTOMATION_TESTS
	bool bForceNextBehaviorTreeRunResultForTesting = false;
	bool bForcedNextBehaviorTreeRunResultForTesting = false;
#endif
};
