// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Browser/FPGXConfigDashboardTabSpawner.h"
#include "Browser/SPGXConfigDashboard.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Workspace/PGXWorkspaceMenu.h"
#include "Style/PGXEditorStyle.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXConfigDashboard"

const FName FPGXConfigDashboardTabSpawner::TabId(TEXT("PGXConfigDashboard"));

void FPGXConfigDashboardTabSpawner::Register()
{
	PGX::Editor::RegisterNomadTab(
		TabId,
		FOnSpawnTab::CreateStatic(&FPGXConfigDashboardTabSpawner::SpawnTab)
	)
	.SetDisplayName(LOCTEXT("TabTitle", "Config"))
	.SetTooltipText(LOCTEXT("TabTooltip", "View and manage all Config DataAssets across the framework"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetGroup(FPGXWorkspaceMenu::GetDashboardsGroup())
	.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.ConfigDashboard"));
}

void FPGXConfigDashboardTabSpawner::Unregister()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> FPGXConfigDashboardTabSpawner::SpawnTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		[
			SNew(SPGXConfigDashboard)
		];
}

#undef LOCTEXT_NAMESPACE
