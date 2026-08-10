// Copyright PGX Framework. All Rights Reserved.

#include "Panel/FPGXCameraPanelTabSpawner.h"
#include "Panel/SPGXCameraPanel.h"

#include "Style/PGXEditorStyle.h"
#include "Workspace/PGXWorkspaceMenu.h"

#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXCamera"

const FName FPGXCameraPanelTabSpawner::TabId(TEXT("PGXCameraPanel"));

void FPGXCameraPanelTabSpawner::Register()
{
	PGX::Editor::RegisterNomadTab(
		TabId,
		FOnSpawnTab::CreateStatic(&FPGXCameraPanelTabSpawner::SpawnTab)
	)
	.SetDisplayName(LOCTEXT("CameraTab", "Camera"))
	.SetTooltipText(LOCTEXT("CameraTooltip", "Inspect PGX Camera: Config DataAssets + IPGXObservable schema + preview envelope."))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetGroup(FPGXWorkspaceMenu::GetSystemPanelsGroup())
	.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.CameraPanel"));
}

void FPGXCameraPanelTabSpawner::Unregister()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> FPGXCameraPanelTabSpawner::SpawnTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		[
			SNew(SPGXCameraPanel)
		];
}

#undef LOCTEXT_NAMESPACE
