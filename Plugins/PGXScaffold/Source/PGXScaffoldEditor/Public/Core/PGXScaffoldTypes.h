// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Misc/Guid.h"

// ============================================================================
// EN: PGXScaffold type definitions — enums, structs, delegates
// ES: Definiciones de tipos PGXScaffold — enums, structs, delegados
// ============================================================================

// ─── Enums ───

/** EN: Detected project type / ES: Tipo de proyecto detectado */
enum class EPGXProjectType : uint8
{
	BlueprintOnly,
	CppProject,
	Plugin
};

/** EN: Action to perform for a scaffold item / ES: Accion a realizar para un item de scaffold */
enum class EPGXScaffoldActionType : uint8
{
	CreateFolder,
	CreateDataAsset,
	CreateBlueprint,
	// Phase 2:
	// CreateCppHeader,
	// CreateCppSource,
	// ModifyBuildCs,
	// PostAction
};

/** EN: Validation severity / ES: Severidad de validacion */
enum class EPGXScaffoldSeverity : uint8
{
	Info,
	Warning,
	Error,   // Blocks execution
	Fatal
};

/** EN: Individual step execution status / ES: Estado de ejecucion de un paso individual */
enum class EPGXScaffoldStepStatus : uint8
{
	Pending,
	InProgress,
	Completed,
	Failed,
	Skipped
};

/** EN: Overall operation pipeline status / ES: Estado general del pipeline de operacion */
enum class EPGXScaffoldOperationStatus : uint8
{
	Idle,
	Analyzing,
	Validating,
	Planning,
	Executing,
	Completed,
	Failed
};

// ─── Structs ───

/**
 * EN: Single item within a scaffold template (folder, DA, BP, etc.)
 * ES: Item individual dentro de un template de scaffold (carpeta, DA, BP, etc.)
 */
struct FPGXScaffoldTemplateItem
{
	/** EN: Unique item ID within template / ES: ID unico del item dentro del template */
	FName ItemId;

	/** EN: Display name in tree view / ES: Nombre en el arbol */
	FText DisplayName;

	/** EN: Action type / ES: Tipo de accion */
	EPGXScaffoldActionType ActionType = EPGXScaffoldActionType::CreateFolder;

	/** EN: Path relative to template root (supports {Variables}) / ES: Ruta relativa a la raiz del template (soporta {Variables}) */
	FString RelativePath;

	/** EN: For CreateDataAsset: UClass name / ES: Para CreateDataAsset: nombre de UClass */
	FName AssetClassName;

	/** EN: Parent item (tree hierarchy) / ES: Item padre (jerarquia del arbol) */
	FName ParentItemId;

	/** EN: Items that must be created first / ES: Items que deben crearse primero */
	TArray<FName> DependsOn;

	/** EN: Order within siblings (lower = first) / ES: Orden entre hermanos (menor = primero) */
	int32 ExecutionOrder = 0;

	/** EN: For CreateBlueprint: parent class path / ES: Para CreateBlueprint: ruta de la clase padre */
	FString ParentClassPath;
};

/**
 * EN: Complete scaffold template (collection of items + variables)
 * ES: Template de scaffold completo (coleccion de items + variables)
 */
struct FPGXScaffoldTemplate
{
	/** EN: Unique template ID / ES: ID unico del template */
	FName TemplateId;

	/** EN: Display name / ES: Nombre para mostrar */
	FText DisplayName;

	/** EN: Template description / ES: Descripcion del template */
	FText Description;

	/** EN: Category for grouping / ES: Categoria para agrupacion */
	FName Category;

	/** EN: All items in this template / ES: Todos los items de este template */
	TArray<FPGXScaffoldTemplateItem> Items;

	/** EN: Variable definitions with defaults ({VariableName} -> DefaultValue) / ES: Definiciones de variables con valores por defecto */
	TMap<FString, FString> DefaultVariables;

	/** EN: Variable display names / ES: Nombres de display para variables */
	TMap<FString, FText> VariableDisplayNames;

	int32 GetItemCount() const { return Items.Num(); }
};

/**
 * EN: Snapshot of the current project state (cached during analysis)
 * ES: Snapshot del estado actual del proyecto (cacheado durante analisis)
 */
struct FPGXScaffoldProjectInfo
{
	EPGXProjectType ProjectType = EPGXProjectType::BlueprintOnly;
	FString ProjectName;
	FString ContentDir;
	TArray<FString> Modules;
	TArray<FString> Plugins;
	TArray<FString> ExistingFolders;
	TArray<FString> ExistingAssets;
};

/**
 * EN: Single validation finding / ES: Hallazgo individual de validacion
 */
struct FPGXScaffoldValidationResult
{
	EPGXScaffoldSeverity Severity = EPGXScaffoldSeverity::Info;
	FName ItemId;
	FText Message;
	FText Suggestion;
};

/**
 * EN: Single step in the execution plan / ES: Paso individual del plan de ejecucion
 */
struct FPGXScaffoldPlanStep
{
	int32 StepIndex = 0;
	FName ItemId;
	EPGXScaffoldActionType ActionType = EPGXScaffoldActionType::CreateFolder;
	FString AbsolutePath;
	EPGXScaffoldStepStatus Status = EPGXScaffoldStepStatus::Pending;
	FText DisplayName;

	/** EN: For DA/BP: class name / ES: Para DA/BP: nombre de clase */
	FName AssetClassName;
	FString ParentClassPath;
};

/**
 * EN: Immutable build plan — generated once, executed once
 * ES: Plan de build inmutable — se genera una vez, se ejecuta una vez
 */
struct FPGXScaffoldBuildPlan
{
	FGuid PlanId;
	TArray<FPGXScaffoldPlanStep> Steps;
	TMap<FString, FString> Variables;
	FName TemplateId;
	FDateTime CreatedAt;

	FPGXScaffoldBuildPlan()
		: PlanId(FGuid::NewGuid())
		, CreatedAt(FDateTime::Now())
	{
	}

	int32 CountByType(EPGXScaffoldActionType Type) const
	{
		int32 Count = 0;
		for (const auto& Step : Steps)
		{
			if (Step.ActionType == Type) { ++Count; }
		}
		return Count;
	}

	FString ToJsonString() const
	{
		TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
		Root->SetStringField(TEXT("PlanId"), PlanId.ToString());
		Root->SetStringField(TEXT("TemplateId"), TemplateId.ToString());
		Root->SetStringField(TEXT("CreatedAt"), CreatedAt.ToString());

		// Variables
		TSharedRef<FJsonObject> VarsObj = MakeShared<FJsonObject>();
		for (const auto& Pair : Variables)
		{
			VarsObj->SetStringField(Pair.Key, Pair.Value);
		}
		Root->SetObjectField(TEXT("Variables"), VarsObj);

		// Steps
		TArray<TSharedPtr<FJsonValue>> StepArray;
		for (const auto& Step : Steps)
		{
			TSharedRef<FJsonObject> StepObj = MakeShared<FJsonObject>();
			StepObj->SetNumberField(TEXT("StepIndex"), Step.StepIndex);
			StepObj->SetStringField(TEXT("ItemId"), Step.ItemId.ToString());
			StepObj->SetStringField(TEXT("ActionType"), FString::FromInt(static_cast<int32>(Step.ActionType)));
			StepObj->SetStringField(TEXT("AbsolutePath"), Step.AbsolutePath);
			StepObj->SetStringField(TEXT("Status"), FString::FromInt(static_cast<int32>(Step.Status)));
			StepObj->SetStringField(TEXT("DisplayName"), Step.DisplayName.ToString());
			StepArray.Add(MakeShared<FJsonValueObject>(StepObj));
		}
		Root->SetArrayField(TEXT("Steps"), StepArray);

		FString Output;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
		FJsonSerializer::Serialize(Root, Writer);
		return Output;
	}
};

/**
 * EN: Tree node for the UI hierarchy view / ES: Nodo del arbol para la vista de jerarquia en UI
 */
struct FPGXScaffoldTreeNode : public TSharedFromThis<FPGXScaffoldTreeNode>
{
	FPGXScaffoldTemplateItem Item;
	bool bChecked = true;
	TArray<TSharedPtr<FPGXScaffoldTreeNode>> Children;
	TWeakPtr<FPGXScaffoldTreeNode> Parent;

	bool HasCheckedChildren() const
	{
		for (const auto& Child : Children)
		{
			if (Child->bChecked) { return true; }
		}
		return false;
	}

	bool AllChildrenChecked() const
	{
		for (const auto& Child : Children)
		{
			if (!Child->bChecked) { return false; }
		}
		return true;
	}
};

/**
 * EN: Audit log entry / ES: Entrada del log de auditoria
 */
struct FPGXScaffoldLogEntry
{
	FDateTime Timestamp;
	int32 StepIndex = -1;
	EPGXScaffoldStepStatus Status = EPGXScaffoldStepStatus::Pending;
	FString Message;
	double DurationMs = 0.0;
};

/**
 * EN: Final execution result / ES: Resultado final de ejecucion
 */
struct FPGXScaffoldExecutionResult
{
	EPGXScaffoldOperationStatus Status = EPGXScaffoldOperationStatus::Idle;
	FPGXScaffoldBuildPlan Plan;
	TArray<FPGXScaffoldLogEntry> LogEntries;
	int32 CompletedSteps = 0;
	int32 FailedSteps = 0;
	int32 SkippedSteps = 0;
	double TotalDurationMs = 0.0;
};

/** EN: Progress callback delegate / ES: Delegado de callback de progreso */
DECLARE_DELEGATE_TwoParams(FPGXScaffoldProgressDelegate, int32 /*CurrentStep*/, int32 /*TotalSteps*/);
