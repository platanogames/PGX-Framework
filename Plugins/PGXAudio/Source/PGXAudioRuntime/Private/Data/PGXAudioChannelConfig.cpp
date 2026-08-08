// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Data/PGXAudioChannelConfig.h"
#include "Observability/PGXCoreObservability.h"

FPGXJsonValue UPGXAudioChannelConfig::ToJson() const
{
	return PGXCoreObservability::MakeJsonEnvelope(this, GetSchemaVersion());
}

FPGXValidationResult UPGXAudioChannelConfig::FromJson(const FPGXJsonValue& Json)
{
	return PGXCoreObservability::ValidateJsonEnvelope(Json);
}

FName UPGXAudioChannelConfig::GetSchemaVersion() const
{
	return TEXT("1.0");
}

FPGXSchemaDescriptor UPGXAudioChannelConfig::GetSchemaDescriptor() const
{
	return PGXCoreObservability::MakeSchemaDescriptor(this, GetSchemaVersion());
}
