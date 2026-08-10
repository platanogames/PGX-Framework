// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PGXLoadingTypes.h"
#include "PGXStreamingManager.generated.h"

/**
 * EN: Zone-based streaming manager placeholder. The capability probe returns a
 *     deterministic Unsupported result until an implementation is available. Callers
 *     can use Unreal Engine level-streaming facilities directly in the meantime.
 *
 * ES: Placeholder de streaming por zonas. El probe de capacidad retorna Unsupported
 *     de forma determinista hasta disponer de implementacion. Mientras tanto, los
 *     callers pueden usar directamente las herramientas de level streaming del motor.
 */
UCLASS(BlueprintType)
class PGXLOADINGRUNTIME_API UPGXStreamingManager : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * EN: Returns false until the real implementation lands. Canonical
	 *     capability probe before attempting to use UPGXStreamingManager
	 *     functionality. Compile-time-stable inline.
	 * ES: Retorna false hasta que la impl real aterrice. Probe canonico de
	 *     capacidad antes de intentar usar funcionalidad de
	 *     UPGXStreamingManager. Inline estable en compile-time.
	 */
	UFUNCTION(BlueprintPure, Category = "PGX|Loading|StreamingManager")
	bool IsImplemented() const { return false; }

	/**
	 * EN: Canonical reason text for the Unsupported gate.
	 * ES: Texto canonico de razon para el gate Unsupported.
	 */
	UFUNCTION(BlueprintPure, Category = "PGX|Loading|StreamingManager")
	static FString GetUnsupportedReason()
	{
		return TEXT("UPGXStreamingManager is not yet implemented. Use UE level streaming volumes or IStreamingManager directly for now.");
	}

	/**
	 * EN: Capability probe entry point — always returns
	 *     EPGXLoadingResultCode::Unsupported with GetUnsupportedReason().
	 *     Makes the unsupported state explicit instead of silently succeeding.
	 * ES: Entry point de probe de capacidad — siempre retorna
	 *     EPGXLoadingResultCode::Unsupported con GetUnsupportedReason().
	 *     Hace explicito el estado unsupported en lugar de devolver exito silencioso.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Loading|StreamingManager")
	FPGXLoadingResult ProbeUnsupported() const;
};
