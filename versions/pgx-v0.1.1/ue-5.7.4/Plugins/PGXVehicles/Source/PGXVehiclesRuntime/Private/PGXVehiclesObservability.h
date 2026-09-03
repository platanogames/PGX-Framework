// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Observability/PGXJsonValue.h"
#include "Observability/PGXSchemaDescriptor.h"
#include "Observability/PGXValidationResult.h"

class UObject;

namespace PGXVehiclesObservability
{
	FString EscapeJsonString(const FString& Value);
	FName GetOwningPluginName(const UObject* Object, FName FallbackPluginName);
	FPGXValidationResult ValidateJsonEnvelope(const FPGXJsonValue& Json, const FText& EmptyPayloadMessage);
	FPGXSchemaDescriptor MakeSchemaDescriptor(const UObject* Object, FName SchemaVersion, FName FallbackPluginName);
}
