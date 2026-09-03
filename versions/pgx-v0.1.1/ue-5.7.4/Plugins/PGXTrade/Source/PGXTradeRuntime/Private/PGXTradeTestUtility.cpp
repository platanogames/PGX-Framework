// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXTradeTestUtility.h"

#include "PGXTradeConfig.h"
#include "PGXTradeRuntime.h"
#include "PGXTradeSettings.h"
#include "PGXTradeSubsystem.h"
#include "Tags/PGXTradeTags.h"
#include "Observability/PGXJsonValue.h"
#include "Observability/PGXSchemaDescriptor.h"
#include "Observability/PGXValidationResult.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Logging/PGXLogMacros.h"

namespace PGXTradeTestUtilityInternal
{
	struct FScopedTradeState
	{
		explicit FScopedTradeState(UPGXTradeSubsystem* InTrade)
			: Trade(InTrade)
		{
			Trade->ClearTradeStateForTesting();
		}

		~FScopedTradeState()
		{
			Trade->ClearTradeStateForTesting();
		}

		UPGXTradeSubsystem* Trade;
	};

	static FPGXTradeActorRecord MakeActor(FGameplayTag ActorTag, const TCHAR* Name)
	{
		FPGXTradeActorRecord Actor;
		Actor.ActorId = FPGXTradeActorId::NewId();
		Actor.ActorTag = ActorTag;
		Actor.ActorType = EPGXTradeActorType::ProjectDefined;
		Actor.DisplayName = FText::FromString(Name);
		Actor.bAvailable = true;
		return Actor;
	}

	static FPGXTradeGood MakeGood(FGameplayTag GoodTag, float UnitValue, int32 Quantity = 1, EPGXTradeGoodType Type = EPGXTradeGoodType::Item)
	{
		FPGXTradeGood Good;
		Good.GoodTag = GoodTag;
		Good.UnitValue = UnitValue;
		Good.Quantity = Quantity;
		Good.Type = Type;
		return Good;
	}

	static bool RegisterPair(UPGXTradeSubsystem* Trade, FPGXTradeActorRecord& Seller, FPGXTradeActorRecord& Buyer, TArray<FString>& OutIssues)
	{
		Seller = MakeActor(TAG_PGX_Trade_Actor_Project.GetTag(), TEXT("Seller"));
		Buyer = MakeActor(TAG_PGX_Trade_Actor_Individual.GetTag(), TEXT("Buyer"));
		const bool bSeller = Trade && Trade->RegisterActor(Seller).bSuccess;
		const bool bBuyer = Trade && Trade->RegisterActor(Buyer).bSuccess;
		UPGXTradeTestUtility::StaticClass();
		OutIssues.Add(FString::Printf(TEXT("[INFO] RegisterPair seller=%s buyer=%s"), bSeller ? TEXT("yes") : TEXT("no"), bBuyer ? TEXT("yes") : TEXT("no")));
		return bSeller && bBuyer;
	}

	static FPGXTradeOfferRequest MakeOfferRequest(const FPGXTradeActorRecord& Seller, const FPGXTradeActorRecord& Buyer)
	{
		FPGXTradeOfferRequest Request;
		Request.SellerActorId = Seller.ActorId;
		Request.BuyerActorId = Buyer.ActorId;
		Request.OfferedGoods.Add(MakeGood(TAG_PGX_Trade_Good_Item.GetTag(), 10.0f, 2));
		Request.RequestedGoods.Add(MakeGood(TAG_PGX_Trade_Good_Information.GetTag(), 20.0f, 1, EPGXTradeGoodType::Information));
		Request.SourceTag = TAG_PGX_Trade_Source_Automation.GetTag();
		Request.ExpirationSeconds = 30.0f;
		return Request;
	}
}

UPGXTradeSubsystem* UPGXTradeTestUtility::GetSubsystem(const UObject* WorldContextObject)
{
	if (!GEngine || !WorldContextObject)
	{
		return nullptr;
	}
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull))
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			return GameInstance->GetSubsystem<UPGXTradeSubsystem>();
		}
	}
	return nullptr;
}

void UPGXTradeTestUtility::RecordResult(TArray<FString>& OutIssues, const FString& TestName, bool bPassed, const FString& Details)
{
	const FString Line = FString::Printf(TEXT("[%s] %s%s%s"),
		bPassed ? TEXT("PASS") : TEXT("FAIL"),
		*TestName,
		Details.IsEmpty() ? TEXT("") : TEXT(": "),
		*Details);
	OutIssues.Add(Line);
	PGX_LOG_INFO(LogPGXTrade, TEXT("%s"), *Line);
}

bool UPGXTradeTestUtility::SubsystemInitializeTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	UPGXTradeSubsystem* Trade = GetSubsystem(WorldContextObject);
	const bool bSubsystem = Trade && Trade->IsTradeInitialized();
	RecordResult(OutIssues, TEXT("SubsystemInitialize.SubsystemExists"), bSubsystem);
	if (!Trade)
	{
		return false;
	}
	PGXTradeTestUtilityInternal::FScopedTradeState TestState(Trade);
	RecordResult(OutIssues, TEXT("SubsystemInitialize.ActorCountZero"), Trade->GetActorCount() == 0, FString::Printf(TEXT("ActorCount=%d"), Trade->GetActorCount()));
	RecordResult(OutIssues, TEXT("SubsystemInitialize.OfferCountZero"), Trade->GetOfferCount() == 0, FString::Printf(TEXT("OfferCount=%d"), Trade->GetOfferCount()));
	return bSubsystem && Trade->GetActorCount() == 0 && Trade->GetOfferCount() == 0;
}

bool UPGXTradeTestUtility::ConfigDefaultsTest(const UObject* /*WorldContextObject*/, TArray<FString>& OutIssues)
{
	const UPGXTradeSettings* Settings = GetDefault<UPGXTradeSettings>();
	RecordResult(OutIssues, TEXT("ConfigDefaults.SettingsReachable"), Settings != nullptr);

	UPGXTradeConfig* Config = NewObject<UPGXTradeConfig>(GetTransientPackage(), UPGXTradeConfig::StaticClass(), NAME_None, RF_Transient);
	const bool bConfigValid = Config && Config->IsPolicyValid();
	RecordResult(OutIssues, TEXT("ConfigDefaults.TransientConfigValid"), bConfigValid);
	if (Config)
	{
		Config->MinReputation = 50.0f;
		Config->MaxReputation = -50.0f;
		RecordResult(OutIssues, TEXT("ConfigDefaults.InvalidClampRejected"), !Config->IsPolicyValid());
		return Settings && bConfigValid && !Config->IsPolicyValid();
	}
	return false;
}

bool UPGXTradeTestUtility::ActorRegistrationTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	UPGXTradeSubsystem* Trade = GetSubsystem(WorldContextObject);
	if (!Trade)
	{
		RecordResult(OutIssues, TEXT("ActorRegistration.Subsystem"), false);
		return false;
	}
	PGXTradeTestUtilityInternal::FScopedTradeState TestState(Trade);

	FPGXTradeActorRecord Actor = PGXTradeTestUtilityInternal::MakeActor(TAG_PGX_Trade_Actor_Project.GetTag(), TEXT("Actor"));
	const FPGXTradeResult First = Trade->RegisterActor(Actor);
	const FPGXTradeResult Duplicate = Trade->RegisterActor(Actor);
	FPGXTradeActorRecord Snapshot;
	const bool bFound = Trade->FindActor(Actor.ActorId, Snapshot);
	RecordResult(OutIssues, TEXT("ActorRegistration.FirstSucceeds"), First.bSuccess);
	RecordResult(OutIssues, TEXT("ActorRegistration.DuplicateFails"), !Duplicate.bSuccess && Duplicate.Code == EPGXTradeResultCode::DuplicateActor);
	RecordResult(OutIssues, TEXT("ActorRegistration.FindSnapshot"), bFound && Snapshot.ActorTag == Actor.ActorTag);
	return First.bSuccess && !Duplicate.bSuccess && bFound;
}

bool UPGXTradeTestUtility::OfferLifecycleTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	UPGXTradeSubsystem* Trade = GetSubsystem(WorldContextObject);
	if (!Trade)
	{
		RecordResult(OutIssues, TEXT("OfferLifecycle.Subsystem"), false);
		return false;
	}
	PGXTradeTestUtilityInternal::FScopedTradeState TestState(Trade);

	FPGXTradeActorRecord Seller;
	FPGXTradeActorRecord Buyer;
	const bool bRegistered = PGXTradeTestUtilityInternal::RegisterPair(Trade, Seller, Buyer, OutIssues);
	FPGXTradeOfferRequest Request = PGXTradeTestUtilityInternal::MakeOfferRequest(Seller, Buyer);
	FPGXTradeValueBreakdown Breakdown;
	const FPGXTradeResult Eval = Trade->EvaluateOffer(Request, Breakdown);
	FPGXTradeOffer Offer;
	const FPGXTradeResult Created = Trade->CreateOffer(Request, Offer);
	const FPGXTradeResult Accepted = Trade->AcceptOffer(Offer.OfferId, TAG_PGX_Trade_Source_Automation.GetTag());
	const FPGXTradeResult Completed = Trade->CompleteOffer(Offer.OfferId, TAG_PGX_Trade_Source_Automation.GetTag());
	FPGXTradeOffer CompletedOffer;
	Trade->FindOffer(Offer.OfferId, CompletedOffer);

	RecordResult(OutIssues, TEXT("OfferLifecycle.ActorsRegistered"), bRegistered);
	RecordResult(OutIssues, TEXT("OfferLifecycle.EvaluateSucceeds"), Eval.bSuccess && FMath::IsNearlyEqual(Breakdown.OfferedValue, 20.0f));
	RecordResult(OutIssues, TEXT("OfferLifecycle.CreateAcceptComplete"), Created.bSuccess && Accepted.bSuccess && Completed.bSuccess);
	RecordResult(OutIssues, TEXT("OfferLifecycle.CompletedStateAndHistory"), CompletedOffer.State == EPGXTradeOfferState::Completed && Trade->GetTransactionCount() == 1);
	return bRegistered && Eval.bSuccess && Created.bSuccess && Accepted.bSuccess && Completed.bSuccess && CompletedOffer.State == EPGXTradeOfferState::Completed && Trade->GetTransactionCount() == 1;
}

bool UPGXTradeTestUtility::ReputationReasonGuardTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	UPGXTradeSubsystem* Trade = GetSubsystem(WorldContextObject);
	if (!Trade)
	{
		RecordResult(OutIssues, TEXT("ReputationReasonGuard.Subsystem"), false);
		return false;
	}
	PGXTradeTestUtilityInternal::FScopedTradeState TestState(Trade);

	FPGXTradeActorRecord Seller;
	FPGXTradeActorRecord Buyer;
	PGXTradeTestUtilityInternal::RegisterPair(Trade, Seller, Buyer, OutIssues);
	const FPGXTradeResult MissingReason = Trade->ApplyReputationDelta(Seller.ActorId, Buyer.ActorId, 10.0f, 5.0f, FGameplayTag());
	const FPGXTradeResult WithReason = Trade->ApplyReputationDelta(Seller.ActorId, Buyer.ActorId, 10.0f, 5.0f, TAG_PGX_Trade_Reputation_Reason_Test.GetTag());
	FPGXTradeReputationState State;
	const bool bState = Trade->GetReputation(Seller.ActorId, Buyer.ActorId, State);
	RecordResult(OutIssues, TEXT("ReputationReasonGuard.MissingReasonFails"), !MissingReason.bSuccess && MissingReason.Code == EPGXTradeResultCode::ReputationReasonMissing);
	RecordResult(OutIssues, TEXT("ReputationReasonGuard.ValidReasonUpdates"), WithReason.bSuccess && bState && State.LastReasonTag == TAG_PGX_Trade_Reputation_Reason_Test.GetTag());
	return !MissingReason.bSuccess && WithReason.bSuccess && bState;
}

bool UPGXTradeTestUtility::InformationFreshnessTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	UPGXTradeSubsystem* Trade = GetSubsystem(WorldContextObject);
	if (!Trade)
	{
		RecordResult(OutIssues, TEXT("InformationFreshness.Subsystem"), false);
		return false;
	}
	PGXTradeTestUtilityInternal::FScopedTradeState TestState(Trade);
	const FPGXTradeResult Invalid = Trade->AcquireInformation(FGameplayTag(), EPGXInformationReliability::Rumor, 1.0f, TAG_PGX_Trade_Source_Automation.GetTag());
	const FPGXTradeResult Valid = Trade->AcquireInformation(TAG_PGX_Trade_Information_Test.GetTag(), EPGXInformationReliability::Trusted, 1.5f, TAG_PGX_Trade_Source_Automation.GetTag());
	FPGXTradeInformationState State;
	const bool bState = Trade->GetInformationState(TAG_PGX_Trade_Information_Test.GetTag(), State);
	RecordResult(OutIssues, TEXT("InformationFreshness.InvalidTagFails"), !Invalid.bSuccess && Invalid.Code == EPGXTradeResultCode::InformationInvalid);
	RecordResult(OutIssues, TEXT("InformationFreshness.ValidClampsFreshness"), Valid.bSuccess && bState && FMath::IsNearlyEqual(State.Freshness, 1.0f));
	return !Invalid.bSuccess && Valid.bSuccess && bState && FMath::IsNearlyEqual(State.Freshness, 1.0f);
}

bool UPGXTradeTestUtility::NativeTagsRegisteredTest(const UObject* /*WorldContextObject*/, TArray<FString>& OutIssues)
{
	const bool bTags = TAG_PGX_Trade_Actor_Project.GetTag().IsValid()
		&& TAG_PGX_Trade_Good_Item.GetTag().IsValid()
		&& TAG_PGX_Trade_Source_Automation.GetTag().IsValid()
		&& TAG_PGX_Trade_Reputation_Reason_Test.GetTag().IsValid()
		&& TAG_PGX_Trade_Information_Test.GetTag().IsValid();
	RecordResult(OutIssues, TEXT("NativeTagsRegistered.CoreTagsValid"), bTags);
	return bTags;
}

bool UPGXTradeTestUtility::ObservableConfigSchemaTest(const UObject* /*WorldContextObject*/, TArray<FString>& OutIssues)
{
	UPGXTradeConfig* Config = NewObject<UPGXTradeConfig>(GetTransientPackage(), UPGXTradeConfig::StaticClass(), NAME_None, RF_Transient);
	if (!Config)
	{
		RecordResult(OutIssues, TEXT("ObservableConfigSchema.TransientConfig"), false);
		return false;
	}

	const FPGXJsonValue Json = Config->ToJson();
	const FPGXValidationResult EmptyValidation = Config->FromJson(FPGXJsonValue());
	const FPGXValidationResult EnvelopeValidation = Config->FromJson(Json);
	const FPGXSchemaDescriptor Descriptor = Config->GetSchemaDescriptor();
	const FString ExpectedType = FString::Printf(TEXT("\"type\":\"%s\""), *Config->GetClass()->GetName());

	const bool bJsonEnvelope = Json.JsonString.Contains(ExpectedType)
		&& Json.JsonString.Contains(TEXT("\"plugin\":\"PGXTradeRuntime\""));
	const bool bSchema = Config->GetSchemaVersion() == UPGXTradeConfig::SchemaVersion
		&& Descriptor.TypeName == UPGXTradeConfig::StaticClass()->GetFName()
		&& Descriptor.SchemaVersion == UPGXTradeConfig::SchemaVersion
		&& Descriptor.OwningPlugin == FName(TEXT("PGXTradeRuntime"))
		&& Descriptor.Fields.Num() == 5;

	RecordResult(OutIssues, TEXT("ObservableConfigSchema.JsonEnvelope"), bJsonEnvelope);
	RecordResult(OutIssues, TEXT("ObservableConfigSchema.SchemaDescriptor"), bSchema);
	RecordResult(OutIssues, TEXT("ObservableConfigSchema.EmptyPayloadFails"), !EmptyValidation.bValid && EmptyValidation.Errors.Num() == 1);
	RecordResult(OutIssues, TEXT("ObservableConfigSchema.EnvelopeValidates"), EnvelopeValidation.bValid);

	return bJsonEnvelope && bSchema && !EmptyValidation.bValid && EnvelopeValidation.bValid;
}

bool UPGXTradeTestUtility::ValidateAll(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	bool bAll = true;
	bAll &= SubsystemInitializeTest(WorldContextObject, OutIssues);
	bAll &= ConfigDefaultsTest(WorldContextObject, OutIssues);
	bAll &= ActorRegistrationTest(WorldContextObject, OutIssues);
	bAll &= OfferLifecycleTest(WorldContextObject, OutIssues);
	bAll &= ReputationReasonGuardTest(WorldContextObject, OutIssues);
	bAll &= InformationFreshnessTest(WorldContextObject, OutIssues);
	bAll &= NativeTagsRegisteredTest(WorldContextObject, OutIssues);
	bAll &= ObservableConfigSchemaTest(WorldContextObject, OutIssues);
	RecordResult(OutIssues, TEXT("ValidateAll.Aggregate"), bAll);
	return bAll;
}
