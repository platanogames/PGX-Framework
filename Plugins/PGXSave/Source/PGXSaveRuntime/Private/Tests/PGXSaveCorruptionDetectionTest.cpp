// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#if WITH_DEV_AUTOMATION_TESTS

#include "Testing/PGXTestBase.h"
#include "Tests/PGXSaveTestHelpers.h"
#include "PGXSaveSubsystem.h"
#include "PGXSaveConfig.h"
#include "PGXSaveGame.h"
#include "Engine/GameInstance.h"
#include "GameplayTagsManager.h"
#include "Misc/Paths.h"

/**
 * EN: Corruption detection contract. Persisted bytes are validated during load;
 *     a mutated payload must return EPGXSaveResult::Corrupted rather than Success.
 * ES: Contrato de deteccion de corrupcion. Los bytes persistidos se validan al cargar;
 *     un payload mutado debe retornar EPGXSaveResult::Corrupted en lugar de Success.
 */
PGX_TEST_GAME(FPGXSave_CorruptionDetection_CorruptionDetection)
{
#if WITH_EDITOR
	// EN: Use an isolated local GameInstance fixture.
	// ES: Usar un fixture GameInstance local y aislado.
	UGameInstance* GameInstance = PGXSaveTestHelpers::CreateLocalTestGameInstance();
	if (!GameInstance)
	{
		AddError(TEXT("Test setup: failed to create local UGameInstance fixture"));
		return false;
	}

	UPGXSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UPGXSaveSubsystem>();
	if (!SaveSubsystem)
	{
		AddError(TEXT("Test setup: UPGXSaveSubsystem missing from GameInstance"));
		PGXSaveTestHelpers::TearDownLocalTestGameInstance(GameInstance);
		return false;
	}

	// EN: Register transient test tags via the gameplay tags manager. Idempotent —
	//     subsequent runs reuse the same tag.
	// ES: Registrar tags de test transitorios via el manager de gameplay tags.
	//     Idempotente — runs subsiguientes reusan el mismo tag.
	UGameplayTagsManager& TagManager = UGameplayTagsManager::Get();
	const FGameplayTag TestContextTag = TagManager.AddNativeGameplayTag(
		TEXT("PGX.Save.Context.AutomationTest_CorruptionDetection"));
	const FGameplayTag TestDomainTag = TagManager.AddNativeGameplayTag(
		TEXT("PGX.Save.Domain.AutomationTest_CorruptionDetection"));

	if (!TestContextTag.IsValid() || !TestDomainTag.IsValid())
	{
		AddError(TEXT("Test setup: failed to register native gameplay tags"));
		PGXSaveTestHelpers::TearDownLocalTestGameInstance(GameInstance);
		return false;
	}

	// EN: Build minimal transient config — single domain, integrity ON,
	//     backup OFF (simpler test surface), MultiSlot mode.
	// ES: Construir config transitorio minimo — un solo dominio, integridad ON,
	//     backup OFF (superficie de test mas simple), modo MultiSlot.
	UPGXSaveConfig* TestConfig = NewObject<UPGXSaveConfig>(
		GetTransientPackage(),
		UPGXSaveConfig::StaticClass(),
		NAME_None,
		RF_Transient);
	if (!TestConfig)
	{
		AddError(TEXT("Test setup: NewObject<UPGXSaveConfig> returned null"));
		PGXSaveTestHelpers::TearDownLocalTestGameInstance(GameInstance);
		return false;
	}

	TestConfig->ContextTag = TestContextTag;
	TestConfig->SaveMode = EPGXSaveMode::MultiSlot;
	TestConfig->bValidateChecksum = true;
	TestConfig->bCreateBackupBeforeSave = false;
	TestConfig->bCompressSaveData = false;

	FPGXSaveDomainEntry DomainEntry;
	DomainEntry.DomainTag = TestDomainTag;
	DomainEntry.SaveGameClass = UPGXSaveGame::StaticClass();
	DomainEntry.bRequired = true;
	TestConfig->SaveDomains.Add(DomainEntry);

	SaveSubsystem->InjectTestConfig(TestConfig);

	const FString TestSlot = PGXSaveTestHelpers::MakeUniqueTestSlotName(TEXT("CorruptionDetection"));

	// EN: Phase 1 — produce a clean save. Setup gate; not the contract under test.
	// ES: Phase 1 — producir un save limpio. Gate de setup; no es el contrato bajo test.
	const EPGXSaveResult SaveResult = SaveSubsystem->SaveContext(TestContextTag, TestSlot);
	if (SaveResult != EPGXSaveResult::Success)
	{
		AddError(FString::Printf(
			TEXT("Test setup: clean SaveContext failed with result=%d"),
			static_cast<int32>(SaveResult)));
		SaveSubsystem->ClearTestConfigs();
		PGXSaveTestHelpers::TearDownLocalTestGameInstance(GameInstance);
		return false;
	}

	// EN: Phase 2 — mutate persisted bytes on disk. Setup gate.
	// ES: Phase 2 — mutar los bytes persistidos en disco. Gate de setup.
	const FString DomainFilePath = PGXSaveTestHelpers::ResolveFirstDomainFilePath(
		SaveSubsystem, TestContextTag, TestSlot);
	if (DomainFilePath.IsEmpty() || !FPaths::FileExists(DomainFilePath))
	{
		AddError(FString::Printf(
			TEXT("Test setup: domain file not resolved or missing: '%s'"),
			*DomainFilePath));
		SaveSubsystem->DeleteSlot(TestContextTag, TestSlot);
		SaveSubsystem->ClearTestConfigs();
		PGXSaveTestHelpers::TearDownLocalTestGameInstance(GameInstance);
		return false;
	}

	if (!PGXSaveTestHelpers::MutateLastByteOfFile(DomainFilePath))
	{
		AddError(FString::Printf(
			TEXT("Test setup: failed to mutate last byte of '%s'"),
			*DomainFilePath));
		SaveSubsystem->DeleteSlot(TestContextTag, TestSlot);
		SaveSubsystem->ClearTestConfigs();
		PGXSaveTestHelpers::TearDownLocalTestGameInstance(GameInstance);
		return false;
	}

	// EN: Phase 3 — load mutated payload. CONTRACT under test.
	//     Expected: EPGXSaveResult::Corrupted (not Success).
	// ES: Phase 3 — cargar payload mutado. CONTRATO bajo test.
	//     Esperado: EPGXSaveResult::Corrupted (no Success).
	const EPGXSaveResult LoadResult = SaveSubsystem->LoadContext(TestContextTag, TestSlot);

	TestEqual(
		TEXT("save-integrity: mutated payload must return Corrupted on load"),
		static_cast<int32>(LoadResult),
		static_cast<int32>(EPGXSaveResult::Corrupted));

	// EN: Cleanup — best-effort.
	// ES: Cleanup — best-effort.
	SaveSubsystem->DeleteSlot(TestContextTag, TestSlot);
	SaveSubsystem->ClearTestConfigs();
	PGXSaveTestHelpers::TearDownLocalTestGameInstance(GameInstance);

	return true;
#else
	AddWarning(TEXT("save-integrity test requires WITH_EDITOR (uses InjectTestConfig)"));
	return true;
#endif
}

#endif // WITH_DEV_AUTOMATION_TESTS
