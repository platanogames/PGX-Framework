// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXLoadingSettings.h"
#include "Observability/PGXCoreObservability.h"

// ============================================================================
// IPGXObservable contract — direct-inline implementation.
// EN: Direct-inline IPGXObservable for UPGXLoadingSettings — exposes Settings
//     surface (ActiveConfig + LoadingProfileTable + bVerboseConfigResolution)
//     via canonical JSON envelope + schema descriptor for external tooling.
// ES: Patron direct-inline IPGXObservable para UPGXLoadingSettings.
// ============================================================================

FPGXJsonValue UPGXLoadingSettings::ToJson() const
{
	return PGXCoreObservability::MakeJsonEnvelope(this, GetSchemaVersion());
}

FPGXValidationResult UPGXLoadingSettings::FromJson(const FPGXJsonValue& Json)
{
	return PGXCoreObservability::ValidateJsonEnvelope(Json);
}

FName UPGXLoadingSettings::GetSchemaVersion() const
{
	return FName(TEXT("1.0"));
}

FPGXSchemaDescriptor UPGXLoadingSettings::GetSchemaDescriptor() const
{
	return PGXCoreObservability::MakeSchemaDescriptor(this, GetSchemaVersion());
}
