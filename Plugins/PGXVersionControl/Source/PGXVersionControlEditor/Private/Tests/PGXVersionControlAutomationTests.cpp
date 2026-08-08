// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "PGXChangelistStore.h"
#include "Logging/PGXLogMacros.h"
#include "PGXCommitTagger.h"
#include "PGXCommitValidator.h"
#include "PGXVersionControlTypes.h"

#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace
{
FString PGXVersionControlAutomationDir()
{
	return FPaths::ProjectIntermediateDir() / TEXT("PGXVersionControlAutomation");
}

FString PGXVersionControlWriteTempFile(const FString& InRelativeName, const FString& InContent)
{
	const FString FilePath = PGXVersionControlAutomationDir() / InRelativeName;
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.CreateDirectoryTree(*FPaths::GetPath(FilePath));
	FFileHelper::SaveStringToFile(InContent, *FilePath);
	return FilePath;
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXVersionControl_TaggerDetectsAndPrefixesAutomationTest,
	"PGX.VersionControl.Behavior.TaggerDetectsAndPrefixes",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXVersionControl_TaggerDetectsAndPrefixesAutomationTest::RunTest(const FString& /*Parameters*/)
{
	FPGXCommitTagger Tagger;
	TArray<FString> FilePaths;
	FilePaths.Add(TEXT("Plugins/PGXAudio/Source/PGXAudioRuntime/Public/PGXAudioSubsystem.h"));
	FilePaths.Add(TEXT("Plugins/PGXVersionControl/Source/PGXVersionControlEditor/Private/PGXCommitTagger.cpp"));
	const TArray<FString> Tags = Tagger.DetectSystems(FilePaths);

	TestEqual(TEXT("Two plugin systems detected"), Tags.Num(), 2);
	TestTrue(TEXT("Audio tag detected"), Tags.Contains(TEXT("Audio")));
	TestTrue(TEXT("VersionControl tag detected"), Tags.Contains(TEXT("VersionControl")));
	TestEqual(TEXT("Prefix is deterministic after sorted tags"), Tagger.BuildCommitPrefix(Tags), FString(TEXT("[Audio][VersionControl]")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXVersionControl_ValidatorLogTempConfidenceAutomationTest,
	"PGX.VersionControl.Behavior.ValidatorLogTempConfidence",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXVersionControl_ValidatorLogTempConfidenceAutomationTest::RunTest(const FString& /*Parameters*/)
{
	const FString FilePath = PGXVersionControlWriteTempFile(
		TEXT("PGXLogTempFixture.cpp"),
		TEXT("// line 1\nvoid Test()\n{\n\tUE_LOG(LogTemp, Warning, TEXT(\"bad\"));\n}\n"));

	FPGXCommitValidator Validator;
	TArray<FString> FilePaths;
	FilePaths.Add(FilePath);
	const TArray<FPGXValidationIssue> Issues = Validator.Validate(FilePaths);

	const FPGXValidationIssue* LogIssue = Issues.FindByPredicate([](const FPGXValidationIssue& Issue)
	{
		return Issue.RuleId == TEXT("Log.TempUsed");
	});

	TestNotNull(TEXT("LogTemp issue emitted"), LogIssue);
	if (LogIssue)
	{
		TestEqual(TEXT("LogTemp issue confidence is operational"), static_cast<int32>(LogIssue->Confidence), static_cast<int32>(EPGXValidationConfidence::Operational));
		TestEqual(TEXT("LogTemp issue line number is visible"), LogIssue->LineNumber, 4);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXVersionControl_ValidatorUPropertyMatchLocationAutomationTest,
	"PGX.VersionControl.Behavior.ValidatorUPropertyMatchLocation",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXVersionControl_ValidatorUPropertyMatchLocationAutomationTest::RunTest(const FString& /*Parameters*/)
{
	const FString FilePath = PGXVersionControlWriteTempFile(
		TEXT("PGXRawPointerFixture.h"),
		TEXT("#pragma once\nclass UPGXRawPointerFixture\n{\n\tUPROPERTY()\n\tUObject* Good;\n\tUObject* Bad;\n};\n"));

	FPGXCommitValidator Validator;
	TArray<FString> FilePaths;
	FilePaths.Add(FilePath);
	const TArray<FPGXValidationIssue> Issues = Validator.Validate(FilePaths);

	const FPGXValidationIssue* UPropertyIssue = Issues.FindByPredicate([](const FPGXValidationIssue& Issue)
	{
		return Issue.RuleId == TEXT("UProperty.MissingTag");
	});

	TestNotNull(TEXT("Second raw UObject pointer is flagged"), UPropertyIssue);
	if (UPropertyIssue)
	{
		TestEqual(TEXT("Issue points at second raw pointer, not first occurrence"), UPropertyIssue->LineNumber, 6);
		TestEqual(TEXT("UProperty issue confidence is heuristic partial"), static_cast<int32>(UPropertyIssue->Confidence), static_cast<int32>(EPGXValidationConfidence::HeuristicPartial));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXVersionControl_ChangelistStoreTypedResultsAutomationTest,
	"PGX.VersionControl.Behavior.ChangelistStoreTypedResults",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXVersionControl_ChangelistStoreTypedResultsAutomationTest::RunTest(const FString& /*Parameters*/)
{
	const FString SavePath = PGXVersionControlAutomationDir() / TEXT("changelists-test.json");
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.DeleteFile(*SavePath);
	PlatformFile.DeleteFile(*(SavePath + TEXT(".tmp")));
	PlatformFile.DeleteFile(*(SavePath + TEXT(".bak")));

	FPGXChangelistStore Store(SavePath);
	Store.Load();

	const FGuid DefaultGuid = Store.GetDefaultChangelistGuid();
	TestTrue(TEXT("Default changelist exists after load"), DefaultGuid.IsValid());

	TestFalse(TEXT("Empty changelist name rejected"), Store.CreateChangelist(TEXT(""), TEXT("Empty" )).IsValid());
	TestEqual(
		TEXT("Empty changelist name returns typed failure"),
		static_cast<int32>(Store.GetLastOperationResult().Status),
		static_cast<int32>(EPGXVersionControlOperationStatus::EmptyName));

	const FGuid FeatureGuid = Store.CreateChangelist(TEXT("Feature"), TEXT("Local test"));
	TestTrue(TEXT("Feature changelist created"), FeatureGuid.IsValid());
	TestTrue(TEXT("File move succeeds"), Store.MoveFileToChangelist(TEXT("Plugins/PGXVersionControl/Test.uasset"), FeatureGuid));
	TestTrue(TEXT("Rename succeeds"), Store.RenameChangelist(FeatureGuid, TEXT("Feature Renamed")));

	TestFalse(TEXT("Default changelist cannot be deleted"), Store.DeleteChangelist(DefaultGuid));
	TestEqual(
		TEXT("Default delete exposes typed protected status"),
		static_cast<int32>(Store.GetLastOperationResult().Status),
		static_cast<int32>(EPGXVersionControlOperationStatus::DefaultChangelistProtected));

	const FPGXVersionControlOperationResult SaveResult = Store.SaveWithResult();
	TestTrue(TEXT("SaveWithResult succeeds"), SaveResult.IsSuccess());
	TestTrue(TEXT("Save file exists after atomic write"), PlatformFile.FileExists(*SavePath));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
