// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXSchemaDescriptor.generated.h"

/**
 * EN: Descriptor for a single field declared by an IPGXObservable class's UPROPERTY surface.
 *     Built from UE reflection at GetSchemaDescriptor() time. Exposes field name + UE-type
 *     identifier + required flag + constraint hints (ClampMin/Max + Categories filter +
 *     meta tags) for external tooling consumption.
 *
 * ES: Descriptor de un campo individual declarado por una clase IPGXObservable. Construido
 *     desde la reflexion UE en GetSchemaDescriptor().
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXSchemaField
{
	GENERATED_BODY()

	/** EN: UPROPERTY field name (matches C++ identifier). */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Observability")
	FName FieldName;

	/**
	 * EN: UE type identifier as FName (e.g. `FString`, `float`, `FGameplayTag`,
	 *     `TSoftObjectPtr<UPGXUIConfig>`). Built from FProperty type traversal.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Observability")
	FName FieldType;

	/** EN: True when the field has no default value (must be authored). */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Observability")
	bool bRequired = true;

	/** EN: Constraint hints (ClampMin/Max + Categories filter + meta tags) as FText. */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Observability")
	FText Constraints;
};

/**
 * EN: Schema descriptor returned by `IPGXObservable::GetSchemaDescriptor()`. Carries the
 *     class name, schema version (FName, e.g. `1.3`), owning plugin module name, and the
 *     full ordered list of field descriptors. Consumers (doc generators,
 *     validation pipelines) use this as the canonical introspection point — no per-tool
 *     wrappers required.
 *
 *     Schema versioning: `SchemaVersion` is bumped manually by the implementing class
 *     when a breaking C++ struct change ships. The bridge refuses writes when JSON's
 *     reported schema version doesn't match the runtime version. The
 *     migration registry in `PGXCoreEditor` registers converters
 *     between schema versions.
 *
 * ES: Descriptor de schema retornado por GetSchemaDescriptor(). Carga nombre clase, version
 *     schema, plugin owning, y lista ordenada de campos. Versioning manual por clase.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXSchemaDescriptor
{
	GENERATED_BODY()

	/** EN: Class name of the observable type (e.g. `UPGXUIConfig`). */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Observability")
	FName TypeName;

	/** EN: Schema version this object's class declares (e.g. `1.3`). Bumped manually on breaking changes. */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Observability")
	FName SchemaVersion;

	/** EN: Owning plugin module name (e.g. `PGXCoreRuntime`, `PGXUIRuntime`). */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Observability")
	FName OwningPlugin;

	/** EN: Ordered list of field descriptors (matches UPROPERTY declaration order). */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Observability")
	TArray<FPGXSchemaField> Fields;
};
