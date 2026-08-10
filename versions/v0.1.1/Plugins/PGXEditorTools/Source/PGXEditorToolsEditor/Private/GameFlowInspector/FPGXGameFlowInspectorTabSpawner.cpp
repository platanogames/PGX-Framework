// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "GameFlowInspector/FPGXGameFlowInspectorTabSpawner.h"
#include "GameFlowInspector/SPGXGameFlowInspectorTab.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Workspace/PGXWorkspaceMenu.h"
#include "Style/PGXEditorStyle.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXGameFlowInspector"

const FName FPGXGameFlowInspectorTabSpawner::TabId(TEXT("PGXGameFlowInspector"));

void FPGXGameFlowInspectorTabSpawner::Register()
{
	PGX::Editor::RegisterNomadTab(
		TabId,
		FOnSpawnTab::CreateStatic(&FPGXGameFlowInspectorTabSpawner::SpawnTab)
	)
	.SetDisplayName(LOCTEXT("PGXGameFlowInspectorTab", "GameFlow"))
	.SetTooltipText(LOCTEXT("PGXGameFlowInspectorTooltip", "Inspect GameFlow channels: current states, transition history, and rules"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetGroup(FPGXWorkspaceMenu::GetInspectorsGroup())
	.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.GameFlow"));
}

void FPGXGameFlowInspectorTabSpawner::Unregister()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> FPGXGameFlowInspectorTabSpawner::SpawnTab(const FSpawnTabArgs& /*Args*/)
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		[
			SNew(SPGXGameFlowInspectorTab)
		];
}

#undef LOCTEXT_NAMESPACE
