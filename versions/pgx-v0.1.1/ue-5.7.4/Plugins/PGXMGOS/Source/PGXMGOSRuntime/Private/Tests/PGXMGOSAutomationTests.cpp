// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "PGXGCObserverConfig.h"
#include "Observability/PGXObservable.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

/**
 * EN: IPGXObservable adoption schema validation test
 *     for UPGXGCObserverConfig. Mirror PGXEnvironment / PGXAI / PGXUI / PGXInput
 *     / PGXAudio precedent. NewObject in transient package — no PIE/world fixture.
 * ES: test de validacion de schema para adopcion
 *     IPGXObservable de UPGXGCObserverConfig. Mirror precedent.
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXMGOSGCObserverConfigObservableSchema,
	"PGX.MGOS.GCObserverConfigObservableSchema",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXMGOSGCObserverConfigObservableSchema::RunTest(const FString& /*Parameters*/)
{
	UPGXGCObserverConfig* Config = NewObject<UPGXGCObserverConfig>(
		GetTransientPackage(), UPGXGCObserverConfig::StaticClass(), NAME_None, RF_Transient);
	if (!TestNotNull(TEXT("UPGXGCObserverConfig instance"), Config))
	{
		return false;
	}

	const FName SchemaVersion = Config->GetSchemaVersion();
	TestEqual(TEXT("UPGXGCObserverConfig::GetSchemaVersion is 1.0"), SchemaVersion, FName(TEXT("1.0")));

	const FPGXSchemaDescriptor Descriptor = Config->GetSchemaDescriptor();
	TestEqual(TEXT("Schema TypeName matches class"), Descriptor.TypeName, UPGXGCObserverConfig::StaticClass()->GetFName());
	TestEqual(TEXT("Schema SchemaVersion matches"), Descriptor.SchemaVersion, SchemaVersion);
	TestTrue(TEXT("Schema Fields enumerated (>0)"), Descriptor.Fields.Num() > 0);

	const FPGXJsonValue Envelope = Config->ToJson();
	TestFalse(TEXT("ToJson envelope non-empty"), Envelope.IsEmpty());
	TestTrue(TEXT("ToJson envelope contains type field"),
		Envelope.JsonString.Contains(TEXT("\"type\":\"PGXGCObserverConfig\"")));
	TestTrue(TEXT("ToJson envelope contains 1.0 version"),
		Envelope.JsonString.Contains(TEXT("\"version\":\"1.0\"")));

	const FPGXJsonValue EmptyJson;
	const FPGXValidationResult EmptyResult = Config->FromJson(EmptyJson);
	TestFalse(TEXT("FromJson rejects empty payload"), EmptyResult.bValid);

	const FPGXValidationResult OkResult = Config->FromJson(Envelope);
	TestTrue(TEXT("FromJson accepts non-empty envelope"), OkResult.bValid);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
