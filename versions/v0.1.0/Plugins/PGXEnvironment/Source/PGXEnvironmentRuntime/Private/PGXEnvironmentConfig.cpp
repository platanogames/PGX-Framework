// Copyright PGX Framework. All Rights Reserved.

#include "PGXEnvironmentConfig.h"
#include "Observability/PGXCoreObservability.h"

// EN: Config DA with no constructor logic. Authoring drives all values.
//     The current implementation does not include ValidateConfig / PostLoad hooks (e.g.
//     duplicate variable tag detection, zone tag uniqueness) that surface
//     authoring mistakes through FPGXEnvironmentResult InvalidConfig at
//     subsystem init.
// ES: Config DA sin logica de constructor. La autoria guia todos los valores.
//     La implementacion no incluye hooks ValidateConfig /
//     PostLoad (ej. deteccion de variable tag duplicada, unicidad de zone
//     tag) que superficien errores de authoring via FPGXEnvironmentResult
//     InvalidConfig en init del subsistema.


FPGXJsonValue UPGXEnvironmentConfig::ToJson() const
{
	return PGXCoreObservability::MakeJsonEnvelope(this, GetSchemaVersion());
}

FPGXValidationResult UPGXEnvironmentConfig::FromJson(const FPGXJsonValue& Json)
{
	return PGXCoreObservability::ValidateJsonEnvelope(Json);
}

FName UPGXEnvironmentConfig::GetSchemaVersion() const
{
	return TEXT("1.0");
}

FPGXSchemaDescriptor UPGXEnvironmentConfig::GetSchemaDescriptor() const
{
	return PGXCoreObservability::MakeSchemaDescriptor(this, GetSchemaVersion());
}
