// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Observability/PGXJsonValue.h"
#include "Observability/PGXSchemaDescriptor.h"
#include "Observability/PGXValidationResult.h"

class UObject;

namespace PGXCoreObservability
{
	/**
	 * EN: Shared helper for PGXCoreRuntime Config / Settings / Data Asset classes that implement
	 *     IPGXObservable directly. Used by PGXLog (3 classes: UPGXLogConfig / UPGXLogDomainConfig /
	 *     UPGXLogSettings), PGXDataRegistry (UPGXRegistrySettings + UPGXRegistryDefinition),
	 *     PGXEventHandler (UPGXEventHandlerConfig + UPGXEventHandlerSettings) — 6 Core
	 *     subsystem Config / Data Asset classes within PGXCoreRuntime adopting IPGXObservable.
	 * ES: Helper compartido para clases Config / Settings / Data Asset de PGXCoreRuntime que
	 *     implementan IPGXObservable directamente. Usado por las 6 clases Core subsystem.
	 */
	PGXCORERUNTIME_API FPGXJsonValue MakeJsonEnvelope(const UObject* Object, FName SchemaVersion);
	PGXCORERUNTIME_API FPGXValidationResult ValidateJsonEnvelope(const FPGXJsonValue& Json);
	PGXCORERUNTIME_API FPGXSchemaDescriptor MakeSchemaDescriptor(const UObject* Object, FName SchemaVersion);
}
