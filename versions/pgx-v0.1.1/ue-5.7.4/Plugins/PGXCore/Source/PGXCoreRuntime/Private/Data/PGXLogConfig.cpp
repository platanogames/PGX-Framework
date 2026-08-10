// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Data/PGXLogConfig.h"
#include "Observability/PGXCoreObservability.h"

// EN: UPGXLogConfig implementation. Behavior is data-driven through UPROPERTY fields
//     exposed in the editor. IPGXObservable adoption —
//     delegates to PGXCoreObservability helper namespace.
// ES: Implementacion de UPGXLogConfig. Comportamiento data-driven via UPROPERTY.
//     La adopcion IPGXObservable — delega al namespace
//     helper PGXCoreObservability.

FPGXJsonValue UPGXLogConfig::ToJson() const
{
	return PGXCoreObservability::MakeJsonEnvelope(this, GetSchemaVersion());
}

FPGXValidationResult UPGXLogConfig::FromJson(const FPGXJsonValue& Json)
{
	return PGXCoreObservability::ValidateJsonEnvelope(Json);
}

FName UPGXLogConfig::GetSchemaVersion() const
{
	return TEXT("3.0");
}

FPGXSchemaDescriptor UPGXLogConfig::GetSchemaDescriptor() const
{
	return PGXCoreObservability::MakeSchemaDescriptor(this, GetSchemaVersion());
}
