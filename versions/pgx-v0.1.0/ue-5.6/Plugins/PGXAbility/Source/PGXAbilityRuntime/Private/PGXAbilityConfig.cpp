// Copyright PGX Framework. All Rights Reserved.

#include "PGXAbilityConfig.h"
#include "Observability/PGXCoreObservability.h"

FPGXJsonValue UPGXAbilityConfig::ToJson() const
{
	return PGXCoreObservability::MakeJsonEnvelope(this, GetSchemaVersion());
}

FPGXValidationResult UPGXAbilityConfig::FromJson(const FPGXJsonValue& Json)
{
	return PGXCoreObservability::ValidateJsonEnvelope(Json);
}

FName UPGXAbilityConfig::GetSchemaVersion() const
{
	return TEXT("1.0");
}

FPGXSchemaDescriptor UPGXAbilityConfig::GetSchemaDescriptor() const
{
	return PGXCoreObservability::MakeSchemaDescriptor(this, GetSchemaVersion());
}
