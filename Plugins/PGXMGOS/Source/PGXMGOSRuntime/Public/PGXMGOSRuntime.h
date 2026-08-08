// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPGXMGOS, Log, All);

/**
 * EN: PGXMGOS runtime module interface.
 *     GC Observability System — inference-based garbage collection monitoring,
 *     snapshot capture, behavioral profiling, and leak detection.
 *
 * ES: Interfaz del modulo runtime de PGXMGOS.
 *     Sistema de Observabilidad GC — monitoreo de garbage collection basado en inferencia,
 *     captura de snapshots, profiling comportamental y deteccion de leaks.
 */
class FPGXMGOSRuntimeModule : public IModuleInterface
{
public:
	/** EN: Called when the module is loaded into memory / ES: Se llama cuando el modulo se carga en memoria */
	void StartupModule() override;

	/** EN: Called when the module is unloaded from memory / ES: Se llama cuando el modulo se descarga de memoria */
	void ShutdownModule() override;
};
