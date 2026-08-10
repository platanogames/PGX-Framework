// Copyright PGX Framework. All Rights Reserved.

#include "PGXAISubsystem.h"
#include "PGXAIRuntime.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Engine/World.h"
#include "Subsystems/PGXLogSubsystem.h"
#include "HAL/PlatformTime.h"

void UPGXAISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	NextAgentId = 1;
	AgentRegistry.Reset();
	BehaviorTreeRunRegistry.Reset();

	PGX_LOG_INFO(LogPGXAI, TEXT("UPGXAISubsystem: Initialized (world: %s)"),
		GetWorld() ? *GetWorld()->GetName() : TEXT("<null>"));
}

void UPGXAISubsystem::Deinitialize()
{
	const int32 LeakedAgents = AgentRegistry.Num();
	if (LeakedAgents > 0)
	{
		PGX_LOG_WARNING(LogPGXAI,
			TEXT("UPGXAISubsystem: Deinitialize with %d agent(s) still registered. Owners should unregister during their EndPlay."),
			LeakedAgents);
	}

	AgentRegistry.Reset();
	BehaviorTreeRunRegistry.Reset();
	NextAgentId = 1;

	PGX_LOG_INFO(LogPGXAI, TEXT("UPGXAISubsystem: Deinitialized"));

	Super::Deinitialize();
}

// ============================================================================
// Agent Registry (agent registry baseline)
// ============================================================================

FPGXAIResult UPGXAISubsystem::ValidateControllerForRegistration(AAIController* Controller) const
{
	if (!IsValid(Controller))
	{
		return FPGXAIResult::MakeFailure(EPGXAIResultCode::InvalidInput,
			TEXT("ValidateControllerForRegistration: Controller is null/invalid"));
	}

	UWorld* SubsystemWorld = GetWorld();
	UWorld* ControllerWorld = Controller->GetWorld();
	if (!SubsystemWorld || !ControllerWorld)
	{
		return FPGXAIResult::MakeFailure(EPGXAIResultCode::InvalidWorld,
			TEXT("ValidateControllerForRegistration: Controller or subsystem world is invalid"));
	}

	if (ControllerWorld != SubsystemWorld)
	{
		PGX_LOG_WARNING(LogPGXAI, TEXT("UPGXAISubsystem::ValidateControllerForRegistration - world mismatch (controller=%s subsystem=%s)"),
			*ControllerWorld->GetName(), *SubsystemWorld->GetName());
		return FPGXAIResult::MakeFailure(EPGXAIResultCode::InvalidWorld,
			FString::Printf(TEXT("Controller world '%s' does not match subsystem world '%s'"),
				*ControllerWorld->GetName(), *SubsystemWorld->GetName()));
	}

	return FPGXAIResult::MakeSuccess();
}

FPGXAIAgentHandle UPGXAISubsystem::RegisterAgent(AAIController* Controller, FPGXAIResult& OutResult)
{
	OutResult = ValidateControllerForRegistration(Controller);
	if (!OutResult.bSucceeded)
	{
		return FPGXAIAgentHandle();
	}

	// EN: Idempotency — return existing handle if this controller is already registered.
	// ES: Idempotencia — retornar handle existente si el controller ya esta registrado.
	for (const TPair<int32, FPGXAIAgentHandle>& Pair : AgentRegistry)
	{
		if (Pair.Value.Controller.Get() == Controller)
		{
			OutResult = FPGXAIResult::MakeSuccess();
			return Pair.Value;
		}
	}

	FPGXAIAgentHandle NewHandle;
	NewHandle.AgentId = NextAgentId++;
	NewHandle.Controller = Controller;

	AgentRegistry.Add(NewHandle.AgentId, NewHandle);
	OutResult = FPGXAIResult::MakeSuccess();

	PGX_LOG_INFO(LogPGXAI, TEXT("UPGXAISubsystem::RegisterAgent — id=%d, controller=%s"),
		NewHandle.AgentId, *Controller->GetName());

	return NewHandle;
}

FPGXAIResult UPGXAISubsystem::UnregisterAgent(const FPGXAIAgentHandle& Handle)
{
	if (!Handle.IsValid())
	{
		return FPGXAIResult::MakeFailure(EPGXAIResultCode::InvalidInput,
			TEXT("UnregisterAgent: Handle is invalid (AgentId==0)"));
	}

	const int32 Removed = AgentRegistry.Remove(Handle.AgentId);
	BehaviorTreeRunRegistry.Remove(Handle.AgentId);
	if (Removed == 0)
	{
		return FPGXAIResult::MakeFailure(EPGXAIResultCode::NotFound,
			FString::Printf(TEXT("UnregisterAgent: AgentId=%d not registered"), Handle.AgentId));
	}

	PGX_LOG_INFO(LogPGXAI, TEXT("UPGXAISubsystem::UnregisterAgent — id=%d"), Handle.AgentId);

	return FPGXAIResult::MakeSuccess();
}

TArray<FPGXAIAgentHandle> UPGXAISubsystem::GetAgentSnapshot() const
{
	TArray<FPGXAIAgentHandle> Snapshot;
	Snapshot.Reserve(AgentRegistry.Num());
	for (const TPair<int32, FPGXAIAgentHandle>& Pair : AgentRegistry)
	{
		// EN: Filter stale entries (controllers GC'd without calling Unregister) at snapshot time.
		// ES: Filtrar entradas stale al momento del snapshot.
		if (Pair.Value.Controller.IsValid())
		{
			Snapshot.Add(Pair.Value);
		}
	}
	return Snapshot;
}

bool UPGXAISubsystem::FindAgent(int32 AgentId, FPGXAIAgentHandle& OutHandle) const
{
	const FPGXAIAgentHandle* Found = AgentRegistry.Find(AgentId);
	if (Found && Found->Controller.IsValid())
	{
		OutHandle = *Found;
		return true;
	}
	OutHandle = FPGXAIAgentHandle();
	return false;
}


FPGXAIResult UPGXAISubsystem::TryRunBehaviorTreeForAgent(const FPGXAIAgentHandle& Handle, UBehaviorTree* BehaviorTree)
{
	if (!Handle.IsValid())
	{
		return FPGXAIResult::MakeFailure(EPGXAIResultCode::InvalidInput,
			TEXT("TryRunBehaviorTreeForAgent: Handle is invalid"));
	}

	FPGXAIAgentHandle RegisteredHandle;
	if (!FindAgent(Handle.AgentId, RegisteredHandle))
	{
		return FPGXAIResult::MakeFailure(EPGXAIResultCode::NotFound,
			FString::Printf(TEXT("TryRunBehaviorTreeForAgent: AgentId=%d not registered"), Handle.AgentId));
	}

	AAIController* Controller = RegisteredHandle.Controller.Get();
	if (!IsValid(Controller))
	{
		return FPGXAIResult::MakeFailure(EPGXAIResultCode::NotFound,
			FString::Printf(TEXT("TryRunBehaviorTreeForAgent: AgentId=%d controller is stale"), Handle.AgentId));
	}

	if (!IsValid(BehaviorTree))
	{
		FPGXAIBehaviorTreeRunStatus Status;
		Status.AgentHandle = RegisteredHandle;
		Status.bRunAttempted = false;
		Status.bRunSucceeded = false;
		Status.LastResultCode = EPGXAIResultCode::BehaviorTreeUnavailable;
		Status.DiagnosticMessage = TEXT("BehaviorTree asset is null/invalid.");
		Status.Timestamp = FPlatformTime::Seconds();
		BehaviorTreeRunRegistry.Add(Handle.AgentId, Status);
		return FPGXAIResult::MakeFailure(EPGXAIResultCode::BehaviorTreeUnavailable, Status.DiagnosticMessage);
	}

	bool bRunSucceeded = false;
#if WITH_DEV_AUTOMATION_TESTS
	if (bForceNextBehaviorTreeRunResultForTesting)
	{
		bRunSucceeded = bForcedNextBehaviorTreeRunResultForTesting;
		bForceNextBehaviorTreeRunResultForTesting = false;
		bForcedNextBehaviorTreeRunResultForTesting = false;
	}
	else
#endif
	{
		bRunSucceeded = Controller->RunBehaviorTree(BehaviorTree);
	}

	FPGXAIBehaviorTreeRunStatus Status;
	Status.AgentHandle = RegisteredHandle;
	Status.BehaviorTree = BehaviorTree;
	Status.bRunAttempted = true;
	Status.bRunSucceeded = bRunSucceeded;
	Status.LastResultCode = bRunSucceeded ? EPGXAIResultCode::Success : EPGXAIResultCode::BehaviorTreeRunFailed;
	Status.DiagnosticMessage = bRunSucceeded ? TEXT("BehaviorTree run accepted.") : TEXT("AIController::RunBehaviorTree failed.");
	Status.Timestamp = FPlatformTime::Seconds();
	BehaviorTreeRunRegistry.Add(Handle.AgentId, Status);

	if (!bRunSucceeded)
	{
		PGX_LOG_WARNING(LogPGXAI, TEXT("UPGXAISubsystem::TryRunBehaviorTreeForAgent - RunBehaviorTree failed (id=%d, bt=%s)"),
			Handle.AgentId, *GetNameSafe(BehaviorTree));
		return FPGXAIResult::MakeFailure(EPGXAIResultCode::BehaviorTreeRunFailed, Status.DiagnosticMessage);
	}

	PGX_LOG_INFO(LogPGXAI, TEXT("UPGXAISubsystem::TryRunBehaviorTreeForAgent - id=%d, bt=%s"),
		Handle.AgentId, *GetNameSafe(BehaviorTree));
	return FPGXAIResult::MakeSuccess();
}

bool UPGXAISubsystem::GetBehaviorTreeRunStatus(const FPGXAIAgentHandle& Handle, FPGXAIBehaviorTreeRunStatus& OutStatus) const
{
	if (!Handle.IsValid())
	{
		OutStatus = FPGXAIBehaviorTreeRunStatus();
		return false;
	}

	const FPGXAIBehaviorTreeRunStatus* Found = BehaviorTreeRunRegistry.Find(Handle.AgentId);
	if (!Found)
	{
		OutStatus = FPGXAIBehaviorTreeRunStatus();
		return false;
	}

	OutStatus = *Found;
	return true;
}

int32 UPGXAISubsystem::CleanupStaleAgents()
{
	int32 RemovedCount = 0;
	for (auto It = AgentRegistry.CreateIterator(); It; ++It)
	{
		if (!It.Value().Controller.IsValid())
		{
			BehaviorTreeRunRegistry.Remove(It.Key());
			It.RemoveCurrent();
			++RemovedCount;
		}
	}
	return RemovedCount;
}

#if WITH_DEV_AUTOMATION_TESTS
void UPGXAISubsystem::SetForceNextBehaviorTreeRunResultForTesting(bool bInForce, bool bInResult)
{
	bForceNextBehaviorTreeRunResultForTesting = bInForce;
	bForcedNextBehaviorTreeRunResultForTesting = bInResult;
}
#endif
