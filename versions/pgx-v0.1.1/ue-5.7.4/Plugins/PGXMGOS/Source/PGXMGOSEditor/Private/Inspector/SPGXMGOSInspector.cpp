// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Inspector/SPGXMGOSInspector.h"
#include "PGXGCObserverConfig.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXMGOSInspector"

const FName SPGXMGOSInspector::TabId(TEXT("PGXMGOSInspector"));

void SPGXMGOSInspector::Construct(const FArguments& /*InArgs*/)
{
	BuildInspectorLayout();
}

void SPGXMGOSInspector::RegisterTabSpawner()
{
	PGX::Editor::RegisterNomadTab(TabId, FOnSpawnTab::CreateStatic(&SPGXMGOSInspector::SpawnTab))
		.SetDisplayName(LOCTEXT("TabTitle", "MGOS Inspector"))
		.SetTooltipText(LOCTEXT("TabTooltip", "Read-only PGXMGOS inspector"));
}

void SPGXMGOSInspector::UnregisterTabSpawner()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> SPGXMGOSInspector::SpawnTab(const FSpawnTabArgs& /*Args*/)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SPGXMGOSInspector)
		];
}

FText SPGXMGOSInspector::GetInspectorTitle() const
{
	return LOCTEXT("Header", "PGXMGOS Inspector");
}

TArray<TPair<FText, UClass*>> SPGXMGOSInspector::GetObservableClasses() const
{
	return {
		{ LOCTEXT("PGXMGOS0Label", "GC Observer Config"), UPGXGCObserverConfig::StaticClass() },
	};
}

TArray<TPair<FText, FText>> SPGXMGOSInspector::GetDeferredCards() const
{
	return {
		{ LOCTEXT("PGXMGOSDeferred0Label", "Deferred"),
		  LOCTEXT("PGXMGOSDeferred0Detail", "GC observation runtime snapshots, allocation history, and memory pressure events require a world-backed inspector data pass; this tab exposes safe config/schema status only.") },
	};
}

#undef LOCTEXT_NAMESPACE
