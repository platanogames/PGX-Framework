// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Inspector/SPGXSpawnInspector.h"
#include "PGXSpawnConfig.h"
#include "PGXWaveDefinition.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXSpawnInspector"

const FName SPGXSpawnInspector::TabId(TEXT("PGXSpawnInspector"));

void SPGXSpawnInspector::Construct(const FArguments& /*InArgs*/)
{
	BuildInspectorLayout();
}

void SPGXSpawnInspector::RegisterTabSpawner()
{
	PGX::Editor::RegisterNomadTab(TabId, FOnSpawnTab::CreateStatic(&SPGXSpawnInspector::SpawnTab))
		.SetDisplayName(LOCTEXT("TabTitle", "Spawn Inspector"))
		.SetTooltipText(LOCTEXT("TabTooltip", "Read-only Development Preview PGXSpawn inspector"));
}

void SPGXSpawnInspector::UnregisterTabSpawner()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> SPGXSpawnInspector::SpawnTab(const FSpawnTabArgs& /*Args*/)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SPGXSpawnInspector)
		];
}

FText SPGXSpawnInspector::GetInspectorTitle() const
{
	return LOCTEXT("Header", "PGXSpawn Development Preview Inspector");
}

TArray<TPair<FText, UClass*>> SPGXSpawnInspector::GetObservableClasses() const
{
	return {
		{ LOCTEXT("PGXSpawn0Label", "Spawn Config"),     UPGXSpawnConfig::StaticClass() },
		{ LOCTEXT("PGXSpawn1Label", "Wave Definition"),  UPGXWaveDefinition::StaticClass() },
	};
}

TArray<TPair<FText, FText>> SPGXSpawnInspector::GetDeferredCards() const
{
	return {
		{ LOCTEXT("PGXSpawnDeferred0Label", "Not included"),
		  LOCTEXT("PGXSpawnDeferred0Detail", "Spawner runtime queue, wave-in-flight state, and spawn telemetry require a world-backed inspector data pass; this Development Preview tab exposes safe config/schema status only.") },
		{ LOCTEXT("PGXSpawnDeferred1Label", "Not included"),
		  LOCTEXT("PGXSpawnDeferred1Detail", "Asset creation shortcuts and live runtime diagnostics are not included in this preview.") },
	};
}

#undef LOCTEXT_NAMESPACE
