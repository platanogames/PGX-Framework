// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXTradeConfig.h"

const FName UPGXTradeConfig::SchemaVersion(TEXT("1.0"));

bool UPGXTradeConfig::IsPolicyValid() const
{
	return FairTradeTolerance >= 0.0f
		&& DefaultOfferExpirationSeconds >= 0.0f
		&& MinReputation <= MaxReputation
		&& DefaultInformationFreshness >= 0.0f
		&& DefaultInformationFreshness <= 1.0f;
}

FPGXJsonValue UPGXTradeConfig::ToJson() const
{
	FPGXJsonValue Out;
	Out.JsonString = FString::Printf(
		TEXT("{\"schema\":{\"type\":\"%s\",\"version\":\"%s\",\"plugin\":\"PGXTradeRuntime\"},\"data\":{\"FairTradeTolerance\":%.6f,\"DefaultOfferExpirationSeconds\":%.6f,\"MinReputation\":%.6f,\"MaxReputation\":%.6f,\"DefaultInformationFreshness\":%.6f}}"),
		*GetClass()->GetName(),
		*GetSchemaVersion().ToString(),
		FairTradeTolerance,
		DefaultOfferExpirationSeconds,
		MinReputation,
		MaxReputation,
		DefaultInformationFreshness);
	return Out;
}

FPGXValidationResult UPGXTradeConfig::FromJson(const FPGXJsonValue& Json)
{
	if (Json.IsEmpty())
	{
		return FPGXValidationResult::MakeFailure(
			TEXT("EmptyPayload"),
			TEXT(""),
			NSLOCTEXT("PGXTrade", "ObservableEmptyPayload", "PGXTradeConfig FromJson received an empty payload."));
	}

	// Sub-Serialization.3.B alpha pilot: IPGXObservable adoption is safe for schema discovery
	// and export. Concrete UPROPERTY mutation from JSON is unavailable because the core
	// observability parser/migration policy is locked.
	return FPGXValidationResult::MakeValid();
}

FName UPGXTradeConfig::GetSchemaVersion() const
{
	return SchemaVersion;
}

FPGXSchemaDescriptor UPGXTradeConfig::GetSchemaDescriptor() const
{
	FPGXSchemaDescriptor Descriptor;
	Descriptor.TypeName = GetClass()->GetFName();
	Descriptor.SchemaVersion = GetSchemaVersion();
	Descriptor.OwningPlugin = FName(TEXT("PGXTradeRuntime"));

	auto AddFloatField = [&Descriptor](const TCHAR* FieldName, const TCHAR* ConstraintText)
	{
		FPGXSchemaField Field;
		Field.FieldName = FName(FieldName);
		Field.FieldType = FName(TEXT("float"));
		Field.bRequired = true;
		Field.Constraints = FText::FromString(ConstraintText);
		Descriptor.Fields.Add(Field);
	};

	AddFloatField(TEXT("FairTradeTolerance"), TEXT("ClampMin=0.0"));
	AddFloatField(TEXT("DefaultOfferExpirationSeconds"), TEXT("ClampMin=0.0"));
	AddFloatField(TEXT("MinReputation"), TEXT("ClampMin=-100.0 ClampMax=100.0"));
	AddFloatField(TEXT("MaxReputation"), TEXT("ClampMin=-100.0 ClampMax=100.0"));
	AddFloatField(TEXT("DefaultInformationFreshness"), TEXT("ClampMin=0.0 ClampMax=1.0"));

	return Descriptor;
}
