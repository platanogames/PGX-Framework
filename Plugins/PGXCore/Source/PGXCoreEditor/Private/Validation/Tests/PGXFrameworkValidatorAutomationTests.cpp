// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Validation/PGXFrameworkValidator.h"

namespace PGXFrameworkValidatorAutomation
{
	FPGXL2DependencyEdge MakeEdge(EPGXFrameworkValidationRule Rule, const TCHAR* SourceModule, const TCHAR* TargetModule)
	{
		FPGXL2DependencyEdge Edge;
		Edge.Rule = Rule;
		Edge.SourcePlugin = FString(SourceModule).Replace(TEXT("Runtime"), TEXT(""));
		Edge.SourceModule = SourceModule;
		Edge.TargetPlugin = FString(TargetModule).Replace(TEXT("Runtime"), TEXT(""));
		Edge.TargetModule = TargetModule;
		Edge.SourceFile = FString::Printf(TEXT("Synthetic/%s.Build.cs"), SourceModule);
		return Edge;
	}

	bool HasIssue(const TArray<FPGXFrameworkValidationIssue>& Issues,
		EPGXFrameworkValidationSeverity Severity,
		EPGXFrameworkValidationRule Rule,
		const TCHAR* SourceModule,
		const TCHAR* TargetModule)
	{
		for (const FPGXFrameworkValidationIssue& Issue : Issues)
		{
			if (Issue.Severity == Severity
				&& Issue.Rule == Rule
				&& Issue.SourceModule.Equals(SourceModule, ESearchCase::IgnoreCase)
				&& Issue.TargetModule.Equals(TargetModule, ESearchCase::IgnoreCase))
			{
				return true;
			}
		}
		return false;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXFrameworkValidator_AllowlistedEdgeAutomationTest,
	"PGX.CoreEditor.Validation.StarTopology.AllowlistedEdge",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXFrameworkValidator_AllowlistedEdgeAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FPGXAllowedL2Edge> AllowedEdges;
	FPGXAllowedL2Edge LoadingToGameFlow;
	LoadingToGameFlow.SourceModule = TEXT("PGXLoadingRuntime");
	LoadingToGameFlow.TargetModule = TEXT("PGXGameFlowRuntime");
	LoadingToGameFlow.Reason = TEXT("Synthetic test allowlist edge");
	AllowedEdges.Add(LoadingToGameFlow);

	TArray<FPGXL2DependencyEdge> Edges;
	Edges.Add(PGXFrameworkValidatorAutomation::MakeEdge(
		EPGXFrameworkValidationRule::StarTopologyRuntimeModuleDependency,
		TEXT("PGXLoadingRuntime"),
		TEXT("PGXGameFlowRuntime")));

	TArray<FPGXFrameworkValidationIssue> Issues;
	TestTrue(TEXT("Synthetic allowlist suppresses L2 edge as non-failing"),
		UPGXFrameworkValidator::ValidateStarTopologyEdges(Edges, AllowedEdges, Issues));
	TestTrue(TEXT("Allowed edge is downgraded to info issue"),
		PGXFrameworkValidatorAutomation::HasIssue(Issues,
			EPGXFrameworkValidationSeverity::Info,
			EPGXFrameworkValidationRule::StarTopologyAllowedRuntimeDependency,
			TEXT("PGXLoadingRuntime"),
			TEXT("PGXGameFlowRuntime")));
	TestFalse(TEXT("Allowed edge does not emit module-dependency error"),
		PGXFrameworkValidatorAutomation::HasIssue(Issues,
			EPGXFrameworkValidationSeverity::Error,
			EPGXFrameworkValidationRule::StarTopologyRuntimeModuleDependency,
			TEXT("PGXLoadingRuntime"),
			TEXT("PGXGameFlowRuntime")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXFrameworkValidator_NonAllowlistedBuildCsAutomationTest,
	"PGX.CoreEditor.Validation.StarTopology.NonAllowlistedBuildCs",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXFrameworkValidator_NonAllowlistedBuildCsAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FPGXAllowedL2Edge> AllowedEdges;
	UPGXFrameworkValidator::GetDefaultAllowedL2Edges(AllowedEdges);

	TArray<FPGXL2DependencyEdge> Edges;
	Edges.Add(PGXFrameworkValidatorAutomation::MakeEdge(
		EPGXFrameworkValidationRule::StarTopologyRuntimeModuleDependency,
		TEXT("PGXAudioRuntime"),
		TEXT("PGXGameFlowRuntime")));

	TArray<FPGXFrameworkValidationIssue> Issues;
	TestFalse(TEXT("Non-allowlisted Build.cs dependency fails star topology validation"),
		UPGXFrameworkValidator::ValidateStarTopologyEdges(Edges, AllowedEdges, Issues));
	TestTrue(TEXT("Non-allowlisted Build.cs dependency emits P0 error"),
		PGXFrameworkValidatorAutomation::HasIssue(Issues,
			EPGXFrameworkValidationSeverity::Error,
			EPGXFrameworkValidationRule::StarTopologyRuntimeModuleDependency,
			TEXT("PGXAudioRuntime"),
			TEXT("PGXGameFlowRuntime")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXFrameworkValidator_DescriptorDependencyAutomationTest,
	"PGX.CoreEditor.Validation.StarTopology.DescriptorDependency",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXFrameworkValidator_DescriptorDependencyAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FPGXL2DependencyEdge> Edges;
	Edges.Add(PGXFrameworkValidatorAutomation::MakeEdge(
		EPGXFrameworkValidationRule::StarTopologyRuntimePluginDependency,
		TEXT("PGXAudioRuntime"),
		TEXT("PGXSaveRuntime")));

	TArray<FPGXFrameworkValidationIssue> Issues;
	TestFalse(TEXT("Non-allowlisted plugin descriptor dependency fails star topology validation"),
		UPGXFrameworkValidator::ValidateStarTopologyEdges(Edges, TArray<FPGXAllowedL2Edge>(), Issues));
	TestTrue(TEXT("Plugin descriptor dependency emits P0 error"),
		PGXFrameworkValidatorAutomation::HasIssue(Issues,
			EPGXFrameworkValidationSeverity::Error,
			EPGXFrameworkValidationRule::StarTopologyRuntimePluginDependency,
			TEXT("PGXAudioRuntime"),
			TEXT("PGXSaveRuntime")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXFrameworkValidator_CleanGraphAutomationTest,
	"PGX.CoreEditor.Validation.StarTopology.CleanGraph",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXFrameworkValidator_CleanGraphAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FPGXFrameworkValidationIssue> Issues;
	TestTrue(TEXT("Empty/clean graph passes"),
		UPGXFrameworkValidator::ValidateStarTopologyEdges(TArray<FPGXL2DependencyEdge>(), TArray<FPGXAllowedL2Edge>(), Issues));
	TestEqual(TEXT("Clean graph has no issues"), Issues.Num(), 0);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
