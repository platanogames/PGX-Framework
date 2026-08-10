// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "LoadingInspector/FPGXLoadingInspectorTabSpawner.h"
#include "LoadingInspector/SPGXLoadingInspectorTab.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Workspace/PGXWorkspaceMenu.h"
#include "Style/PGXEditorStyle.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXLoadingInspector"

const FName FPGXLoadingInspectorTabSpawner::TabId(TEXT("PGXLoadingInspector"));

void FPGXLoadingInspectorTabSpawner::Register()
{
	PGX::Editor::RegisterNomadTab(
		TabId,
		FOnSpawnTab::CreateStatic(&FPGXLoadingInspectorTabSpawner::SpawnTab)
	)
	.SetDisplayName(LOCTEXT("PGXLoadingInspectorTab", "Loading"))
	.SetTooltipText(LOCTEXT("PGXLoadingInspectorTooltip", "Inspect loading screen: status, profiles, history, and debug controls"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetGroup(FPGXWorkspaceMenu::GetInspectorsGroup())
	.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.Loading"));
}

void FPGXLoadingInspectorTabSpawner::Unregister()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> FPGXLoadingInspectorTabSpawner::SpawnTab(const FSpawnTabArgs& /*Args*/)
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		[
			SNew(SPGXLoadingInspectorTab)
		];
}

#undef LOCTEXT_NAMESPACE
