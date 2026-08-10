// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Registry/PGXRegistryDefinition.h"
#include "Observability/PGXCoreObservability.h"

// ============================================================================
// EN: UPGXRegistryDefinition implementation. Most logic is inline in header
//     (IsCategoryAllowed / IsValidForIngestion). Translation unit hosts IPGXObservable methods and delegates to
//     PGXCoreObservability helper namespace.
// ES: Implementacion de UPGXRegistryDefinition. Logica inline en header.
//     La adopcion IPGXObservable.
// ============================================================================

FPGXJsonValue UPGXRegistryDefinition::ToJson() const
{
	return PGXCoreObservability::MakeJsonEnvelope(this, GetSchemaVersion());
}

FPGXValidationResult UPGXRegistryDefinition::FromJson(const FPGXJsonValue& Json)
{
	return PGXCoreObservability::ValidateJsonEnvelope(Json);
}

FName UPGXRegistryDefinition::GetSchemaVersion() const
{
	return TEXT("2.0");
}

FPGXSchemaDescriptor UPGXRegistryDefinition::GetSchemaDescriptor() const
{
	return PGXCoreObservability::MakeSchemaDescriptor(this, GetSchemaVersion());
}
