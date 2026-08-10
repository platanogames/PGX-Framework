// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "PGXAsyncLoader.h"
#include "PGXStreamingManager.h"
#include "PGXLoadingTypes.h"
#include "PGXLoadingSubsystem.h"
#include "PGXLevelFlowSubsystem.h"
#include "PGXLoadingSettings.h"
#include "PGXLoadingProfile.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXLoadingOpenLevelOptionsNormalization,
	"PGX.Loading.OpenLevelOptionsNormalization",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FPGXLoadingOpenLevelOptionsNormalization::RunTest(const FString&)
{
	FString Out;
	TestTrue(TEXT("empty accepted"), UPGXLevelFlowSubsystem::NormalizeOpenLevelOptionsForTesting(TEXT(""), Out));
	TestTrue(TEXT("empty remains empty"), Out.IsEmpty());
	TestTrue(TEXT("plain option accepted"), UPGXLevelFlowSubsystem::NormalizeOpenLevelOptionsForTesting(TEXT("game=/Script/Test.GameMode"), Out));
	TestEqual(TEXT("plain option preserved"), Out, FString(TEXT("game=/Script/Test.GameMode")));
	TestTrue(TEXT("leading question mark accepted"), UPGXLevelFlowSubsystem::NormalizeOpenLevelOptionsForTesting(TEXT("?game=/Script/Test.GameMode"), Out));
	TestEqual(TEXT("leading question mark normalized"), Out, FString(TEXT("game=/Script/Test.GameMode")));
	TestFalse(TEXT("control rejected"), UPGXLevelFlowSubsystem::NormalizeOpenLevelOptionsForTesting(TEXT("game=x\nlisten"), Out));
	TestFalse(TEXT("ambiguous separator rejected"), UPGXLevelFlowSubsystem::NormalizeOpenLevelOptionsForTesting(TEXT("game=x#Map"), Out));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXLoadingCompletionSignalIdempotence,
	"PGX.Loading.CompletionSignalIdempotence",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FPGXLoadingCompletionSignalIdempotence::RunTest(const FString&)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UPGXLoadingSubsystem* Loading = NewObject<UPGXLoadingSubsystem>(GameInstance);
	for (const EPGXLoadingScreenState State : { EPGXLoadingScreenState::Preparing, EPGXLoadingScreenState::FadingIn,
		EPGXLoadingScreenState::Active, EPGXLoadingScreenState::WaitingClose })
	{
		Loading->SetStateForTesting(State);
		TestTrue(TEXT("first completion accepted"), Loading->SignalCompletionForTesting());
		TestTrue(TEXT("completion flag set"), Loading->WasCompletionReceivedForTesting());
		TestEqual(TEXT("frame counter reset once"), Loading->GetPostLoadFrameCountForTesting(), 0);
		TestFalse(TEXT("duplicate completion ignored"), Loading->SignalCompletionForTesting());
	}
	Loading->SetStateForTesting(EPGXLoadingScreenState::Idle);
	TestFalse(TEXT("idle completion ignored"), Loading->SignalCompletionForTesting());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXLoadingProfileTableResolution,
	"PGX.Loading.ProfileTableResolution",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPGXLoadingProfileTableResolution::RunTest(const FString&)
{
	UPGXLoadingSettings* Settings = GetMutableDefault<UPGXLoadingSettings>();
	const TSoftObjectPtr<UDataTable> Original = Settings->LoadingProfileTable;
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UPGXLoadingSubsystem* Loading = NewObject<UPGXLoadingSubsystem>(GameInstance);
	UPGXLoadingProfile* Profile = NewObject<UPGXLoadingProfile>(GetTransientPackage(), TEXT("PGXLoading_TableProfile"));
	UDataTable* Table = NewObject<UDataTable>(GetTransientPackage(), TEXT("PGXLoading_ProfileTable"));
	Table->RowStruct = FPGXLoadingProfileRow::StaticStruct();
	FPGXLoadingProfileRow Valid; Valid.ProfileRef = Profile;
	FPGXLoadingProfileRow Duplicate; Duplicate.ProfileRef = Profile;
	FPGXLoadingProfileRow Invalid;
	Table->AddRow(TEXT("B_Duplicate"), Duplicate);
	Table->AddRow(TEXT("A_Valid"), Valid);
	Table->AddRow(TEXT("C_Invalid"), Invalid);
	Settings->LoadingProfileTable = Table;
	Loading->RediscoverProfilesForTesting();
	TestEqual(TEXT("valid rows deduplicate deterministically"), Loading->GetDiscoveredProfileCount(), 1);
	TestFalse(TEXT("valid table avoids AssetRegistry fallback"), Loading->DidProfileTableFallbackForTesting());

	UDataTable* Empty = NewObject<UDataTable>(GetTransientPackage(), TEXT("PGXLoading_EmptyProfileTable"));
	Empty->RowStruct = FPGXLoadingProfileRow::StaticStruct();
	Settings->LoadingProfileTable = Empty;
	Loading->RediscoverProfilesForTesting();
	TestTrue(TEXT("empty table follows AssetRegistry fallback policy"), Loading->DidProfileTableFallbackForTesting());

	Settings->LoadingProfileTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/PGXLoading/Tests/MissingTable.MissingTable")));
	Loading->RediscoverProfilesForTesting();
	TestTrue(TEXT("load failure follows AssetRegistry fallback policy"), Loading->DidProfileTableFallbackForTesting());
	Settings->LoadingProfileTable = Original;
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
