// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXSettings.h"
#include "PGXMessageSettings.generated.h"

class UPGXMessageConfig;

// ============================================================================
// EN: Message System settings — appears in Project Settings > PGX > Message System.
//     Provides deterministic config resolution: assign the DA here instead of
//     relying on AssetRegistry auto-discovery.
//
// ES: Settings del Sistema de Mensajes — aparece en Project Settings > PGX > Message System.
//     Provee resolucion determinista de config: asignar el DA aqui en vez de
//     depender del auto-discovery de AssetRegistry.
// ============================================================================

UCLASS(config = PGX, defaultconfig, meta = (DisplayName = "PGX Message System"))
class PGXCORERUNTIME_API UPGXMessageSettings : public UPGXSettings
{
	GENERATED_BODY()

public:
	FName GetSectionName() const override { return TEXT("Message"); }

	/**
	 * EN: The active Message config DA. If empty, falls back to AssetRegistry discovery (deprecated).
	 * ES: El DA de config de Message activo. Si esta vacio, hace fallback a discovery de AssetRegistry (deprecated).
	 */
	UPROPERTY(config, EditAnywhere, Category = "Message",
		meta = (DisplayName = "Active Config"))
	TSoftObjectPtr<UPGXMessageConfig> ActiveConfig;

	/**
	 * EN: Emergency history retention used when CachedConfig fails to load.
	 *     Production path resolves history bound via Config DA; this is the
	 *     deterministic fallback exposed in Project Settings instead of a
	 *     hardcoded literal (the configuration-source invariant).
	 *     Set to 0 to disable history entirely when Config DA is missing.
	 * ES: Retencion de historial de emergencia cuando CachedConfig falla.
	 *     El path productivo resuelve el limite via Config DA; este es el
	 *     fallback determinista expuesto en Project Settings en vez de un
	 *     literal hardcoded por el invariante de fuente de configuracion.
	 *     Poner a 0 para deshabilitar historial cuando Config DA falta.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Message|Diagnostics",
		meta = (DisplayName = "Emergency History Fallback",
			ClampMin = "0", ClampMax = "1024"))
	int32 EmergencyHistoryFallback = 100;

	/**
	 * EN: Emergency broadcast recursion-depth cap used when CachedConfig fails to load.
	 *     Production path resolves via Config DA (MaxBroadcastRecursionDepth); this Settings
	 *     entry is the deterministic Project-Settings-driven fallback per the configuration-source invariant.
	 *     Mirrors the deterministic EmergencyHistoryFallback pattern.
	 *     Set to 0 to fail-open the recursion guard (queue cap remains the safety net).
	 *     Default 4 covers typical request->response->ack->finalize chains.
	 * ES: Cap de emergencia para profundidad de recursion cuando CachedConfig falla.
	 *     El path productivo resuelve via Config DA; este Settings entry es el fallback
	 *     determinista expuesto en Project Settings. Espeja el patron
	 *     de EmergencyHistoryFallback. Poner a 0 deshabilita el guard (la cola queda como red).
	 */
	UPROPERTY(config, EditAnywhere, Category = "Message|Diagnostics",
		meta = (DisplayName = "Emergency Max Broadcast Recursion Depth",
			ClampMin = "0", ClampMax = "32"))
	int32 EmergencyMaxBroadcastRecursionDepth = 4;

	/**
	 * EN: Enable verbose logging for config resolution.
	 * ES: Habilitar logging verboso para resolucion de config.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Message|Debug",
		meta = (DisplayName = "Verbose Config Resolution"))
	bool bVerboseConfigResolution = false;
};
