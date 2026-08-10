// SPDX-License-Identifier: Apache-2.0
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Interfaces/IPluginManager.h"
#include "Tests/AutomationEditorCommon.h"
#include "Editor.h"
#include "PGXDemoGameMode.h"
#include "PGXDemoPlayerController.h"
#include "PGXDemoTags.h"
#include "PGXInputSubsystem.h"
#include "PGXInputBuffer.h"
#include "PGXSaveSubsystem.h"
#include "PGXSaveGame.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformTime.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXDemoAssetsTest, "PGX.Demo.Assets.AssetRegistry", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPGXDemoAssetsTest::RunTest(const FString&)
{
    static const TCHAR* Paths[] = {
        TEXT("/Game/Maps/PGXDemo.PGXDemo"),
        TEXT("/Game/PGXDemo/Config/DA_PGXDemoMessage.DA_PGXDemoMessage"),
        TEXT("/Game/PGXDemo/Config/DA_PGXDemoGameFlow.DA_PGXDemoGameFlow"),
        TEXT("/Game/PGXDemo/Config/DA_PGXDemoGlobalRules.DA_PGXDemoGlobalRules"),
        TEXT("/Game/PGXDemo/Config/DT_PGXDemoFlowRules.DT_PGXDemoFlowRules"),
        TEXT("/Game/PGXDemo/Config/DA_PGXDemoSave.DA_PGXDemoSave"),
        TEXT("/Game/PGXDemo/Config/DT_PGXDemoSaveContexts.DT_PGXDemoSaveContexts")
    };
    TSet<FName> DemoPackages;
    for (const TCHAR* Path : Paths)
    {
        DemoPackages.Add(FName(*FPackageName::ObjectPathToPackageName(FString(Path))));
    }
    FAssetRegistryModule& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
    for (const TCHAR* Path : Paths)
    {
        const FAssetData Asset = Registry.Get().GetAssetByObjectPath(FSoftObjectPath(Path));
        TestTrue(FString::Printf(TEXT("AssetRegistry contains %s"), Path), Asset.IsValid());
        TestFalse(FString::Printf(TEXT("%s is not a redirector"), Path), Asset.AssetClassPath.GetAssetName() == TEXT("ObjectRedirector"));
        TArray<FName> Dependencies;
        Registry.Get().GetDependencies(FName(*FPackageName::ObjectPathToPackageName(FString(Path))), Dependencies);
        for (const FName Dependency : Dependencies)
        {
            const FString Package = Dependency.ToString();
            if (Package.StartsWith(TEXT("/Game/")))
            {
                TestTrue(FString::Printf(TEXT("%s only references generated /Game packages: %s"), Path, *Package), DemoPackages.Contains(Dependency));
            }
            TestFalse(FString::Printf(TEXT("%s excludes Marketplace/Megascans dependencies: %s"), Path, *Package), Package.Contains(TEXT("Marketplace")) || Package.Contains(TEXT("Megascans")));
        }
    }
    return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXDemoConstellationTest, "PGX.Demo.Constellation.AllPluginsEnabled", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPGXDemoConstellationTest::RunTest(const FString&)
{
    static const TCHAR* Names[] = { TEXT("PGXCore"),TEXT("PGXGameFlow"),TEXT("PGXSave"),TEXT("PGXPSO"),TEXT("PGXLoading"),TEXT("PGXMGOS"),TEXT("PGXAudio"),TEXT("PGXDocs"),TEXT("PGXVersionControl"),TEXT("PGXTutorials"),TEXT("PGXScaffold"),TEXT("PGXEditorTools"),TEXT("PGXSimHarness"),TEXT("PGXCamera"),TEXT("PGXUI"),TEXT("PGXInput"),TEXT("PGXInteraction"),TEXT("PGXInventory"),TEXT("PGXAbility"),TEXT("PGXSpawn"),TEXT("PGXAI"),TEXT("PGXColony"),TEXT("PGXCrafting"),TEXT("PGXEnvironment"),TEXT("PGXTrade"),TEXT("PGXVehicles") };
    for (const TCHAR* Name : Names)
    {
        const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(Name);
        TestTrue(FString::Printf(TEXT("%s is available"), Name), Plugin.IsValid());
        TestTrue(FString::Printf(TEXT("%s is enabled"), Name), Plugin.IsValid() && Plugin->IsEnabled());
    }
    return !HasAnyErrors();
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FPGXDemoPIEVerifyCommand, FAutomationTestBase*, Test, double, Deadline);
bool FPGXDemoPIEVerifyCommand::Update()
{
    UWorld* World = GEditor ? GEditor->PlayWorld : nullptr;
    if (!World)
    {
        if (FPlatformTime::Seconds() < Deadline) return false;
        Test->AddError(TEXT("PIE world unavailable after 30 seconds"));
        return true;
    }
    APGXDemoPlayerController* PC = Cast<APGXDemoPlayerController>(World->GetFirstPlayerController());
    APGXDemoGameMode* Mode = World->GetAuthGameMode<APGXDemoGameMode>();
    if (!PC || !Mode) { Test->AddError(TEXT("PGXDemo controller or game mode unavailable")); return true; }
    const int32 Before = Mode->GetInteractionCount();
    PC->TriggerInteractionForAutomation();
    Test->TestEqual(TEXT("interaction count increments"), Mode->GetInteractionCount(), Before + 1);
    Test->TestEqual(TEXT("flow reaches Interacted"), Mode->GetCurrentFlowState(), PGXDemoTags::FlowInteracted());
    UGameInstance* GI = World->GetGameInstance();
    UPGXInputSubsystem* Input = GI ? GI->GetSubsystem<UPGXInputSubsystem>() : nullptr;
    Test->TestNotNull(TEXT("Input subsystem"), Input);
    Test->TestTrue(TEXT("InputBuffer was consumed"), Input && Input->GetInputBuffer() && Input->GetInputBuffer()->Num() == 0);
    UPGXSaveSubsystem* Save = GI ? GI->GetSubsystem<UPGXSaveSubsystem>() : nullptr;
    Test->TestNotNull(TEXT("Save subsystem"), Save);
    if (Save)
    {
        Test->TestEqual(TEXT("LoadContext round trip"), Save->LoadContext(PGXDemoTags::SaveContext(), TEXT("DemoSlot")), EPGXSaveResult::Success);
        const UPGXSaveGame* Data = Save->GetSaveGame(PGXDemoTags::SaveDomainProgress());
        Test->TestTrue(TEXT("persisted count round trips"), Data && Data->ReadInt(TEXT("InteractionCount"), -1) == Before + 1);
        Test->TestEqual(TEXT("test slot cleanup"), Save->DeleteSlot(PGXDemoTags::SaveContext(), TEXT("DemoSlot")), EPGXSaveResult::Success);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXDemoPIETest, "PGX.Demo.Runtime.PIE.InteractionRoundTrip", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPGXDemoPIETest::RunTest(const FString&)
{
    FAutomationEditorCommonUtils::LoadMap(TEXT("/Game/Maps/PGXDemo"));
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    ADD_LATENT_AUTOMATION_COMMAND(FPGXDemoPIEVerifyCommand(this, FPlatformTime::Seconds() + 30.0));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
    return true;
}
#endif
