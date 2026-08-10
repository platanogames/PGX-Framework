// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

// EN: Global config DataAsset implementation (minimal — all logic in header)
// ES: Implementacion del config DataAsset global (minima — toda la logica en el header)

#include "PGXGameFlowConfig.h"
#include "Observability/PGXCoreObservability.h"

FPGXJsonValue UPGXGameFlowConfig::ToJson() const
{
	return PGXCoreObservability::MakeJsonEnvelope(this, GetSchemaVersion());
}

FPGXValidationResult UPGXGameFlowConfig::FromJson(const FPGXJsonValue& Json)
{
	return PGXCoreObservability::ValidateJsonEnvelope(Json);
}

FName UPGXGameFlowConfig::GetSchemaVersion() const
{
	return TEXT("1.0");
}

FPGXSchemaDescriptor UPGXGameFlowConfig::GetSchemaDescriptor() const
{
	return PGXCoreObservability::MakeSchemaDescriptor(this, GetSchemaVersion());
}
