// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXSpawnConfig.h"

#include "PGXSpawnObservability.h"
#include "Observability/PGXCoreObservability.h"

const FName UPGXSpawnConfig::SchemaVersion(TEXT("1.0"));

FPGXJsonValue UPGXSpawnConfig::ToJson() const
{
	const FString DataJson = FString::Printf(
		TEXT("{\"MaxConcurrentActors\":%d,\"DefaultWaveDelay\":%.6f,\"bUsePoolingForSpawns\":%s,\"SpawnCheckInterval\":%.6f,\"MaxSpawnDistance\":%.6f,\"MinSpawnDistance\":%.6f}"),
		MaxConcurrentActors,
		DefaultWaveDelay,
		bUsePoolingForSpawns ? TEXT("true") : TEXT("false"),
		SpawnCheckInterval,
		MaxSpawnDistance,
		MinSpawnDistance);
	return PGXSpawnObservability::MakeJsonEnvelope(this, GetSchemaVersion(), DataJson);
}

FPGXValidationResult UPGXSpawnConfig::FromJson(const FPGXJsonValue& Json)
{
	return PGXCoreObservability::ValidateJsonEnvelope(Json);
}

FName UPGXSpawnConfig::GetSchemaVersion() const
{
	return SchemaVersion;
}

FPGXSchemaDescriptor UPGXSpawnConfig::GetSchemaDescriptor() const
{
	return PGXCoreObservability::MakeSchemaDescriptor(this, GetSchemaVersion());
}
