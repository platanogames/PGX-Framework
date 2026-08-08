// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#if WITH_DEV_AUTOMATION_TESTS

#include "PGXSaveConfig.h"
#include "Misc/AutomationTest.h"
#include "Observability/PGXObservable.h"

#define PGX_SAVE_OBSERVABILITY_AUTOMATION_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXSave_ObservableConfigAutomationTest,
	"PGX.Save.Observability.ObservableConfig",
	PGX_SAVE_OBSERVABILITY_AUTOMATION_FLAGS)

bool FPGXSave_ObservableConfigAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UPGXSaveConfig* Config = NewObject<UPGXSaveConfig>(
		GetTransientPackage(), UPGXSaveConfig::StaticClass(), FName(TEXT("PGXSave_ObservableConfig")), RF_Transient);
	if (!TestNotNull(TEXT("Save config asset"), Config))
	{
		return false;
	}

	Config->ContextDisplayName = FText::FromString(TEXT("Observable Save Context"));
	Config->MaxSaveSlots = 4;

	TestTrue(TEXT("UPGXSaveConfig implements IPGXObservable"),
		UPGXSaveConfig::StaticClass()->ImplementsInterface(UPGXObservable::StaticClass()));

	const FPGXJsonValue Json = Config->ToJson();
	TestFalse(TEXT("Save config ToJson non-empty"), Json.IsEmpty());
	TestTrue(TEXT("Save config JSON contains type"), Json.JsonString.Contains(TEXT("UPGXSaveConfig")));
	TestTrue(TEXT("Save config JSON contains schema version"), Json.JsonString.Contains(TEXT("\"version\":\"1.0\"")));

	const FPGXSchemaDescriptor Descriptor = Config->GetSchemaDescriptor();
	TestEqual(TEXT("Save config descriptor type"), Descriptor.TypeName, UPGXSaveConfig::StaticClass()->GetFName());
	TestEqual(TEXT("Save config schema version"), Config->GetSchemaVersion(), FName(TEXT("1.0")));
	TestTrue(TEXT("Save config descriptor has reflected fields"), Descriptor.Fields.Num() > 0);

	const FPGXValidationResult EmptyValidation = Config->FromJson(FPGXJsonValue());
	TestFalse(TEXT("Save config FromJson empty payload fails"), EmptyValidation.bValid);
	TestTrue(TEXT("Save config FromJson reports errors"), EmptyValidation.Errors.Num() > 0);

	const FPGXValidationResult EnvelopeValidation = Config->FromJson(Json);
	TestTrue(TEXT("Save config FromJson envelope validates"), EnvelopeValidation.bValid);

	return true;
}

#undef PGX_SAVE_OBSERVABILITY_AUTOMATION_FLAGS

#endif // WITH_DEV_AUTOMATION_TESTS
