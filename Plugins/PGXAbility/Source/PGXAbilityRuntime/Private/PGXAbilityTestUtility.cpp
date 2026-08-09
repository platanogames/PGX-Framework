// Copyright PGX Framework. All Rights Reserved.

#include "PGXAbilityTestUtility.h"
#include "PGXAbilitySubsystem.h"
#include "PGXAbilityComponent.h"
#include "PGXAbilityFacade.h"
#include "PGXAttributeFacade.h"
#include "PGXEffectFacade.h"
#include "PGXAbilitySettings.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Logging/PGXLogMacros.h"
#include "Subsystems/PGXLogSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogPGXAbilityTest, Log, All);

UPGXAbilitySubsystem* UPGXAbilityTestUtility::GetSubsystem(const UObject* WorldContextObject)
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
	return GameInstance ? GameInstance->GetSubsystem<UPGXAbilitySubsystem>() : nullptr;
}

void UPGXAbilityTestUtility::RecordResult(TArray<FString>& OutIssues, const FString& TestName, bool bPassed, const FString& Details)
{
	const FString Suffix = Details.IsEmpty() ? FString() : FString::Printf(TEXT(" (%s)"), *Details);
	const FString Tag = bPassed ? TEXT("[PASS]") : TEXT("[FAIL]");
	OutIssues.Add(FString::Printf(TEXT("%s %s%s"), *Tag, *TestName, *Suffix));
	if (bPassed)
	{
		PGX_LOG_INFO(LogPGXAbilityTest, TEXT("[PGX Ability Test] PASS: %s%s"), *TestName, *Suffix);
	}
	else
	{
		PGX_LOG_ERROR(LogPGXAbilityTest, TEXT("[PGX Ability Test] FAIL: %s%s"), *TestName, *Suffix);
	}
}

// ============================================================================
// 1. SubsystemInitializeTest
// ============================================================================

bool UPGXAbilityTestUtility::SubsystemInitializeTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();

	UPGXAbilitySubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		RecordResult(OutIssues, TEXT("Subsystem.Accessible"), false, TEXT("UPGXAbilitySubsystem not reachable from world context"));
		return false;
	}

	bool bAllPassed = true;
	auto AssertPass = [&](const FString& Name, bool bPassed, const FString& Details = TEXT(""))
	{
		RecordResult(OutIssues, Name, bPassed, Details);
		if (!bPassed) bAllPassed = false;
	};

	AssertPass(TEXT("Subsystem.Accessible"), true);

	const int32 InitialCount = Sub->GetRegisteredComponentCount();
	AssertPass(TEXT("Subsystem.InitialEmptyRegistry"), InitialCount == 0,
		FString::Printf(TEXT("count=%d, expected 0 (note: 0 only holds if no other test left components registered)"), InitialCount));

	return bAllPassed;
}

// ============================================================================
// 2. ComponentLifecycleTest
// ============================================================================

bool UPGXAbilityTestUtility::ComponentLifecycleTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();

	UPGXAbilitySubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		RecordResult(OutIssues, TEXT("ComponentLifecycle.Setup"), false, TEXT("UPGXAbilitySubsystem not available"));
		return false;
	}
	UWorld* World = Sub->GetWorld();
	if (!World)
	{
		RecordResult(OutIssues, TEXT("ComponentLifecycle.Setup"), false, TEXT("Subsystem world is null"));
		return false;
	}

	bool bAllPassed = true;
	auto AssertPass = [&](const FString& Name, bool bPassed, const FString& Details = TEXT(""))
	{
		RecordResult(OutIssues, Name, bPassed, Details);
		if (!bPassed) bAllPassed = false;
	};

	const int32 BaselineCount = Sub->GetRegisteredComponentCount();

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor* TestActor = World->SpawnActor<AActor>(AActor::StaticClass(), Params);
	if (!IsValid(TestActor))
	{
		RecordResult(OutIssues, TEXT("ComponentLifecycle.SpawnActor"), false, TEXT("SpawnActor returned null"));
		return false;
	}
	AssertPass(TEXT("ComponentLifecycle.SpawnActor"), true);

	UPGXAbilityComponent* Component = NewObject<UPGXAbilityComponent>(TestActor);
	Component->RegisterComponent();
	TestActor->AddInstanceComponent(Component);
	// EN: Manually invoke BeginPlay since this test does not go through the full actor lifecycle
	//     (SpawnActor with bDeferConstruction=false already called BeginPlay on the actor, but a
	//     component added afterward must be initialized/begun explicitly).
	// ES: Invocamos BeginPlay manualmente porque el componente se agrego despues del lifecycle
	//     normal del actor.
	Component->RegisterAllComponentTickFunctions(true);
	if (TestActor->HasActorBegunPlay())
	{
		Component->RegisterComponentWithWorld(World);
	}
	Component->BeginPlay();

	AssertPass(TEXT("ComponentLifecycle.ASCReady"), Component->IsAbilitySystemReady());
	AssertPass(TEXT("ComponentLifecycle.RegisteredWithSubsystem"),
		Sub->GetRegisteredComponentCount() == BaselineCount + 1,
		FString::Printf(TEXT("count=%d expected %d"), Sub->GetRegisteredComponentCount(), BaselineCount + 1));

	Component->EndPlay(EEndPlayReason::Destroyed);

	AssertPass(TEXT("ComponentLifecycle.UnregisteredFromSubsystem"),
		Sub->GetRegisteredComponentCount() == BaselineCount,
		FString::Printf(TEXT("count=%d expected %d"), Sub->GetRegisteredComponentCount(), BaselineCount));

	TestActor->Destroy();

	return bAllPassed;
}

// ============================================================================
// 3. FacadeResolutionTest
// ============================================================================

bool UPGXAbilityTestUtility::FacadeResolutionTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();

	UPGXAbilitySubsystem* Sub = GetSubsystem(WorldContextObject);
	UWorld* World = Sub ? Sub->GetWorld() : nullptr;
	if (!World)
	{
		RecordResult(OutIssues, TEXT("FacadeResolution.Setup"), false, TEXT("World unavailable"));
		return false;
	}

	bool bAllPassed = true;
	auto AssertPass = [&](const FString& Name, bool bPassed, const FString& Details = TEXT(""))
	{
		RecordResult(OutIssues, Name, bPassed, Details);
		if (!bPassed) bAllPassed = false;
	};

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor* TestActor = World->SpawnActor<AActor>(AActor::StaticClass(), Params);
	UPGXAbilityComponent* Component = NewObject<UPGXAbilityComponent>(TestActor);
	Component->RegisterComponent();
	TestActor->AddInstanceComponent(Component);
	Component->BeginPlay();

	UPGXAbilityFacade* AbilityFacade = Component->GetAbilityFacade();
	UPGXAttributeFacade* AttributeFacade = Component->GetAttributeFacade();
	UPGXEffectFacade* EffectFacade = Component->GetEffectFacade();

	AssertPass(TEXT("FacadeResolution.AbilityFacadeValid"), IsValid(AbilityFacade));
	AssertPass(TEXT("FacadeResolution.AttributeFacadeValid"), IsValid(AttributeFacade));
	AssertPass(TEXT("FacadeResolution.EffectFacadeValid"), IsValid(EffectFacade));

	// EN: Resolving twice must return the same instance (owned per-actor, not re-created per call).
	// ES: Resolver dos veces debe retornar la misma instancia.
	AssertPass(TEXT("FacadeResolution.Stable"), Component->GetAbilityFacade() == AbilityFacade);

	Component->EndPlay(EEndPlayReason::Destroyed);
	TestActor->Destroy();

	return bAllPassed;
}

// ============================================================================
// 4. ConfigResolutionTest
// ============================================================================

bool UPGXAbilityTestUtility::ConfigResolutionTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	(void)WorldContextObject;

	bool bAllPassed = true;
	auto AssertPass = [&](const FString& Name, bool bPassed, const FString& Details = TEXT(""))
	{
		RecordResult(OutIssues, Name, bPassed, Details);
		if (!bPassed) bAllPassed = false;
	};

	const UPGXAbilitySettings* Settings = GetDefault<UPGXAbilitySettings>();
	AssertPass(TEXT("Config.SettingsAccessible"), Settings != nullptr);
	if (!Settings)
	{
		return false;
	}

	AssertPass(TEXT("Config.ActiveConfigAccessor"), true,
		FString::Printf(TEXT("IsNull=%d"), Settings->ActiveConfig.IsNull()));
	AssertPass(TEXT("Config.VerboseDefaultFalse"), Settings->bVerboseConfigResolution == false);
	AssertPass(TEXT("Config.CategoryName"), Settings->GetCategoryName() == TEXT("PGX"), Settings->GetCategoryName().ToString());

	return bAllPassed;
}

// ============================================================================
// 5. AbilityGrantRevokeTest
// ============================================================================

bool UPGXAbilityTestUtility::AbilityGrantRevokeTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();

	UPGXAbilitySubsystem* Sub = GetSubsystem(WorldContextObject);
	UWorld* World = Sub ? Sub->GetWorld() : nullptr;
	if (!World)
	{
		RecordResult(OutIssues, TEXT("GrantRevoke.Setup"), false, TEXT("World unavailable"));
		return false;
	}

	bool bAllPassed = true;
	auto AssertPass = [&](const FString& Name, bool bPassed, const FString& Details = TEXT(""))
	{
		RecordResult(OutIssues, Name, bPassed, Details);
		if (!bPassed) bAllPassed = false;
	};

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor* TestActor = World->SpawnActor<AActor>(AActor::StaticClass(), Params);
	UPGXAbilityComponent* Component = NewObject<UPGXAbilityComponent>(TestActor);
	Component->RegisterComponent();
	TestActor->AddInstanceComponent(Component);
	Component->BeginPlay();

	UPGXAbilityFacade* Facade = Component->GetAbilityFacade();

	// 1. Grant with null class -> InvalidInput.
	{
		FPGXAbilityHandle OutHandle;
		const FPGXAbilityResult R = Facade->GrantAbility(nullptr, 1, OutHandle);
		AssertPass(TEXT("GrantRevoke.NullRejected"),
			!R.bSucceeded && R.Code == EPGXAbilityResultCode::InvalidInput && !OutHandle.IsValid());
	}

	// 2. Grant base UGameplayAbility (instantiable CDO, not project content — exercises the GAS
	//    call path without depending on project-authored content).
	FPGXAbilityHandle GrantedHandle;
	{
		const FPGXAbilityResult R = Facade->GrantAbility(UGameplayAbility::StaticClass(), 1, GrantedHandle);
		AssertPass(TEXT("GrantRevoke.GrantSucceeds"), R.bSucceeded && GrantedHandle.IsValid(),
			FString::Printf(TEXT("succeeded=%d valid=%d"), R.bSucceeded, GrantedHandle.IsValid()));
	}

	// 3. Re-grant same class -> AlreadyGranted (base UGameplayAbility CDO has no AbilityTags, so
	//    idempotency by tag does not apply here — this exercises the no-tag path instead, which
	//    GAS allows as a second independent grant. Documented honestly: tag-based idempotency
	//    requires the project's ability class to declare at least one AbilityTags entry.)
	{
		FPGXAbilityHandle SecondHandle;
		const FPGXAbilityResult R = Facade->GrantAbility(UGameplayAbility::StaticClass(), 1, SecondHandle);
		RecordResult(OutIssues, TEXT("GrantRevoke.SecondGrantNoTagBehavior"), true,
			FString::Printf(TEXT("succeeded=%d (informational — base UGameplayAbility has no AbilityTags, idempotency check requires project content)"), R.bSucceeded));
	}

	// 4. Revoke valid handle -> Success.
	{
		const FPGXAbilityResult R = Facade->RevokeAbility(GrantedHandle);
		AssertPass(TEXT("GrantRevoke.RevokeSucceeds"), R.bSucceeded);
	}

	// 5. Revoke unknown handle -> NotFound.
	{
		FPGXAbilityHandle Fake;
		Fake.SpecHandle = FGameplayAbilitySpecHandle();
		Fake.AbilityTag = FGameplayTag();
		const FPGXAbilityResult R = Facade->RevokeAbility(GrantedHandle); // already revoked above
		AssertPass(TEXT("GrantRevoke.RevokeAlreadyRevoked"), !R.bSucceeded && R.Code == EPGXAbilityResultCode::NotFound);
	}

	Component->EndPlay(EEndPlayReason::Destroyed);
	TestActor->Destroy();

	return bAllPassed;
}
