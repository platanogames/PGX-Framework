// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Observability/PGXValidationResult.h"
#include "PGXTradeTypes.generated.h"

UENUM(BlueprintType)
enum class EPGXTradeResultCode : uint8
{
	Success = 0 UMETA(DisplayName = "Success"),
	Failed UMETA(DisplayName = "Failed"),
	NotInitialized UMETA(DisplayName = "Not Initialized"),
	InvalidInput UMETA(DisplayName = "Invalid Input"),
	ActorNotFound UMETA(DisplayName = "Actor Not Found"),
	DuplicateActor UMETA(DisplayName = "Duplicate Actor"),
	OfferNotFound UMETA(DisplayName = "Offer Not Found"),
	InvalidState UMETA(DisplayName = "Invalid State"),
	Expired UMETA(DisplayName = "Expired"),
	ReputationReasonMissing UMETA(DisplayName = "Reputation Reason Missing"),
	InformationInvalid UMETA(DisplayName = "Information Invalid")
};

UENUM(BlueprintType)
enum class EPGXTradeActorType : uint8
{
	Individual = 0 UMETA(DisplayName = "Individual"),
	Itinerant UMETA(DisplayName = "Itinerant"),
	Refugee UMETA(DisplayName = "Refugee"),
	Settlement UMETA(DisplayName = "Settlement"),
	Caravan UMETA(DisplayName = "Caravan"),
	Faction UMETA(DisplayName = "Faction"),
	ProjectDefined UMETA(DisplayName = "Project Defined")
};

UENUM(BlueprintType)
enum class EPGXTradeGoodType : uint8
{
	Item = 0 UMETA(DisplayName = "Item"),
	Recipe UMETA(DisplayName = "Recipe"),
	Information UMETA(DisplayName = "Information"),
	Service UMETA(DisplayName = "Service"),
	Access UMETA(DisplayName = "Access"),
	Reputation UMETA(DisplayName = "Reputation"),
	Recruitment UMETA(DisplayName = "Recruitment"),
	Promise UMETA(DisplayName = "Promise"),
	ProjectDefined UMETA(DisplayName = "Project Defined")
};

UENUM(BlueprintType)
enum class EPGXTradeOfferState : uint8
{
	Draft = 0 UMETA(DisplayName = "Draft"),
	Pending UMETA(DisplayName = "Pending"),
	Accepted UMETA(DisplayName = "Accepted"),
	Rejected UMETA(DisplayName = "Rejected"),
	Expired UMETA(DisplayName = "Expired"),
	Cancelled UMETA(DisplayName = "Cancelled"),
	Completed UMETA(DisplayName = "Completed"),
	Failed UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EPGXInformationReliability : uint8
{
	Verified = 0 UMETA(DisplayName = "Verified"),
	Trusted UMETA(DisplayName = "Trusted"),
	Uncertain UMETA(DisplayName = "Uncertain"),
	Rumor UMETA(DisplayName = "Rumor"),
	Falsified UMETA(DisplayName = "False"),
	Trap UMETA(DisplayName = "Trap"),
	Expired UMETA(DisplayName = "Expired")
};

UENUM(BlueprintType)
enum class EPGXTradePromiseState : uint8
{
	Pending = 0 UMETA(DisplayName = "Pending"),
	Honored UMETA(DisplayName = "Honored"),
	Broken UMETA(DisplayName = "Broken"),
	Waived UMETA(DisplayName = "Waived"),
	Expired UMETA(DisplayName = "Expired"),
	Invalidated UMETA(DisplayName = "Invalidated")
};

USTRUCT(BlueprintType)
struct PGXTRADERUNTIME_API FPGXTradeActorId
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade")
	FGuid Id;

	bool IsValid() const { return Id.IsValid(); }
	static FPGXTradeActorId NewId();
	bool operator==(const FPGXTradeActorId& Other) const { return Id == Other.Id; }
};

USTRUCT(BlueprintType)
struct PGXTRADERUNTIME_API FPGXTradeOfferId
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade")
	FGuid Id;

	bool IsValid() const { return Id.IsValid(); }
	static FPGXTradeOfferId NewId();
	bool operator==(const FPGXTradeOfferId& Other) const { return Id == Other.Id; }
};

USTRUCT(BlueprintType)
struct PGXTRADERUNTIME_API FPGXTradeTransactionId
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade")
	FGuid Id;

	bool IsValid() const { return Id.IsValid(); }
	static FPGXTradeTransactionId NewId();
};

USTRUCT(BlueprintType)
struct PGXTRADERUNTIME_API FPGXTradeGood
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade", meta = (Categories = "PGX.Trade.Good"))
	FGameplayTag GoodTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade")
	EPGXTradeGoodType Type = EPGXTradeGoodType::Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade", meta = (ClampMin = "1"))
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade", meta = (ClampMin = "0.0"))
	float UnitValue = 1.0f;

	bool IsValid() const { return GoodTag.IsValid() && Quantity > 0 && UnitValue >= 0.0f; }
	float GetTotalValue() const { return static_cast<float>(Quantity) * UnitValue; }
};

USTRUCT(BlueprintType)
struct PGXTRADERUNTIME_API FPGXTradeActorRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade")
	FPGXTradeActorId ActorId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade", meta = (Categories = "PGX.Trade.Actor"))
	FGameplayTag ActorTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade")
	EPGXTradeActorType ActorType = EPGXTradeActorType::Individual;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade")
	bool bAvailable = true;

	bool IsValid() const { return ActorId.IsValid() && ActorTag.IsValid(); }
};

USTRUCT(BlueprintType)
struct PGXTRADERUNTIME_API FPGXTradeOfferRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade")
	FPGXTradeActorId SellerActorId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade")
	FPGXTradeActorId BuyerActorId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade")
	TArray<FPGXTradeGood> OfferedGoods;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade")
	TArray<FPGXTradeGood> RequestedGoods;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade", meta = (Categories = "PGX.Trade.Source"))
	FGameplayTag SourceTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade", meta = (ClampMin = "0.0"))
	float ExpirationSeconds = 300.0f;

	bool IsValid() const;
};

USTRUCT(BlueprintType)
struct PGXTRADERUNTIME_API FPGXTradeValueBreakdown
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Trade")
	float OfferedValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Trade")
	float RequestedValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Trade")
	float ValueDelta = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Trade")
	bool bIsFair = false;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Trade")
	FString Explanation;
};

USTRUCT(BlueprintType)
struct PGXTRADERUNTIME_API FPGXTradeOffer
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Trade")
	FPGXTradeOfferId OfferId;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Trade")
	FPGXTradeOfferRequest Request;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Trade")
	EPGXTradeOfferState State = EPGXTradeOfferState::Draft;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Trade")
	FPGXTradeValueBreakdown ValueBreakdown;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Trade")
	double CreatedTimeSeconds = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Trade")
	double ExpirationTimeSeconds = 0.0;
};

USTRUCT(BlueprintType)
struct PGXTRADERUNTIME_API FPGXTradeTransactionRecord
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Trade")
	FPGXTradeTransactionId TransactionId;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Trade")
	FPGXTradeOfferId OfferId;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Trade")
	EPGXTradeResultCode ResultCode = EPGXTradeResultCode::Failed;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Trade")
	FGameplayTag ReasonTag;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Trade")
	double ResolvedTimeSeconds = 0.0;
};

USTRUCT(BlueprintType)
struct PGXTRADERUNTIME_API FPGXTradeReputationState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade")
	FPGXTradeActorId SourceActorId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade")
	FPGXTradeActorId TargetActorId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade")
	float Reputation = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade")
	float Trust = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade")
	FGameplayTag LastReasonTag;
};

USTRUCT(BlueprintType)
struct PGXTRADERUNTIME_API FPGXTradeInformationState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade", meta = (Categories = "PGX.Trade.Information"))
	FGameplayTag InformationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade")
	EPGXInformationReliability Reliability = EPGXInformationReliability::Uncertain;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade")
	float Freshness = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade")
	double AcquiredTimeSeconds = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade", meta = (Categories = "PGX.Trade.Source"))
	FGameplayTag SourceTag;
};

USTRUCT(BlueprintType)
struct PGXTRADERUNTIME_API FPGXTradePromise
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade")
	FGuid PromiseId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade")
	FPGXTradeActorId OwingActorId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade")
	FPGXTradeActorId OwedActorId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade", meta = (Categories = "PGX.Trade.Promise"))
	FGameplayTag PromiseTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trade")
	EPGXTradePromiseState State = EPGXTradePromiseState::Pending;
};

USTRUCT(BlueprintType)
struct PGXTRADERUNTIME_API FPGXTradeResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Trade")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Trade")
	EPGXTradeResultCode Code = EPGXTradeResultCode::Failed;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Trade")
	FPGXTradeOfferId OfferId;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Trade")
	FPGXTradeTransactionId TransactionId;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Trade")
	FString Message;

	static FPGXTradeResult Success(FString InMessage = FString(), FPGXTradeOfferId InOfferId = FPGXTradeOfferId(), FPGXTradeTransactionId InTransactionId = FPGXTradeTransactionId());
	static FPGXTradeResult Failure(EPGXTradeResultCode InCode, FString InMessage = FString(), FPGXTradeOfferId InOfferId = FPGXTradeOfferId());

	/**
	 * EN: Convert to the canonical FPGXValidationResult for cross-cutting concerns
	 *     (logs, telemetry, message-bus, JSON export). Maps bSuccess -> bValid and
	 *     emits a single error entry carrying the typed Code + Message. Domain
	 *     fields (OfferId, TransactionId) are NOT carried — they stay in this
	 *     local struct; consumers that need them keep using FPGXTradeResult.
	 *
	 *     Bridge to FPGXValidationResult.
	 *
	 * ES: Convertir al FPGXValidationResult canonico para concerns cross-cutting
	 *     (logs, telemetry, message-bus, JSON export). Mapea bSuccess -> bValid y
	 *     emite una unica entrada de error llevando el Code tipado + Message. Los
	 *     campos de dominio (OfferId, TransactionId) NO se llevan — se quedan en
	 *     esta struct local; los consumers que los necesitan siguen usando FPGXTradeResult.
	 */
	FPGXValidationResult ToValidationResult() const
	{
		if (bSuccess)
		{
			return FPGXValidationResult::MakeValid();
		}
		// EN: Convert EPGXTradeResultCode to FName via static string.
		// ES: Convertir EPGXTradeResultCode a FName via string estatico.
		const FName CodeName(*UEnum::GetValueAsString(Code));
		FPGXValidationResult R;
		R.AddError(CodeName, FString(), FText::FromString(Message));
		return R;
	}
};

uint32 GetTypeHash(const FPGXTradeActorId& ActorId);
uint32 GetTypeHash(const FPGXTradeOfferId& OfferId);
