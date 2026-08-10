// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "EventHandler/PGXEventHandlerSettings.h"
#include "Observability/PGXCoreObservability.h"

// ============================================================================
// EN: UPGXEventHandlerSettings — UPGXSettings subclass for Project Settings > PGX > Event Handler.
//     Most logic lives inline in the header (`GetSectionName`). Translation unit added
//     IPGXObservable adoption — delegates to
//     PGXCoreObservability helper namespace.
// ES: UPGXEventHandlerSettings — subclase de UPGXSettings.
//     La adopcion IPGXObservable.
// ============================================================================

FPGXJsonValue UPGXEventHandlerSettings::ToJson() const
{
	return PGXCoreObservability::MakeJsonEnvelope(this, GetSchemaVersion());
}

FPGXValidationResult UPGXEventHandlerSettings::FromJson(const FPGXJsonValue& Json)
{
	return PGXCoreObservability::ValidateJsonEnvelope(Json);
}

FName UPGXEventHandlerSettings::GetSchemaVersion() const
{
	return TEXT("1.0");
}

FPGXSchemaDescriptor UPGXEventHandlerSettings::GetSchemaDescriptor() const
{
	return PGXCoreObservability::MakeSchemaDescriptor(this, GetSchemaVersion());
}
