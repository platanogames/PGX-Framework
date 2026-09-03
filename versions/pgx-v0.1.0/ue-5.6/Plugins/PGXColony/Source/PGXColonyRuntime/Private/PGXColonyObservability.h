// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Observability/PGXJsonValue.h"

class UObject;

namespace PGXColonyObservability
{
	FPGXJsonValue MakeJsonEnvelope(const UObject* Object, FName SchemaVersion, const FString& DataJson);
}
