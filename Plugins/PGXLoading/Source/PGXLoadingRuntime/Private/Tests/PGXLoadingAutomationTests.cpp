// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "PGXAsyncLoader.h"
#include "PGXStreamingManager.h"
#include "PGXLoadingTypes.h"

#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

/**
 * EN: Smoke tests for the explicit unsupported probes exposed by AsyncLoader and
 *     StreamingManager. Subsystem budget and queue-policy coverage requires a live
 *     PIE fixture and is intentionally outside this file-level test suite.
 *
 * ES: Smoke tests de los probes unsupported de AsyncLoader y StreamingManager.
 *     La cobertura de budgets y queue policy requiere un fixture PIE vivo y queda
 *     fuera de esta suite de tests a nivel de archivo.
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXLoadingAsyncLoaderProbeReturnsUnsupported,
	"PGX.Loading.AsyncLoaderProbeReturnsUnsupported",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXLoadingAsyncLoaderProbeReturnsUnsupported::RunTest(
	const FString& /*Parameters*/)
{
	UPGXAsyncLoader* Loader = NewObject<UPGXAsyncLoader>(
		GetTransientPackage(),
		UPGXAsyncLoader::StaticClass(),
		NAME_None,
		RF_Transient);
	if (!TestNotNull(TEXT("UPGXAsyncLoader instance"), Loader))
	{
		return false;
	}

	TestFalse(
		TEXT("UPGXAsyncLoader::IsImplemented() must return false until real impl lands"),
		Loader->IsImplemented());

	const FString Reason = UPGXAsyncLoader::GetUnsupportedReason();
	TestFalse(
		TEXT("UPGXAsyncLoader::GetUnsupportedReason() must not be empty"),
		Reason.IsEmpty());

	const FPGXLoadingResult Probed = Loader->ProbeUnsupported();
	TestEqual(
		TEXT("UPGXAsyncLoader::ProbeUnsupported() must return EPGXLoadingResultCode::Unsupported"),
		static_cast<int32>(Probed.Code),
		static_cast<int32>(EPGXLoadingResultCode::Unsupported));
	TestFalse(
		TEXT("UPGXAsyncLoader::ProbeUnsupported() reason must not be empty"),
		Probed.Description.IsEmpty());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXLoadingStreamingManagerProbeReturnsUnsupported,
	"PGX.Loading.StreamingManagerProbeReturnsUnsupported",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXLoadingStreamingManagerProbeReturnsUnsupported::RunTest(
	const FString& /*Parameters*/)
{
	UPGXStreamingManager* Manager = NewObject<UPGXStreamingManager>(
		GetTransientPackage(),
		UPGXStreamingManager::StaticClass(),
		NAME_None,
		RF_Transient);
	if (!TestNotNull(TEXT("UPGXStreamingManager instance"), Manager))
	{
		return false;
	}

	TestFalse(
		TEXT("UPGXStreamingManager::IsImplemented() must return false until real impl lands"),
		Manager->IsImplemented());

	const FString Reason = UPGXStreamingManager::GetUnsupportedReason();
	TestFalse(
		TEXT("UPGXStreamingManager::GetUnsupportedReason() must not be empty"),
		Reason.IsEmpty());

	const FPGXLoadingResult Probed = Manager->ProbeUnsupported();
	TestEqual(
		TEXT("UPGXStreamingManager::ProbeUnsupported() must return EPGXLoadingResultCode::Unsupported"),
		static_cast<int32>(Probed.Code),
		static_cast<int32>(EPGXLoadingResultCode::Unsupported));
	TestFalse(
		TEXT("UPGXStreamingManager::ProbeUnsupported() reason must not be empty"),
		Probed.Description.IsEmpty());

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
