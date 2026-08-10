// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "LevelFlowInspector/FPGXLevelFlowInspectorTabSpawner.h"
#include "LevelFlowInspector/SPGXLevelFlowInspectorTab.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Workspace/PGXWorkspaceMenu.h"
#include "Style/PGXEditorStyle.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXLevelFlowInspector"

const FName FPGXLevelFlowInspectorTabSpawner::TabId(TEXT("PGXLevelFlowInspector"));

void FPGXLevelFlowInspectorTabSpawner::Register()
{
	PGX::Editor::RegisterNomadTab(
		TabId,
		FOnSpawnTab::CreateStatic(&FPGXLevelFlowInspectorTabSpawner::SpawnTab)
	)
	.SetDisplayName(LOCTEXT("PGXLevelFlowInspectorTab", "LevelFlow"))
	.SetTooltipText(LOCTEXT("PGXLevelFlowInspectorTooltip", "Inspect level transitions: status, catalog, history, and sub-levels"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetGroup(FPGXWorkspaceMenu::GetInspectorsGroup())
	.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.LevelFlow"));
}

void FPGXLevelFlowInspectorTabSpawner::Unregister()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> FPGXLevelFlowInspectorTabSpawner::SpawnTab(const FSpawnTabArgs& /*Args*/)
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		[
			SNew(SPGXLevelFlowInspectorTab)
		];
}

#undef LOCTEXT_NAMESPACE
