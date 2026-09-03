// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Inspector/SPGXInputInspector.h"
#include "PGXInputConfig.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXInputInspector"

const FName SPGXInputInspector::TabId(TEXT("PGXInputInspector"));

void SPGXInputInspector::Construct(const FArguments& /*InArgs*/)
{
	BuildInspectorLayout();
}

void SPGXInputInspector::RegisterTabSpawner()
{
	PGX::Editor::RegisterNomadTab(TabId, FOnSpawnTab::CreateStatic(&SPGXInputInspector::SpawnTab))
		.SetDisplayName(LOCTEXT("TabTitle", "Input Inspector"))
		.SetTooltipText(LOCTEXT("TabTooltip", "Read-only PGXInput inspector"));
}

void SPGXInputInspector::UnregisterTabSpawner()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> SPGXInputInspector::SpawnTab(const FSpawnTabArgs& /*Args*/)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SPGXInputInspector)
		];
}

FText SPGXInputInspector::GetInspectorTitle() const
{
	return LOCTEXT("Header", "PGXInput Inspector");
}

TArray<TPair<FText, UClass*>> SPGXInputInspector::GetObservableClasses() const
{
	return {
		{ LOCTEXT("PGXInput0Label", "Input Config"), UPGXInputConfig::StaticClass() },
	};
}

TArray<TPair<FText, FText>> SPGXInputInspector::GetDeferredCards() const
{
	return {
		{ LOCTEXT("PGXInputDeferred0Label", "Not included"),
		  LOCTEXT("PGXInputDeferred0Detail", "Context stack, device, buffer and rebind snapshots are not exposed by the current inspector data contract.") },
		{ LOCTEXT("PGXInputDeferred1Label", "Not included"),
		  LOCTEXT("PGXInputDeferred1Detail", "Content Browser factories, toolbar integration, Hub routing, harness population, default config and demo assets are not included.") },
	};
}

#undef LOCTEXT_NAMESPACE
