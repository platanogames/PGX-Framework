// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Canonical log-category declaration/definition macros for PGX plugins.
//     Reduces boilerplate currently duplicated across 7 tooling plugins
//     (PGXDocs, PGXScaffold, PGXTutorials, PGXVersionControl,
//     PGXEditorTools) and progressively across the rest of
//     the framework. Each plugin declares its log category via these macros
//     instead of hand-writing DECLARE_LOG_CATEGORY_EXTERN(LogPGXXxx, Log, All).
//
// ES: Macros canonical para declaracion/definicion de categorias de log de
//     plugins PGX. Reduce boilerplate actualmente duplicado en 7 plugins del
//     cluster tooling y progresivamente en el resto del framework. Cada
//     plugin declara su log category via estas macros en lugar de escribir
//     a mano DECLARE_LOG_CATEGORY_EXTERN(LogPGXXxx, Log, All).
//
// Usage / Uso:
//   // En el header publico del modulo (.h):
//   PGX_DECLARE_PLUGIN_LOG(Docs);                       // LogPGXDocs (sin DLL export)
//   PGX_DECLARE_PLUGIN_LOG_API(PGXDOCS_API, Docs);      // LogPGXDocs (con DLL export)
//
//   // En el cpp del modulo:
//   PGX_DEFINE_PLUGIN_LOG(Docs);                        // -> DEFINE_LOG_CATEGORY(LogPGXDocs)
//
// Notes / Notas:
//   - Name argument is the suffix only ("Docs", "Scaffold"); the macro adds
//     the LogPGX prefix automatically — keeps log-category naming consistent.
//   - El argumento Name es solo el sufijo; el prefijo LogPGX lo anade la macro.
//   - Default verbosity is (Log, All). For non-default verbosities, plugins
//     should keep using DECLARE_LOG_CATEGORY_EXTERN directly.
//   - Verbosity default (Log, All). Para verbosities no-default seguir usando
//     DECLARE_LOG_CATEGORY_EXTERN directamente.

#pragma once

#include "Logging/LogMacros.h"

// EN: Forward declaration of a PGX plugin log category — no DLL export prefix.
//     Suitable for plugins whose log category is consumed only inside the same
//     module (most PGX plugins fall in this case).
// ES: Forward declaration de una categoria de log de plugin PGX — sin prefijo
//     de export DLL. Adecuado para plugins cuya log category se consume solo
//     dentro del mismo modulo (la mayoria de plugins PGX).
#define PGX_DECLARE_PLUGIN_LOG(Name) \
	DECLARE_LOG_CATEGORY_EXTERN(LogPGX##Name, Log, All)

// EN: Forward declaration of a PGX plugin log category — with DLL export prefix.
//     Use this variant when the log category must be visible across module
//     boundaries (e.g. editor module logging through a runtime category that
//     is referenced by other dependent modules). Pass the module's MODULENAME_API
//     macro as ApiPrefix (e.g. PGXDOCS_API, PGXCORERUNTIME_API).
// ES: Forward declaration de una categoria de log de plugin PGX — con prefijo
//     de export DLL. Usar esta variante cuando la log category debe ser visible
//     cruzando limites de modulos (por ejemplo modulo editor logueando a una
//     categoria runtime referenciada por otros modulos dependientes). Pasar la
//     macro MODULENAME_API del modulo como ApiPrefix.
#define PGX_DECLARE_PLUGIN_LOG_API(ApiPrefix, Name) \
	ApiPrefix DECLARE_LOG_CATEGORY_EXTERN(LogPGX##Name, Log, All)

// EN: Definition of a PGX plugin log category — pairs with PGX_DECLARE_PLUGIN_LOG
//     (or PGX_DECLARE_PLUGIN_LOG_API). Place in exactly one .cpp per declaration.
// ES: Definicion de una categoria de log de plugin PGX — emparejada con
//     PGX_DECLARE_PLUGIN_LOG (o PGX_DECLARE_PLUGIN_LOG_API). Colocar en
//     exactamente un .cpp por cada declaracion.
#define PGX_DEFINE_PLUGIN_LOG(Name) \
	DEFINE_LOG_CATEGORY(LogPGX##Name)
