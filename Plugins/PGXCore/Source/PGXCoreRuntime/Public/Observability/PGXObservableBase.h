// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Observability/PGXObservable.h"
#include "PGXObservableBase.generated.h"

/**
 * EN: Abstract default base class for PGX-defined observable types. Implements all four
 *     `IPGXObservable` methods using UE reflection over the subclass's `UPROPERTY` surface:
 *
 *     - `ToJson()` walks `FProperty*` chain, serializes each UPROPERTY into the JSON envelope.
 *     - `FromJson()` parses the JSON envelope, reflects values back into UPROPERTY slots,
 *       returns `FPGXValidationResult` with per-field errors on schema mismatch.
 *     - `GetSchemaVersion()` returns the `static const FName SchemaVersion` declared by the
 *       subclass (or `NAME_None` at base level — subclasses override to declare).
 *     - `GetSchemaDescriptor()` builds `FPGXSchemaDescriptor` from class metadata + UPROPERTY
 *       iteration.
 *
 *     Subclasses override individual methods only when custom serialization is required
 *     (e.g. classes with non-UPROPERTY runtime state that should/shouldn't serialize, or
 *     classes that need pre/post-serialize hooks).
 *
 *     **initial observability scaffold**: the reflection-driven default implementations are
 *     scaffolded with the correct method signatures + reflection iteration shape, but the
 *     concrete JSON parser library binding (UE FJsonValue vs nlohmann::json vs custom)
 *     remains an explicit future extension. In the current implementation,
 *     `ToJson()` returns a populated `FPGXJsonValue` envelope with type +
 *     schema version + plugin metadata; full UPROPERTY-to-JSON value reflection is the
 *     future property-serialization implementation.
 *
 *     Auto-registration: `UPGXObservableBase` subclasses are discovered via
 *     `TObjectIterator<UClass>` filter at module startup (`FPGXObservabilityRegistry`).
 *     Subclasses do NOT need to call `Register()` manually unless they want non-base
 *     observable types — see `FPGXObservabilityRegistry::Register()` for manual fallback.
 *
 * ES: Clase base abstracta default para tipos observables definidos por PGX. Implementa los
 *     4 metodos de IPGXObservable usando reflexion UE sobre la superficie UPROPERTY de la
 *     subclase. El scaffold inicial de observabilidad difiere la reflexion completa
 *     de valores UPROPERTY-to-JSON a una extension futura.
 */
UCLASS(Abstract, BlueprintType)
class PGXCORERUNTIME_API UPGXObservableBase : public UObject, public IPGXObservable
{
	GENERATED_BODY()

public:
	//~ Begin IPGXObservable
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;
	//~ End IPGXObservable

protected:
	/**
	 * EN: Subclasses override to declare schema version. Default returns `NAME_None`.
	 *     Convention: dotted version FName (e.g. `1.3`). Bumped manually on breaking
	 *     C++ struct changes.
	 * ES: Subclases sobrescriben para declarar version. Convencion: FName con version dotted.
	 */
	virtual FName GetDeclaredSchemaVersion() const { return NAME_None; }

	/**
	 * EN: Returns the owning plugin module name (e.g. `PGXCoreRuntime`, `PGXUIRuntime`).
	 *     Default derives from the subclass's package; subclasses may override to provide
	 *     a different attribution.
	 * ES: Retorna el nombre del modulo del plugin owning.
	 */
	virtual FName GetOwningPluginName() const;
};
