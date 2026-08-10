// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "PlatformHealth/FPGXPlatformHealthTabSpawner.h"
#include "PlatformHealth/SPGXPlatformHealthTab.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Workspace/PGXWorkspaceMenu.h"
#include "Style/PGXEditorStyle.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXPlatformHealth"

const FName FPGXPlatformHealthTabSpawner::TabId(TEXT("PGXPlatformHealthDashboard"));

void FPGXPlatformHealthTabSpawner::Register()
{
	PGX::Editor::RegisterNomadTab(
		TabId,
		FOnSpawnTab::CreateStatic(&FPGXPlatformHealthTabSpawner::SpawnTab)
	)
	.SetDisplayName(LOCTEXT("TabTitle", "Platform"))
	.SetTooltipText(LOCTEXT("TabTooltip", "Platform budget overview — per-system limits, global budgets, platform comparison"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetGroup(FPGXWorkspaceMenu::GetDashboardsGroup())
	.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.PlatformHealth"));
}

void FPGXPlatformHealthTabSpawner::Unregister()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> FPGXPlatformHealthTabSpawner::SpawnTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		[
			SNew(SPGXPlatformHealthTab)
		];
}

#undef LOCTEXT_NAMESPACE
