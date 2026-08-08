// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PSOInspector/FPGXPSOInspectorTabSpawner.h"
#include "PSOInspector/SPGXPSOInspectorTab.h"
#include "Widgets/Docking/SDockTab.h"
#include "Workspace/PGXWorkspaceMenu.h"
#include "Style/PGXEditorStyle.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXPSOInspector"

const FName FPGXPSOInspectorTabSpawner::TabId(TEXT("PGXPSOInspector"));

void FPGXPSOInspectorTabSpawner::Register()
{
	PGX::Editor::RegisterNomadTab(
		TabId,
		FOnSpawnTab::CreateStatic(&FPGXPSOInspectorTabSpawner::SpawnTab)
	)
	.SetDisplayName(LOCTEXT("PGXPSOInspectorTab", "PSO"))
	.SetTooltipText(LOCTEXT("PGXPSOInspectorTooltip", "Inspect PSO warm-up pipeline: state, progress, configs, and recording sessions"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetGroup(FPGXWorkspaceMenu::GetInspectorsGroup())
	.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.PSO"));
}

void FPGXPSOInspectorTabSpawner::Unregister()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> FPGXPSOInspectorTabSpawner::SpawnTab(const FSpawnTabArgs& /*Args*/)
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		[
			SNew(SPGXPSOInspectorTab)
		];
}

#undef LOCTEXT_NAMESPACE
