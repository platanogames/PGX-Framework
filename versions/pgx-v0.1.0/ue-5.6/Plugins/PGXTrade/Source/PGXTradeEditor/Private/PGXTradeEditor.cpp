// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXTradeEditor.h"
#include "Inspector/SPGXTradeInspector.h"

void FPGXTradeEditorModule::StartupModule()
{
	// Development Preview config-time inspector (CDO/schema view). The live PIE inspector
	// (SPGXTradeInspectorTab) lives in PGXEditorTools alongside the other live
	// InspectorTabs (Save/Loading/GameFlow) — registered there, not here.
	SPGXTradeInspector::RegisterTabSpawner();
}

void FPGXTradeEditorModule::ShutdownModule()
{
	SPGXTradeInspector::UnregisterTabSpawner();
}

IMPLEMENT_MODULE(FPGXTradeEditorModule, PGXTradeEditor)
