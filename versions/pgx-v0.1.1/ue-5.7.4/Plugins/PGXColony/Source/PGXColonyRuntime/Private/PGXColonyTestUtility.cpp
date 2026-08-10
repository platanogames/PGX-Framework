// Copyright PGX Framework. All Rights Reserved.

#include "PGXColonyTestUtility.h"
#include "PGXColonySettings.h"
#include "PGXColonySubsystem.h"
#include "PGXColonyTypes.h"
#include "Tags/PGXColonyTags.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Logging/PGXLogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogPGXColonyTest, Log, All);

UPGXColonySubsystem* UPGXColonyTestUtility::GetSubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject || !GEngine)
	{
		return nullptr;
	}
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return nullptr;
	}
	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return nullptr;
	}
	return GameInstance->GetSubsystem<UPGXColonySubsystem>();
}

void UPGXColonyTestUtility::RecordResult(TArray<FString>& OutIssues, const FString& TestName, bool bPassed, const FString& Details)
{
	const FString Suffix = Details.IsEmpty() ? FString() : FString::Printf(TEXT(" (%s)"), *Details);
	const FString Tag = bPassed ? TEXT("[PASS]") : TEXT("[FAIL]");
	OutIssues.Add(FString::Printf(TEXT("%s %s%s"), *Tag, *TestName, *Suffix));
	if (bPassed)
	{
		PGX_LOG_INFO(LogPGXColonyTest, TEXT("[PGX Colony Test] PASS: %s%s"), *TestName, *Suffix);
	}
	else
	{
		PGX_LOG_ERROR(LogPGXColonyTest, TEXT("[PGX Colony Test] FAIL: %s%s"), *TestName, *Suffix);
	}
}

// ============================================================================
// 1. SubsystemInitializeTest
// ============================================================================

bool UPGXColonyTestUtility::SubsystemInitializeTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();

	UPGXColonySubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		RecordResult(OutIssues, TEXT("Subsystem.Accessible"), false, TEXT("UPGXColonySubsystem not reachable via GameInstance"));
		return false;
	}

	bool bAllPassed = true;
	auto AssertPass = [&](const FString& Name, bool bPassed, const FString& Details = TEXT(""))
	{
		RecordResult(OutIssues, Name, bPassed, Details);
		if (!bPassed) bAllPassed = false;
	};

	AssertPass(TEXT("Subsystem.Accessible"), true);

	// EN: Freshly-initialized subsystem reports zero registered survivors.
	// ES: Subsistema recien inicializado reporta cero supervivientes registrados.
	const int32 InitialCount = Sub->GetRegisteredSurvivorCount();
	AssertPass(TEXT("Subsystem.InitialEmptyRegistry"), InitialCount == 0,
		FString::Printf(TEXT("count=%d, expected 0"), InitialCount));

	// EN: Snapshot of an empty registry yields an empty array.
	// ES: Snapshot de un registro vacio rinde un array vacio.
	const TArray<FPGXColonySurvivorHandle> Snapshot = Sub->GetSurvivorSnapshot();
	AssertPass(TEXT("Subsystem.InitialEmptySnapshot"), Snapshot.Num() == 0,
		FString::Printf(TEXT("snapshot size=%d, expected 0"), Snapshot.Num()));

	return bAllPassed;
}

// ============================================================================
// 2. SurvivorRegisterUnregisterTest
// ============================================================================

bool UPGXColonyTestUtility::SurvivorRegisterUnregisterTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();

	UPGXColonySubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		RecordResult(OutIssues, TEXT("RegisterUnregister.Setup"), false, TEXT("UPGXColonySubsystem not available"));
		return false;
	}

	bool bAllPassed = true;
	auto AssertPass = [&](const FString& Name, bool bPassed, const FString& Details = TEXT(""))
	{
		RecordResult(OutIssues, Name, bPassed, Details);
		if (!bPassed) bAllPassed = false;
	};

	const int32 BaselineCount = Sub->GetRegisteredSurvivorCount();

	// 1. Register with valid native tag → Success, handle valid, count +1.
	FPGXColonySurvivorHandle FirstHandle;
	{
		FPGXColonyResult R;
		FirstHandle = Sub->RegisterSurvivor(TAG_PGX_Colony_Role_Worker.GetTag(), R);
		AssertPass(TEXT("RegisterUnregister.RegisterFirst"),
			R.bSucceeded && FirstHandle.IsValid() && Sub->GetRegisteredSurvivorCount() == BaselineCount + 1,
			FString::Printf(TEXT("id=%d count=%d"), FirstHandle.SurvivorId, Sub->GetRegisteredSurvivorCount()));
	}

	// 2. Register with empty tag → Success, handle valid (DefinitionTag is optional at baseline).
	FPGXColonySurvivorHandle SecondHandle;
	{
		FPGXColonyResult R;
		SecondHandle = Sub->RegisterSurvivor(FGameplayTag::EmptyTag, R);
		AssertPass(TEXT("RegisterUnregister.RegisterEmptyTagOK"),
			R.bSucceeded && SecondHandle.IsValid() && SecondHandle.SurvivorId != FirstHandle.SurvivorId,
			FString::Printf(TEXT("first=%d second=%d"), FirstHandle.SurvivorId, SecondHandle.SurvivorId));
	}

	// 3. Unregister invalid handle (SurvivorId==0) → InvalidInput.
	{
		const FPGXColonyResult R = Sub->UnregisterSurvivor(FPGXColonySurvivorHandle{});
		AssertPass(TEXT("RegisterUnregister.UnregisterInvalid"),
			!R.bSucceeded && R.Code == EPGXColonyResultCode::InvalidInput,
			FString::Printf(TEXT("code=%d"), static_cast<int32>(R.Code)));
	}

	// 4. Unregister unknown id → NotFound.
	{
		FPGXColonySurvivorHandle Fake;
		Fake.SurvivorId = 999999;
		const FPGXColonyResult R = Sub->UnregisterSurvivor(Fake);
		AssertPass(TEXT("RegisterUnregister.UnregisterUnknown"),
			!R.bSucceeded && R.Code == EPGXColonyResultCode::NotFound,
			FString::Printf(TEXT("code=%d"), static_cast<int32>(R.Code)));
	}

	// 5. Unregister First → Success, count back to BaselineCount+1 (Second still registered).
	{
		const FPGXColonyResult R = Sub->UnregisterSurvivor(FirstHandle);
		AssertPass(TEXT("RegisterUnregister.UnregisterValidFirst"),
			R.bSucceeded && Sub->GetRegisteredSurvivorCount() == BaselineCount + 1,
			FString::Printf(TEXT("count=%d expected %d"), Sub->GetRegisteredSurvivorCount(), BaselineCount + 1));
	}

	// 6. Re-unregister First → NotFound (idempotent, no crash).
	{
		const FPGXColonyResult R = Sub->UnregisterSurvivor(FirstHandle);
		AssertPass(TEXT("RegisterUnregister.UnregisterIdempotent"),
			!R.bSucceeded && R.Code == EPGXColonyResultCode::NotFound,
			FString::Printf(TEXT("code=%d"), static_cast<int32>(R.Code)));
	}

	// 7. Cleanup Second → back to baseline count.
	{
		const FPGXColonyResult R = Sub->UnregisterSurvivor(SecondHandle);
		AssertPass(TEXT("RegisterUnregister.UnregisterValidSecond"),
			R.bSucceeded && Sub->GetRegisteredSurvivorCount() == BaselineCount,
			FString::Printf(TEXT("count=%d expected %d"), Sub->GetRegisteredSurvivorCount(), BaselineCount));
	}

	return bAllPassed;
}

// ============================================================================
// 3. SurvivorRegistrySnapshotTest
// ============================================================================

bool UPGXColonyTestUtility::SurvivorRegistrySnapshotTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();

	UPGXColonySubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		RecordResult(OutIssues, TEXT("Snapshot.Setup"), false, TEXT("UPGXColonySubsystem not available"));
		return false;
	}

	bool bAllPassed = true;
	auto AssertPass = [&](const FString& Name, bool bPassed, const FString& Details = TEXT(""))
	{
		RecordResult(OutIssues, Name, bPassed, Details);
		if (!bPassed) bAllPassed = false;
	};

	constexpr int32 Count = 3;
	const FGameplayTag Tags[Count] = {
		TAG_PGX_Colony_Role_Worker.GetTag(),
		TAG_PGX_Colony_Role_Scout.GetTag(),
		TAG_PGX_Colony_Role_Leader.GetTag()
	};
	TArray<FPGXColonySurvivorHandle> Handles;

	// EN: Register N survivors and verify snapshot + Find resolution + id uniqueness.
	for (int32 i = 0; i < Count; ++i)
	{
		FPGXColonyResult R;
		const FPGXColonySurvivorHandle H = Sub->RegisterSurvivor(Tags[i], R);
		if (!R.bSucceeded || !H.IsValid())
		{
			RecordResult(OutIssues, TEXT("Snapshot.RegisterChain"), false,
				FString::Printf(TEXT("Register failed at i=%d"), i));
			bAllPassed = false;
		}
		Handles.Add(H);
	}

	// IDs unique across registrations.
	{
		TSet<int32> SeenIds;
		bool bUnique = true;
		for (const FPGXColonySurvivorHandle& H : Handles)
		{
			if (SeenIds.Contains(H.SurvivorId))
			{
				bUnique = false;
				break;
			}
			SeenIds.Add(H.SurvivorId);
		}
		AssertPass(TEXT("Snapshot.IdsUnique"), bUnique && SeenIds.Num() == Count,
			FString::Printf(TEXT("unique ids=%d expected %d"), SeenIds.Num(), Count));
	}

	const TArray<FPGXColonySurvivorHandle> Snapshot = Sub->GetSurvivorSnapshot();
	AssertPass(TEXT("Snapshot.SizeMatchesRegistration"),
		Snapshot.Num() >= Count,
		FString::Printf(TEXT("snapshot=%d expected >=%d"), Snapshot.Num(), Count));

	// Each registered handle present in snapshot.
	bool bAllPresent = true;
	for (const FPGXColonySurvivorHandle& Expected : Handles)
	{
		bool bFound = false;
		for (const FPGXColonySurvivorHandle& In : Snapshot)
		{
			if (In == Expected)
			{
				bFound = true;
				break;
			}
		}
		if (!bFound)
		{
			bAllPresent = false;
			break;
		}
	}
	AssertPass(TEXT("Snapshot.HandlesPresent"), bAllPresent);

	// FindSurvivor resolves each id; DefinitionTag preserved.
	bool bAllFound = true;
	bool bAllTagsPreserved = true;
	for (int32 i = 0; i < Handles.Num(); ++i)
	{
		FPGXColonySurvivorHandle FoundHandle;
		const bool bOk = Sub->FindSurvivor(Handles[i].SurvivorId, FoundHandle);
		if (!bOk || FoundHandle != Handles[i])
		{
			bAllFound = false;
			break;
		}
		if (FoundHandle.DefinitionTag != Tags[i])
		{
			bAllTagsPreserved = false;
		}
	}
	AssertPass(TEXT("Snapshot.FindResolves"), bAllFound);
	AssertPass(TEXT("Snapshot.DefinitionTagPreserved"), bAllTagsPreserved);

	// FindSurvivor returns false on unknown id.
	{
		FPGXColonySurvivorHandle Out;
		const bool bOk = Sub->FindSurvivor(999999, Out);
		AssertPass(TEXT("Snapshot.FindUnknownReturnsFalse"), !bOk && !Out.IsValid());
	}

	// Cleanup.
	for (const FPGXColonySurvivorHandle& H : Handles)
	{
		Sub->UnregisterSurvivor(H);
	}

	return bAllPassed;
}

// ============================================================================
// 4. ConfigResolutionTest
// ============================================================================

bool UPGXColonyTestUtility::ConfigResolutionTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	(void)WorldContextObject;

	bool bAllPassed = true;
	auto AssertPass = [&](const FString& Name, bool bPassed, const FString& Details = TEXT(""))
	{
		RecordResult(OutIssues, Name, bPassed, Details);
		if (!bPassed) bAllPassed = false;
	};

	const UPGXColonySettings* Settings = GetDefault<UPGXColonySettings>();
	AssertPass(TEXT("Config.SettingsAccessible"), Settings != nullptr);
	if (!Settings)
	{
		return false;
	}

	// EN: Default DiscoveryMode is AssetRegistryScan (deprecated fallback at-baseline shape).
	AssertPass(TEXT("Config.DiscoveryModeDefault"),
		Settings->DiscoveryMode == EPGXColonyDiscoveryMode::AssetRegistryScan,
		FString::Printf(TEXT("mode=%d"), static_cast<int32>(Settings->DiscoveryMode)));

	// EN: ActiveConfig accessor reachable; soft pointer may be null/empty without error
	//     (NOT CONSUMED AT RUNTIME — runtime consumption disclaimer baked in Settings header).
	const TSoftObjectPtr<UPGXColonyConfig>& ActiveConfig = Settings->ActiveConfig;
	AssertPass(TEXT("Config.ActiveConfigAccessor"), true,
		FString::Printf(TEXT("IsNull=%d"), ActiveConfig.IsNull()));

	// EN: Verbose flag default false (production-safe).
	AssertPass(TEXT("Config.VerboseDefaultFalse"), Settings->bVerboseConfigResolution == false);

	// EN: GetCategoryName returns "PGX" for project-settings grouping consistency.
	AssertPass(TEXT("Config.CategoryName"),
		Settings->GetCategoryName() == TEXT("PGX"),
		Settings->GetCategoryName().ToString());

	return bAllPassed;
}

// ============================================================================
// 5. NativeTagsRegisteredTest
// ============================================================================

bool UPGXColonyTestUtility::NativeTagsRegisteredTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	(void)WorldContextObject;

	bool bAllPassed = true;
	auto AssertTag = [&](const FNativeGameplayTag& Tag, const TCHAR* Label)
	{
		const bool bValid = Tag.GetTag().IsValid();
		RecordResult(OutIssues, FString::Printf(TEXT("Tags.%s"), Label), bValid, Tag.GetTag().ToString());
		if (!bValid) bAllPassed = false;
	};

	AssertTag(TAG_PGX_Colony, TEXT("Root"));

	AssertTag(TAG_PGX_Colony_Role,         TEXT("Role"));
	AssertTag(TAG_PGX_Colony_Role_Worker,  TEXT("Role_Worker"));
	AssertTag(TAG_PGX_Colony_Role_Scout,   TEXT("Role_Scout"));
	AssertTag(TAG_PGX_Colony_Role_Leader,  TEXT("Role_Leader"));

	AssertTag(TAG_PGX_Colony_Task,         TEXT("Task"));
	AssertTag(TAG_PGX_Colony_Task_Idle,    TEXT("Task_Idle"));
	AssertTag(TAG_PGX_Colony_Task_Gather,  TEXT("Task_Gather"));
	AssertTag(TAG_PGX_Colony_Task_Build,   TEXT("Task_Build"));

	AssertTag(TAG_PGX_Colony_Need,         TEXT("Need"));
	AssertTag(TAG_PGX_Colony_Need_Hunger,  TEXT("Need_Hunger"));
	AssertTag(TAG_PGX_Colony_Need_Rest,    TEXT("Need_Rest"));
	AssertTag(TAG_PGX_Colony_Need_Social,  TEXT("Need_Social"));

	AssertTag(TAG_PGX_Colony_Event,             TEXT("Event"));
	AssertTag(TAG_PGX_Colony_Event_Recruitment, TEXT("Event_Recruitment"));
	AssertTag(TAG_PGX_Colony_Event_Conflict,    TEXT("Event_Conflict"));

	return bAllPassed;
}
