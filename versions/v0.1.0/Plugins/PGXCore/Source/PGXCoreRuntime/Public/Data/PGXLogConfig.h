// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Data/PGXConfigDataAsset.h"
#include "Logging/PGXLogTypes.h"
#include "Observability/PGXObservable.h"
#include "Trace/PGXTraceTypes.h"
#include "PGXLogConfig.generated.h"

/**
 * EN: Config DataAsset for the PGX Log System (dashboard profile).
 *     Multiple profiles can exist for different contexts: Dev, QA, Shipping.
 *     The active profile is selected via UPGXConfigSubsystem and applied
 *     to UPGXLogSubsystem at initialization.
 *
 * ES: Config DataAsset para el Sistema de Logs PGX (perfil de dashboard).
 *     Multiples perfiles pueden existir para diferentes contextos: Dev, QA, Shipping.
 *     El perfil activo se selecciona via UPGXConfigSubsystem y se aplica
 *     a UPGXLogSubsystem en la inicializacion.
 *
 * Example profiles / Perfiles ejemplo:
 *     DA_LogConfig_Development  → All logs, screen on, persist JSON
 *     DA_LogConfig_QA           → Warning+, no screen, persist JSON+SAV
 *     DA_LogConfig_Shipping     → Error only, no screen, no persist
 */
UCLASS(BlueprintType, Blueprintable)
class PGXCORERUNTIME_API UPGXLogConfig : public UPGXConfigDataAsset, public IPGXObservable
{
	GENERATED_BODY()

public:
	//~ Begin IPGXObservable (delegates to PGXCoreObservability helpers)
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;
	//~ End IPGXObservable

	// ═══════════════════════════════════════
	// Buffer / Buffer
	// ═══════════════════════════════════════

	/** EN: Ring buffer capacity (entries) / ES: Capacidad del ring buffer (entradas) */
	UPROPERTY(EditDefaultsOnly, Category = "PGX|Log|Buffer",
		meta = (AdvancedDisplay, ClampMin = "256", ClampMax = "65536"))
	int32 RingBufferCapacity = 4096;

	// ═══════════════════════════════════════
	// Filtering / Filtrado
	// ═══════════════════════════════════════

	/** EN: Global minimum verbosity to capture / ES: Verbosity minima global para capturar */
	UPROPERTY(EditDefaultsOnly, Category = "PGX|Log|Filtering")
	EPGXLogVerbosity GlobalMinVerbosity = EPGXLogVerbosity::Verbose;

	/** EN: Per-category verbosity overrides / ES: Overrides de verbosity por categoria */
	UPROPERTY(EditDefaultsOnly, Category = "PGX|Log|Filtering", meta = (AdvancedDisplay))
	TMap<FName, EPGXLogVerbosity> CategoryOverrides;

	/**
	 * EN: Whitelist of categories (empty = all allowed).
	 *     If non-empty, only listed categories are captured.
	 * ES: Whitelist de categorias (vacio = todas permitidas).
	 *     Si no esta vacio, solo las categorias listadas se capturan.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "PGX|Log|Filtering", meta = (AdvancedDisplay))
	TArray<FName> CategoryWhitelist;

	// ═══════════════════════════════════════
	// Screen Display / Visualizacion en Pantalla
	// ═══════════════════════════════════════

	/** EN: Show logs on screen (dev only) / ES: Mostrar logs en pantalla (solo dev) */
	UPROPERTY(EditDefaultsOnly, Category = "PGX|Log|Screen")
	bool bShowOnScreen = true;

	/** EN: Minimum verbosity for screen display / ES: Verbosity minima para mostrar en pantalla */
	UPROPERTY(EditDefaultsOnly, Category = "PGX|Log|Screen",
		meta = (AdvancedDisplay, EditCondition = "bShowOnScreen"))
	EPGXLogVerbosity ScreenMinVerbosity = EPGXLogVerbosity::Warning;

	/** EN: Screen display duration in seconds / ES: Duracion en pantalla (segundos) */
	UPROPERTY(EditDefaultsOnly, Category = "PGX|Log|Screen",
		meta = (AdvancedDisplay, EditCondition = "bShowOnScreen", ClampMin = "0.5", ClampMax = "30.0"))
	float ScreenDisplayDuration = 5.0f;

	/**
	 * EN: Color overrides per verbosity. Empty = use defaults.
	 *     Defaults: Verbose=#808080, Debug=#00C8C8, Info=#FFFFFF,
	 *     Warning=#FFFF00, Error=#FF5050, Fatal=#FF00FF
	 * ES: Colores por verbosity. Vacio = usar defaults.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "PGX|Log|Screen",
		meta = (AdvancedDisplay, EditCondition = "bShowOnScreen"))
	TMap<EPGXLogVerbosity, FLinearColor> VerbosityColors;

	// ═══════════════════════════════════════
	// Persistence / Persistencia
	// ═══════════════════════════════════════

	/** EN: Auto-export to JSON at session end / ES: Auto-exportar a JSON al final de sesion */
	UPROPERTY(EditDefaultsOnly, Category = "PGX|Log|Persistence")
	bool bExportJsonOnSessionEnd = false;

	/** EN: JSON export directory (relative to Saved/) / ES: Directorio de exportacion JSON (relativo a Saved/) */
	UPROPERTY(EditDefaultsOnly, Category = "PGX|Log|Persistence",
		meta = (AdvancedDisplay, EditCondition = "bExportJsonOnSessionEnd"))
	FString JsonExportDirectory = TEXT("Logs/PGX");

	/** EN: Auto-save to .sav for runtime widget / ES: Auto-guardar a .sav para widget runtime */
	UPROPERTY(EditDefaultsOnly, Category = "PGX|Log|Persistence", meta = (AdvancedDisplay))
	bool bAutoSaveToSlot = false;

	/** EN: Save interval in seconds (0 = only on session end) / ES: Intervalo de guardado (0 = solo al final) */
	UPROPERTY(EditDefaultsOnly, Category = "PGX|Log|Persistence",
		meta = (AdvancedDisplay, EditCondition = "bAutoSaveToSlot", ClampMin = "0.0"))
	float SaveIntervalSeconds = 0.0f;

	/** EN: Maximum JSON files to keep (0 = unlimited) / ES: Maximo de archivos JSON a mantener (0 = ilimitado) */
	UPROPERTY(EditDefaultsOnly, Category = "PGX|Log|Persistence",
		meta = (AdvancedDisplay, ClampMin = "0"))
	int32 MaxJsonFiles = 20;

	// ═══════════════════════════════════════
	// Delegates / Delegados
	// ═══════════════════════════════════════

	/** EN: Broadcast delegate on every log entry / ES: Emitir delegado en cada entrada de log */
	UPROPERTY(EditDefaultsOnly, Category = "PGX|Log|Events", meta = (AdvancedDisplay))
	bool bBroadcastOnEveryEntry = true;

	/** EN: Minimum verbosity to broadcast / ES: Verbosity minima para broadcast */
	UPROPERTY(EditDefaultsOnly, Category = "PGX|Log|Events",
		meta = (AdvancedDisplay, EditCondition = "bBroadcastOnEveryEntry"))
	EPGXLogVerbosity BroadcastMinVerbosity = EPGXLogVerbosity::Info;

	// ═══════════════════════════════════════
	// Traceability / Trazabilidad
	// ═══════════════════════════════════════

	/** EN: Trace configuration for the Log system. Controls automatic C++ tracing of log operations. / ES: Configuracion de traza para el sistema de Log. Controla trazado automatico de operaciones de log en C++. */
	UPROPERTY(EditDefaultsOnly, Category = "PGX|Log|Trace", meta = (AdvancedDisplay))
	FPGXTraceConfig TraceConfig;
};
