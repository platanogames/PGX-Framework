// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXEnvironmentZoneDefinition.h"
#include "Observability/PGXCoreObservability.h"

// EN: Authoring-only Object DA. No runtime constructor logic in baseline.
// ES: Object DA solo authoring. Sin logica de constructor runtime en baseline.


FPGXJsonValue UPGXEnvironmentZoneDefinition::ToJson() const
{
	return PGXCoreObservability::MakeJsonEnvelope(this, GetSchemaVersion());
}

FPGXValidationResult UPGXEnvironmentZoneDefinition::FromJson(const FPGXJsonValue& Json)
{
	return PGXCoreObservability::ValidateJsonEnvelope(Json);
}

FName UPGXEnvironmentZoneDefinition::GetSchemaVersion() const
{
	return TEXT("1.0");
}

FPGXSchemaDescriptor UPGXEnvironmentZoneDefinition::GetSchemaDescriptor() const
{
	return PGXCoreObservability::MakeSchemaDescriptor(this, GetSchemaVersion());
}
