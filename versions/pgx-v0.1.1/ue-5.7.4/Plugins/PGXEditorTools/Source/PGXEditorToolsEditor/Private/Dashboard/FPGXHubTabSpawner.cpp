// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Dashboard/FPGXHubTabSpawner.h"
#include "Dashboard/SPGXHubTab.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Workspace/PGXWorkspaceMenu.h"
#include "Style/PGXEditorStyle.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXHub"

const FName FPGXHubTabSpawner::TabId(TEXT("PGXHub"));

void FPGXHubTabSpawner::Register()
{
	PGX::Editor::RegisterNomadTab(
		TabId,
		FOnSpawnTab::CreateStatic(&FPGXHubTabSpawner::SpawnTab)
	)
	.SetDisplayName(LOCTEXT("PGXHubTab", "PGX Hub"))
	.SetTooltipText(LOCTEXT("PGXHubTooltip", "Open the PGX Framework Hub"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetGroup(FPGXWorkspaceMenu::GetRoot())
	.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.Hub"));
}

void FPGXHubTabSpawner::Unregister()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> FPGXHubTabSpawner::SpawnTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		[
			SNew(SPGXHubTab)
		];
}

#undef LOCTEXT_NAMESPACE
