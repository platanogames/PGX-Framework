// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXScaffoldEditor.h"
#include "Logging/PGXLogMacros.h"
#include "Style/PGXEditorStyle.h"
#include "Workspace/PGXWorkspaceMenu.h"
#include "UI/SPGXScaffoldTab.h"
#include "Core/PGXScaffoldTemplateRegistry.h"
#include "Templates/PGXBuiltInTemplates.h"

#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Utils/PGXTabRegistration.h"

DEFINE_LOG_CATEGORY(LogPGXScaffold);

#define LOCTEXT_NAMESPACE "PGXScaffold"

static const FName ScaffoldTabId(TEXT("PGXScaffoldPanel"));

void FPGXScaffoldEditorModule::StartupModule()
{
	PGX_LOG_INFO(LogPGXScaffold, TEXT("PGXScaffoldEditor: Module starting..."));

	// EN: Register the Scaffold Panel NomadTab
	// ES: Registrar el NomadTab del Panel de Scaffold
	PGX::Editor::RegisterNomadTab(
		ScaffoldTabId,
		FOnSpawnTab::CreateLambda([](const FSpawnTabArgs&) -> TSharedRef<SDockTab>
		{
			return SNew(SDockTab)
				.TabRole(NomadTab)
				[
					SNew(SPGXScaffoldTab)
				];
		})
	)
	.SetDisplayName(LOCTEXT("ScaffoldTab", "Scaffold"))
	.SetTooltipText(LOCTEXT("ScaffoldTooltip", "PGX Scaffold — Automated project scaffolding"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetGroup(FPGXWorkspaceMenu::GetToolsGroup())
	// Use the dedicated panel brush; the general scaffold brush remains available to other consumers.
	.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.ScaffoldPanel"));

	// EN: Register built-in templates / ES: Registrar templates built-in
	FPGXBuiltInTemplates::RegisterAll(FPGXScaffoldTemplateRegistry::Get());

	PGX_LOG_INFO(LogPGXScaffold, TEXT("PGXScaffoldEditor: Module started — Scaffold Panel + 4 templates registered"));
}

void FPGXScaffoldEditorModule::ShutdownModule()
{
	PGX::Editor::UnregisterNomadTab(ScaffoldTabId);

	PGX_LOG_INFO(LogPGXScaffold, TEXT("PGXScaffoldEditor: Module shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXScaffoldEditorModule, PGXScaffoldEditor)
