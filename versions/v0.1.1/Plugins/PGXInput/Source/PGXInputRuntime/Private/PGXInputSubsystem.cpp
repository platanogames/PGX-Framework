// Copyright PGX Framework. All Rights Reserved.

#include "PGXInputSubsystem.h"

#include "PGXInputBuffer.h"
#include "PGXInputConfig.h"
#include "PGXInputContext.h"
#include "PGXInputSettings.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Logging/PGXLogCategories.h"
#include "Logging/PGXLogMacros.h"

void UPGXInputSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	ActiveContexts.Reset();
	ContextCache.Reset();
	AppliedContextsByLocalPlayer.Reset();
	InputBuffer = nullptr;
	InputConfig = nullptr;

	const UPGXInputSettings* Settings = GetDefault<UPGXInputSettings>();
	if (Settings && !Settings->ActiveConfig.IsNull())
	{
		InputConfig = Settings->ActiveConfig.LoadSynchronous();
	}
	EnsureRuntimeObjects();
	RebuildContextCache();
}

void UPGXInputSubsystem::Deinitialize()
{
	DeactivateAllContexts();
	ContextCache.Reset();
	if (InputBuffer)
	{
		InputBuffer->Clear();
	}
	InputBuffer = nullptr;
	InputConfig = nullptr;
	Super::Deinitialize();
}

FPGXInputContextResult UPGXInputSubsystem::ActivateContext(FGameplayTag ContextTag, int32 PriorityOverride)
{
	EnsureRuntimeObjects();

	if (!ContextTag.IsValid())
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXInput: ActivateContext rejected invalid context tag"));
		return FPGXInputContextResult::Failure(EPGXInputContextResultCode::InvalidTag, ContextTag, TEXT("Invalid input context tag."));
	}

	if (IsContextActive(ContextTag))
	{
		FPGXInputContextResult Result;
		Result.bSuccess = true;
		Result.Code = EPGXInputContextResultCode::AlreadyActive;
		Result.ContextTag = ContextTag;
		Result.Message = FString::Printf(TEXT("Input context already active: %s"), *ContextTag.ToString());
		return Result;
	}

	UPGXInputContext* Context = FindContextAsset(ContextTag);
	if (!Context)
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXInput: context not found for tag %s"), *ContextTag.ToString());
		return FPGXInputContextResult::Failure(EPGXInputContextResultCode::ContextNotFound, ContextTag, TEXT("Input context not found."));
	}

	const int32 ResolvedPriority = ResolvePriority(Context, PriorityOverride);
	if (Context->ActivationMode == EPGXInputContextActivationMode::Exclusive)
	{
		ActiveContexts.RemoveAll([ResolvedPriority](const FPGXActiveInputContextEntry& Entry)
		{
			return Entry.Priority <= ResolvedPriority;
		});
	}

	FPGXActiveInputContextEntry ActiveEntry;
	ActiveEntry.ContextTag = ContextTag;
	ActiveEntry.Priority = ResolvedPriority;
	ActiveEntry.ActivationMode = Context->ActivationMode;
	ActiveEntry.Context = Context;
	ActiveContexts.Add(ActiveEntry);
	SortActiveContexts();

	return FPGXInputContextResult::Success(ContextTag, TEXT("Input context activated in PGX stack."));
}

FPGXInputContextResult UPGXInputSubsystem::ActivateContextForLocalPlayer(FGameplayTag ContextTag, ULocalPlayer* LocalPlayer, int32 PriorityOverride)
{
	EnsureRuntimeObjects();

	UPGXInputContext* Context = nullptr;
	UInputMappingContext* MappingContext = nullptr;
	UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = nullptr;
	FPGXInputContextResult ResolveResult = ResolveEnhancedInputApplyTargets(ContextTag, LocalPlayer, Context, MappingContext, EnhancedInputSubsystem);
	if (!ResolveResult.bSuccess)
	{
		return ResolveResult;
	}

	const int32 ResolvedPriority = ResolvePriority(Context, PriorityOverride);
	if (Context->ActivationMode == EPGXInputContextActivationMode::Exclusive)
	{
		RemoveAppliedContextsPrunedByExclusive(ResolvedPriority);
	}

	const FPGXInputContextResult StackResult = ActivateContext(ContextTag, PriorityOverride);
	if (!StackResult.bSuccess)
	{
		return StackResult;
	}

	TSet<FGameplayTag>& AppliedContexts = AppliedContextsByLocalPlayer.FindOrAdd(TObjectKey<ULocalPlayer>(LocalPlayer));
	const bool bAlreadyAppliedToLocalPlayer = AppliedContexts.Contains(ContextTag);
	if (!bAlreadyAppliedToLocalPlayer)
	{
		EnhancedInputSubsystem->AddMappingContext(MappingContext, ResolvedPriority);
		AppliedContexts.Add(ContextTag);
	}

	if (StackResult.Code == EPGXInputContextResultCode::AlreadyActive)
	{
		FPGXInputContextResult Result = StackResult;
		Result.Message = FString::Printf(TEXT("Input context already active in PGX stack; %s Enhanced Input mapping for LocalPlayer: %s"),
			bAlreadyAppliedToLocalPlayer ? TEXT("kept existing") : TEXT("applied"),
			*ContextTag.ToString());
		return Result;
	}

	return FPGXInputContextResult::Success(ContextTag, TEXT("Input context activated in PGX stack and applied to LocalPlayer Enhanced Input."));
}

FPGXInputContextResult UPGXInputSubsystem::DeactivateContext(FGameplayTag ContextTag)
{
	if (!ContextTag.IsValid())
	{
		return FPGXInputContextResult::Failure(EPGXInputContextResultCode::InvalidTag, ContextTag, TEXT("Invalid input context tag."));
	}

	const int32 RemovedCount = ActiveContexts.RemoveAll([ContextTag](const FPGXActiveInputContextEntry& Entry)
	{
		return Entry.ContextTag == ContextTag;
	});
	TArray<ULocalPlayer*> LocalPlayersToRemove;
	for (const TPair<TObjectKey<ULocalPlayer>, TSet<FGameplayTag>>& Pair : AppliedContextsByLocalPlayer)
	{
		if (Pair.Value.Contains(ContextTag))
		{
			if (ULocalPlayer* LocalPlayer = Pair.Key.ResolveObjectPtr())
			{
				LocalPlayersToRemove.Add(LocalPlayer);
			}
		}
	}
	for (ULocalPlayer* LocalPlayer : LocalPlayersToRemove)
	{
		RemoveAppliedContextFromLocalPlayer(LocalPlayer, ContextTag);
	}

	if (RemovedCount <= 0)
	{
		FPGXInputContextResult Result;
		Result.bSuccess = true;
		Result.Code = EPGXInputContextResultCode::AlreadyInactive;
		Result.ContextTag = ContextTag;
		Result.Message = FString::Printf(TEXT("Input context already inactive: %s"), *ContextTag.ToString());
		return Result;
	}

	return FPGXInputContextResult::Success(ContextTag, TEXT("Input context deactivated from PGX stack."));
}

FPGXInputContextResult UPGXInputSubsystem::DeactivateContextForLocalPlayer(FGameplayTag ContextTag, ULocalPlayer* LocalPlayer)
{
	if (!ContextTag.IsValid())
	{
		return FPGXInputContextResult::Failure(EPGXInputContextResultCode::InvalidTag, ContextTag, TEXT("Invalid input context tag."));
	}

	if (!LocalPlayer)
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXInput: DeactivateContextForLocalPlayer rejected missing LocalPlayer for %s"), *ContextTag.ToString());
		return FPGXInputContextResult::Failure(EPGXInputContextResultCode::LocalPlayerNotReady, ContextTag, TEXT("LocalPlayer is required for Enhanced Input removal."));
	}

	UPGXInputContext* Context = FindContextAsset(ContextTag);
	if (!Context)
	{
		return FPGXInputContextResult::Failure(EPGXInputContextResultCode::ContextNotFound, ContextTag, TEXT("Input context not found."));
	}

	UInputMappingContext* MappingContext = Context->MappingContext.Get();
	if (!MappingContext && !Context->MappingContext.IsNull())
	{
		MappingContext = Context->MappingContext.LoadSynchronous();
	}
	if (!MappingContext)
	{
		return FPGXInputContextResult::Failure(EPGXInputContextResultCode::MappingContextMissing, ContextTag, TEXT("Enhanced Input mapping context is missing."));
	}

	UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!EnhancedInputSubsystem)
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXInput: Enhanced Input LocalPlayer subsystem unavailable while removing %s"), *ContextTag.ToString());
		return FPGXInputContextResult::Failure(EPGXInputContextResultCode::LocalPlayerNotReady, ContextTag, TEXT("Enhanced Input LocalPlayer subsystem is unavailable."));
	}

	EnhancedInputSubsystem->RemoveMappingContext(MappingContext);
	if (TSet<FGameplayTag>* AppliedContexts = AppliedContextsByLocalPlayer.Find(TObjectKey<ULocalPlayer>(LocalPlayer)))
	{
		AppliedContexts->Remove(ContextTag);
		if (AppliedContexts->Num() == 0)
		{
			AppliedContextsByLocalPlayer.Remove(TObjectKey<ULocalPlayer>(LocalPlayer));
		}
	}

	bool bStillAppliedToAnotherLocalPlayer = false;
	for (const TPair<TObjectKey<ULocalPlayer>, TSet<FGameplayTag>>& Pair : AppliedContextsByLocalPlayer)
	{
		if (Pair.Value.Contains(ContextTag))
		{
			bStillAppliedToAnotherLocalPlayer = true;
			break;
		}
	}
	if (bStillAppliedToAnotherLocalPlayer)
	{
		return FPGXInputContextResult::Success(ContextTag, TEXT("Input context removed from LocalPlayer Enhanced Input; PGX stack kept for another LocalPlayer."));
	}

	return DeactivateContext(ContextTag);
}

void UPGXInputSubsystem::DeactivateAllContexts()
{
	TArray<TPair<ULocalPlayer*, FGameplayTag>> RemovalPairs;
	for (const FPGXActiveInputContextEntry& Entry : ActiveContexts)
	{
		for (const TPair<TObjectKey<ULocalPlayer>, TSet<FGameplayTag>>& Pair : AppliedContextsByLocalPlayer)
		{
			if (Pair.Value.Contains(Entry.ContextTag))
			{
				if (ULocalPlayer* LocalPlayer = Pair.Key.ResolveObjectPtr())
				{
					RemovalPairs.Emplace(LocalPlayer, Entry.ContextTag);
				}
			}
		}
	}
	for (const TPair<ULocalPlayer*, FGameplayTag>& RemovalPair : RemovalPairs)
	{
		RemoveAppliedContextFromLocalPlayer(RemovalPair.Key, RemovalPair.Value);
	}
	AppliedContextsByLocalPlayer.Reset();
	ActiveContexts.Reset();
}

void UPGXInputSubsystem::DeactivateAllContextsForLocalPlayer(ULocalPlayer* LocalPlayer)
{
	if (!LocalPlayer)
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXInput: DeactivateAllContextsForLocalPlayer rejected missing LocalPlayer"));
		return;
	}

	if (TSet<FGameplayTag>* AppliedContexts = AppliedContextsByLocalPlayer.Find(TObjectKey<ULocalPlayer>(LocalPlayer)))
	{
		TArray<FGameplayTag> ContextTags = AppliedContexts->Array();
		for (const FGameplayTag& ContextTag : ContextTags)
		{
			RemoveAppliedContextFromLocalPlayer(LocalPlayer, ContextTag);
		}
		AppliedContextsByLocalPlayer.Remove(TObjectKey<ULocalPlayer>(LocalPlayer));
	}
}

bool UPGXInputSubsystem::IsContextActive(FGameplayTag ContextTag) const
{
	return ActiveContexts.ContainsByPredicate([ContextTag](const FPGXActiveInputContextEntry& Entry)
	{
		return Entry.ContextTag == ContextTag;
	});
}

TArray<FPGXActiveInputContextEntry> UPGXInputSubsystem::GetActiveContexts() const
{
	return ActiveContexts;
}

int32 UPGXInputSubsystem::GetActiveContextCount() const
{
	return ActiveContexts.Num();
}

UPGXInputContext* UPGXInputSubsystem::FindContextAsset(FGameplayTag ContextTag) const
{
	if (!ContextTag.IsValid())
	{
		return nullptr;
	}

	if (const TObjectPtr<UPGXInputContext>* CachedContext = ContextCache.Find(ContextTag))
	{
		return CachedContext->Get();
	}

	if (!InputConfig)
	{
		return nullptr;
	}

	for (const FPGXInputContextEntry& Entry : InputConfig->DefaultContexts)
	{
		UPGXInputContext* Context = Entry.Context.Get();
		if (!Context && !Entry.Context.IsNull())
		{
			Context = Entry.Context.LoadSynchronous();
		}

		const FGameplayTag ResolvedTag = Entry.ContextTag.IsValid() || !Context ? Entry.ContextTag : Context->ContextTag;
		if (ResolvedTag == ContextTag)
		{
			return Context;
		}
	}

	return nullptr;
}

UPGXInputConfig* UPGXInputSubsystem::GetActiveInputConfig() const
{
	return InputConfig;
}

UPGXInputBuffer* UPGXInputSubsystem::GetInputBuffer() const
{
	return InputBuffer;
}

bool UPGXInputSubsystem::HasEntryByTag(FGameplayTag Tag) const
{
	return FindContextAsset(Tag) != nullptr;
}

int32 UPGXInputSubsystem::GetCount() const
{
	return ContextCache.Num();
}

void UPGXInputSubsystem::GetSnapshot(TArray<FGameplayTag>& OutTags) const
{
	ContextCache.GetKeys(OutTags);
}

#if WITH_DEV_AUTOMATION_TESTS
void UPGXInputSubsystem::InjectTestInputConfig(UPGXInputConfig* InConfig)
{
	InputConfig = InConfig;
	EnsureRuntimeObjects();
	if (InputBuffer && InputConfig)
	{
		InputBuffer->Clear();
		InputBuffer->Configure(InputConfig->InputBufferCapacity, InputConfig->InputBufferWindowSeconds);
	}
	ContextCache.Reset();
	RebuildContextCache();
}

void UPGXInputSubsystem::InjectTestContext(UPGXInputContext* InContext)
{
	if (!InContext || !InContext->ContextTag.IsValid())
	{
		return;
	}
	ContextCache.Add(InContext->ContextTag, InContext);
}

void UPGXInputSubsystem::ClearTestContexts()
{
	ContextCache.Reset();
	ActiveContexts.Reset();
	AppliedContextsByLocalPlayer.Reset();
	RebuildContextCache();
}
#endif

void UPGXInputSubsystem::EnsureRuntimeObjects()
{
	if (!InputConfig)
	{
		InputConfig = NewObject<UPGXInputConfig>(this, UPGXInputConfig::StaticClass(), NAME_None, RF_Transient);
	}

	if (!InputBuffer)
	{
		InputBuffer = NewObject<UPGXInputBuffer>(this, UPGXInputBuffer::StaticClass(), NAME_None, RF_Transient);
		if (InputBuffer && InputConfig)
		{
			InputBuffer->Configure(InputConfig->InputBufferCapacity, InputConfig->InputBufferWindowSeconds);
		}
	}
}

void UPGXInputSubsystem::RebuildContextCache()
{
	if (!InputConfig)
	{
		return;
	}

	for (const FPGXInputContextEntry& Entry : InputConfig->DefaultContexts)
	{
		UPGXInputContext* Context = Entry.Context.Get();
		if (!Context && !Entry.Context.IsNull())
		{
			Context = Entry.Context.LoadSynchronous();
		}
		if (!Context)
		{
			continue;
		}

		const FGameplayTag ResolvedTag = Entry.ContextTag.IsValid() ? Entry.ContextTag : Context->ContextTag;
		if (ResolvedTag.IsValid())
		{
			if (!ContextCache.Contains(ResolvedTag))
			{
				ContextCache.Add(ResolvedTag, Context);
			}
		}
	}
}

void UPGXInputSubsystem::SortActiveContexts()
{
	ActiveContexts.Sort([](const FPGXActiveInputContextEntry& Left, const FPGXActiveInputContextEntry& Right)
	{
		if (Left.Priority != Right.Priority)
		{
			return Left.Priority > Right.Priority;
		}
		return Left.ContextTag.ToString() < Right.ContextTag.ToString();
	});
}

int32 UPGXInputSubsystem::ResolvePriority(const UPGXInputContext* Context, int32 PriorityOverride) const
{
	if (PriorityOverride != INDEX_NONE)
	{
		return PriorityOverride;
	}
	return Context ? Context->Priority : 0;
}

FPGXInputContextResult UPGXInputSubsystem::ResolveEnhancedInputApplyTargets(FGameplayTag ContextTag, ULocalPlayer* LocalPlayer, UPGXInputContext*& OutContext, UInputMappingContext*& OutMappingContext, UEnhancedInputLocalPlayerSubsystem*& OutEnhancedInputSubsystem) const
{
	OutContext = nullptr;
	OutMappingContext = nullptr;
	OutEnhancedInputSubsystem = nullptr;

	if (!ContextTag.IsValid())
	{
		return FPGXInputContextResult::Failure(EPGXInputContextResultCode::InvalidTag, ContextTag, TEXT("Invalid input context tag."));
	}

	OutContext = FindContextAsset(ContextTag);
	if (!OutContext)
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXInput: context not found for LocalPlayer apply tag %s"), *ContextTag.ToString());
		return FPGXInputContextResult::Failure(EPGXInputContextResultCode::ContextNotFound, ContextTag, TEXT("Input context not found."));
	}

	OutMappingContext = OutContext->MappingContext.Get();
	if (!OutMappingContext && !OutContext->MappingContext.IsNull())
	{
		OutMappingContext = OutContext->MappingContext.LoadSynchronous();
	}
	if (!OutMappingContext)
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXInput: context %s has no Enhanced Input MappingContext"), *ContextTag.ToString());
		return FPGXInputContextResult::Failure(EPGXInputContextResultCode::MappingContextMissing, ContextTag, TEXT("Enhanced Input mapping context is missing."));
	}

	if (!LocalPlayer)
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXInput: LocalPlayer missing while applying context %s"), *ContextTag.ToString());
		return FPGXInputContextResult::Failure(EPGXInputContextResultCode::LocalPlayerNotReady, ContextTag, TEXT("LocalPlayer is required for Enhanced Input application."));
	}

	OutEnhancedInputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!OutEnhancedInputSubsystem)
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXInput: Enhanced Input LocalPlayer subsystem unavailable while applying %s"), *ContextTag.ToString());
		return FPGXInputContextResult::Failure(EPGXInputContextResultCode::LocalPlayerNotReady, ContextTag, TEXT("Enhanced Input LocalPlayer subsystem is unavailable."));
	}

	return FPGXInputContextResult::Success(ContextTag, TEXT("Enhanced Input apply targets resolved."));
}

void UPGXInputSubsystem::RemoveAppliedContextFromLocalPlayer(ULocalPlayer* LocalPlayer, FGameplayTag ContextTag)
{
	if (!LocalPlayer || !ContextTag.IsValid())
	{
		return;
	}

	UPGXInputContext* Context = FindContextAsset(ContextTag);
	if (!Context)
	{
		return;
	}

	UInputMappingContext* MappingContext = Context->MappingContext.Get();
	if (!MappingContext && !Context->MappingContext.IsNull())
	{
		MappingContext = Context->MappingContext.LoadSynchronous();
	}
	if (!MappingContext)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		EnhancedInputSubsystem->RemoveMappingContext(MappingContext);
	}
	if (TSet<FGameplayTag>* AppliedContexts = AppliedContextsByLocalPlayer.Find(TObjectKey<ULocalPlayer>(LocalPlayer)))
	{
		AppliedContexts->Remove(ContextTag);
	}
}

void UPGXInputSubsystem::RemoveAppliedContextsPrunedByExclusive(int32 ResolvedPriority)
{
	TArray<TPair<ULocalPlayer*, FGameplayTag>> RemovalPairs;
	for (const FPGXActiveInputContextEntry& Entry : ActiveContexts)
	{
		if (Entry.Priority <= ResolvedPriority)
		{
			for (const TPair<TObjectKey<ULocalPlayer>, TSet<FGameplayTag>>& Pair : AppliedContextsByLocalPlayer)
			{
				if (Pair.Value.Contains(Entry.ContextTag))
				{
					if (ULocalPlayer* LocalPlayer = Pair.Key.ResolveObjectPtr())
					{
						RemovalPairs.Emplace(LocalPlayer, Entry.ContextTag);
					}
				}
			}
		}
	}

	for (const TPair<ULocalPlayer*, FGameplayTag>& RemovalPair : RemovalPairs)
	{
		RemoveAppliedContextFromLocalPlayer(RemovalPair.Key, RemovalPair.Value);
	}
}
