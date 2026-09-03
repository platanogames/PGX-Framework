// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Inspector/SPGXUIInspector.h"
#include "PGXUIConfig.h"
#include "PGXScreenDefinition.h"
#include "PGXNotificationProfile.h"
#include "PGXWidgetPoolProfile.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXUIInspector"

const FName SPGXUIInspector::TabId(TEXT("PGXUIInspector"));

void SPGXUIInspector::Construct(const FArguments& /*InArgs*/)
{
	BuildInspectorLayout();
}

void SPGXUIInspector::RegisterTabSpawner()
{
	PGX::Editor::RegisterNomadTab(TabId, FOnSpawnTab::CreateStatic(&SPGXUIInspector::SpawnTab))
		.SetDisplayName(LOCTEXT("TabTitle", "UI Inspector"))
		.SetTooltipText(LOCTEXT("TabTooltip", "Read-only PGXUI inspector"));
}

void SPGXUIInspector::UnregisterTabSpawner()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> SPGXUIInspector::SpawnTab(const FSpawnTabArgs& /*Args*/)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SPGXUIInspector)
		];
}

FText SPGXUIInspector::GetInspectorTitle() const
{
	return LOCTEXT("Header", "PGXUI Inspector");
}

TArray<TPair<FText, UClass*>> SPGXUIInspector::GetObservableClasses() const
{
	return {
		{ LOCTEXT("PGXUI0Label", "UI Config"),              UPGXUIConfig::StaticClass() },
		{ LOCTEXT("PGXUI1Label", "Screen Definition"),      UPGXScreenDefinition::StaticClass() },
		{ LOCTEXT("PGXUI2Label", "Notification Profile"),   UPGXNotificationProfile::StaticClass() },
		{ LOCTEXT("PGXUI3Label", "Widget Pool Profile"),    UPGXWidgetPoolProfile::StaticClass() },
	};
}

TArray<TPair<FText, FText>> SPGXUIInspector::GetDeferredCards() const
{
	return {
		{ LOCTEXT("PGXUIDeferred0Label", "Not included"),
		  LOCTEXT("PGXUIDeferred0Detail", "The current UI runtime does not consume Settings ActiveConfig.") },
		{ LOCTEXT("PGXUIDeferred1Label", "Not included"),
		  LOCTEXT("PGXUIDeferred1Detail", "UMG spawning, accessibility, binding, themes, localization, Content Browser factories, toolbar integration, Hub routing, harness population, default config and demo assets are not included.") },
	};
}

#undef LOCTEXT_NAMESPACE
