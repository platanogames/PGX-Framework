// Copyright PGX Framework. All Rights Reserved.

using UnrealBuildTool;

public class PGXSimHarnessEditor : ModuleRules
{
	public PGXSimHarnessEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			"UnrealEd",
			"ToolMenus",
			"GameplayTags",
			"Projects",                  //  coverage — IPluginManager for FPGXHarnessCoverage
			"PGXCoreRuntime",
			"PGXCoreEditor"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			// EN: core runtime (covered runtime modules)
			// ES: Runtime core (modulos runtime cubiertos)
			"PGXGameFlowRuntime",
			"PGXSaveRuntime",
			"PGXPSORuntime",
			"PGXLoadingRuntime",
			"PGXMGOSRuntime",
			"PGXAudioRuntime",
			"PGXDocs",
			"PGXInputRuntime",     //  coverage — Input harness integration

			// EN: Engine modules for visual harness (UBehaviorTree, UUserWidget, AAIController)
			// ES: Modulos engine para visual harness (UBehaviorTree, UUserWidget, AAIController)
			"AIModule",
			"UMG",

			//  coverage and manifest runtime dependencies
			// modules needed by FPGXHarnessCoverage::VerifyPluginPresence so
			// LoadClass<USubsystem>(...) can resolve the subsystem UClass
			// during presence checks. 22 modules added in one shot.
			//
			// Implemented (high-priority)
			"PGXAIRuntime",
			"PGXAbilityRuntime",
			"PGXSpawnRuntime",
			"PGXUIRuntime",
			// Partial surface (medium-priority)
			"PGXCameraRuntime",
			"PGXInteractionRuntime",
			"PGXInventoryRuntime",
			// Manifest-missing with runtime subsystem
			"PGXColonyRuntime",
			"PGXCraftingRuntime",
			"PGXEnvironmentRuntime",
			"PGXTradeRuntime",
			"PGXVehiclesRuntime",
			//  Tools-only
			//   and editor-only modules intentionally OMITTED. PGXEditorTools,
			//   PGXScaffold, PGXTutorials, PGXVersionControl have no runtime module
			//   (their .uplugin declares only Editor-typed module name) and no
			//   subsystem class; presence check maps them to NotApplicable per
			//   FPGXHarnessCoverage.cpp catalog. Module names to use IF a runtime
			//   equivalent is ever added: PGXEditorToolsEditor, PGXScaffoldEditor,
			//   PGXTutorialsEditor, PGXVersionControlEditor.

			// EN: Editor internals — only used by .cpp files
			// ES: Internos de editor — solo usados por archivos .cpp
			"InputCore",
			"EditorStyle",
			"ContentBrowser",
			"WorkspaceMenuStructure"
		});
	}
}
