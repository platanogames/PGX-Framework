// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXTradeSubsystem.h"

#include "PGXTradeConfig.h"
#include "PGXTradeRuntime.h"
#include "PGXTradeSettings.h"
#include "Utils/PGXConfigResolution.h"
#include "Engine/World.h"
#include "Logging/PGXLogMacros.h"

void UPGXTradeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	bInitialized = true;

	// EN: Settings-first resolution via PGX::ResolveSingleConfig<T>() — handles
	//     LoadSynchronous + AssetRegistry fallback (deprecated) + logs.
	//     Replaces the broken pattern: Settings->ActiveConfig.Get() (no load).
	// ES: Resolucion Settings-first via PGX::ResolveSingleConfig<T>() — maneja
	//     LoadSynchronous + fallback AssetRegistry (deprecado) + logs.
	//     Reemplaza el patron roto: Settings->ActiveConfig.Get() (sin load).
	// Fix critical: config always null.
	const UPGXTradeSettings* Settings = GetDefault<UPGXTradeSettings>();
	ResolvedConfig = Settings
		? PGX::ResolveSingleConfig<UPGXTradeConfig>(Settings->ActiveConfig, TEXT("Trade"))
		: nullptr;
	PGX_LOG_INFO(LogPGXTrade, TEXT("UPGXTradeSubsystem: Initialized. Actors=%d Offers=%d Config=%s"),
		Actors.Num(), Offers.Num(), ResolvedConfig ? *ResolvedConfig->GetName() : TEXT("(none)"));
}

void UPGXTradeSubsystem::Deinitialize()
{
	ClearTradeStateForTesting();
	ResolvedConfig = nullptr;
	bInitialized = false;
	PGX_LOG_INFO(LogPGXTrade, TEXT("UPGXTradeSubsystem: Deinitialized."));
	Super::Deinitialize();
}

FPGXTradeResult UPGXTradeSubsystem::RegisterActor(FPGXTradeActorRecord Actor)
{
	if (!bInitialized)
	{
		return FPGXTradeResult::Failure(EPGXTradeResultCode::NotInitialized, TEXT("Trade subsystem is not initialized."));
	}
	if (!Actor.ActorId.IsValid())
	{
		Actor.ActorId = FPGXTradeActorId::NewId();
	}
	if (!Actor.IsValid())
	{
		return FPGXTradeResult::Failure(EPGXTradeResultCode::InvalidInput, TEXT("Trade actor requires valid id and actor tag."));
	}
	if (HasActor(Actor.ActorId))
	{
		return FPGXTradeResult::Failure(EPGXTradeResultCode::DuplicateActor, TEXT("Trade actor id already registered."));
	}

	Actors.Add(Actor);
	return FPGXTradeResult::Success(TEXT("Trade actor registered."));
}

bool UPGXTradeSubsystem::HasActor(FPGXTradeActorId ActorId) const
{
	return Actors.ContainsByPredicate([ActorId](const FPGXTradeActorRecord& Actor)
	{
		return Actor.ActorId == ActorId;
	});
}

bool UPGXTradeSubsystem::FindActor(FPGXTradeActorId ActorId, FPGXTradeActorRecord& OutActor) const
{
	if (const FPGXTradeActorRecord* Found = Actors.FindByPredicate([ActorId](const FPGXTradeActorRecord& Actor)
	{
		return Actor.ActorId == ActorId;
	}))
	{
		OutActor = *Found;
		return true;
	}
	return false;
}

FPGXTradeResult UPGXTradeSubsystem::EvaluateOffer(const FPGXTradeOfferRequest& Request, FPGXTradeValueBreakdown& OutBreakdown) const
{
	OutBreakdown = FPGXTradeValueBreakdown();
	if (!bInitialized)
	{
		return FPGXTradeResult::Failure(EPGXTradeResultCode::NotInitialized, TEXT("Trade subsystem is not initialized."));
	}
	if (!Request.IsValid())
	{
		return FPGXTradeResult::Failure(EPGXTradeResultCode::InvalidInput, TEXT("Offer request is structurally invalid."));
	}
	if (!HasActor(Request.SellerActorId) || !HasActor(Request.BuyerActorId))
	{
		return FPGXTradeResult::Failure(EPGXTradeResultCode::ActorNotFound, TEXT("Offer request references an unknown trade actor."));
	}

	for (const FPGXTradeGood& Good : Request.OfferedGoods)
	{
		OutBreakdown.OfferedValue += Good.GetTotalValue();
	}
	for (const FPGXTradeGood& Good : Request.RequestedGoods)
	{
		OutBreakdown.RequestedValue += Good.GetTotalValue();
	}
	OutBreakdown.ValueDelta = OutBreakdown.OfferedValue - OutBreakdown.RequestedValue;
	const float Denominator = FMath::Max(1.0f, OutBreakdown.RequestedValue);
	OutBreakdown.bIsFair = FMath::Abs(OutBreakdown.ValueDelta) / Denominator <= GetFairTradeTolerance();
	OutBreakdown.Explanation = FString::Printf(TEXT("Offered=%.2f Requested=%.2f Delta=%.2f Tolerance=%.2f"),
		OutBreakdown.OfferedValue,
		OutBreakdown.RequestedValue,
		OutBreakdown.ValueDelta,
		GetFairTradeTolerance());
	return FPGXTradeResult::Success(TEXT("Offer evaluated."));
}

FPGXTradeResult UPGXTradeSubsystem::CreateOffer(const FPGXTradeOfferRequest& Request, FPGXTradeOffer& OutOffer)
{
	FPGXTradeValueBreakdown Breakdown;
	FPGXTradeResult EvalResult = EvaluateOffer(Request, Breakdown);
	if (!EvalResult.bSuccess)
	{
		return EvalResult;
	}

	FPGXTradeOffer Offer;
	Offer.OfferId = FPGXTradeOfferId::NewId();
	Offer.Request = Request;
	Offer.State = EPGXTradeOfferState::Pending;
	Offer.ValueBreakdown = Breakdown;
	Offer.CreatedTimeSeconds = GetNowSeconds();
	const float Expiration = Request.ExpirationSeconds > 0.0f ? Request.ExpirationSeconds : GetDefaultOfferExpirationSeconds();
	Offer.ExpirationTimeSeconds = Expiration > 0.0f ? Offer.CreatedTimeSeconds + Expiration : 0.0;

	Offers.Add(Offer);
	OutOffer = Offer;
	return FPGXTradeResult::Success(TEXT("Offer created."), Offer.OfferId);
}

FPGXTradeResult UPGXTradeSubsystem::AcceptOffer(FPGXTradeOfferId OfferId, FGameplayTag /*ReasonTag*/)
{
	FPGXTradeOffer* Offer = FindOfferMutable(OfferId);
	if (!Offer)
	{
		return FPGXTradeResult::Failure(EPGXTradeResultCode::OfferNotFound, TEXT("Offer not found."), OfferId);
	}
	if (Offer->State != EPGXTradeOfferState::Pending)
	{
		return FPGXTradeResult::Failure(EPGXTradeResultCode::InvalidState, TEXT("Only pending offers can be accepted."), OfferId);
	}
	if (Offer->ExpirationTimeSeconds > 0.0 && GetNowSeconds() > Offer->ExpirationTimeSeconds)
	{
		return ExpireOffer(OfferId, Offer->Request.SourceTag);
	}
	Offer->State = EPGXTradeOfferState::Accepted;
	return FPGXTradeResult::Success(TEXT("Offer accepted locally; external asset transfer is not performed."), OfferId);
}

FPGXTradeResult UPGXTradeSubsystem::RejectOffer(FPGXTradeOfferId OfferId, FGameplayTag ReasonTag)
{
	FPGXTradeOffer* Offer = FindOfferMutable(OfferId);
	if (!Offer)
	{
		return FPGXTradeResult::Failure(EPGXTradeResultCode::OfferNotFound, TEXT("Offer not found."), OfferId);
	}
	if (Offer->State != EPGXTradeOfferState::Pending && Offer->State != EPGXTradeOfferState::Accepted)
	{
		return FPGXTradeResult::Failure(EPGXTradeResultCode::InvalidState, TEXT("Offer cannot be rejected from its current state."), OfferId);
	}
	return ResolveOfferToTransaction(*Offer, EPGXTradeResultCode::Failed, ReasonTag, EPGXTradeOfferState::Rejected);
}

FPGXTradeResult UPGXTradeSubsystem::ExpireOffer(FPGXTradeOfferId OfferId, FGameplayTag ReasonTag)
{
	FPGXTradeOffer* Offer = FindOfferMutable(OfferId);
	if (!Offer)
	{
		return FPGXTradeResult::Failure(EPGXTradeResultCode::OfferNotFound, TEXT("Offer not found."), OfferId);
	}
	if (Offer->State != EPGXTradeOfferState::Pending && Offer->State != EPGXTradeOfferState::Accepted)
	{
		return FPGXTradeResult::Failure(EPGXTradeResultCode::InvalidState, TEXT("Offer cannot expire from its current state."), OfferId);
	}
	return ResolveOfferToTransaction(*Offer, EPGXTradeResultCode::Expired, ReasonTag, EPGXTradeOfferState::Expired);
}

FPGXTradeResult UPGXTradeSubsystem::CompleteOffer(FPGXTradeOfferId OfferId, FGameplayTag ReasonTag)
{
	FPGXTradeOffer* Offer = FindOfferMutable(OfferId);
	if (!Offer)
	{
		return FPGXTradeResult::Failure(EPGXTradeResultCode::OfferNotFound, TEXT("Offer not found."), OfferId);
	}
	if (Offer->State != EPGXTradeOfferState::Accepted)
	{
		return FPGXTradeResult::Failure(EPGXTradeResultCode::InvalidState, TEXT("Only accepted offers can be completed."), OfferId);
	}
	return ResolveOfferToTransaction(*Offer, EPGXTradeResultCode::Success, ReasonTag, EPGXTradeOfferState::Completed);
}

bool UPGXTradeSubsystem::FindOffer(FPGXTradeOfferId OfferId, FPGXTradeOffer& OutOffer) const
{
	if (const FPGXTradeOffer* Found = FindOfferInternal(OfferId))
	{
		OutOffer = *Found;
		return true;
	}
	return false;
}

FPGXTradeResult UPGXTradeSubsystem::ApplyReputationDelta(FPGXTradeActorId SourceActorId, FPGXTradeActorId TargetActorId, float ReputationDelta, float TrustDelta, FGameplayTag ReasonTag)
{
	if (!ReasonTag.IsValid())
	{
		return FPGXTradeResult::Failure(EPGXTradeResultCode::ReputationReasonMissing, TEXT("Reputation delta requires a reason tag."));
	}
	if (!HasActor(SourceActorId) || !HasActor(TargetActorId))
	{
		return FPGXTradeResult::Failure(EPGXTradeResultCode::ActorNotFound, TEXT("Reputation delta references unknown actor."));
	}

	FPGXTradeReputationState* State = FindReputationMutable(SourceActorId, TargetActorId);
	if (!State)
	{
		FPGXTradeReputationState NewState;
		NewState.SourceActorId = SourceActorId;
		NewState.TargetActorId = TargetActorId;
		ReputationStates.Add(NewState);
		State = &ReputationStates.Last();
	}
	const UPGXTradeConfig* Config = ResolvedConfig.Get();
	const float MinRep = Config ? Config->MinReputation : -100.0f;
	const float MaxRep = Config ? Config->MaxReputation : 100.0f;
	State->Reputation = FMath::Clamp(State->Reputation + ReputationDelta, MinRep, MaxRep);
	State->Trust = FMath::Clamp(State->Trust + TrustDelta, MinRep, MaxRep);
	State->LastReasonTag = ReasonTag;
	return FPGXTradeResult::Success(TEXT("Reputation updated."));
}

bool UPGXTradeSubsystem::GetReputation(FPGXTradeActorId SourceActorId, FPGXTradeActorId TargetActorId, FPGXTradeReputationState& OutState) const
{
	if (const FPGXTradeReputationState* Found = FindReputationInternal(SourceActorId, TargetActorId))
	{
		OutState = *Found;
		return true;
	}
	return false;
}

FPGXTradeResult UPGXTradeSubsystem::AcquireInformation(FGameplayTag InformationTag, EPGXInformationReliability Reliability, float Freshness, FGameplayTag SourceTag)
{
	if (!InformationTag.IsValid())
	{
		return FPGXTradeResult::Failure(EPGXTradeResultCode::InformationInvalid, TEXT("Information acquisition requires an information tag."));
	}

	FPGXTradeInformationState* Existing = InformationStates.FindByPredicate([InformationTag](const FPGXTradeInformationState& State)
	{
		return State.InformationTag == InformationTag;
	});
	if (!Existing)
	{
		FPGXTradeInformationState NewState;
		NewState.InformationTag = InformationTag;
		InformationStates.Add(NewState);
		Existing = &InformationStates.Last();
	}
	Existing->Reliability = Reliability;
	Existing->Freshness = FMath::Clamp(Freshness, 0.0f, 1.0f);
	Existing->SourceTag = SourceTag;
	Existing->AcquiredTimeSeconds = GetNowSeconds();
	return FPGXTradeResult::Success(TEXT("Information acquired."));
}

bool UPGXTradeSubsystem::GetInformationState(FGameplayTag InformationTag, FPGXTradeInformationState& OutState) const
{
	if (const FPGXTradeInformationState* Found = InformationStates.FindByPredicate([InformationTag](const FPGXTradeInformationState& State)
	{
		return State.InformationTag == InformationTag;
	}))
	{
		OutState = *Found;
		return true;
	}
	return false;
}

FPGXTradeResult UPGXTradeSubsystem::RegisterPromise(const FPGXTradePromise& Promise)
{
	if (!Promise.PromiseId.IsValid() || !Promise.OwingActorId.IsValid() || !Promise.OwedActorId.IsValid() || !Promise.PromiseTag.IsValid())
	{
		return FPGXTradeResult::Failure(EPGXTradeResultCode::InvalidInput, TEXT("Promise requires valid id, actors and tag."));
	}
	Promises.Add(Promise);
	return FPGXTradeResult::Success(TEXT("Promise registered."));
}

void UPGXTradeSubsystem::ClearTradeStateForTesting()
{
	Actors.Reset();
	Offers.Reset();
	Transactions.Reset();
	ReputationStates.Reset();
	InformationStates.Reset();
	Promises.Reset();
}

FPGXTradeOffer* UPGXTradeSubsystem::FindOfferMutable(FPGXTradeOfferId OfferId)
{
	return Offers.FindByPredicate([OfferId](const FPGXTradeOffer& Offer)
	{
		return Offer.OfferId == OfferId;
	});
}

const FPGXTradeOffer* UPGXTradeSubsystem::FindOfferInternal(FPGXTradeOfferId OfferId) const
{
	return Offers.FindByPredicate([OfferId](const FPGXTradeOffer& Offer)
	{
		return Offer.OfferId == OfferId;
	});
}

FPGXTradeReputationState* UPGXTradeSubsystem::FindReputationMutable(FPGXTradeActorId SourceActorId, FPGXTradeActorId TargetActorId)
{
	return ReputationStates.FindByPredicate([SourceActorId, TargetActorId](const FPGXTradeReputationState& State)
	{
		return State.SourceActorId == SourceActorId && State.TargetActorId == TargetActorId;
	});
}

const FPGXTradeReputationState* UPGXTradeSubsystem::FindReputationInternal(FPGXTradeActorId SourceActorId, FPGXTradeActorId TargetActorId) const
{
	return ReputationStates.FindByPredicate([SourceActorId, TargetActorId](const FPGXTradeReputationState& State)
	{
		return State.SourceActorId == SourceActorId && State.TargetActorId == TargetActorId;
	});
}

float UPGXTradeSubsystem::GetFairTradeTolerance() const
{
	return ResolvedConfig ? ResolvedConfig->FairTradeTolerance : 0.25f;
}

float UPGXTradeSubsystem::GetDefaultOfferExpirationSeconds() const
{
	return ResolvedConfig ? ResolvedConfig->DefaultOfferExpirationSeconds : 300.0f;
}

double UPGXTradeSubsystem::GetNowSeconds() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetTimeSeconds() : FPlatformTime::Seconds();
}

FPGXTradeResult UPGXTradeSubsystem::ResolveOfferToTransaction(FPGXTradeOffer& Offer, EPGXTradeResultCode ResultCode, FGameplayTag ReasonTag, EPGXTradeOfferState FinalState)
{
	Offer.State = FinalState;

	FPGXTradeTransactionRecord Record;
	Record.TransactionId = FPGXTradeTransactionId::NewId();
	Record.OfferId = Offer.OfferId;
	Record.ResultCode = ResultCode;
	Record.ReasonTag = ReasonTag;
	Record.ResolvedTimeSeconds = GetNowSeconds();
	Transactions.Add(Record);

	return ResultCode == EPGXTradeResultCode::Success
		? FPGXTradeResult::Success(TEXT("Offer resolved locally."), Offer.OfferId, Record.TransactionId)
		: FPGXTradeResult::Failure(ResultCode, TEXT("Offer resolved to non-success terminal state."), Offer.OfferId);
}
