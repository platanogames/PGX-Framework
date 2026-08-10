// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Onboarding tutorial step generators (T1-T5).
// ES: Generadores de pasos de tutoriales de onboarding (T1-T5).
#pragma once

#include "CoreMinimal.h"
#include "PGXTutorialTypes.h"

/**
 * EN: Provides step arrays for the 5 onboarding tutorials.
 *     Each function takes a language parameter and returns localized steps.
 * ES: Provee arrays de pasos para los 5 tutoriales de onboarding.
 *     Cada funcion toma un parametro de idioma y retorna pasos localizados.
 */
namespace PGXOnboardingTutorials
{
	/** T1: What is PGX? — Complete editor tour (11 steps) */
	TArray<FPGXTutorialStep> GetT1_WhatIsPGX(EPGXTutorialLanguage Lang);

	/** T2: Zero-Config Game Loop (10 steps) */
	TArray<FPGXTutorialStep> GetT2_ZeroConfigGameLoop(EPGXTutorialLanguage Lang);

	/** T3: Data-Driven Workflow (8 steps) */
	TArray<FPGXTutorialStep> GetT3_DataDrivenWorkflow(EPGXTutorialLanguage Lang);

	/** T4: Full Observability (9 steps) */
	TArray<FPGXTutorialStep> GetT4_FullObservability(EPGXTutorialLanguage Lang);

	/** T5: Decoupled Architecture (8 steps) */
	TArray<FPGXTutorialStep> GetT5_DecoupledArchitecture(EPGXTutorialLanguage Lang);
}
