// Copyright PGX Framework. All Rights Reserved.

#include "PGXTradeBlueprintLibrary.h"

#include "PGXTradeSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UPGXTradeSubsystem* UPGXTradeBlueprintLibrary::GetTradeSubsystem(const UObject* WorldContextObject)
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

FPGXTradeResult UPGXTradeBlueprintLibrary::RegisterTradeActor(const UObject* WorldContextObject, const FPGXTradeActorRecord& Actor)
{
	if (UPGXTradeSubsystem* Trade = GetTradeSubsystem(WorldContextObject))
	{
		return Trade->RegisterActor(Actor);
	}
	return FPGXTradeResult::Failure(EPGXTradeResultCode::NotInitialized, TEXT("Trade subsystem unavailable."));
}

FPGXTradeResult UPGXTradeBlueprintLibrary::CreateTradeOffer(const UObject* WorldContextObject, const FPGXTradeOfferRequest& Request, FPGXTradeOffer& OutOffer)
{
	if (UPGXTradeSubsystem* Trade = GetTradeSubsystem(WorldContextObject))
	{
		return Trade->CreateOffer(Request, OutOffer);
	}
	return FPGXTradeResult::Failure(EPGXTradeResultCode::NotInitialized, TEXT("Trade subsystem unavailable."));
}

FPGXTradeResult UPGXTradeBlueprintLibrary::AcceptTradeOffer(const UObject* WorldContextObject, FPGXTradeOfferId OfferId, FGameplayTag ReasonTag)
{
	if (UPGXTradeSubsystem* Trade = GetTradeSubsystem(WorldContextObject))
	{
		return Trade->AcceptOffer(OfferId, ReasonTag);
	}
	return FPGXTradeResult::Failure(EPGXTradeResultCode::NotInitialized, TEXT("Trade subsystem unavailable."));
}
