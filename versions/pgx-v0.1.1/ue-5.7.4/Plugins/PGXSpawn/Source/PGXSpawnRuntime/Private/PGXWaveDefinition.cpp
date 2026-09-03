// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXWaveDefinition.h"

#include "PGXSpawnObservability.h"
#include "Observability/PGXCoreObservability.h"

const FName UPGXWaveDefinition::SchemaVersion(TEXT("1.0"));

FPGXJsonValue UPGXWaveDefinition::ToJson() const
{
	const FString DataJson = FString::Printf(
		TEXT("{\"WaveName\":\"%s\",\"TotalSpawnCount\":%d,\"SpawnInterval\":%.6f}"),
		*PGXSpawnObservability::EscapeJsonString(WaveName.ToString()),
		TotalSpawnCount,
		SpawnInterval);
	return PGXSpawnObservability::MakeJsonEnvelope(this, GetSchemaVersion(), DataJson);
}

FPGXValidationResult UPGXWaveDefinition::FromJson(const FPGXJsonValue& Json)
{
	return PGXCoreObservability::ValidateJsonEnvelope(Json);
}

FName UPGXWaveDefinition::GetSchemaVersion() const
{
	return SchemaVersion;
}

FPGXSchemaDescriptor UPGXWaveDefinition::GetSchemaDescriptor() const
{
	return PGXCoreObservability::MakeSchemaDescriptor(this, GetSchemaVersion());
}

// EN: No implementation yet. / ES: Sin implementacion aun.
