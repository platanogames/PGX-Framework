// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#if WITH_DEV_AUTOMATION_TESTS

#include "PGXInteractionComponent.h"
#include "Components/SceneComponent.h"
#include "Misc/AutomationTest.h"
#include "Tags/PGXInteractionTags.h"
#include "GameFramework/Actor.h"

namespace PGXInteractionAutomation
{
#define PGX_INTERACTION_AUTOMATION_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

	UPGXInteractionComponent* MakeInteractionComponent(const TCHAR* Name)
	{
		return NewObject<UPGXInteractionComponent>(GetTransientPackage(), UPGXInteractionComponent::StaticClass(), FName(Name), RF_Transient);
	}

	AActor* MakeActor(const TCHAR* Name)
	{
		return NewObject<AActor>(GetTransientPackage(), AActor::StaticClass(), FName(Name), RF_Transient);
	}

	AActor* MakeActorAtLocation(const TCHAR* Name, const FVector& Location)
	{
		AActor* Actor = MakeActor(Name);
		USceneComponent* Root = NewObject<USceneComponent>(Actor, USceneComponent::StaticClass(), FName(TEXT("PGXInteraction_Automation_Root")), RF_Transient);
		Actor->SetRootComponent(Root);
		Root->SetWorldLocation(Location);
		return Actor;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInteraction_ValidRegistrationAutomationTest,
	"PGX.Interaction.preview.ValidRegistration", PGX_INTERACTION_AUTOMATION_FLAGS)
bool FPGXInteraction_ValidRegistrationAutomationTest::RunTest(const FString& Parameters)
{
	UPGXInteractionComponent* Interaction = PGXInteractionAutomation::MakeInteractionComponent(TEXT("PGXInteraction_ValidRegistration"));
	AActor* Target = PGXInteractionAutomation::MakeActor(TEXT("PGXInteraction_Target_ValidRegistration"));

	const FPGXInteractionResult Result = Interaction->RegisterTarget(Target, TAG_PGX_Interaction_Target_Generic.GetTag(), FText::FromString(TEXT("Interact")), 10);

	TestTrue(TEXT("ValidRegistration succeeds"), Result.bSuccess);
	TestTrue(TEXT("ValidRegistration target handle valid"), Result.TargetHandle.IsValid());
	TestTrue(TEXT("ValidRegistration target stored"), Interaction->HasTarget(Result.TargetHandle));
	TestEqual(TEXT("ValidRegistration target count"), Interaction->GetRegisteredTargetCount(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInteraction_InvalidTargetTypedFailureAutomationTest,
	"PGX.Interaction.preview.InvalidTargetTypedFailure", PGX_INTERACTION_AUTOMATION_FLAGS)
bool FPGXInteraction_InvalidTargetTypedFailureAutomationTest::RunTest(const FString& Parameters)
{
	UPGXInteractionComponent* Interaction = PGXInteractionAutomation::MakeInteractionComponent(TEXT("PGXInteraction_InvalidTarget"));

	const FPGXInteractionResult RegisterResult = Interaction->RegisterTarget(nullptr, TAG_PGX_Interaction_Target_Generic.GetTag(), FText::GetEmpty());
	const FPGXInteractionResult ValidateResult = Interaction->ValidateTarget(FPGXInteractionHandle());

	TestFalse(TEXT("Invalid target registration fails"), RegisterResult.bSuccess);
	TestTrue(TEXT("Invalid target registration code"), RegisterResult.Code == EPGXInteractionResultCode::InvalidTarget);
	TestFalse(TEXT("Invalid target validate fails"), ValidateResult.bSuccess);
	TestTrue(TEXT("Invalid target validate code"), ValidateResult.Code == EPGXInteractionResultCode::TargetNotRegistered);
	TestFalse(TEXT("Invalid target messages visible"), RegisterResult.Message.IsEmpty() || ValidateResult.Message.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInteraction_InvalidActionTypedFailureAutomationTest,
	"PGX.Interaction.preview.InvalidActionTypedFailure", PGX_INTERACTION_AUTOMATION_FLAGS)
bool FPGXInteraction_InvalidActionTypedFailureAutomationTest::RunTest(const FString& Parameters)
{
	UPGXInteractionComponent* Interaction = PGXInteractionAutomation::MakeInteractionComponent(TEXT("PGXInteraction_InvalidAction"));
	AActor* Target = PGXInteractionAutomation::MakeActor(TEXT("PGXInteraction_Target_InvalidAction"));
	const FPGXInteractionResult RegisterResult = Interaction->RegisterTarget(Target, TAG_PGX_Interaction_Target_Generic.GetTag(), FText::GetEmpty());

	const FPGXInteractionResult BeginResult = Interaction->BeginInteraction(RegisterResult.TargetHandle, FGameplayTag::EmptyTag);

	TestFalse(TEXT("Invalid action begin fails"), BeginResult.bSuccess);
	TestTrue(TEXT("Invalid action code"), BeginResult.Code == EPGXInteractionResultCode::InvalidAction);
	TestEqual(TEXT("Invalid action no active records"), Interaction->GetActiveInteractionCount(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInteraction_BeginCompleteCancelAutomationTest,
	"PGX.Interaction.preview.BeginCompleteCancel", PGX_INTERACTION_AUTOMATION_FLAGS)
bool FPGXInteraction_BeginCompleteCancelAutomationTest::RunTest(const FString& Parameters)
{
	UPGXInteractionComponent* Interaction = PGXInteractionAutomation::MakeInteractionComponent(TEXT("PGXInteraction_BeginCompleteCancel"));
	AActor* FirstTarget = PGXInteractionAutomation::MakeActor(TEXT("PGXInteraction_Target_First"));
	AActor* SecondTarget = PGXInteractionAutomation::MakeActor(TEXT("PGXInteraction_Target_Second"));
	const FPGXInteractionResult FirstRegister = Interaction->RegisterTarget(FirstTarget, TAG_PGX_Interaction_Target_Generic.GetTag(), FText::GetEmpty());
	const FPGXInteractionResult SecondRegister = Interaction->RegisterTarget(SecondTarget, TAG_PGX_Interaction_Target_Generic.GetTag(), FText::GetEmpty());

	const FPGXInteractionResult BeginResult = Interaction->BeginInteraction(FirstRegister.TargetHandle, TAG_PGX_Interaction_Action_Default.GetTag());
	const FPGXInteractionResult CompleteResult = Interaction->CompleteInteraction(BeginResult.ActionHandle);
	const FPGXInteractionResult RepeatCompleteResult = Interaction->CompleteInteraction(BeginResult.ActionHandle);
	const FPGXInteractionResult SecondBeginResult = Interaction->BeginInteraction(SecondRegister.TargetHandle, TAG_PGX_Interaction_Action_Default.GetTag());
	const FPGXInteractionResult CancelResult = Interaction->CancelInteraction(SecondBeginResult.ActionHandle);

	TestTrue(TEXT("BeginCompleteCancel begin succeeds"), BeginResult.bSuccess && BeginResult.State == EPGXInteractionActionState::Started);
	TestTrue(TEXT("BeginCompleteCancel complete succeeds"), CompleteResult.bSuccess && CompleteResult.State == EPGXInteractionActionState::Completed);
	TestFalse(TEXT("BeginCompleteCancel repeat complete fails"), RepeatCompleteResult.bSuccess);
	TestTrue(TEXT("BeginCompleteCancel repeat complete code"), RepeatCompleteResult.Code == EPGXInteractionResultCode::AlreadyResolved);
	TestTrue(TEXT("BeginCompleteCancel cancel succeeds"), CancelResult.bSuccess && CancelResult.State == EPGXInteractionActionState::Cancelled);
	TestEqual(TEXT("BeginCompleteCancel no active interactions"), Interaction->GetActiveInteractionCount(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInteraction_CleanupQueryAutomationTest,
	"PGX.Interaction.preview.CleanupQuery", PGX_INTERACTION_AUTOMATION_FLAGS)
bool FPGXInteraction_CleanupQueryAutomationTest::RunTest(const FString& Parameters)
{
	UPGXInteractionComponent* Interaction = PGXInteractionAutomation::MakeInteractionComponent(TEXT("PGXInteraction_CleanupQuery"));
	AActor* Target = PGXInteractionAutomation::MakeActor(TEXT("PGXInteraction_Target_CleanupQuery"));
	const FPGXInteractionResult RegisterResult = Interaction->RegisterTarget(Target, TAG_PGX_Interaction_Target_Generic.GetTag(), FText::GetEmpty(), 5);
	const FPGXInteractionResult BeginResult = Interaction->BeginInteraction(RegisterResult.TargetHandle, TAG_PGX_Interaction_Action_Default.GetTag());
	Interaction->CompleteInteraction(BeginResult.ActionHandle);

	const TArray<FPGXInteractableTarget> Targets = Interaction->GetTargetsSnapshot();
	const int32 RemovedCount = Interaction->CleanupResolvedInteractions();

	TestEqual(TEXT("CleanupQuery target snapshot count"), Targets.Num(), 1);
	TestEqual(TEXT("CleanupQuery removed resolved interaction"), RemovedCount, 1);
	TestEqual(TEXT("CleanupQuery no records after cleanup"), Interaction->GetInteractionRecordCount(), 0);
	TestEqual(TEXT("CleanupQuery target remains registered"), Interaction->GetRegisteredTargetCount(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInteraction_ConditionWrapperAutomationTest,
	"PGX.Interaction.preview.ConditionWrapper", PGX_INTERACTION_AUTOMATION_FLAGS)
bool FPGXInteraction_ConditionWrapperAutomationTest::RunTest(const FString& Parameters)
{
	UPGXInteractionComponent* Interaction = PGXInteractionAutomation::MakeInteractionComponent(TEXT("PGXInteraction_ConditionWrapper"));
	const FPGXInteractionResult NoConditionResult = Interaction->EvaluateConditionTyped(nullptr, nullptr);

	TestTrue(TEXT("ConditionWrapper explicit no-condition success"), NoConditionResult.bSuccess);
	TestTrue(TEXT("ConditionWrapper state none"), NoConditionResult.State == EPGXInteractionActionState::None);
	TestFalse(TEXT("ConditionWrapper message visible"), NoConditionResult.Message.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInteraction_QueryOwnerMissingTypedFailureAutomationTest,
	"PGX.Interaction.preview.QueryOwnerMissingTypedFailure", PGX_INTERACTION_AUTOMATION_FLAGS)
bool FPGXInteraction_QueryOwnerMissingTypedFailureAutomationTest::RunTest(const FString& Parameters)
{
	UPGXInteractionComponent* Interaction = PGXInteractionAutomation::MakeInteractionComponent(TEXT("PGXInteraction_QueryOwnerMissing"));

	const FPGXInteractionQueryResult Result = Interaction->QueryBestTargetFromOwner();

	TestFalse(TEXT("Owner-missing query does not silently succeed"), Result.bSuccess);
	TestTrue(TEXT("Owner-missing query returns typed code"), Result.Code == EPGXInteractionResultCode::OwnerMissing);
	TestFalse(TEXT("Owner-missing query has visible message"), Result.Message.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInteraction_QueryOutOfRangeTypedFailureAutomationTest,
	"PGX.Interaction.preview.QueryOutOfRangeTypedFailure", PGX_INTERACTION_AUTOMATION_FLAGS)
bool FPGXInteraction_QueryOutOfRangeTypedFailureAutomationTest::RunTest(const FString& Parameters)
{
	UPGXInteractionComponent* Interaction = PGXInteractionAutomation::MakeInteractionComponent(TEXT("PGXInteraction_QueryOutOfRange"));
	AActor* Interactor = PGXInteractionAutomation::MakeActorAtLocation(TEXT("PGXInteraction_Interactor_OutOfRange"), FVector::ZeroVector);
	AActor* Target = PGXInteractionAutomation::MakeActorAtLocation(TEXT("PGXInteraction_Target_OutOfRange"), FVector(1000.0, 0.0, 0.0));
	Interaction->RegisterTarget(Target, TAG_PGX_Interaction_Target_Generic.GetTag(), FText::FromString(TEXT("Far Target")), 0);

	const FPGXInteractionQueryResult Result = Interaction->QueryBestTargetFromLocation(Interactor, FVector::ZeroVector, 10.0f, false);

	TestFalse(TEXT("Out-of-range query does not silently succeed"), Result.bSuccess);
	TestTrue(TEXT("Out-of-range query returns typed code"), Result.Code == EPGXInteractionResultCode::TargetOutOfRange);
	TestEqual(TEXT("Out-of-range query does not mutate active interactions"), Interaction->GetActiveInteractionCount(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInteraction_QueryInvalidInterfaceTypedFailureAutomationTest,
	"PGX.Interaction.preview.QueryInvalidInterfaceTypedFailure", PGX_INTERACTION_AUTOMATION_FLAGS)
bool FPGXInteraction_QueryInvalidInterfaceTypedFailureAutomationTest::RunTest(const FString& Parameters)
{
	UPGXInteractionComponent* Interaction = PGXInteractionAutomation::MakeInteractionComponent(TEXT("PGXInteraction_QueryInvalidInterface"));
	AActor* Interactor = PGXInteractionAutomation::MakeActorAtLocation(TEXT("PGXInteraction_Interactor_InvalidInterface"), FVector::ZeroVector);
	AActor* Target = PGXInteractionAutomation::MakeActorAtLocation(TEXT("PGXInteraction_Target_InvalidInterface"), FVector::ZeroVector);
	Interaction->RegisterTarget(Target, TAG_PGX_Interaction_Target_Generic.GetTag(), FText::FromString(TEXT("Plain Actor")), 0);

	const FPGXInteractionQueryResult Result = Interaction->QueryBestTargetFromLocation(Interactor, FVector::ZeroVector, 100.0f, true);

	TestFalse(TEXT("Invalid-interface query does not silently succeed"), Result.bSuccess);
	TestTrue(TEXT("Invalid-interface query returns typed code"), Result.Code == EPGXInteractionResultCode::InvalidInterface);
	TestEqual(TEXT("Invalid-interface query does not mutate active interactions"), Interaction->GetActiveInteractionCount(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInteraction_QueryBuildsPromptSnapshotNoMutationAutomationTest,
	"PGX.Interaction.preview.QueryBuildsPromptSnapshotNoMutation", PGX_INTERACTION_AUTOMATION_FLAGS)
bool FPGXInteraction_QueryBuildsPromptSnapshotNoMutationAutomationTest::RunTest(const FString& Parameters)
{
	UPGXInteractionComponent* Interaction = PGXInteractionAutomation::MakeInteractionComponent(TEXT("PGXInteraction_QueryBuildsPrompt"));
	AActor* Interactor = PGXInteractionAutomation::MakeActorAtLocation(TEXT("PGXInteraction_Interactor_QueryBuildsPrompt"), FVector::ZeroVector);
	AActor* LowPriorityTarget = PGXInteractionAutomation::MakeActorAtLocation(TEXT("PGXInteraction_Target_LowPriority"), FVector(20.0, 0.0, 0.0));
	AActor* HighPriorityTarget = PGXInteractionAutomation::MakeActorAtLocation(TEXT("PGXInteraction_Target_HighPriority"), FVector(40.0, 0.0, 0.0));
	Interaction->RegisterTarget(LowPriorityTarget, TAG_PGX_Interaction_Target_Generic.GetTag(), FText::FromString(TEXT("Low Priority")), 1);
	const FPGXInteractionResult HighRegister = Interaction->RegisterTarget(HighPriorityTarget, TAG_PGX_Interaction_Target_Generic.GetTag(), FText::FromString(TEXT("High Priority")), 5);

	const FPGXInteractionQueryResult QueryResult = Interaction->QueryBestTargetFromLocation(Interactor, FVector::ZeroVector, 100.0f, false);
	const FPGXInteractionQueryResult PromptResult = Interaction->BuildPromptSnapshot(HighRegister.TargetHandle, TAG_PGX_Interaction_Action_Default.GetTag(), 40.0f);

	TestTrue(TEXT("Query succeeds through seam without requiring interface"), QueryResult.bSuccess);
	TestTrue(TEXT("Query chooses higher priority target"), QueryResult.PromptSnapshot.TargetHandle.Id == HighRegister.TargetHandle.Id);
	TestTrue(TEXT("Prompt snapshot succeeds"), PromptResult.bSuccess);
	TestTrue(TEXT("Prompt snapshot is presentation-only"), PromptResult.PromptSnapshot.bPresentationOnly);
	TestTrue(TEXT("Prompt snapshot has prompt"), PromptResult.PromptSnapshot.bHasPrompt);
	TestEqual(TEXT("Prompt snapshot carries priority"), PromptResult.PromptSnapshot.Priority, 5);
	TestTrue(TEXT("Prompt/query do not mutate active interactions"), Interaction->GetActiveInteractionCount() == 0);
	TestTrue(TEXT("Prompt/query do not create records"), Interaction->GetInteractionRecordCount() == 0);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
