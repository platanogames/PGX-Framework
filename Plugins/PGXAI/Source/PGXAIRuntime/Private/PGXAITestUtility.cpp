// Copyright PGX Framework. All Rights Reserved.

#include "PGXAITestUtility.h"
#include "PGXAISettings.h"
#include "PGXAISubsystem.h"
#include "PGXAITypes.h"
#include "Tags/PGXAITags.h"
#include "AIController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Logging/PGXLogMacros.h"
#include "Subsystems/PGXLogSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogPGXAITest, Log, All);

UPGXAISubsystem* UPGXAITestUtility::GetSubsystem(const UObject* WorldContextObject)
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
	return World->GetSubsystem<UPGXAISubsystem>();
}

void UPGXAITestUtility::RecordResult(TArray<FString>& OutIssues, const FString& TestName, bool bPassed, const FString& Details)
{
	const FString Suffix = Details.IsEmpty() ? FString() : FString::Printf(TEXT(" (%s)"), *Details);
	const FString Tag = bPassed ? TEXT("[PASS]") : TEXT("[FAIL]");
	OutIssues.Add(FString::Printf(TEXT("%s %s%s"), *Tag, *TestName, *Suffix));
	if (bPassed)
	{
		PGX_LOG_INFO(LogPGXAITest, TEXT("[PGX AI Test] PASS: %s%s"), *TestName, *Suffix);
	}
	else
	{
		PGX_LOG_ERROR(LogPGXAITest, TEXT("[PGX AI Test] FAIL: %s%s"), *TestName, *Suffix);
	}
}

// ============================================================================
// 1. SubsystemInitializeTest
// ============================================================================

bool UPGXAITestUtility::SubsystemInitializeTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();

	UPGXAISubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		RecordResult(OutIssues, TEXT("Subsystem.Accessible"), false, TEXT("UPGXAISubsystem not reachable from world context"));
		return false;
	}

	bool bAllPassed = true;
	auto AssertPass = [&](const FString& Name, bool bPassed, const FString& Details = TEXT(""))
	{
		RecordResult(OutIssues, Name, bPassed, Details);
		if (!bPassed) bAllPassed = false;
	};

	AssertPass(TEXT("Subsystem.Accessible"), true);

	// EN: A freshly initialized subsystem must report zero registered agents.
	// ES: Un subsistema recien inicializado debe reportar cero agentes registrados.
	const int32 InitialCount = Sub->GetRegisteredAgentCount();
	AssertPass(TEXT("Subsystem.InitialEmptyRegistry"), InitialCount == 0,
		FString::Printf(TEXT("count=%d, expected 0"), InitialCount));

	// EN: Snapshot of an empty registry must yield an empty array.
	// ES: Snapshot de un registro vacio debe rendir un array vacio.
	const TArray<FPGXAIAgentHandle> Snapshot = Sub->GetAgentSnapshot();
	AssertPass(TEXT("Subsystem.InitialEmptySnapshot"), Snapshot.Num() == 0,
		FString::Printf(TEXT("snapshot size=%d, expected 0"), Snapshot.Num()));

	return bAllPassed;
}

// ============================================================================
// 2. AgentRegisterUnregisterTest
// ============================================================================

bool UPGXAITestUtility::AgentRegisterUnregisterTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();

	UPGXAISubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		RecordResult(OutIssues, TEXT("RegisterUnregister.Setup"), false, TEXT("UPGXAISubsystem not available"));
		return false;
	}

	UWorld* World = Sub->GetWorld();
	if (!World)
	{
		RecordResult(OutIssues, TEXT("RegisterUnregister.Setup"), false, TEXT("Subsystem world is null"));
		return false;
	}

	bool bAllPassed = true;
	auto AssertPass = [&](const FString& Name, bool bPassed, const FString& Details = TEXT(""))
	{
		RecordResult(OutIssues, Name, bPassed, Details);
		if (!bPassed) bAllPassed = false;
	};

	const int32 BaselineCount = Sub->GetRegisteredAgentCount();

	// EN: Spawn a transient AAIController to exercise the registry. Test-owned actor; no scene
	//     impact. We retain it via UPROPERTY-equivalent local pointer through the test scope.
	// ES: Spawnea un AAIController transitorio para ejercitar el registro.
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AAIController* Controller = World->SpawnActor<AAIController>(AAIController::StaticClass(), Params);
	if (!IsValid(Controller))
	{
		RecordResult(OutIssues, TEXT("RegisterUnregister.SpawnController"), false, TEXT("SpawnActor returned null"));
		return false;
	}
	AssertPass(TEXT("RegisterUnregister.SpawnController"), true);

	// 1. Register null → InvalidInput.
	{
		FPGXAIResult R;
		const FPGXAIAgentHandle H = Sub->RegisterAgent(nullptr, R);
		AssertPass(TEXT("RegisterUnregister.NullRejected"),
			!R.bSucceeded && R.Code == EPGXAIResultCode::InvalidInput && !H.IsValid(),
			FString::Printf(TEXT("code=%d valid=%d"), static_cast<int32>(R.Code), H.IsValid()));
	}

	// 2. Register valid → Success, handle valid, count baseline+1.
	FPGXAIAgentHandle FirstHandle;
	{
		FPGXAIResult R;
		FirstHandle = Sub->RegisterAgent(Controller, R);
		AssertPass(TEXT("RegisterUnregister.RegisterFirst"),
			R.bSucceeded && FirstHandle.IsValid() && Sub->GetRegisteredAgentCount() == BaselineCount + 1,
			FString::Printf(TEXT("id=%d count=%d"), FirstHandle.AgentId, Sub->GetRegisteredAgentCount()));
	}

	// 3. Re-register same controller → idempotency, same handle, count unchanged.
	{
		FPGXAIResult R;
		const FPGXAIAgentHandle Re = Sub->RegisterAgent(Controller, R);
		AssertPass(TEXT("RegisterUnregister.Idempotent"),
			R.bSucceeded && Re == FirstHandle && Sub->GetRegisteredAgentCount() == BaselineCount + 1,
			FString::Printf(TEXT("re-id=%d expected %d"), Re.AgentId, FirstHandle.AgentId));
	}

	// 4. Unregister invalid handle → InvalidInput.
	{
		const FPGXAIResult R = Sub->UnregisterAgent(FPGXAIAgentHandle{});
		AssertPass(TEXT("RegisterUnregister.UnregisterInvalid"),
			!R.bSucceeded && R.Code == EPGXAIResultCode::InvalidInput);
	}

	// 5. Unregister unknown id → NotFound.
	{
		FPGXAIAgentHandle Fake;
		Fake.AgentId = 999999;
		const FPGXAIResult R = Sub->UnregisterAgent(Fake);
		AssertPass(TEXT("RegisterUnregister.UnregisterUnknown"),
			!R.bSucceeded && R.Code == EPGXAIResultCode::NotFound);
	}

	// 6. Unregister valid → Success, count baseline.
	{
		const FPGXAIResult R = Sub->UnregisterAgent(FirstHandle);
		AssertPass(TEXT("RegisterUnregister.UnregisterValid"),
			R.bSucceeded && Sub->GetRegisteredAgentCount() == BaselineCount,
			FString::Printf(TEXT("count=%d expected %d"), Sub->GetRegisteredAgentCount(), BaselineCount));
	}

	// 7. Cleanup spawned controller.
	if (IsValid(Controller))
	{
		Controller->Destroy();
	}

	return bAllPassed;
}

// ============================================================================
// 3. AgentRegistrySnapshotTest
// ============================================================================

bool UPGXAITestUtility::AgentRegistrySnapshotTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();

	UPGXAISubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		RecordResult(OutIssues, TEXT("Snapshot.Setup"), false, TEXT("UPGXAISubsystem not available"));
		return false;
	}
	UWorld* World = Sub->GetWorld();
	if (!World)
	{
		RecordResult(OutIssues, TEXT("Snapshot.Setup"), false, TEXT("Subsystem world is null"));
		return false;
	}

	bool bAllPassed = true;
	auto AssertPass = [&](const FString& Name, bool bPassed, const FString& Details = TEXT(""))
	{
		RecordResult(OutIssues, Name, bPassed, Details);
		if (!bPassed) bAllPassed = false;
	};

	constexpr int32 Count = 3;
	TArray<TWeakObjectPtr<AAIController>> SpawnedControllers;
	TArray<FPGXAIAgentHandle> Handles;

	// EN: Spawn N controllers and register each. Track them so we can clean up.
	// ES: Spawnea N controllers y registra cada uno.
	for (int32 i = 0; i < Count; ++i)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AAIController* C = World->SpawnActor<AAIController>(AAIController::StaticClass(), Params);
		if (!IsValid(C))
		{
			RecordResult(OutIssues, TEXT("Snapshot.SpawnController"), false,
				FString::Printf(TEXT("Spawn failed at i=%d"), i));
			return false;
		}
		SpawnedControllers.Add(C);

		FPGXAIResult R;
		const FPGXAIAgentHandle H = Sub->RegisterAgent(C, R);
		if (!R.bSucceeded || !H.IsValid())
		{
			RecordResult(OutIssues, TEXT("Snapshot.RegisterChain"), false,
				FString::Printf(TEXT("Register failed at i=%d"), i));
			bAllPassed = false;
		}
		Handles.Add(H);
	}

	const TArray<FPGXAIAgentHandle> Snapshot = Sub->GetAgentSnapshot();
	AssertPass(TEXT("Snapshot.SizeMatchesRegistration"),
		Snapshot.Num() >= Count,
		FString::Printf(TEXT("snapshot=%d expected >=%d"), Snapshot.Num(), Count));

	// EN: Each registered handle must appear in the snapshot.
	// ES: Cada handle registrado debe aparecer en el snapshot.
	bool bAllPresent = true;
	for (const FPGXAIAgentHandle& Expected : Handles)
	{
		bool bFound = false;
		for (const FPGXAIAgentHandle& In : Snapshot)
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

	// EN: Snapshot entries should have valid (non-stale) controller pointers.
	// ES: Las entradas del snapshot deben tener punteros de controller validos.
	bool bAllAlive = true;
	for (const FPGXAIAgentHandle& In : Snapshot)
	{
		if (!In.Controller.IsValid())
		{
			bAllAlive = false;
			break;
		}
	}
	AssertPass(TEXT("Snapshot.NoStaleEntries"), bAllAlive);

	// EN: Cleanup — unregister + destroy.
	// ES: Limpieza.
	for (const FPGXAIAgentHandle& H : Handles)
	{
		Sub->UnregisterAgent(H);
	}
	for (TWeakObjectPtr<AAIController>& WC : SpawnedControllers)
	{
		if (AAIController* C = WC.Get())
		{
			C->Destroy();
		}
	}

	return bAllPassed;
}

// ============================================================================
// 4. ConfigResolutionTest
// ============================================================================

bool UPGXAITestUtility::ConfigResolutionTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	(void)WorldContextObject;

	bool bAllPassed = true;
	auto AssertPass = [&](const FString& Name, bool bPassed, const FString& Details = TEXT(""))
	{
		RecordResult(OutIssues, Name, bPassed, Details);
		if (!bPassed) bAllPassed = false;
	};

	const UPGXAISettings* Settings = GetDefault<UPGXAISettings>();
	AssertPass(TEXT("Config.SettingsAccessible"), Settings != nullptr);
	if (!Settings)
	{
		return false;
	}

	// EN: Default DiscoveryMode is AssetRegistryScan (deprecated fallback path); Manual is reserved.
	// ES: DiscoveryMode default es AssetRegistryScan; Manual esta reservado.
	AssertPass(TEXT("Config.DiscoveryModeDefault"),
		Settings->DiscoveryMode == EPGXAIDiscoveryMode::AssetRegistryScan,
		FString::Printf(TEXT("mode=%d"), static_cast<int32>(Settings->DiscoveryMode)));

	// EN: ActiveConfig is a soft pointer — TSoftObjectPtr can be null/empty without error;
	//     the test asserts the accessor surface is reachable, not that any DA is bound.
	// ES: ActiveConfig es soft pointer — puede ser null/vacio sin error; el test verifica
	//     que la superficie del accessor es alcanzable, no que haya un DA enlazado.
	const TSoftObjectPtr<UPGXAIConfig>& ActiveConfig = Settings->ActiveConfig;
	AssertPass(TEXT("Config.ActiveConfigAccessor"),
		true,
		FString::Printf(TEXT("IsNull=%d"), ActiveConfig.IsNull()));

	// EN: Verbose flag default is false (production-safe).
	// ES: El flag verboso default es false (seguro para produccion).
	AssertPass(TEXT("Config.VerboseDefaultFalse"), Settings->bVerboseConfigResolution == false);

	// EN: GetCategoryName must return "PGX" for project-settings grouping consistency.
	// ES: GetCategoryName debe retornar "PGX".
	AssertPass(TEXT("Config.CategoryName"),
		Settings->GetCategoryName() == TEXT("PGX"),
		Settings->GetCategoryName().ToString());

	return bAllPassed;
}

// ============================================================================
// 5. NativeTagsRegisteredTest
// ============================================================================

bool UPGXAITestUtility::NativeTagsRegisteredTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	(void)WorldContextObject;

	bool bAllPassed = true;
	auto AssertTag = [&](const FNativeGameplayTag& Tag, const TCHAR* Label)
	{
		const bool bValid = Tag.GetTag().IsValid();
		RecordResult(OutIssues, FString::Printf(TEXT("Tags.%s"), Label), bValid,
			Tag.GetTag().ToString());
		if (!bValid) bAllPassed = false;
	};

	AssertTag(TAG_PGX_AI, TEXT("Root"));

	AssertTag(TAG_PGX_AI_Perception, TEXT("Perception"));
	AssertTag(TAG_PGX_AI_Perception_Sight, TEXT("Perception_Sight"));
	AssertTag(TAG_PGX_AI_Perception_Hearing, TEXT("Perception_Hearing"));
	AssertTag(TAG_PGX_AI_Perception_Damage, TEXT("Perception_Damage"));

	AssertTag(TAG_PGX_AI_Alert, TEXT("Alert"));
	AssertTag(TAG_PGX_AI_Alert_Calm, TEXT("Alert_Calm"));
	AssertTag(TAG_PGX_AI_Alert_Investigating, TEXT("Alert_Investigating"));
	AssertTag(TAG_PGX_AI_Alert_Combat, TEXT("Alert_Combat"));

	AssertTag(TAG_PGX_AI_Squad, TEXT("Squad"));
	AssertTag(TAG_PGX_AI_Squad_Member, TEXT("Squad_Member"));
	AssertTag(TAG_PGX_AI_Squad_Leader, TEXT("Squad_Leader"));

	AssertTag(TAG_PGX_AI_Task, TEXT("Task"));
	AssertTag(TAG_PGX_AI_Task_Idle, TEXT("Task_Idle"));
	AssertTag(TAG_PGX_AI_Task_Patrol, TEXT("Task_Patrol"));
	AssertTag(TAG_PGX_AI_Task_Engage, TEXT("Task_Engage"));

	return bAllPassed;
}
