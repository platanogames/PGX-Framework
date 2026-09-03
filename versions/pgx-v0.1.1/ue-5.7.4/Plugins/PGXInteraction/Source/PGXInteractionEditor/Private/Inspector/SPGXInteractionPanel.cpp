// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Inspector/SPGXInteractionPanel.h"
#include "PGXInteractable.h"
#include "PGXInteractionComponent.h"
#include "PGXInteractionCondition.h"
#include "PGXInteractionPrompt.h"
#include "Style/PGXEditorStyle.h"
#include "Style/PGXVisualTokens.h"
#include "Widgets/SPGXPanelHeader.h"
#include "Workspace/PGXWorkspaceMenu.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXInteractionPanel"

const FName SPGXInteractionPanel::TabId(TEXT("PGXInteractionPanel"));

void SPGXInteractionPanel::Construct(const FArguments& /*InArgs*/)
{
	BuildInspectorLayout();
}

void SPGXInteractionPanel::RegisterTabSpawner()
{
	PGX::Editor::RegisterNomadTab(TabId, FOnSpawnTab::CreateStatic(&SPGXInteractionPanel::SpawnTab))
		.SetDisplayName(LOCTEXT("TabTitle", "Interaction"))
		.SetTooltipText(LOCTEXT("TabTooltip", "PGXInteraction observability — interactables, conditions, prompts"))
		.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.InteractionPanel"))
		.SetGroup(FPGXWorkspaceMenu::GetSystemPanelsGroup());
}

void SPGXInteractionPanel::UnregisterTabSpawner()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> SPGXInteractionPanel::SpawnTab(const FSpawnTabArgs& /*Args*/)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SPGXInteractionPanel)
		];
}

TArray<TPair<FText, UClass*>> SPGXInteractionPanel::GetObservableClasses() const
{
	return {
		{ LOCTEXT("InteractionComponentLabel", "Interaction Component"), UPGXInteractionComponent::StaticClass() },
		{ LOCTEXT("InteractionConditionLabel", "Interaction Condition"), UPGXInteractionCondition::StaticClass() },
		{ LOCTEXT("InteractionPromptLabel",    "Interaction Prompt"),    UPGXInteractionPrompt::StaticClass() },
	};
}

TArray<TPair<FText, FText>> SPGXInteractionPanel::GetDeferredCards() const
{
	return {
		{ LOCTEXT("InteractionDeferred0Label", "Not included"),
		  LOCTEXT("InteractionDeferred0Detail", "Live per-actor interaction telemetry (active prompts, condition evaluation traces, OnInteractionBegan/Ended event stream) requires a world-backed inspector data pass; this read-only preview exposes CDO schema status only.") },
		{ LOCTEXT("InteractionDeferred1Label", "Not included"),
		  LOCTEXT("InteractionDeferred1Detail", "Settings/Config DataAsset integration, AssetTypeActions, factory menu entries, toolbar pin, Hub card routing, and SystemObserver entry are not included in this inspector.") },
	};
}

TSharedRef<SWidget> SPGXInteractionPanel::BuildHeader() const
{
	return SNew(SPGXPanelHeader)
		.SystemColor(PGX::System::Interaction)
		.Title(LOCTEXT("HeaderTitle", "PGXInteraction Panel"))
		.Subtitle(LOCTEXT("HeaderSubtitle", "read-only — interactables / conditions / prompts"));
}

#undef LOCTEXT_NAMESPACE
