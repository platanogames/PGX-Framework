// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXCraftingTypes.h"
#include "PGXCraftingObservability.h"

FPGXCraftJobHandle FPGXCraftJobHandle::NewHandle()
{
	FPGXCraftJobHandle Handle;
	Handle.Id = FGuid::NewGuid();
	return Handle;
}

bool FPGXCraftingRecipeDefinition::IsValid() const
{
	if (!RecipeTag.IsValid() || Outputs.Num() <= 0)
	{
		return false;
	}

	for (const FPGXCraftingResourceQuantity& Input : Inputs)
	{
		if (!Input.IsValid())
		{
			return false;
		}
	}

	for (const FPGXCraftingResourceQuantity& Output : Outputs)
	{
		if (!Output.IsValid())
		{
			return false;
		}
	}

	return true;
}

FPGXCraftingResult FPGXCraftingResult::Success(FGameplayTag InRecipeTag, FPGXCraftJobHandle InHandle, EPGXCraftJobState InState, FString InMessage)
{
	FPGXCraftingResult Result;
	Result.bSuccess = true;
	Result.Code = EPGXCraftingResultCode::Success;
	Result.State = InState;
	Result.Handle = InHandle;
	Result.RecipeTag = InRecipeTag;
	Result.Message = MoveTemp(InMessage);
	return Result;
}

FPGXCraftingResult FPGXCraftingResult::Failure(EPGXCraftingResultCode InCode, EPGXCraftJobState InState, FString InMessage, FGameplayTag InRecipeTag, FPGXCraftJobHandle InHandle)
{
	FPGXCraftingResult Result;
	Result.bSuccess = false;
	Result.Code = InCode;
	Result.State = InState;
	Result.Handle = InHandle;
	Result.RecipeTag = InRecipeTag;
	Result.Message = MoveTemp(InMessage);
	return Result;
}

const FName UPGXRecipeDefinition::SchemaVersion(TEXT("1.0"));

FPGXJsonValue UPGXRecipeDefinition::ToJson() const
{
	FPGXJsonValue Out;
	Out.JsonString = FString::Printf(
		TEXT("{\"schema\":{\"type\":\"%s\",\"version\":\"%s\",\"plugin\":\"%s\"},\"data\":{\"RecipeTag\":\"%s\",\"CategoryTag\":\"%s\",\"DisplayName\":\"%s\",\"InputCount\":%d,\"OutputCount\":%d,\"CraftDurationSeconds\":%.6f,\"bIsSupportedByBaseline\":%s}}"),
		*GetClass()->GetName(),
		*GetSchemaVersion().ToString(),
		*PGXCraftingObservability::GetOwningPluginName(this, TEXT("PGXCraftingRuntime")).ToString(),
		*PGXCraftingObservability::EscapeJsonString(Recipe.RecipeTag.ToString()),
		*PGXCraftingObservability::EscapeJsonString(Recipe.CategoryTag.ToString()),
		*PGXCraftingObservability::EscapeJsonString(Recipe.DisplayName.ToString()),
		Recipe.Inputs.Num(),
		Recipe.Outputs.Num(),
		Recipe.CraftDurationSeconds,
		Recipe.bIsSupportedByBaseline ? TEXT("true") : TEXT("false"));
	return Out;
}

FPGXValidationResult UPGXRecipeDefinition::FromJson(const FPGXJsonValue& Json)
{
	return PGXCraftingObservability::ValidateJsonEnvelope(
		Json,
		NSLOCTEXT("PGXCrafting", "ObservableEmptyPayload", "UPGXRecipeDefinition FromJson received an empty payload."));
}

FName UPGXRecipeDefinition::GetSchemaVersion() const
{
	return SchemaVersion;
}

FPGXSchemaDescriptor UPGXRecipeDefinition::GetSchemaDescriptor() const
{
	return PGXCraftingObservability::MakeSchemaDescriptor(this, GetSchemaVersion(), TEXT("PGXCraftingRuntime"));
}
