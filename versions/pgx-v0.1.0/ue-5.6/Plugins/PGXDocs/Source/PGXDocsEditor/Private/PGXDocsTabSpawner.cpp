// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXDocsTabSpawner.h"
#include "Widgets/SDocWindow.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXDocs"

const FName FPGXDocsTabSpawner::TabId(TEXT("PGXDocs"));

// EN: Find PGX Developer Tools group by tree-walking the workspace root.
//     PGXDocsEditor cannot link against PGXCoreEditor (circular dep via PGXDocs),
//     so we navigate the workspace tree to find the "PGX Framework" > "Developer Tools" group.
// ES: Encontrar grupo PGX Developer Tools recorriendo la raiz del workspace.
//     PGXDocsEditor no puede linkar contra PGXCoreEditor (dep circular via PGXDocs),
//     asi que navegamos el arbol del workspace para encontrar "PGX Framework" > "Developer Tools".
static TSharedPtr<FWorkspaceItem> FindPGXToolsGroup()
{
	for (const TSharedRef<FWorkspaceItem>& Child : WorkspaceMenu::GetMenuStructure().GetStructureRoot()->GetChildItems())
	{
		if (Child->GetDisplayName().ToString().Contains(TEXT("PGX")))
		{
			for (const TSharedRef<FWorkspaceItem>& SubChild : Child->GetChildItems())
			{
				if (SubChild->GetDisplayName().ToString().Contains(TEXT("Tool")))
				{
					return SubChild;
				}
			}
			return Child;
		}
	}
	return nullptr;
}

void FPGXDocsTabSpawner::Register()
{
	FTabSpawnerEntry& Spawner = PGX::Editor::RegisterNomadTab(
		TabId,
		FOnSpawnTab::CreateStatic(&FPGXDocsTabSpawner::SpawnTab)
	)
	.SetDisplayName(LOCTEXT("PGXDocsTab", "Docs"))
	.SetTooltipText(LOCTEXT("PGXDocsTooltip", "Open the PGX Documentation Viewer"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetIcon(FSlateIcon(FName("PGXEditorStyle"), "PGXEditor.Icon.Docs"));

	// EN: Set group only if found (avoids crash if PGXCoreEditor hasn't registered workspace yet)
	// ES: Asignar grupo solo si existe (evita crash si PGXCoreEditor aun no registro el workspace)
	if (TSharedPtr<FWorkspaceItem> ToolsGroup = FindPGXToolsGroup())
	{
		Spawner.SetGroup(ToolsGroup.ToSharedRef());
	}
}

void FPGXDocsTabSpawner::Unregister()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> FPGXDocsTabSpawner::SpawnTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		[
			SNew(SDocWindow)
		];
}

#undef LOCTEXT_NAMESPACE
