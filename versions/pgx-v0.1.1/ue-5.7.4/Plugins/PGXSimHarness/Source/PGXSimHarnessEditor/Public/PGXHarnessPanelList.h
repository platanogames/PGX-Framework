// Copyright PGX Framework. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

// EN: Canonical list of harness panel tab-ids. Lives in PGXSimHarnessEditor (not PGXCoreEditor)
//     because the ONLY consumer is the harness itself — placing it in the shared base would push
//     a harness-specific concern into PGXCore with no benefit. Promote to PGXCoreEditor only when
//     a second consumer (Hub, tooling) needs it; until then it stays local. Direction of
//     dependency (harness -> PGXCoreEditor) is preserved either way; no plugin depends on this.
// ES: Lista canonica de tab-ids de paneles del harness. Vive en PGXSimHarnessEditor (no en
//     PGXCoreEditor) porque el UNICO consumidor es el propio harness — ponerla en la base comun
//     metaria un concern del harness en PGXCore sin beneficio. Promover a PGXCoreEditor solo
//     cuando un segundo consumidor (Hub, tooling) la necesite; hasta entonces queda local. La
//     direccion de dependencia (harness -> PGXCoreEditor) se preserva igual; ningun plugin
//     depende de esta.

namespace PGX::Harness
{
	// EN: Panels that display harness-injected PIE data (the launcher set).
	// ES: Paneles que muestran datos PIE inyectados por el harness (el conjunto del launcher).
	PGXSIMHARNESSEDITOR_API const TArray<FName>& GetHarnessDataPanelIds();

	// EN: Panels that do NOT display harness data — covered by separate unit tests.
	// ES: Paneles que NO muestran datos del harness — cubiertos por pruebas unitarias aparte.
	PGXSIMHARNESSEDITOR_API const TArray<FName>& GetHarnessToolingPanelIds();
}
