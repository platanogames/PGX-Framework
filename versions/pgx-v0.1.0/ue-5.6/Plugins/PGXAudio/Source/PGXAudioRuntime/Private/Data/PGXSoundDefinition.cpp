// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Data/PGXSoundDefinition.h"
#include "Observability/PGXCoreObservability.h"

FPGXJsonValue UPGXSoundDefinition::ToJson() const
{
	return PGXCoreObservability::MakeJsonEnvelope(this, GetSchemaVersion());
}

FPGXValidationResult UPGXSoundDefinition::FromJson(const FPGXJsonValue& Json)
{
	return PGXCoreObservability::ValidateJsonEnvelope(Json);
}

FName UPGXSoundDefinition::GetSchemaVersion() const
{
	return TEXT("1.0");
}

FPGXSchemaDescriptor UPGXSoundDefinition::GetSchemaDescriptor() const
{
	return PGXCoreObservability::MakeSchemaDescriptor(this, GetSchemaVersion());
}
