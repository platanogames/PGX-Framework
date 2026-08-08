// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

// EN: Automation tests for UPGXVersionControlSettings IPGXObservable behavior.
// ES: Tests de automatizacion del comportamiento IPGXObservable de UPGXVersionControlSettings.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "PGXVersionControlSettings.h"
#include "Observability/PGXObservable.h"
#include "Observability/PGXJsonValue.h"
#include "Observability/PGXSchemaDescriptor.h"
#include "Observability/PGXValidationResult.h"
#include "UObject/Class.h"

namespace
{
// EN: Helper — get CDO of UPGXVersionControlSettings for IPGXObservable contract testing.
// ES: Helper — obtener CDO de UPGXVersionControlSettings para testing del contrato IPGXObservable.
UPGXVersionControlSettings* GetSettingsCDO()
{
	return GetMutableDefault<UPGXVersionControlSettings>();
}
} // namespace

// ============================================================================
// Invariant 1: Class implements IPGXObservable
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXVersionControl_Settings_ClassImplementsObservable,
	"PGX.VersionControl.Settings.ObservableConfig.ClassImplementsObservable",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXVersionControl_Settings_ClassImplementsObservable::RunTest(const FString& /*Parameters*/)
{
	UPGXVersionControlSettings* Settings = GetSettingsCDO();
	if (!TestNotNull(TEXT("CDO must exist"), Settings)) return false;

	IPGXObservable* Observable = Cast<IPGXObservable>(Settings);
	TestNotNull(TEXT("UPGXVersionControlSettings must implement IPGXObservable interface"), Observable);
	return true;
}

// ============================================================================
// Invariant 2: ToJson() returns non-empty FPGXJsonValue
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXVersionControl_Settings_ToJsonNonEmpty,
	"PGX.VersionControl.Settings.ObservableConfig.ToJsonNonEmpty",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXVersionControl_Settings_ToJsonNonEmpty::RunTest(const FString& /*Parameters*/)
{
	UPGXVersionControlSettings* Settings = GetSettingsCDO();
	if (!TestNotNull(TEXT("CDO must exist"), Settings)) return false;

	const FPGXJsonValue Envelope = Settings->ToJson();
	TestFalse(TEXT("ToJson() must return non-empty envelope"), Envelope.IsEmpty());
	return true;
}

// ============================================================================
// Invariant 3: Envelope contains type + version "1.0"
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXVersionControl_Settings_EnvelopeTypeAndVersion,
	"PGX.VersionControl.Settings.ObservableConfig.EnvelopeTypeAndVersion",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXVersionControl_Settings_EnvelopeTypeAndVersion::RunTest(const FString& /*Parameters*/)
{
	UPGXVersionControlSettings* Settings = GetSettingsCDO();
	if (!TestNotNull(TEXT("CDO must exist"), Settings)) return false;

	const FPGXJsonValue Envelope = Settings->ToJson();
	const FString EnvelopeStr = Envelope.JsonString;

	TestTrue(TEXT("Envelope must contain 'type' field"), EnvelopeStr.Contains(TEXT("\"type\"")));
	TestTrue(TEXT("Envelope must contain 'version' field"), EnvelopeStr.Contains(TEXT("\"version\"")));
	TestTrue(TEXT("Envelope version must be '1.0'"), EnvelopeStr.Contains(TEXT("\"1.0\"")));
	return true;
}

// ============================================================================
// Invariant 4: Descriptor.TypeName == StaticClass name
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXVersionControl_Settings_DescriptorTypeName,
	"PGX.VersionControl.Settings.ObservableConfig.DescriptorTypeName",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXVersionControl_Settings_DescriptorTypeName::RunTest(const FString& /*Parameters*/)
{
	UPGXVersionControlSettings* Settings = GetSettingsCDO();
	if (!TestNotNull(TEXT("CDO must exist"), Settings)) return false;

	const FPGXSchemaDescriptor Descriptor = Settings->GetSchemaDescriptor();
	const FName ExpectedName = UPGXVersionControlSettings::StaticClass()->GetFName();
	TestEqual(TEXT("Descriptor.TypeName must match StaticClass name"), Descriptor.TypeName, ExpectedName);
	return true;
}

// ============================================================================
// Invariant 5: Fields > 0 (provider config + validation rules + tag templates expected)
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXVersionControl_Settings_FieldsNonZero,
	"PGX.VersionControl.Settings.ObservableConfig.FieldsNonZero",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXVersionControl_Settings_FieldsNonZero::RunTest(const FString& /*Parameters*/)
{
	UPGXVersionControlSettings* Settings = GetSettingsCDO();
	if (!TestNotNull(TEXT("CDO must exist"), Settings)) return false;

	const FPGXSchemaDescriptor Descriptor = Settings->GetSchemaDescriptor();
	TestTrue(TEXT("Schema descriptor must expose at least one field"), Descriptor.Fields.Num() > 0);
	return true;
}

// ============================================================================
// Invariant 6: FromJson(empty) rejects with validation error
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXVersionControl_Settings_FromJsonEmptyRejects,
	"PGX.VersionControl.Settings.ObservableConfig.FromJsonEmptyRejects",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXVersionControl_Settings_FromJsonEmptyRejects::RunTest(const FString& /*Parameters*/)
{
	UPGXVersionControlSettings* Settings = GetSettingsCDO();
	if (!TestNotNull(TEXT("CDO must exist"), Settings)) return false;

	const FPGXJsonValue EmptyJson;
	const FPGXValidationResult Result = Settings->FromJson(EmptyJson);
	TestFalse(TEXT("FromJson(empty) must NOT be valid"), Result.bValid);
	return true;
}

// ============================================================================
// Invariant 7: ToJson() envelope validates against own descriptor (round-trip)
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXVersionControl_Settings_EnvelopeValidates,
	"PGX.VersionControl.Settings.ObservableConfig.EnvelopeValidates",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXVersionControl_Settings_EnvelopeValidates::RunTest(const FString& /*Parameters*/)
{
	UPGXVersionControlSettings* Settings = GetSettingsCDO();
	if (!TestNotNull(TEXT("CDO must exist"), Settings)) return false;

	const FPGXJsonValue Envelope = Settings->ToJson();
	const FPGXValidationResult Result = Settings->FromJson(Envelope);
	TestTrue(TEXT("Round-trip ToJson() -> FromJson() must validate successfully"), Result.bValid);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
