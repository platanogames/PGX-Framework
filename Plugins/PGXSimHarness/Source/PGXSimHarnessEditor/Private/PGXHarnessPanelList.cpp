// Copyright PGX Framework. All Rights Reserved.

#include "PGXHarnessPanelList.h"

namespace PGX::Harness
{
	// EN: Canonical tab-ids — these MUST match the FName each panel passes to
	//     PGX::Editor::RegisterNomadTab. When a divergence was found vs the old display-name map,
	//     the registered spawner id is the authority (verified spawner-by-spawner):
	//     PGXLogViewer (not PGXLogInspector), PGXDataRegistryBrowser (not PGXRegistryBrowser),
	//     PGXPlatformHealthDashboard (not PGXPlatformHealth), PGXDocs (not PGXDocsViewer).
	// ES: Tab-ids canonicos — DEBEN coincidir con el FName que cada panel pasa a
	//     PGX::Editor::RegisterNomadTab. Ante divergencia vs el viejo map de display-name, el id
	//     del spawner registrado es la autoridad (verificado spawner por spawner).
	const TArray<FName>& GetHarnessDataPanelIds()
	{
		static const TArray<FName> DataPanels = {
			FName("PGXHub"),
			FName("PGXProfileInspector"),
			FName("PGXPlatformHealthDashboard"),
			FName("PGXGameFlowInspector"),
			FName("PGXLogViewer"),
			FName("PGXSaveInspector"),
			FName("PGXPSOInspector"),
			FName("PGXMGOSInspector"),
			FName("PGXDataRegistryBrowser"),
			FName("PGXConfigDashboard"),
			FName("PGXMessageInspector"),
			FName("PGXEventDebugger"),
			FName("PGXLevelFlowInspector"),
			FName("PGXLoadingInspector"),
			FName("PGXSystemObserver"),
		};
		return DataPanels;
	}

	const TArray<FName>& GetHarnessToolingPanelIds()
	{
		static const TArray<FName> ToolingPanels = {
			FName("PGXPSOAutoPopulator"),
			FName("PGXTestDashboard"),
			FName("PGXDocs"),
			FName("PGXVersionControlInspector"),
		};
		return ToolingPanels;
	}
}
