// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Data/PGXLogDomainConfig.h"
#include "Observability/PGXCoreObservability.h"

UPGXLogDomainConfig::UPGXLogDomainConfig()
{
	DomainDisplayName = FText::FromString(TEXT("Generic"));
}

FPrimaryAssetId UPGXLogDomainConfig::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("PGXLogDomainConfig"), GetFName());
}

// ============================================================================
// EN: IPGXObservable adoption — delegates to PGXCoreObservability helper.
// ES: Adopcion IPGXObservable (IPGXObservable integration) — delega a PGXCoreObservability.
// ============================================================================

FPGXJsonValue UPGXLogDomainConfig::ToJson() const
{
	return PGXCoreObservability::MakeJsonEnvelope(this, GetSchemaVersion());
}

FPGXValidationResult UPGXLogDomainConfig::FromJson(const FPGXJsonValue& Json)
{
	return PGXCoreObservability::ValidateJsonEnvelope(Json);
}

FName UPGXLogDomainConfig::GetSchemaVersion() const
{
	return TEXT("3.0");
}

FPGXSchemaDescriptor UPGXLogDomainConfig::GetSchemaDescriptor() const
{
	return PGXCoreObservability::MakeSchemaDescriptor(this, GetSchemaVersion());
}
