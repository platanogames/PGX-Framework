// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Showcase/FPGXVisualShowcaseTabSpawner.h"
#include "Showcase/SPGXVisualShowcaseTab.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Workspace/PGXWorkspaceMenu.h"
#include "Style/PGXEditorStyle.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXVisualShowcase"

const FName FPGXVisualShowcaseTabSpawner::TabId(TEXT("PGXVisualShowcase"));

void FPGXVisualShowcaseTabSpawner::Register()
{
	PGX::Editor::RegisterNomadTab(
		TabId,
		FOnSpawnTab::CreateStatic(&FPGXVisualShowcaseTabSpawner::SpawnTab)
	)
	.SetDisplayName(LOCTEXT("TabName", "PGX Visual Showcase"))
	.SetTooltipText(LOCTEXT("TabTooltip", "Visual prototype showcasing all PGX editor components"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetGroup(FPGXWorkspaceMenu::GetToolsGroup())
	.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.Showcase"));
}

void FPGXVisualShowcaseTabSpawner::Unregister()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> FPGXVisualShowcaseTabSpawner::SpawnTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		.TabColorScale(FLinearColor(0.3f, 0.6f, 0.9f))
		[
			SNew(SPGXVisualShowcaseTab)
		];
}

#undef LOCTEXT_NAMESPACE
