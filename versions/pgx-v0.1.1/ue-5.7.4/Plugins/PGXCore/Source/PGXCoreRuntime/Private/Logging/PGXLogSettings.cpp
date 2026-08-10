// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Logging/PGXLogSettings.h"
#include "Observability/PGXCoreObservability.h"

// ============================================================================
// EN: UPGXLogSettings — UPGXSettings subclass for Project Settings > PGX > Log System.
//     Most logic lives inline in the header (`GetSectionName`). Translation unit added
//     IPGXObservable adoption — delegates to
//     PGXCoreObservability helper namespace.
// ES: UPGXLogSettings — subclase de UPGXSettings para Project Settings > PGX > Log System.
//     La adopcion IPGXObservable.
// ============================================================================

FPGXJsonValue UPGXLogSettings::ToJson() const
{
	return PGXCoreObservability::MakeJsonEnvelope(this, GetSchemaVersion());
}

FPGXValidationResult UPGXLogSettings::FromJson(const FPGXJsonValue& Json)
{
	return PGXCoreObservability::ValidateJsonEnvelope(Json);
}

FName UPGXLogSettings::GetSchemaVersion() const
{
	return TEXT("1.0");
}

FPGXSchemaDescriptor UPGXLogSettings::GetSchemaDescriptor() const
{
	return PGXCoreObservability::MakeSchemaDescriptor(this, GetSchemaVersion());
}
