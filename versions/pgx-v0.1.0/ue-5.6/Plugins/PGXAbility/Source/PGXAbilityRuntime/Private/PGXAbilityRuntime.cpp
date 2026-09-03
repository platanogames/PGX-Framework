// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "PGXAbilityRuntime.h"
#include "Subsystems/PGXLogSubsystem.h"

PGX_DEFINE_LOG_CATEGORY(LogPGXAbility);

/**
 * EN: StartupModule - Called when the PGXAbilityRuntime module is loaded.
 *     Logs initialization of the PGX Ability facade layer.
 * ES: StartupModule - Se llama cuando el modulo PGXAbilityRuntime se carga.
 *     Registra la inicializacion de la capa facade de PGX Ability.
 */
void FPGXAbilityRuntimeModule::StartupModule()
{
	PGX_LOG_INFO(LogPGXAbility, TEXT("PGXAbilityRuntime module started. GAS facade layer initialized."));
}

/**
 * EN: ShutdownModule - Called when the PGXAbilityRuntime module is unloaded.
 *     Logs shutdown of the PGX Ability facade layer.
 * ES: ShutdownModule - Se llama cuando el modulo PGXAbilityRuntime se descarga.
 *     Registra el shutdown de la capa facade de PGX Ability.
 */
void FPGXAbilityRuntimeModule::ShutdownModule()
{
	PGX_LOG_INFO(LogPGXAbility, TEXT("PGXAbilityRuntime module shut down. GAS facade layer deinitialized."));
}

IMPLEMENT_MODULE(FPGXAbilityRuntimeModule, PGXAbilityRuntime)
