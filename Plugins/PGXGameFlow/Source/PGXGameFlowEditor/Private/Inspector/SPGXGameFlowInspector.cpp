// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Inspector/SPGXGameFlowInspector.h"
#include "PGXGameFlowConfig.h"
#include "PGXFlowRulesConfig.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXGameFlowInspector"

const FName SPGXGameFlowInspector::TabId(TEXT("PGXGameFlowInspector"));

void SPGXGameFlowInspector::Construct(const FArguments& /*InArgs*/)
{
	BuildInspectorLayout();
}

void SPGXGameFlowInspector::RegisterTabSpawner()
{
	PGX::Editor::RegisterNomadTab(TabId, FOnSpawnTab::CreateStatic(&SPGXGameFlowInspector::SpawnTab))
		.SetDisplayName(LOCTEXT("TabTitle", "GameFlow Inspector"))
		.SetTooltipText(LOCTEXT("TabTooltip", "Read-only PGXGameFlow inspector"));
}

void SPGXGameFlowInspector::UnregisterTabSpawner()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> SPGXGameFlowInspector::SpawnTab(const FSpawnTabArgs& /*Args*/)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SPGXGameFlowInspector)
		];
}

FText SPGXGameFlowInspector::GetInspectorTitle() const
{
	return LOCTEXT("Header", "PGXGameFlow Inspector");
}

TArray<TPair<FText, UClass*>> SPGXGameFlowInspector::GetObservableClasses() const
{
	return {
		{ LOCTEXT("PGXGameFlow0Label", "GameFlow Config"),   UPGXGameFlowConfig::StaticClass() },
		{ LOCTEXT("PGXGameFlow1Label", "Flow Rules Config"), UPGXFlowRulesConfig::StaticClass() },
	};
}

TArray<TPair<FText, FText>> SPGXGameFlowInspector::GetDeferredCards() const
{
	return {
		{ LOCTEXT("PGXGameFlowDeferred0Label", "Deferred"),
		  LOCTEXT("PGXGameFlowDeferred0Detail", "Channel state snapshots, transition history, and bridge message audit require a world-backed inspector data pass; this tab exposes safe config/schema status only.") },
	};
}

#undef LOCTEXT_NAMESPACE
