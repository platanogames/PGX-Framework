// Copyright PGX Framework. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "PGXInputBuffer.h"
#include "PGXInputConfig.h"
#include "PGXInputContext.h"
#include "PGXInputDeviceManager.h"
#include "PGXInputSettings.h"
#include "PGXInputSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Misc/AutomationTest.h"
#include "Tags/PGXInputTags.h"
#include "Observability/PGXObservable.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

namespace PGXInputAutomation
{
#define PGX_INPUT_AUTOMATION_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

	struct FScopedInputSubsystemFixture
	{
		explicit FScopedInputSubsystemFixture(FAutomationTestBase& InTest)
			: Test(InTest)
		{
			if (!GEngine)
			{
				Test.AddError(TEXT("PGXInput automation setup failed: engine is unavailable."));
				return;
			}

			GameInstance = NewObject<UGameInstance>(
				GEngine,
				UGameInstance::StaticClass(),
				NAME_None,
				RF_Transient);
			if (!GameInstance)
			{
				Test.AddError(TEXT("PGXInput automation setup failed: could not create transient GameInstance."));
				return;
			}

			GameInstance->AddToRoot();
			GameInstance->InitializeStandalone(TEXT("PGXInputAutomationWorld"));
			InputSubsystem = GameInstance->GetSubsystem<UPGXInputSubsystem>();
			if (!InputSubsystem)
			{
				Test.AddError(TEXT("PGXInput automation setup failed: UPGXInputSubsystem missing."));
			}
		}

		~FScopedInputSubsystemFixture()
		{
			if (!GameInstance)
			{
				return;
			}

			GameInstance->Shutdown();
			if (GameInstance->IsRooted())
			{
				GameInstance->RemoveFromRoot();
			}
		}

		FScopedInputSubsystemFixture(const FScopedInputSubsystemFixture&) = delete;
		FScopedInputSubsystemFixture& operator=(const FScopedInputSubsystemFixture&) = delete;

		UPGXInputSubsystem* Get() const { return InputSubsystem; }
		UPGXInputDeviceManager* GetDeviceManager() const
		{
			return GameInstance ? GameInstance->GetSubsystem<UPGXInputDeviceManager>() : nullptr;
		}

	private:
		FAutomationTestBase& Test;
		UGameInstance* GameInstance = nullptr;
		UPGXInputSubsystem* InputSubsystem = nullptr;
	};

	UPGXInputConfig* MakeConfig(const TCHAR* Name, int32 Capacity = 16, float WindowSeconds = 0.15f)
	{
		UPGXInputConfig* Config = NewObject<UPGXInputConfig>(GetTransientPackage(), UPGXInputConfig::StaticClass(), FName(Name), RF_Transient);
		Config->InputBufferCapacity = Capacity;
		Config->InputBufferWindowSeconds = WindowSeconds;
		return Config;
	}

	UPGXInputContext* MakeContext(const TCHAR* Name, FGameplayTag ContextTag, int32 Priority, EPGXInputContextActivationMode ActivationMode = EPGXInputContextActivationMode::Additive)
	{
		UPGXInputContext* Context = NewObject<UPGXInputContext>(GetTransientPackage(), UPGXInputContext::StaticClass(), FName(Name), RF_Transient);
		Context->ContextTag = ContextTag;
		Context->ContextName = FName(Name);
		Context->Priority = Priority;
		Context->ActivationMode = ActivationMode;
		return Context;
	}

	UInputMappingContext* MakeMappingContext(const TCHAR* Name)
	{
		return NewObject<UInputMappingContext>(GetTransientPackage(), UInputMappingContext::StaticClass(), FName(Name), RF_Transient);
	}
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInput_SettingsAccessorAutomationTest,
	"PGX.Input.SettingsAccessor", PGX_INPUT_AUTOMATION_FLAGS)
bool FPGXInput_SettingsAccessorAutomationTest::RunTest(const FString& Parameters)
{
	const UPGXInputSettings* Settings = GetDefault<UPGXInputSettings>();
	TestNotNull(TEXT("SettingsAccessor default settings"), Settings);
	if (!Settings)
	{
		return false;
	}

	TestEqual(TEXT("SettingsAccessor category"), Settings->GetCategoryName(), FName(TEXT("PGX")));
	TestTrue(TEXT("SettingsAccessor default discovery mode"), Settings->DiscoveryMode == EPGXInputDiscoveryMode::AssetRegistryScan);
	TestFalse(TEXT("SettingsAccessor ActiveConfig default empty"), Settings->ActiveConfig.IsValid());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInput_ContextActivationPrioritySortAutomationTest,
	"PGX.Input.preview.ContextActivationPrioritySort", PGX_INPUT_AUTOMATION_FLAGS)
bool FPGXInput_ContextActivationPrioritySortAutomationTest::RunTest(const FString& Parameters)
{
	PGXInputAutomation::FScopedInputSubsystemFixture Fixture(*this);
	UPGXInputSubsystem* Input = Fixture.Get();
	if (!Input)
	{
		return true;
	}
	Input->InjectTestInputConfig(PGXInputAutomation::MakeConfig(TEXT("PGXInput_Automation_Priority")));

	UPGXInputContext* DefaultContext = PGXInputAutomation::MakeContext(TEXT("PGXInput_Automation_Default"), TAG_PGX_Input_Context_Default.GetTag(), 5);
	UPGXInputContext* MenuContext = PGXInputAutomation::MakeContext(TEXT("PGXInput_Automation_Menu"), TAG_PGX_Input_Context_Menu.GetTag(), 10);
	UPGXInputContext* GameplayContext = PGXInputAutomation::MakeContext(TEXT("PGXInput_Automation_Gameplay"), TAG_PGX_Input_Context_Gameplay.GetTag(), 20);
	Input->InjectTestContext(DefaultContext);
	Input->InjectTestContext(MenuContext);
	Input->InjectTestContext(GameplayContext);

	Input->ActivateContext(MenuContext->ContextTag);
	Input->ActivateContext(GameplayContext->ContextTag);
	Input->ActivateContext(DefaultContext->ContextTag);

	const TArray<FPGXActiveInputContextEntry> ActiveContexts = Input->GetActiveContexts();
	TestEqual(TEXT("ContextActivationPrioritySort active count"), ActiveContexts.Num(), 3);
	if (ActiveContexts.Num() == 3)
	{
		TestTrue(TEXT("Highest priority context first"), ActiveContexts[0].ContextTag == TAG_PGX_Input_Context_Gameplay.GetTag());
		TestTrue(TEXT("Middle priority context second"), ActiveContexts[1].ContextTag == TAG_PGX_Input_Context_Menu.GetTag());
		TestTrue(TEXT("Lowest priority context third"), ActiveContexts[2].ContextTag == TAG_PGX_Input_Context_Default.GetTag());
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInput_ContextActivationAlreadyActiveAutomationTest,
	"PGX.Input.preview.ContextActivationAlreadyActive", PGX_INPUT_AUTOMATION_FLAGS)
bool FPGXInput_ContextActivationAlreadyActiveAutomationTest::RunTest(const FString& Parameters)
{
	PGXInputAutomation::FScopedInputSubsystemFixture Fixture(*this);
	UPGXInputSubsystem* Input = Fixture.Get();
	if (!Input)
	{
		return true;
	}
	Input->InjectTestInputConfig(PGXInputAutomation::MakeConfig(TEXT("PGXInput_Automation_AlreadyActive")));
	UPGXInputContext* GameplayContext = PGXInputAutomation::MakeContext(TEXT("PGXInput_Automation_Gameplay_AlreadyActive"), TAG_PGX_Input_Context_Gameplay.GetTag(), 10);
	Input->InjectTestContext(GameplayContext);

	const FPGXInputContextResult FirstResult = Input->ActivateContext(GameplayContext->ContextTag);
	const FPGXInputContextResult SecondResult = Input->ActivateContext(GameplayContext->ContextTag);

	TestTrue(TEXT("First activation succeeds"), FirstResult.bSuccess && FirstResult.Code == EPGXInputContextResultCode::Success);
	TestTrue(TEXT("Second activation returns typed AlreadyActive"), SecondResult.bSuccess && SecondResult.Code == EPGXInputContextResultCode::AlreadyActive);
	TestEqual(TEXT("AlreadyActive does not duplicate stack entry"), Input->GetActiveContextCount(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInput_ExclusiveContextPrunesLowerPriorityAutomationTest,
	"PGX.Input.preview.ExclusiveContextPrunesLowerPriority", PGX_INPUT_AUTOMATION_FLAGS)
bool FPGXInput_ExclusiveContextPrunesLowerPriorityAutomationTest::RunTest(const FString& Parameters)
{
	PGXInputAutomation::FScopedInputSubsystemFixture Fixture(*this);
	UPGXInputSubsystem* Input = Fixture.Get();
	if (!Input)
	{
		return true;
	}
	Input->InjectTestInputConfig(PGXInputAutomation::MakeConfig(TEXT("PGXInput_Automation_Exclusive")));
	UPGXInputContext* GameplayContext = PGXInputAutomation::MakeContext(TEXT("PGXInput_Automation_Gameplay_Exclusive"), TAG_PGX_Input_Context_Gameplay.GetTag(), 5);
	UPGXInputContext* MenuContext = PGXInputAutomation::MakeContext(TEXT("PGXInput_Automation_Menu_Exclusive"), TAG_PGX_Input_Context_Menu.GetTag(), 10, EPGXInputContextActivationMode::Exclusive);
	Input->InjectTestContext(GameplayContext);
	Input->InjectTestContext(MenuContext);

	Input->ActivateContext(GameplayContext->ContextTag);
	const FPGXInputContextResult MenuResult = Input->ActivateContext(MenuContext->ContextTag);

	TestTrue(TEXT("Exclusive activation succeeds"), MenuResult.bSuccess);
	TestFalse(TEXT("Exclusive activation prunes lower-priority context"), Input->IsContextActive(GameplayContext->ContextTag));
	TestTrue(TEXT("Exclusive context remains active"), Input->IsContextActive(MenuContext->ContextTag));
	TestEqual(TEXT("Exclusive stack has one entry"), Input->GetActiveContextCount(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInput_ExclusiveContextKeepsHigherPriorityAutomationTest,
	"PGX.Input.preview.ExclusiveContextKeepsHigherPriority", PGX_INPUT_AUTOMATION_FLAGS)
bool FPGXInput_ExclusiveContextKeepsHigherPriorityAutomationTest::RunTest(const FString& Parameters)
{
	PGXInputAutomation::FScopedInputSubsystemFixture Fixture(*this);
	UPGXInputSubsystem* Input = Fixture.Get();
	if (!Input)
	{
		return true;
	}
	Input->InjectTestInputConfig(PGXInputAutomation::MakeConfig(TEXT("PGXInput_Automation_ExclusiveKeepsHigher")));
	UPGXInputContext* GameplayContext = PGXInputAutomation::MakeContext(TEXT("PGXInput_Automation_Gameplay_HigherPriority"), TAG_PGX_Input_Context_Gameplay.GetTag(), 20);
	UPGXInputContext* MenuContext = PGXInputAutomation::MakeContext(TEXT("PGXInput_Automation_Menu_LowerExclusive"), TAG_PGX_Input_Context_Menu.GetTag(), 10, EPGXInputContextActivationMode::Exclusive);
	Input->InjectTestContext(GameplayContext);
	Input->InjectTestContext(MenuContext);

	Input->ActivateContext(GameplayContext->ContextTag);
	const FPGXInputContextResult MenuResult = Input->ActivateContext(MenuContext->ContextTag);

	TestTrue(TEXT("Exclusive activation succeeds"), MenuResult.bSuccess);
	TestTrue(TEXT("Exclusive activation keeps higher-priority additive context"), Input->IsContextActive(GameplayContext->ContextTag));
	TestTrue(TEXT("Exclusive context remains active"), Input->IsContextActive(MenuContext->ContextTag));
	TestEqual(TEXT("Exclusive stack keeps higher-priority context plus exclusive context"), Input->GetActiveContextCount(), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInput_DeactivateContextAutomationTest,
	"PGX.Input.preview.DeactivateContext", PGX_INPUT_AUTOMATION_FLAGS)
bool FPGXInput_DeactivateContextAutomationTest::RunTest(const FString& Parameters)
{
	PGXInputAutomation::FScopedInputSubsystemFixture Fixture(*this);
	UPGXInputSubsystem* Input = Fixture.Get();
	if (!Input)
	{
		return true;
	}
	Input->InjectTestInputConfig(PGXInputAutomation::MakeConfig(TEXT("PGXInput_Automation_Deactivate")));
	UPGXInputContext* MenuContext = PGXInputAutomation::MakeContext(TEXT("PGXInput_Automation_Menu_Deactivate"), TAG_PGX_Input_Context_Menu.GetTag(), 10);
	Input->InjectTestContext(MenuContext);
	Input->ActivateContext(MenuContext->ContextTag);

	const FPGXInputContextResult DeactivateResult = Input->DeactivateContext(MenuContext->ContextTag);
	const FPGXInputContextResult RepeatResult = Input->DeactivateContext(MenuContext->ContextTag);

	TestTrue(TEXT("Deactivate succeeds"), DeactivateResult.bSuccess && DeactivateResult.Code == EPGXInputContextResultCode::Success);
	TestTrue(TEXT("Deactivate removes active context"), !Input->IsContextActive(MenuContext->ContextTag));
	TestTrue(TEXT("Repeat deactivate returns AlreadyInactive"), RepeatResult.bSuccess && RepeatResult.Code == EPGXInputContextResultCode::AlreadyInactive);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInput_LocalPlayerApplyMissingMappingTypedFailureAutomationTest,
	"PGX.Input.preview.LocalPlayerApplyMissingMappingTypedFailure", PGX_INPUT_AUTOMATION_FLAGS)
bool FPGXInput_LocalPlayerApplyMissingMappingTypedFailureAutomationTest::RunTest(const FString& Parameters)
{
	PGXInputAutomation::FScopedInputSubsystemFixture Fixture(*this);
	UPGXInputSubsystem* Input = Fixture.Get();
	if (!Input)
	{
		return true;
	}
	Input->InjectTestInputConfig(PGXInputAutomation::MakeConfig(TEXT("PGXInput_Automation_LocalPlayerMissingMapping")));
	UPGXInputContext* GameplayContext = PGXInputAutomation::MakeContext(TEXT("PGXInput_Automation_Gameplay_NoMapping"), TAG_PGX_Input_Context_Gameplay.GetTag(), 10);
	Input->InjectTestContext(GameplayContext);

	const FPGXInputContextResult Result = Input->ActivateContextForLocalPlayer(GameplayContext->ContextTag, nullptr);

	TestFalse(TEXT("Missing MappingContext does not silently succeed"), Result.bSuccess);
	TestTrue(TEXT("Missing MappingContext returns typed code"), Result.Code == EPGXInputContextResultCode::MappingContextMissing);
	TestEqual(TEXT("Missing MappingContext does not mutate PGX stack"), Input->GetActiveContextCount(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInput_LocalPlayerApplyMissingPlayerTypedFailureAutomationTest,
	"PGX.Input.preview.LocalPlayerApplyMissingPlayerTypedFailure", PGX_INPUT_AUTOMATION_FLAGS)
bool FPGXInput_LocalPlayerApplyMissingPlayerTypedFailureAutomationTest::RunTest(const FString& Parameters)
{
	PGXInputAutomation::FScopedInputSubsystemFixture Fixture(*this);
	UPGXInputSubsystem* Input = Fixture.Get();
	if (!Input)
	{
		return true;
	}
	Input->InjectTestInputConfig(PGXInputAutomation::MakeConfig(TEXT("PGXInput_Automation_LocalPlayerMissingPlayer")));
	UPGXInputContext* GameplayContext = PGXInputAutomation::MakeContext(TEXT("PGXInput_Automation_Gameplay_WithMapping"), TAG_PGX_Input_Context_Gameplay.GetTag(), 10);
	GameplayContext->MappingContext = PGXInputAutomation::MakeMappingContext(TEXT("PGXInput_Automation_Mapping_Gameplay"));
	Input->InjectTestContext(GameplayContext);

	const FPGXInputContextResult Result = Input->ActivateContextForLocalPlayer(GameplayContext->ContextTag, nullptr);

	TestFalse(TEXT("Missing LocalPlayer does not silently succeed"), Result.bSuccess);
	TestTrue(TEXT("Missing LocalPlayer returns typed code"), Result.Code == EPGXInputContextResultCode::LocalPlayerNotReady);
	TestEqual(TEXT("Missing LocalPlayer does not mutate PGX stack"), Input->GetActiveContextCount(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInput_LocalPlayerRemoveMissingPlayerTypedFailureAutomationTest,
	"PGX.Input.preview.LocalPlayerRemoveMissingPlayerTypedFailure", PGX_INPUT_AUTOMATION_FLAGS)
bool FPGXInput_LocalPlayerRemoveMissingPlayerTypedFailureAutomationTest::RunTest(const FString& Parameters)
{
	PGXInputAutomation::FScopedInputSubsystemFixture Fixture(*this);
	UPGXInputSubsystem* Input = Fixture.Get();
	if (!Input)
	{
		return true;
	}
	Input->InjectTestInputConfig(PGXInputAutomation::MakeConfig(TEXT("PGXInput_Automation_LocalPlayerRemoveMissingPlayer")));
	UPGXInputContext* GameplayContext = PGXInputAutomation::MakeContext(TEXT("PGXInput_Automation_Gameplay_Remove"), TAG_PGX_Input_Context_Gameplay.GetTag(), 10);
	GameplayContext->MappingContext = PGXInputAutomation::MakeMappingContext(TEXT("PGXInput_Automation_Mapping_Remove"));
	Input->InjectTestContext(GameplayContext);
	Input->ActivateContext(GameplayContext->ContextTag);

	const FPGXInputContextResult Result = Input->DeactivateContextForLocalPlayer(GameplayContext->ContextTag, nullptr);

	TestFalse(TEXT("Missing LocalPlayer remove does not silently succeed"), Result.bSuccess);
	TestTrue(TEXT("Missing LocalPlayer remove returns typed code"), Result.Code == EPGXInputContextResultCode::LocalPlayerNotReady);
	TestTrue(TEXT("Missing LocalPlayer remove leaves PGX stack unchanged"), Input->IsContextActive(GameplayContext->ContextTag));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInput_InvalidContextFailureVisibleTypedAutomationTest,
	"PGX.Input.preview.InvalidContextFailureVisibleTyped", PGX_INPUT_AUTOMATION_FLAGS)
bool FPGXInput_InvalidContextFailureVisibleTypedAutomationTest::RunTest(const FString& Parameters)
{
	PGXInputAutomation::FScopedInputSubsystemFixture Fixture(*this);
	UPGXInputSubsystem* Input = Fixture.Get();
	if (!Input)
	{
		return true;
	}
	Input->InjectTestInputConfig(PGXInputAutomation::MakeConfig(TEXT("PGXInput_Automation_Invalid")));

	const FPGXInputContextResult InvalidTagResult = Input->ActivateContext(FGameplayTag::EmptyTag);
	const FPGXInputContextResult MissingContextResult = Input->ActivateContext(TAG_PGX_Input_Context_Gameplay.GetTag());

	TestFalse(TEXT("Invalid tag does not silently succeed"), InvalidTagResult.bSuccess);
	TestTrue(TEXT("Invalid tag returns typed code"), InvalidTagResult.Code == EPGXInputContextResultCode::InvalidTag);
	TestFalse(TEXT("Missing context does not silently succeed"), MissingContextResult.bSuccess);
	TestTrue(TEXT("Missing context returns typed code"), MissingContextResult.Code == EPGXInputContextResultCode::ContextNotFound);
	TestFalse(TEXT("Failure messages are visible"), InvalidTagResult.Message.IsEmpty() || MissingContextResult.Message.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInput_InputBufferBoundedByConfigAutomationTest,
	"PGX.Input.preview.InputBufferBoundedByConfig", PGX_INPUT_AUTOMATION_FLAGS)
bool FPGXInput_InputBufferBoundedByConfigAutomationTest::RunTest(const FString& Parameters)
{
	UPGXInputBuffer* Buffer = NewObject<UPGXInputBuffer>(GetTransientPackage(), UPGXInputBuffer::StaticClass(), NAME_None, RF_Transient);
	Buffer->Configure(3, 10.0);
	for (int32 Index = 0; Index < 5; ++Index)
	{
		Buffer->RecordInput(TAG_PGX_Input_Action_Jump.GetTag(), FVector::ZeroVector, static_cast<double>(Index));
	}

	const TArray<FPGXInputBufferEntry> Entries = Buffer->GetBufferedInputs();
	TestEqual(TEXT("InputBufferBoundedByConfig keeps configured capacity"), Entries.Num(), 3);
	if (Entries.Num() == 3)
	{
		TestTrue(TEXT("InputBufferBoundedByConfig drops oldest entry"), FMath::IsNearlyEqual(Entries[0].Timestamp, 2.0));
		TestTrue(TEXT("InputBufferBoundedByConfig keeps newest entry"), FMath::IsNearlyEqual(Entries[2].Timestamp, 4.0));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInput_InputBufferWindowExpirationAutomationTest,
	"PGX.Input.preview.InputBufferWindowExpiration", PGX_INPUT_AUTOMATION_FLAGS)
bool FPGXInput_InputBufferWindowExpirationAutomationTest::RunTest(const FString& Parameters)
{
	UPGXInputBuffer* Buffer = NewObject<UPGXInputBuffer>(GetTransientPackage(), UPGXInputBuffer::StaticClass(), NAME_None, RF_Transient);
	Buffer->Configure(8, 0.25);
	Buffer->RecordInput(TAG_PGX_Input_Action_Jump.GetTag(), FVector::ZeroVector, 1.0);

	TestTrue(TEXT("InputBufferWindowExpiration recent entry visible inside window"), Buffer->ContainsRecentInput(TAG_PGX_Input_Action_Jump.GetTag(), 1.2));
	TestFalse(TEXT("InputBufferWindowExpiration old entry hidden outside window"), Buffer->ContainsRecentInput(TAG_PGX_Input_Action_Jump.GetTag(), 1.3));
	TestFalse(TEXT("InputBufferWindowExpiration cannot consume expired entry"), Buffer->ConsumeRecentInput(TAG_PGX_Input_Action_Jump.GetTag(), 1.3));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInput_DeviceOverrideForceClearAutomationTest,
	"PGX.Input.preview.DeviceOverrideForceClear", PGX_INPUT_AUTOMATION_FLAGS)
bool FPGXInput_DeviceOverrideForceClearAutomationTest::RunTest(const FString& Parameters)
{
	PGXInputAutomation::FScopedInputSubsystemFixture Fixture(*this);
	UPGXInputDeviceManager* DeviceManager = Fixture.GetDeviceManager();
	if (!TestNotNull(TEXT("Device manager fixture"), DeviceManager))
	{
		return true;
	}
	DeviceManager->SetActiveDeviceType(EPGXInputDeviceType::KeyboardMouse);
	DeviceManager->ForceDeviceType(EPGXInputDeviceType::Gamepad);

	TestTrue(TEXT("Device override is marked active"), DeviceManager->HasDeviceOverride());
	TestTrue(TEXT("Device override resolves gamepad"), DeviceManager->IsUsingGamepad());

	DeviceManager->SetActiveDeviceType(EPGXInputDeviceType::KeyboardMouse);
	TestTrue(TEXT("Forced device ignores observed device changes"), DeviceManager->IsUsingGamepad());

	DeviceManager->ClearDeviceOverride();
	DeviceManager->SetActiveDeviceType(EPGXInputDeviceType::KeyboardMouse);
	TestFalse(TEXT("Device override clears"), DeviceManager->HasDeviceOverride());
	TestTrue(TEXT("Device can return to keyboard/mouse after clear"), DeviceManager->IsUsingKeyboardMouse());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInput_ConfigDrivesBufferPolicyAutomationTest,
	"PGX.Input.preview.ConfigDrivesBufferPolicy", PGX_INPUT_AUTOMATION_FLAGS)
bool FPGXInput_ConfigDrivesBufferPolicyAutomationTest::RunTest(const FString& Parameters)
{
	PGXInputAutomation::FScopedInputSubsystemFixture Fixture(*this);
	UPGXInputSubsystem* Input = Fixture.Get();
	if (!Input)
	{
		return true;
	}
	UPGXInputConfig* Config = PGXInputAutomation::MakeConfig(TEXT("PGXInput_Automation_BufferPolicy"), 2, 0.5f);
	Input->InjectTestInputConfig(Config);

	UPGXInputBuffer* Buffer = Input->GetInputBuffer();
	TestTrue(TEXT("ConfigDrivesBufferPolicy buffer exists"), Buffer != nullptr);
	if (!Buffer)
	{
		return true;
	}

	TestEqual(TEXT("ConfigDrivesBufferPolicy capacity"), Buffer->GetCapacity(), 2);
	TestTrue(TEXT("ConfigDrivesBufferPolicy window"), FMath::IsNearlyEqual(Buffer->GetWindowSeconds(), 0.5));
	Buffer->RecordInput(TAG_PGX_Input_Action_Move.GetTag(), FVector::ZeroVector, 0.0);
	Buffer->RecordInput(TAG_PGX_Input_Action_Look.GetTag(), FVector::ZeroVector, 0.1);
	Buffer->RecordInput(TAG_PGX_Input_Action_Jump.GetTag(), FVector::ZeroVector, 0.2);
	TestEqual(TEXT("ConfigDrivesBufferPolicy enforces capacity"), Buffer->Num(), 2);
	return true;
}

// ============================================================================
// EN: Observability support — IPGXObservable schema validation test for
//     UPGXInputConfig. Mirror PGXEnvironment 8.3.B / PGXAI / PGXUI 8.3.C
//     reference. NewObject in transient package — no PIE/world fixture.
// ES: Observability support — test de validacion de schema para adopcion
//     IPGXObservable de UPGXInputConfig. Mirror referencia PGXEnvironment
//     8.3.B / PGXAI / PGXUI 8.3.C. NewObject en transient package — sin
//     fixture PIE/world.
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXInputConfigObservableSchema,
	"PGX.Input.ConfigObservableSchema",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXInputConfigObservableSchema::RunTest(const FString& /*Parameters*/)
{
	UPGXInputConfig* Config = NewObject<UPGXInputConfig>(
		GetTransientPackage(), UPGXInputConfig::StaticClass(), NAME_None, RF_Transient);
	if (!TestNotNull(TEXT("UPGXInputConfig instance"), Config))
	{
		return false;
	}

	const FName SchemaVersion = Config->GetSchemaVersion();
	TestEqual(TEXT("UPGXInputConfig::GetSchemaVersion is 1.0"), SchemaVersion, FName(TEXT("1.0")));

	const FPGXSchemaDescriptor Descriptor = Config->GetSchemaDescriptor();
	TestEqual(TEXT("Schema TypeName matches class"), Descriptor.TypeName, UPGXInputConfig::StaticClass()->GetFName());
	TestEqual(TEXT("Schema SchemaVersion matches"), Descriptor.SchemaVersion, SchemaVersion);
	TestTrue(TEXT("Schema Fields enumerated (>0)"), Descriptor.Fields.Num() > 0);

	const FPGXJsonValue Envelope = Config->ToJson();
	TestFalse(TEXT("ToJson envelope non-empty"), Envelope.IsEmpty());
	TestTrue(
		TEXT("ToJson envelope contains type field"),
		Envelope.JsonString.Contains(TEXT("\"type\":\"PGXInputConfig\"")));
	TestTrue(
		TEXT("ToJson envelope contains 1.0 version"),
		Envelope.JsonString.Contains(TEXT("\"version\":\"1.0\"")));

	const FPGXJsonValue EmptyJson;
	const FPGXValidationResult EmptyResult = Config->FromJson(EmptyJson);
	TestFalse(TEXT("FromJson rejects empty payload"), EmptyResult.bValid);

	const FPGXValidationResult OkResult = Config->FromJson(Envelope);
	TestTrue(TEXT("FromJson accepts non-empty envelope"), OkResult.bValid);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
