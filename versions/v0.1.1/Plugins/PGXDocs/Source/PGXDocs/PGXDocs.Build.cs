// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

using UnrealBuildTool;
using System.IO;

public class PGXDocs : ModuleRules
{
	public PGXDocs(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// EN: md4c is pure C — compile .c files from ThirdParty (private to this module)
		// ES: md4c es C puro — compilar archivos .c desde ThirdParty (privado a este modulo)
		string Md4cPath = Path.Combine(ModuleDirectory, "Private", "ThirdParty", "md4c");
		PrivateIncludePaths.Add(Md4cPath);

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"DeveloperSettings",
			"Json",
			"JsonUtilities",
			"PGXCoreRuntime"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});

		// EN: Conditional editor dependency for FSourceCodeNavigation (used in PGXDocLinkResolver)
		// ES: Dependencia condicional de editor para FSourceCodeNavigation (usado en PGXDocLinkResolver)
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("UnrealEd");
		}

		// EN: Allow compilation of .c files (md4c)
		// ES: Permitir compilacion de archivos .c (md4c)
		CppCompileWarningSettings.UndefinedIdentifierWarningLevel = WarningLevel.Off;
	}
}
