// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXVersionControlTypes.h"

/**
 * EN: Text-scan based validator for PGX coding rules. Runs 5 heuristic rules
 *     against pending file contents. No AST — pure regex/string matching.
 *     Each rule can be toggled individually via UPGXVersionControlSettings.
 *
 * ES: Validador basado en text-scan para reglas de codigo PGX. Ejecuta 5 reglas
 *     heuristicas contra el contenido de archivos pendientes. Sin AST — puro regex/string.
 *     Cada regla se puede activar/desactivar via UPGXVersionControlSettings.
 */
class FPGXCommitValidator
{
public:
	/** EN: Run all enabled rules against the given file paths / ES: Ejecutar todas las reglas habilitadas */
	TArray<FPGXValidationIssue> Validate(const TArray<FString>& InFilePaths) const;

private:
	/** EN: [Partial] Shallow scan — check #include vs Build.cs / ES: [Partial] Scan superficial */
	void ValidateBuildCsDeps(const FString& InFilePath, const FString& InContent, TArray<FPGXValidationIssue>& OutIssues) const;

	/** EN: [Partial] Same-file — AddUObject/AddRaw without RemoveAll / ES: [Partial] Mismo archivo */
	void ValidateDelegateSymmetry(const FString& InFilePath, const FString& InContent, TArray<FPGXValidationIssue>& OutIssues) const;

	/** EN: [Operational] LogTemp in PGX modules / ES: [Operational] LogTemp en modulos PGX */
	void ValidateLogCategories(const FString& InFilePath, const FString& InContent, TArray<FPGXValidationIssue>& OutIssues) const;

	/** EN: [Partial] Naming convention check / ES: [Partial] Check de convenciones de nombre */
	void ValidateNamingConventions(const FString& InFilePath, const FString& InContent, TArray<FPGXValidationIssue>& OutIssues) const;

	/** EN: [Partial] UObject* without UPROPERTY / ES: [Partial] UObject* sin UPROPERTY */
	void ValidateUPropertyAnnotation(const FString& InFilePath, const FString& InContent, TArray<FPGXValidationIssue>& OutIssues) const;

	bool IsPGXSourceFile(const FString& InFilePath) const;
};
