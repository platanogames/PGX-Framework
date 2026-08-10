// Copyright PGX Framework. All Rights Reserved.

#include "PGXEnvironmentTickProfile.h"
#include "Observability/PGXCoreObservability.h"

// EN: Authoring-only DA. Runtime scheduler that consumes these fields lands
//     outside the current product boundary (no propagation scheduler
//     the current product scope). No constructor logic in baseline.
// ES: DA solo authoring. El scheduler runtime que consume estos campos
//     aterriza en una version futura (propagation scheduler diferido per
//     current product boundary). Sin logica de constructor en baseline.


FPGXJsonValue UPGXEnvironmentTickProfile::ToJson() const
{
	return PGXCoreObservability::MakeJsonEnvelope(this, GetSchemaVersion());
}

FPGXValidationResult UPGXEnvironmentTickProfile::FromJson(const FPGXJsonValue& Json)
{
	return PGXCoreObservability::ValidateJsonEnvelope(Json);
}

FName UPGXEnvironmentTickProfile::GetSchemaVersion() const
{
	return TEXT("1.0");
}

FPGXSchemaDescriptor UPGXEnvironmentTickProfile::GetSchemaDescriptor() const
{
	return PGXCoreObservability::MakeSchemaDescriptor(this, GetSchemaVersion());
}
