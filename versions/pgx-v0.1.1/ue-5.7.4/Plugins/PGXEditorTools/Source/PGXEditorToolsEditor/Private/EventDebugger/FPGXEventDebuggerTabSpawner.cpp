// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "EventDebugger/FPGXEventDebuggerTabSpawner.h"
#include "EventDebugger/SPGXEventDebuggerTab.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Workspace/PGXWorkspaceMenu.h"
#include "Style/PGXEditorStyle.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXEventDebugger"

const FName FPGXEventDebuggerTabSpawner::TabId(TEXT("PGXEventDebugger"));

void FPGXEventDebuggerTabSpawner::Register()
{
	PGX::Editor::RegisterNomadTab(
		TabId,
		FOnSpawnTab::CreateStatic(&FPGXEventDebuggerTabSpawner::SpawnTab)
	)
	.SetDisplayName(LOCTEXT("TabTitle", "Events"))
	.SetTooltipText(LOCTEXT("TabTooltip", "Debug PGX Event Handler: resolution, telemetry, lifecycle"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetGroup(FPGXWorkspaceMenu::GetInspectorsGroup())
	.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.EventHandler"));
}

void FPGXEventDebuggerTabSpawner::Unregister()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> FPGXEventDebuggerTabSpawner::SpawnTab(const FSpawnTabArgs& /*Args*/)
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		[
			SNew(SPGXEventDebuggerTab)
		];
}

#undef LOCTEXT_NAMESPACE
