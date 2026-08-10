// Copyright PGX Framework. All Rights Reserved.

#include "PGXColonyConfig.h"

#include "PGXColonyObservability.h"
#include "Observability/PGXCoreObservability.h"

const FName UPGXColonyConfig::SchemaVersion(TEXT("1.0"));

FPGXJsonValue UPGXColonyConfig::ToJson() const
{
	const FString DataJson = FString::Printf(
		TEXT("{\"MaxSurvivorsPerSettlement\":%d,\"DefaultWorkerRoleWeight\":%.6f,\"DefaultMoraleDecayRate\":%.6f,\"ConflictMoraleThreshold\":%.6f,\"DormancyThresholdSeconds\":%.6f,\"bQueueDormantEventsForCatchUp\":%s}"),
		MaxSurvivorsPerSettlement,
		DefaultWorkerRoleWeight,
		DefaultMoraleDecayRate,
		ConflictMoraleThreshold,
		DormancyThresholdSeconds,
		bQueueDormantEventsForCatchUp ? TEXT("true") : TEXT("false"));
	return PGXColonyObservability::MakeJsonEnvelope(this, GetSchemaVersion(), DataJson);
}

FPGXValidationResult UPGXColonyConfig::FromJson(const FPGXJsonValue& Json)
{
	return PGXCoreObservability::ValidateJsonEnvelope(Json);
}

FName UPGXColonyConfig::GetSchemaVersion() const
{
	return SchemaVersion;
}

FPGXSchemaDescriptor UPGXColonyConfig::GetSchemaDescriptor() const
{
	return PGXCoreObservability::MakeSchemaDescriptor(this, GetSchemaVersion());
}

// EN: All Config DA fields are inline UPROPERTY defaults. This translation unit exists so the
//     UCLASS reflection table resolves and Build.cs always finds a matching cpp.
// ES: Todos los campos del Config DA son inline UPROPERTY defaults.
