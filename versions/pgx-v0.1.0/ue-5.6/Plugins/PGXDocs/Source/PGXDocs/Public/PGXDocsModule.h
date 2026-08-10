// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "Modules/ModuleManager.h"

// EN: Dedicated log category for PGXDocs (S7 compliance)
// ES: Categoria de log dedicada para PGXDocs (cumplimiento S7)
PGXDOCS_API DECLARE_LOG_CATEGORY_EXTERN(LogPGXDocs, Log, All);

/**
 * EN: PGXDocs shared module. Provides core types, settings, registry,
 *     search index, validator, and link resolver for documentation.
 *     This module is loaded in both Editor and Runtime contexts.
 *
 * ES: Modulo compartido PGXDocs. Proporciona tipos core, settings, registry,
 *     indice de busqueda, validador y resolutor de links para documentacion.
 *     Este modulo se carga en contextos Editor y Runtime.
 */
class FPGXDocsModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};
