// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "Modules/ModuleManager.h"

// EN: Centralized PGX Framework version constants — single source of truth.
// ES: Constantes centralizadas de version del Framework PGX — unica fuente de verdad.
namespace PGXVersion
{
	inline constexpr int32 Major = 0;
	inline constexpr int32 Minor = 4;
	inline constexpr int32 Patch = 0;
	inline constexpr const TCHAR* String = TEXT("0.4.0");
	inline constexpr int32 PluginCount = 23;
}

/**
 * EN: PGXCoreRuntime module interface.
 *     This is the foundational runtime module of the PGX framework.
 *     It provides base types, interfaces, and core systems that all
 *     other PGX plugins depend on.
 *
 * ES: Interfaz del modulo PGXCoreRuntime.
 *     Este es el modulo runtime fundacional del framework PGX.
 *     Proporciona tipos base, interfaces y sistemas centrales de los
 *     que dependen todos los demas plugins PGX.
 */
class FPGXCoreRuntimeModule : public IModuleInterface
{
public:
	/** EN: Called when the module is loaded into memory / ES: Se llama cuando el modulo se carga en memoria */
	void StartupModule() override;

	/** EN: Called when the module is unloaded from memory / ES: Se llama cuando el modulo se descarga de memoria */
	void ShutdownModule() override;
};
