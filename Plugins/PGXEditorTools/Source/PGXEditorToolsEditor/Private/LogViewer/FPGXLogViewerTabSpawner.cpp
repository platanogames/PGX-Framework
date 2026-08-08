// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "LogViewer/FPGXLogViewerTabSpawner.h"
#include "LogViewer/SPGXLogViewerTab.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Workspace/PGXWorkspaceMenu.h"
#include "Style/PGXEditorStyle.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXLogViewer"

const FName FPGXLogViewerTabSpawner::TabId(TEXT("PGXLogViewer"));

void FPGXLogViewerTabSpawner::Register()
{
	PGX::Editor::RegisterNomadTab(
		TabId,
		FOnSpawnTab::CreateStatic(&FPGXLogViewerTabSpawner::SpawnTab)
	)
	.SetDisplayName(LOCTEXT("PGXLogViewerTab", "Log"))
	.SetTooltipText(LOCTEXT("PGXLogViewerTooltip", "View and search PGX structured logs with GameplayTag filtering"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetGroup(FPGXWorkspaceMenu::GetInspectorsGroup())
	.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.LogViewer"));
}

void FPGXLogViewerTabSpawner::Unregister()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> FPGXLogViewerTabSpawner::SpawnTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		[
			SNew(SPGXLogViewerTab)
		];
}

#undef LOCTEXT_NAMESPACE
