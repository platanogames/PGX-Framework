// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "MessageInspector/FPGXMessageInspectorTabSpawner.h"
#include "MessageInspector/SPGXMessageInspectorTab.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Workspace/PGXWorkspaceMenu.h"
#include "Style/PGXEditorStyle.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXMessageInspector"

const FName FPGXMessageInspectorTabSpawner::TabId(TEXT("PGXMessageInspector"));

void FPGXMessageInspectorTabSpawner::Register()
{
	PGX::Editor::RegisterNomadTab(
		TabId,
		FOnSpawnTab::CreateStatic(&FPGXMessageInspectorTabSpawner::SpawnTab)
	)
	.SetDisplayName(LOCTEXT("TabTitle", "Messages"))
	.SetTooltipText(LOCTEXT("TabTooltip", "Inspect PGX Message System: channels, listeners, history"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetGroup(FPGXWorkspaceMenu::GetInspectorsGroup())
	.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.Message"));
}

void FPGXMessageInspectorTabSpawner::Unregister()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> FPGXMessageInspectorTabSpawner::SpawnTab(const FSpawnTabArgs& /*Args*/)
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		[
			SNew(SPGXMessageInspectorTab)
		];
}

#undef LOCTEXT_NAMESPACE
