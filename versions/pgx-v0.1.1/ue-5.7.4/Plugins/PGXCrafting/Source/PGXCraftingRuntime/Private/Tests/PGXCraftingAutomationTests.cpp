// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#if WITH_DEV_AUTOMATION_TESTS

#include "PGXCraftingSubsystem.h"
#include "Tags/PGXCraftingTags.h"
#include "Engine/GameInstance.h"
#include "Misc/AutomationTest.h"

namespace PGXCraftingAutomation
{
#define PGX_CRAFTING_AUTOMATION_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

	UPGXCraftingSubsystem* MakeSubsystem()
	{
		UGameInstance* GameInstance = NewObject<UGameInstance>(GetTransientPackage(), UGameInstance::StaticClass(), NAME_None, RF_Transient);
		UPGXCraftingSubsystem* Subsystem = NewObject<UPGXCraftingSubsystem>(GameInstance, UPGXCraftingSubsystem::StaticClass(), NAME_None, RF_Transient);
		Subsystem->ClearCraftingStateForTesting();
		return Subsystem;
	}

	FPGXCraftingResourceQuantity MakeResource(FGameplayTag ResourceTag, int32 Quantity = 1)
	{
		FPGXCraftingResourceQuantity Resource;
		Resource.ResourceTag = ResourceTag;
		Resource.Quantity = Quantity;
		return Resource;
	}

	FPGXCraftingRecipeDefinition MakeRecipe(bool bSupportedByBaseline = true)
	{
		FPGXCraftingRecipeDefinition Recipe;
		Recipe.RecipeTag = TAG_PGX_Crafting_Recipe_Default.GetTag();
		Recipe.CategoryTag = TAG_PGX_Crafting_Category_Generic.GetTag();
		Recipe.DisplayName = FText::FromString(TEXT("Automation Recipe"));
		Recipe.Inputs.Add(MakeResource(TAG_PGX_Crafting_Resource_Input.GetTag(), 2));
		Recipe.Outputs.Add(MakeResource(TAG_PGX_Crafting_Resource_Output.GetTag(), 1));
		Recipe.CraftDurationSeconds = 0.1f;
		Recipe.bIsSupportedByBaseline = bSupportedByBaseline;
		return Recipe;
	}

	FPGXCraftRequest MakeRequest(bool bSimulateOnly = false)
	{
		FPGXCraftRequest Request;
		Request.RecipeTag = TAG_PGX_Crafting_Recipe_Default.GetTag();
		Request.Quantity = 1;
		Request.SourceTag = TAG_PGX_Crafting_Source_Automation.GetTag();
		Request.bSimulateOnly = bSimulateOnly;
		return Request;
	}

	bool ValidateBaselineUtility(UPGXCraftingSubsystem* Subsystem, TArray<FString>& OutIssues)
	{
		if (!IsValid(Subsystem))
		{
			OutIssues.Add(TEXT("Subsystem is invalid."));
			return false;
		}

		const FPGXCraftingResult RegisterResult = Subsystem->RegisterRecipe(MakeRecipe());
		if (!RegisterResult.bSuccess)
		{
			OutIssues.Add(TEXT("Baseline recipe registration failed."));
		}

		const FPGXCraftingResult SimulationResult = Subsystem->SimulateCraft(MakeRequest(true));
		if (!SimulationResult.bSuccess || SimulationResult.State != EPGXCraftJobState::Completed)
		{
			OutIssues.Add(TEXT("Baseline simulation did not complete."));
		}

		if (Subsystem->GetCraftJobCount() != 0)
		{
			OutIssues.Add(TEXT("Simulation mutated craft job state."));
		}

		return OutIssues.Num() == 0;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXCrafting_RecipeRegisterSuccessAutomationTest,
	"PGX.Crafting.Behavior.RecipeRegisterSuccess", PGX_CRAFTING_AUTOMATION_FLAGS)
bool FPGXCrafting_RecipeRegisterSuccessAutomationTest::RunTest(const FString& Parameters)
{
	UPGXCraftingSubsystem* Subsystem = PGXCraftingAutomation::MakeSubsystem();
	const FPGXCraftingResult Result = Subsystem->RegisterRecipe(PGXCraftingAutomation::MakeRecipe());

	TestTrue(TEXT("RecipeRegisterSuccess succeeds"), Result.bSuccess);
	TestTrue(TEXT("RecipeRegisterSuccess tag stored"), Subsystem->HasRecipe(TAG_PGX_Crafting_Recipe_Default.GetTag()));
	TestEqual(TEXT("RecipeRegisterSuccess count"), Subsystem->GetRegisteredRecipeCount(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXCrafting_DuplicateRecipeTypedFailureAutomationTest,
	"PGX.Crafting.Behavior.DuplicateRecipeTypedFailure", PGX_CRAFTING_AUTOMATION_FLAGS)
bool FPGXCrafting_DuplicateRecipeTypedFailureAutomationTest::RunTest(const FString& Parameters)
{
	UPGXCraftingSubsystem* Subsystem = PGXCraftingAutomation::MakeSubsystem();
	const FPGXCraftingResult FirstResult = Subsystem->RegisterRecipe(PGXCraftingAutomation::MakeRecipe());
	const FPGXCraftingResult DuplicateResult = Subsystem->RegisterRecipe(PGXCraftingAutomation::MakeRecipe());

	TestTrue(TEXT("DuplicateRecipe setup succeeds"), FirstResult.bSuccess);
	TestFalse(TEXT("DuplicateRecipe second registration fails"), DuplicateResult.bSuccess);
	TestTrue(TEXT("DuplicateRecipe typed code"), DuplicateResult.Code == EPGXCraftingResultCode::DuplicateRecipe);
	TestFalse(TEXT("DuplicateRecipe visible message"), DuplicateResult.Message.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXCrafting_InvalidRecipeTypedFailureAutomationTest,
	"PGX.Crafting.Behavior.InvalidRecipeTypedFailure", PGX_CRAFTING_AUTOMATION_FLAGS)
bool FPGXCrafting_InvalidRecipeTypedFailureAutomationTest::RunTest(const FString& Parameters)
{
	UPGXCraftingSubsystem* Subsystem = PGXCraftingAutomation::MakeSubsystem();
	FPGXCraftingRecipeDefinition Recipe = PGXCraftingAutomation::MakeRecipe();
	Recipe.Outputs.Reset();

	const FPGXCraftingResult Result = Subsystem->RegisterRecipe(Recipe);

	TestFalse(TEXT("InvalidRecipe registration fails"), Result.bSuccess);
	TestTrue(TEXT("InvalidRecipe typed code"), Result.Code == EPGXCraftingResultCode::InvalidRecipe);
	TestEqual(TEXT("InvalidRecipe count unchanged"), Subsystem->GetRegisteredRecipeCount(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXCrafting_RequestRecipeNotFoundAutomationTest,
	"PGX.Crafting.Behavior.RequestRecipeNotFound", PGX_CRAFTING_AUTOMATION_FLAGS)
bool FPGXCrafting_RequestRecipeNotFoundAutomationTest::RunTest(const FString& Parameters)
{
	UPGXCraftingSubsystem* Subsystem = PGXCraftingAutomation::MakeSubsystem();
	const FPGXCraftingResult Result = Subsystem->ValidateCraftRequest(PGXCraftingAutomation::MakeRequest());

	TestFalse(TEXT("RecipeNotFound request fails"), Result.bSuccess);
	TestTrue(TEXT("RecipeNotFound typed code"), Result.Code == EPGXCraftingResultCode::RecipeNotFound);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXCrafting_SimulationNoMutationAutomationTest,
	"PGX.Crafting.Behavior.SimulationNoMutation", PGX_CRAFTING_AUTOMATION_FLAGS)
bool FPGXCrafting_SimulationNoMutationAutomationTest::RunTest(const FString& Parameters)
{
	UPGXCraftingSubsystem* Subsystem = PGXCraftingAutomation::MakeSubsystem();
	Subsystem->RegisterRecipe(PGXCraftingAutomation::MakeRecipe());

	const FPGXCraftingResult Result = Subsystem->SimulateCraft(PGXCraftingAutomation::MakeRequest(true));

	TestTrue(TEXT("SimulationNoMutation succeeds"), Result.bSuccess);
	TestTrue(TEXT("SimulationNoMutation completed state"), Result.State == EPGXCraftJobState::Completed);
	TestEqual(TEXT("SimulationNoMutation no jobs created"), Subsystem->GetCraftJobCount(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXCrafting_StartCompleteJobAutomationTest,
	"PGX.Crafting.Behavior.StartCompleteJob", PGX_CRAFTING_AUTOMATION_FLAGS)
bool FPGXCrafting_StartCompleteJobAutomationTest::RunTest(const FString& Parameters)
{
	UPGXCraftingSubsystem* Subsystem = PGXCraftingAutomation::MakeSubsystem();
	Subsystem->RegisterRecipe(PGXCraftingAutomation::MakeRecipe());
	const FPGXCraftingResult StartResult = Subsystem->StartCraft(PGXCraftingAutomation::MakeRequest());
	const FPGXCraftingResult CompleteResult = Subsystem->CompleteCraftForTesting(StartResult.Handle, TEXT("Automation complete"));

	TestTrue(TEXT("StartComplete start succeeds"), StartResult.bSuccess);
	TestTrue(TEXT("StartComplete handle valid"), StartResult.Handle.IsValid());
	TestTrue(TEXT("StartComplete complete succeeds"), CompleteResult.bSuccess);
	TestTrue(TEXT("StartComplete completed state"), CompleteResult.State == EPGXCraftJobState::Completed);
	TestEqual(TEXT("StartComplete active count"), Subsystem->GetActiveCraftJobCount(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXCrafting_CancelCleanupJobAutomationTest,
	"PGX.Crafting.Behavior.CancelCleanupJob", PGX_CRAFTING_AUTOMATION_FLAGS)
bool FPGXCrafting_CancelCleanupJobAutomationTest::RunTest(const FString& Parameters)
{
	UPGXCraftingSubsystem* Subsystem = PGXCraftingAutomation::MakeSubsystem();
	Subsystem->RegisterRecipe(PGXCraftingAutomation::MakeRecipe());
	const FPGXCraftingResult StartResult = Subsystem->StartCraft(PGXCraftingAutomation::MakeRequest());
	const FPGXCraftingResult CancelResult = Subsystem->CancelCraft(StartResult.Handle, TEXT("Automation cancel"));

	TestTrue(TEXT("CancelCleanup start succeeds"), StartResult.bSuccess);
	TestTrue(TEXT("CancelCleanup cancel succeeds"), CancelResult.bSuccess);
	TestTrue(TEXT("CancelCleanup cancelled state"), CancelResult.State == EPGXCraftJobState::Cancelled);
	TestEqual(TEXT("CancelCleanup removed count"), Subsystem->CleanupResolvedCraftJobs(), 1);
	TestEqual(TEXT("CancelCleanup job count"), Subsystem->GetCraftJobCount(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXCrafting_BudgetTypedFailureAutomationTest,
	"PGX.Crafting.Behavior.BudgetTypedFailure", PGX_CRAFTING_AUTOMATION_FLAGS)
bool FPGXCrafting_BudgetTypedFailureAutomationTest::RunTest(const FString& Parameters)
{
	UPGXCraftingSubsystem* Subsystem = PGXCraftingAutomation::MakeSubsystem();
	Subsystem->RegisterRecipe(PGXCraftingAutomation::MakeRecipe());
	Subsystem->SetMaxActiveCraftJobsForTesting(1);
	const FPGXCraftingResult FirstResult = Subsystem->StartCraft(PGXCraftingAutomation::MakeRequest());
	const FPGXCraftingResult BudgetResult = Subsystem->StartCraft(PGXCraftingAutomation::MakeRequest());

	TestTrue(TEXT("BudgetFailure setup succeeds"), FirstResult.bSuccess);
	TestFalse(TEXT("BudgetFailure second job fails"), BudgetResult.bSuccess);
	TestTrue(TEXT("BudgetFailure typed code"), BudgetResult.Code == EPGXCraftingResultCode::Unsupported);
	TestEqual(TEXT("BudgetFailure active count unchanged"), Subsystem->GetActiveCraftJobCount(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXCrafting_BaselineUtilityNoIssuesAutomationTest,
	"PGX.Crafting.Behavior.BaselineUtilityNoIssues", PGX_CRAFTING_AUTOMATION_FLAGS)
bool FPGXCrafting_BaselineUtilityNoIssuesAutomationTest::RunTest(const FString& Parameters)
{
	UPGXCraftingSubsystem* Subsystem = PGXCraftingAutomation::MakeSubsystem();
	TArray<FString> Issues;
	const bool bPassed = PGXCraftingAutomation::ValidateBaselineUtility(Subsystem, Issues);

	TestTrue(TEXT("BaselineUtility passes"), bPassed);
	TestEqual(TEXT("BaselineUtility issue count"), Issues.Num(), 0);
	return true;
}


// ============================================================================
// EN: Observability 8.3.C economy cluster
// ES: Observability 8.3.C cluster economia
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXCrafting_ObservableRecipeDefinitionAutomationTest,
	"PGX.Crafting.Serialization.ObservableRecipeDefinition", PGX_CRAFTING_AUTOMATION_FLAGS)
bool FPGXCrafting_ObservableRecipeDefinitionAutomationTest::RunTest(const FString& Parameters)
{
	UPGXRecipeDefinition* Asset = NewObject<UPGXRecipeDefinition>(GetTransientPackage(), UPGXRecipeDefinition::StaticClass(), NAME_None, RF_Transient);
	if (!TestNotNull(TEXT("Recipe definition asset"), Asset))
	{
		return false;
	}
	Asset->Recipe = PGXCraftingAutomation::MakeRecipe();

	TestTrue(TEXT("Recipe asset implements IPGXObservable"), UPGXRecipeDefinition::StaticClass()->ImplementsInterface(UPGXObservable::StaticClass()));
	const FPGXJsonValue Json = Asset->ToJson();
	const FString ExpectedType = FString::Printf(TEXT("\"type\":\"%s\""), *Asset->GetClass()->GetName());
	TestFalse(TEXT("Recipe observable ToJson non-empty"), Json.IsEmpty());
	TestTrue(TEXT("Recipe JSON contains type"), Json.JsonString.Contains(ExpectedType));
	TestTrue(TEXT("Recipe JSON contains concrete data"), Json.JsonString.Contains(TEXT("InputCount")));
	TestEqual(TEXT("Recipe schema version"), Asset->GetSchemaVersion(), UPGXRecipeDefinition::SchemaVersion);

	const FPGXSchemaDescriptor Descriptor = Asset->GetSchemaDescriptor();
	TestEqual(TEXT("Recipe descriptor type"), Descriptor.TypeName, UPGXRecipeDefinition::StaticClass()->GetFName());
	TestTrue(TEXT("Recipe descriptor fields"), Descriptor.Fields.Num() > 0);

	const FPGXValidationResult EmptyValidation = Asset->FromJson(FPGXJsonValue());
	TestFalse(TEXT("Recipe FromJson empty payload fails"), EmptyValidation.bValid);
	TestTrue(TEXT("Recipe FromJson reports errors"), EmptyValidation.Errors.Num() > 0);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
