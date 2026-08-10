// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/FileHelper.h"

//
/**
 * [ES] Validador editor-only para impedir que un plugin use módulos de otro
 *      plugin en Build.cs sin declarar la dependencia en .uplugin.
 * [EN] Editor-only validator that prevents using another plugin's modules in
 *      Build.cs without declaring the plugin dependency in .uplugin.
 *
 * Depende de / Depends on: FFileHelper text reads.
 * Usado por / Used by: PGXSimHarness validation tests before cross-plugin harness work.
 */
struct FPGXPluginDescriptorDependencyValidator
{
	struct FDependencyCheck
	{
		FString PluginName;
		FString ModuleName;
	};

	static bool ValidateBuildModulesDeclaredInDescriptor(
		const FString& BuildCsPath,
		const FString& UPluginPath,
		const TArray<FDependencyCheck>& Checks,
		TArray<FString>& OutIssues)
	{
		FString BuildText;
		FString DescriptorText;
		if (!FFileHelper::LoadFileToString(BuildText, *BuildCsPath))
		{
			OutIssues.Add(FString::Printf(TEXT("[FAIL] Build.cs unreadable: %s"), *BuildCsPath));
			return false;
		}
		if (!FFileHelper::LoadFileToString(DescriptorText, *UPluginPath))
		{
			OutIssues.Add(FString::Printf(TEXT("[FAIL] .uplugin unreadable: %s"), *UPluginPath));
			return false;
		}

		bool bPassed = true;
		for (const FDependencyCheck& Check : Checks)
		{
			const bool bUsesModule = BuildText.Contains(FString::Printf(TEXT("\"%s\""), *Check.ModuleName));
			const bool bDeclaresPlugin = DescriptorText.Contains(FString::Printf(TEXT("\"Name\": \"%s\""), *Check.PluginName));
			if (bUsesModule && !bDeclaresPlugin)
			{
				OutIssues.Add(FString::Printf(TEXT("[FAIL] Module %s used in Build.cs but plugin %s is missing from .uplugin."), *Check.ModuleName, *Check.PluginName));
				bPassed = false;
			}
		}

		if (bPassed)
		{
			OutIssues.Add(TEXT("[PASS] Build.cs cross-plugin modules are declared in .uplugin."));
		}
		return bPassed;
	}
};
