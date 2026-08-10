// SPDX-License-Identifier: Apache-2.0
#include "PGXDemoAuthorCommandlet.h"
#include "PGXDemoGameMode.h"
#include "PGXDemoTags.h"
#include "Messages/PGXMessageConfig.h"
#include "PGXGameFlowConfig.h"
#include "PGXFlowRulesConfig.h"
#include "PGXGameFlowTypes.h"
#include "PGXSaveConfig.h"
#include "PGXSaveGame.h"
#include "PGXSaveTypes.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Editor.h"
#include "FileHelpers.h"
#include "Engine/DataTable.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/WorldSettings.h"
#include "HAL/FileManager.h"
#include "Dom/JsonObject.h"
#include "Components/LightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

THIRD_PARTY_INCLUDES_START
#define UI OpenSSLUI
#include <openssl/evp.h>
#undef UI
THIRD_PARTY_INCLUDES_END

namespace PGXDemoAuthor
{
    const TArray<FString> Packages = {
        TEXT("/Game/Maps/PGXDemo"),
        TEXT("/Game/PGXDemo/Config/DA_PGXDemoMessage"),
        TEXT("/Game/PGXDemo/Config/DA_PGXDemoGameFlow"),
        TEXT("/Game/PGXDemo/Config/DA_PGXDemoGlobalRules"),
        TEXT("/Game/PGXDemo/Config/DT_PGXDemoFlowRules"),
        TEXT("/Game/PGXDemo/Config/DA_PGXDemoSave"),
        TEXT("/Game/PGXDemo/Config/DT_PGXDemoSaveContexts")
    };

    FString ObjectPath(const FString& Package) { return Package + TEXT(".") + FPackageName::GetLongPackageAssetName(Package); }
    FString Filename(const FString& Package) { return FPackageName::LongPackageNameToFilename(Package, Package.Contains(TEXT("/Maps/")) ? FPackageName::GetMapPackageExtension() : FPackageName::GetAssetPackageExtension()); }

    template <typename T> T* NewAsset(const FString& PackageName)
    {
        UPackage* Package = CreatePackage(*PackageName);
        if (!Package) return nullptr;
        T* Asset = NewObject<T>(Package, T::StaticClass(), FName(*FPackageName::GetLongPackageAssetName(PackageName)), RF_Public | RF_Standalone);
        if (Asset) { FAssetRegistryModule::AssetCreated(Asset); Package->MarkPackageDirty(); }
        return Asset;
    }

    bool SaveAsset(UObject* Asset, const FString& PackageName)
    {
        if (!Asset) return false;
        FSavePackageArgs Args; Args.TopLevelFlags = RF_Public | RF_Standalone; Args.SaveFlags = SAVE_NoError;
        return UPackage::SavePackage(Asset->GetPackage(), Asset, *Filename(PackageName), Args);
    }

    bool HashFile(const FString& Path, FString& Out)
    {
        TArray<uint8> Bytes;
        if (!FFileHelper::LoadFileToArray(Bytes, *Path)) return false;
        uint8 Digest[EVP_MAX_MD_SIZE]{};
        uint32 DigestLength = 0;
        if (EVP_Digest(Bytes.GetData(), static_cast<size_t>(Bytes.Num()), Digest, &DigestLength, EVP_sha256(), nullptr) != 1 || DigestLength != 32) return false;
        Out.Reset(64);
        for (uint32 Index = 0; Index < DigestLength; ++Index) Out += FString::Printf(TEXT("%02x"), Digest[Index]);
        return true;
    }

    bool WriteReceipt(const FString& ReceiptPath)
    {
        TArray<TSharedPtr<FJsonValue>> Assets;
        for (const FString& Package : Packages)
        {
            FString Hash; const FString File = Filename(Package);
            if (!HashFile(File, Hash)) return false;
            TSharedRef<FJsonObject> Item = MakeShared<FJsonObject>();
            Item->SetStringField(TEXT("package"), Package);
            Item->SetStringField(TEXT("class"), Package.Contains(TEXT("/Maps/")) ? TEXT("World") : LoadObject<UObject>(nullptr, *ObjectPath(Package))->GetClass()->GetName());
            Item->SetStringField(TEXT("relative_file"), File.RightChop(FPaths::ProjectContentDir().Len() - FString(TEXT("Content/")).Len()).Replace(TEXT("\\"), TEXT("/")));
            Item->SetStringField(TEXT("sha256"), Hash);
            Item->SetStringField(TEXT("provenance"), TEXT("generated-by-PGXDemoAuthor-from-public-source"));
            Item->SetStringField(TEXT("license"), TEXT("Apache-2.0"));
            TArray<TSharedPtr<FJsonValue>> Dependencies;
            if (Package == TEXT("/Game/Maps/PGXDemo"))
            {
                TSharedRef<FJsonObject> Dependency = MakeShared<FJsonObject>();
                Dependency->SetStringField(TEXT("package"), TEXT("/Engine/BasicShapes/Cube"));
                Dependency->SetStringField(TEXT("disposition"), TEXT("engine-reference-only"));
                Dependencies.Add(MakeShared<FJsonValueObject>(Dependency));
            }
            Item->SetArrayField(TEXT("dependencies"), Dependencies);
            Assets.Add(MakeShared<FJsonValueObject>(Item));
        }
        TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
        Root->SetStringField(TEXT("schema_version"), TEXT("1.0.0"));
        TSharedRef<FJsonObject> Generator = MakeShared<FJsonObject>();
        Generator->SetStringField(TEXT("commandlet"), TEXT("PGXDemoAuthor"));
        Generator->SetStringField(TEXT("source_file"), TEXT("Source/PGXDemoEditor/Private/PGXDemoAuthorCommandlet.cpp"));
        FString SourceHash;
        if (!HashFile(FPaths::ProjectDir() / TEXT("Source/PGXDemoEditor/Private/PGXDemoAuthorCommandlet.cpp"), SourceHash)) return false;
        Generator->SetStringField(TEXT("source_sha256"), SourceHash);
        Root->SetObjectField(TEXT("generator"), Generator);
        TSharedRef<FJsonObject> Engine = MakeShared<FJsonObject>(); Engine->SetStringField(TEXT("association"), TEXT("5.7")); Engine->SetStringField(TEXT("build_version"), TEXT("5.7.4"));
        Root->SetObjectField(TEXT("engine"), Engine);
        Root->SetStringField(TEXT("project"), TEXT("PGXDemo"));
        Root->SetStringField(TEXT("license_policy"), TEXT("project-generated Apache-2.0; /Engine references are prerequisites and not redistributed"));
        Root->SetArrayField(TEXT("assets"), Assets);
        FString Json; TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Json);
        if (!FJsonSerializer::Serialize(Root, Writer)) return false;
        return FFileHelper::SaveStringToFile(Json + TEXT("\n"), *ReceiptPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
    }

    bool VerifyAssets()
    {
        UPGXMessageConfig* Message = LoadObject<UPGXMessageConfig>(nullptr, *ObjectPath(Packages[1]));
        UPGXGameFlowConfig* Flow = LoadObject<UPGXGameFlowConfig>(nullptr, *ObjectPath(Packages[2]));
        UPGXFlowRulesConfig* Rules = LoadObject<UPGXFlowRulesConfig>(nullptr, *ObjectPath(Packages[3]));
        UDataTable* FlowTable = LoadObject<UDataTable>(nullptr, *ObjectPath(Packages[4]));
        UPGXSaveConfig* Save = LoadObject<UPGXSaveConfig>(nullptr, *ObjectPath(Packages[5]));
        UDataTable* SaveTable = LoadObject<UDataTable>(nullptr, *ObjectPath(Packages[6]));
        UPackage* Map = LoadPackage(nullptr, *Packages[0], LOAD_None);
        if (!Message || !Flow || !Rules || !FlowTable || !Save || !SaveTable || !Map) return false;
        if (Message->MaxMessageHistory != 32 || Message->bEnablePartialMatching) return false;
        if (Flow->InitialChannelStates.FindRef(EPGXFlowChannel::Global) != PGXDemoTags::FlowReady()) return false;
        if (Rules->Channel != EPGXFlowChannel::Global || Rules->FlowRules.Num() != 2) return false;
        if (FlowTable->GetRowStruct() != FPGXFlowRulesRow::StaticStruct() || SaveTable->GetRowStruct() != FPGXSaveContextRow::StaticStruct()) return false;
        return Save->ContextTag == PGXDemoTags::SaveContext() && Save->SaveDomains.Num() == 1;
    }

    bool CreateAssets()
    {
        UPGXMessageConfig* Message = NewAsset<UPGXMessageConfig>(Packages[1]);
        if (!Message) return false;
        Message->MaxMessageHistory = 32; Message->bLogBroadcasts = false; Message->bLogRegistrations = false;
        Message->bEnablePartialMatching = false; Message->bAllowTestBroadcasts = false; Message->MaxBroadcastQueueDepth = 16;
        if (!SaveAsset(Message, Packages[1])) return false;

        UPGXGameFlowConfig* Flow = NewAsset<UPGXGameFlowConfig>(Packages[2]);
        if (!Flow) return false;
        Flow->ContextTag = PGXDemoTags::FlowConfig(); Flow->InitialChannelStates.Add(EPGXFlowChannel::Global, PGXDemoTags::FlowReady());
        Flow->MaxHistoryDepth = 16; Flow->bLogTransitions = true; Flow->bAllowConsoleMutations = false;
        Flow->DuplicateRulesPolicy = EPGXFlowDuplicateRulesPolicy::HighestPriorityWins; Flow->bVerboseDebug = false;
        if (!SaveAsset(Flow, Packages[2])) return false;

        UPGXFlowRulesConfig* Rules = NewAsset<UPGXFlowRulesConfig>(Packages[3]);
        if (!Rules) return false;
        Rules->Channel = EPGXFlowChannel::Global; Rules->ConflictPriority = 100;
        FPGXFlowRule ReadyRule; ReadyRule.RuleName = TEXT("ReadyToInteracted"); ReadyRule.AllowedDestinations.Add(PGXDemoTags::FlowInteracted()); ReadyRule.bAllowRevert = true;
        FPGXFlowRule InteractedRule; InteractedRule.RuleName = TEXT("InteractedToReady"); InteractedRule.AllowedDestinations.Add(PGXDemoTags::FlowReady()); InteractedRule.bAllowRevert = true;
        Rules->FlowRules.Add(PGXDemoTags::FlowReady(), ReadyRule); Rules->FlowRules.Add(PGXDemoTags::FlowInteracted(), InteractedRule);
        if (!SaveAsset(Rules, Packages[3])) return false;

        UDataTable* FlowTable = NewAsset<UDataTable>(Packages[4]);
        if (!FlowTable) return false; FlowTable->RowStruct = FPGXFlowRulesRow::StaticStruct();
        FPGXFlowRulesRow FlowRow; FlowRow.Channel = EPGXFlowChannel::Global; FlowRow.RulesRef = Rules; FlowRow.Description = FText::FromString(TEXT("PGXDemo global rules"));
        FlowTable->AddRow(TEXT("Global"), FlowRow); if (!SaveAsset(FlowTable, Packages[4])) return false;

        UPGXSaveConfig* Save = NewAsset<UPGXSaveConfig>(Packages[5]);
        if (!Save) return false;
        Save->ContextTag = PGXDemoTags::SaveContext(); Save->ContextDisplayName = FText::FromString(TEXT("PGXDemo")); Save->SaveMode = EPGXSaveMode::MultiSlot;
        FPGXSaveDomainEntry Domain; Domain.DomainTag = PGXDemoTags::SaveDomainProgress(); Domain.SaveGameClass = UPGXSaveGame::StaticClass(); Domain.DisplayName = FText::FromString(TEXT("Progress")); Domain.bRequired = true;
        Save->SaveDomains.Add(Domain); Save->MaxSaveSlots = 1; Save->SlotNamePattern = TEXT("DemoSlot"); Save->bAutoSaveEnabled = false; Save->bEnableQuickSave = false;
        Save->bCompressSaveData = false; Save->bValidateChecksum = true; Save->CurrentSaveVersion = 1; Save->bCreateBackupBeforeSave = false;
        Save->BaseDirectory = TEXT("SaveGames/PGXDemo"); Save->PathStrategy = EPGXPathStrategy::NamedSlot; Save->FileNamePrefix = TEXT("Demo");
        if (!SaveAsset(Save, Packages[5])) return false;

        UDataTable* SaveTable = NewAsset<UDataTable>(Packages[6]);
        if (!SaveTable) return false; SaveTable->RowStruct = FPGXSaveContextRow::StaticStruct();
        FPGXSaveContextRow SaveRow; SaveRow.ContextTag = PGXDemoTags::SaveContext(); SaveRow.ConfigRef = Save; SaveRow.Description = FText::FromString(TEXT("PGXDemo context"));
        SaveTable->AddRow(TEXT("Demo"), SaveRow); if (!SaveAsset(SaveTable, Packages[6])) return false;

        GEditor->CreateNewMapForEditing(false, false);
        UWorld* World = GEditor->GetEditorWorldContext().World();
        if (!World) return false;
        World->GetWorldSettings()->DefaultGameMode = APGXDemoGameMode::StaticClass();
        UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")); if (!Cube) return false;
        AStaticMeshActor* Floor = World->SpawnActor<AStaticMeshActor>(); Floor->GetStaticMeshComponent()->SetStaticMesh(Cube); Floor->SetActorScale3D(FVector(12.0, 12.0, 0.2)); Floor->SetActorLocation(FVector(0,0,-100));
        AStaticMeshActor* Beacon = World->SpawnActor<AStaticMeshActor>(); Beacon->GetStaticMeshComponent()->SetStaticMesh(Cube); Beacon->SetActorLocation(FVector(300,0,50)); Beacon->SetActorScale3D(FVector(1,1,2));
        World->SpawnActor<APlayerStart>(FVector(0,0,100), FRotator::ZeroRotator);
        ADirectionalLight* Light = World->SpawnActor<ADirectionalLight>(); Light->SetActorRotation(FRotator(-45,-30,0));
        APointLight* Point = World->SpawnActor<APointLight>(FVector(300,0,250), FRotator::ZeroRotator); Point->GetLightComponent()->SetIntensity(5000.0f);
        return FEditorFileUtils::SaveLevel(World->PersistentLevel, Filename(Packages[0]));
    }
}

UPGXDemoAuthorCommandlet::UPGXDemoAuthorCommandlet()
{
    LogToConsole = true; IsClient = false; IsServer = false; IsEditor = true;
}
int32 UPGXDemoAuthorCommandlet::Main(const FString& Params)
{
    const bool bCreate = FParse::Param(*Params, TEXT("create"));
    const bool bVerify = FParse::Param(*Params, TEXT("verify"));
    FString ReceiptPath; FParse::Value(*Params, TEXT("receipt="), ReceiptPath);
    if (bCreate == bVerify || ReceiptPath.IsEmpty()) { UE_LOG(LogTemp, Error, TEXT("PGXDemoAuthorResult status=FAIL reason=usage")); return 2; }
    if (bCreate)
    {
        for (const FString& Package : PGXDemoAuthor::Packages)
        {
            if (FPackageName::DoesPackageExist(Package)) { UE_LOG(LogTemp, Error, TEXT("PGXDemoAuthorResult status=FAIL reason=RefuseExisting package=%s"), *Package); return 3; }
        }
        if (!PGXDemoAuthor::CreateAssets() || !PGXDemoAuthor::VerifyAssets() || !PGXDemoAuthor::WriteReceipt(ReceiptPath))
        { UE_LOG(LogTemp, Error, TEXT("PGXDemoAuthorResult status=FAIL reason=create-or-readback")); return 4; }
        UE_LOG(LogTemp, Display, TEXT("PGXDemoAuthorResult status=PASS mode=create created=7 failed=0")); return 0;
    }
    if (!PGXDemoAuthor::VerifyAssets() || !IFileManager::Get().FileExists(*ReceiptPath))
    { UE_LOG(LogTemp, Error, TEXT("PGXDemoAuthorResult status=FAIL reason=verify")); return 5; }
    UE_LOG(LogTemp, Display, TEXT("PGXDemoAuthorResult status=PASS mode=verify verified=7 failed=0")); return 0;
}
