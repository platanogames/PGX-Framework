// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Logging/PGXLogTypes.h"

// ═══════════════════════════════════════════════════════════════
// EN: Static helper for mapping between PGX Log concepts and GameplayTags/colors.
//     Used by: Builder (auto-tag), Editor (display), any system needing the mapping.
//     Single source of truth for Verbosity↔Tag, Category↔Context Tag, Verbosity↔Color.
// ES: Helper estatico para mapeos entre conceptos del Log PGX y GameplayTags/colores.
//     Usado por: Builder (auto-tag), Editor (display), cualquier sistema que necesite el mapeo.
//     Fuente unica de verdad para Verbosity↔Tag, Category↔Tag Contexto, Verbosity↔Color.
// ═══════════════════════════════════════════════════════════════

class PGXCORERUNTIME_API FPGXLogTagHelper
{
public:
	/**
	 * EN: Get the GameplayTag corresponding to a verbosity level.
	 *     e.g. EPGXLogVerbosity::Warning → TAG_PGX_Log_Warning ("PGX.Log.Warning")
	 * ES: Obtener el GameplayTag correspondiente a un nivel de verbosity.
	 */
	static FGameplayTag GetVerbosityTag(EPGXLogVerbosity Verbosity);

	/**
	 * EN: Get the context GameplayTag for a log category name.
	 *     e.g. "LogPGXSave" → TAG_PGX_Log_Context_IO, "LogPGX" → TAG_PGX_Log_Context_Core
	 *     Returns empty tag if no mapping found.
	 * ES: Obtener el GameplayTag de contexto para un nombre de categoria de log.
	 *     Retorna tag vacio si no se encuentra mapeo.
	 */
	static FGameplayTag GetContextTag(FName Category);

	/**
	 * EN: Get the display color for a verbosity level.
	 *     Used by editor widgets for consistent color-coding.
	 * ES: Obtener el color de display para un nivel de verbosity.
	 *     Usado por widgets de editor para codificacion de color consistente.
	 */
	static FLinearColor GetVerbosityColor(EPGXLogVerbosity Verbosity);

	/**
	 * EN: Auto-populate tags on an entry based on its Verbosity and Category.
	 *     Adds verbosity tag + context tag to the entry's Tags container.
	 *     Does NOT clear existing tags — appends to any developer-set tags.
	 * ES: Auto-popular tags en una entrada basado en su Verbosity y Category.
	 *     Agrega tag de verbosity + tag de contexto al contenedor Tags de la entrada.
	 *     NO limpia tags existentes — agrega a cualquier tag establecido por el developer.
	 */
	static void AutoPopulateTags(FPGXLogEntry& Entry);

	/**
	 * EN: Auto-infer a domain tag from a log category name.
	 *     e.g. "LogPGXAudio" → PGX.Log.Domain.Audio, "LogPGXSave" → PGX.Log.Domain.Save
	 *     Returns empty tag if no mapping found.
	 * ES: Auto-inferir un tag de dominio desde un nombre de categoria de log.
	 *     Retorna tag vacio si no se encuentra mapeo.
	 */
	static FGameplayTag AutoInferDomainTag(FName Category);

	/**
	 * EN: Get the display color for a domain tag. Uses built-in color map.
	 *     Fallback: gray if domain unknown.
	 * ES: Obtener el color de display para un tag de dominio. Usa mapa de colores built-in.
	 */
	static FLinearColor GetDomainColor(const FGameplayTag& DomainTag);
};
