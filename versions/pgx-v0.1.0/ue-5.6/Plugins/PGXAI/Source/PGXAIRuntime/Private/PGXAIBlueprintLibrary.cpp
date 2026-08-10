// Copyright PGX Framework. All Rights Reserved.

#include "PGXAIBlueprintLibrary.h"

#include "PGXAISubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UPGXAISubsystem* UPGXAIBlueprintLibrary::GetAISubsystem(const UObject* WorldContextObject)
{
	if (!GEngine || !WorldContextObject)
	{
		return nullptr;
	}
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull))
	{
		return World->GetSubsystem<UPGXAISubsystem>();
	}
	return nullptr;
}

FPGXAIResult UPGXAIBlueprintLibrary::ValidateControllerForRegistration(const UObject* WorldContextObject, AAIController* Controller)
{
	if (UPGXAISubsystem* AI = GetAISubsystem(WorldContextObject))
	{
		return AI->ValidateControllerForRegistration(Controller);
	}
	return FPGXAIResult::MakeFailure(EPGXAIResultCode::SubsystemUnavailable, TEXT("AI subsystem unavailable."));
}

FPGXAIAgentHandle UPGXAIBlueprintLibrary::RegisterAgent(const UObject* WorldContextObject, AAIController* Controller, FPGXAIResult& OutResult)
{
	if (UPGXAISubsystem* AI = GetAISubsystem(WorldContextObject))
	{
		return AI->RegisterAgent(Controller, OutResult);
	}
	OutResult = FPGXAIResult::MakeFailure(EPGXAIResultCode::SubsystemUnavailable, TEXT("AI subsystem unavailable."));
	return FPGXAIAgentHandle();
}

FPGXAIResult UPGXAIBlueprintLibrary::UnregisterAgent(const UObject* WorldContextObject, const FPGXAIAgentHandle& Handle)
{
	if (UPGXAISubsystem* AI = GetAISubsystem(WorldContextObject))
	{
		return AI->UnregisterAgent(Handle);
	}
	return FPGXAIResult::MakeFailure(EPGXAIResultCode::SubsystemUnavailable, TEXT("AI subsystem unavailable."));
}

TArray<FPGXAIAgentHandle> UPGXAIBlueprintLibrary::GetAgentSnapshot(const UObject* WorldContextObject)
{
	if (UPGXAISubsystem* AI = GetAISubsystem(WorldContextObject))
	{
		return AI->GetAgentSnapshot();
	}
	return TArray<FPGXAIAgentHandle>();
}

bool UPGXAIBlueprintLibrary::FindAgent(const UObject* WorldContextObject, int32 AgentId, FPGXAIAgentHandle& OutHandle)
{
	if (UPGXAISubsystem* AI = GetAISubsystem(WorldContextObject))
	{
		return AI->FindAgent(AgentId, OutHandle);
	}
	OutHandle = FPGXAIAgentHandle();
	return false;
}

FPGXAIResult UPGXAIBlueprintLibrary::TryRunBehaviorTreeForAgent(const UObject* WorldContextObject, const FPGXAIAgentHandle& Handle, UBehaviorTree* BehaviorTree)
{
	if (UPGXAISubsystem* AI = GetAISubsystem(WorldContextObject))
	{
		return AI->TryRunBehaviorTreeForAgent(Handle, BehaviorTree);
	}
	return FPGXAIResult::MakeFailure(EPGXAIResultCode::SubsystemUnavailable, TEXT("AI subsystem unavailable."));
}

bool UPGXAIBlueprintLibrary::GetBehaviorTreeRunStatus(const UObject* WorldContextObject, const FPGXAIAgentHandle& Handle, FPGXAIBehaviorTreeRunStatus& OutStatus)
{
	if (UPGXAISubsystem* AI = GetAISubsystem(WorldContextObject))
	{
		return AI->GetBehaviorTreeRunStatus(Handle, OutStatus);
	}
	OutStatus = FPGXAIBehaviorTreeRunStatus();
	return false;
}

int32 UPGXAIBlueprintLibrary::CleanupStaleAgents(const UObject* WorldContextObject)
{
	if (UPGXAISubsystem* AI = GetAISubsystem(WorldContextObject))
	{
		return AI->CleanupStaleAgents();
	}
	return 0;
}

int32 UPGXAIBlueprintLibrary::GetRegisteredAgentCount(const UObject* WorldContextObject)
{
	if (UPGXAISubsystem* AI = GetAISubsystem(WorldContextObject))
	{
		return AI->GetRegisteredAgentCount();
	}
	return 0;
}
