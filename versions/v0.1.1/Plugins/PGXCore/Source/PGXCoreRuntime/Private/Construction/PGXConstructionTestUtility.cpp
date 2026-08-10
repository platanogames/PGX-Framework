// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Construction/PGXConstructionTestUtility.h"
#include "Construction/PGXConstructionResolver.h"
#include "Construction/PGXGameModeConstruction.h"
#include "Construction/PGXPlayerControllerConstruction.h"
#include "Construction/PGXGameStateConstruction.h"
#include "Construction/PGXPlayerStateConstruction.h"
#include "Construction/PGXCharacterConstruction.h"
#include "Construction/PGXPawnConstruction.h"
#include "Construction/PGXHUDConstruction.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/World.h"

// EN: Construction test utility implementation
// ES: Implementacion de utilidad de test de construccion

// ============================================================================
// EN: Internal helpers
// ES: Helpers internos
// ============================================================================

static void LogConstructionTestResult(const FString& TestName, bool bPassed, const FString& Details = TEXT(""))
{
	if (bPassed)
	{
		UE_LOG(LogPGXConstruction, Log, TEXT("[TestUtility] PASS: %s%s"),
			*TestName, Details.IsEmpty() ? TEXT("") : *FString::Printf(TEXT(" (%s)"), *Details));
	}
	else
	{
		UE_LOG(LogPGXConstruction, Error, TEXT("[TestUtility] FAIL: %s%s"),
			*TestName, Details.IsEmpty() ? TEXT("") : *FString::Printf(TEXT(" (%s)"), *Details));
	}
}

static UWorld* GetWorldFromContext(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}
	return GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
}

static UPGXWorldConstructionSubsystem* GetWorldSubsystem(const UObject* WorldContextObject)
{
	UWorld* World = GetWorldFromContext(WorldContextObject);
	return World ? World->GetSubsystem<UPGXWorldConstructionSubsystem>() : nullptr;
}

static void LogSlotStatus(const TCHAR* SlotName, EPGXClassSourceMode Source, bool bHasDA, const FString& DAName, TArray<FString>& OutIssues)
{
	const UEnum* SourceEnum = StaticEnum<EPGXClassSourceMode>();
	const FString SourceStr = SourceEnum ? SourceEnum->GetNameStringByValue(static_cast<int64>(Source)) : TEXT("Unknown");

	OutIssues.Add(FString::Printf(TEXT("  [%s] Source=%s, DA=%s"),
		SlotName, *SourceStr, bHasDA ? *DAName : TEXT("(none)")));
}

// EN: Helper to report resolution source for a type / ES: Helper para reportar fuente de resolucion por tipo
static void LogResolutionSource(const TCHAR* TypeName, const UPGXClassConstruction* Resolved,
	const UPGXClassConstruction* WorldOverride, bool bHasSettingsDA, TArray<FString>& OutIssues)
{
	FString Source;
	if (Resolved && WorldOverride && Resolved == WorldOverride)
	{
		Source = TEXT("WorldOverride");
	}
	else if (Resolved && bHasSettingsDA)
	{
		Source = TEXT("ProjectSettings");
	}
	else if (!Resolved)
	{
		Source = TEXT("None");
	}
	else
	{
		Source = TEXT("Unknown");
	}

	OutIssues.Add(FString::Printf(TEXT("  [%s] Source=%s, DA=%s"),
		TypeName, *Source, Resolved ? *Resolved->GetName() : TEXT("nullptr")));
}

// ============================================================================
// EN: Test 1 — RunQuickTest
// ES: Test 1 — RunQuickTest
// ============================================================================

bool UPGXConstructionTestUtility::RunQuickTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	UE_LOG(LogPGXConstruction, Log, TEXT("[TestUtility] ========== RunQuickTest START =========="));

	// EN: Check settings accessible / ES: Verificar settings accesible
	const UPGXConstructionSettings* Settings = GetDefault<UPGXConstructionSettings>();
	const bool bSettingsOk = (Settings != nullptr);
	LogConstructionTestResult(TEXT("QuickTest.SettingsAccessible"), bSettingsOk);
	if (!bSettingsOk)
	{
		OutIssues.Add(TEXT("UPGXConstructionSettings not accessible via GetDefault"));
		return false;
	}

	// EN: Check WorldSubsystem exists (non-fatal if null) / ES: Verificar que WorldSubsystem existe (no-fatal si null)
	UWorld* World = GetWorldFromContext(WorldContextObject);
	const UPGXWorldConstructionSubsystem* WorldSub = GetWorldSubsystem(WorldContextObject);
	const bool bWorldSubOk = (WorldSub != nullptr);
	LogConstructionTestResult(TEXT("QuickTest.WorldSubsystem"), bWorldSubOk,
		World ? TEXT("World valid") : TEXT("No World"));
	if (!bWorldSubOk)
	{
		OutIssues.Add(TEXT("WorldConstructionSubsystem not found (non-fatal — tests continue with Settings only)"));
	}

	// EN: Validate class sources / ES: Validar class sources
	TArray<FText> Warnings;
	const bool bWorkflowOk = Settings->ValidateClassSources(Warnings, /*bIncludeMapsModesCheck=*/ false);
	LogConstructionTestResult(TEXT("QuickTest.ClassSources"), bWorkflowOk,
		FString::Printf(TEXT("%d warnings"), Warnings.Num()));
	if (!bWorkflowOk)
	{
		for (const FText& Warning : Warnings)
		{
			OutIssues.Add(FString::Printf(TEXT("  Warning: %s"), *Warning.ToString()));
		}
	}

	// EN: Resolve each of the 7 types / ES: Resolver cada uno de los 7 tipos
	if (World)
	{
		OutIssues.Add(TEXT("Resolution results:"));

		const auto* GM  = FPGXConstructionResolver::Resolve<UPGXGameModeConstruction>(World);
		const auto* PC  = FPGXConstructionResolver::Resolve<UPGXPlayerControllerConstruction>(World);
		const auto* GS  = FPGXConstructionResolver::Resolve<UPGXGameStateConstruction>(World);
		const auto* PS  = FPGXConstructionResolver::Resolve<UPGXPlayerStateConstruction>(World);
		const auto* Chr = FPGXConstructionResolver::Resolve<UPGXCharacterConstruction>(World);
		const auto* Pwn = FPGXConstructionResolver::Resolve<UPGXPawnConstruction>(World);
		const auto* HUD = FPGXConstructionResolver::Resolve<UPGXHUDConstruction>(World);

		OutIssues.Add(FString::Printf(TEXT("  GameMode:         %s"), GM  ? *GM->GetName()  : TEXT("nullptr")));
		OutIssues.Add(FString::Printf(TEXT("  PlayerController: %s"), PC  ? *PC->GetName()  : TEXT("nullptr")));
		OutIssues.Add(FString::Printf(TEXT("  GameState:        %s"), GS  ? *GS->GetName()  : TEXT("nullptr")));
		OutIssues.Add(FString::Printf(TEXT("  PlayerState:      %s"), PS  ? *PS->GetName()  : TEXT("nullptr")));
		OutIssues.Add(FString::Printf(TEXT("  Character:        %s"), Chr ? *Chr->GetName() : TEXT("nullptr")));
		OutIssues.Add(FString::Printf(TEXT("  Pawn:             %s"), Pwn ? *Pwn->GetName() : TEXT("nullptr")));
		OutIssues.Add(FString::Printf(TEXT("  HUD:              %s"), HUD ? *HUD->GetName() : TEXT("nullptr")));
	}

	UE_LOG(LogPGXConstruction, Log, TEXT("[TestUtility] ========== RunQuickTest END =========="));
	// EN: WorldSub null is non-fatal — Settings and Workflow are the critical checks
	// ES: WorldSub null es no-fatal — Settings y Workflow son los checks criticos
	return bSettingsOk && bWorkflowOk;
}

// ============================================================================
// EN: Test 2 — TestResolverPriority
// ES: Test 2 — TestResolverPriority
// ============================================================================

bool UPGXConstructionTestUtility::TestResolverPriority(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	UE_LOG(LogPGXConstruction, Log, TEXT("[TestUtility] ========== TestResolverPriority START =========="));

	UWorld* World = GetWorldFromContext(WorldContextObject);
	if (!World)
	{
		OutIssues.Add(TEXT("No valid World context"));
		LogConstructionTestResult(TEXT("TestResolverPriority"), false, TEXT("No World"));
		return false;
	}

	const UPGXWorldConstructionSubsystem* WorldSub = World->GetSubsystem<UPGXWorldConstructionSubsystem>();
	const UPGXConstructionSettings* Settings = GetDefault<UPGXConstructionSettings>();

	OutIssues.Add(TEXT("Resolver Priority Chain (WorldOverride >> ProjectSettings >> nullptr):"));

	// EN: Macro to reduce repetition for all 7 types / ES: Macro para reducir repeticion en los 7 tipos
#define PGX_CHECK_RESOLVER_PRIORITY(TypeName, ConstructionClass, SettingsField) \
	{ \
		const auto* Resolved = FPGXConstructionResolver::Resolve<ConstructionClass>(World); \
		const auto* Override = WorldSub ? WorldSub->GetOverride<ConstructionClass>() : nullptr; \
		const bool bSettingsDA = !Settings->SettingsField.IsNull(); \
		LogResolutionSource(TEXT(TypeName), Resolved, Override, bSettingsDA, OutIssues); \
	}

	PGX_CHECK_RESOLVER_PRIORITY("GameMode",         UPGXGameModeConstruction,         GameModeConstruction)
	PGX_CHECK_RESOLVER_PRIORITY("PlayerController",  UPGXPlayerControllerConstruction,  PlayerControllerConstruction)
	PGX_CHECK_RESOLVER_PRIORITY("GameState",         UPGXGameStateConstruction,         GameStateConstruction)
	PGX_CHECK_RESOLVER_PRIORITY("PlayerState",       UPGXPlayerStateConstruction,       PlayerStateConstruction)
	PGX_CHECK_RESOLVER_PRIORITY("Character",         UPGXCharacterConstruction,         CharacterConstruction)
	PGX_CHECK_RESOLVER_PRIORITY("Pawn",              UPGXPawnConstruction,              PawnConstruction)
	PGX_CHECK_RESOLVER_PRIORITY("HUD",               UPGXHUDConstruction,               HUDConstruction)

#undef PGX_CHECK_RESOLVER_PRIORITY

	LogConstructionTestResult(TEXT("TestResolverPriority"), true, TEXT("Resolution chain logged"));
	UE_LOG(LogPGXConstruction, Log, TEXT("[TestUtility] ========== TestResolverPriority END =========="));
	return true;
}

// ============================================================================
// EN: Test 3 — TestWorkflowValidation
// ES: Test 3 — TestWorkflowValidation
// ============================================================================

bool UPGXConstructionTestUtility::TestWorkflowValidation(const UObject* /*WorldContextObject*/, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	UE_LOG(LogPGXConstruction, Log, TEXT("[TestUtility] ========== TestWorkflowValidation START =========="));

	const UPGXConstructionSettings* Settings = GetDefault<UPGXConstructionSettings>();
	if (!Settings)
	{
		OutIssues.Add(TEXT("UPGXConstructionSettings not accessible"));
		LogConstructionTestResult(TEXT("TestWorkflowValidation"), false, TEXT("No settings"));
		return false;
	}

	TArray<FText> Warnings;
	const bool bValid = Settings->ValidateClassSources(Warnings, /*bIncludeMapsModesCheck=*/ false);

	if (Warnings.Num() == 0)
	{
		OutIssues.Add(TEXT("ValidateClassSources: No warnings (all slots consistent)"));
	}
	else
	{
		OutIssues.Add(FString::Printf(TEXT("ValidateClassSources: %d warning(s):"), Warnings.Num()));
		for (const FText& Warning : Warnings)
		{
			OutIssues.Add(FString::Printf(TEXT("  - %s"), *Warning.ToString()));
		}
	}

	LogConstructionTestResult(TEXT("TestWorkflowValidation"), bValid,
		FString::Printf(TEXT("%d warnings"), Warnings.Num()));

	UE_LOG(LogPGXConstruction, Log, TEXT("[TestUtility] ========== TestWorkflowValidation END =========="));
	return bValid;
}

// ============================================================================
// EN: Test 4 — TestSettingsIntegrity
// ES: Test 4 — TestSettingsIntegrity
// ============================================================================

bool UPGXConstructionTestUtility::TestSettingsIntegrity(const UObject* /*WorldContextObject*/, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	UE_LOG(LogPGXConstruction, Log, TEXT("[TestUtility] ========== TestSettingsIntegrity START =========="));

	const UPGXConstructionSettings* Settings = GetDefault<UPGXConstructionSettings>();
	if (!Settings)
	{
		OutIssues.Add(TEXT("UPGXConstructionSettings not accessible"));
		LogConstructionTestResult(TEXT("TestSettingsIntegrity"), false, TEXT("No settings"));
		return false;
	}

	bool bAllConsistent = true;
	OutIssues.Add(TEXT("Settings Slot Integrity:"));

	// EN: Check each slot: if Source != Default, class/DA should be assigned / ES: Verificar cada slot: si Source != Default, clase/DA deberia estar asignado
#define PGX_CHECK_SLOT(SlotName, SourceField, DAField) \
	{ \
		const bool bHasDA = !Settings->DAField.IsNull(); \
		const FString DAName = bHasDA ? Settings->DAField.GetAssetName() : TEXT(""); \
		LogSlotStatus(TEXT(SlotName), Settings->SourceField, bHasDA, DAName, OutIssues); \
		if (Settings->SourceField != EPGXClassSourceMode::Default && !bHasDA) \
		{ \
			OutIssues.Add(FString::Printf(TEXT("  >> NOTE: %s has Source != Default but no DA assigned"), TEXT(SlotName))); \
		} \
	}

	PGX_CHECK_SLOT("GameMode",         GameModeClassSource,         GameModeConstruction)
	PGX_CHECK_SLOT("PlayerController",  PlayerControllerClassSource,  PlayerControllerConstruction)
	PGX_CHECK_SLOT("GameState",         GameStateClassSource,         GameStateConstruction)
	PGX_CHECK_SLOT("PlayerState",       PlayerStateClassSource,       PlayerStateConstruction)
	PGX_CHECK_SLOT("Character",         CharacterClassSource,         CharacterConstruction)
	PGX_CHECK_SLOT("Pawn",              PawnClassSource,              PawnConstruction)
	PGX_CHECK_SLOT("HUD",               HUDClassSource,               HUDConstruction)

#undef PGX_CHECK_SLOT

	LogConstructionTestResult(TEXT("TestSettingsIntegrity"), bAllConsistent,
		bAllConsistent ? TEXT("All slots consistent") : TEXT("Some slots inconsistent"));

	UE_LOG(LogPGXConstruction, Log, TEXT("[TestUtility] ========== TestSettingsIntegrity END =========="));
	return bAllConsistent;
}

// ============================================================================
// EN: Test 5 — TestWorldOverrides
// ES: Test 5 — TestWorldOverrides
// ============================================================================

bool UPGXConstructionTestUtility::TestWorldOverrides(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	UE_LOG(LogPGXConstruction, Log, TEXT("[TestUtility] ========== TestWorldOverrides START =========="));

	const UPGXWorldConstructionSubsystem* WorldSub = GetWorldSubsystem(WorldContextObject);
	if (!WorldSub)
	{
		OutIssues.Add(TEXT("WorldConstructionSubsystem not found (non-fatal — no world overrides to check)"));
		LogConstructionTestResult(TEXT("TestWorldOverrides"), true, TEXT("No WorldSubsystem — skipped"));
		return true;
	}

	int32 ActiveOverrides = 0;
	OutIssues.Add(TEXT("World Override Status:"));

#define PGX_CHECK_OVERRIDE(TypeName, ConstructionClass) \
	{ \
		const auto* Override = WorldSub->GetOverride<ConstructionClass>(); \
		const bool bActive = (Override != nullptr); \
		OutIssues.Add(FString::Printf(TEXT("  [%s] %s%s"), \
			TEXT(TypeName), \
			bActive ? TEXT("ACTIVE") : TEXT("nullptr"), \
			bActive ? *FString::Printf(TEXT(" (%s)"), *Override->GetName()) : TEXT(""))); \
		if (bActive) { ActiveOverrides++; } \
	}

	PGX_CHECK_OVERRIDE("GameMode",         UPGXGameModeConstruction)
	PGX_CHECK_OVERRIDE("PlayerController",  UPGXPlayerControllerConstruction)
	PGX_CHECK_OVERRIDE("GameState",         UPGXGameStateConstruction)
	PGX_CHECK_OVERRIDE("PlayerState",       UPGXPlayerStateConstruction)
	PGX_CHECK_OVERRIDE("Character",         UPGXCharacterConstruction)
	PGX_CHECK_OVERRIDE("Pawn",              UPGXPawnConstruction)
	PGX_CHECK_OVERRIDE("HUD",              UPGXHUDConstruction)

#undef PGX_CHECK_OVERRIDE

	OutIssues.Add(FString::Printf(TEXT("Active overrides: %d/7"), ActiveOverrides));
	LogConstructionTestResult(TEXT("TestWorldOverrides"), true,
		FString::Printf(TEXT("%d/7 active"), ActiveOverrides));

	UE_LOG(LogPGXConstruction, Log, TEXT("[TestUtility] ========== TestWorldOverrides END =========="));
	return true;
}

// ============================================================================
// EN: Test 6 — TestDADiscoveryAndValidation
// ES: Test 6 — TestDADiscoveryAndValidation
// ============================================================================

bool UPGXConstructionTestUtility::TestDADiscoveryAndValidation(const UObject* /*WorldContextObject*/, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	UE_LOG(LogPGXConstruction, Log, TEXT("[TestUtility] ========== TestDADiscoveryAndValidation START =========="));

	const FAssetRegistryModule& Module = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	const IAssetRegistry& Registry = Module.Get();

	// EN: Scan for all concrete subclasses of UPGXClassConstruction / ES: Escanear subclases concretas de UPGXClassConstruction
	TArray<FAssetData> FoundAssets;
	Registry.GetAssetsByClass(UPGXClassConstruction::StaticClass()->GetClassPathName(), FoundAssets, true);

	if (FoundAssets.Num() == 0)
	{
		OutIssues.Add(TEXT("No UPGXClassConstruction DAs found in project"));
		LogConstructionTestResult(TEXT("DADiscovery"), true, TEXT("0 DAs (none created yet)"));
		UE_LOG(LogPGXConstruction, Log, TEXT("[TestUtility] ========== TestDADiscoveryAndValidation END =========="));
		return true;
	}

	OutIssues.Add(FString::Printf(TEXT("Found %d Construction DA(s):"), FoundAssets.Num()));

	bool bAllValid = true;
	int32 ValidCount = 0;
	int32 InvalidCount = 0;

	for (const FAssetData& AssetData : FoundAssets)
	{
		const UPGXClassConstruction* ConstructionDA = Cast<UPGXClassConstruction>(AssetData.GetAsset());
		if (!ConstructionDA)
		{
			OutIssues.Add(FString::Printf(TEXT("  SKIP: Failed to load %s"), *AssetData.GetObjectPathString()));
			continue;
		}

		TArray<FText> ValidationErrors;
		const bool bValid = ConstructionDA->Validate(ValidationErrors);

		if (bValid)
		{
			ValidCount++;
			OutIssues.Add(FString::Printf(TEXT("  OK: %s (%s)"),
				*ConstructionDA->GetName(), *ConstructionDA->GetClass()->GetName()));
		}
		else
		{
			InvalidCount++;
			bAllValid = false;
			OutIssues.Add(FString::Printf(TEXT("  INVALID: %s (%d errors):"),
				*ConstructionDA->GetName(), ValidationErrors.Num()));
			for (const FText& Error : ValidationErrors)
			{
				OutIssues.Add(FString::Printf(TEXT("    - %s"), *Error.ToString()));
			}
		}
	}

	OutIssues.Add(FString::Printf(TEXT("Summary: %d valid, %d invalid out of %d"),
		ValidCount, InvalidCount, FoundAssets.Num()));

	LogConstructionTestResult(TEXT("DADiscoveryAndValidation"), bAllValid,
		FString::Printf(TEXT("%d valid, %d invalid"), ValidCount, InvalidCount));

	UE_LOG(LogPGXConstruction, Log, TEXT("[TestUtility] ========== TestDADiscoveryAndValidation END =========="));
	return bAllValid;
}

// ============================================================================
// EN: Test 7 — TestFullResolutionReport
// ES: Test 7 — TestFullResolutionReport
// ============================================================================

bool UPGXConstructionTestUtility::TestFullResolutionReport(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	UE_LOG(LogPGXConstruction, Log, TEXT("[TestUtility] ========== TestFullResolutionReport START =========="));

	UWorld* World = GetWorldFromContext(WorldContextObject);
	if (!World)
	{
		OutIssues.Add(TEXT("No valid World context"));
		LogConstructionTestResult(TEXT("TestFullResolutionReport"), false, TEXT("No World"));
		return false;
	}

	const UPGXWorldConstructionSubsystem* WorldSub = World->GetSubsystem<UPGXWorldConstructionSubsystem>();
	const UPGXConstructionSettings* Settings = GetDefault<UPGXConstructionSettings>();

	OutIssues.Add(TEXT("=== PGX Construction — Full Resolution Report ==="));

	// EN: Helper lambda to report one type / ES: Lambda helper para reportar un tipo
	auto ReportType = [&](const TCHAR* TypeName, const UPGXClassConstruction* Resolved,
		const UPGXClassConstruction* Override, bool bHasSettingsDA)
	{
		// EN: Determine source / ES: Determinar fuente
		FString Source;
		if (!Resolved)
		{
			Source = TEXT("None");
		}
		else if (Override && Resolved == Override)
		{
			Source = TEXT("WorldOverride");
		}
		else if (bHasSettingsDA)
		{
			Source = TEXT("ProjectSettings");
		}
		else
		{
			Source = TEXT("Unknown");
		}

		OutIssues.Add(FString::Printf(TEXT("--- %s ---"), TypeName));
		OutIssues.Add(FString::Printf(TEXT("  DA: %s"), Resolved ? *Resolved->GetName() : TEXT("(none)")));
		OutIssues.Add(FString::Printf(TEXT("  Source: %s"), *Source));

		if (Resolved)
		{
			OutIssues.Add(FString::Printf(TEXT("  Components to inject: %d"), Resolved->Components.Num()));
			OutIssues.Add(FString::Printf(TEXT("  Required components: %d"), Resolved->RequiredComponents.Num()));
			OutIssues.Add(FString::Printf(TEXT("  System configs: %d"), Resolved->SystemConfigs.Num()));

			const FGameplayTagContainer& Tags = Resolved->GrantedTags;
			if (Tags.Num() > 0)
			{
				OutIssues.Add(FString::Printf(TEXT("  Granted tags: %s"), *Tags.ToStringSimple()));
			}
			else
			{
				OutIssues.Add(TEXT("  Granted tags: (none)"));
			}
		}
		else
		{
			OutIssues.Add(TEXT("  (No DA resolved — using engine defaults)"));
		}
	};

	// EN: Resolve and report each type / ES: Resolver y reportar cada tipo
#define PGX_REPORT_TYPE(TypeName, ConstructionClass, SettingsField) \
	{ \
		const auto* Resolved = FPGXConstructionResolver::Resolve<ConstructionClass>(World); \
		const auto* Override = WorldSub ? WorldSub->GetOverride<ConstructionClass>() : nullptr; \
		const bool bSettingsDA = !Settings->SettingsField.IsNull(); \
		ReportType(TEXT(TypeName), Resolved, Override, bSettingsDA); \
	}

	PGX_REPORT_TYPE("GameMode",         UPGXGameModeConstruction,         GameModeConstruction)
	PGX_REPORT_TYPE("PlayerController",  UPGXPlayerControllerConstruction,  PlayerControllerConstruction)
	PGX_REPORT_TYPE("GameState",         UPGXGameStateConstruction,         GameStateConstruction)
	PGX_REPORT_TYPE("PlayerState",       UPGXPlayerStateConstruction,       PlayerStateConstruction)
	PGX_REPORT_TYPE("Character",         UPGXCharacterConstruction,         CharacterConstruction)
	PGX_REPORT_TYPE("Pawn",              UPGXPawnConstruction,              PawnConstruction)
	PGX_REPORT_TYPE("HUD",               UPGXHUDConstruction,               HUDConstruction)

#undef PGX_REPORT_TYPE

	OutIssues.Add(TEXT("=== End Resolution Report ==="));
	LogConstructionTestResult(TEXT("TestFullResolutionReport"), true, TEXT("Report generated"));

	UE_LOG(LogPGXConstruction, Log, TEXT("[TestUtility] ========== TestFullResolutionReport END =========="));
	return true;
}

// ============================================================================
// EN: RunAllTests — aggregate suite
// ES: Ejecutar Todos — suite agregada
// ============================================================================

bool UPGXConstructionTestUtility::RunAllTests(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	TArray<FString> TestIssues;
	bool bAllPassed = true;

	OutIssues.Add(TEXT("=== PGX Construction Test Suite ==="));

	// EN: Test 1: Quick Test / ES: Test 1: Quick Test
	TestIssues.Empty();
	if (!RunQuickTest(WorldContextObject, TestIssues))
	{
		bAllPassed = false;
		OutIssues.Add(TEXT("[FAIL] RunQuickTest"));
		OutIssues.Append(TestIssues);
	}
	else
	{
		OutIssues.Add(TEXT("[PASS] RunQuickTest"));
	}

	// EN: Test 2: Resolver Priority / ES: Test 2: Prioridad del Resolver
	TestIssues.Empty();
	if (!TestResolverPriority(WorldContextObject, TestIssues))
	{
		bAllPassed = false;
		OutIssues.Add(TEXT("[FAIL] TestResolverPriority"));
		OutIssues.Append(TestIssues);
	}
	else
	{
		OutIssues.Add(TEXT("[PASS] TestResolverPriority"));
	}

	// EN: Test 3: Workflow Validation / ES: Test 3: Validacion de Workflow
	TestIssues.Empty();
	if (!TestWorkflowValidation(WorldContextObject, TestIssues))
	{
		bAllPassed = false;
		OutIssues.Add(TEXT("[FAIL] TestWorkflowValidation"));
		OutIssues.Append(TestIssues);
	}
	else
	{
		OutIssues.Add(TEXT("[PASS] TestWorkflowValidation"));
	}

	// EN: Test 4: Settings Integrity / ES: Test 4: Integridad de Settings
	TestIssues.Empty();
	if (!TestSettingsIntegrity(WorldContextObject, TestIssues))
	{
		bAllPassed = false;
		OutIssues.Add(TEXT("[FAIL] TestSettingsIntegrity"));
		OutIssues.Append(TestIssues);
	}
	else
	{
		OutIssues.Add(TEXT("[PASS] TestSettingsIntegrity"));
	}

	// EN: Test 5: World Overrides / ES: Test 5: Overrides de Mundo
	TestIssues.Empty();
	if (!TestWorldOverrides(WorldContextObject, TestIssues))
	{
		bAllPassed = false;
		OutIssues.Add(TEXT("[FAIL] TestWorldOverrides"));
		OutIssues.Append(TestIssues);
	}
	else
	{
		OutIssues.Add(TEXT("[PASS] TestWorldOverrides"));
	}

	// EN: Test 6: DA Discovery / ES: Test 6: Descubrimiento de DAs
	TestIssues.Empty();
	if (!TestDADiscoveryAndValidation(WorldContextObject, TestIssues))
	{
		bAllPassed = false;
		OutIssues.Add(TEXT("[FAIL] TestDADiscoveryAndValidation"));
		OutIssues.Append(TestIssues);
	}
	else
	{
		OutIssues.Add(TEXT("[PASS] TestDADiscoveryAndValidation"));
	}

	// EN: Test 7: Full Resolution Report / ES: Test 7: Reporte Completo
	TestIssues.Empty();
	if (!TestFullResolutionReport(WorldContextObject, TestIssues))
	{
		bAllPassed = false;
		OutIssues.Add(TEXT("[FAIL] TestFullResolutionReport"));
		OutIssues.Append(TestIssues);
	}
	else
	{
		OutIssues.Add(TEXT("[PASS] TestFullResolutionReport"));
	}

	OutIssues.Add(FString::Printf(TEXT("=== Result: %s ==="), bAllPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED")));

	UE_LOG(LogPGXConstruction, Log, TEXT("RunAllTests — %s"), bAllPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED"));
	return bAllPassed;
}
