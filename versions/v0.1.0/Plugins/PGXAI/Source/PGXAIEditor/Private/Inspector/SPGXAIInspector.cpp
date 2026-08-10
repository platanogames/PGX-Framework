// Copyright PGX Framework. All Rights Reserved.

#include "Inspector/SPGXAIInspector.h"
#include "PGXAIConfig.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXAIInspector"

const FName SPGXAIInspector::TabId(TEXT("PGXAIInspector"));

void SPGXAIInspector::Construct(const FArguments& /*InArgs*/)
{
	BuildInspectorLayout();
}

void SPGXAIInspector::RegisterTabSpawner()
{
	PGX::Editor::RegisterNomadTab(TabId, FOnSpawnTab::CreateStatic(&SPGXAIInspector::SpawnTab))
		.SetDisplayName(LOCTEXT("TabTitle", "AI Inspector"))
		.SetTooltipText(LOCTEXT("TabTooltip", "Read-only Development Preview PGXAI inspector"));
}

void SPGXAIInspector::UnregisterTabSpawner()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> SPGXAIInspector::SpawnTab(const FSpawnTabArgs& /*Args*/)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SPGXAIInspector)
		];
}

FText SPGXAIInspector::GetInspectorTitle() const
{
	return LOCTEXT("Header", "PGXAI Development Preview Inspector");
}

TArray<TPair<FText, UClass*>> SPGXAIInspector::GetObservableClasses() const
{
	return {
		{ LOCTEXT("PGXAI0Label", "AI Config"), UPGXAIConfig::StaticClass() },
	};
}

TArray<TPair<FText, FText>> SPGXAIInspector::GetDeferredCards() const
{
	return {
		{ LOCTEXT("PGXAIDeferred0Label", "Not included"),
		  LOCTEXT("PGXAIDeferred0Detail", "Agent registry and BehaviorTree runtime snapshots require a world-backed inspector data pass; this Development Preview tab exposes safe config/schema status only.") },
		{ LOCTEXT("PGXAIDeferred1Label", "Not included"),
		  LOCTEXT("PGXAIDeferred1Detail", "Asset creation shortcuts and live runtime diagnostics are not included in this preview.") },
	};
}

#undef LOCTEXT_NAMESPACE
