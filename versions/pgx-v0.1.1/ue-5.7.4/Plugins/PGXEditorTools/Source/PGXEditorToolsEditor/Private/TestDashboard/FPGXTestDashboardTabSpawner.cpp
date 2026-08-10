// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "TestDashboard/FPGXTestDashboardTabSpawner.h"
#include "TestDashboard/SPGXTestDashboardTab.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Workspace/PGXWorkspaceMenu.h"
#include "Style/PGXEditorStyle.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXTestDashboard"

const FName FPGXTestDashboardTabSpawner::TabId(TEXT("PGXTestDashboard"));

void FPGXTestDashboardTabSpawner::Register()
{
	PGX::Editor::RegisterNomadTab(
		TabId,
		FOnSpawnTab::CreateStatic(&FPGXTestDashboardTabSpawner::SpawnTab)
	)
	.SetDisplayName(LOCTEXT("TestDashboardTab", "Tests"))
	.SetTooltipText(LOCTEXT("TestDashboardTooltip", "Run and view PGX system validation tests"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetGroup(FPGXWorkspaceMenu::GetDashboardsGroup())
	.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.TestDashboard"));
}

void FPGXTestDashboardTabSpawner::Unregister()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> FPGXTestDashboardTabSpawner::SpawnTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		[
			SNew(SPGXTestDashboardTab)
		];
}

#undef LOCTEXT_NAMESPACE
