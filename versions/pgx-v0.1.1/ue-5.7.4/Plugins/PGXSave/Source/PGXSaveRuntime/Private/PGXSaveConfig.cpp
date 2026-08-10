// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXSaveConfig.h"
#include "Observability/PGXCoreObservability.h"

FPGXJsonValue UPGXSaveConfig::ToJson() const
{
	return PGXCoreObservability::MakeJsonEnvelope(this, GetSchemaVersion());
}

FPGXValidationResult UPGXSaveConfig::FromJson(const FPGXJsonValue& Json)
{
	return PGXCoreObservability::ValidateJsonEnvelope(Json);
}

FName UPGXSaveConfig::GetSchemaVersion() const
{
	return TEXT("1.0");
}

FPGXSchemaDescriptor UPGXSaveConfig::GetSchemaDescriptor() const
{
	return PGXCoreObservability::MakeSchemaDescriptor(this, GetSchemaVersion());
}
