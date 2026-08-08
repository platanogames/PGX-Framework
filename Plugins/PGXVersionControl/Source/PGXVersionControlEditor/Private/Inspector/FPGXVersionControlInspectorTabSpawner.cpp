// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Inspector/FPGXVersionControlInspectorTabSpawner.h"
#include "Inspector/SPGXVersionControlInspectorTab.h"
#include "Widgets/Docking/SDockTab.h"
#include "Workspace/PGXWorkspaceMenu.h"
#include "Style/PGXEditorStyle.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXVersionControlInspector"

const FName FPGXVersionControlInspectorTabSpawner::TabId(TEXT("PGXVersionControlInspector"));

void FPGXVersionControlInspectorTabSpawner::Register()
{
	PGX::Editor::RegisterNomadTab(
		TabId,
		FOnSpawnTab::CreateStatic(&FPGXVersionControlInspectorTabSpawner::SpawnTab)
	)
	.SetDisplayName(LOCTEXT("TabTitle", "VCS"))
	.SetTooltipText(LOCTEXT("TabTooltip", "PGX Version Control: changelists, auto-tagging, validation, and commit workflow"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetGroup(FPGXWorkspaceMenu::GetToolsGroup())
	// Use the dedicated panel brush; the general version-control brush remains available to other contexts.
	.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.VersionControlPanel"));
}

void FPGXVersionControlInspectorTabSpawner::Unregister()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> FPGXVersionControlInspectorTabSpawner::SpawnTab(const FSpawnTabArgs& /*Args*/)
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		[
			SNew(SPGXVersionControlInspectorTab)
		];
}

#undef LOCTEXT_NAMESPACE
