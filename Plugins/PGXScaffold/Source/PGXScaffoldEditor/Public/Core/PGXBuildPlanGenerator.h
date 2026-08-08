// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Core/PGXScaffoldTypes.h"

/**
 * EN: Generates an immutable build plan from validated items.
 *     Performs topological sort: folders first, then assets, then blueprints.
 *
 * ES: Genera un plan de build inmutable a partir de items validados.
 *     Realiza ordenamiento topologico: carpetas primero, luego assets, luego blueprints.
 */
class PGXSCAFFOLDEDITOR_API FPGXBuildPlanGenerator
{
public:
	/**
	 * EN: Generate build plan from selected items + variables.
	 *     ContentDir is the project's /Game/ root on disk.
	 * ES: Generar plan desde items seleccionados + variables.
	 *     ContentDir es la raiz /Game/ del proyecto en disco.
	 */
	FPGXScaffoldBuildPlan Generate(
		const TArray<FPGXScaffoldTemplateItem>& Items,
		const TMap<FString, FString>& Variables,
		const FString& ContentDir,
		FName TemplateId);

private:
	/** EN: Resolve variables in path / ES: Resolver variables en ruta */
	static FString ResolveVariables(const FString& Path, const TMap<FString, FString>& Variables);
};
