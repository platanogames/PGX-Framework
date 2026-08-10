// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Data/PGXAudioDuckingConfig.h"
#include "Observability/PGXCoreObservability.h"

FPGXJsonValue UPGXAudioDuckingConfig::ToJson() const
{
	return PGXCoreObservability::MakeJsonEnvelope(this, GetSchemaVersion());
}

FPGXValidationResult UPGXAudioDuckingConfig::FromJson(const FPGXJsonValue& Json)
{
	return PGXCoreObservability::ValidateJsonEnvelope(Json);
}

FName UPGXAudioDuckingConfig::GetSchemaVersion() const
{
	return TEXT("1.0");
}

FPGXSchemaDescriptor UPGXAudioDuckingConfig::GetSchemaDescriptor() const
{
	return PGXCoreObservability::MakeSchemaDescriptor(this, GetSchemaVersion());
}
