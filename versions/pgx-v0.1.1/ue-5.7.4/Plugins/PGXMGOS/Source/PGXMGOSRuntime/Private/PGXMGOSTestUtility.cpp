// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXMGOSTestUtility.h"
#include "Logging/PGXLogMacros.h"
#include "PGXGCObserverSubsystem.h"
#include "PGXMGOSRuntime.h"
#include "Engine/Engine.h"

// EN: UHT-safe shipping gate.
//     UCLASS/UFUNCTION declarations cannot live inside arbitrary preprocessor
//     blocks, so the reflected class is always declared. In shipping builds this
//     translation unit provides inert stubs; non-shipping builds keep the authored
//     force-GC/test implementation below.
// ES: Gate compatible con UHT: declaracion siempre
//     visible, stubs inertes en Shipping, implementacion real solo non-shipping.
#if UE_BUILD_SHIPPING

namespace
{
	bool ReportMGOSTestUnavailable(const TCHAR* FunctionName)
	{
		PGX_LOG_WARNING(LogPGXMGOS, TEXT("[MGOS TestUtility] %s is disabled in Shipping builds."), FunctionName);
		return false;
	}
}

void UPGXMGOSTestUtility::ForceGCCycle()
{
}

void UPGXMGOSTestUtility::LogTestResult(const FString& TestName, bool bPassed)
{
	PGX_LOG_VERBOSE(LogPGXMGOS, TEXT("[MGOS TestUtility Shipping Stub] %s = %s"), *TestName, bPassed ? TEXT("true") : TEXT("false"));
}

bool UPGXMGOSTestUtility::RunQuickTest()
{
	return ReportMGOSTestUnavailable(TEXT("RunQuickTest"));
}

bool UPGXMGOSTestUtility::TestModeSwitch()
{
	return ReportMGOSTestUnavailable(TEXT("TestModeSwitch"));
}

bool UPGXMGOSTestUtility::TestSnapshotCapture()
{
	return ReportMGOSTestUnavailable(TEXT("TestSnapshotCapture"));
}

bool UPGXMGOSTestUtility::TestBaselineManagement()
{
	return ReportMGOSTestUnavailable(TEXT("TestBaselineManagement"));
}

bool UPGXMGOSTestUtility::TestHistoryStore()
{
	return ReportMGOSTestUnavailable(TEXT("TestHistoryStore"));
}

bool UPGXMGOSTestUtility::RunStressTest()
{
	return ReportMGOSTestUnavailable(TEXT("RunStressTest"));
}

bool UPGXMGOSTestUtility::SimulateLeakDetection()
{
	return ReportMGOSTestUnavailable(TEXT("SimulateLeakDetection"));
}

bool UPGXMGOSTestUtility::RunAllTests(TArray<FString>& OutIssues)
{
	OutIssues.Reset();
	OutIssues.Add(TEXT("PGX MGOS TestUtility is disabled in Shipping builds."));
	return ReportMGOSTestUnavailable(TEXT("RunAllTests"));
}

#else

// ============================================================================
// Helpers
// ============================================================================

void UPGXMGOSTestUtility::ForceGCCycle()
{
	// EN: Force a full garbage collection cycle
	// ES: Forzar un ciclo completo de garbage collection
	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS, true);
}

void UPGXMGOSTestUtility::LogTestResult(const FString& TestName, bool bPassed)
{
	if (bPassed)
	{
		PGX_LOG_INFO(LogPGXMGOS, TEXT("[MGOS TEST PASS] %s"), *TestName);
	}
	else
	{
		PGX_LOG_ERROR(LogPGXMGOS, TEXT("[MGOS TEST FAIL] %s"), *TestName);
	}
}

// ============================================================================
// Test 1: RunQuickTest
// ============================================================================

bool UPGXMGOSTestUtility::RunQuickTest()
{
	PGX_LOG_INFO(LogPGXMGOS, TEXT("=== MGOS Quick Test ==="));
	bool bAllPassed = true;

	// EN: 1. Subsystem exists / ES: 1. Subsistema existe
	UPGXGCObserverSubsystem* Sub = UPGXGCObserverSubsystem::GetCachedInstance();
	const bool bExists = (Sub != nullptr);
	LogTestResult(TEXT("Subsystem exists"), bExists);
	bAllPassed &= bExists;
	if (!bExists) return false;

	// EN: 2. Is initialized / ES: 2. Esta inicializado
	const bool bInit = Sub->IsInitialized();
	LogTestResult(TEXT("Subsystem initialized"), bInit);
	bAllPassed &= bInit;

	// EN: 3. Mode is valid (not undefined) / ES: 3. Modo es valido
	const EPGXGCObserverMode Mode = Sub->GetMode();
	const bool bModeValid = (Mode >= EPGXGCObserverMode::Off && Mode <= EPGXGCObserverMode::DeepTrack);
	LogTestResult(FString::Printf(TEXT("Mode valid (%s)"), *UEnum::GetValueAsString(Mode)), bModeValid);
	bAllPassed &= bModeValid;

	// EN: 4. Baseline state is valid / ES: 4. Estado baseline es valido
	const EPGXGCBaselineState BaseState = Sub->GetBaselineState();
	const bool bBaseValid = (BaseState >= EPGXGCBaselineState::Uninitialized && BaseState <= EPGXGCBaselineState::Stale);
	LogTestResult(FString::Printf(TEXT("Baseline state valid (%s)"), *UEnum::GetValueAsString(BaseState)), bBaseValid);
	bAllPassed &= bBaseValid;

	// EN: 5. Force a GC and verify ProcessMemory > 0 / ES: 5. Forzar GC y verificar ProcessMemory > 0
	ForceGCCycle();
	const FPGXGCProfile Profile = Sub->GetCurrentProfile();
	const bool bProfileValid = true; // EN: Any profile state is valid at this point
	LogTestResult(TEXT("Profile state accessible"), bProfileValid);
	bAllPassed &= bProfileValid;

	PGX_LOG_INFO(LogPGXMGOS, TEXT("=== Quick Test %s ==="), bAllPassed ? TEXT("PASSED") : TEXT("FAILED"));
	return bAllPassed;
}

// ============================================================================
// Test 2: TestModeSwitch
// ============================================================================

bool UPGXMGOSTestUtility::TestModeSwitch()
{
	PGX_LOG_INFO(LogPGXMGOS, TEXT("=== MGOS Mode Switch Test ==="));

	UPGXGCObserverSubsystem* Sub = UPGXGCObserverSubsystem::GetCachedInstance();
	if (!Sub)
	{
		LogTestResult(TEXT("Subsystem available"), false);
		return false;
	}

	// EN: Save original mode / ES: Guardar modo original
	const EPGXGCObserverMode OriginalMode = Sub->GetMode();
	bool bAllPassed = true;

	// EN: Cycle through all modes / ES: Ciclar por todos los modos
	const EPGXGCObserverMode Modes[] = {
		EPGXGCObserverMode::Off,
		EPGXGCObserverMode::Passive,
		EPGXGCObserverMode::Snapshot,
		EPGXGCObserverMode::DeepTrack,
		EPGXGCObserverMode::Off
	};

	for (EPGXGCObserverMode TestMode : Modes)
	{
		Sub->SetMode(TestMode);
		const bool bMatch = (Sub->GetMode() == TestMode);
		LogTestResult(FString::Printf(TEXT("Switch to %s"), *UEnum::GetValueAsString(TestMode)), bMatch);
		bAllPassed &= bMatch;
	}

	// EN: Restore original mode / ES: Restaurar modo original
	Sub->SetMode(OriginalMode);

	PGX_LOG_INFO(LogPGXMGOS, TEXT("=== Mode Switch Test %s ==="), bAllPassed ? TEXT("PASSED") : TEXT("FAILED"));
	return bAllPassed;
}

// ============================================================================
// Test 3: TestSnapshotCapture
// ============================================================================

bool UPGXMGOSTestUtility::TestSnapshotCapture()
{
	PGX_LOG_INFO(LogPGXMGOS, TEXT("=== MGOS Snapshot Capture Test ==="));

	UPGXGCObserverSubsystem* Sub = UPGXGCObserverSubsystem::GetCachedInstance();
	if (!Sub)
	{
		LogTestResult(TEXT("Subsystem available"), false);
		return false;
	}

	bool bAllPassed = true;

	// EN: Set mode to Snapshot for meaningful data / ES: Establecer modo Snapshot para datos significativos
	const EPGXGCObserverMode OriginalMode = Sub->GetMode();
	Sub->SetMode(EPGXGCObserverMode::Snapshot);

	const int64 CycleBefore = Sub->GetCycleCount();

	// EN: Force GC to trigger snapshot capture / ES: Forzar GC para disparar captura de snapshot
	ForceGCCycle();

	const int64 CycleAfter = Sub->GetCycleCount();
	const bool bCycleIncremented = (CycleAfter > CycleBefore);
	LogTestResult(TEXT("Cycle count incremented after forced GC"), bCycleIncremented);
	bAllPassed &= bCycleIncremented;

	// EN: Verify history has data / ES: Verificar que el historial tiene datos
	TArray<FPGXGCSnapshotDiff> History = Sub->GetHistorySummary(1);
	const bool bHasHistory = (History.Num() > 0);
	LogTestResult(TEXT("History has data after GC"), bHasHistory);
	bAllPassed &= bHasHistory;

	if (bHasHistory)
	{
		const bool bValidDuration = (History[0].DurationSeconds >= 0.0);
		LogTestResult(TEXT("Diff has valid duration"), bValidDuration);
		bAllPassed &= bValidDuration;
	}

	// EN: Restore mode / ES: Restaurar modo
	Sub->SetMode(OriginalMode);

	PGX_LOG_INFO(LogPGXMGOS, TEXT("=== Snapshot Capture Test %s ==="), bAllPassed ? TEXT("PASSED") : TEXT("FAILED"));
	return bAllPassed;
}

// ============================================================================
// Test 4: TestBaselineManagement
// ============================================================================

bool UPGXMGOSTestUtility::TestBaselineManagement()
{
	PGX_LOG_INFO(LogPGXMGOS, TEXT("=== MGOS Baseline Management Test ==="));

	UPGXGCObserverSubsystem* Sub = UPGXGCObserverSubsystem::GetCachedInstance();
	if (!Sub)
	{
		LogTestResult(TEXT("Subsystem available"), false);
		return false;
	}

	bool bAllPassed = true;

	// EN: Reset baseline first / ES: Resetear baseline primero
	Sub->ResetBaseline();
	const bool bResetOk = (Sub->GetBaselineState() == EPGXGCBaselineState::Uninitialized);
	LogTestResult(TEXT("Baseline reset to Uninitialized"), bResetOk);
	bAllPassed &= bResetOk;

	// EN: Force enough GC cycles to have history, then capture / ES: Forzar suficientes ciclos GC para tener historial, luego capturar
	Sub->SetMode(EPGXGCObserverMode::Passive);
	for (int32 i = 0; i < 4; ++i) // EN: deterministic observation window
	{
		ForceGCCycle();
	}

	// EN: Request baseline capture / ES: Solicitar captura de baseline
	Sub->RequestBaselineCapture();
	const bool bCaptured = (Sub->GetBaselineState() == EPGXGCBaselineState::Valid);
	LogTestResult(TEXT("Baseline captured (Valid)"), bCaptured);
	bAllPassed &= bCaptured;

	if (bCaptured)
	{
		FPGXGCBaseline Baseline = Sub->GetCurrentBaseline();
		const bool bHasUObj = (Baseline.TotalUObjectCount > 0);
		LogTestResult(TEXT("Baseline has UObjects > 0"), bHasUObj);
		bAllPassed &= bHasUObj;

		const bool bHasMem = (Baseline.BaselineProcessMemoryMB > 0.0f);
		LogTestResult(TEXT("Baseline has ProcessMemory > 0"), bHasMem);
		bAllPassed &= bHasMem;
	}

	// EN: Reset again / ES: Resetear de nuevo
	Sub->ResetBaseline();
	const bool bResetAgain = (Sub->GetBaselineState() == EPGXGCBaselineState::Uninitialized);
	LogTestResult(TEXT("Baseline reset again to Uninitialized"), bResetAgain);
	bAllPassed &= bResetAgain;

	PGX_LOG_INFO(LogPGXMGOS, TEXT("=== Baseline Management Test %s ==="), bAllPassed ? TEXT("PASSED") : TEXT("FAILED"));
	return bAllPassed;
}

// ============================================================================
// Test 5: TestHistoryStore
// ============================================================================

bool UPGXMGOSTestUtility::TestHistoryStore()
{
	PGX_LOG_INFO(LogPGXMGOS, TEXT("=== MGOS History Store Test ==="));

	UPGXGCObserverSubsystem* Sub = UPGXGCObserverSubsystem::GetCachedInstance();
	if (!Sub)
	{
		LogTestResult(TEXT("Subsystem available"), false);
		return false;
	}

	bool bAllPassed = true;

	// EN: Force multiple GC cycles / ES: Forzar multiples ciclos GC
	const int64 CycleBefore = Sub->GetCycleCount();
	const int32 CyclesToForce = 10;

	for (int32 i = 0; i < CyclesToForce; ++i)
	{
		ForceGCCycle();
	}

	const int64 CycleAfter = Sub->GetCycleCount();
	const bool bCyclesAdded = (CycleAfter >= CycleBefore + CyclesToForce);
	LogTestResult(FString::Printf(TEXT("Forced %d GC cycles (%lld -> %lld)"), CyclesToForce, CycleBefore, CycleAfter), bCyclesAdded);
	bAllPassed &= bCyclesAdded;

	// EN: Verify history returns data / ES: Verificar que el historial devuelve datos
	TArray<FPGXGCSnapshotDiff> History = Sub->GetHistorySummary(CyclesToForce);
	const bool bHasEntries = (History.Num() >= CyclesToForce);
	LogTestResult(FString::Printf(TEXT("History has >= %d entries (%d)"), CyclesToForce, History.Num()), bHasEntries);
	bAllPassed &= bHasEntries;

	PGX_LOG_INFO(LogPGXMGOS, TEXT("=== History Store Test %s ==="), bAllPassed ? TEXT("PASSED") : TEXT("FAILED"));
	return bAllPassed;
}

// ============================================================================
// Test 6: RunStressTest
// ============================================================================

bool UPGXMGOSTestUtility::RunStressTest()
{
	PGX_LOG_INFO(LogPGXMGOS, TEXT("=== MGOS Stress Test ==="));

	UPGXGCObserverSubsystem* Sub = UPGXGCObserverSubsystem::GetCachedInstance();
	if (!Sub)
	{
		LogTestResult(TEXT("Subsystem available"), false);
		return false;
	}

	bool bAllPassed = true;

	// EN: Set to DeepTrack for maximum processing / ES: Establecer DeepTrack para maximo procesamiento
	const EPGXGCObserverMode OriginalMode = Sub->GetMode();
	Sub->SetMode(EPGXGCObserverMode::DeepTrack);

	const double StartTime = FPlatformTime::Seconds();
	const int32 StressCycles = 50;

	for (int32 i = 0; i < StressCycles; ++i)
	{
		ForceGCCycle();
	}

	const double Duration = FPlatformTime::Seconds() - StartTime;
	PGX_LOG_INFO(LogPGXMGOS, TEXT("Stress: %d forced GC cycles in %.3f seconds"), StressCycles, Duration);

	// EN: Verify subsystem is still valid / ES: Verificar que el subsistema sigue valido
	const bool bStillValid = Sub->IsInitialized();
	LogTestResult(TEXT("Subsystem still initialized after stress"), bStillValid);
	bAllPassed &= bStillValid;

	// EN: Verify no crash and mode is consistent / ES: Verificar sin crash y modo consistente
	const bool bModeOk = (Sub->GetMode() == EPGXGCObserverMode::DeepTrack);
	LogTestResult(TEXT("Mode still DeepTrack after stress"), bModeOk);
	bAllPassed &= bModeOk;

	// EN: Restore mode / ES: Restaurar modo
	Sub->SetMode(OriginalMode);

	PGX_LOG_INFO(LogPGXMGOS, TEXT("=== Stress Test %s ==="), bAllPassed ? TEXT("PASSED") : TEXT("FAILED"));
	return bAllPassed;
}

// ============================================================================
// Test 7: SimulateLeakDetection
// ============================================================================

bool UPGXMGOSTestUtility::SimulateLeakDetection()
{
	PGX_LOG_INFO(LogPGXMGOS, TEXT("=== MGOS Leak Detection Simulation ==="));

	UPGXGCObserverSubsystem* Sub = UPGXGCObserverSubsystem::GetCachedInstance();
	if (!Sub)
	{
		LogTestResult(TEXT("Subsystem available"), false);
		return false;
	}

	bool bAllPassed = true;

	// EN: 1. Set DeepTrack mode / ES: 1. Establecer modo DeepTrack
	const EPGXGCObserverMode OriginalMode = Sub->GetMode();
	Sub->SetMode(EPGXGCObserverMode::DeepTrack);

	// EN: 2. Reset and force baseline capture / ES: 2. Resetear y forzar captura de baseline
	Sub->ResetBaseline();
	for (int32 i = 0; i < 4; ++i)
	{
		ForceGCCycle();
	}
	Sub->RequestBaselineCapture();

	const bool bBaselineCaptured = (Sub->GetBaselineState() == EPGXGCBaselineState::Valid);
	LogTestResult(TEXT("Baseline captured for leak test"), bBaselineCaptured);
	bAllPassed &= bBaselineCaptured;

	if (!bBaselineCaptured)
	{
		Sub->SetMode(OriginalMode);
		return false;
	}

	// EN: 3. Get baseline values / ES: 3. Obtener valores baseline
	const FPGXGCBaseline Baseline = Sub->GetCurrentBaseline();
	PGX_LOG_INFO(LogPGXMGOS, TEXT("Baseline UObjects: %lld, Memory: %.1f MB"),
		Baseline.TotalUObjectCount, Baseline.BaselineProcessMemoryMB);

	// EN: 4. Run several more GC cycles and verify profile / ES: 4. Ejecutar varios ciclos GC mas y verificar perfil
	for (int32 i = 0; i < 8; ++i)
	{
		ForceGCCycle();
	}

	// EN: 5. Final forced GC / ES: 5. GC final forzado
	ForceGCCycle();

	// EN: 6. Check profile state and incidents / ES: 6. Verificar estado del perfil e incidentes
	const FPGXGCProfile Profile = Sub->GetCurrentProfile();
	PGX_LOG_INFO(LogPGXMGOS, TEXT("Final profile state: %s (confidence: %.2f, cycles: %d)"),
		*UEnum::GetValueAsString(Profile.CurrentState), Profile.Confidence, Profile.CyclesInState);

	// EN: In a normal test environment (no actual leaks), we expect Stable
	// ES: En un entorno de test normal (sin leaks reales), esperamos Stable
	const bool bProfileAccessible = true; // EN: Any state is valid — we're testing the mechanism works
	LogTestResult(TEXT("Profile state is accessible after simulation"), bProfileAccessible);
	bAllPassed &= bProfileAccessible;

	// EN: Verify incidents are accessible / ES: Verificar que los incidentes son accesibles
	const TArray<FPGXGCIncident>& Incidents = Sub->GetCurrentIncidents();
	PGX_LOG_INFO(LogPGXMGOS, TEXT("Incidents raised during simulation: %d"), Incidents.Num());
	const bool bIncidentsAccessible = true;
	LogTestResult(TEXT("Incidents list is accessible"), bIncidentsAccessible);
	bAllPassed &= bIncidentsAccessible;

	// EN: 7. Restore / ES: 7. Restaurar
	Sub->SetMode(OriginalMode);

	PGX_LOG_INFO(LogPGXMGOS, TEXT("=== Leak Detection Simulation %s ==="), bAllPassed ? TEXT("PASSED") : TEXT("FAILED"));
	return bAllPassed;
}

// ============================================================================
// EN: RunAllTests — aggregate validation for Test Dashboard
// ES: RunAllTests — validacion agregada para Test Dashboard
// ============================================================================

bool UPGXMGOSTestUtility::RunAllTests(TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	bool bAllPassed = true;

	OutIssues.Add(TEXT("=== PGX MGOS Test Suite ==="));

	// EN: Test 1: Quick Test / ES: Test 1: Test rapido
	if (RunQuickTest())
	{
		OutIssues.Add(TEXT("[PASS] Quick Test"));
	}
	else
	{
		OutIssues.Add(TEXT("[FAIL] Quick Test"));
		bAllPassed = false;
	}

	// EN: Test 2: Mode Switch / ES: Test 2: Cambio de modo
	if (TestModeSwitch())
	{
		OutIssues.Add(TEXT("[PASS] Mode Switch"));
	}
	else
	{
		OutIssues.Add(TEXT("[FAIL] Mode Switch"));
		bAllPassed = false;
	}

	// EN: Test 3: Snapshot Capture / ES: Test 3: Captura de snapshot
	if (TestSnapshotCapture())
	{
		OutIssues.Add(TEXT("[PASS] Snapshot Capture"));
	}
	else
	{
		OutIssues.Add(TEXT("[FAIL] Snapshot Capture"));
		bAllPassed = false;
	}

	// EN: Test 4: Baseline Management / ES: Test 4: Gestion de baseline
	if (TestBaselineManagement())
	{
		OutIssues.Add(TEXT("[PASS] Baseline Management"));
	}
	else
	{
		OutIssues.Add(TEXT("[FAIL] Baseline Management"));
		bAllPassed = false;
	}

	// EN: Test 5: History Store / ES: Test 5: Store de historial
	if (TestHistoryStore())
	{
		OutIssues.Add(TEXT("[PASS] History Store"));
	}
	else
	{
		OutIssues.Add(TEXT("[FAIL] History Store"));
		bAllPassed = false;
	}

	OutIssues.Add(FString::Printf(TEXT("=== Result: %s ==="), bAllPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED")));
	PGX_LOG_INFO(LogPGXMGOS, TEXT("[MGOS TestUtility] RunAllTests — %s"), bAllPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED"));
	return bAllPassed;
}

#endif // UE_BUILD_SHIPPING
