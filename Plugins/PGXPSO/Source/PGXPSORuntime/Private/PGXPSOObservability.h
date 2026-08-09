// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

// EN: IPGXObservable helper namespace for PGXPSO.
//     Implements the shared observable-helper contract used by PGX runtime modules.
// ES: namespace helper IPGXObservable para PGXPSO.
//     Implementa el contrato compartido de helpers observables de los modulos runtime PGX.

#include "CoreMinimal.h"
#include "Observability/PGXObservable.h"

class UObject;

namespace PGXPSOObservability
{
	/**
	 * EN: Build a JSON envelope describing the IPGXObservable owner.
	 * ES: Construye un envelope JSON que describe el owner IPGXObservable.
	 */
	FPGXJsonValue MakeJsonEnvelope(const UObject* Owner, FName SchemaVersion);

	/**
	 * EN: Validate that the supplied JSON envelope is non-empty and well-formed.
	 * ES: Valida que el envelope JSON suministrado es no-vacio y bien formado.
	 */
	FPGXValidationResult ValidateJsonEnvelope(const FPGXJsonValue& Json);

	/**
	 * EN: Build a schema descriptor from the owner's reflected UPROPERTY fields.
	 * ES: Construye un schema descriptor desde los campos UPROPERTY reflejados del owner.
	 */
	FPGXSchemaDescriptor MakeSchemaDescriptor(const UObject* Owner, FName SchemaVersion);
}
