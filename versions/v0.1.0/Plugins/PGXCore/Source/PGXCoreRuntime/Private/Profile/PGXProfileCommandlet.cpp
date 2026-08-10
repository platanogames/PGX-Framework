// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Profile/PGXProfileBuildValidator.h"

#if WITH_EDITOR

#include "Profile/PGXProfileTypes.h"
#include "Profile/PGXProfileSettings.h"
#include "Profile/PGXProjectProfileConfig.h"
#include "Logging/PGXLogCategories.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"

// ============================================================================
// UPGXProfileValidateCommandlet::Main
// ============================================================================

int32 UPGXProfileValidateCommandlet::Main(const FString& /*Params*/)
{
	UE_LOG(LogPGX, Log, TEXT("========================================"));
	UE_LOG(LogPGX, Log, TEXT("  PGX Profile Validation Commandlet"));
	UE_LOG(LogPGX, Log, TEXT("========================================"));

	// EN: Load the active profile from settings
	// ES: Cargar el profile activo desde settings
	const UPGXProfileSettings* Settings = GetDefault<UPGXProfileSettings>();
	UPGXProjectProfileConfig* Config = nullptr;

	if (Settings && !Settings->ActiveProfile.IsNull())
	{
		Config = Settings->ActiveProfile.LoadSynchronous();
	}

	if (!Config)
	{
		// EN: Try auto-discover single config
		// ES: Intentar auto-descubrir config unico
		IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		AssetRegistry.SearchAllAssets(true);

		FARFilter Filter;
		Filter.ClassPaths.Add(UPGXProjectProfileConfig::StaticClass()->GetClassPathName());
		Filter.bRecursiveClasses = true;

		TArray<FAssetData> AssetDataList;
		AssetRegistry.GetAssets(Filter, AssetDataList);

		if (AssetDataList.Num() == 1)
		{
			Config = Cast<UPGXProjectProfileConfig>(AssetDataList[0].GetAsset());
		}
	}

	if (!Config)
	{
		UE_LOG(LogPGX, Warning, TEXT("No profile config found. Validation skipped."));
		return 0;
	}

	// EN: Resolve the profile (lightweight — no subsystem needed)
	// ES: Resolver el profile (ligero — no se necesita subsistema)
	FPGXResolvedProfile Profile;
	Profile.Identity = Config->Identity;
	Profile.Capabilities = Config->BaseCapabilities;
	Profile.Policies = Config->BasePolicies;
	Profile.Budgets = Config->BaseBudgets;
	Profile.Features = Config->BaseFeatures;
	Profile.State = EPGXProfileState::Resolved;

	// EN: Run validation
	// ES: Ejecutar validacion
	TArray<FPGXBuildValidationResult> Results;
	FPGXProfileBuildValidator::ValidateForBuild(Profile, Results);

	// EN: Log results
	// ES: Loguear resultados
	int32 InfoCount = 0, WarnCount = 0, ErrorCount = 0, BlockerCount = 0;

	for (const auto& R : Results)
	{
		switch (R.Severity)
		{
		case EPGXBuildValidationSeverity::Info:
			++InfoCount;
			UE_LOG(LogPGX, Log, TEXT("[INFO] %s"), *R.Description.ToString());
			break;
		case EPGXBuildValidationSeverity::Warning:
			++WarnCount;
			UE_LOG(LogPGX, Warning, TEXT("[WARN] %s"), *R.Description.ToString());
			break;
		case EPGXBuildValidationSeverity::Error:
			++ErrorCount;
			UE_LOG(LogPGX, Error, TEXT("[ERROR] %s"), *R.Description.ToString());
			break;
		case EPGXBuildValidationSeverity::Blocker:
			++BlockerCount;
			UE_LOG(LogPGX, Error, TEXT("[BLOCKER] %s"), *R.Description.ToString());
			break;
		}
	}

	UE_LOG(LogPGX, Log, TEXT(""));
	UE_LOG(LogPGX, Log, TEXT("Validation complete: %d Info, %d Warnings, %d Errors, %d Blockers"),
		InfoCount, WarnCount, ErrorCount, BlockerCount);

	return BlockerCount > 0 ? 1 : 0;
}

#else // !WITH_EDITOR

// EN: Non-editor stub — commandlets only run in editor context but UCLASS needs definition for linker.
// ES: Stub no-editor — commandlets solo corren en contexto editor pero UCLASS necesita definicion para linker.
int32 UPGXProfileValidateCommandlet::Main(const FString& /*Params*/)
{
	return 0;
}

#endif // WITH_EDITOR
