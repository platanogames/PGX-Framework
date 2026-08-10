// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Workspace/PGXWorkspaceMenu.h"
#include "Style/PGXEditorStyle.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

#define LOCTEXT_NAMESPACE "PGXWorkspaceMenu"

// EN: Static workspace group pointers / ES: Punteros estaticos de grupos workspace
static TSharedPtr<FWorkspaceItem> GPGXRoot;
static TSharedPtr<FWorkspaceItem> GPGXInspectors;
static TSharedPtr<FWorkspaceItem> GPGXDashboards;
static TSharedPtr<FWorkspaceItem> GPGXTools;
static TSharedPtr<FWorkspaceItem> GPGXSystemPanels;

void FPGXWorkspaceMenu::Initialize()
{
	if (GPGXRoot.IsValid())
	{
		return;
	}

	// EN: Create root "PGX Framework" group under Window menu
	// ES: Crear grupo raiz "PGX Framework" bajo el menu Window
	GPGXRoot = WorkspaceMenu::GetMenuStructure().GetStructureRoot()->AddGroup(
		LOCTEXT("PGXRoot", "PGX Framework"),
		LOCTEXT("PGXRootTooltip", "PGX Framework editor panels and tools"),
		FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.Hub"),
		true // bInFront — appear near top of Window menu
	);

	// EN: Create sub-groups / ES: Crear sub-grupos
	GPGXInspectors = GPGXRoot->AddGroup(
		LOCTEXT("PGXInspectors", "System Inspectors"),
		LOCTEXT("PGXInspectorsTooltip", "Runtime system inspectors and debuggers"),
		FSlateIcon(),
		true
	);

	GPGXDashboards = GPGXRoot->AddGroup(
		LOCTEXT("PGXDashboards", "Dashboards"),
		LOCTEXT("PGXDashboardsTooltip", "Data dashboards and validation tools"),
		FSlateIcon(),
		false
	);

	GPGXTools = GPGXRoot->AddGroup(
		LOCTEXT("PGXTools", "Developer Tools"),
		LOCTEXT("PGXToolsTooltip", "Documentation, testing, and development utilities"),
		FSlateIcon(),
		false
	);

	GPGXSystemPanels = GPGXRoot->AddGroup(
		LOCTEXT("PGXSystemPanels", "System Panels"),
		LOCTEXT("PGXSystemPanelsTooltip", "Feature-specific editor panels for content authoring and configuration"),
		FSlateIcon(),
		false
	);
}

TSharedRef<FWorkspaceItem> FPGXWorkspaceMenu::GetRoot()
{
	Initialize();
	return GPGXRoot.ToSharedRef();
}

TSharedRef<FWorkspaceItem> FPGXWorkspaceMenu::GetInspectorsGroup()
{
	Initialize();
	return GPGXInspectors.ToSharedRef();
}

TSharedRef<FWorkspaceItem> FPGXWorkspaceMenu::GetDashboardsGroup()
{
	Initialize();
	return GPGXDashboards.ToSharedRef();
}

TSharedRef<FWorkspaceItem> FPGXWorkspaceMenu::GetToolsGroup()
{
	Initialize();
	return GPGXTools.ToSharedRef();
}

TSharedRef<FWorkspaceItem> FPGXWorkspaceMenu::GetSystemPanelsGroup()
{
	Initialize();
	return GPGXSystemPanels.ToSharedRef();
}

#undef LOCTEXT_NAMESPACE
