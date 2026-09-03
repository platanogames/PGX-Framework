// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Inspector/SPGXCraftingInspector.h"
#include "PGXCraftingTypes.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXCraftingInspector"

const FName SPGXCraftingInspector::TabId(TEXT("PGXCraftingInspector"));

void SPGXCraftingInspector::Construct(const FArguments& /*InArgs*/)
{
	BuildInspectorLayout();
}

void SPGXCraftingInspector::RegisterTabSpawner()
{
	PGX::Editor::RegisterNomadTab(TabId, FOnSpawnTab::CreateStatic(&SPGXCraftingInspector::SpawnTab))
		.SetDisplayName(LOCTEXT("TabTitle", "Crafting Inspector"))
		.SetTooltipText(LOCTEXT("TabTooltip", "Read-only Development Preview PGXCrafting inspector"));
}

void SPGXCraftingInspector::UnregisterTabSpawner()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> SPGXCraftingInspector::SpawnTab(const FSpawnTabArgs& /*Args*/)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SPGXCraftingInspector)
		];
}

FText SPGXCraftingInspector::GetInspectorTitle() const
{
	return LOCTEXT("Header", "PGXCrafting Development Preview Inspector");
}

TArray<TPair<FText, UClass*>> SPGXCraftingInspector::GetObservableClasses() const
{
	return {
		{ LOCTEXT("PGXCrafting0Label", "Recipe Definition"), UPGXRecipeDefinition::StaticClass() },
	};
}

TArray<TPair<FText, FText>> SPGXCraftingInspector::GetDeferredCards() const
{
	return {
		{ LOCTEXT("PGXCraftingDeferred0Label", "Not included"),
		  LOCTEXT("PGXCraftingDeferred0Detail", "Recipe runtime resolution, ingredient inventory queries, and craft history snapshots require a world-backed inspector data pass; this Development Preview tab exposes safe config/schema status only.") },
		{ LOCTEXT("PGXCraftingDeferred1Label", "Not included"),
		  LOCTEXT("PGXCraftingDeferred1Detail", "Asset creation shortcuts and live runtime diagnostics are not included in this preview.") },
	};
}

#undef LOCTEXT_NAMESPACE
