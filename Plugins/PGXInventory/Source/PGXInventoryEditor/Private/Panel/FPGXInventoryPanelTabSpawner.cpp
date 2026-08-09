// Copyright PGX Framework. All Rights Reserved.

#include "Panel/FPGXInventoryPanelTabSpawner.h"
#include "Panel/SPGXInventoryPanel.h"

#include "Style/PGXEditorStyle.h"
#include "Workspace/PGXWorkspaceMenu.h"

#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXInventory"

const FName FPGXInventoryPanelTabSpawner::TabId(TEXT("PGXInventoryPanel"));

void FPGXInventoryPanelTabSpawner::Register()
{
	PGX::Editor::RegisterNomadTab(
		TabId,
		FOnSpawnTab::CreateStatic(&FPGXInventoryPanelTabSpawner::SpawnTab)
	)
	.SetDisplayName(LOCTEXT("InventoryTab", "Inventory"))
	.SetTooltipText(LOCTEXT("InventoryTooltip", "Inspect PGX Inventory: ItemDefinition IPGXObservable observable contract + Component live state + Subsystem GAP."))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetGroup(FPGXWorkspaceMenu::GetSystemPanelsGroup())
	.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.InventoryPanel"));
}

void FPGXInventoryPanelTabSpawner::Unregister()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> FPGXInventoryPanelTabSpawner::SpawnTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		[
			SNew(SPGXInventoryPanel)
		];
}

#undef LOCTEXT_NAMESPACE
