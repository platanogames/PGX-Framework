// Copyright PGX Framework. All Rights Reserved.

#include "Inspector/SPGXTradeInspector.h"
#include "PGXTradeConfig.h"
#include "PGXTradeSettings.h"
#include "PGXTradeSubsystem.h"
#include "Style/PGXEditorStyle.h"
#include "Style/PGXVisualTokens.h"
#include "Widgets/SPGXPanelHeader.h"
#include "Workspace/PGXWorkspaceMenu.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXTradeInspector"

const FName SPGXTradeInspector::TabId(TEXT("PGXTradeInspector"));

void SPGXTradeInspector::Construct(const FArguments& /*InArgs*/)
{
	BuildInspectorLayout();
}

void SPGXTradeInspector::RegisterTabSpawner()
{
	PGX::Editor::RegisterNomadTab(TabId, FOnSpawnTab::CreateStatic(&SPGXTradeInspector::SpawnTab))
		.SetDisplayName(LOCTEXT("TabTitle", "Trade"))
		.SetTooltipText(LOCTEXT("TabTooltip", "PGXTrade observability — actors, offers, transactions, reputation"))
		.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.TradePanel"))
		.SetGroup(FPGXWorkspaceMenu::GetInspectorsGroup());
}

void SPGXTradeInspector::UnregisterTabSpawner()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> SPGXTradeInspector::SpawnTab(const FSpawnTabArgs& /*Args*/)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SPGXTradeInspector)
		];
}

TArray<TPair<FText, UClass*>> SPGXTradeInspector::GetObservableClasses() const
{
	return {
		{ LOCTEXT("PGXTrade0Label", "Trade Config"),             UPGXTradeConfig::StaticClass() },
		{ LOCTEXT("PGXTrade1Label", "Trade Settings"),           UPGXTradeSettings::StaticClass() },
		{ LOCTEXT("PGXTrade2Label", "Trade Subsystem (CDO)"),    UPGXTradeSubsystem::StaticClass() },
	};
}

TArray<TPair<FText, FText>> SPGXTradeInspector::GetDeferredCards() const
{
	return {
		{ LOCTEXT("PGXTradeDeferred0Label", "Not included"),
		  LOCTEXT("PGXTradeDeferred0Detail", "Live offer, transaction, reputation and information views require a world-backed inspector and are not included in this preview.") },
		{ LOCTEXT("PGXTradeDeferred1Label", "Not included"),
		  LOCTEXT("PGXTradeDeferred1Detail", "Asset creation shortcuts and data export actions are not included in this preview.") },
	};
}

TSharedRef<SWidget> SPGXTradeInspector::BuildHeader() const
{
	return SNew(SPGXPanelHeader)
		.SystemColor(PGX::System::Trade)
		.Title(LOCTEXT("HeaderTitle", "PGXTrade Panel"))
		.Subtitle(LOCTEXT("HeaderSubtitle", "L1 — actors / offers / transactions / reputation"));
}

#undef LOCTEXT_NAMESPACE
