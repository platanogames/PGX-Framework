// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PGXLoadingTypes.h"
#include "PGXAsyncLoader.generated.h"

/**
 * EN: Priority-based async asset loader placeholder. The capability probe returns a
 *     deterministic Unsupported result until an implementation is available. Callers
 *     can use FStreamableManager directly when they need production loading today.
 *
 * ES: Placeholder de carga async de assets por prioridad. El probe de capacidad
 *     retorna Unsupported de forma determinista hasta disponer de implementacion.
 *     Los callers pueden usar FStreamableManager directamente cuando necesiten carga.
 */
UCLASS(BlueprintType)
class PGXLOADINGRUNTIME_API UPGXAsyncLoader : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * EN: Returns false until the real implementation lands. Callers should
	 *     treat this as the canonical capability probe before attempting to
	 *     use UPGXAsyncLoader functionality. Compile-time-stable inline.
	 * ES: Retorna false hasta que la impl real aterrice. Los callers deben
	 *     tratar esto como el probe canonico de capacidad antes de intentar
	 *     usar funcionalidad de UPGXAsyncLoader. Inline estable en compile-time.
	 */
	UFUNCTION(BlueprintPure, Category = "PGX|Loading|AsyncLoader")
	bool IsImplemented() const { return false; }

	/**
	 * EN: Canonical reason text for the Unsupported gate. Provided so callers
	 *     can surface a consistent diagnostic without inventing their own
	 *     wording.
	 * ES: Texto canonico de razon para el gate Unsupported. Provisto para que
	 *     los callers puedan superficiar un diagnostico consistente sin
	 *     inventar su propio wording.
	 */
	UFUNCTION(BlueprintPure, Category = "PGX|Loading|AsyncLoader")
	static FString GetUnsupportedReason()
	{
		return TEXT("UPGXAsyncLoader is not yet implemented. Use FStreamableManager directly for now.");
	}

	/**
	 * EN: Capability probe entry point — always returns
	 *     EPGXLoadingResultCode::Unsupported with GetUnsupportedReason() as
	 *     the description. Provides a stable, BP-callable surface for code that invokes the
	 *     async-load placeholder and makes the unsupported state explicit.
	 * ES: Entry point de probe de capacidad — siempre retorna
	 *     EPGXLoadingResultCode::Unsupported con GetUnsupportedReason() como
	 *     descripcion. Provee una surface BP-callable estable para el placeholder de async-load
	 *     y hace explicito el estado unsupported.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Loading|AsyncLoader")
	FPGXLoadingResult ProbeUnsupported() const;
};
