// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Core/PGXScaffoldTypes.h"

/**
 * EN: Dry-run validator. Checks all items BEFORE execution to detect errors.
 *     Validation is mandatory — execution cannot proceed if HasErrors() returns true.
 *
 * ES: Validador dry-run. Verifica todos los items ANTES de ejecucion para detectar errores.
 *     Validacion es obligatoria — la ejecucion no puede proceder si HasErrors() retorna true.
 */
class PGXSCAFFOLDEDITOR_API FPGXScaffoldValidator
{
public:
	/** EN: Validate selected items against project state / ES: Validar items seleccionados contra estado del proyecto */
	TArray<FPGXScaffoldValidationResult> Validate(
		const TArray<FPGXScaffoldTemplateItem>& Items,
		const TMap<FString, FString>& Variables,
		const FPGXScaffoldProjectInfo& ProjectInfo);

	/** EN: Check if last validation had blocking errors / ES: Verificar si la ultima validacion tuvo errores bloqueantes */
	bool HasErrors() const { return bHasErrors; }

private:
	bool bHasErrors = false;

	void ValidatePaths(const TArray<FPGXScaffoldTemplateItem>& Items, const TMap<FString, FString>& Variables,
		const FPGXScaffoldProjectInfo& ProjectInfo, TArray<FPGXScaffoldValidationResult>& OutResults);

	void ValidateDuplicates(const TArray<FPGXScaffoldTemplateItem>& Items, const TMap<FString, FString>& Variables,
		TArray<FPGXScaffoldValidationResult>& OutResults);

	void ValidateDependencies(const TArray<FPGXScaffoldTemplateItem>& Items,
		TArray<FPGXScaffoldValidationResult>& OutResults);

	void ValidateVariables(const TArray<FPGXScaffoldTemplateItem>& Items, const TMap<FString, FString>& Variables,
		TArray<FPGXScaffoldValidationResult>& OutResults);

	/** EN: Resolve variables in a path string / ES: Resolver variables en un string de ruta */
	static FString ResolveVariables(const FString& Path, const TMap<FString, FString>& Variables);
};
