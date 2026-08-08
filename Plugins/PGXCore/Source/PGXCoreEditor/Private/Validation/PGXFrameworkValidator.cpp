// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Validation/PGXFrameworkValidator.h"

#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "ModuleDescriptor.h"
#include "Notifications/PGXEditorNotification.h"
#include "PluginDescriptor.h"
#include "PluginReferenceDescriptor.h"

#define LOCTEXT_NAMESPACE "PGXValidation"

namespace
{
constexpr const TCHAR* PGXPluginPrefix = TEXT("PGX");
constexpr const TCHAR* PGXCorePluginName = TEXT("PGXCore");
constexpr const TCHAR* PGXCoreRuntimeModuleName = TEXT("PGXCoreRuntime");
constexpr const TCHAR* RuntimeModuleSuffix = TEXT("Runtime");

struct FPGXRuntimePluginInfo
{
	FString PluginName;
	FString BaseDir;
	TArray<FString> RuntimeModules;
};

bool IsPGXPluginName(const FString& PluginName)
{
	return PluginName.StartsWith(PGXPluginPrefix);
}

bool IsPGXCorePluginName(const FString& PluginName)
{
	return PluginName.Equals(PGXCorePluginName, ESearchCase::IgnoreCase);
}

bool IsRuntimeModuleName(const FString& ModuleName)
{
	return ModuleName.EndsWith(RuntimeModuleSuffix, ESearchCase::CaseSensitive);
}

bool IsL2RuntimeModuleName(const FString& ModuleName)
{
	return IsRuntimeModuleName(ModuleName) && !ModuleName.Equals(PGXCoreRuntimeModuleName, ESearchCase::IgnoreCase);
}

FString NormalizeFilename(FString Filename)
{
	FPaths::NormalizeFilename(Filename);
	return Filename;
}

FString ToDisplayPath(const FString& Filename)
{
	FString DisplayPath = Filename;
	FPaths::MakePathRelativeTo(DisplayPath, *FPaths::ProjectDir());
	return DisplayPath;
}

FString GetSeverityName(const EPGXFrameworkValidationSeverity Severity)
{
	switch (Severity)
	{
	case EPGXFrameworkValidationSeverity::Error:
		return TEXT("Error");
	case EPGXFrameworkValidationSeverity::Warning:
		return TEXT("Warning");
	case EPGXFrameworkValidationSeverity::Info:
	default:
		return TEXT("Info");
	}
}

FString GetRuleName(const EPGXFrameworkValidationRule Rule)
{
	switch (Rule)
	{
	case EPGXFrameworkValidationRule::StarTopologyRuntimePluginDependency:
		return TEXT("PGX.Framework.StarTopology.RuntimePluginDependency");
	case EPGXFrameworkValidationRule::StarTopologyRuntimeModuleDependency:
		return TEXT("PGX.Framework.StarTopology.RuntimeModuleDependency");
	case EPGXFrameworkValidationRule::StarTopologyAllowedRuntimeDependency:
		return TEXT("PGX.Framework.StarTopology.AllowedRuntimeDependency");
	case EPGXFrameworkValidationRule::StarTopologyScanUnavailable:
	default:
		return TEXT("PGX.Framework.StarTopology.ScanUnavailable");
	}
}

void AddAllowedEdge(TArray<FPGXAllowedL2Edge>& AllowedEdges, const TCHAR* SourceModule, const TCHAR* TargetModule, const TCHAR* Reason)
{
	FPGXAllowedL2Edge Edge;
	Edge.SourceModule = SourceModule;
	Edge.TargetModule = TargetModule;
	Edge.Reason = Reason;
	AllowedEdges.AddUnique(Edge);
}

bool FindAllowedEdgeReason(const FString& SourceModule, const FString& TargetModule, const TArray<FPGXAllowedL2Edge>& AllowedEdges, FString& OutReason)
{
	for (const FPGXAllowedL2Edge& AllowedEdge : AllowedEdges)
	{
		if (AllowedEdge.SourceModule.Equals(SourceModule, ESearchCase::IgnoreCase)
			&& AllowedEdge.TargetModule.Equals(TargetModule, ESearchCase::IgnoreCase))
		{
			OutReason = AllowedEdge.Reason;
			return true;
		}
	}
	return false;
}

FPGXFrameworkValidationIssue MakeIssue(
	const EPGXFrameworkValidationSeverity Severity,
	const EPGXFrameworkValidationRule Rule,
	const FString& SourcePlugin,
	const FString& SourceModule,
	const FString& TargetPlugin,
	const FString& TargetModule,
	const FString& SourceFile,
	const FString& Reason,
	const FString& Detail)
{
	FPGXFrameworkValidationIssue Issue;
	Issue.Severity = Severity;
	Issue.Rule = Rule;
	Issue.SourcePlugin = SourcePlugin;
	Issue.SourceModule = SourceModule;
	Issue.TargetPlugin = TargetPlugin;
	Issue.TargetModule = TargetModule;
	Issue.SourceFile = SourceFile;
	Issue.Reason = Reason;
	Issue.Message = FString::Printf(
		TEXT("[%s] %s: %s"),
		*GetSeverityName(Severity),
		*GetRuleName(Rule),
		*Detail);
	return Issue;
}

FPGXL2DependencyEdge MakeDependencyEdge(
	const EPGXFrameworkValidationRule Rule,
	const FString& SourcePlugin,
	const FString& SourceModule,
	const FString& TargetPlugin,
	const FString& TargetModule,
	const FString& SourceFile)
{
	FPGXL2DependencyEdge Edge;
	Edge.Rule = Rule;
	Edge.SourcePlugin = SourcePlugin;
	Edge.SourceModule = SourceModule;
	Edge.TargetPlugin = TargetPlugin;
	Edge.TargetModule = TargetModule;
	Edge.SourceFile = SourceFile;
	return Edge;
}

void AppendPluginInfo(const TSharedRef<IPlugin>& Plugin, TMap<FString, FPGXRuntimePluginInfo>& OutL2RuntimePlugins, TMap<FString, FString>& OutRuntimeModuleToPlugin)
{
	const FString PluginName = Plugin->GetName();
	if (!IsPGXPluginName(PluginName) || IsPGXCorePluginName(PluginName))
	{
		return;
	}

	FPGXRuntimePluginInfo Info;
	Info.PluginName = PluginName;
	Info.BaseDir = NormalizeFilename(Plugin->GetBaseDir());

	const FPluginDescriptor& Descriptor = Plugin->GetDescriptor();
	for (const FModuleDescriptor& ModuleDescriptor : Descriptor.Modules)
	{
		const FString ModuleName = ModuleDescriptor.Name.ToString();
		if (IsL2RuntimeModuleName(ModuleName))
		{
			Info.RuntimeModules.Add(ModuleName);
			OutRuntimeModuleToPlugin.Add(ModuleName, PluginName);
		}
	}

	if (!Info.RuntimeModules.IsEmpty())
	{
		OutL2RuntimePlugins.Add(PluginName, MoveTemp(Info));
	}
}

void CollectPluginDescriptorReferences(
	const TArray<TSharedRef<IPlugin>>& Plugins,
	const TMap<FString, FPGXRuntimePluginInfo>& L2RuntimePlugins,
	TArray<FPGXL2DependencyEdge>& OutDetectedEdges)
{
	for (const TSharedRef<IPlugin>& Plugin : Plugins)
	{
		const FString SourcePluginName = Plugin->GetName();
		const FPGXRuntimePluginInfo* SourceInfo = L2RuntimePlugins.Find(SourcePluginName);
		if (SourceInfo == nullptr)
		{
			continue;
		}

		const FPluginDescriptor& Descriptor = Plugin->GetDescriptor();
		for (const FPluginReferenceDescriptor& PluginReference : Descriptor.Plugins)
		{
			if (!PluginReference.bEnabled || PluginReference.Name.Equals(SourcePluginName, ESearchCase::IgnoreCase))
			{
				continue;
			}

			const FPGXRuntimePluginInfo* TargetInfo = L2RuntimePlugins.Find(PluginReference.Name);
			if (TargetInfo == nullptr)
			{
				continue;
			}

			for (const FString& SourceModule : SourceInfo->RuntimeModules)
			{
				for (const FString& TargetModule : TargetInfo->RuntimeModules)
				{
					OutDetectedEdges.Add(MakeDependencyEdge(
						EPGXFrameworkValidationRule::StarTopologyRuntimePluginDependency,
						SourcePluginName,
						SourceModule,
						TargetInfo->PluginName,
						TargetModule,
						ToDisplayPath(Plugin->GetDescriptorFileName())));
				}
			}
		}
	}
}

FString StripCSharpComments(const FString& Input)
{
	FString Output;
	Output.Reserve(Input.Len());

	bool bInLineComment = false;
	bool bInBlockComment = false;
	bool bInString = false;
	bool bEscapeNext = false;

	for (int32 Index = 0; Index < Input.Len(); ++Index)
	{
		const TCHAR Current = Input[Index];
		const TCHAR Next = Index + 1 < Input.Len() ? Input[Index + 1] : TCHAR('\0');

		if (bInLineComment)
		{
			if (Current == TCHAR('\n') || Current == TCHAR('\r'))
			{
				bInLineComment = false;
				Output.AppendChar(Current);
			}
			continue;
		}

		if (bInBlockComment)
		{
			if (Current == TCHAR('*') && Next == TCHAR('/'))
			{
				bInBlockComment = false;
				++Index;
			}
			else if (Current == TCHAR('\n') || Current == TCHAR('\r'))
			{
				Output.AppendChar(Current);
			}
			continue;
		}

		if (bInString)
		{
			Output.AppendChar(Current);
			if (bEscapeNext)
			{
				bEscapeNext = false;
			}
			else if (Current == TCHAR('\\'))
			{
				bEscapeNext = true;
			}
			else if (Current == TCHAR('"'))
			{
				bInString = false;
			}
			continue;
		}

		if (Current == TCHAR('/') && Next == TCHAR('/'))
		{
			bInLineComment = true;
			++Index;
			continue;
		}

		if (Current == TCHAR('/') && Next == TCHAR('*'))
		{
			bInBlockComment = true;
			++Index;
			continue;
		}

		if (Current == TCHAR('"'))
		{
			bInString = true;
		}

		Output.AppendChar(Current);
	}

	return Output;
}

const FPGXRuntimePluginInfo* FindPluginForBuildFile(const FString& BuildFile, const TMap<FString, FPGXRuntimePluginInfo>& L2RuntimePlugins)
{
	const FString NormalizedBuildFile = NormalizeFilename(BuildFile);
	for (const TPair<FString, FPGXRuntimePluginInfo>& Pair : L2RuntimePlugins)
	{
		if (NormalizedBuildFile.StartsWith(Pair.Value.BaseDir, ESearchCase::IgnoreCase))
		{
			return &Pair.Value;
		}
	}
	return nullptr;
}

void CollectRuntimeBuildDependencies(
	const TMap<FString, FPGXRuntimePluginInfo>& L2RuntimePlugins,
	const TMap<FString, FString>& RuntimeModuleToPlugin,
	TArray<FPGXL2DependencyEdge>& OutDetectedEdges,
	TArray<FPGXFrameworkValidationIssue>& OutIssues)
{
	const FString ProjectPluginsDir = NormalizeFilename(FPaths::ProjectPluginsDir());
	if (!FPaths::DirectoryExists(ProjectPluginsDir))
	{
		OutIssues.Add(MakeIssue(
			EPGXFrameworkValidationSeverity::Warning,
			EPGXFrameworkValidationRule::StarTopologyScanUnavailable,
			TEXT(""),
			TEXT(""),
			TEXT(""),
			TEXT(""),
			ToDisplayPath(ProjectPluginsDir),
			TEXT(""),
			TEXT("Project plugins directory was not available; runtime Build.cs dependency scan was skipped.")));
		return;
	}

	TArray<FString> BuildFiles;
	IFileManager::Get().FindFilesRecursive(BuildFiles, *ProjectPluginsDir, TEXT("*Runtime.Build.cs"), true, false, false);

	for (const FString& BuildFile : BuildFiles)
	{
		const FPGXRuntimePluginInfo* SourcePluginInfo = FindPluginForBuildFile(BuildFile, L2RuntimePlugins);
		if (SourcePluginInfo == nullptr)
		{
			continue;
		}

		const FString SourceModule = FPaths::GetBaseFilename(BuildFile).Replace(TEXT(".Build"), TEXT(""));
		if (!SourcePluginInfo->RuntimeModules.Contains(SourceModule))
		{
			continue;
		}

		FString BuildFileText;
		if (!FFileHelper::LoadFileToString(BuildFileText, *BuildFile))
		{
			OutIssues.Add(MakeIssue(
				EPGXFrameworkValidationSeverity::Warning,
				EPGXFrameworkValidationRule::StarTopologyScanUnavailable,
				SourcePluginInfo->PluginName,
				SourceModule,
				TEXT(""),
				TEXT(""),
				ToDisplayPath(BuildFile),
				TEXT(""),
				TEXT("Unable to read runtime Build.cs file; dependency scan skipped for this module.")));
			continue;
		}

		const FString SanitizedBuildFileText = StripCSharpComments(BuildFileText);
		for (const TPair<FString, FString>& ModuleToPlugin : RuntimeModuleToPlugin)
		{
			const FString& TargetModule = ModuleToPlugin.Key;
			const FString& TargetPlugin = ModuleToPlugin.Value;
			if (TargetModule.Equals(SourceModule, ESearchCase::IgnoreCase)
				|| TargetPlugin.Equals(SourcePluginInfo->PluginName, ESearchCase::IgnoreCase))
			{
				continue;
			}

			const FString QuotedTargetModule = FString::Printf(TEXT("\"%s\""), *TargetModule);
			if (!SanitizedBuildFileText.Contains(QuotedTargetModule, ESearchCase::CaseSensitive))
			{
				continue;
			}

			OutDetectedEdges.Add(MakeDependencyEdge(
				EPGXFrameworkValidationRule::StarTopologyRuntimeModuleDependency,
				SourcePluginInfo->PluginName,
				SourceModule,
				TargetPlugin,
				TargetModule,
				ToDisplayPath(BuildFile)));
		}
	}
}

bool ContainsError(const TArray<FPGXFrameworkValidationIssue>& Issues)
{
	for (const FPGXFrameworkValidationIssue& Issue : Issues)
	{
		if (Issue.Severity == EPGXFrameworkValidationSeverity::Error)
		{
			return true;
		}
	}
	return false;
}
} // namespace

UPGXValidationSettings::UPGXValidationSettings()
{
	UPGXFrameworkValidator::GetDefaultAllowedL2Edges(AllowedL2Edges);
}

bool UPGXFrameworkValidator::ValidateFramework(TArray<FText>& OutErrors, TArray<FText>& OutWarnings)
{
	TArray<FPGXFrameworkValidationIssue> Issues;
	const bool bValid = ValidateStarTopology(Issues);

	for (const FPGXFrameworkValidationIssue& Issue : Issues)
	{
		if (Issue.Severity == EPGXFrameworkValidationSeverity::Error)
		{
			OutErrors.Add(FText::FromString(Issue.Message));
		}
		else if (Issue.Severity == EPGXFrameworkValidationSeverity::Warning)
		{
			OutWarnings.Add(FText::FromString(Issue.Message));
		}
	}

	if (bValid)
	{
		UPGXEditorNotification::ShowInfo(LOCTEXT("ValidationPassed", "PGX: Framework validation passed."));
	}
	else
	{
		UPGXEditorNotification::ShowWarning(LOCTEXT("ValidationFailed", "PGX: Framework validation found star-topology issues."));
	}

	return bValid;
}

bool UPGXFrameworkValidator::ValidateStarTopology(TArray<FPGXFrameworkValidationIssue>& OutIssues)
{
	OutIssues.Reset();

	const TArray<TSharedRef<IPlugin>> Plugins = IPluginManager::Get().GetDiscoveredPlugins();
	TMap<FString, FPGXRuntimePluginInfo> L2RuntimePlugins;
	TMap<FString, FString> RuntimeModuleToPlugin;

	for (const TSharedRef<IPlugin>& Plugin : Plugins)
	{
		AppendPluginInfo(Plugin, L2RuntimePlugins, RuntimeModuleToPlugin);
	}

	TArray<FPGXL2DependencyEdge> DetectedEdges;
	CollectPluginDescriptorReferences(Plugins, L2RuntimePlugins, DetectedEdges);
	CollectRuntimeBuildDependencies(L2RuntimePlugins, RuntimeModuleToPlugin, DetectedEdges, OutIssues);

	TArray<FPGXAllowedL2Edge> AllowedEdges;
	GetEffectiveAllowedL2Edges(AllowedEdges);
	ValidateStarTopologyEdges(DetectedEdges, AllowedEdges, OutIssues);

	return !ContainsError(OutIssues);
}

void UPGXFrameworkValidator::GetDefaultAllowedL2Edges(TArray<FPGXAllowedL2Edge>& OutAllowedEdges)
{
	OutAllowedEdges.Reset();
}

void UPGXFrameworkValidator::GetEffectiveAllowedL2Edges(TArray<FPGXAllowedL2Edge>& OutAllowedEdges)
{
	OutAllowedEdges.Reset();

	const UPGXValidationSettings* Settings = GetDefault<UPGXValidationSettings>();
	if (!Settings)
	{
		GetDefaultAllowedL2Edges(OutAllowedEdges);
		return;
	}

	for (const FPGXAllowedL2Edge& ConfiguredEdge : Settings->AllowedL2Edges)
	{
		if (!ConfiguredEdge.SourceModule.IsEmpty() && !ConfiguredEdge.TargetModule.IsEmpty())
		{
			OutAllowedEdges.AddUnique(ConfiguredEdge);
		}
	}
}

bool UPGXFrameworkValidator::ValidateStarTopologyEdges(const TArray<FPGXL2DependencyEdge>& DetectedEdges,
	const TArray<FPGXAllowedL2Edge>& AllowedEdges,
	TArray<FPGXFrameworkValidationIssue>& OutIssues)
{
	for (const FPGXL2DependencyEdge& Edge : DetectedEdges)
	{
		FString AllowedReason;
		if (FindAllowedEdgeReason(Edge.SourceModule, Edge.TargetModule, AllowedEdges, AllowedReason))
		{
			OutIssues.Add(MakeIssue(
				EPGXFrameworkValidationSeverity::Info,
				EPGXFrameworkValidationRule::StarTopologyAllowedRuntimeDependency,
				Edge.SourcePlugin,
				Edge.SourceModule,
				Edge.TargetPlugin,
				Edge.TargetModule,
				Edge.SourceFile,
				AllowedReason,
				FString::Printf(
					TEXT("Allowed documented L2 runtime edge '%s' -> '%s'. Reason: %s"),
					*Edge.SourceModule,
					*Edge.TargetModule,
					*AllowedReason)));
			continue;
		}

		const bool bPluginDependency = Edge.Rule == EPGXFrameworkValidationRule::StarTopologyRuntimePluginDependency;
		OutIssues.Add(MakeIssue(
			EPGXFrameworkValidationSeverity::Error,
			Edge.Rule,
			Edge.SourcePlugin,
			Edge.SourceModule,
			Edge.TargetPlugin,
			Edge.TargetModule,
			Edge.SourceFile,
			TEXT(""),
			bPluginDependency
				? FString::Printf(
					TEXT("Plugin '%s' declares a direct dependency from L2 runtime module '%s' to L2 runtime module '%s'. Route runtime cross-plugin communication through PGXCoreRuntime Message instead."),
					*Edge.SourcePlugin,
					*Edge.SourceModule,
					*Edge.TargetModule)
				: FString::Printf(
					TEXT("Runtime module '%s' depends directly on L2 runtime module '%s'. Route cross-plugin communication through PGXCoreRuntime Message instead."),
					*Edge.SourceModule,
					*Edge.TargetModule)));
	}

	return !ContainsError(OutIssues);
}

#undef LOCTEXT_NAMESPACE
