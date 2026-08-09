// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Inspector/FPGXMessageInspectorTabSpawner.h"
#include "Inspector/SPGXMessageInspectorTab.h"
#include "Style/PGXEditorStyle.h"
#include "Workspace/PGXWorkspaceMenu.h"

#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXMessageInspector"

const FName FPGXMessageInspectorTabSpawner::TabId(TEXT("PGXMessageInspector"));

void FPGXMessageInspectorTabSpawner::Register()
{
	// EN: Step 6 — visibility promoted Hidden -> Enabled. Workspace group:
	//     PGX Inspectors group (shared by GameFlow/Save/Message). Icon: reuse
	//     PGXEditor.Icon.LogViewer as the closest existing diagnostics-tool
	//     proxy because no Message-specific SVG brush is currently registered.
	// ES: Step 6 — visibilidad promovida Hidden -> Enabled. Grupo workspace:
	//     PGX Inspectors group (compartido con GameFlow/Save/Message). Icon:
	//     reuso PGXEditor.Icon.LogViewer como proxy mas cercano de herramienta
	//     diagnostica porque no hay un brush SVG especifico para Message registrado.
	PGX::Editor::RegisterNomadTab(
		TabId,
		FOnSpawnTab::CreateStatic(&FPGXMessageInspectorTabSpawner::SpawnTab))
		.SetDisplayName(LOCTEXT("PGXMessageInspectorTab", "Messages"))
		.SetTooltipText(LOCTEXT(
			"PGXMessageInspectorTooltip",
			"Inspect PGX Message Subsystem: channels, listeners, history, broadcasts"))
		.SetMenuType(ETabSpawnerMenuType::Enabled)
		.SetGroup(FPGXWorkspaceMenu::GetInspectorsGroup())
		.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.LogViewer"));
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
