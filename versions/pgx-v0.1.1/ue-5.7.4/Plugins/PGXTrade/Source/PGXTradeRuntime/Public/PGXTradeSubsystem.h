// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PGXTradeTypes.h"
#include "PGXTradeSubsystem.generated.h"

class UPGXTradeConfig;

/**
 * EN: Greenfield PGXTrade baseline subsystem. Owns local generic actors, offers,
 *     transactions, reputation, information freshness and promises. No adjacent L2 handoff.
 * ES: Subsistema baseline PGXTrade. Posee estado local generico sin integracion L2 adyacente.
 */
UCLASS()
class PGXTRADERUNTIME_API UPGXTradeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	bool IsTradeInitialized() const { return bInitialized; }
	const UPGXTradeConfig* GetResolvedConfig() const { return ResolvedConfig.Get(); }

	FPGXTradeResult RegisterActor(FPGXTradeActorRecord Actor);
	bool HasActor(FPGXTradeActorId ActorId) const;
	bool FindActor(FPGXTradeActorId ActorId, FPGXTradeActorRecord& OutActor) const;
	int32 GetActorCount() const { return Actors.Num(); }
	TArray<FPGXTradeActorRecord> GetActorsSnapshot() const { return Actors; }

	FPGXTradeResult EvaluateOffer(const FPGXTradeOfferRequest& Request, FPGXTradeValueBreakdown& OutBreakdown) const;
	FPGXTradeResult CreateOffer(const FPGXTradeOfferRequest& Request, FPGXTradeOffer& OutOffer);
	FPGXTradeResult AcceptOffer(FPGXTradeOfferId OfferId, FGameplayTag ReasonTag);
	FPGXTradeResult RejectOffer(FPGXTradeOfferId OfferId, FGameplayTag ReasonTag);
	FPGXTradeResult ExpireOffer(FPGXTradeOfferId OfferId, FGameplayTag ReasonTag);
	FPGXTradeResult CompleteOffer(FPGXTradeOfferId OfferId, FGameplayTag ReasonTag);

	bool FindOffer(FPGXTradeOfferId OfferId, FPGXTradeOffer& OutOffer) const;
	int32 GetOfferCount() const { return Offers.Num(); }
	TArray<FPGXTradeOffer> GetOffersSnapshot() const { return Offers; }
	int32 GetTransactionCount() const { return Transactions.Num(); }
	TArray<FPGXTradeTransactionRecord> GetTransactionsSnapshot() const { return Transactions; }

	FPGXTradeResult ApplyReputationDelta(FPGXTradeActorId SourceActorId, FPGXTradeActorId TargetActorId, float ReputationDelta, float TrustDelta, FGameplayTag ReasonTag);
	bool GetReputation(FPGXTradeActorId SourceActorId, FPGXTradeActorId TargetActorId, FPGXTradeReputationState& OutState) const;
	int32 GetReputationEntryCount() const { return ReputationStates.Num(); }

	FPGXTradeResult AcquireInformation(FGameplayTag InformationTag, EPGXInformationReliability Reliability, float Freshness, FGameplayTag SourceTag);
	bool GetInformationState(FGameplayTag InformationTag, FPGXTradeInformationState& OutState) const;
	int32 GetInformationCount() const { return InformationStates.Num(); }

	FPGXTradeResult RegisterPromise(const FPGXTradePromise& Promise);
	int32 GetPromiseCount() const { return Promises.Num(); }

	void ClearTradeStateForTesting();

private:
	FPGXTradeOffer* FindOfferMutable(FPGXTradeOfferId OfferId);
	const FPGXTradeOffer* FindOfferInternal(FPGXTradeOfferId OfferId) const;
	FPGXTradeReputationState* FindReputationMutable(FPGXTradeActorId SourceActorId, FPGXTradeActorId TargetActorId);
	const FPGXTradeReputationState* FindReputationInternal(FPGXTradeActorId SourceActorId, FPGXTradeActorId TargetActorId) const;
	float GetFairTradeTolerance() const;
	float GetDefaultOfferExpirationSeconds() const;
	double GetNowSeconds() const;
	FPGXTradeResult ResolveOfferToTransaction(FPGXTradeOffer& Offer, EPGXTradeResultCode ResultCode, FGameplayTag ReasonTag, EPGXTradeOfferState FinalState);

	UPROPERTY(Transient)
	bool bInitialized = false;

	UPROPERTY(Transient)
	TObjectPtr<UPGXTradeConfig> ResolvedConfig = nullptr;

	UPROPERTY(Transient)
	TArray<FPGXTradeActorRecord> Actors;

	UPROPERTY(Transient)
	TArray<FPGXTradeOffer> Offers;

	UPROPERTY(Transient)
	TArray<FPGXTradeTransactionRecord> Transactions;

	UPROPERTY(Transient)
	TArray<FPGXTradeReputationState> ReputationStates;

	UPROPERTY(Transient)
	TArray<FPGXTradeInformationState> InformationStates;

	UPROPERTY(Transient)
	TArray<FPGXTradePromise> Promises;
};
