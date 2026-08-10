// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Observability/PGXJsonValue.h"
#include "Observability/PGXSchemaDescriptor.h"
#include "Observability/PGXValidationResult.h"

class UObject;

namespace PGXCraftingObservability
{
	FString EscapeJsonString(const FString& Value);
	FName GetOwningPluginName(const UObject* Object, FName FallbackPluginName);
	FPGXValidationResult ValidateJsonEnvelope(const FPGXJsonValue& Json, const FText& EmptyPayloadMessage);
	FPGXSchemaDescriptor MakeSchemaDescriptor(const UObject* Object, FName SchemaVersion, FName FallbackPluginName);
}
