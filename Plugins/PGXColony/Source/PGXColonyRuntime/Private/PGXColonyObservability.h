// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Observability/PGXJsonValue.h"

class UObject;

namespace PGXColonyObservability
{
	FPGXJsonValue MakeJsonEnvelope(const UObject* Object, FName SchemaVersion, const FString& DataJson);
}
