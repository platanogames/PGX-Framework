// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Observability/PGXJsonValue.h"
#include "Observability/PGXSchemaDescriptor.h"
#include "Observability/PGXValidationResult.h"
#include "PGXObservable.generated.h"

/**
 * EN: Contract for objects exposed to the PGX Observability Framework. Every PGX-defined
 *     DataAsset / DataTable / configuration struct that ships in L1 (PGXCoreRuntime) or any
 *     L2 plugin SHOULD implement this interface; Tool plugins may opt in when useful.
 *     Default implementation in `UPGXObservableBase`
 *     reflects all `UPROPERTY`-marked fields automatically; subclasses override only when
 *     custom serialization is needed.
 *
 *     Schema versioning: each implementing class declares a `static const FName SchemaVersion`
 *     constant (or returns a literal from `GetSchemaVersion()`). Bumped manually when the
 *     C++ struct changes in breaking ways. The bridge refuses writes when JSON schema
 *     version doesn't match the runtime version. Migration support remains a
 *     capability not provided by this interface.
 *
 *     Validation: `FromJson()` runs `FPGXValidationResult` per-call. Soft-fail mode
 *     (warnings only, `bValid=true`) is opt-in for migration tooling.
 *
 *     Style consistency: drop-in compatible with the `IPGX*` interface family
 *     (`IPGXSaveable`, `IPGXValidatable`, `IPGXConfigurable`, `IPGXInitializable`,
 *     `IPGXProfileAware`, `IPGXPoolable`) — same `UINTERFACE(MinimalAPI, BlueprintType)`
 *     boilerplate, EN/ES bilingual docstrings, `PGXCORERUNTIME_API` export macro,
 *     pure-virtual methods.
 *
 *     Initial observability behavior:
 *       1. Interface contract (this file) — four pure-virtual methods.
 *       2. Companion types: `FPGXJsonValue`, `FPGXValidationResult`, `FPGXSchemaDescriptor`.
 *       3. `UPGXObservableBase` abstract default with reflection-driven UPROPERTY scaffold
 *          (no concrete JSON serialization-library binding is provided).
 *       4. `FPGXObservabilityRegistry` auto-discovery + manual fallback.
 *
 * ES: Contrato para objetos expuestos al PGX Observability Framework. Cada DataAsset /
 *     DataTable / struct de configuracion definido por PGX en L1 o L2 DEBE implementar esta
 *     interfaz. Implementacion default en `UPGXObservableBase` refleja UPROPERTY
 *     automaticamente. Estilo drop-in con la familia IPGX* existente.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UPGXObservable : public UInterface
{
	GENERATED_BODY()
};

class PGXCORERUNTIME_API IPGXObservable
{
	GENERATED_BODY()

public:
	/**
	 * EN: Returns canonical JSON representation of this object. Schema includes type, version,
	 *     and all UPROPERTY-marked fields (default behavior in UPGXObservableBase).
	 * ES: Retorna representacion JSON canonica del objeto.
	 */
	virtual FPGXJsonValue ToJson() const = 0;

	/**
	 * EN: Populates this object from JSON. Returns FPGXValidationResult with errors if schema
	 *     violation. Soft-fail-mode opt-in for migration tooling.
	 * ES: Pobla el objeto desde JSON. Retorna validacion con errores si hay violacion de schema.
	 */
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) = 0;

	/**
	 * EN: Returns the schema version this object's class declares. Bumped manually on
	 *     breaking C++ struct changes. Convention: dotted version (e.g. `1.3`) as FName.
	 * ES: Retorna la version del schema declarada por la clase.
	 */
	virtual FName GetSchemaVersion() const = 0;

	/**
	 * EN: Returns the schema descriptor (field names + types + constraints + reflection
	 *     metadata).
	 * ES: Retorna el descriptor del schema.
	 */
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const = 0;
};
