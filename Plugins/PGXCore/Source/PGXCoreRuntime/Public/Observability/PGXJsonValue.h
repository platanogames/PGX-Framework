// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXJsonValue.generated.h"

class FJsonValue;

/**
 * EN: JSON envelope used by the PGX Observability Framework. Wraps a serialized JSON form
 *     (`JsonString`) plus an optional transient cached parsed form. Robust JSON
 *     library binding remains intentionally decoupled from this value type. `JsonString` is
 *     the canonical persistent form; `CachedParsed` is a runtime-only TSharedPtr<FJsonValue>
 *     wrapper using UE's built-in `FJsonValue` (Json module) — this is a placeholder pattern
 *     that may be replaced by a future serialization-library integration.
 *
 *     Serialization round-trip: `ToJson()` returns FPGXJsonValue with populated `JsonString`;
 *     `FromJson()` parses `JsonString` (lazy, into `CachedParsed`) and reflects fields back
 *     onto the implementing object via UPROPERTY iteration in `UPGXObservableBase` default
 *     implementation.
 *
 * ES: Envoltura JSON usada por el PGX Observability Framework. Envuelve la forma serializada
 *     (`JsonString`) mas una forma transitoria cacheada parseada opcional. La libreria JSON
 *     concreta se difiere a una integracion futura de serializacion.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXJsonValue
{
	GENERATED_BODY()

	/** EN: Canonical serialized JSON string form. Empty when value is unset. */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Observability")
	FString JsonString;

	/**
	 * EN: Transient cached parsed form. Non-UPROPERTY (runtime-only, not serialized). Lazy
	 *     populated by FromJson() / parsing utilities; cleared by ToJson() / mutation. Wraps
	 *     UE's built-in FJsonValue (Json module). Production JSON library decision deferred
	 *     to a future serialization extension.
	 * ES: Forma cacheada parseada transitoria. No-UPROPERTY.
	 */
	TSharedPtr<FJsonValue> CachedParsed;

	/** EN: True when JsonString is empty. */
	bool IsEmpty() const { return JsonString.IsEmpty(); }

	/** EN: Reset both serialized + cached forms. */
	void Reset()
	{
		JsonString.Reset();
		CachedParsed.Reset();
	}

	bool operator==(const FPGXJsonValue& Other) const { return JsonString == Other.JsonString; }
	bool operator!=(const FPGXJsonValue& Other) const { return !(*this == Other); }
};
