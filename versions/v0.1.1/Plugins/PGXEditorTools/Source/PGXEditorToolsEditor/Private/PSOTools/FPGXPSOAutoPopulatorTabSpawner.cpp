// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PSOTools/FPGXPSOAutoPopulatorTabSpawner.h"
#include "PSOTools/SPGXPSOAutoPopulatorTab.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Workspace/PGXWorkspaceMenu.h"
#include "Style/PGXEditorStyle.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXPSOAutoPopulator"

const FName FPGXPSOAutoPopulatorTabSpawner::TabId(TEXT("PGXPSOAutoPopulator"));

void FPGXPSOAutoPopulatorTabSpawner::Register()
{
	PGX::Editor::RegisterNomadTab(
		TabId,
		FOnSpawnTab::CreateStatic(&FPGXPSOAutoPopulatorTabSpawner::SpawnTab)
	)
	.SetDisplayName(LOCTEXT("PGXPSOAutoPopulatorTab", "PSO Tool"))
	.SetTooltipText(LOCTEXT("PGXPSOAutoPopulatorTooltip", "Auto-populate PSO WarmUpConfig DAs from Content Browser selections"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetGroup(FPGXWorkspaceMenu::GetToolsGroup())
	.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.PSO"));
}

void FPGXPSOAutoPopulatorTabSpawner::Unregister()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> FPGXPSOAutoPopulatorTabSpawner::SpawnTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		[
			SNew(SPGXPSOAutoPopulatorTab)
		];
}

#undef LOCTEXT_NAMESPACE
