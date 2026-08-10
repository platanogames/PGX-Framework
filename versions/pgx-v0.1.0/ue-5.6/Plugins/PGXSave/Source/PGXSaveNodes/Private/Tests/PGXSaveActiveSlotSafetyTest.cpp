// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#if WITH_DEV_AUTOMATION_TESTS

#include "Testing/PGXTestBase.h"
#include "PGXSaveFailingTestProvider.h"
#include "PGXSaveSubsystem.h"
#include "PGXSaveConfig.h"
#include "PGXSaveGame.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformTime.h"
#include "Misc/AutomationTest.h"
#include "Misc/Guid.h"

/**
 * EN: Active-slot safety contract. The active slot changes only after a save
 *     completes successfully. A failed asynchronous save preserves the previous slot.
 * ES: Contrato de seguridad del active slot. El slot activo cambia solo despues
 *     de completar un save correctamente. Un save asincrono fallido conserva el slot anterior.
 */

namespace
{
	constexpr double GPGXActiveSlotAsyncTimeoutSeconds = 5.0;

	UGameInstance* CreateLocalTestGameInstance()
	{
		if (!GEngine)
		{
			return nullptr;
		}

		UGameInstance* GameInstance = NewObject<UGameInstance>(
			GEngine,
			UGameInstance::StaticClass(),
			NAME_None,
			RF_Transient);
		if (!GameInstance)
		{
			return nullptr;
		}

		GameInstance->AddToRoot();
		GameInstance->InitializeStandalone();
		return GameInstance;
	}

	void TearDownLocalTestGameInstance(UGameInstance* GameInstance)
	{
		if (GameInstance)
		{
			GameInstance->Shutdown();
			GameInstance->RemoveFromRoot();
		}
	}

	FString MakeUniqueTestSlotName(const TCHAR* Prefix)
	{
		return FString::Printf(
			TEXT("Test_%s_%s"),
			Prefix,
			*FGuid::NewGuid().ToString(EGuidFormats::Short).Left(8));
	}
}

PGX_TEST_GAME(FPGXSave_ActiveSlotSafety_ActiveSlotSafety)
{
#if WITH_EDITOR
	UGameInstance* GameInstance = CreateLocalTestGameInstance();
	if (!GameInstance)
	{
		AddError(TEXT("Test setup: failed to create local UGameInstance fixture"));
		return false;
	}

	UPGXSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UPGXSaveSubsystem>();
	if (!SaveSubsystem)
	{
		AddError(TEXT("Test setup: UPGXSaveSubsystem missing"));
		TearDownLocalTestGameInstance(GameInstance);
		return false;
	}

	const FGameplayTag TestContextTag = FGameplayTag::RequestGameplayTag(TEXT("PGX.Save.Context"));
	const FGameplayTag TestDomainTag = FGameplayTag::RequestGameplayTag(TEXT("PGX.Save.Domain"));

	if (!TestContextTag.IsValid() || !TestDomainTag.IsValid())
	{
		AddError(TEXT("Test setup: failed to register native gameplay tags"));
		TearDownLocalTestGameInstance(GameInstance);
		return false;
	}

	// EN: Build config with failing provider class — every SaveBytes call
	//     returns false → operation should yield a failed typed result.
	// ES: Construir config con clase provider fallido — cada llamada SaveBytes
	//     retorna false → la operacion deberia producir un resultado tipado
	//     fallido.
	UPGXSaveConfig* TestConfig = NewObject<UPGXSaveConfig>(
		GetTransientPackage(),
		UPGXSaveConfig::StaticClass(),
		NAME_None,
		RF_Transient);
	if (!TestConfig)
	{
		AddError(TEXT("Test setup: NewObject<UPGXSaveConfig> returned null"));
		TearDownLocalTestGameInstance(GameInstance);
		return false;
	}

	TestConfig->ContextTag = TestContextTag;
	TestConfig->SaveMode = EPGXSaveMode::MultiSlot;
	TestConfig->bValidateChecksum = false;
	TestConfig->bCreateBackupBeforeSave = false;
	TestConfig->bCompressSaveData = false;
	TestConfig->SaveProviderClass = UPGXSaveFailingTestProvider::StaticClass();

	FPGXSaveDomainEntry DomainEntry;
	DomainEntry.DomainTag = TestDomainTag;
	DomainEntry.SaveGameClass = UPGXSaveGame::StaticClass();
	DomainEntry.bRequired = true;
	TestConfig->SaveDomains.Add(DomainEntry);

	SaveSubsystem->InjectTestConfig(TestConfig);

	const FString PreExistingSlot = MakeUniqueTestSlotName(TEXT("ActiveSlotSafety_Pre"));
	const FString FailedSlot = MakeUniqueTestSlotName(TEXT("ActiveSlotSafety_Fail"));

	// EN: Establish a known active slot via SetActiveSlot (no actual write —
	//     decoupled from any provider behavior).
	// ES: Establecer un active slot conocido via SetActiveSlot (sin escritura
	//     real — desacoplado de cualquier comportamiento del provider).
	SaveSubsystem->SetActiveSlot(TestContextTag, PreExistingSlot);

	const FString ActiveBeforeFailedAttempt = SaveSubsystem->GetActiveSlotName(TestContextTag);
	if (!TestEqual(
		TEXT("active-slot-safety setup: active slot equals pre-existing after SetActiveSlot"),
		ActiveBeforeFailedAttempt,
		PreExistingSlot))
	{
		SaveSubsystem->ClearTestConfigs();
		TearDownLocalTestGameInstance(GameInstance);
		return false;
	}

	// EN: Attempt async save that will fail at SaveBytes (provider returns false).
	// ES: Intento de async save que fallara en SaveBytes (provider retorna false).
	SaveSubsystem->SaveContextAsync(TestContextTag, FailedSlot);

	const double Deadline = FPlatformTime::Seconds() + GPGXActiveSlotAsyncTimeoutSeconds;

	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand(
		[SaveSubsystem, Deadline]() -> bool
		{
			if (!SaveSubsystem)
			{
				return true;
			}
			if (!SaveSubsystem->IsSaveInProgress() && !SaveSubsystem->IsLoadInProgress())
			{
				return true;
			}
			return FPlatformTime::Seconds() >= Deadline;
		}));

	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand(
		[this, SaveSubsystem, GameInstance, TestContextTag, PreExistingSlot, FailedSlot]() -> bool
		{
			const FString ActiveAfterFailedAttempt =
				SaveSubsystem->GetActiveSlotName(TestContextTag);

			// EN: CONTRACT — failed save MUST NOT advance ActiveSlot.
			// ES: CONTRATO — un save fallido NO DEBE avanzar el ActiveSlot.
			TestEqual(
				TEXT("active-slot-safety: failed async save must not advance ActiveSlot"),
				ActiveAfterFailedAttempt,
				PreExistingSlot);

			TestNotEqual(
				TEXT("active-slot-safety: ActiveSlot must NOT equal the failed slot"),
				ActiveAfterFailedAttempt,
				FailedSlot);

			SaveSubsystem->ClearTestConfigs();
			TearDownLocalTestGameInstance(GameInstance);
			return true;
		}));

	return true;
#else
	AddWarning(TEXT("active-slot-safety test requires WITH_EDITOR (uses InjectTestConfig)"));
	return true;
#endif
}

#endif // WITH_DEV_AUTOMATION_TESTS
