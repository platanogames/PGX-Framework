// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Panel/FPGXLoadingPanelTabSpawner.h"
#include "Panel/SPGXLoadingPanel.h"
#include "Style/PGXEditorStyle.h"
#include "Workspace/PGXWorkspaceMenu.h"

#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXLoadingPanel"

const FName FPGXLoadingPanelTabSpawner::TabId(TEXT("PGXLoadingPanel"));

void FPGXLoadingPanelTabSpawner::Register()
{
	PGX::Editor::RegisterNomadTab(
		TabId,
		FOnSpawnTab::CreateStatic(&FPGXLoadingPanelTabSpawner::SpawnTab))
		.SetDisplayName(LOCTEXT("PGXLoadingPanelTab", "Loading"))
		.SetTooltipText(LOCTEXT(
			"PGXLoadingPanelTooltip",
			"Inspect PGX Loading + LevelFlow: active strategies, transitions, async loader queue, streaming chunks"))
		.SetMenuType(ETabSpawnerMenuType::Enabled)
		.SetGroup(FPGXWorkspaceMenu::GetSystemPanelsGroup())
		.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.LoadingPanel"));
}

void FPGXLoadingPanelTabSpawner::Unregister()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> FPGXLoadingPanelTabSpawner::SpawnTab(const FSpawnTabArgs& /*Args*/)
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		[
			SNew(SPGXLoadingPanel)
		];
}

#undef LOCTEXT_NAMESPACE
