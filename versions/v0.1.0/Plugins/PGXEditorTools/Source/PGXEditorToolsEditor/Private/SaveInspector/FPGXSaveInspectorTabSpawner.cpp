// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "SaveInspector/FPGXSaveInspectorTabSpawner.h"
#include "SaveInspector/SPGXSaveInspectorTab.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Workspace/PGXWorkspaceMenu.h"
#include "Style/PGXEditorStyle.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXSaveInspector"

const FName FPGXSaveInspectorTabSpawner::TabId(TEXT("PGXSaveInspector"));

void FPGXSaveInspectorTabSpawner::Register()
{
	PGX::Editor::RegisterNomadTab(
		TabId,
		FOnSpawnTab::CreateStatic(&FPGXSaveInspectorTabSpawner::SpawnTab)
	)
	.SetDisplayName(LOCTEXT("PGXSaveInspectorTab", "Save"))
	.SetTooltipText(LOCTEXT("PGXSaveInspectorTooltip", "Inspect PGX Save system state: contexts, domains, slots and pipeline activity"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetGroup(FPGXWorkspaceMenu::GetInspectorsGroup())
	.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.SaveInspector"));
}

void FPGXSaveInspectorTabSpawner::Unregister()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> FPGXSaveInspectorTabSpawner::SpawnTab(const FSpawnTabArgs& /*Args*/)
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		[
			SNew(SPGXSaveInspectorTab)
		];
}

#undef LOCTEXT_NAMESPACE
