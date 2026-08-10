// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXVersionControlTypes.generated.h"

/**
 * EN: Severity level for validation issues.
 * ES: Nivel de severidad para issues de validacion.
 */
UENUM(BlueprintType)
enum class EPGXValidationSeverity : uint8
{
	Info     UMETA(DisplayName = "Info"),
	Warning  UMETA(DisplayName = "Warning"),
	Error    UMETA(DisplayName = "Error")
};

/**
 * EN: How the pre-commit validation behaves.
 *     ErrorBlocks: Errors prevent commit. WarningOnly: Show warnings, allow commit. InfoOnly: Log only.
 * ES: Como se comporta la validacion pre-commit.
 *     ErrorBlocks: Errores bloquean commit. WarningOnly: Muestra warnings, permite commit. InfoOnly: Solo log.
 */
UENUM(BlueprintType)
enum class EPGXValidationMode : uint8
{
	ErrorBlocks  UMETA(DisplayName = "Error Blocks Commit"),
	WarningOnly  UMETA(DisplayName = "Warning Only"),
	InfoOnly     UMETA(DisplayName = "Info Only")
};

/**
 * EN: Confidence level for text-scan validation results.
 *     Operational checks are direct string findings; heuristic checks are
 *     useful lint hints and must not be presented as authoritative AST proof.
 * ES: Nivel de confianza para resultados de validacion por text-scan.
 */
UENUM(BlueprintType)
enum class EPGXValidationConfidence : uint8
{
	Operational      UMETA(DisplayName = "Operational"),
	HeuristicPartial UMETA(DisplayName = "Heuristic Partial")
};

/**
 * EN: Typed status for PGX Version Control local operations.
 * ES: Estado tipado para operaciones locales de PGX Version Control.
 */
UENUM(BlueprintType)
enum class EPGXVersionControlOperationStatus : uint8
{
	Success                  UMETA(DisplayName = "Success"),
	StoreUnavailable         UMETA(DisplayName = "Store Unavailable"),
	InvalidChangelist        UMETA(DisplayName = "Invalid Changelist"),
	DefaultChangelistProtected UMETA(DisplayName = "Default Changelist Protected"),
	EmptyName                UMETA(DisplayName = "Empty Name"),
	EmptyFilePath            UMETA(DisplayName = "Empty File Path"),
	TempWriteFailed          UMETA(DisplayName = "Temporary Write Failed"),
	BackupCreateFailed       UMETA(DisplayName = "Backup Create Failed"),
	AtomicReplaceFailed      UMETA(DisplayName = "Atomic Replace Failed")
};

/**
 * EN: A single validation issue found during pre-commit analysis.
 * ES: Un issue de validacion encontrado durante el analisis pre-commit.
 */
USTRUCT(BlueprintType)
struct FPGXValidationIssue
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|VersionControl")
	EPGXValidationSeverity Severity = EPGXValidationSeverity::Info;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|VersionControl")
	EPGXValidationConfidence Confidence = EPGXValidationConfidence::HeuristicPartial;

	/** EN: Rule identifier (e.g. "BuildCs.MissingDep") / ES: Identificador de regla */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|VersionControl")
	FString RuleId;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|VersionControl")
	FString Message;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|VersionControl")
	FString FilePath;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|VersionControl")
	int32 LineNumber = INDEX_NONE;
};

/**
 * EN: Typed local-operation result with a short diagnostic message.
 * ES: Resultado tipado de operacion local con diagnostico breve.
 */
USTRUCT(BlueprintType)
struct FPGXVersionControlOperationResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|VersionControl")
	EPGXVersionControlOperationStatus Status = EPGXVersionControlOperationStatus::Success;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|VersionControl")
	FString Message;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|VersionControl")
	FString ContextPath;

	bool IsSuccess() const
	{
		return Status == EPGXVersionControlOperationStatus::Success;
	}
};

/**
 * EN: A logical grouping of file changes, similar to Perforce changelists.
 *     Persisted to JSON. Files can be moved between changelists before committing.
 * ES: Agrupacion logica de cambios de archivos, similar a changelists de Perforce.
 *     Persistido a JSON. Los archivos pueden moverse entre changelists antes de commit.
 */
USTRUCT(BlueprintType)
struct FPGXChangelist
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|VersionControl")
	FGuid Guid;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|VersionControl")
	FString DisplayName;

	/** EN: Draft commit message / ES: Borrador de mensaje de commit */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|VersionControl")
	FString Description;

	/** EN: Absolute file paths / ES: Rutas absolutas de archivos */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|VersionControl")
	TArray<FString> FilePaths;

	/** EN: Default CL cannot be deleted / ES: CL por defecto no se puede borrar */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|VersionControl")
	bool bIsDefault = false;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|VersionControl")
	FDateTime CreatedAt;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|VersionControl")
	FDateTime ModifiedAt;
};

/** EN: Broadcast when changelists are created/deleted/modified / ES: Broadcast cuando changelists son creados/borrados/modificados */
DECLARE_MULTICAST_DELEGATE(FOnPGXChangelistsChanged);
