// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXEditorToolsEditor.h"
#include "Logging/PGXLogMacros.h"
#include "Dashboard/FPGXHubTabSpawner.h"
#include "LogViewer/FPGXLogViewerTabSpawner.h"
#include "SaveInspector/FPGXSaveInspectorTabSpawner.h"
#include "GameFlowInspector/FPGXGameFlowInspectorTabSpawner.h"
#include "PSOTools/FPGXPSOAutoPopulatorTabSpawner.h"
#include "PSOInspector/FPGXPSOInspectorTabSpawner.h"
#include "LevelFlowInspector/FPGXLevelFlowInspectorTabSpawner.h"
#include "LoadingInspector/FPGXLoadingInspectorTabSpawner.h"
#include "ProfileInspector/FPGXProfileInspectorTabSpawner.h"
#include "MGOSInspector/FPGXMGOSInspectorTabSpawner.h"
#include "Observer/FPGXSystemObserverTabSpawner.h"
#include "MessageInspector/FPGXMessageInspectorTabSpawner.h"
#include "EventDebugger/FPGXEventDebuggerTabSpawner.h"
#include "PlatformHealth/FPGXPlatformHealthTabSpawner.h"
#include "Browser/FPGXDataRegistryBrowserTabSpawner.h"
#include "Browser/FPGXConfigDashboardTabSpawner.h"
#include "TestDashboard/FPGXTestDashboardTabSpawner.h"
#include "Showcase/FPGXVisualShowcaseTabSpawner.h"

DEFINE_LOG_CATEGORY(LogPGXEditorTools);

#define LOCTEXT_NAMESPACE "FPGXEditorToolsEditorModule"

void FPGXEditorToolsEditorModule::StartupModule()
{
	PGX_LOG_INFO(LogPGXEditorTools, TEXT("PGXEditorToolsEditor: Module starting..."));

	// Register PGX Hub tab
	FPGXHubTabSpawner::Register();

	// Register PGX Log Viewer tab
	FPGXLogViewerTabSpawner::Register();

	// Register PGX Save Inspector tab
	FPGXSaveInspectorTabSpawner::Register();

	// Register PGX GameFlow Inspector tab
	FPGXGameFlowInspectorTabSpawner::Register();

	// Register PGX PSO Auto-Populator tab
	FPGXPSOAutoPopulatorTabSpawner::Register();

	// Register PGX PSO Inspector tab
	FPGXPSOInspectorTabSpawner::Register();

	// Register PGX LevelFlow Inspector tab
	FPGXLevelFlowInspectorTabSpawner::Register();

	// Register PGX Loading Inspector tab
	FPGXLoadingInspectorTabSpawner::Register();

	// Register PGX Profile Inspector tab (BEFORE MGOS and System Observer)
	FPGXProfileInspectorTabSpawner::Register();

	// Register PGX MGOS Inspector tab (position 10, BEFORE System Observer)
	FPGXMGOSInspectorTabSpawner::Register();

	// Register PGX Data Registry Browser tab
	FPGXDataRegistryBrowserTabSpawner::Register();

	// Register PGX Config Dashboard tab
	FPGXConfigDashboardTabSpawner::Register();

	// Register PGX Test Dashboard tab
	FPGXTestDashboardTabSpawner::Register();

	// Register PGX Message Inspector tab
	FPGXMessageInspectorTabSpawner::Register();

	// Register PGX Event Debugger tab
	FPGXEventDebuggerTabSpawner::Register();

	// Register PGX Platform Health Dashboard tab
	FPGXPlatformHealthTabSpawner::Register();

	// Register PGX Visual Showcase tab (prototype)
	FPGXVisualShowcaseTabSpawner::Register();

	// Register PGX System Observer tab (always last)
	FPGXSystemObserverTabSpawner::Register();

	PGX_LOG_INFO(LogPGXEditorTools, TEXT("PGXEditorToolsEditor: Module started - PGX Hub + Log Viewer + Save Inspector + GameFlow Inspector + PSO Auto-Populator + PSO Inspector + LevelFlow Inspector + Loading Inspector + Profile Inspector + MGOS Inspector + Data Registry Browser + Config Dashboard + Test Dashboard + Message Inspector + Event Debugger + Platform Health + System Observer registered"));
}

void FPGXEditorToolsEditorModule::ShutdownModule()
{
	// EN: Unregister in LIFO order / ES: Desregistrar en orden LIFO
	FPGXSystemObserverTabSpawner::Unregister();

	// Unregister PGX Visual Showcase tab
	FPGXVisualShowcaseTabSpawner::Unregister();

	// Unregister PGX Platform Health Dashboard tab
	FPGXPlatformHealthTabSpawner::Unregister();

	// Unregister PGX Event Debugger tab
	FPGXEventDebuggerTabSpawner::Unregister();

	// Unregister PGX Message Inspector tab
	FPGXMessageInspectorTabSpawner::Unregister();

	// Unregister PGX Test Dashboard tab
	FPGXTestDashboardTabSpawner::Unregister();

	// Unregister PGX Config Dashboard tab
	FPGXConfigDashboardTabSpawner::Unregister();

	// Unregister PGX Data Registry Browser tab
	FPGXDataRegistryBrowserTabSpawner::Unregister();

	// Unregister PGX MGOS Inspector tab
	FPGXMGOSInspectorTabSpawner::Unregister();

	// Unregister PGX Profile Inspector tab
	FPGXProfileInspectorTabSpawner::Unregister();

	// Unregister PGX Loading Inspector tab
	FPGXLoadingInspectorTabSpawner::Unregister();

	// Unregister PGX LevelFlow Inspector tab
	FPGXLevelFlowInspectorTabSpawner::Unregister();

	// Unregister PGX PSO Inspector tab
	FPGXPSOInspectorTabSpawner::Unregister();

	// Unregister PGX PSO Auto-Populator tab
	FPGXPSOAutoPopulatorTabSpawner::Unregister();

	// Unregister PGX GameFlow Inspector tab
	FPGXGameFlowInspectorTabSpawner::Unregister();

	// Unregister PGX Save Inspector tab
	FPGXSaveInspectorTabSpawner::Unregister();

	// Unregister PGX Log Viewer tab
	FPGXLogViewerTabSpawner::Unregister();

	// Unregister PGX Hub tab
	FPGXHubTabSpawner::Unregister();

	PGX_LOG_INFO(LogPGXEditorTools, TEXT("PGXEditorToolsEditor: Module shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXEditorToolsEditorModule, PGXEditorToolsEditor)
