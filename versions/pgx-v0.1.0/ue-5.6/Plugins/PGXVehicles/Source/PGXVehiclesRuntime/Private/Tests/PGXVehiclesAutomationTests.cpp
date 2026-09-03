// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#if WITH_DEV_AUTOMATION_TESTS

#include "PGXVehiclesSubsystem.h"
#include "Tags/PGXVehiclesTags.h"
#include "Engine/GameInstance.h"
#include "Misc/AutomationTest.h"

namespace PGXVehiclesAutomation
{
#define PGX_VEHICLES_AUTOMATION_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

	UPGXVehiclesSubsystem* MakeSubsystem()
	{
		UGameInstance* GameInstance = NewObject<UGameInstance>(GetTransientPackage(), NAME_None, RF_Transient);
		UPGXVehiclesSubsystem* Subsystem = NewObject<UPGXVehiclesSubsystem>(GameInstance, UPGXVehiclesSubsystem::StaticClass(), NAME_None, RF_Transient);
		Subsystem->ClearVehiclesStateForTesting();
		return Subsystem;
	}

	FPGXVehicleDefinition MakeDefinition()
	{
		FPGXVehicleDefinition Definition;
		Definition.DefinitionTag = TAG_PGX_Vehicles_Definition_Default.GetTag();
		Definition.VehicleTypeTag = TAG_PGX_Vehicles_Type_Generic.GetTag();
		Definition.DisplayName = FText::FromString(TEXT("Automation Vehicle"));
		Definition.MaxFuel = 100.0f;
		Definition.MaxCondition = 100.0f;
		Definition.PassengerCapacity = 2;
		return Definition;
	}

	FPGXVehicleRegistration MakeRegistration()
	{
		FPGXVehicleRegistration Registration;
		Registration.Definition = MakeDefinition();
		Registration.SourceTag = TAG_PGX_Vehicles_Source_Automation.GetTag();
		return Registration;
	}

	bool ValidateBaselineUtility(UPGXVehiclesSubsystem* Subsystem, TArray<FString>& OutIssues)
	{
		if (!IsValid(Subsystem))
		{
			OutIssues.Add(TEXT("Subsystem is invalid."));
			return false;
		}

		const FPGXVehicleResult RegisterResult = Subsystem->RegisterVehicle(MakeRegistration());
		if (!RegisterResult.bSuccess)
		{
			OutIssues.Add(TEXT("Baseline vehicle registration failed."));
		}

		const FPGXVehicleResult ClaimResult = Subsystem->ClaimVehicle(RegisterResult.Handle, TEXT("AutomationOwner"), TAG_PGX_Vehicles_Source_Automation.GetTag());
		if (!ClaimResult.bSuccess)
		{
			OutIssues.Add(TEXT("Baseline vehicle claim failed."));
		}

		FPGXVehicleState State;
		if (!Subsystem->GetVehicleState(RegisterResult.Handle, State) || State.Availability != EPGXVehicleAvailability::Claimed)
		{
			OutIssues.Add(TEXT("Baseline vehicle state snapshot invalid."));
		}

		return OutIssues.Num() == 0;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXVehicles_RegisterVehicleSuccessAutomationTest,
	"PGX.Vehicles.CoreOps.RegisterVehicleSuccess", PGX_VEHICLES_AUTOMATION_FLAGS)
bool FPGXVehicles_RegisterVehicleSuccessAutomationTest::RunTest(const FString& Parameters)
{
	UPGXVehiclesSubsystem* Subsystem = PGXVehiclesAutomation::MakeSubsystem();
	const FPGXVehicleResult Result = Subsystem->RegisterVehicle(PGXVehiclesAutomation::MakeRegistration());

	TestTrue(TEXT("RegisterVehicle succeeds"), Result.bSuccess);
	TestTrue(TEXT("RegisterVehicle handle valid"), Result.Handle.IsValid());
	TestEqual(TEXT("RegisterVehicle count"), Subsystem->GetVehicleCount(), 1);
	TestEqual(TEXT("RegisterVehicle history count"), Subsystem->GetOperationRecordCount(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXVehicles_InvalidDefinitionTypedFailureAutomationTest,
	"PGX.Vehicles.CoreOps.InvalidDefinitionTypedFailure", PGX_VEHICLES_AUTOMATION_FLAGS)
bool FPGXVehicles_InvalidDefinitionTypedFailureAutomationTest::RunTest(const FString& Parameters)
{
	UPGXVehiclesSubsystem* Subsystem = PGXVehiclesAutomation::MakeSubsystem();
	FPGXVehicleRegistration Registration = PGXVehiclesAutomation::MakeRegistration();
	Registration.Definition.DefinitionTag = FGameplayTag();

	const FPGXVehicleResult Result = Subsystem->RegisterVehicle(Registration);

	TestFalse(TEXT("InvalidDefinition registration fails"), Result.bSuccess);
	TestTrue(TEXT("InvalidDefinition typed code"), Result.Code == EPGXVehicleResultCode::InvalidDefinition);
	TestFalse(TEXT("InvalidDefinition visible message"), Result.Message.IsEmpty());
	TestEqual(TEXT("InvalidDefinition count unchanged"), Subsystem->GetVehicleCount(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXVehicles_CapacityTypedFailureAutomationTest,
	"PGX.Vehicles.CoreOps.CapacityTypedFailure", PGX_VEHICLES_AUTOMATION_FLAGS)
bool FPGXVehicles_CapacityTypedFailureAutomationTest::RunTest(const FString& Parameters)
{
	UPGXVehiclesSubsystem* Subsystem = PGXVehiclesAutomation::MakeSubsystem();
	Subsystem->SetMaxVehiclesForTesting(1);
	const FPGXVehicleResult FirstResult = Subsystem->RegisterVehicle(PGXVehiclesAutomation::MakeRegistration());
	const FPGXVehicleResult CapacityResult = Subsystem->RegisterVehicle(PGXVehiclesAutomation::MakeRegistration());

	TestTrue(TEXT("Capacity setup succeeds"), FirstResult.bSuccess);
	TestFalse(TEXT("Capacity second registration fails"), CapacityResult.bSuccess);
	TestTrue(TEXT("Capacity typed code"), CapacityResult.Code == EPGXVehicleResultCode::CapacityExceeded);
	TestEqual(TEXT("Capacity count unchanged"), Subsystem->GetVehicleCount(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXVehicles_ClaimParkTransitionsAutomationTest,
	"PGX.Vehicles.CoreOps.ClaimParkTransitions", PGX_VEHICLES_AUTOMATION_FLAGS)
bool FPGXVehicles_ClaimParkTransitionsAutomationTest::RunTest(const FString& Parameters)
{
	UPGXVehiclesSubsystem* Subsystem = PGXVehiclesAutomation::MakeSubsystem();
	const FPGXVehicleResult RegisterResult = Subsystem->RegisterVehicle(PGXVehiclesAutomation::MakeRegistration());
	const FPGXVehicleResult ClaimResult = Subsystem->ClaimVehicle(RegisterResult.Handle, TEXT("AutomationOwner"), TAG_PGX_Vehicles_Source_Automation.GetTag());
	const FPGXVehicleResult ParkResult = Subsystem->ParkVehicle(RegisterResult.Handle, TAG_PGX_Vehicles_Source_Automation.GetTag());
	FPGXVehicleState State;
	const bool bHasState = Subsystem->GetVehicleState(RegisterResult.Handle, State);

	TestTrue(TEXT("ClaimPark registration succeeds"), RegisterResult.bSuccess);
	TestTrue(TEXT("ClaimPark claim succeeds"), ClaimResult.bSuccess);
	TestTrue(TEXT("ClaimPark park succeeds"), ParkResult.bSuccess);
	TestTrue(TEXT("ClaimPark state exists"), bHasState);
	TestTrue(TEXT("ClaimPark final parked"), State.Availability == EPGXVehicleAvailability::Parked);
	TestEqual(TEXT("ClaimPark history count"), Subsystem->GetOperationRecordCount(), 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXVehicles_RefuelClampAutomationTest,
	"PGX.Vehicles.CoreOps.RefuelClamp", PGX_VEHICLES_AUTOMATION_FLAGS)
bool FPGXVehicles_RefuelClampAutomationTest::RunTest(const FString& Parameters)
{
	UPGXVehiclesSubsystem* Subsystem = PGXVehiclesAutomation::MakeSubsystem();
	const FPGXVehicleResult RegisterResult = Subsystem->RegisterVehicle(PGXVehiclesAutomation::MakeRegistration());
	FPGXVehicleState InitialState;
	Subsystem->GetVehicleState(RegisterResult.Handle, InitialState);
	const FPGXVehicleResult RefuelResult = Subsystem->RefuelVehicle(RegisterResult.Handle, 50.0f, TAG_PGX_Vehicles_Source_Automation.GetTag());
	FPGXVehicleState FinalState;
	Subsystem->GetVehicleState(RegisterResult.Handle, FinalState);

	TestTrue(TEXT("RefuelClamp setup fuel full"), FMath::IsNearlyEqual(InitialState.Fuel, InitialState.Definition.MaxFuel));
	TestTrue(TEXT("RefuelClamp refuel succeeds"), RefuelResult.bSuccess);
	TestTrue(TEXT("RefuelClamp remains clamped"), FMath::IsNearlyEqual(FinalState.Fuel, FinalState.Definition.MaxFuel));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXVehicles_RepairClampAutomationTest,
	"PGX.Vehicles.CoreOps.RepairClamp", PGX_VEHICLES_AUTOMATION_FLAGS)
bool FPGXVehicles_RepairClampAutomationTest::RunTest(const FString& Parameters)
{
	UPGXVehiclesSubsystem* Subsystem = PGXVehiclesAutomation::MakeSubsystem();
	const FPGXVehicleResult RegisterResult = Subsystem->RegisterVehicle(PGXVehiclesAutomation::MakeRegistration());
	const FPGXVehicleResult RepairResult = Subsystem->RepairVehicle(RegisterResult.Handle, 50.0f, TAG_PGX_Vehicles_Source_Automation.GetTag());
	FPGXVehicleState State;
	Subsystem->GetVehicleState(RegisterResult.Handle, State);

	TestTrue(TEXT("RepairClamp repair succeeds"), RepairResult.bSuccess);
	TestTrue(TEXT("RepairClamp remains clamped"), FMath::IsNearlyEqual(State.Condition, State.Definition.MaxCondition));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXVehicles_InvalidOperationTypedFailureAutomationTest,
	"PGX.Vehicles.CoreOps.InvalidOperationTypedFailure", PGX_VEHICLES_AUTOMATION_FLAGS)
bool FPGXVehicles_InvalidOperationTypedFailureAutomationTest::RunTest(const FString& Parameters)
{
	UPGXVehiclesSubsystem* Subsystem = PGXVehiclesAutomation::MakeSubsystem();
	const FPGXVehicleResult RegisterResult = Subsystem->RegisterVehicle(PGXVehiclesAutomation::MakeRegistration());
	const FPGXVehicleResult InvalidRefuelResult = Subsystem->RefuelVehicle(RegisterResult.Handle, -1.0f, TAG_PGX_Vehicles_Source_Automation.GetTag());

	TestFalse(TEXT("InvalidOperation refuel fails"), InvalidRefuelResult.bSuccess);
	TestTrue(TEXT("InvalidOperation typed code"), InvalidRefuelResult.Code == EPGXVehicleResultCode::InvalidRequest);
	TestEqual(TEXT("InvalidOperation rejected history recorded"), Subsystem->GetOperationRecordCount(), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXVehicles_MissingVehicleTypedFailureAutomationTest,
	"PGX.Vehicles.CoreOps.MissingVehicleTypedFailure", PGX_VEHICLES_AUTOMATION_FLAGS)
bool FPGXVehicles_MissingVehicleTypedFailureAutomationTest::RunTest(const FString& Parameters)
{
	UPGXVehiclesSubsystem* Subsystem = PGXVehiclesAutomation::MakeSubsystem();
	const FPGXVehicleResult Result = Subsystem->ParkVehicle(FPGXVehicleHandle::NewHandle(), TAG_PGX_Vehicles_Source_Automation.GetTag());

	TestFalse(TEXT("MissingVehicle operation fails"), Result.bSuccess);
	TestTrue(TEXT("MissingVehicle typed code"), Result.Code == EPGXVehicleResultCode::VehicleNotFound);
	TestFalse(TEXT("MissingVehicle visible message"), Result.Message.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXVehicles_BaselineUtilityNoIssuesAutomationTest,
	"PGX.Vehicles.CoreOps.BaselineUtilityNoIssues", PGX_VEHICLES_AUTOMATION_FLAGS)
bool FPGXVehicles_BaselineUtilityNoIssuesAutomationTest::RunTest(const FString& Parameters)
{
	UPGXVehiclesSubsystem* Subsystem = PGXVehiclesAutomation::MakeSubsystem();
	TArray<FString> Issues;
	const bool bPassed = PGXVehiclesAutomation::ValidateBaselineUtility(Subsystem, Issues);

	TestTrue(TEXT("BaselineUtility passes"), bPassed);
	TestEqual(TEXT("BaselineUtility issue count"), Issues.Num(), 0);
	return true;
}


// ============================================================================
// EN: Observability 8.3.C economy cluster
// ES: Observability 8.3.C cluster economia
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXVehicles_ObservableDefinitionAssetAutomationTest,
	"PGX.Vehicles.Observable.ObservableDefinitionAsset", PGX_VEHICLES_AUTOMATION_FLAGS)
bool FPGXVehicles_ObservableDefinitionAssetAutomationTest::RunTest(const FString& Parameters)
{
	UPGXVehicleDefinitionAsset* Asset = NewObject<UPGXVehicleDefinitionAsset>(GetTransientPackage(), UPGXVehicleDefinitionAsset::StaticClass(), NAME_None, RF_Transient);
	if (!TestNotNull(TEXT("Vehicle definition asset"), Asset))
	{
		return false;
	}
	Asset->Definition = PGXVehiclesAutomation::MakeDefinition();

	TestTrue(TEXT("Vehicle asset implements IPGXObservable"), UPGXVehicleDefinitionAsset::StaticClass()->ImplementsInterface(UPGXObservable::StaticClass()));
	const FPGXJsonValue Json = Asset->ToJson();
	TestFalse(TEXT("Vehicle observable ToJson non-empty"), Json.IsEmpty());
	TestTrue(TEXT("Vehicle JSON contains type"), Json.JsonString.Contains(UPGXVehicleDefinitionAsset::StaticClass()->GetName()));
	TestTrue(TEXT("Vehicle JSON contains concrete data"), Json.JsonString.Contains(TEXT("PassengerCapacity")));
	TestEqual(TEXT("Vehicle schema version"), Asset->GetSchemaVersion(), UPGXVehicleDefinitionAsset::SchemaVersion);

	const FPGXSchemaDescriptor Descriptor = Asset->GetSchemaDescriptor();
	TestEqual(TEXT("Vehicle descriptor type"), Descriptor.TypeName, UPGXVehicleDefinitionAsset::StaticClass()->GetFName());
	TestTrue(TEXT("Vehicle descriptor fields"), Descriptor.Fields.Num() > 0);

	const FPGXValidationResult EmptyValidation = Asset->FromJson(FPGXJsonValue());
	TestFalse(TEXT("Vehicle FromJson empty payload fails"), EmptyValidation.bValid);
	TestTrue(TEXT("Vehicle FromJson reports errors"), EmptyValidation.Errors.Num() > 0);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
