// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PGXRegistryValidationTypes.generated.h"

// ═══════════════════════════════════════════════════════════════
// Enums / Enumeraciones
// ═══════════════════════════════════════════════════════════════

/**
 * EN: Severity level for validation issues.
 * ES: Nivel de severidad para problemas de validacion.
 */
UENUM()
enum class EPGXRegistryValidationSeverity : uint8
{
	Error   UMETA(DisplayName = "Error"),
	Warning UMETA(DisplayName = "Warning"),
	Info    UMETA(DisplayName = "Info")
};

/**
 * EN: Stable rule IDs for registry validation. These IDs are permanent
 *     and will not change across versions.
 * ES: IDs de regla estables para validacion del registro. Estos IDs son
 *     permanentes y no cambiaran entre versiones.
 *
 * Schema rules:        RDT001 - RDT003
 * Tag integrity:       RDT010 - RDT012
 * Duplicate/conflict:  RDT020 - RDT022
 * Asset references:    RDT030 - RDT033
 * Runtime quality:     RDT040 - RDT042
 * Policy/documentation:RDT050 - RDT051
 */
UENUM()
enum class EPGXValidationRuleId : uint8
{
	None = 0,

	// ─── Schema ───
	RDT001_InvalidRowStruct    = 1,   // EN: Invalid row struct type / ES: Tipo de row struct invalido
	RDT002_MissingCategoryTag  = 2,   // EN: Missing CategoryTag / ES: CategoryTag faltante
	RDT003_EmptyItemsByTag     = 3,   // EN: ItemsByTag is empty / ES: ItemsByTag esta vacio

	// ─── Tag Integrity ───
	RDT010_InvalidCategoryTag  = 10,  // EN: Invalid CategoryTag (not valid GameplayTag) / ES: CategoryTag invalido
	RDT011_InvalidItemTag      = 11,  // EN: Invalid ItemTag / ES: ItemTag invalido
	RDT012_TagOutsideNamespace = 12,  // EN: Tag root outside approved namespace / ES: Raiz de tag fuera de namespace aprobado

	// ─── Duplicate / Conflict ───
	RDT020_DuplicateCategoryRow    = 20, // EN: Duplicate category row in same table / ES: Fila de categoria duplicada
	RDT021_DuplicateItemKey        = 21, // EN: Duplicate item key in same category / ES: Clave de item duplicada
	RDT022_CrossTableConflict      = 22, // EN: Cross-table composite key conflict / ES: Conflicto de clave compuesta entre tablas

	// ─── Asset References ───
	RDT030_NullAssetRef        = 30,  // EN: Null asset reference / ES: Referencia a asset nula
	RDT031_BrokenSoftRef       = 31,  // EN: Broken soft reference path / ES: Ruta de ref soft rota
	RDT032_IncompatibleClass   = 32,  // EN: Asset class incompatible with contract / ES: Clase de asset incompatible
	RDT033_RedirectorDetected  = 33,  // EN: Redirector detected / ES: Redirector detectado

	// ─── Runtime Quality ───
	RDT040_CategoryBudgetExceeded  = 40, // EN: Category size exceeds budget / ES: Tamano de categoria excede presupuesto
	RDT041_TableBudgetExceeded     = 41, // EN: Table total exceeds budget / ES: Total de tabla excede presupuesto
	RDT042_HighHashCollision       = 42, // EN: High collision rate in buckets / ES: Alta tasa de colision

	// ─── Policy / Documentation ───
	RDT050_UndefinedConflictPolicy = 50, // EN: Conflict policy undefined / ES: Politica de conflicto indefinida
	RDT051_TableNotLinked          = 51  // EN: Table not linked to definition / ES: Tabla no vinculada a definicion
};

// ═══════════════════════════════════════════════════════════════
// Structs / Estructuras
// ═══════════════════════════════════════════════════════════════

/**
 * EN: A single validation issue found during registry validation.
 * ES: Un problema individual encontrado durante la validacion del registro.
 */
USTRUCT()
struct FPGXRegistryValidationIssue
{
	GENERATED_BODY()

	/** EN: Rule that detected this issue / ES: Regla que detecto este problema */
	EPGXValidationRuleId RuleId = EPGXValidationRuleId::None;

	/** EN: Severity level / ES: Nivel de severidad */
	EPGXRegistryValidationSeverity Severity = EPGXRegistryValidationSeverity::Error;

	/** EN: DataTable where the issue was found / ES: DataTable donde se encontro el problema */
	FString TableName;

	/** EN: Row name within the DataTable / ES: Nombre de fila dentro del DataTable */
	FName RowName;

	/** EN: Composite key string for context / ES: String de clave compuesta para contexto */
	FString KeyContext;

	/** EN: Asset path involved (if applicable) / ES: Ruta del asset involucrado (si aplica) */
	FString AssetPath;

	/** EN: Human-readable message / ES: Mensaje legible */
	FString Message;

	/** EN: Whether this issue can be auto-fixed / ES: Si este problema puede ser auto-corregido */
	bool bAutoFixable = false;

	// ─── Helpers ───

	FString GetRuleIdString() const
	{
		switch (RuleId)
		{
		case EPGXValidationRuleId::RDT001_InvalidRowStruct:       return TEXT("RDT001");
		case EPGXValidationRuleId::RDT002_MissingCategoryTag:     return TEXT("RDT002");
		case EPGXValidationRuleId::RDT003_EmptyItemsByTag:        return TEXT("RDT003");
		case EPGXValidationRuleId::RDT010_InvalidCategoryTag:     return TEXT("RDT010");
		case EPGXValidationRuleId::RDT011_InvalidItemTag:         return TEXT("RDT011");
		case EPGXValidationRuleId::RDT012_TagOutsideNamespace:    return TEXT("RDT012");
		case EPGXValidationRuleId::RDT020_DuplicateCategoryRow:   return TEXT("RDT020");
		case EPGXValidationRuleId::RDT021_DuplicateItemKey:       return TEXT("RDT021");
		case EPGXValidationRuleId::RDT022_CrossTableConflict:     return TEXT("RDT022");
		case EPGXValidationRuleId::RDT030_NullAssetRef:           return TEXT("RDT030");
		case EPGXValidationRuleId::RDT031_BrokenSoftRef:          return TEXT("RDT031");
		case EPGXValidationRuleId::RDT032_IncompatibleClass:      return TEXT("RDT032");
		case EPGXValidationRuleId::RDT033_RedirectorDetected:     return TEXT("RDT033");
		case EPGXValidationRuleId::RDT040_CategoryBudgetExceeded: return TEXT("RDT040");
		case EPGXValidationRuleId::RDT041_TableBudgetExceeded:    return TEXT("RDT041");
		case EPGXValidationRuleId::RDT042_HighHashCollision:      return TEXT("RDT042");
		case EPGXValidationRuleId::RDT050_UndefinedConflictPolicy:return TEXT("RDT050");
		case EPGXValidationRuleId::RDT051_TableNotLinked:         return TEXT("RDT051");
		default: return TEXT("UNKNOWN");
		}
	}

	FString GetSeverityString() const
	{
		switch (Severity)
		{
		case EPGXRegistryValidationSeverity::Error:   return TEXT("ERROR");
		case EPGXRegistryValidationSeverity::Warning: return TEXT("WARN");
		case EPGXRegistryValidationSeverity::Info:    return TEXT("INFO");
		default: return TEXT("?");
		}
	}
};
