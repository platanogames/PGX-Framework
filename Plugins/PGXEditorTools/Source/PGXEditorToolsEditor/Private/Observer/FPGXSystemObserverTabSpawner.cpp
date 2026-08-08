// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Observer/FPGXSystemObserverTabSpawner.h"
#include "Observer/SPGXSystemObserverTab.h"
#include "Workspace/PGXWorkspaceMenu.h"
#include "Style/PGXEditorStyle.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXSystemObserver"

const FName FPGXSystemObserverTabSpawner::TabId(TEXT("PGXSystemObserver"));

void FPGXSystemObserverTabSpawner::Register()
{
	PGX::Editor::RegisterNomadTab(
		TabId,
		FOnSpawnTab::CreateStatic(&FPGXSystemObserverTabSpawner::SpawnTab)
	)
	.SetDisplayName(LOCTEXT("PGXSystemObserverTab", "Observer"))
	.SetTooltipText(LOCTEXT("PGXSystemObserverTooltip", "Live dashboard of PGX framework health: core classes, subsystems, instances, and plugins"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetGroup(FPGXWorkspaceMenu::GetRoot())
	.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.SystemObserver"));
}

void FPGXSystemObserverTabSpawner::Unregister()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> FPGXSystemObserverTabSpawner::SpawnTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		[
			SNew(SPGXSystemObserverTab)
		];
}

#undef LOCTEXT_NAMESPACE
