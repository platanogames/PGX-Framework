// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Inspector/SPGXVehiclesInspector.h"
#include "PGXVehiclesTypes.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXVehiclesInspector"

const FName SPGXVehiclesInspector::TabId(TEXT("PGXVehiclesInspector"));

void SPGXVehiclesInspector::Construct(const FArguments& /*InArgs*/)
{
	BuildInspectorLayout();
}

void SPGXVehiclesInspector::RegisterTabSpawner()
{
	PGX::Editor::RegisterNomadTab(TabId, FOnSpawnTab::CreateStatic(&SPGXVehiclesInspector::SpawnTab))
		.SetDisplayName(LOCTEXT("TabTitle", "Vehicles Inspector"))
		.SetTooltipText(LOCTEXT("TabTooltip", "Read-only PGXVehicles inspector"));
}

void SPGXVehiclesInspector::UnregisterTabSpawner()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> SPGXVehiclesInspector::SpawnTab(const FSpawnTabArgs& /*Args*/)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SPGXVehiclesInspector)
		];
}

FText SPGXVehiclesInspector::GetInspectorTitle() const
{
	return LOCTEXT("Header", "PGXVehicles Inspector");
}

TArray<TPair<FText, UClass*>> SPGXVehiclesInspector::GetObservableClasses() const
{
	return {
		{ LOCTEXT("PGXVehicles0Label", "Vehicle Definition Asset"), UPGXVehicleDefinitionAsset::StaticClass() },
	};
}

TArray<TPair<FText, FText>> SPGXVehiclesInspector::GetDeferredCards() const
{
	return {
		{ LOCTEXT("PGXVehiclesDeferred0Label", "Not included"),
		  LOCTEXT("PGXVehiclesDeferred0Detail", "Vehicle runtime telemetry, suspension state and gameplay activity require a world-backed data source; this tab exposes config and schema status only.") },
		{ LOCTEXT("PGXVehiclesDeferred1Label", "Not included"),
		  LOCTEXT("PGXVehiclesDeferred1Detail", "Vehicle factories, toolbar integration, Hub routing, SimHarness population, default config and demo assets are not included.") },
	};
}

#undef LOCTEXT_NAMESPACE
