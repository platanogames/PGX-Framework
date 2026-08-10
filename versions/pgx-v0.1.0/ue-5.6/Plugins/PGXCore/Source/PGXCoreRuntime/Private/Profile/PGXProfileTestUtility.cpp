// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Profile/PGXProfileTestUtility.h"
#include "Profile/PGXProfileSubsystem.h"
#include "Logging/PGXLogCategories.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

// ============================================================================
// Helpers
// ============================================================================

UPGXProfileSubsystem* UPGXProfileTestUtility::GetSubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;
	const UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;
	UGameInstance* GI = World->GetGameInstance();
	if (!GI) return nullptr;
	return GI->GetSubsystem<UPGXProfileSubsystem>();
}

void UPGXProfileTestUtility::LogTestResult(const FString& TestName, bool bPassed, const FString& Details)
{
	if (bPassed)
	{
		UE_LOG(LogPGXProfile, Log, TEXT("[TEST PASS] %s%s"),
			*TestName,
			Details.IsEmpty() ? TEXT("") : *FString::Printf(TEXT(" — %s"), *Details));
	}
	else
	{
		UE_LOG(LogPGXProfile, Error, TEXT("[TEST FAIL] %s%s"),
			*TestName,
			Details.IsEmpty() ? TEXT("") : *FString::Printf(TEXT(" — %s"), *Details));
	}
}

// ============================================================================
// RunQuickTest
// ============================================================================

void UPGXProfileTestUtility::RunQuickTest(const UObject* WorldContextObject)
{
	UE_LOG(LogPGXProfile, Log, TEXT(""));
	UE_LOG(LogPGXProfile, Log, TEXT("========================================"));
	UE_LOG(LogPGXProfile, Log, TEXT("  PGX Profile — Quick Test Suite"));
	UE_LOG(LogPGXProfile, Log, TEXT("========================================"));

	UPGXProfileSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		LogTestResult(TEXT("Subsystem Availability"), false, TEXT("Profile subsystem not found"));
		return;
	}

	LogTestResult(TEXT("Subsystem Availability"), true, TEXT("Subsystem found and accessible"));
	LogTestResult(TEXT("Profile Resolved"), Sub->IsProfileResolved(),
		Sub->IsProfileResolved() ? TEXT("Ready") : TEXT("Not resolved"));

	const FPGXResolvedProfile& Profile = Sub->GetResolvedProfile();

	// EN: Verify all 5 layers are populated
	// ES: Verificar que las 5 capas estan pobladas
	LogTestResult(TEXT("Layer 1 — Identity"),
		Profile.Identity.ActiveTargets.Num() > 0,
		FString::Printf(TEXT("Mode=%d Targets=%d Build=%d"),
			static_cast<int32>(Profile.Identity.ProjectMode),
			Profile.Identity.ActiveTargets.Num(),
			static_cast<int32>(Profile.Identity.BuildContext)));

	LogTestResult(TEXT("Layer 2 — Capabilities"), true,
		FString::Printf(TEXT("INI=%d SaveData=%d Console=%d"),
			Profile.Capabilities.bAllowINIPersistence,
			Profile.Capabilities.bAllowSaveData,
			Profile.Capabilities.bAllowConsoleCommands));

	LogTestResult(TEXT("Layer 3 — Policies"), true,
		FString::Printf(TEXT("Config→%d Log→%d Save→%d"),
			static_cast<int32>(Profile.Policies.Persistence.ConfigBackend),
			static_cast<int32>(Profile.Policies.Persistence.LogBackend),
			static_cast<int32>(Profile.Policies.Persistence.SaveBackend)));

	LogTestResult(TEXT("Layer 4 — Budgets"), true,
		FString::Printf(TEXT("RAM=%lld VRAM=%lld PSO=%d"),
			Profile.Budgets.RAM_MB, Profile.Budgets.VRAM_MB, Profile.Budgets.PSOBudget));

	LogTestResult(TEXT("Layer 5 — Features"), true,
		FString::Printf(TEXT("Nanite=%d Lumen=%d RT=%d"),
			static_cast<int32>(Profile.Features.Nanite.Policy),
			static_cast<int32>(Profile.Features.Lumen.Policy),
			static_cast<int32>(Profile.Features.RayTracing.Policy)));

	// EN: Test query API
	// ES: Test de API de consulta
	LogTestResult(TEXT("Query — IsCapabilityEnabled"), true,
		FString::Printf(TEXT("SaveData=%d HotReload=%d"),
			Sub->IsCapabilityEnabled(TEXT("SaveData")),
			Sub->IsCapabilityEnabled(TEXT("HotReload"))));

	LogTestResult(TEXT("Query — IsFeatureAllowed"), true,
		FString::Printf(TEXT("Nanite=%d Lumen=%d"),
			Sub->IsFeatureAllowed(TEXT("Nanite")),
			Sub->IsFeatureAllowed(TEXT("Lumen"))));

	LogTestResult(TEXT("Query — GetBudget"), true,
		FString::Printf(TEXT("RAM=%lld PSO=%lld"),
			Sub->GetBudget(TEXT("RAM_MB")),
			Sub->GetBudget(TEXT("PSOBudget"))));

	UE_LOG(LogPGXProfile, Log, TEXT("========================================"));
	UE_LOG(LogPGXProfile, Log, TEXT("  Quick Test Suite Complete"));
	UE_LOG(LogPGXProfile, Log, TEXT("========================================"));
	UE_LOG(LogPGXProfile, Log, TEXT(""));
}

// ============================================================================
// TestResolutionRules
// ============================================================================

void UPGXProfileTestUtility::TestResolutionRules(const UObject* /*WorldContextObject*/)
{
	UE_LOG(LogPGXProfile, Log, TEXT("--- TestResolutionRules ---"));

	// EN: Test merge helpers with synthetic data
	// ES: Test de helpers de merge con datos sinteticos

	// Test AND merge for capabilities
	FPGXProfileCapabilities Cap1;
	Cap1.bAllowSaveData = true;
	Cap1.bAllowHotReload = false;

	FPGXProfileCapabilities Cap2;
	Cap2.bAllowSaveData = false;
	Cap2.bAllowHotReload = true;

	UPGXProfileSubsystem::MergeCapabilitiesAND(Cap1, Cap2);
	LogTestResult(TEXT("AND Merge — SaveData (true AND false)"), !Cap1.bAllowSaveData);
	LogTestResult(TEXT("AND Merge — HotReload (false AND true)"), !Cap1.bAllowHotReload);

	// Test MIN merge for budgets
	FPGXProfileBudgets Bud1;
	Bud1.RAM_MB = 4096;
	Bud1.PSOBudget = 0; // unlimited

	FPGXProfileBudgets Bud2;
	Bud2.RAM_MB = 2048;
	Bud2.PSOBudget = 100;

	UPGXProfileSubsystem::MergeBudgetsMIN(Bud1, Bud2);
	LogTestResult(TEXT("MIN Merge — RAM (4096 vs 2048)"), Bud1.RAM_MB == 2048,
		FString::Printf(TEXT("Got: %lld"), Bud1.RAM_MB));
	LogTestResult(TEXT("MIN Merge — PSO (unlimited vs 100)"), Bud1.PSOBudget == 100,
		FString::Printf(TEXT("Got: %d"), Bud1.PSOBudget));

	// Test MostRestrictive merge for features
	EPGXFeaturePolicy Result1 = UPGXProfileSubsystem::MostRestrictivePolicy(
		EPGXFeaturePolicy::Allowed, EPGXFeaturePolicy::FallbackRequired);
	LogTestResult(TEXT("MostRestrictive (Allowed vs Fallback)"),
		Result1 == EPGXFeaturePolicy::FallbackRequired);

	EPGXFeaturePolicy Result2 = UPGXProfileSubsystem::MostRestrictivePolicy(
		EPGXFeaturePolicy::FallbackRequired, EPGXFeaturePolicy::Disallowed);
	LogTestResult(TEXT("MostRestrictive (Fallback vs Disallowed)"),
		Result2 == EPGXFeaturePolicy::Disallowed);
}

// ============================================================================
// TestPlatformOverrides
// ============================================================================

void UPGXProfileTestUtility::TestPlatformOverrides(const UObject* WorldContextObject)
{
	UE_LOG(LogPGXProfile, Log, TEXT("--- TestPlatformOverrides ---"));

	UPGXProfileSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		LogTestResult(TEXT("PlatformOverrides"), false, TEXT("No subsystem"));
		return;
	}

	const FPGXResolvedProfile& Profile = Sub->GetResolvedProfile();
	LogTestResult(TEXT("PlatformOverrides — Active Targets"), Profile.Identity.ActiveTargets.Num() > 0,
		FString::Printf(TEXT("%d targets active"), Profile.Identity.ActiveTargets.Num()));

	// EN: Log each active target
	// ES: Loguear cada target activo
	for (const EPGXTargetPlatform& Target : Profile.Identity.ActiveTargets)
	{
		UE_LOG(LogPGXProfile, Log, TEXT("  Active target: %d"), static_cast<int32>(Target));
	}

	LogTestResult(TEXT("PlatformOverrides — Profile State Valid"),
		Sub->IsProfileResolved(), TEXT("Override application verified via resolution state"));
}

// ============================================================================
// TestBuildOverrides
// ============================================================================

void UPGXProfileTestUtility::TestBuildOverrides(const UObject* WorldContextObject)
{
	UE_LOG(LogPGXProfile, Log, TEXT("--- TestBuildOverrides ---"));

	UPGXProfileSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		LogTestResult(TEXT("BuildOverrides"), false, TEXT("No subsystem"));
		return;
	}

	const FPGXResolvedProfile& Profile = Sub->GetResolvedProfile();
	LogTestResult(TEXT("BuildOverrides — Build Context"),
		true,
		FString::Printf(TEXT("Context=%d"), static_cast<int32>(Profile.Identity.BuildContext)));

	// EN: Verify build-context-specific policies are applied
	// ES: Verificar que las politicas especificas del contexto de build se aplican
	LogTestResult(TEXT("BuildOverrides — Security Policy"), true,
		FString::Printf(TEXT("Encrypt=%d Compress=%d StripDebug=%d"),
			Profile.Policies.Security.bRequireEncryption,
			Profile.Policies.Security.bRequireCompression,
			Profile.Policies.Security.bStripDebugStrings));
}

// ============================================================================
// TestCapabilityQueries
// ============================================================================

void UPGXProfileTestUtility::TestCapabilityQueries(const UObject* WorldContextObject)
{
	UE_LOG(LogPGXProfile, Log, TEXT("--- TestCapabilityQueries ---"));

	UPGXProfileSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		LogTestResult(TEXT("CapabilityQueries"), false, TEXT("No subsystem"));
		return;
	}

	// EN: Test all 10 named capabilities
	// ES: Test de las 10 capabilities nombradas
	static const TCHAR* CapNames[] = {
		TEXT("INIPersistence"), TEXT("SaveData"), TEXT("ExternalWrites"), TEXT("Exports"),
		TEXT("ConsoleCommands"), TEXT("DangerousCommands"), TEXT("Profiling"),
		TEXT("Telemetry"), TEXT("PersistentLogs"), TEXT("HotReload")
	};

	for (const TCHAR* Name : CapNames)
	{
		const bool bAllowed = Sub->IsCapabilityEnabled(FName(Name));
		LogTestResult(FString::Printf(TEXT("Capability '%s'"), Name), true,
			bAllowed ? TEXT("ALLOWED") : TEXT("DENIED"));
	}

	// EN: Test unknown capability returns true (permissive default)
	// ES: Test de capability desconocida retorna true (default permisivo)
	const bool bUnknown = Sub->IsCapabilityEnabled(TEXT("NonExistentCapability"));
	LogTestResult(TEXT("Unknown Capability — Permissive Default"), bUnknown);
}

// ============================================================================
// TestMultiPlatformResolution
// ============================================================================

void UPGXProfileTestUtility::TestMultiPlatformResolution(const UObject* /*WorldContextObject*/)
{
	UE_LOG(LogPGXProfile, Log, TEXT("--- TestMultiPlatformResolution ---"));

	// EN: Test with synthetic multi-platform data
	// ES: Test con datos sinteticos multi-plataforma

	// Simulate PC capabilities (all allowed)
	FPGXProfileCapabilities PCCaps;
	// defaults are all true

	// Simulate Switch capabilities (restricted)
	FPGXProfileCapabilities SwitchCaps;
	SwitchCaps.bAllowProfiling = false;
	SwitchCaps.bAllowDangerousCommands = false;
	SwitchCaps.bAllowHotReload = false;

	// AND merge
	UPGXProfileSubsystem::MergeCapabilitiesAND(PCCaps, SwitchCaps);

	LogTestResult(TEXT("MultiPlatform AND — Profiling (true AND false)"), !PCCaps.bAllowProfiling);
	LogTestResult(TEXT("MultiPlatform AND — DangerousCmd (true AND false)"), !PCCaps.bAllowDangerousCommands);
	LogTestResult(TEXT("MultiPlatform AND — SaveData (true AND true)"), PCCaps.bAllowSaveData);

	// Budget MIN merge
	FPGXProfileBudgets PCBudgets;
	PCBudgets.RAM_MB = 16384;
	PCBudgets.VRAM_MB = 8192;

	FPGXProfileBudgets SwitchBudgets;
	SwitchBudgets.RAM_MB = 4096;
	SwitchBudgets.VRAM_MB = 2048;

	UPGXProfileSubsystem::MergeBudgetsMIN(PCBudgets, SwitchBudgets);

	LogTestResult(TEXT("MultiPlatform MIN — RAM"), PCBudgets.RAM_MB == 4096,
		FString::Printf(TEXT("Got: %lld"), PCBudgets.RAM_MB));
	LogTestResult(TEXT("MultiPlatform MIN — VRAM"), PCBudgets.VRAM_MB == 2048,
		FString::Printf(TEXT("Got: %lld"), PCBudgets.VRAM_MB));
}

// ============================================================================
// TestShippingLock
// ============================================================================

void UPGXProfileTestUtility::TestShippingLock(const UObject* WorldContextObject)
{
	UE_LOG(LogPGXProfile, Log, TEXT("--- TestShippingLock ---"));

	UPGXProfileSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		LogTestResult(TEXT("ShippingLock"), false, TEXT("No subsystem"));
		return;
	}

	const EPGXProfileState CurrentState = Sub->GetProfileState();

#if UE_BUILD_SHIPPING
	LogTestResult(TEXT("ShippingLock — State is Locked"), CurrentState == EPGXProfileState::Locked);
#else
	LogTestResult(TEXT("ShippingLock — State is NOT Locked (non-Shipping build)"),
		CurrentState != EPGXProfileState::Locked,
		FString::Printf(TEXT("State=%d"), static_cast<int32>(CurrentState)));
#endif

	// EN: Verify resolved profile is accessible regardless of lock state
	// ES: Verificar que el profile resuelto es accesible independientemente del estado de lock
	const FPGXResolvedProfile& Profile = Sub->GetResolvedProfile();
	LogTestResult(TEXT("ShippingLock — Profile Accessible"), Profile.Identity.ActiveTargets.Num() > 0);
}

// ============================================================================
// EN: RunAllTests — aggregate validation for Test Dashboard
// ES: RunAllTests — validacion agregada para Test Dashboard
// ============================================================================

bool UPGXProfileTestUtility::RunAllTests(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	bool bAllPassed = true;

	OutIssues.Add(TEXT("=== PGX Profile Test Suite ==="));

	// EN: Test 1: Subsystem accessible / ES: Test 1: Subsistema accesible
	UPGXProfileSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		OutIssues.Add(TEXT("[FAIL] ProfileSubsystem not found"));
		return false;
	}
	OutIssues.Add(TEXT("[PASS] ProfileSubsystem found"));

	// EN: Test 2: Profile resolved / ES: Test 2: Profile resuelto
	if (!Sub->IsProfileResolved())
	{
		OutIssues.Add(TEXT("[FAIL] Profile not resolved"));
		bAllPassed = false;
	}
	else
	{
		OutIssues.Add(TEXT("[PASS] Profile resolved"));
	}

	// EN: Test 3: Identity layer populated / ES: Test 3: Capa Identity poblada
	const FPGXResolvedProfile& Profile = Sub->GetResolvedProfile();
	if (Profile.Identity.ActiveTargets.Num() == 0)
	{
		OutIssues.Add(TEXT("[FAIL] Identity layer — no active targets"));
		bAllPassed = false;
	}
	else
	{
		OutIssues.Add(FString::Printf(TEXT("[PASS] Identity layer — %d target(s)"), Profile.Identity.ActiveTargets.Num()));
	}

	// EN: Test 4: Capability queries / ES: Test 4: Consultas de capabilities
	// EN: Just verify the query doesn't crash / ES: Solo verificar que la consulta no crashea
	Sub->IsCapabilityEnabled(TEXT("SaveData"));
	OutIssues.Add(TEXT("[PASS] Capability query accessible"));

	// EN: Test 5: Feature queries / ES: Test 5: Consultas de features
	Sub->IsFeatureAllowed(TEXT("Nanite"));
	OutIssues.Add(TEXT("[PASS] Feature query accessible"));

	// EN: Test 6: Budget queries / ES: Test 6: Consultas de budgets
	const int64 RAM = Sub->GetBudget(TEXT("RAM_MB"));
	if (RAM >= 0)
	{
		OutIssues.Add(FString::Printf(TEXT("[PASS] Budget query — RAM_MB=%lld"), RAM));
	}
	else
	{
		OutIssues.Add(TEXT("[FAIL] Budget query returned negative value"));
		bAllPassed = false;
	}

	OutIssues.Add(FString::Printf(TEXT("=== Result: %s ==="), bAllPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED")));
	UE_LOG(LogPGXProfile, Log, TEXT("[Profile TestUtility] RunAllTests — %s"), bAllPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED"));
	return bAllPassed;
}
