// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Inspector/SPGXMessageInspectorTab.h"

#include "Layout/Geometry.h"
#include "HAL/PlatformTime.h"
#include "Templates/SharedPointer.h"
#include "Widgets/SNullWidget.h"

/**
 * EN: Smoke tests for the Inspector Slate widgets. They verify that:
 *     - Widget can be constructed under empty-subsystem (no PIE) without crash.
 *     - Tick can be called on the empty-state widget without crash.
 *     - Destructor releases PIE delegate handles cleanly.
 *
 *     Uses a file-level WITH_DEV_AUTOMATION_TESTS guard and
 *     IMPLEMENT_SIMPLE_AUTOMATION_TEST. No PGX_TEST dependency on
 *     PGXCoreDeveloper, so PGXCoreEditor.Build.cs requires no mutation.
 *     Naming convention: `PGX.Editor.Inspector.<Tab>.<Test>`.
 *
 *     These are intentionally narrow smoke tests. Richer behavioral tests
 *     (binding under PIE, lifecycle event capture, panel rendering with real
 *     subsystem state) require a transient PIE world fixture and are deferred.
 *
 * ES: Smoke tests para los widgets Slate del Inspector. Verifican:
 *     - El widget puede construirse bajo empty-subsystem (sin PIE) sin crash.
 *     - Tick se puede llamar sobre el widget en empty-state sin crash.
 *     - El destructor libera los handles de delegate PIE limpiamente.
 *
 *     Usa guard file-level WITH_DEV_AUTOMATION_TESTS junto con
 *     IMPLEMENT_SIMPLE_AUTOMATION_TEST. Sin
 *     dependencia PGX_TEST en PGXCoreDeveloper, asi que PGXCoreEditor.Build.cs
 *     no requiere mutation. Convencion de naming:
 *     `PGX.Editor.Inspector.<Tab>.<Test>`.
 *
 *     Son intencionalmente smoke tests estrechos. Tests behavioral mas ricos
 *     (binding bajo PIE, captura de eventos lifecycle, render de paneles con
 *     estado real de subsistema) requieren un fixture transitorio de PIE world
 *     y se difieren.
 */

namespace
{
	// EN: Tick a widget a few times under a synthetic geometry to exercise
	//     empty-state pollings without requiring a window. Returns the number
	//     of ticks invoked successfully (no exception thrown).
	// ES: Tick a un widget unas pocas veces bajo una geometry sintetica para
	//     ejercitar el polling de empty-state sin requerir una ventana.
	//     Retorna el numero de ticks invocados exitosamente (sin exception).
	template <typename WidgetType>
	int32 TickWidgetTimes(const TSharedRef<WidgetType>& Widget, int32 NumTicks)
	{
		const FGeometry Geometry; // EN: synthetic / ES: sintetica
		int32 Count = 0;
		const double BaseTime = FPlatformTime::Seconds();
		for (int32 Idx = 0; Idx < NumTicks; ++Idx)
		{
			// EN: Advance time by 1.5 s per tick so the 1 Hz refresh gate
			//     fires every iteration (workaround for 1 Hz cadence).
			// ES: Avanzar tiempo 1.5 s por tick para que el gate de refresh
			//     1 Hz dispare en cada iteracion (workaround para cadencia 1 Hz).
			Widget->Tick(Geometry, BaseTime + 1.5 * (Idx + 1), 1.5f);
			++Count;
		}
		return Count;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXEditorInspectorMessageTabConstructAndDestructDoesNotCrash,
	"PGX.Editor.Inspector.MessageTab.ConstructAndDestructDoesNotCrash",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXEditorInspectorMessageTabConstructAndDestructDoesNotCrash::RunTest(
	const FString& /*Parameters*/)
{
	{
		TSharedRef<SPGXMessageInspectorTab> Widget = SNew(SPGXMessageInspectorTab);

		const int32 Ticks = TickWidgetTimes(Widget, 4);
		TestEqual(
			TEXT("Message inspector ticked 4 times without crash"),
			Ticks, 4);
	}

	// EN: Widget is destructed when the local SharedRef goes out of scope.
	//     Reaching here means destructor finished without crash.
	// ES: El widget se destruye cuando el SharedRef local sale de scope.
	//     Llegar aqui significa que el destructor termino sin crash.
	AddInfo(TEXT("SPGXMessageInspectorTab destructor completed without crash"));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
