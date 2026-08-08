// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXFrameworkValidator.generated.h"

UENUM(BlueprintType)
enum class EPGXFrameworkValidationSeverity : uint8
{
	Info,
	Warning,
	Error
};

UENUM(BlueprintType)
enum class EPGXFrameworkValidationRule : uint8
{
	StarTopologyRuntimePluginDependency,
	StarTopologyRuntimeModuleDependency,
	StarTopologyAllowedRuntimeDependency,
	StarTopologyScanUnavailable
};

USTRUCT(BlueprintType)
struct PGXCOREEDITOR_API FPGXAllowedL2Edge
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Validation")
	FString SourceModule;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Validation")
	FString TargetModule;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Validation")
	FString Reason;

	bool operator==(const FPGXAllowedL2Edge& Other) const
	{
		return SourceModule.Equals(Other.SourceModule, ESearchCase::IgnoreCase)
			&& TargetModule.Equals(Other.TargetModule, ESearchCase::IgnoreCase);
	}
};

USTRUCT(BlueprintType)
struct PGXCOREEDITOR_API FPGXL2DependencyEdge
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Validation")
	EPGXFrameworkValidationRule Rule = EPGXFrameworkValidationRule::StarTopologyRuntimeModuleDependency;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Validation")
	FString SourcePlugin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Validation")
	FString SourceModule;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Validation")
	FString TargetPlugin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Validation")
	FString TargetModule;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Validation")
	FString SourceFile;
};

USTRUCT(BlueprintType)
struct PGXCOREEDITOR_API FPGXFrameworkValidationIssue
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PGX|Validation")
	EPGXFrameworkValidationSeverity Severity = EPGXFrameworkValidationSeverity::Info;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PGX|Validation")
	EPGXFrameworkValidationRule Rule = EPGXFrameworkValidationRule::StarTopologyScanUnavailable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PGX|Validation")
	FString SourcePlugin;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PGX|Validation")
	FString SourceModule;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PGX|Validation")
	FString TargetPlugin;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PGX|Validation")
	FString TargetModule;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PGX|Validation")
	FString SourceFile;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PGX|Validation")
	FString Reason;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PGX|Validation")
	FString Message;
};

UCLASS(config=EditorPerProjectUserSettings, defaultconfig)
class PGXCOREEDITOR_API UPGXValidationSettings : public UObject
{
	GENERATED_BODY()

public:
	UPGXValidationSettings();

	// EN: Temporary documented exceptions while L2 runtime edges are migrated to Message.
	// ES: Excepciones documentadas temporales mientras los edges runtime L2 migran a Message.
	UPROPERTY(EditAnywhere, Config, Category = "PGX|Validation|StarTopology")
	TArray<FPGXAllowedL2Edge> AllowedL2Edges;
};

/**
 * EN: Level 1 framework validation. Verifies PGX star-topology dependency integrity.
 * ES: Validacion de framework Nivel 1. Verifica integridad de dependencias de topologia en estrella PGX.
 */
UCLASS()
class PGXCOREEDITOR_API UPGXFrameworkValidator : public UObject
{
	GENERATED_BODY()

public:
	// EN: Run full framework validation / ES: Ejecutar validacion completa del framework
	UFUNCTION(BlueprintCallable, Category = "PGX|Validation")
	static bool ValidateFramework(TArray<FText>& OutErrors, TArray<FText>& OutWarnings);

	// EN: Validate that L2 runtime plugins do not depend directly on other L2 runtime plugins.
	// ES: Validar que plugins runtime L2 no dependan directamente de otros plugins runtime L2.
	UFUNCTION(BlueprintCallable, Category = "PGX|Validation")
	static bool ValidateStarTopology(TArray<FPGXFrameworkValidationIssue>& OutIssues);

	static void GetDefaultAllowedL2Edges(TArray<FPGXAllowedL2Edge>& OutAllowedEdges);
	static void GetEffectiveAllowedL2Edges(TArray<FPGXAllowedL2Edge>& OutAllowedEdges);
	static bool ValidateStarTopologyEdges(const TArray<FPGXL2DependencyEdge>& DetectedEdges,
		const TArray<FPGXAllowedL2Edge>& AllowedEdges,
		TArray<FPGXFrameworkValidationIssue>& OutIssues);
};
