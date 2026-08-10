// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Inspector/SPGXPSOPanel.h"
#include "PGXPSOSettings.h"
#include "PGXPSOWarmUpConfig.h"
#include "PGXPSOSubsystem.h"
#include "Style/PGXEditorStyle.h"
#include "Style/PGXVisualTokens.h"
#include "Widgets/SPGXPanelHeader.h"
#include "Workspace/PGXWorkspaceMenu.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXPSOPanel"

const FName SPGXPSOPanel::TabId(TEXT("PGXPSOPanel"));

void SPGXPSOPanel::Construct(const FArguments& /*InArgs*/)
{
	BuildInspectorLayout();
}

void SPGXPSOPanel::RegisterTabSpawner()
{
	PGX::Editor::RegisterNomadTab(TabId, FOnSpawnTab::CreateStatic(&SPGXPSOPanel::SpawnTab))
		.SetDisplayName(LOCTEXT("TabTitle", "PSO"))
		.SetTooltipText(LOCTEXT("TabTooltip", "PGXPSO observability — warm-up state, discovered configs, recording"))
		.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.PSOPanel"))
		.SetGroup(FPGXWorkspaceMenu::GetSystemPanelsGroup());
}

void SPGXPSOPanel::UnregisterTabSpawner()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> SPGXPSOPanel::SpawnTab(const FSpawnTabArgs& /*Args*/)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SPGXPSOPanel)
		];
}

TArray<TPair<FText, UClass*>> SPGXPSOPanel::GetObservableClasses() const
{
	return {
		{ LOCTEXT("PSOSettingsLabel",       "PSO Settings"),               UPGXPSOSettings::StaticClass() },
		{ LOCTEXT("PSOWarmUpConfigLabel",   "PSO Warm-Up Config (CDO)"),   UPGXPSOWarmUpConfig::StaticClass() },
		{ LOCTEXT("PSOSubsystemLabel",      "PSO Subsystem (CDO)"),        UPGXPSOSubsystem::StaticClass() },
	};
}

TArray<TPair<FText, FText>> SPGXPSOPanel::GetDeferredCards() const
{
	return {
		{ LOCTEXT("PSODeferred0Label", "Live warm-up state unavailable"),
		  LOCTEXT("PSODeferred0Detail", "Live warm-up state machine snapshot, GetWarmUpProgress reactive (SProgressBar bound), 6 dynamic + 4 native delegates subscription (OnWarmUpBegin/Progress/Complete/Failed/StateChanged), per-discovered-config SListView and recording session controls require a world-backed inspector data pass with native delegate subscribers; the current read-only panel exposes CDO schema status only.") },
		{ LOCTEXT("PSODeferred1Label", "Editor actions unavailable"),
		  LOCTEXT("PSODeferred1Detail", "This read-only panel does not expose recording controls, asset creation actions, toolbar integration, or runtime mutations.") },
	};
}

TSharedRef<SWidget> SPGXPSOPanel::BuildHeader() const
{
	return SNew(SPGXPanelHeader)
		.SystemColor(PGX::System::PSO)
		.Title(LOCTEXT("HeaderTitle", "PGXPSO Panel"))
		.Subtitle(LOCTEXT("HeaderSubtitle", "Read-only config and schema overview"));
}

#undef LOCTEXT_NAMESPACE
