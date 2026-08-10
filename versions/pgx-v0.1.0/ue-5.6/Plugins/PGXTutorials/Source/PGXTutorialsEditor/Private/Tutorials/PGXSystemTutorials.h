// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Per-system tutorial step generators (S1-S13).
// ES: Generadores de pasos de tutoriales por sistema (S1-S13).
#pragma once

#include "CoreMinimal.h"
#include "PGXTutorialTypes.h"

/**
 * EN: Provides step arrays for the 13 per-system tutorials.
 *     Each function takes a language parameter and returns localized steps.
 * ES: Provee arrays de pasos para los 13 tutoriales por sistema.
 *     Cada funcion toma un parametro de idioma y retorna pasos localizados.
 */
namespace PGXSystemTutorials
{
	/** S1: Profile System (10 steps) */
	TArray<FPGXTutorialStep> GetS1_Profile(EPGXTutorialLanguage Lang);

	/** S2: GameFlow System (10 steps) */
	TArray<FPGXTutorialStep> GetS2_GameFlow(EPGXTutorialLanguage Lang);

	/** S3: Save System (10 steps) */
	TArray<FPGXTutorialStep> GetS3_Save(EPGXTutorialLanguage Lang);

	/** S4: PSO System (10 steps) */
	TArray<FPGXTutorialStep> GetS4_PSO(EPGXTutorialLanguage Lang);

	/** S5: Loading System (10 steps) */
	TArray<FPGXTutorialStep> GetS5_Loading(EPGXTutorialLanguage Lang);

	/** S6: LevelFlow System (9 steps) */
	TArray<FPGXTutorialStep> GetS6_LevelFlow(EPGXTutorialLanguage Lang);

	/** S7: Audio System (12 steps) */
	TArray<FPGXTutorialStep> GetS7_Audio(EPGXTutorialLanguage Lang);

	/** S8: Log System (10 steps) */
	TArray<FPGXTutorialStep> GetS8_Log(EPGXTutorialLanguage Lang);

	/** S9: Data Registry (10 steps) */
	TArray<FPGXTutorialStep> GetS9_DataRegistry(EPGXTutorialLanguage Lang);

	/** S10: Construction System (9 steps) */
	TArray<FPGXTutorialStep> GetS10_Construction(EPGXTutorialLanguage Lang);

	/** S11: Message System (10 steps) */
	TArray<FPGXTutorialStep> GetS11_Message(EPGXTutorialLanguage Lang);

	/** S12: EventHandler System (10 steps) */
	TArray<FPGXTutorialStep> GetS12_EventHandler(EPGXTutorialLanguage Lang);

	/** S13: MGOS System (10 steps) */
	TArray<FPGXTutorialStep> GetS13_MGOS(EPGXTutorialLanguage Lang);

	/** S14: Trade System (11 steps) */
	TArray<FPGXTutorialStep> GetS14_Trade(EPGXTutorialLanguage Lang);

	/** S15: Crafting System (9 steps) */
	TArray<FPGXTutorialStep> GetS15_Crafting(EPGXTutorialLanguage Lang);
}
