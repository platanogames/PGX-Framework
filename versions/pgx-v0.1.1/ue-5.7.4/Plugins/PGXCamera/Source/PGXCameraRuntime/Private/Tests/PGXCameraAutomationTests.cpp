// Copyright PGX Framework. All Rights Reserved.
// Camera automation covers mode switching, data asset defaults and active-mode queries.

#if WITH_DEV_AUTOMATION_TESTS

#include "PGXCameraSubsystem.h"
#include "PGXCameraMode.h"
#include "PGXCameraConfig.h"
#include "Misc/AutomationTest.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

namespace PGXCameraAutomation
{
#define PGX_CAMERA_AUTOMATION_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

	UPGXCameraSubsystem* MakeCameraSubsystem()
	{
		return NewObject<UPGXCameraSubsystem>(GetTransientPackage(), UPGXCameraSubsystem::StaticClass(), NAME_None, RF_Transient);
	}

	UPGXCameraMode* MakeMode(const TCHAR* Name, FName InModeName = NAME_None, float InFieldOfView = 90.0f, float InBlendTime = 0.5f)
	{
		UPGXCameraMode* Mode = NewObject<UPGXCameraMode>(GetTransientPackage(), UPGXCameraMode::StaticClass(), FName(Name), RF_Transient);
		Mode->ModeName = InModeName;
		Mode->FieldOfView = InFieldOfView;
		Mode->BlendTime = InBlendTime;
		return Mode;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXCamera_ModeDataAssetDefaultsAutomationTest,
	"PGX.Camera.preview.ModeDataAssetDefaults", PGX_CAMERA_AUTOMATION_FLAGS)
bool FPGXCamera_ModeDataAssetDefaultsAutomationTest::RunTest(const FString& Parameters)
{
	UPGXCameraMode* Mode = PGXCameraAutomation::MakeMode(TEXT("PGXCamera_ModeDefaults"));
	TestEqual(TEXT("ModeDataAssetDefaults FieldOfView"), Mode->FieldOfView, 90.0f);
	TestEqual(TEXT("ModeDataAssetDefaults BlendTime"), Mode->BlendTime, 0.5f);
	TestEqual(TEXT("ModeDataAssetDefaults CameraOffset"), Mode->CameraOffset, FVector::ZeroVector);
	TestEqual(TEXT("ModeDataAssetDefaults ModeName empty by default"), Mode->ModeName, NAME_None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXCamera_ModeDataAssetCustomValuesAutomationTest,
	"PGX.Camera.preview.ModeDataAssetCustomValues", PGX_CAMERA_AUTOMATION_FLAGS)
bool FPGXCamera_ModeDataAssetCustomValuesAutomationTest::RunTest(const FString& Parameters)
{
	UPGXCameraMode* Mode = PGXCameraAutomation::MakeMode(TEXT("PGXCamera_ModeCustom"), FName(TEXT("CombatMode")), 75.0f, 1.25f);
	Mode->CameraOffset = FVector(50.0, 0.0, 100.0);
	TestEqual(TEXT("ModeDataAssetCustomValues ModeName"), Mode->ModeName, FName(TEXT("CombatMode")));
	TestEqual(TEXT("ModeDataAssetCustomValues FieldOfView"), Mode->FieldOfView, 75.0f);
	TestEqual(TEXT("ModeDataAssetCustomValues BlendTime"), Mode->BlendTime, 1.25f);
	TestEqual(TEXT("ModeDataAssetCustomValues CameraOffset"), Mode->CameraOffset, FVector(50.0, 0.0, 100.0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXCamera_ConfigDataAssetDefaultsAutomationTest,
	"PGX.Camera.preview.ConfigDataAssetDefaults", PGX_CAMERA_AUTOMATION_FLAGS)
bool FPGXCamera_ConfigDataAssetDefaultsAutomationTest::RunTest(const FString& Parameters)
{
	UPGXCameraConfig* Config = NewObject<UPGXCameraConfig>(GetTransientPackage(), UPGXCameraConfig::StaticClass(), NAME_None, RF_Transient);
	TestEqual(TEXT("ConfigDataAssetDefaults DefaultFieldOfView"), Config->DefaultFieldOfView, 90.0f);
	TestEqual(TEXT("ConfigDataAssetDefaults DefaultBlendTime"), Config->DefaultBlendTime, 0.5f);
	TestTrue(TEXT("ConfigDataAssetDefaults bEnableCameraCollision"), Config->bEnableCameraCollision);
	TestEqual(TEXT("ConfigDataAssetDefaults CollisionProbeSize"), Config->CollisionProbeSize, 12.0f);
	TestEqual(TEXT("ConfigDataAssetDefaults CameraLagSpeed"), Config->CameraLagSpeed, 10.0f);
	TestEqual(TEXT("ConfigDataAssetDefaults MaxLagDistance"), Config->MaxLagDistance, 200.0f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXCamera_SetCameraModeValidAutomationTest,
	"PGX.Camera.preview.SetCameraModeValid", PGX_CAMERA_AUTOMATION_FLAGS)
bool FPGXCamera_SetCameraModeValidAutomationTest::RunTest(const FString& Parameters)
{
	UPGXCameraSubsystem* Subsystem = PGXCameraAutomation::MakeCameraSubsystem();
	UPGXCameraMode* Mode = PGXCameraAutomation::MakeMode(TEXT("PGXCamera_SetValid"), FName(TEXT("ExplorationMode")));

	const bool bResult = Subsystem->SetCameraMode(Mode);
	TestTrue(TEXT("SetCameraModeValid returns true for valid mode"), bResult);
	TestEqual(TEXT("SetCameraModeValid active mode matches"), Subsystem->GetActiveCameraMode(), Mode);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXCamera_SetCameraModeNullAutomationTest,
	"PGX.Camera.preview.SetCameraModeNull", PGX_CAMERA_AUTOMATION_FLAGS)
bool FPGXCamera_SetCameraModeNullAutomationTest::RunTest(const FString& Parameters)
{
	UPGXCameraSubsystem* Subsystem = PGXCameraAutomation::MakeCameraSubsystem();
	UPGXCameraMode* Mode = PGXCameraAutomation::MakeMode(TEXT("PGXCamera_SetNullSetup"));
	Subsystem->SetCameraMode(Mode);

	const bool bResult = Subsystem->SetCameraMode(nullptr);
	TestFalse(TEXT("SetCameraModeNull returns false for null mode"), bResult);
	TestEqual(TEXT("SetCameraModeNull active mode unchanged"), Subsystem->GetActiveCameraMode(), Mode);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXCamera_ClearCameraModeAutomationTest,
	"PGX.Camera.preview.ClearCameraMode", PGX_CAMERA_AUTOMATION_FLAGS)
bool FPGXCamera_ClearCameraModeAutomationTest::RunTest(const FString& Parameters)
{
	UPGXCameraSubsystem* Subsystem = PGXCameraAutomation::MakeCameraSubsystem();
	UPGXCameraMode* Mode = PGXCameraAutomation::MakeMode(TEXT("PGXCamera_Clear"), FName(TEXT("MenuMode")));
	Subsystem->SetCameraMode(Mode);

	Subsystem->ClearCameraMode();
	TestNull(TEXT("ClearCameraMode active mode is null"), Subsystem->GetActiveCameraMode());
	TestEqual(TEXT("ClearCameraMode active name is NAME_None"), Subsystem->GetActiveCameraModeName(), NAME_None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXCamera_GetActiveCameraModeNameAutomationTest,
	"PGX.Camera.preview.GetActiveCameraModeName", PGX_CAMERA_AUTOMATION_FLAGS)
bool FPGXCamera_GetActiveCameraModeNameAutomationTest::RunTest(const FString& Parameters)
{
	UPGXCameraSubsystem* Subsystem = PGXCameraAutomation::MakeCameraSubsystem();
	UPGXCameraMode* Mode = PGXCameraAutomation::MakeMode(TEXT("PGXCamera_GetName"), FName(TEXT("CinematicMode")));
	Subsystem->SetCameraMode(Mode);

	TestEqual(TEXT("GetActiveCameraModeName returns mode name"), Subsystem->GetActiveCameraModeName(), FName(TEXT("CinematicMode")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXCamera_SetCameraModeOverwriteAutomationTest,
	"PGX.Camera.preview.SetCameraModeOverwrite", PGX_CAMERA_AUTOMATION_FLAGS)
bool FPGXCamera_SetCameraModeOverwriteAutomationTest::RunTest(const FString& Parameters)
{
	UPGXCameraSubsystem* Subsystem = PGXCameraAutomation::MakeCameraSubsystem();
	UPGXCameraMode* FirstMode = PGXCameraAutomation::MakeMode(TEXT("PGXCamera_Overwrite_First"), FName(TEXT("FirstMode")));
	UPGXCameraMode* SecondMode = PGXCameraAutomation::MakeMode(TEXT("PGXCamera_Overwrite_Second"), FName(TEXT("SecondMode")));

	Subsystem->SetCameraMode(FirstMode);
	TestEqual(TEXT("SetCameraModeOverwrite first active"), Subsystem->GetActiveCameraMode(), FirstMode);

	Subsystem->SetCameraMode(SecondMode);
	TestEqual(TEXT("SetCameraModeOverwrite second active"), Subsystem->GetActiveCameraMode(), SecondMode);
	TestEqual(TEXT("SetCameraModeOverwrite name updated"), Subsystem->GetActiveCameraModeName(), FName(TEXT("SecondMode")));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
