// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "ProfileInspector/FPGXProfileInspectorTabSpawner.h"
#include "ProfileInspector/SPGXProfileInspectorTab.h"
#include "Widgets/Docking/SDockTab.h"
#include "Workspace/PGXWorkspaceMenu.h"
#include "Style/PGXEditorStyle.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXProfileInspector"

const FName FPGXProfileInspectorTabSpawner::TabId(TEXT("PGXProfileInspector"));

void FPGXProfileInspectorTabSpawner::Register()
{
	PGX::Editor::RegisterNomadTab(
		TabId,
		FOnSpawnTab::CreateStatic(&FPGXProfileInspectorTabSpawner::SpawnTab)
	)
	.SetDisplayName(LOCTEXT("PGXProfileInspectorTab", "Profile"))
	.SetTooltipText(LOCTEXT("PGXProfileInspectorTooltip", "Inspect project profile: identity, capabilities, policies, budgets, features, and simulation"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetGroup(FPGXWorkspaceMenu::GetInspectorsGroup())
	.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.Profile"));
}

void FPGXProfileInspectorTabSpawner::Unregister()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> FPGXProfileInspectorTabSpawner::SpawnTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		[
			SNew(SPGXProfileInspectorTab)
		];
}

#undef LOCTEXT_NAMESPACE
