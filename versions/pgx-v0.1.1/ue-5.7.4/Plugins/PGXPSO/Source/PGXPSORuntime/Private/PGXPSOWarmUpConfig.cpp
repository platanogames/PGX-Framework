// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXPSOWarmUpConfig.h"
#include "PGXPSOObservability.h"

FPGXJsonValue UPGXPSOWarmUpConfig::ToJson() const
{
	return PGXPSOObservability::MakeJsonEnvelope(this, GetSchemaVersion());
}

FPGXValidationResult UPGXPSOWarmUpConfig::FromJson(const FPGXJsonValue& Json)
{
	return PGXPSOObservability::ValidateJsonEnvelope(Json);
}

FName UPGXPSOWarmUpConfig::GetSchemaVersion() const
{
	return TEXT("1.0");
}

FPGXSchemaDescriptor UPGXPSOWarmUpConfig::GetSchemaDescriptor() const
{
	return PGXPSOObservability::MakeSchemaDescriptor(this, GetSchemaVersion());
}
