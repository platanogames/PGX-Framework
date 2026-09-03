// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
//
// EN: Automation wrappers around UPGXUITestUtility BPL helpers, registered under the
//     serialized `PGX.UI.preview.<TestName>` names. Each wrapper constructs a transient
//     instance of the relevant surface, forwards to the BPL static bool+OutIssues contract
//     (shared with PGXBridge / PGXMessage / PGXInspector / PGXPSO /
//     PGXAI / PGXColony / PGXUI), surfaces every [PASS]/[FAIL] line via FAutomationTestBase::AddInfo,
//     and propagates the helper's bool result.
//
//     Runtime tests in Tests/PGXUIAutomationTests.cpp retain their serialized names.
//
// ES: Wrappers de Automation sobre los helpers BPL de UPGXUITestUtility. Las pruebas runtime
//     de PGXUIAutomationTests.cpp conservan sus nombres serializados.

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Misc/AutomationTest.h"
#include "PGXNotificationProfile.h"
#include "PGXScreenDefinition.h"
#include "PGXUITestUtility.h"
#include "PGXWidgetPoolProfile.h"
#include "Tags/PGXUITags.h"
#include "Blueprint/UserWidget.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace PGXUIWave4AutomationTestsInternal
{
#define PGX_UI_WAVE4_AUTOMATION_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

	static void ForwardIssues(FAutomationTestBase& Test, const TArray<FString>& OutIssues)
	{
		for (const FString& Issue : OutIssues)
		{
			Test.AddInfo(Issue);
		}
	}

	// EN: Build a fully-valid UPGXScreenDefinition for happy-path assertions.
	// ES: Construye UPGXScreenDefinition completamente valida para asserts happy-path.
	static UPGXScreenDefinition* MakeValidScreenDefinition()
	{
		UPGXScreenDefinition* Def = NewObject<UPGXScreenDefinition>(GetTransientPackage(),
			UPGXScreenDefinition::StaticClass(), NAME_None, RF_Transient);
		Def->ScreenTag = TAG_PGX_UI_Screen_Type_Default.GetTag();
		Def->LayerTag = TAG_PGX_UI_Screen_Layer_HUD.GetTag();
		Def->WidgetClass = TSoftClassPtr<UUserWidget>(FSoftObjectPath(TEXT("/Engine/EngineResources/Black.UserWidget")));
		Def->LayerOrder = 0;
		Def->bIsModal = false;
		Def->bPausesGameplay = false;
		return Def;
	}

	static UPGXNotificationProfile* MakeValidNotificationProfile()
	{
		UPGXNotificationProfile* Profile = NewObject<UPGXNotificationProfile>(GetTransientPackage(),
			UPGXNotificationProfile::StaticClass(), NAME_None, RF_Transient);
		Profile->CategoryTag = TAG_PGX_UI_Notification_Category_Default.GetTag();
		Profile->PriorityTag = TAG_PGX_UI_Notification_Priority_Normal.GetTag();
		Profile->PriorityNumeric = 50;
		Profile->DefaultDisplayTimeSeconds = 3.0f;
		Profile->bAllowCoalescing = false;
		Profile->MaxQueueDepth = 8;
		Profile->bDismissOnHide = true;
		return Profile;
	}

	static UPGXWidgetPoolProfile* MakeValidWidgetPoolProfile()
	{
		UPGXWidgetPoolProfile* Profile = NewObject<UPGXWidgetPoolProfile>(GetTransientPackage(),
			UPGXWidgetPoolProfile::StaticClass(), NAME_None, RF_Transient);
		Profile->PoolTypeTag = TAG_PGX_UI_WidgetPool_Type_Default.GetTag();
		Profile->WidgetClass = TSoftClassPtr<UUserWidget>(FSoftObjectPath(TEXT("/Engine/EngineResources/Black.UserWidget")));
		Profile->bIsAbstractPool = false;
		Profile->InitialCapacity = 16;
		Profile->MaxCapacity = 64;
		Profile->bResetOnRelease = true;
		Profile->MaxReuseCount = 0;
		return Profile;
	}
}

// ============================================================
// Automation wrappers around UPGXUITestUtility BPL helpers
// ============================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXUI_Wave4_ScreenDefinitionValidationAutomationTest,
	"PGX.UI.preview.ScreenDefinitionValidation",
	PGX_UI_WAVE4_AUTOMATION_FLAGS)

bool FPGXUI_Wave4_ScreenDefinitionValidationAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UPGXScreenDefinition* Def = PGXUIWave4AutomationTestsInternal::MakeValidScreenDefinition();
	TArray<FString> OutIssues;
	const bool bPassed = UPGXUITestUtility::ValidateScreenDefinition(Def, OutIssues);
	PGXUIWave4AutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXUI_Wave4_NotificationProfileValidationAutomationTest,
	"PGX.UI.preview.NotificationProfileValidation",
	PGX_UI_WAVE4_AUTOMATION_FLAGS)

bool FPGXUI_Wave4_NotificationProfileValidationAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UPGXNotificationProfile* Profile = PGXUIWave4AutomationTestsInternal::MakeValidNotificationProfile();
	TArray<FString> OutIssues;
	const bool bPassed = UPGXUITestUtility::ValidateNotificationProfile(Profile, OutIssues);
	PGXUIWave4AutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXUI_Wave4_WidgetPoolProfileValidationAutomationTest,
	"PGX.UI.preview.WidgetPoolProfileValidation",
	PGX_UI_WAVE4_AUTOMATION_FLAGS)

bool FPGXUI_Wave4_WidgetPoolProfileValidationAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UPGXWidgetPoolProfile* Profile = PGXUIWave4AutomationTestsInternal::MakeValidWidgetPoolProfile();
	TArray<FString> OutIssues;
	const bool bPassed = UPGXUITestUtility::ValidateWidgetPoolProfile(Profile, OutIssues);
	PGXUIWave4AutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXUI_Wave4_TagInNamespaceCheckAutomationTest,
	"PGX.UI.preview.TagInNamespaceCheck",
	PGX_UI_WAVE4_AUTOMATION_FLAGS)

bool FPGXUI_Wave4_TagInNamespaceCheckAutomationTest::RunTest(const FString& /*Parameters*/)
{
	// EN: Happy-path: PGX.UI.Screen.Layer.HUD must match PGX.UI.Screen.Layer namespace root.
	TArray<FString> OutIssues;
	const bool bPassed = UPGXUITestUtility::ValidateTagInNamespace(
		TAG_PGX_UI_Screen_Layer_HUD.GetTag(),
		TAG_PGX_UI_Screen_Layer.GetTag(),
		OutIssues);
	PGXUIWave4AutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXUI_Wave4_SettingsAccessorValidationAutomationTest,
	"PGX.UI.preview.SettingsAccessorValidation",
	PGX_UI_WAVE4_AUTOMATION_FLAGS)

bool FPGXUI_Wave4_SettingsAccessorValidationAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = UPGXUITestUtility::ValidateSettingsAccessor(OutIssues);
	PGXUIWave4AutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

#endif // WITH_DEV_AUTOMATION_TESTS
