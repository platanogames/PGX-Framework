// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Browser/FPGXDataRegistryBrowserTabSpawner.h"
#include "Browser/SPGXDataRegistryBrowser.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Workspace/PGXWorkspaceMenu.h"
#include "Style/PGXEditorStyle.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXDataRegistryBrowser"

const FName FPGXDataRegistryBrowserTabSpawner::TabId(TEXT("PGXDataRegistryBrowser"));

void FPGXDataRegistryBrowserTabSpawner::Register()
{
	PGX::Editor::RegisterNomadTab(
		TabId,
		FOnSpawnTab::CreateStatic(&FPGXDataRegistryBrowserTabSpawner::SpawnTab)
	)
	.SetDisplayName(LOCTEXT("TabTitle", "Registry"))
	.SetTooltipText(LOCTEXT("TabTooltip", "Browse registered Object DataAssets — databases, entries, and cache"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetGroup(FPGXWorkspaceMenu::GetDashboardsGroup())
	.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.DataRegistry"));
}

void FPGXDataRegistryBrowserTabSpawner::Unregister()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> FPGXDataRegistryBrowserTabSpawner::SpawnTab(const FSpawnTabArgs& /*Args*/)
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		[
			SNew(SPGXDataRegistryBrowser)
		];
}

#undef LOCTEXT_NAMESPACE
