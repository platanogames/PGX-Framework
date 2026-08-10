// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXDocsEditorModule.h"
#include "Logging/PGXLogMacros.h"
#include "PGXDocsTabSpawner.h"
#include "PGXDocsModule.h"
#include "PGXDocSystem.h"
#include "PGXDocsSettings.h"
#include "Render/FDocWatcher.h"

#define LOCTEXT_NAMESPACE "FPGXDocsEditorModule"

static TUniquePtr<FDocWatcher> GDocWatcher;

void FPGXDocsEditorModule::StartupModule()
{
	PGX_LOG_INFO(LogPGXDocs, TEXT("PGXDocsEditor: Module starting..."));

	// EN: Register the Documentation Viewer NomadTab / ES: Registrar el NomadTab del visor de documentacion
	FPGXDocsTabSpawner::Register();

	// EN: Initialize the documentation system (lazy — will scan on first tab open)
	// ES: Inicializar el sistema de documentacion (lazy — escaneara al abrir el primer tab)

	// EN: Start file watcher if live reload enabled / ES: Iniciar file watcher si live reload esta habilitado
	const UPGXDocsSettings* Settings = UPGXDocsSettings::Get();
	if (Settings && Settings->bEnableLiveReload)
	{
		// EN: Initialize doc system so registry has the root paths, then start watcher
		// ES: Inicializar sistema de docs para que el registry tenga las rutas raiz, luego iniciar watcher
		FDocSystem::Get().Initialize();
		GDocWatcher = MakeUnique<FDocWatcher>();
		GDocWatcher->Start(FDocSystem::Get().GetRegistry().GetDocRootPaths());
	}

	PGX_LOG_INFO(LogPGXDocs, TEXT("PGXDocsEditor: Module started — Documentation tab registered"));
}

void FPGXDocsEditorModule::ShutdownModule()
{
	// EN: Stop file watcher / ES: Detener file watcher
	if (GDocWatcher.IsValid())
	{
		GDocWatcher->Stop();
		GDocWatcher.Reset();
	}

	// EN: Shutdown doc system / ES: Apagar sistema de docs
	FDocSystem::Get().Shutdown();

	// EN: Unregister the Documentation Viewer NomadTab / ES: Desregistrar el NomadTab del visor de documentacion
	FPGXDocsTabSpawner::Unregister();

	PGX_LOG_INFO(LogPGXDocs, TEXT("PGXDocsEditor: Module shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXDocsEditorModule, PGXDocsEditor)
