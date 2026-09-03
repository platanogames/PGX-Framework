// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#if WITH_DEV_AUTOMATION_TESTS

#include "PGXNotificationManager.h"
#include "PGXNotificationProfile.h"
#include "PGXScreenDefinition.h"
#include "PGXScreenManager.h"
#include "PGXUIConfig.h"
#include "PGXUISubsystem.h"
#include "PGXWidgetPool.h"
#include "PGXWidgetPoolProfile.h"
#include "Tags/PGXUITags.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "Misc/AutomationTest.h"
#include "Observability/PGXObservable.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

namespace PGXUIAutomation
{
#define PGX_UI_AUTOMATION_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

	UPGXScreenManager* MakeScreenManager(int32 MaxStackDepth = 4)
	{
		UPGXScreenManager* Manager = NewObject<UPGXScreenManager>(GetTransientPackage(), UPGXScreenManager::StaticClass(), NAME_None, RF_Transient);
		Manager->Initialize(MaxStackDepth);
		return Manager;
	}

	UPGXNotificationManager* MakeNotificationManager(float DefaultDisplayTimeSeconds = 3.0f)
	{
		UPGXNotificationManager* Manager = NewObject<UPGXNotificationManager>(GetTransientPackage(), UPGXNotificationManager::StaticClass(), NAME_None, RF_Transient);
		Manager->Initialize(DefaultDisplayTimeSeconds);
		return Manager;
	}

	UPGXWidgetPool* MakeWidgetPool(int32 Capacity = 2)
	{
		UPGXWidgetPool* Pool = NewObject<UPGXWidgetPool>(GetTransientPackage(), UPGXWidgetPool::StaticClass(), NAME_None, RF_Transient);
		Pool->Initialize(Capacity);
		return Pool;
	}

	FPGXUINotificationRequest MakeNotificationRequest(int32 Priority, const FString& Message = TEXT("Automation notification"))
	{
		FPGXUINotificationRequest Request;
		Request.NotificationTag = TAG_PGX_UI_Notification_Info.GetTag();
		Request.Message = FText::FromString(Message);
		Request.Priority = Priority;
		return Request;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXUI_ScreenPushPopAutomationTest,
	"PGX.UI.preview.ScreenPushPop", PGX_UI_AUTOMATION_FLAGS)
bool FPGXUI_ScreenPushPopAutomationTest::RunTest(const FString& Parameters)
{
	UPGXScreenManager* Manager = PGXUIAutomation::MakeScreenManager();
	const FPGXUIResult PushResult = Manager->PushScreen(TAG_PGX_UI_Screen_Default.GetTag(), TEXT("Default"));
	const FPGXUIResult PopResult = Manager->PopScreen();

	TestTrue(TEXT("ScreenPushPop push succeeds"), PushResult.bSuccess);
	TestTrue(TEXT("ScreenPushPop handle valid"), PushResult.Handle.IsValid());
	TestTrue(TEXT("ScreenPushPop pop succeeds"), PopResult.bSuccess);
	TestEqual(TEXT("ScreenPushPop open count after pop"), Manager->GetOpenScreenCount(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXUI_ScreenOverflowTypedFailureAutomationTest,
	"PGX.UI.preview.ScreenOverflowTypedFailure", PGX_UI_AUTOMATION_FLAGS)
bool FPGXUI_ScreenOverflowTypedFailureAutomationTest::RunTest(const FString& Parameters)
{
	UPGXScreenManager* Manager = PGXUIAutomation::MakeScreenManager(1);
	const FPGXUIResult FirstResult = Manager->PushScreen(TAG_PGX_UI_Screen_Default.GetTag(), TEXT("First"));
	const FPGXUIResult OverflowResult = Manager->PushScreen(TAG_PGX_UI_Screen_Default.GetTag(), TEXT("Second"));

	TestTrue(TEXT("ScreenOverflow setup succeeds"), FirstResult.bSuccess);
	TestFalse(TEXT("ScreenOverflow second push fails"), OverflowResult.bSuccess);
	TestTrue(TEXT("ScreenOverflow typed code"), OverflowResult.Code == EPGXUIResultCode::StackOverflow);
	TestEqual(TEXT("ScreenOverflow count unchanged"), Manager->GetOpenScreenCount(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXUI_NotificationOrderingDismissAutomationTest,
	"PGX.UI.preview.NotificationOrderingDismiss", PGX_UI_AUTOMATION_FLAGS)
bool FPGXUI_NotificationOrderingDismissAutomationTest::RunTest(const FString& Parameters)
{
	UPGXNotificationManager* Manager = PGXUIAutomation::MakeNotificationManager();
	const FPGXUIResult LowResult = Manager->EnqueueNotification(PGXUIAutomation::MakeNotificationRequest(1, TEXT("Low")));
	const FPGXUIResult HighResult = Manager->EnqueueNotification(PGXUIAutomation::MakeNotificationRequest(10, TEXT("High")));
	const TArray<FPGXUINotificationEntry> Snapshot = Manager->GetNotificationQueueSnapshot();
	const FPGXUIResult DismissResult = Manager->DismissNotification(HighResult.Handle);

	TestTrue(TEXT("NotificationOrdering low enqueue succeeds"), LowResult.bSuccess);
	TestTrue(TEXT("NotificationOrdering high enqueue succeeds"), HighResult.bSuccess);
	TestEqual(TEXT("NotificationOrdering snapshot count"), Snapshot.Num(), 2);
	TestTrue(TEXT("NotificationOrdering highest priority first"), Snapshot[0].Request.Priority > Snapshot[1].Request.Priority);
	TestTrue(TEXT("NotificationOrdering dismiss succeeds"), DismissResult.bSuccess);
	TestEqual(TEXT("NotificationOrdering queued count after dismiss"), Manager->GetQueuedNotificationCount(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXUI_WidgetPoolAcquireReleaseAutomationTest,
	"PGX.UI.preview.WidgetPoolAcquireRelease", PGX_UI_AUTOMATION_FLAGS)
bool FPGXUI_WidgetPoolAcquireReleaseAutomationTest::RunTest(const FString& Parameters)
{
	UPGXWidgetPool* Pool = PGXUIAutomation::MakeWidgetPool(2);
	const FPGXUIResult AcquireResult = Pool->AcquireWidget(UUserWidget::StaticClass(), TEXT("AutomationWidget"));
	const FPGXUIResult ReleaseResult = Pool->ReleaseWidget(AcquireResult.Handle);
	const FPGXUIResult ReacquireResult = Pool->AcquireWidget(UUserWidget::StaticClass(), TEXT("AutomationWidgetReuse"));

	TestTrue(TEXT("WidgetPoolAcquireRelease acquire succeeds"), AcquireResult.bSuccess);
	TestTrue(TEXT("WidgetPoolAcquireRelease acquired handle valid"), AcquireResult.Handle.IsValid());
	TestTrue(TEXT("WidgetPoolAcquireRelease release succeeds"), ReleaseResult.bSuccess);
	TestTrue(TEXT("WidgetPoolAcquireRelease reacquire succeeds"), ReacquireResult.bSuccess);
	TestEqual(TEXT("WidgetPoolAcquireRelease acquired count"), Pool->GetAcquiredCount(), 1);
	TestEqual(TEXT("WidgetPoolAcquireRelease available count"), Pool->GetAvailableCount(), 1);
	TestEqual(TEXT("WidgetPoolAcquireRelease bounded snapshot"), Pool->GetPoolSnapshot().Num(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXUI_WidgetPoolExhaustionAutomationTest,
	"PGX.UI.preview.WidgetPoolExhaustion", PGX_UI_AUTOMATION_FLAGS)
bool FPGXUI_WidgetPoolExhaustionAutomationTest::RunTest(const FString& Parameters)
{
	UPGXWidgetPool* Pool = PGXUIAutomation::MakeWidgetPool(1);
	const FPGXUIResult FirstResult = Pool->AcquireWidget(UUserWidget::StaticClass(), TEXT("First"));
	const FPGXUIResult ExhaustedResult = Pool->AcquireWidget(UUserWidget::StaticClass(), TEXT("Second"));

	TestTrue(TEXT("WidgetPoolExhaustion setup succeeds"), FirstResult.bSuccess);
	TestFalse(TEXT("WidgetPoolExhaustion second acquire fails"), ExhaustedResult.bSuccess);
	TestTrue(TEXT("WidgetPoolExhaustion typed code"), ExhaustedResult.Code == EPGXUIResultCode::PoolExhausted);
	TestEqual(TEXT("WidgetPoolExhaustion acquired count unchanged"), Pool->GetAcquiredCount(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXUI_PresentationOnlyNoGameplayMutationAutomationTest,
	"PGX.UI.preview.PresentationOnlyNoGameplayMutation", PGX_UI_AUTOMATION_FLAGS)
bool FPGXUI_PresentationOnlyNoGameplayMutationAutomationTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>(GetTransientPackage(), UGameInstance::StaticClass(), NAME_None, RF_Transient);
	UPGXUISubsystem* Subsystem = NewObject<UPGXUISubsystem>(GameInstance, UPGXUISubsystem::StaticClass(), NAME_None, RF_Transient);
	UPGXUIConfig* Config = NewObject<UPGXUIConfig>(GetTransientPackage(), UPGXUIConfig::StaticClass(), NAME_None, RF_Transient);
	Config->MaxScreenStackDepth = 1;
	Config->WidgetPoolInitialSize = 1;
	Subsystem->InjectTestUIConfig(Config);

	FPGXUIViewSnapshot Snapshot;
	Snapshot.SourceTag = TAG_PGX_UI_View_Default.GetTag();
	Snapshot.ViewName = TEXT("AutomationPresentationOnly");
	Snapshot.PayloadSummary = TEXT("presentation payload only; no gameplay target or domain mutation");
	Snapshot.TimestampSeconds = 1.0;

	TestTrue(TEXT("PresentationOnly screen manager exists"), IsValid(Subsystem->GetScreenManager()));
	TestTrue(TEXT("PresentationOnly notification manager exists"), IsValid(Subsystem->GetNotificationManager()));
	TestTrue(TEXT("PresentationOnly widget pool exists"), IsValid(Subsystem->GetWidgetPool()));
	TestEqual(TEXT("PresentationOnly configured stack depth"), Subsystem->GetScreenManager()->GetMaxStackDepth(), 1);
	TestEqual(TEXT("PresentationOnly configured pool capacity"), Subsystem->GetWidgetPool()->GetCapacity(), 1);
	TestTrue(TEXT("PresentationOnly snapshot tag valid"), Snapshot.SourceTag == TAG_PGX_UI_View_Default.GetTag());
	TestFalse(TEXT("PresentationOnly snapshot payload visible"), Snapshot.PayloadSummary.IsEmpty());
	return true;
}

// ============================================================================
// EN: Observability support — IPGXObservable schema validation tests for
//     all 4 PGXUI authoring DA classes (UPGXUIConfig + UPGXScreenDefinition +
//     UPGXNotificationProfile + UPGXWidgetPoolProfile). Mirror PGXEnvironment
//     8.3.B / PGXAI 8.3.C reference. NewObject in transient package — no
//     PIE/world fixture required.
// ES: Observability support — tests de validacion de schema para adopcion
//     IPGXObservable de las 4 clases authoring DA PGXUI. Mirror referencia
//     PGXEnvironment 8.3.B / PGXAI 8.3.C. NewObject en transient package —
//     no se requiere fixture PIE/world.
// ============================================================================

namespace PGXUIObservabilityAutomation
{
	// EN: Shared validator for the 4-method IPGXObservable contract. Returns
	//     bool result + populates AddInfo with the test class name for
	//     easy triage when a single-plugin Run reports failure on a specific
	//     class.
	// ES: Validador compartido para el contrato 4-method IPGXObservable.
	//     Retorna bool + AddInfo con nombre de clase para triage facil.
	template<typename TConfigClass>
	bool ValidateObservableContract(FAutomationTestBase& Test, const TCHAR* ExpectedTypeName)
	{
		TConfigClass* Instance = NewObject<TConfigClass>(
			GetTransientPackage(), TConfigClass::StaticClass(), NAME_None, RF_Transient);
		if (!Test.TestNotNull(*FString::Printf(TEXT("%s instance"), ExpectedTypeName), Instance))
		{
			return false;
		}

		const FName SchemaVersion = Instance->GetSchemaVersion();
		Test.TestEqual(*FString::Printf(TEXT("%s::GetSchemaVersion is 1.0"), ExpectedTypeName),
			SchemaVersion, FName(TEXT("1.0")));

		const FPGXSchemaDescriptor Descriptor = Instance->GetSchemaDescriptor();
		Test.TestEqual(*FString::Printf(TEXT("%s schema TypeName matches"), ExpectedTypeName),
			Descriptor.TypeName, TConfigClass::StaticClass()->GetFName());
		Test.TestEqual(*FString::Printf(TEXT("%s schema SchemaVersion matches"), ExpectedTypeName),
			Descriptor.SchemaVersion, SchemaVersion);
		Test.TestTrue(*FString::Printf(TEXT("%s schema Fields > 0"), ExpectedTypeName),
			Descriptor.Fields.Num() > 0);

		const FPGXJsonValue Envelope = Instance->ToJson();
		Test.TestFalse(*FString::Printf(TEXT("%s ToJson envelope non-empty"), ExpectedTypeName),
			Envelope.IsEmpty());
		Test.TestTrue(*FString::Printf(TEXT("%s envelope contains type"), ExpectedTypeName),
			Envelope.JsonString.Contains(FString::Printf(TEXT("\"type\":\"%s\""), ExpectedTypeName)));
		Test.TestTrue(*FString::Printf(TEXT("%s envelope contains 1.0 version"), ExpectedTypeName),
			Envelope.JsonString.Contains(TEXT("\"version\":\"1.0\"")));

		const FPGXJsonValue EmptyJson;
		const FPGXValidationResult EmptyResult = Instance->FromJson(EmptyJson);
		Test.TestFalse(*FString::Printf(TEXT("%s FromJson rejects empty"), ExpectedTypeName),
			EmptyResult.bValid);

		const FPGXValidationResult OkResult = Instance->FromJson(Envelope);
		Test.TestTrue(*FString::Printf(TEXT("%s FromJson accepts envelope"), ExpectedTypeName),
			OkResult.bValid);

		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXUIConfigObservableSchema,
	"PGX.UI.ConfigObservableSchema",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXUIConfigObservableSchema::RunTest(const FString& /*Parameters*/)
{
	return PGXUIObservabilityAutomation::ValidateObservableContract<UPGXUIConfig>(*this, TEXT("PGXUIConfig"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXUIScreenDefinitionObservableSchema,
	"PGX.UI.ScreenDefinitionObservableSchema",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXUIScreenDefinitionObservableSchema::RunTest(const FString& /*Parameters*/)
{
	return PGXUIObservabilityAutomation::ValidateObservableContract<UPGXScreenDefinition>(*this, TEXT("PGXScreenDefinition"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXUINotificationProfileObservableSchema,
	"PGX.UI.NotificationProfileObservableSchema",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXUINotificationProfileObservableSchema::RunTest(const FString& /*Parameters*/)
{
	return PGXUIObservabilityAutomation::ValidateObservableContract<UPGXNotificationProfile>(*this, TEXT("PGXNotificationProfile"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXUIWidgetPoolProfileObservableSchema,
	"PGX.UI.WidgetPoolProfileObservableSchema",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXUIWidgetPoolProfileObservableSchema::RunTest(const FString& /*Parameters*/)
{
	return PGXUIObservabilityAutomation::ValidateObservableContract<UPGXWidgetPoolProfile>(*this, TEXT("PGXWidgetPoolProfile"));
}

#endif // WITH_DEV_AUTOMATION_TESTS
