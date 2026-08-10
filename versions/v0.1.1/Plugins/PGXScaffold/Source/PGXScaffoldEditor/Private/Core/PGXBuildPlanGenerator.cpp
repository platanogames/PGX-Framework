// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Core/PGXBuildPlanGenerator.h"
#include "PGXScaffoldEditor.h"
#include "Logging/PGXLogMacros.h"

FPGXScaffoldBuildPlan FPGXBuildPlanGenerator::Generate(
	const TArray<FPGXScaffoldTemplateItem>& Items,
	const TMap<FString, FString>& Variables,
	const FString& ContentDir,
	FName TemplateId)
{
	FPGXScaffoldBuildPlan Plan;
	Plan.TemplateId = TemplateId;
	Plan.Variables = Variables;

	// EN: Topological sort — folders first, then DataAssets, then Blueprints
	//     Within each group, sort by ExecutionOrder
	// ES: Ordenamiento topologico — carpetas primero, luego DataAssets, luego Blueprints
	//     Dentro de cada grupo, ordenar por ExecutionOrder
	TArray<FPGXScaffoldTemplateItem> Folders;
	TArray<FPGXScaffoldTemplateItem> DataAssets;
	TArray<FPGXScaffoldTemplateItem> Blueprints;

	for (const auto& Item : Items)
	{
		switch (Item.ActionType)
		{
		case EPGXScaffoldActionType::CreateFolder:
			Folders.Add(Item);
			break;
		case EPGXScaffoldActionType::CreateDataAsset:
			DataAssets.Add(Item);
			break;
		case EPGXScaffoldActionType::CreateBlueprint:
			Blueprints.Add(Item);
			break;
		}
	}

	auto SortByOrder = [](const FPGXScaffoldTemplateItem& A, const FPGXScaffoldTemplateItem& B)
	{
		return A.ExecutionOrder < B.ExecutionOrder;
	};

	Folders.Sort(SortByOrder);
	DataAssets.Sort(SortByOrder);
	Blueprints.Sort(SortByOrder);

	// EN: Build steps in order: Folders → DataAssets → Blueprints
	// ES: Construir pasos en orden: Carpetas → DataAssets → Blueprints
	int32 StepIdx = 0;

	auto AddSteps = [&](const TArray<FPGXScaffoldTemplateItem>& Group)
	{
		for (const auto& Item : Group)
		{
			FPGXScaffoldPlanStep Step;
			Step.StepIndex = StepIdx++;
			Step.ItemId = Item.ItemId;
			Step.ActionType = Item.ActionType;
			Step.DisplayName = Item.DisplayName;
			Step.AssetClassName = Item.AssetClassName;
			Step.ParentClassPath = Item.ParentClassPath;

			FString ResolvedPath = ResolveVariables(Item.RelativePath, Variables);
			Step.AbsolutePath = ContentDir / ResolvedPath;

			Plan.Steps.Add(Step);
		}
	};

	AddSteps(Folders);
	AddSteps(DataAssets);
	AddSteps(Blueprints);

	PGX_LOG_INFO(LogPGXScaffold, TEXT("FPGXBuildPlanGenerator: Generated plan '%s' — %d steps (%d folders, %d DAs, %d BPs)"),
		*Plan.PlanId.ToString(), Plan.Steps.Num(), Folders.Num(), DataAssets.Num(), Blueprints.Num());

	return Plan;
}

FString FPGXBuildPlanGenerator::ResolveVariables(const FString& Path, const TMap<FString, FString>& Variables)
{
	FString Result = Path;
	for (const auto& Pair : Variables)
	{
		FString Token = FString::Printf(TEXT("{%s}"), *Pair.Key);
		Result = Result.Replace(*Token, *Pair.Value);
	}
	return Result;
}
