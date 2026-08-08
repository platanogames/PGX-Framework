// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Validation/PGXRegistryValidationTypes.h"

class UDataTable;
class UPGXRegistryDefinition;
class UPGXRegistrySettings;

/**
 * EN: Rules engine for PGX Registry validation. Implements 18 stable rules
 *     (RDT001-RDT051) that check schema, tags, duplicates, asset refs,
 *     runtime quality, and policy compliance.
 *     Non-UObject — instantiated on demand by tab/commandlet.
 *
 * ES: Motor de reglas para validacion del PGX Registry. Implementa 18 reglas
 *     estables (RDT001-RDT051) que verifican esquema, tags, duplicados, refs de assets,
 *     calidad runtime y cumplimiento de politicas.
 *     No-UObject — instanciado bajo demanda por tab/commandlet.
 */
class PGXREGISTRYEDITOR_API FPGXRegistryValidationService
{
public:
	/**
	 * EN: Validate a single DataTable against all applicable rules.
	 * ES: Validar un DataTable individual contra todas las reglas aplicables.
	 */
	int32 ValidateTable(const UDataTable* Table, const UPGXRegistryDefinition* Definition,
		TArray<FPGXRegistryValidationIssue>& OutIssues) const;

	/**
	 * EN: Validate all DataTables across all known definitions.
	 * ES: Validar todos los DataTables en todas las definiciones conocidas.
	 */
	int32 ValidateAll(TArray<FPGXRegistryValidationIssue>& OutIssues) const;

	/**
	 * EN: Get the total error count from a list of issues.
	 * ES: Obtener el conteo total de errores de una lista de problemas.
	 */
	static int32 CountErrors(const TArray<FPGXRegistryValidationIssue>& Issues);

	/**
	 * EN: Get the total warning count from a list of issues.
	 * ES: Obtener el conteo total de warnings de una lista de problemas.
	 */
	static int32 CountWarnings(const TArray<FPGXRegistryValidationIssue>& Issues);

private:
	// ─── Individual Rule Implementations ───

	// Schema
	void RunRDT001_InvalidRowStruct(const UDataTable* Table, TArray<FPGXRegistryValidationIssue>& OutIssues) const;
	void RunRDT002_MissingCategoryTag(const UDataTable* Table, TArray<FPGXRegistryValidationIssue>& OutIssues) const;
	void RunRDT003_EmptyItemsByTag(const UDataTable* Table, TArray<FPGXRegistryValidationIssue>& OutIssues) const;

	// Tag Integrity
	void RunRDT010_InvalidCategoryTag(const UDataTable* Table, TArray<FPGXRegistryValidationIssue>& OutIssues) const;
	void RunRDT011_InvalidItemTag(const UDataTable* Table, TArray<FPGXRegistryValidationIssue>& OutIssues) const;
	void RunRDT012_TagOutsideNamespace(const UDataTable* Table, const UPGXRegistrySettings* Settings, TArray<FPGXRegistryValidationIssue>& OutIssues) const;

	// Duplicate / Conflict
	void RunRDT020_DuplicateCategoryRow(const UDataTable* Table, TArray<FPGXRegistryValidationIssue>& OutIssues) const;
	void RunRDT021_DuplicateItemKey(const UDataTable* Table, TArray<FPGXRegistryValidationIssue>& OutIssues) const;
	void RunRDT022_CrossTableConflict(const TArray<const UDataTable*>& Tables, const UPGXRegistryDefinition* Definition, TArray<FPGXRegistryValidationIssue>& OutIssues) const;

	// Asset References
	void RunRDT030_NullAssetRef(const UDataTable* Table, TArray<FPGXRegistryValidationIssue>& OutIssues) const;
	void RunRDT031_BrokenSoftRef(const UDataTable* Table, TArray<FPGXRegistryValidationIssue>& OutIssues) const;
	void RunRDT032_IncompatibleClass(const UDataTable* Table, const UPGXRegistryDefinition* Definition, TArray<FPGXRegistryValidationIssue>& OutIssues) const;
	void RunRDT033_RedirectorDetected(const UDataTable* Table, TArray<FPGXRegistryValidationIssue>& OutIssues) const;

	// Runtime Quality
	void RunRDT040_CategoryBudgetExceeded(const UDataTable* Table, const UPGXRegistrySettings* Settings, TArray<FPGXRegistryValidationIssue>& OutIssues) const;
	void RunRDT041_TableBudgetExceeded(const UDataTable* Table, const UPGXRegistrySettings* Settings, TArray<FPGXRegistryValidationIssue>& OutIssues) const;

	// Policy / Documentation
	void RunRDT050_UndefinedConflictPolicy(const UPGXRegistryDefinition* Definition, TArray<FPGXRegistryValidationIssue>& OutIssues) const;
	void RunRDT051_TableNotLinked(const UDataTable* Table, TArray<FPGXRegistryValidationIssue>& OutIssues) const;

	// ─── Helpers ───

	/** EN: Build issue struct with common fields / ES: Construir struct de issue con campos comunes */
	static FPGXRegistryValidationIssue MakeIssue(EPGXValidationRuleId RuleId, EPGXRegistryValidationSeverity Severity,
		const FString& TableName, FName RowName, const FString& Message, bool bAutoFixable = false);
};
