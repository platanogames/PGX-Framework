// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Data/PGXAudioProfile.h"
#include "Observability/PGXCoreObservability.h"

FPGXJsonValue UPGXAudioProfile::ToJson() const
{
	return PGXCoreObservability::MakeJsonEnvelope(this, GetSchemaVersion());
}

FPGXValidationResult UPGXAudioProfile::FromJson(const FPGXJsonValue& Json)
{
	return PGXCoreObservability::ValidateJsonEnvelope(Json);
}

FName UPGXAudioProfile::GetSchemaVersion() const
{
	return TEXT("1.0");
}

FPGXSchemaDescriptor UPGXAudioProfile::GetSchemaDescriptor() const
{
	return PGXCoreObservability::MakeSchemaDescriptor(this, GetSchemaVersion());
}
