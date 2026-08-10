// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Core/PGXBuildPlanGenerator.h"
#include "Core/PGXHierarchyResolver.h"
#include "Core/PGXProjectAnalyzer.h"
#include "Core/PGXScaffoldTemplateRegistry.h"
#include "Core/PGXScaffoldTypes.h"
#include "Core/PGXScaffoldValidator.h"

#include "Misc/Paths.h"

namespace
{
FPGXScaffoldTemplateItem PGXScaffoldMakeItem(
	const FName ItemId,
	const EPGXScaffoldActionType ActionType,
	const FString& RelativePath,
	const FName ParentItemId = NAME_None,
	const int32 ExecutionOrder = 0)
{
	FPGXScaffoldTemplateItem Item;
	Item.ItemId = ItemId;
	Item.DisplayName = FText::FromName(ItemId);
	Item.ActionType = ActionType;
	Item.RelativePath = RelativePath;
	Item.ParentItemId = ParentItemId;
	Item.ExecutionOrder = ExecutionOrder;
	return Item;
}

FPGXScaffoldTemplate PGXScaffoldMakeTemplate(const FName TemplateId, const FName Category)
{
	FPGXScaffoldTemplate Template;
	Template.TemplateId = TemplateId;
	Template.DisplayName = FText::FromName(TemplateId);
	Template.Description = FText::FromString(TEXT("Automation fixture"));
	Template.Category = Category;
	return Template;
}

FPGXScaffoldProjectInfo PGXScaffoldMakeProjectInfo()
{
	FPGXScaffoldProjectInfo Info;
	Info.ProjectType = EPGXProjectType::CppProject;
	Info.ProjectName = TEXT("PGXAutomationProject");
	Info.ContentDir = FPaths::ProjectContentDir();
	Info.Modules.Add(TEXT("PGXAutomationModule"));
	Info.Plugins.Add(TEXT("PGXScaffold"));
	Info.ExistingAssets.Add(TEXT("ExistingAsset"));
	return Info;
}

bool PGXScaffoldHasSeverity(const TArray<FPGXScaffoldValidationResult>& Results, const EPGXScaffoldSeverity Severity)
{
	return Results.ContainsByPredicate([Severity](const FPGXScaffoldValidationResult& Result)
	{
		return Result.Severity == Severity;
	});
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXScaffold_ProjectAnalyzerSourceTruthAutomationTest,
	"PGX.Scaffold.Behavior.ProjectAnalyzerSourceTruth",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXScaffold_ProjectAnalyzerSourceTruthAutomationTest::RunTest(const FString& /*Parameters*/)
{
	FPGXProjectAnalyzer Analyzer;
	const FPGXScaffoldProjectInfo First = Analyzer.Analyze();
	const FPGXScaffoldProjectInfo Cached = Analyzer.GetCachedInfo();

	TestFalse(TEXT("Analyzer project name is visible"), First.ProjectName.IsEmpty());
	TestTrue(TEXT("Analyzer content dir is visible"), FPaths::IsRelative(First.ContentDir) == false);
	TestEqual(TEXT("Cached project name matches first analysis"), Cached.ProjectName, First.ProjectName);

	Analyzer.InvalidateCache();
	const FPGXScaffoldProjectInfo Second = Analyzer.Analyze();
	TestEqual(TEXT("Re-analysis keeps project identity stable"), Second.ProjectName, First.ProjectName);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXScaffold_TemplateRegistryDuplicateAndCategoryAutomationTest,
	"PGX.Scaffold.Behavior.TemplateRegistryDuplicateAndCategory",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXScaffold_TemplateRegistryDuplicateAndCategoryAutomationTest::RunTest(const FString& /*Parameters*/)
{
	FPGXScaffoldTemplateRegistry Registry;
	FPGXScaffoldTemplate Alpha = PGXScaffoldMakeTemplate(TEXT("pgx.test.alpha"), TEXT("Automation"));
	Alpha.Items.Add(PGXScaffoldMakeItem(TEXT("root"), EPGXScaffoldActionType::CreateFolder, TEXT("Root")));
	FPGXScaffoldTemplate Duplicate = Alpha;
	Duplicate.DisplayName = FText::FromString(TEXT("Duplicate should be ignored"));

	Registry.RegisterTemplate(Alpha);
	Registry.RegisterTemplate(Duplicate);
	Registry.RegisterTemplate(PGXScaffoldMakeTemplate(TEXT("pgx.test.beta"), TEXT("Other")));

	TestEqual(TEXT("Duplicate template id is ignored"), Registry.GetAllTemplates().Num(), 2);
	TestNotNull(TEXT("Registered template can be found"), Registry.FindTemplate(TEXT("pgx.test.alpha")));
	TestNull(TEXT("Unknown template is absent"), Registry.FindTemplate(TEXT("pgx.test.missing")));

	const TArray<FName> Categories = Registry.GetCategories();
	TestTrue(TEXT("Automation category present"), Categories.Contains(TEXT("Automation")));
	TestEqual(TEXT("Automation category has one template"), Registry.GetTemplatesByCategory(TEXT("Automation")).Num(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXScaffold_HierarchyResolverPropagationAutomationTest,
	"PGX.Scaffold.Behavior.HierarchyResolverPropagation",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXScaffold_HierarchyResolverPropagationAutomationTest::RunTest(const FString& /*Parameters*/)
{
	FPGXScaffoldTemplate Template = PGXScaffoldMakeTemplate(TEXT("pgx.test.hierarchy"), TEXT("Automation"));
	Template.Items.Add(PGXScaffoldMakeItem(TEXT("root"), EPGXScaffoldActionType::CreateFolder, TEXT("Root"), NAME_None, 0));
	Template.Items.Add(PGXScaffoldMakeItem(TEXT("child_b"), EPGXScaffoldActionType::CreateFolder, TEXT("Root/B"), TEXT("root"), 2));
	Template.Items.Add(PGXScaffoldMakeItem(TEXT("child_a"), EPGXScaffoldActionType::CreateFolder, TEXT("Root/A"), TEXT("root"), 1));
	Template.Items.Add(PGXScaffoldMakeItem(TEXT("orphan"), EPGXScaffoldActionType::CreateFolder, TEXT("Orphan"), TEXT("missing_parent"), 3));

	FPGXHierarchyResolver Resolver;
	TArray<TSharedPtr<FPGXScaffoldTreeNode>> Roots = Resolver.BuildTree(Template);
	TestEqual(TEXT("Root plus orphan become top-level nodes"), Roots.Num(), 2);
	TestEqual(TEXT("Primary root first by order"), Roots[0]->Item.ItemId, FName(TEXT("root")));
	TestEqual(TEXT("Children sorted by execution order"), Roots[0]->Children[0]->Item.ItemId, FName(TEXT("child_a")));

	FPGXHierarchyResolver::ToggleNode(Roots[0], false);
	TestFalse(TEXT("Unchecking parent unchecks child A"), Roots[0]->Children[0]->bChecked);
	TestFalse(TEXT("Unchecking parent unchecks child B"), Roots[0]->Children[1]->bChecked);

	FPGXHierarchyResolver::ToggleNode(Roots[0]->Children[0], true);
	TestTrue(TEXT("Checking child re-checks ancestor"), Roots[0]->bChecked);
	TestTrue(TEXT("Checked child is selected"), Roots[0]->Children[0]->bChecked);
	TestFalse(TEXT("Unchecked sibling remains unchecked"), Roots[0]->Children[1]->bChecked);

	const TArray<FPGXScaffoldTemplateItem> Selected = Resolver.GetSelectedItems(Roots);
	TestTrue(TEXT("Selected items include root"), Selected.ContainsByPredicate([](const FPGXScaffoldTemplateItem& Item) { return Item.ItemId == FName(TEXT("root")); }));
	TestTrue(TEXT("Selected items include checked child"), Selected.ContainsByPredicate([](const FPGXScaffoldTemplateItem& Item) { return Item.ItemId == FName(TEXT("child_a")); }));
	TestFalse(TEXT("Selected items exclude unchecked sibling"), Selected.ContainsByPredicate([](const FPGXScaffoldTemplateItem& Item) { return Item.ItemId == FName(TEXT("child_b")); }));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXScaffold_ValidatorPureRulesAutomationTest,
	"PGX.Scaffold.Behavior.ValidatorPureRules",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXScaffold_ValidatorPureRulesAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FPGXScaffoldTemplateItem> Items;
	Items.Add(PGXScaffoldMakeItem(TEXT("root"), EPGXScaffoldActionType::CreateFolder, TEXT("{ProjectName}/Root")));
	Items.Add(PGXScaffoldMakeItem(TEXT("dup_a"), EPGXScaffoldActionType::CreateFolder, TEXT("{ProjectName}/Duplicate")));
	Items.Add(PGXScaffoldMakeItem(TEXT("dup_b"), EPGXScaffoldActionType::CreateFolder, TEXT("{ProjectName}/Duplicate")));
	FPGXScaffoldTemplateItem MissingDep = PGXScaffoldMakeItem(TEXT("asset"), EPGXScaffoldActionType::CreateDataAsset, TEXT("{ProjectName}/Data/ExistingAsset"));
	MissingDep.DependsOn.Add(TEXT("missing_dependency"));
	Items.Add(MissingDep);

	TMap<FString, FString> Variables;
	Variables.Add(TEXT("ProjectName"), TEXT(""));

	FPGXScaffoldValidator Validator;
	const TArray<FPGXScaffoldValidationResult> Results = Validator.Validate(Items, Variables, PGXScaffoldMakeProjectInfo());

	TestTrue(TEXT("Validator reports blocking errors"), Validator.HasErrors());
	TestTrue(TEXT("Validator emitted at least one error"), PGXScaffoldHasSeverity(Results, EPGXScaffoldSeverity::Error));
	TestTrue(TEXT("Duplicate path finding present"), Results.ContainsByPredicate([](const FPGXScaffoldValidationResult& Result)
	{
		return Result.Message.ToString().Contains(TEXT("Duplicate path"));
	}));
	TestTrue(TEXT("Missing variable finding present"), Results.ContainsByPredicate([](const FPGXScaffoldValidationResult& Result)
	{
		return Result.Message.ToString().Contains(TEXT("ProjectName"));
	}));
	TestTrue(TEXT("Missing dependency finding present"), Results.ContainsByPredicate([](const FPGXScaffoldValidationResult& Result)
	{
		return Result.Message.ToString().Contains(TEXT("depends on"));
	}));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXScaffold_BuildPlanOrderingAutomationTest,
	"PGX.Scaffold.Behavior.BuildPlanOrdering",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXScaffold_BuildPlanOrderingAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FPGXScaffoldTemplateItem> Items;
	FPGXScaffoldTemplateItem Blueprint = PGXScaffoldMakeItem(TEXT("bp"), EPGXScaffoldActionType::CreateBlueprint, TEXT("{ProjectName}/Blueprints/BP_Test"), NAME_None, 0);
	Blueprint.ParentClassPath = TEXT("/Script/Engine.Actor");
	Items.Add(Blueprint);
	Items.Add(PGXScaffoldMakeItem(TEXT("folder"), EPGXScaffoldActionType::CreateFolder, TEXT("{ProjectName}/Blueprints"), NAME_None, 10));
	FPGXScaffoldTemplateItem DataAsset = PGXScaffoldMakeItem(TEXT("da"), EPGXScaffoldActionType::CreateDataAsset, TEXT("{ProjectName}/Data/DA_Test"), NAME_None, 5);
	DataAsset.AssetClassName = TEXT("UPrimaryDataAsset");
	Items.Add(DataAsset);

	TMap<FString, FString> Variables;
	Variables.Add(TEXT("ProjectName"), TEXT("ProjectAlpha"));

	FPGXBuildPlanGenerator Generator;
	const FString ContentDir = FPaths::Combine(FPaths::ProjectDir(), TEXT("Content"));
	const FPGXScaffoldBuildPlan Plan = Generator.Generate(Items, Variables, ContentDir, TEXT("pgx.test.plan"));

	TestEqual(TEXT("Plan has three steps"), Plan.Steps.Num(), 3);
	TestEqual(TEXT("Folders are emitted first"), static_cast<int32>(Plan.Steps[0].ActionType), static_cast<int32>(EPGXScaffoldActionType::CreateFolder));
	TestEqual(TEXT("DataAssets are emitted second"), static_cast<int32>(Plan.Steps[1].ActionType), static_cast<int32>(EPGXScaffoldActionType::CreateDataAsset));
	TestEqual(TEXT("Blueprints are emitted third"), static_cast<int32>(Plan.Steps[2].ActionType), static_cast<int32>(EPGXScaffoldActionType::CreateBlueprint));
	TestTrue(TEXT("Variables are substituted in absolute paths"), Plan.Steps[0].AbsolutePath.Contains(TEXT("ProjectAlpha")));
	TestEqual(TEXT("CountByType sees one Blueprint"), Plan.CountByType(EPGXScaffoldActionType::CreateBlueprint), 1);
	TestTrue(TEXT("Plan JSON includes TemplateId"), Plan.ToJsonString().Contains(TEXT("pgx.test.plan")));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
