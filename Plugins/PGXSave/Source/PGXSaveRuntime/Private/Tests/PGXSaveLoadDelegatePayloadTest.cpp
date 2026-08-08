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
#include "Delegates/IDelegateInstance.h"

/**
 * EN: Load-completion payload contract. A successful load broadcasts the loaded
 *     primary-domain UPGXSaveGame pointer through the native completion delegate.
 *     The native delegate is used because automation test classes are not UObjects.
 * ES: Contrato del payload de load completion. Un load exitoso transmite el puntero
 *     UPGXSaveGame del dominio primario mediante el delegate nativo de completion.
 *     Se usa el delegate nativo porque las clases de automation test no son UObjects.
 */
PGX_TEST_GAME(FPGXSave_LoadDelegatePayload_LoadDelegatePayload)
{
#if WITH_EDITOR
	UGameInstance* GameInstance = PGXSaveTestHelpers::CreateLocalTestGameInstance();
	if (!GameInstance)
	{
		AddError(TEXT("Test setup: failed to create local UGameInstance fixture"));
		return false;
	}

	UPGXSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UPGXSaveSubsystem>();
	if (!SaveSubsystem)
	{
		AddError(TEXT("Test setup: UPGXSaveSubsystem missing"));
		PGXSaveTestHelpers::TearDownLocalTestGameInstance(GameInstance);
		return false;
	}

	UGameplayTagsManager& TagManager = UGameplayTagsManager::Get();
	const FGameplayTag TestContextTag = TagManager.AddNativeGameplayTag(
		TEXT("PGX.Save.Context.AutomationTest_LoadDelegatePayload"));
	const FGameplayTag TestDomainTag = TagManager.AddNativeGameplayTag(
		TEXT("PGX.Save.Domain.AutomationTest_LoadDelegatePayload"));

	if (!TestContextTag.IsValid() || !TestDomainTag.IsValid())
	{
		AddError(TEXT("Test setup: failed to register native gameplay tags"));
		PGXSaveTestHelpers::TearDownLocalTestGameInstance(GameInstance);
		return false;
	}

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
	TestConfig->bValidateChecksum = false;
	TestConfig->bCreateBackupBeforeSave = false;
	TestConfig->bCompressSaveData = false;

	FPGXSaveDomainEntry DomainEntry;
	DomainEntry.DomainTag = TestDomainTag;
	DomainEntry.SaveGameClass = UPGXSaveGame::StaticClass();
	DomainEntry.bRequired = true;
	TestConfig->SaveDomains.Add(DomainEntry);

	SaveSubsystem->InjectTestConfig(TestConfig);

	const FString TestSlot = PGXSaveTestHelpers::MakeUniqueTestSlotName(TEXT("LoadDelegatePayload"));

	// EN: Setup gate — create the slot via successful save.
	// ES: Gate de setup — crear el slot via save exitoso.
	const EPGXSaveResult SaveResult = SaveSubsystem->SaveContext(TestContextTag, TestSlot);
	if (SaveResult != EPGXSaveResult::Success)
	{
		AddError(FString::Printf(
			TEXT("Test setup: clean SaveContext failed result=%d"),
			static_cast<int32>(SaveResult)));
		SaveSubsystem->ClearTestConfigs();
		PGXSaveTestHelpers::TearDownLocalTestGameInstance(GameInstance);
		return false;
	}

	// EN: Capture the load completion via NATIVE multicast delegate.
	//     Automation tests are FAutomationTestBase, not UObject — cannot bind
	//     dynamic delegates. Native lambda binding is the test-side equivalent.
	// ES: Capturar el load completion via delegate multicast NATIVO.
	//     Los automation tests son FAutomationTestBase, no UObject — no pueden
	//     bind delegates dinamicos. Bind con lambda nativo es el equivalente
	//     test-side.
	bool bDelegateFired = false;
	EPGXSaveResult CapturedResult = EPGXSaveResult::Failed;
	UPGXSaveGame* CapturedSaveGame = nullptr;
	FString CapturedSlotName;

	FDelegateHandle Handle = SaveSubsystem->OnLoadCompletedNative.AddLambda(
		[&bDelegateFired, &CapturedResult, &CapturedSaveGame, &CapturedSlotName]
		(const FString& BroadcastSlot, EPGXSaveResult BroadcastResult, UPGXSaveGame* BroadcastSaveGame)
		{
			bDelegateFired = true;
			CapturedResult = BroadcastResult;
			CapturedSaveGame = BroadcastSaveGame;
			CapturedSlotName = BroadcastSlot;
		});

	// EN: Execute sync load — delegate fires within LoadContext on game thread.
	// ES: Ejecutar sync load — el delegate dispara dentro de LoadContext en
	//     el game thread.
	const EPGXSaveResult LoadResult = SaveSubsystem->LoadContext(TestContextTag, TestSlot);

	// EN: Drop subscription before assertions to avoid late re-entries.
	// ES: Soltar la subscripcion antes de las assertions para evitar
	//     re-entries tardios.
	SaveSubsystem->OnLoadCompletedNative.Remove(Handle);

	TestEqual(
		TEXT("load-delegate-payload setup: sync LoadContext must succeed"),
		static_cast<int32>(LoadResult),
		static_cast<int32>(EPGXSaveResult::Success));

	TestTrue(
		TEXT("load-delegate-payload: OnLoadCompletedNative must fire after successful load"),
		bDelegateFired);

	TestEqual(
		TEXT("load-delegate-payload: delegate result mirrors LoadContext typed result"),
		static_cast<int32>(CapturedResult),
		static_cast<int32>(EPGXSaveResult::Success));

	TestEqual(
		TEXT("load-delegate-payload: delegate slot name matches the loaded slot"),
		CapturedSlotName,
		TestSlot);

	// EN: CONTRACT under test — payload must NOT be nullptr on successful load.
	// ES: CONTRATO bajo test — el payload NO DEBE ser nullptr en un load exitoso.
	TestNotNull(
		TEXT("load-delegate-payload: OnLoadCompleted payload (UPGXSaveGame*) must not be nullptr per delegate contract"),
		CapturedSaveGame);

	// EN: Cleanup.
	// ES: Cleanup.
	SaveSubsystem->DeleteSlot(TestContextTag, TestSlot);
	SaveSubsystem->ClearTestConfigs();
	PGXSaveTestHelpers::TearDownLocalTestGameInstance(GameInstance);

	return true;
#else
	AddWarning(TEXT("load-delegate-payload test requires WITH_EDITOR (uses InjectTestConfig)"));
	return true;
#endif
}

#endif // WITH_DEV_AUTOMATION_TESTS
