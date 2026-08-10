// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "MGOSInspector/FPGXMGOSInspectorTabSpawner.h"
#include "MGOSInspector/SPGXMGOSInspectorTab.h"
#include "Widgets/Docking/SDockTab.h"
#include "Workspace/PGXWorkspaceMenu.h"
#include "Style/PGXEditorStyle.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXMGOSInspector"

const FName FPGXMGOSInspectorTabSpawner::TabId(TEXT("PGXMGOSInspector"));

void FPGXMGOSInspectorTabSpawner::Register()
{
	PGX::Editor::RegisterNomadTab(
		TabId,
		FOnSpawnTab::CreateStatic(&FPGXMGOSInspectorTabSpawner::SpawnTab)
	)
	.SetDisplayName(LOCTEXT("PGXMGOSInspectorTab", "MGOS"))
	.SetTooltipText(LOCTEXT("PGXMGOSInspectorTooltip", "GC Observability: mode, baseline, profile, incidents, history, and class health"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetGroup(FPGXWorkspaceMenu::GetInspectorsGroup())
	.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.MGOS"));
}

void FPGXMGOSInspectorTabSpawner::Unregister()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> FPGXMGOSInspectorTabSpawner::SpawnTab(const FSpawnTabArgs& /*Args*/)
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		[
			SNew(SPGXMGOSInspectorTab)
		];
}

#undef LOCTEXT_NAMESPACE
