// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"

/**
 * EN: Static validator that runs on editor startup to check workflow mode vs DA assignment.
 *     Scheduled with a 3s delay so Slate notifications are available.
 * ES: Validador estatico que corre al iniciar el editor para verificar workflow mode vs asignacion de DA.
 *     Programado con 3s de delay para que las notificaciones Slate esten disponibles.
 */
class FPGXConstructionStartupValidator
{
public:
	/** EN: Schedule validation with delay (call from StartupModule) / ES: Programar validacion con delay */
	static void ScheduleValidation();

private:
	/** EN: Execute the actual validation / ES: Ejecutar la validacion */
	static void RunValidation();
};
