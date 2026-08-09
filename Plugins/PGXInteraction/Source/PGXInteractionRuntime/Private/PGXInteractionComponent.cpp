// Copyright PGX Framework. All Rights Reserved.

#include "PGXInteractionComponent.h"

#include "PGXInteractable.h"
#include "PGXInteractionCondition.h"
#include "Logging/PGXLogCategories.h"
#include "Logging/PGXLogMacros.h"
#include "HAL/PlatformTime.h"
#include "GameFramework/Actor.h"

UPGXInteractionComponent::UPGXInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

FPGXInteractionResult UPGXInteractionComponent::RegisterTarget(AActor* TargetActor, FGameplayTag TargetTag, FText PromptText, int32 Priority)
{
	if (!IsValid(TargetActor))
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXInteraction: RegisterTarget rejected invalid target actor"));
		return FPGXInteractionResult::Failure(EPGXInteractionResultCode::InvalidTarget, EPGXInteractionActionState::Failed, TEXT("Invalid interaction target."));
	}

	FPGXInteractableTarget Target;
	Target.Handle = FPGXInteractionHandle::NewHandle();
	Target.TargetActor = TargetActor;
	Target.TargetTag = TargetTag;
	Target.PromptText = MoveTemp(PromptText);
	Target.Priority = Priority;
	RegisteredTargets.Add(Target);

	return FPGXInteractionResult::Success(FPGXInteractionHandle(), Target.Handle, EPGXInteractionActionState::None, TEXT("Interaction target registered."));
}

FPGXInteractionResult UPGXInteractionComponent::UnregisterTarget(FPGXInteractionHandle TargetHandle)
{
	if (!TargetHandle.IsValid())
	{
		return FPGXInteractionResult::Failure(EPGXInteractionResultCode::InvalidTarget, EPGXInteractionActionState::Failed, TEXT("Invalid target handle."), FPGXInteractionHandle(), TargetHandle);
	}

	const int32 RemovedCount = RegisteredTargets.RemoveAll([TargetHandle](const FPGXInteractableTarget& Target)
	{
		return Target.Handle.Id == TargetHandle.Id;
	});

	if (RemovedCount <= 0)
	{
		return FPGXInteractionResult::Failure(EPGXInteractionResultCode::TargetNotRegistered, EPGXInteractionActionState::Failed, TEXT("Interaction target not registered."), FPGXInteractionHandle(), TargetHandle);
	}

	for (FPGXInteractionRecord& Record : InteractionRecords)
	{
		if (Record.TargetHandle.Id == TargetHandle.Id && IsRecordActive(Record))
		{
			Record.State = EPGXInteractionActionState::Cancelled;
			Record.ResultCode = EPGXInteractionResultCode::Success;
			Record.Message = TEXT("Interaction target unregistered; action cancelled.");
			Record.ResolvedTimeSeconds = FPlatformTime::Seconds();
		}
	}

	return FPGXInteractionResult::Success(FPGXInteractionHandle(), TargetHandle, EPGXInteractionActionState::Cancelled, TEXT("Interaction target unregistered."));
}

FPGXInteractionResult UPGXInteractionComponent::ValidateTarget(FPGXInteractionHandle TargetHandle) const
{
	const FPGXInteractableTarget* Target = FindTarget(TargetHandle);
	if (!Target)
	{
		return FPGXInteractionResult::Failure(EPGXInteractionResultCode::TargetNotRegistered, EPGXInteractionActionState::Failed, TEXT("Interaction target not registered."), FPGXInteractionHandle(), TargetHandle);
	}
	if (!IsValid(Target->TargetActor))
	{
		return FPGXInteractionResult::Failure(EPGXInteractionResultCode::InvalidTarget, EPGXInteractionActionState::Failed, TEXT("Interaction target actor is invalid."), FPGXInteractionHandle(), TargetHandle);
	}
	return FPGXInteractionResult::Success(FPGXInteractionHandle(), TargetHandle, EPGXInteractionActionState::None, TEXT("Interaction target valid."));
}

FPGXInteractionResult UPGXInteractionComponent::BeginInteraction(FPGXInteractionHandle TargetHandle, FGameplayTag ActionTag)
{
	const FPGXInteractionResult TargetResult = ValidateTarget(TargetHandle);
	if (!TargetResult.bSuccess)
	{
		return TargetResult;
	}
	if (!ActionTag.IsValid())
	{
		return FPGXInteractionResult::Failure(EPGXInteractionResultCode::InvalidAction, EPGXInteractionActionState::Failed, TEXT("Invalid interaction action tag."), FPGXInteractionHandle(), TargetHandle);
	}

	const bool bAlreadyActive = InteractionRecords.ContainsByPredicate([this, TargetHandle](const FPGXInteractionRecord& Record)
	{
		return Record.TargetHandle.Id == TargetHandle.Id && IsRecordActive(Record);
	});
	if (bAlreadyActive)
	{
		return FPGXInteractionResult::Failure(EPGXInteractionResultCode::AlreadyActive, EPGXInteractionActionState::Started, TEXT("Interaction already active for target."), FPGXInteractionHandle(), TargetHandle);
	}

	FPGXInteractionRecord Record;
	Record.ActionHandle = FPGXInteractionHandle::NewHandle();
	Record.TargetHandle = TargetHandle;
	Record.ActionTag = ActionTag;
	Record.State = EPGXInteractionActionState::Started;
	Record.ResultCode = EPGXInteractionResultCode::Success;
	Record.Message = TEXT("Interaction started.");
	Record.StartedTimeSeconds = FPlatformTime::Seconds();
	InteractionRecords.Add(Record);

	return FPGXInteractionResult::Success(Record.ActionHandle, TargetHandle, Record.State, Record.Message);
}

FPGXInteractionResult UPGXInteractionComponent::CompleteInteraction(FPGXInteractionHandle ActionHandle)
{
	FPGXInteractionRecord* Record = FindRecordMutable(ActionHandle);
	if (!Record)
	{
		return FPGXInteractionResult::Failure(EPGXInteractionResultCode::NoActiveAction, EPGXInteractionActionState::Failed, TEXT("Interaction action not found."), ActionHandle);
	}
	if (!IsRecordActive(*Record))
	{
		return FPGXInteractionResult::Failure(EPGXInteractionResultCode::AlreadyResolved, Record->State, TEXT("Interaction action already resolved."), ActionHandle, Record->TargetHandle);
	}

	Record->State = EPGXInteractionActionState::Completed;
	Record->ResultCode = EPGXInteractionResultCode::Success;
	Record->Message = TEXT("Interaction completed.");
	Record->ResolvedTimeSeconds = FPlatformTime::Seconds();
	return FPGXInteractionResult::Success(ActionHandle, Record->TargetHandle, Record->State, Record->Message);
}

FPGXInteractionResult UPGXInteractionComponent::CancelInteraction(FPGXInteractionHandle ActionHandle)
{
	FPGXInteractionRecord* Record = FindRecordMutable(ActionHandle);
	if (!Record)
	{
		return FPGXInteractionResult::Failure(EPGXInteractionResultCode::NoActiveAction, EPGXInteractionActionState::Failed, TEXT("Interaction action not found."), ActionHandle);
	}
	if (!IsRecordActive(*Record))
	{
		return FPGXInteractionResult::Failure(EPGXInteractionResultCode::AlreadyResolved, Record->State, TEXT("Interaction action already resolved."), ActionHandle, Record->TargetHandle);
	}

	Record->State = EPGXInteractionActionState::Cancelled;
	Record->ResultCode = EPGXInteractionResultCode::Success;
	Record->Message = TEXT("Interaction cancelled.");
	Record->ResolvedTimeSeconds = FPlatformTime::Seconds();
	return FPGXInteractionResult::Success(ActionHandle, Record->TargetHandle, Record->State, Record->Message);
}

int32 UPGXInteractionComponent::CleanupResolvedInteractions()
{
	const int32 BeforeCount = InteractionRecords.Num();
	InteractionRecords.RemoveAll([this](const FPGXInteractionRecord& Record)
	{
		return !IsRecordActive(Record);
	});
	return BeforeCount - InteractionRecords.Num();
}

FPGXInteractionResult UPGXInteractionComponent::EvaluateConditionTyped(const UPGXInteractionCondition* Condition, AActor* Interactor) const
{
	if (!Condition)
	{
		return FPGXInteractionResult::Success(FPGXInteractionHandle(), FPGXInteractionHandle(), EPGXInteractionActionState::None, TEXT("No interaction condition supplied."));
	}
	if (!IsValid(Interactor))
	{
		return FPGXInteractionResult::Failure(EPGXInteractionResultCode::InvalidInteractor, EPGXInteractionActionState::Failed, TEXT("Invalid interactor actor."));
	}

	const bool bConditionPassed = Condition->EvaluateCondition(Interactor);
	return bConditionPassed
		? FPGXInteractionResult::Success(FPGXInteractionHandle(), FPGXInteractionHandle(), EPGXInteractionActionState::None, TEXT("Interaction condition passed."))
		: FPGXInteractionResult::Failure(EPGXInteractionResultCode::ConditionFailed, EPGXInteractionActionState::Failed, TEXT("Interaction condition failed."));
}

FPGXInteractionQueryResult UPGXInteractionComponent::QueryBestTargetFromOwner(float MaxRange, bool bRequireInteractableInterface) const
{
	AActor* OwnerActor = GetOwner();
	if (!IsValid(OwnerActor))
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXInteraction: QueryBestTargetFromOwner rejected missing owner."));
		return FPGXInteractionQueryResult::Failure(EPGXInteractionResultCode::OwnerMissing, TEXT("Interaction component has no valid owner actor."));
	}

	if (!GetWorld())
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXInteraction: QueryBestTargetFromOwner rejected unavailable world."));
		return FPGXInteractionQueryResult::Failure(EPGXInteractionResultCode::WorldUnavailable, TEXT("Interaction component has no valid world."));
	}

	return QueryBestTargetFromLocation(OwnerActor, OwnerActor->GetActorLocation(), MaxRange, bRequireInteractableInterface);
}

FPGXInteractionQueryResult UPGXInteractionComponent::QueryBestTargetFromLocation(AActor* Interactor, FVector Origin, float MaxRange, bool bRequireInteractableInterface) const
{
	if (!IsValid(Interactor))
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXInteraction: QueryBestTargetFromLocation rejected invalid interactor."));
		return FPGXInteractionQueryResult::Failure(EPGXInteractionResultCode::InvalidInteractor, TEXT("Invalid interactor actor."));
	}

	const float EffectiveRange = MaxRange >= 0.0f ? MaxRange : InteractionRange;
	const float EffectiveRangeSq = FMath::Square(FMath::Max(EffectiveRange, 0.0f));
	const FPGXInteractableTarget* BestTarget = nullptr;
	float BestDistanceSq = TNumericLimits<float>::Max();
	bool bSawValidActor = false;
	bool bSawOutOfRangeTarget = false;
	bool bSawInvalidInterfaceTarget = false;

	for (const FPGXInteractableTarget& Target : RegisteredTargets)
	{
		AActor* TargetActor = Target.TargetActor;
		if (!IsValid(TargetActor))
		{
			continue;
		}

		bSawValidActor = true;
		const float DistanceSq = FVector::DistSquared(Origin, TargetActor->GetActorLocation());
		if (DistanceSq > EffectiveRangeSq)
		{
			bSawOutOfRangeTarget = true;
			continue;
		}

		if (bRequireInteractableInterface && !TargetActor->GetClass()->ImplementsInterface(UPGXInteractable::StaticClass()))
		{
			bSawInvalidInterfaceTarget = true;
			continue;
		}

		const bool bBetterPriority = !BestTarget || Target.Priority > BestTarget->Priority;
		const bool bSamePriorityCloser = BestTarget && Target.Priority == BestTarget->Priority && DistanceSq < BestDistanceSq;
		if (bBetterPriority || bSamePriorityCloser)
		{
			BestTarget = &Target;
			BestDistanceSq = DistanceSq;
		}
	}

	if (BestTarget)
	{
		return BuildPromptSnapshot(BestTarget->Handle, FGameplayTag(), FMath::Sqrt(BestDistanceSq));
	}

	if (bSawInvalidInterfaceTarget)
	{
		return FPGXInteractionQueryResult::Failure(EPGXInteractionResultCode::InvalidInterface, TEXT("Candidate target does not implement PGXInteractable."));
	}

	if (bSawOutOfRangeTarget)
	{
		return FPGXInteractionQueryResult::Failure(EPGXInteractionResultCode::TargetOutOfRange, TEXT("No interaction target is inside configured range."));
	}

	if (!bSawValidActor)
	{
		return FPGXInteractionQueryResult::Failure(EPGXInteractionResultCode::NoTargetFound, TEXT("No valid interaction targets are registered."));
	}

	return FPGXInteractionQueryResult::Failure(EPGXInteractionResultCode::NoTargetFound, TEXT("No interaction target matched the query."));
}

FPGXInteractionQueryResult UPGXInteractionComponent::BuildPromptSnapshot(FPGXInteractionHandle TargetHandle, FGameplayTag ActionTag, float Distance) const
{
	const FPGXInteractableTarget* Target = FindTarget(TargetHandle);
	if (!Target)
	{
		return FPGXInteractionQueryResult::Failure(EPGXInteractionResultCode::TargetNotRegistered, TEXT("Interaction target not registered."), TargetHandle);
	}

	if (!IsValid(Target->TargetActor))
	{
		return FPGXInteractionQueryResult::Failure(EPGXInteractionResultCode::InvalidTarget, TEXT("Interaction target actor is invalid."), TargetHandle);
	}

	FPGXInteractionPromptSnapshot Snapshot;
	Snapshot.bHasPrompt = true;
	Snapshot.bPresentationOnly = true;
	Snapshot.TargetHandle = Target->Handle;
	Snapshot.TargetTag = Target->TargetTag;
	Snapshot.ActionTag = ActionTag;
	Snapshot.PromptText = Target->PromptText;
	Snapshot.Distance = FMath::Max(Distance, 0.0f);
	Snapshot.Priority = Target->Priority;
	return FPGXInteractionQueryResult::Success(MoveTemp(Snapshot), TEXT("Interaction prompt snapshot built."));
}

bool UPGXInteractionComponent::HasTarget(FPGXInteractionHandle TargetHandle) const
{
	return FindTarget(TargetHandle) != nullptr;
}

int32 UPGXInteractionComponent::GetRegisteredTargetCount() const
{
	return RegisteredTargets.Num();
}

int32 UPGXInteractionComponent::GetActiveInteractionCount() const
{
	int32 ActiveCount = 0;
	for (const FPGXInteractionRecord& Record : InteractionRecords)
	{
		if (IsRecordActive(Record))
		{
			++ActiveCount;
		}
	}
	return ActiveCount;
}

int32 UPGXInteractionComponent::GetInteractionRecordCount() const
{
	return InteractionRecords.Num();
}

TArray<FPGXInteractableTarget> UPGXInteractionComponent::GetTargetsSnapshot() const
{
	TArray<FPGXInteractableTarget> Snapshot = RegisteredTargets;
	Snapshot.Sort([](const FPGXInteractableTarget& Left, const FPGXInteractableTarget& Right)
	{
		return Left.Priority > Right.Priority;
	});
	return Snapshot;
}

TArray<FPGXInteractionRecord> UPGXInteractionComponent::GetInteractionRecordsSnapshot() const
{
	return InteractionRecords;
}

FPGXInteractableTarget* UPGXInteractionComponent::FindTargetMutable(FPGXInteractionHandle TargetHandle)
{
	return RegisteredTargets.FindByPredicate([TargetHandle](const FPGXInteractableTarget& Target)
	{
		return Target.Handle.Id == TargetHandle.Id;
	});
}

const FPGXInteractableTarget* UPGXInteractionComponent::FindTarget(FPGXInteractionHandle TargetHandle) const
{
	return RegisteredTargets.FindByPredicate([TargetHandle](const FPGXInteractableTarget& Target)
	{
		return Target.Handle.Id == TargetHandle.Id;
	});
}

FPGXInteractionRecord* UPGXInteractionComponent::FindRecordMutable(FPGXInteractionHandle ActionHandle)
{
	return InteractionRecords.FindByPredicate([ActionHandle](const FPGXInteractionRecord& Record)
	{
		return Record.ActionHandle.Id == ActionHandle.Id;
	});
}

const FPGXInteractionRecord* UPGXInteractionComponent::FindRecord(FPGXInteractionHandle ActionHandle) const
{
	return InteractionRecords.FindByPredicate([ActionHandle](const FPGXInteractionRecord& Record)
	{
		return Record.ActionHandle.Id == ActionHandle.Id;
	});
}

bool UPGXInteractionComponent::IsRecordActive(const FPGXInteractionRecord& Record) const
{
	return Record.State == EPGXInteractionActionState::Requested || Record.State == EPGXInteractionActionState::Started;
}
