// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXTradeTypes.h"

FPGXTradeActorId FPGXTradeActorId::NewId()
{
	FPGXTradeActorId Out;
	Out.Id = FGuid::NewGuid();
	return Out;
}

FPGXTradeOfferId FPGXTradeOfferId::NewId()
{
	FPGXTradeOfferId Out;
	Out.Id = FGuid::NewGuid();
	return Out;
}

FPGXTradeTransactionId FPGXTradeTransactionId::NewId()
{
	FPGXTradeTransactionId Out;
	Out.Id = FGuid::NewGuid();
	return Out;
}

bool FPGXTradeOfferRequest::IsValid() const
{
	if (!SellerActorId.IsValid() || !BuyerActorId.IsValid() || SellerActorId == BuyerActorId || OfferedGoods.IsEmpty() || RequestedGoods.IsEmpty())
	{
		return false;
	}
	for (const FPGXTradeGood& Good : OfferedGoods)
	{
		if (!Good.IsValid())
		{
			return false;
		}
	}
	for (const FPGXTradeGood& Good : RequestedGoods)
	{
		if (!Good.IsValid())
		{
			return false;
		}
	}
	return true;
}

FPGXTradeResult FPGXTradeResult::Success(FString InMessage, FPGXTradeOfferId InOfferId, FPGXTradeTransactionId InTransactionId)
{
	FPGXTradeResult Result;
	Result.bSuccess = true;
	Result.Code = EPGXTradeResultCode::Success;
	Result.OfferId = InOfferId;
	Result.TransactionId = InTransactionId;
	Result.Message = MoveTemp(InMessage);
	return Result;
}

FPGXTradeResult FPGXTradeResult::Failure(EPGXTradeResultCode InCode, FString InMessage, FPGXTradeOfferId InOfferId)
{
	FPGXTradeResult Result;
	Result.bSuccess = false;
	Result.Code = InCode;
	Result.OfferId = InOfferId;
	Result.Message = MoveTemp(InMessage);
	return Result;
}

uint32 GetTypeHash(const FPGXTradeActorId& ActorId)
{
	return GetTypeHash(ActorId.Id);
}

uint32 GetTypeHash(const FPGXTradeOfferId& OfferId)
{
	return GetTypeHash(OfferId.Id);
}
