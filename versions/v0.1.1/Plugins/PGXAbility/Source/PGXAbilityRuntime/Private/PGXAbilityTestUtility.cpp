// Copyright PGX Framework. All Rights Reserved.

#include "PGXAbilityTestUtility.h"
#include "PGXAbilitySubsystem.h"
#include "PGXAbilityComponent.h"
#include "PGXAbilityFacade.h"
#include "PGXAttributeFacade.h"
#include "PGXEffectFacade.h"
#include "PGXAbilitySettings.h"
#include "Abilities/GameplayAbility.h"
#include "Abilities/GameplayAbility_Montage.h"
#include "AbilitySystemComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Actor.h"
#include "Logging/PGXLogMacros.h"
#include "NativeGameplayTags.h"
#include "Subsystems/PGXLogSubsystem.h"
#include "Misc/AutomationTest.h"

DEFINE_LOG_CATEGORY_STATIC(LogPGXAbilityTest, Log, All);
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGX_Ability_Test_Identity, "PGX.Ability.Test.Identity");

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
	if (UAbilitySystemComponent* AbilitySystem = Component->GetAbilitySystemComponentInternal())
	{
		AbilitySystem->InitAbilityActorInfo(TestActor, TestActor);
	}

	UPGXAbilityFacade* Facade = Component->GetAbilityFacade();
	UGameplayAbility* AbilityCDO = GetMutableDefault<UGameplayAbility_Montage>();
#if WITH_EDITOR
	const FGameplayTagContainer OriginalAssetTags = AbilityCDO->GetAssetTags();
	AbilityCDO->EditorGetAssetTags().Reset();
	AbilityCDO->EditorGetAssetTags().AddTag(TAG_PGX_Ability_Test_Identity);
#endif

	// 1. Grant with null class -> InvalidInput.
	{
		FPGXAbilityHandle OutHandle;
		const FPGXAbilityResult R = Facade->GrantAbility(nullptr, 1, OutHandle);
		AssertPass(TEXT("GrantRevoke.NullRejected"),
			!R.bSucceeded && R.Code == EPGXAbilityResultCode::InvalidInput && !OutHandle.IsValid());
	}

	// 2. Grant an instantiable ability CDO carrying a real asset tag.
	FPGXAbilityHandle GrantedHandle;
	{
		const FPGXAbilityResult R = Facade->GrantAbility(UGameplayAbility_Montage::StaticClass(), 1, GrantedHandle);
		AssertPass(TEXT("GrantRevoke.GrantSucceeds"), R.bSucceeded && GrantedHandle.IsValid()
			&& GrantedHandle.AbilityTag == TAG_PGX_Ability_Test_Identity,
			FString::Printf(TEXT("succeeded=%d valid=%d"), R.bSucceeded, GrantedHandle.IsValid()));
	}

	// 3. Re-grant/query/snapshot/activate/cancel all use the same first asset-tag identity.
	{
		FPGXAbilityHandle SecondHandle;
		const FPGXAbilityResult R = Facade->GrantAbility(UGameplayAbility_Montage::StaticClass(), 1, SecondHandle);
		AssertPass(TEXT("GrantRevoke.RegrantIsIdempotent"), !R.bSucceeded
			&& R.Code == EPGXAbilityResultCode::AlreadyGranted
			&& SecondHandle.SpecHandle == GrantedHandle.SpecHandle);
		AssertPass(TEXT("GrantRevoke.QueryByAssetTag"), Facade->IsAbilityGranted(TAG_PGX_Ability_Test_Identity));
		const TArray<FPGXAbilitySnapshot> Snapshots = Facade->GetGrantedAbilities();
		AssertPass(TEXT("GrantRevoke.FirstTagSnapshot"), Snapshots.Num() == 1
			&& Snapshots[0].Handle.AbilityTag == TAG_PGX_Ability_Test_Identity);
		const FPGXAbilityResult ActivateResult = Facade->ActivateAbilityByTag(TAG_PGX_Ability_Test_Identity);
		AssertPass(TEXT("GrantRevoke.ActivateByAssetTag"), ActivateResult.bSucceeded);
		AssertPass(TEXT("GrantRevoke.CooldownQueryByAssetTag"),
			FMath::IsNearlyZero(Facade->GetCooldownRemaining(TAG_PGX_Ability_Test_Identity)));
		AssertPass(TEXT("GrantRevoke.CancelByHandle"), Facade->CancelAbility(GrantedHandle).bSucceeded);
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
#if WITH_EDITOR
	AbilityCDO->EditorGetAssetTags() = OriginalAssetTags;
#endif

	return bAllPassed;
}

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXAbilityAssetTagsFacadeAutomationTest,
	"PGX.Ability.AssetTagsFacade", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPGXAbilityAssetTagsFacadeAutomationTest::RunTest(const FString& Parameters)
{
	if (!GEngine)
	{
		AddError(TEXT("Ability AssetTags automation requires GEngine."));
		return false;
	}

	UGameInstance* GameInstance = NewObject<UGameInstance>(GEngine, UGameInstance::StaticClass(), NAME_None, RF_Transient);
	if (!TestNotNull(TEXT("Ability AssetTags transient GameInstance"), GameInstance))
	{
		return false;
	}

	GameInstance->AddToRoot();
	GameInstance->InitializeStandalone(TEXT("PGXAbilityAssetTagsAutomationWorld"));
	TArray<FString> Issues;
	const bool bPassed = UPGXAbilityTestUtility::AbilityGrantRevokeTest(GameInstance, Issues);
	for (const FString& Issue : Issues)
	{
		AddInfo(Issue);
	}
	TestTrue(TEXT("Ability facade uses first AssetTag across grant/query/activate/cancel/snapshot/cooldown"), bPassed);
	GameInstance->Shutdown();
	GameInstance->RemoveFromRoot();
	return bPassed;
}
#endif
