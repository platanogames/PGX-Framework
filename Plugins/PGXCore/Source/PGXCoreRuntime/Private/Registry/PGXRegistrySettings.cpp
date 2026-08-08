// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Registry/PGXRegistrySettings.h"
#include "Observability/PGXCoreObservability.h"

#define LOCTEXT_NAMESPACE "PGXRegistrySettings"

FText UPGXRegistrySettings::GetSectionText() const
{
	return LOCTEXT("SectionText", "Registry");
}

FText UPGXRegistrySettings::GetSectionDescription() const
{
	return LOCTEXT("SectionDesc",
		"Configure the PGX Data Registry v2.0. "
		"Controls DataTable scan roots, validation budgets, "
		"tag namespace policy, and conflict resolution defaults.");
}

#undef LOCTEXT_NAMESPACE

// ============================================================================
// EN: IPGXObservable adoption — delegates to PGXCoreObservability.
// ES: Adopcion IPGXObservable (IPGXObservable integration) — delega a PGXCoreObservability.
// ============================================================================

FPGXJsonValue UPGXRegistrySettings::ToJson() const
{
	return PGXCoreObservability::MakeJsonEnvelope(this, GetSchemaVersion());
}

FPGXValidationResult UPGXRegistrySettings::FromJson(const FPGXJsonValue& Json)
{
	return PGXCoreObservability::ValidateJsonEnvelope(Json);
}

FName UPGXRegistrySettings::GetSchemaVersion() const
{
	return TEXT("2.0");
}

FPGXSchemaDescriptor UPGXRegistrySettings::GetSchemaDescriptor() const
{
	return PGXCoreObservability::MakeSchemaDescriptor(this, GetSchemaVersion());
}
