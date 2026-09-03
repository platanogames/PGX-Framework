// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXEnvironmentVariable.h"
#include "Observability/PGXCoreObservability.h"

// EN: Object DA with no runtime constructor logic; authored properties provide
//     all values. PostLoad or Data Asset validation hooks are not included.
// ES: Object DA sin logica de constructor runtime; las propiedades autoradas
//     proporcionan todos los valores. No incluye hooks de PostLoad o validacion.


FPGXJsonValue UPGXEnvironmentVariable::ToJson() const
{
	return PGXCoreObservability::MakeJsonEnvelope(this, GetSchemaVersion());
}

FPGXValidationResult UPGXEnvironmentVariable::FromJson(const FPGXJsonValue& Json)
{
	return PGXCoreObservability::ValidateJsonEnvelope(Json);
}

FName UPGXEnvironmentVariable::GetSchemaVersion() const
{
	return TEXT("1.0");
}

FPGXSchemaDescriptor UPGXEnvironmentVariable::GetSchemaDescriptor() const
{
	return PGXCoreObservability::MakeSchemaDescriptor(this, GetSchemaVersion());
}
