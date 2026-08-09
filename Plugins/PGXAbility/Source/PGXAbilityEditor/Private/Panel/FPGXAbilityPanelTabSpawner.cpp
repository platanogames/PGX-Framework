// Copyright PGX Framework. All Rights Reserved.

#include "Panel/FPGXAbilityPanelTabSpawner.h"
#include "Panel/SPGXAbilityPanel.h"
#include "Style/PGXEditorStyle.h"
#include "Workspace/PGXWorkspaceMenu.h"

#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXAbilityPanel"

// EN: This panel owns a local stable tab identifier; no shared tab-id registry is required.
// ES: Centralizacion de TabId diferida — header PGX_TabIds.h no creado todavia.
const FName FPGXAbilityPanelTabSpawner::TabId(TEXT("PGXAbilityPanel"));

void FPGXAbilityPanelTabSpawner::Register()
{
	PGX::Editor::RegisterNomadTab(
		TabId,
		FOnSpawnTab::CreateStatic(&FPGXAbilityPanelTabSpawner::SpawnTab))
		.SetDisplayName(LOCTEXT("PGXAbilityPanelTab", "Ability"))
		.SetTooltipText(LOCTEXT(
			"PGXAbilityPanelTooltip",
			"Inspect PGX Ability subsystem: active abilities, registered components"))
		.SetMenuType(ETabSpawnerMenuType::Enabled)
		.SetGroup(FPGXWorkspaceMenu::GetSystemPanelsGroup())
		.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.AbilityPanel"));
}

void FPGXAbilityPanelTabSpawner::Unregister()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> FPGXAbilityPanelTabSpawner::SpawnTab(const FSpawnTabArgs& /*Args*/)
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		[
			SNew(SPGXAbilityPanel)
		];
}

#undef LOCTEXT_NAMESPACE
