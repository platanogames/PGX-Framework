// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Inspector/FPGXRegistryValidationTabSpawner.h"
#include "Inspector/SPGXRegistryValidationTab.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

#define LOCTEXT_NAMESPACE "PGXRegistryEditor"

const FName FPGXRegistryValidationTabSpawner::TabId(TEXT("PGXRegistryValidation"));

// EN: Find PGX Dashboards group by tree-walking the workspace root.
//     PGXRegistryEditor cannot link against PGXCoreEditor (circular dep),
//     so we navigate the workspace tree to find the "PGX Framework" > "Dashboards" group.
// ES: Encontrar grupo PGX Dashboards recorriendo la raiz del workspace.
//     PGXRegistryEditor no puede linkar contra PGXCoreEditor (dep circular),
//     asi que navegamos el arbol del workspace para encontrar el grupo "PGX Framework" > "Dashboards".
static TSharedPtr<FWorkspaceItem> FindPGXDashboardsGroup()
{
	for (const TSharedRef<FWorkspaceItem>& Child : WorkspaceMenu::GetMenuStructure().GetStructureRoot()->GetChildItems())
	{
		if (Child->GetDisplayName().ToString().Contains(TEXT("PGX")))
		{
			for (const TSharedRef<FWorkspaceItem>& SubChild : Child->GetChildItems())
			{
				if (SubChild->GetDisplayName().ToString().Contains(TEXT("Dashboard")))
				{
					return SubChild;
				}
			}
			return Child;
		}
	}
	return nullptr;
}

void FPGXRegistryValidationTabSpawner::Register()
{
	FTabSpawnerEntry& Spawner = FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		TabId,
		FOnSpawnTab::CreateStatic(&FPGXRegistryValidationTabSpawner::SpawnTab)
	)
	.SetDisplayName(LOCTEXT("RegistryValidationTab", "Validation"))
	.SetTooltipText(LOCTEXT("RegistryValidationTooltip", "Validate DataTable registry entries, apply safe fixes, and export reports"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetIcon(FSlateIcon(FName("PGXEditorStyle"), "PGXEditor.Icon.Validate"));

	// EN: Set group only if found (avoids crash if PGXCoreEditor hasn't registered workspace yet)
	// ES: Asignar grupo solo si existe (evita crash si PGXCoreEditor aun no registro el workspace)
	if (TSharedPtr<FWorkspaceItem> DashboardGroup = FindPGXDashboardsGroup())
	{
		Spawner.SetGroup(DashboardGroup.ToSharedRef());
	}
}

void FPGXRegistryValidationTabSpawner::Unregister()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabId);
}

TSharedRef<SDockTab> FPGXRegistryValidationTabSpawner::SpawnTab(const FSpawnTabArgs& /*Args*/)
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		[
			SNew(SPGXRegistryValidationTab)
		];
}

#undef LOCTEXT_NAMESPACE
