// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Core/PGXScaffoldTypes.h"

/**
 * EN: Analyzes the current project to detect type, modules, plugins, and existing structure.
 *     Result is cached and reused across the scaffold pipeline.
 *
 * ES: Analiza el proyecto actual para detectar tipo, modulos, plugins, y estructura existente.
 *     El resultado se cachea y se reutiliza en el pipeline de scaffold.
 */
class PGXSCAFFOLDEDITOR_API FPGXProjectAnalyzer
{
public:
	/** EN: Perform full project analysis / ES: Realizar analisis completo del proyecto */
	FPGXScaffoldProjectInfo Analyze();

	/** EN: Get cached result (call Analyze() first) / ES: Obtener resultado cacheado (llamar Analyze() primero) */
	const FPGXScaffoldProjectInfo& GetCachedInfo() const { return CachedInfo; }

	/** EN: Invalidate cache / ES: Invalidar cache */
	void InvalidateCache() { bCacheValid = false; }

private:
	FPGXScaffoldProjectInfo CachedInfo;
	bool bCacheValid = false;

	void DetectProjectType(FPGXScaffoldProjectInfo& OutInfo);
	void CollectModules(FPGXScaffoldProjectInfo& OutInfo);
	void CollectPlugins(FPGXScaffoldProjectInfo& OutInfo);
	void CollectExistingStructure(FPGXScaffoldProjectInfo& OutInfo);
};
