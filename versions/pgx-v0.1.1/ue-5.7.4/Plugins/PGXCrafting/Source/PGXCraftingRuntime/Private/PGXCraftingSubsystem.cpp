// Copyright PGX Framework. All Rights Reserved.

#include "PGXCraftingSubsystem.h"

#include "HAL/PlatformTime.h"
#include "Logging/PGXLogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogPGXCraftingSubsystem, Log, All);

void UPGXCraftingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Recipes.Reset();
	CraftJobs.Reset();
}

void UPGXCraftingSubsystem::Deinitialize()
{
	Recipes.Reset();
	CraftJobs.Reset();
	Super::Deinitialize();
}

FPGXCraftingResult UPGXCraftingSubsystem::RegisterRecipe(const FPGXCraftingRecipeDefinition& Recipe)
{
	const FPGXCraftingResult ValidationResult = ValidateRecipeDefinition(Recipe);
	if (!ValidationResult.bSuccess)
	{
		return ValidationResult;
	}

	if (HasRecipe(Recipe.RecipeTag))
	{
		PGX_LOG_WARNING(LogPGXCraftingSubsystem, TEXT("PGXCrafting: duplicate recipe registration rejected: %s"), *Recipe.RecipeTag.ToString());
		return FPGXCraftingResult::Failure(EPGXCraftingResultCode::DuplicateRecipe, EPGXCraftJobState::Failed, TEXT("Recipe is already registered."), Recipe.RecipeTag);
	}

	Recipes.Add(Recipe);
	return FPGXCraftingResult::Success(Recipe.RecipeTag, FPGXCraftJobHandle(), EPGXCraftJobState::None, TEXT("Recipe registered."));
}

FPGXCraftingResult UPGXCraftingSubsystem::RegisterRecipeAsset(const UPGXRecipeDefinition* RecipeAsset)
{
	if (!IsValid(RecipeAsset))
	{
		PGX_LOG_WARNING(LogPGXCraftingSubsystem, TEXT("PGXCrafting: invalid recipe asset registration rejected"));
		return FPGXCraftingResult::Failure(EPGXCraftingResultCode::InvalidRecipe, EPGXCraftJobState::Failed, TEXT("Recipe asset is invalid."));
	}
	return RegisterRecipe(RecipeAsset->Recipe);
}

FPGXCraftingResult UPGXCraftingSubsystem::ValidateRecipeDefinition(const FPGXCraftingRecipeDefinition& Recipe) const
{
	if (!Recipe.RecipeTag.IsValid())
	{
		return FPGXCraftingResult::Failure(EPGXCraftingResultCode::InvalidRecipe, EPGXCraftJobState::Failed, TEXT("Recipe tag is invalid."));
	}
	if (Recipe.Outputs.Num() <= 0)
	{
		return FPGXCraftingResult::Failure(EPGXCraftingResultCode::InvalidRecipe, EPGXCraftJobState::Failed, TEXT("Recipe must define at least one output."), Recipe.RecipeTag);
	}
	for (const FPGXCraftingResourceQuantity& Input : Recipe.Inputs)
	{
		if (!Input.IsValid())
		{
			return FPGXCraftingResult::Failure(EPGXCraftingResultCode::InvalidRecipe, EPGXCraftJobState::Failed, TEXT("Recipe contains an invalid input."), Recipe.RecipeTag);
		}
	}
	for (const FPGXCraftingResourceQuantity& Output : Recipe.Outputs)
	{
		if (!Output.IsValid())
		{
			return FPGXCraftingResult::Failure(EPGXCraftingResultCode::InvalidRecipe, EPGXCraftJobState::Failed, TEXT("Recipe contains an invalid output."), Recipe.RecipeTag);
		}
	}
	return FPGXCraftingResult::Success(Recipe.RecipeTag, FPGXCraftJobHandle(), EPGXCraftJobState::Validated, TEXT("Recipe definition valid."));
}

const FPGXCraftingRecipeDefinition* UPGXCraftingSubsystem::FindRecipe(FGameplayTag RecipeTag) const
{
	return Recipes.FindByPredicate([RecipeTag](const FPGXCraftingRecipeDefinition& Recipe)
	{
		return Recipe.RecipeTag == RecipeTag;
	});
}

bool UPGXCraftingSubsystem::HasRecipe(FGameplayTag RecipeTag) const
{
	return FindRecipe(RecipeTag) != nullptr;
}

int32 UPGXCraftingSubsystem::GetRegisteredRecipeCount() const
{
	return Recipes.Num();
}

TArray<FPGXCraftingRecipeDefinition> UPGXCraftingSubsystem::GetRegisteredRecipesSnapshot() const
{
	TArray<FPGXCraftingRecipeDefinition> Snapshot = Recipes;
	Snapshot.Sort([](const FPGXCraftingRecipeDefinition& Left, const FPGXCraftingRecipeDefinition& Right)
	{
		return Left.RecipeTag.ToString() < Right.RecipeTag.ToString();
	});
	return Snapshot;
}

bool UPGXCraftingSubsystem::HasEntryByTag(FGameplayTag Tag) const
{
	return HasRecipe(Tag);
}

int32 UPGXCraftingSubsystem::GetCount() const
{
	return Recipes.Num();
}

void UPGXCraftingSubsystem::GetSnapshot(TArray<FGameplayTag>& OutTags) const
{
	OutTags.Reset();
	OutTags.Reserve(Recipes.Num());
	for (const FPGXCraftingRecipeDefinition& Recipe : Recipes)
	{
		if (Recipe.RecipeTag.IsValid())
		{
			OutTags.Add(Recipe.RecipeTag);
		}
	}
}

FPGXCraftingResult UPGXCraftingSubsystem::ValidateCraftRequest(const FPGXCraftRequest& Request) const
{
	if (!Request.IsValid())
	{
		PGX_LOG_WARNING(LogPGXCraftingSubsystem, TEXT("PGXCrafting: invalid craft request rejected"));
		return FPGXCraftingResult::Failure(EPGXCraftingResultCode::InvalidRequest, EPGXCraftJobState::Failed, TEXT("Craft request is invalid."), Request.RecipeTag);
	}

	const FPGXCraftingRecipeDefinition* Recipe = FindRecipe(Request.RecipeTag);
	if (!Recipe)
	{
		PGX_LOG_WARNING(LogPGXCraftingSubsystem, TEXT("PGXCrafting: request rejected because recipe is not registered: %s"), *Request.RecipeTag.ToString());
		return FPGXCraftingResult::Failure(EPGXCraftingResultCode::RecipeNotFound, EPGXCraftJobState::Failed, TEXT("Recipe is not registered."), Request.RecipeTag);
	}

	if (!Recipe->bIsSupportedByBaseline)
	{
		return FPGXCraftingResult::Failure(EPGXCraftingResultCode::Unsupported, EPGXCraftJobState::Failed, TEXT("Recipe is outside the baseline supported policy."), Request.RecipeTag);
	}

	return FPGXCraftingResult::Success(Request.RecipeTag, FPGXCraftJobHandle(), EPGXCraftJobState::Validated, TEXT("Craft request validated."));
}

FPGXCraftingResult UPGXCraftingSubsystem::SimulateCraft(const FPGXCraftRequest& Request) const
{
	const FPGXCraftingResult ValidationResult = ValidateCraftRequest(Request);
	if (!ValidationResult.bSuccess)
	{
		return ValidationResult;
	}
	return FPGXCraftingResult::Success(Request.RecipeTag, FPGXCraftJobHandle(), EPGXCraftJobState::Completed, TEXT("Craft simulation succeeded; no inventory or gameplay mutation performed."));
}

FPGXCraftingResult UPGXCraftingSubsystem::StartCraft(const FPGXCraftRequest& Request)
{
	const FPGXCraftingResult ValidationResult = ValidateCraftRequest(Request);
	if (!ValidationResult.bSuccess)
	{
		return ValidationResult;
	}

	if (Request.bSimulateOnly)
	{
		return SimulateCraft(Request);
	}

	if (GetActiveCraftJobCount() >= MaxActiveCraftJobs)
	{
		return FPGXCraftingResult::Failure(EPGXCraftingResultCode::Unsupported, EPGXCraftJobState::Failed, TEXT("Active craft job budget exhausted."), Request.RecipeTag);
	}

	const FPGXCraftingRecipeDefinition* Recipe = FindRecipe(Request.RecipeTag);
	if (!Recipe)
	{
		return FPGXCraftingResult::Failure(EPGXCraftingResultCode::RecipeNotFound, EPGXCraftJobState::Failed, TEXT("Recipe is not registered."), Request.RecipeTag);
	}

	FPGXCraftJobRecord Record;
	Record.Handle = FPGXCraftJobHandle::NewHandle();
	Record.Request = Request;
	Record.Recipe = *Recipe;
	Record.State = EPGXCraftJobState::Active;
	Record.ResultCode = EPGXCraftingResultCode::Success;
	Record.Message = TEXT("Craft job started.");
	Record.CreatedTimeSeconds = FPlatformTime::Seconds();
	CraftJobs.Add(Record);

	return FPGXCraftingResult::Success(Request.RecipeTag, Record.Handle, Record.State, Record.Message);
}

FPGXCraftingResult UPGXCraftingSubsystem::CompleteCraftForTesting(FPGXCraftJobHandle Handle, FString Message)
{
	FPGXCraftJobRecord* Record = FindCraftJobMutable(Handle);
	if (!Record)
	{
		return FPGXCraftingResult::Failure(EPGXCraftingResultCode::JobNotFound, EPGXCraftJobState::Failed, TEXT("Craft job was not found."), FGameplayTag(), Handle);
	}
	if (!IsCraftJobActive(*Record))
	{
		return FPGXCraftingResult::Failure(EPGXCraftingResultCode::AlreadyResolved, Record->State, TEXT("Craft job is already resolved."), Record->Request.RecipeTag, Handle);
	}

	Record->State = EPGXCraftJobState::Completed;
	Record->ResultCode = EPGXCraftingResultCode::Success;
	Record->Message = Message.IsEmpty() ? TEXT("Craft job completed.") : MoveTemp(Message);
	Record->ResolvedTimeSeconds = FPlatformTime::Seconds();
	return FPGXCraftingResult::Success(Record->Request.RecipeTag, Handle, Record->State, Record->Message);
}

FPGXCraftingResult UPGXCraftingSubsystem::CancelCraft(FPGXCraftJobHandle Handle, FString Message)
{
	FPGXCraftJobRecord* Record = FindCraftJobMutable(Handle);
	if (!Record)
	{
		return FPGXCraftingResult::Failure(EPGXCraftingResultCode::JobNotFound, EPGXCraftJobState::Failed, TEXT("Craft job was not found."), FGameplayTag(), Handle);
	}
	if (!IsCraftJobActive(*Record))
	{
		return FPGXCraftingResult::Failure(EPGXCraftingResultCode::AlreadyResolved, Record->State, TEXT("Craft job is already resolved."), Record->Request.RecipeTag, Handle);
	}

	Record->State = EPGXCraftJobState::Cancelled;
	Record->ResultCode = EPGXCraftingResultCode::Success;
	Record->Message = Message.IsEmpty() ? TEXT("Craft job cancelled.") : MoveTemp(Message);
	Record->ResolvedTimeSeconds = FPlatformTime::Seconds();
	return FPGXCraftingResult::Success(Record->Request.RecipeTag, Handle, Record->State, Record->Message);
}

bool UPGXCraftingSubsystem::HasCraftJob(FPGXCraftJobHandle Handle) const
{
	return FindCraftJob(Handle) != nullptr;
}

bool UPGXCraftingSubsystem::GetCraftJob(FPGXCraftJobHandle Handle, FPGXCraftJobRecord& OutRecord) const
{
	const FPGXCraftJobRecord* Record = FindCraftJob(Handle);
	if (!Record)
	{
		return false;
	}
	OutRecord = *Record;
	return true;
}

int32 UPGXCraftingSubsystem::GetActiveCraftJobCount() const
{
	int32 ActiveCount = 0;
	for (const FPGXCraftJobRecord& Record : CraftJobs)
	{
		if (IsCraftJobActive(Record))
		{
			++ActiveCount;
		}
	}
	return ActiveCount;
}

int32 UPGXCraftingSubsystem::GetCraftJobCount() const
{
	return CraftJobs.Num();
}

TArray<FPGXCraftJobRecord> UPGXCraftingSubsystem::GetCraftJobsSnapshot() const
{
	return CraftJobs;
}

int32 UPGXCraftingSubsystem::CleanupResolvedCraftJobs()
{
	const int32 BeforeCount = CraftJobs.Num();
	CraftJobs.RemoveAll([this](const FPGXCraftJobRecord& Record)
	{
		return !IsCraftJobActive(Record);
	});
	return BeforeCount - CraftJobs.Num();
}

#if WITH_DEV_AUTOMATION_TESTS
void UPGXCraftingSubsystem::ClearCraftingStateForTesting()
{
	Recipes.Reset();
	CraftJobs.Reset();
	MaxActiveCraftJobs = 8;
}

void UPGXCraftingSubsystem::SetMaxActiveCraftJobsForTesting(int32 InMaxActiveCraftJobs)
{
	MaxActiveCraftJobs = FMath::Max(0, InMaxActiveCraftJobs);
}
#endif

FPGXCraftJobRecord* UPGXCraftingSubsystem::FindCraftJobMutable(FPGXCraftJobHandle Handle)
{
	return CraftJobs.FindByPredicate([Handle](const FPGXCraftJobRecord& Record)
	{
		return Handle.IsValid() && Record.Handle.Id == Handle.Id;
	});
}

const FPGXCraftJobRecord* UPGXCraftingSubsystem::FindCraftJob(FPGXCraftJobHandle Handle) const
{
	return CraftJobs.FindByPredicate([Handle](const FPGXCraftJobRecord& Record)
	{
		return Handle.IsValid() && Record.Handle.Id == Handle.Id;
	});
}

bool UPGXCraftingSubsystem::IsCraftJobActive(const FPGXCraftJobRecord& Record) const
{
	return Record.State == EPGXCraftJobState::Active;
}
