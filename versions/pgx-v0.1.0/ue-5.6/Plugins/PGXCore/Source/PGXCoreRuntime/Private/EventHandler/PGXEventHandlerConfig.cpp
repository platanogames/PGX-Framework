// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "EventHandler/PGXEventHandlerConfig.h"
#include "Observability/PGXCoreObservability.h"

UPGXEventHandlerConfig::UPGXEventHandlerConfig()
{
	ConfigDisplayName = FText::FromString(TEXT("Event Handler Config"));
	ConfigDescription = FText::FromString(TEXT("Configuration for the PGX Event Handler System (behavior resolution bus)."));
}

// ============================================================================
// EN: IPGXObservable methods delegate to the shared PGXCoreObservability helpers,
//     matching the other observable configuration types.
// ES: Los metodos IPGXObservable delegan en los helpers compartidos de
//     PGXCoreObservability, igual que los otros tipos de configuracion observable.
// ============================================================================

FPGXJsonValue UPGXEventHandlerConfig::ToJson() const
{
	return PGXCoreObservability::MakeJsonEnvelope(this, GetSchemaVersion());
}

FPGXValidationResult UPGXEventHandlerConfig::FromJson(const FPGXJsonValue& Json)
{
	return PGXCoreObservability::ValidateJsonEnvelope(Json);
}

FName UPGXEventHandlerConfig::GetSchemaVersion() const
{
	return TEXT("1.0");
}

FPGXSchemaDescriptor UPGXEventHandlerConfig::GetSchemaDescriptor() const
{
	return PGXCoreObservability::MakeSchemaDescriptor(this, GetSchemaVersion());
}
