// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

// EN: Automation tests for IPGXObservable behavior across loading and level-flow settings.
// ES: Tests de automatizacion del comportamiento IPGXObservable en settings de carga y flujo de niveles.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "PGXLoadingSettings.h"
#include "Observability/PGXObservable.h"
#include "Observability/PGXJsonValue.h"
#include "Observability/PGXSchemaDescriptor.h"
#include "Observability/PGXValidationResult.h"
#include "UObject/Class.h"

namespace
{
// EN: Helper — get CDO of UPGXLoadingSettings.
// ES: Helper — obtener CDO de UPGXLoadingSettings.
UPGXLoadingSettings* GetLoadingSettingsCDO()
{
	return GetMutableDefault<UPGXLoadingSettings>();
}
} // namespace

// ============================================================================
// UPGXLoadingSettings observable contract
// ============================================================================

// Invariant 1: Class implements IPGXObservable
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXLoading_ObservableSettings_ClassImplementsObservable,
	"PGX.Loading.ObservableSettings.ClassImplementsObservable",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXLoading_ObservableSettings_ClassImplementsObservable::RunTest(const FString& /*Parameters*/)
{
	UPGXLoadingSettings* Settings = GetLoadingSettingsCDO();
	if (!TestNotNull(TEXT("CDO must exist"), Settings)) return false;

	IPGXObservable* Observable = Cast<IPGXObservable>(Settings);
	TestNotNull(TEXT("UPGXLoadingSettings must implement IPGXObservable"), Observable);
	return true;
}

// Invariant 2: ToJson() returns non-empty FPGXJsonValue
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXLoading_ObservableSettings_ToJsonNonEmpty,
	"PGX.Loading.ObservableSettings.ToJsonNonEmpty",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXLoading_ObservableSettings_ToJsonNonEmpty::RunTest(const FString& /*Parameters*/)
{
	UPGXLoadingSettings* Settings = GetLoadingSettingsCDO();
	if (!TestNotNull(TEXT("CDO must exist"), Settings)) return false;

	const FPGXJsonValue Envelope = Settings->ToJson();
	TestFalse(TEXT("ToJson() must return non-empty envelope"), Envelope.IsEmpty());
	return true;
}

// Invariant 3: Envelope contains type + version "1.0"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXLoading_ObservableSettings_EnvelopeTypeAndVersion,
	"PGX.Loading.ObservableSettings.EnvelopeTypeAndVersion",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXLoading_ObservableSettings_EnvelopeTypeAndVersion::RunTest(const FString& /*Parameters*/)
{
	UPGXLoadingSettings* Settings = GetLoadingSettingsCDO();
	if (!TestNotNull(TEXT("CDO must exist"), Settings)) return false;

	const FPGXJsonValue Envelope = Settings->ToJson();
	const FString& EnvelopeStr = Envelope.JsonString;

	TestTrue(TEXT("Envelope must contain 'type' field"), EnvelopeStr.Contains(TEXT("\"type\"")));
	TestTrue(TEXT("Envelope must contain 'version' field"), EnvelopeStr.Contains(TEXT("\"version\"")));
	TestTrue(TEXT("Envelope version must be '1.0'"), EnvelopeStr.Contains(TEXT("\"1.0\"")));
	return true;
}

// Invariant 4: Descriptor.TypeName == StaticClass name
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXLoading_ObservableSettings_DescriptorTypeName,
	"PGX.Loading.ObservableSettings.DescriptorTypeName",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXLoading_ObservableSettings_DescriptorTypeName::RunTest(const FString& /*Parameters*/)
{
	UPGXLoadingSettings* Settings = GetLoadingSettingsCDO();
	if (!TestNotNull(TEXT("CDO must exist"), Settings)) return false;

	const FPGXSchemaDescriptor Descriptor = Settings->GetSchemaDescriptor();
	const FName ExpectedName = UPGXLoadingSettings::StaticClass()->GetFName();
	TestEqual(TEXT("Descriptor.TypeName must match StaticClass name"), Descriptor.TypeName, ExpectedName);
	return true;
}

// Invariant 5: Fields > 0
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXLoading_ObservableSettings_FieldsNonZero,
	"PGX.Loading.ObservableSettings.FieldsNonZero",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXLoading_ObservableSettings_FieldsNonZero::RunTest(const FString& /*Parameters*/)
{
	UPGXLoadingSettings* Settings = GetLoadingSettingsCDO();
	if (!TestNotNull(TEXT("CDO must exist"), Settings)) return false;

	const FPGXSchemaDescriptor Descriptor = Settings->GetSchemaDescriptor();
	TestTrue(TEXT("Schema descriptor must expose at least one field"), Descriptor.Fields.Num() > 0);
	return true;
}

// Invariant 6: FromJson(empty) rejects
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXLoading_ObservableSettings_FromJsonEmptyRejects,
	"PGX.Loading.ObservableSettings.FromJsonEmptyRejects",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXLoading_ObservableSettings_FromJsonEmptyRejects::RunTest(const FString& /*Parameters*/)
{
	UPGXLoadingSettings* Settings = GetLoadingSettingsCDO();
	if (!TestNotNull(TEXT("CDO must exist"), Settings)) return false;

	const FPGXJsonValue EmptyJson;
	const FPGXValidationResult Result = Settings->FromJson(EmptyJson);
	TestFalse(TEXT("FromJson(empty) must NOT be valid"), Result.bValid);
	return true;
}

// Invariant 7: Round-trip validates
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXLoading_ObservableSettings_EnvelopeValidates,
	"PGX.Loading.ObservableSettings.EnvelopeValidates",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXLoading_ObservableSettings_EnvelopeValidates::RunTest(const FString& /*Parameters*/)
{
	UPGXLoadingSettings* Settings = GetLoadingSettingsCDO();
	if (!TestNotNull(TEXT("CDO must exist"), Settings)) return false;

	const FPGXJsonValue Envelope = Settings->ToJson();
	const FPGXValidationResult Result = Settings->FromJson(Envelope);
	TestTrue(TEXT("Round-trip ToJson() -> FromJson() must validate successfully"), Result.bValid);
	return true;
}


#endif // WITH_DEV_AUTOMATION_TESTS
