// Copyright PGX Framework. All Rights Reserved.

#include "Commandlet/PGXDemoAssetGeneratorCommandlet.h"
#include "PGXSimHarnessEditorModule.h"
#include "Logging/PGXLogMacros.h"
#include "PGXDemoRegistry.h"
#include "PGXDemoPopulator.h"

#include "Misc/CommandLine.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "UObject/PrimaryAssetId.h"
#include "Engine/DataAsset.h"
#include "AssetRegistry/AssetRegistryModule.h"

namespace PGXDemoAssetGenerator
{
	enum class EExitCode : int32
	{
		Success = 0,
		CreateFailed = 1,
		SaveFailed = 2,
		InternalError = 3
	};

	struct FOptions
	{
		bool bForce = false;
		bool bDryRun = false;
		bool bHelp = false;
		FString SaveDir = TEXT("/Game/PGX_Demo");
		TSet<FString> OnlyNames; // EN: empty = all / ES: vacio = todos
	};

	// EN: Switch helpers mirror PGXRegistryValidateCommandlet for consistency.
	// ES: Helpers de switch espejo de PGXRegistryValidateCommandlet por consistencia.
	bool HasSwitch(const TArray<FString>& Switches, const TCHAR* Name)
	{
		for (const FString& S : Switches)
		{
			if (S.Equals(Name, ESearchCase::IgnoreCase)) { return true; }
		}
		return false;
	}

	bool TryGetSwitchValue(const TArray<FString>& Switches, const TCHAR* Name, FString& OutValue)
	{
		const FString Prefix = FString::Printf(TEXT("%s="), Name);
		for (const FString& S : Switches)
		{
			if (S.StartsWith(Prefix, ESearchCase::IgnoreCase))
			{
				OutValue = S.RightChop(Prefix.Len()).TrimStartAndEnd();
				return !OutValue.IsEmpty();
			}
		}
		return false;
	}

	FOptions ParseOptions(const FString& Params)
	{
		TArray<FString> Tokens;
		TArray<FString> Switches;
		FCommandLine::Parse(*Params, Tokens, Switches);

		FOptions Options;
		Options.bForce = HasSwitch(Switches, TEXT("force"));
		Options.bDryRun = HasSwitch(Switches, TEXT("dryrun"));
		Options.bHelp = HasSwitch(Switches, TEXT("help")) || HasSwitch(Switches, TEXT("?"));

		FString SaveDir;
		if (TryGetSwitchValue(Switches, TEXT("savedir"), SaveDir))
		{
			Options.SaveDir = SaveDir;
		}

		FString Only;
		if (TryGetSwitchValue(Switches, TEXT("only"), Only))
		{
			TArray<FString> Names;
			Only.ParseIntoArray(Names, TEXT(","), true);
			for (FString& N : Names)
			{
				Options.OnlyNames.Add(N.TrimStartAndEnd());
			}
		}
		return Options;
	}

	void PrintUsage()
	{
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("Usage: UnrealEditor-Cmd.exe <ProjectPath> -run=PGXDemoAssetGenerator [-only=<name>[,<...>]] [-force] [-dryrun] [-savedir=/Game/PGX_Demo]"));
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("Exit codes: 0=success, 1=create failed, 2=save failed, 3=internal error"));
	}

	// EN: Machine-parseable single-line summary (key=value), no Json dependency needed.
	// ES: Resumen de una linea parseable por maquina (key=value), sin dependencia Json.
	FString BuildSummaryLine(int32 Created, int32 Skipped, int32 Planned, int32 Failed, EExitCode ExitCode)
	{
		return FString::Printf(
			TEXT("status=%s exitCode=%d created=%d skipped=%d planned=%d failed=%d"),
			ExitCode == EExitCode::Success ? TEXT("PASS") : TEXT("FAIL"),
			static_cast<int32>(ExitCode), Created, Skipped, Planned, Failed);
	}
} // namespace PGXDemoAssetGenerator

UPGXDemoAssetGeneratorCommandlet::UPGXDemoAssetGeneratorCommandlet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	LogToConsole = true;
	IsClient = false;
	IsServer = false;
	IsEditor = true;
}

int32 UPGXDemoAssetGeneratorCommandlet::Main(const FString& Params)
{
	using namespace PGXDemoAssetGenerator;

	PGX_LOG_INFO(LogPGXSimHarness, TEXT("============================================"));
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("PGX Demo Asset Generator Commandlet"));
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("============================================"));

	const FOptions Options = ParseOptions(Params);
	if (Options.bHelp)
	{
		PrintUsage();
		return static_cast<int32>(EExitCode::Success);
	}

	PGX_LOG_INFO(LogPGXSimHarness, TEXT("SaveDir: %s | Force: %s | DryRun: %s | Only: %d names"),
		*Options.SaveDir,
		Options.bForce ? TEXT("yes") : TEXT("no"),
		Options.bDryRun ? TEXT("yes") : TEXT("no"),
		Options.OnlyNames.Num());

	int32 Created = 0;
	int32 Skipped = 0;
	int32 Planned = 0;
	int32 Failed = 0;
	bool bAnySaveFail = false;

	for (const FPGXDemoEntry& Entry : FPGXDemoRegistry::GetDemoEntries())
	{
		// EN: -only filter / ES: filtro -only
		if (Options.OnlyNames.Num() > 0 && !Options.OnlyNames.Contains(Entry.SuggestedName))
		{
			continue;
		}

		const FString PackageName = Options.SaveDir / Entry.SuggestedName; // /Game/PGX_Demo/DA_Demo_X

		// EN: Idempotency — skip the 18 already-present assets unless -force.
		// ES: Idempotencia — saltar los 18 ya presentes salvo -force.
		if (!Options.bForce && FPackageName::DoesPackageExist(PackageName))
		{
			PGX_LOG_INFO(LogPGXSimHarness, TEXT("SKIP (exists): %s"), *Entry.SuggestedName);
			++Skipped;
			continue;
		}

		// EN: Resolve the DA class from the registry ClassPath (pattern from CreateDemoAsset).
		// ES: Resolver la clase DA desde el ClassPath del registry.
		UClass* DataAssetClass = LoadObject<UClass>(nullptr, *Entry.ClassPath);
		if (!DataAssetClass)
		{
			PGX_LOG_ERROR(LogPGXSimHarness, TEXT("FAIL (class not found — plugin not loaded?): %s [%s]"),
				*Entry.SuggestedName, *Entry.ClassPath);
			++Failed;
			continue;
		}

		if (Options.bDryRun)
		{
			PGX_LOG_INFO(LogPGXSimHarness, TEXT("DRYRUN would create: %s (%s)"), *Entry.SuggestedName, *Entry.ClassPath);
			++Planned;
			continue;
		}

		// EN: Create package + DataAsset (pattern from CreateDemoAsset, sans modal save dialog).
		// ES: Crear paquete + DataAsset (patron de CreateDemoAsset, sin dialogo modal).
		UPackage* Package = CreatePackage(*PackageName);
		if (!Package)
		{
			PGX_LOG_ERROR(LogPGXSimHarness, TEXT("FAIL (CreatePackage): %s"), *Entry.SuggestedName);
			++Failed;
			continue;
		}

		UObject* NewDA = NewObject<UDataAsset>(Package, DataAssetClass, FName(*Entry.SuggestedName), RF_Public | RF_Standalone);
		if (!IsValid(NewDA))
		{
			PGX_LOG_ERROR(LogPGXSimHarness, TEXT("FAIL (NewObject): %s"), *Entry.SuggestedName);
			++Failed;
			continue;
		}

		// EN: Populate (container-only fallback leaves defaults for cases without a populate yet).
		// ES: Poblar (el fallback solo-contenedor deja defaults para casos aun sin poblado).
		const bool bPopulated = FPGXDemoPopulator::PopulateDemo(NewDA);

		FAssetRegistryModule::AssetCreated(NewDA);
		Package->MarkPackageDirty();

		// EN: Save to disk (the step the CB extension does not do — it opens a modal dialog instead).
		// ES: Guardar a disco (el paso que la extension CB no hace — abre un dialogo modal).
		const FString FileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.SaveFlags = SAVE_NoError;
		const bool bSaved = UPackage::SavePackage(Package, NewDA, *FileName, SaveArgs);
		if (!bSaved)
		{
			PGX_LOG_ERROR(LogPGXSimHarness, TEXT("FAIL (SavePackage): %s -> %s"), *Entry.SuggestedName, *FileName);
			bAnySaveFail = true;
			++Failed;
			continue;
		}

		PGX_LOG_INFO(LogPGXSimHarness, TEXT("OK created: %s populated=%s"),
			*Entry.SuggestedName, bPopulated ? TEXT("yes") : TEXT("defaults"));
		++Created;
	}

	const EExitCode ExitCode = bAnySaveFail
		? EExitCode::SaveFailed
		: (Failed > 0 ? EExitCode::CreateFailed : EExitCode::Success);

	PGX_LOG_INFO(LogPGXSimHarness, TEXT("--------------------------------------------"));
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("Created: %d | Skipped: %d | Planned(dryrun): %d | Failed: %d"),
		Created, Skipped, Planned, Failed);
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("PGXDemoAssetGeneratorResult: %s"),
		*BuildSummaryLine(Created, Skipped, Planned, Failed, ExitCode));
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("Result: %s (exit code %d)"),
		ExitCode == EExitCode::Success ? TEXT("PASS") : TEXT("FAIL"), static_cast<int32>(ExitCode));
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("============================================"));

	return static_cast<int32>(ExitCode);
}
