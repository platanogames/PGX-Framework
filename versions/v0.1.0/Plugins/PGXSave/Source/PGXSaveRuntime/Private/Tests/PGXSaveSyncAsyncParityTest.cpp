// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#if WITH_DEV_AUTOMATION_TESTS

#include "Testing/PGXTestBase.h"
#include "Tests/PGXSaveTestHelpers.h"
#include "PGXSaveSubsystem.h"
#include "PGXSaveConfig.h"
#include "PGXSaveGame.h"
#include "PGXSaveSerializer.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformTime.h"
#include "Misc/AutomationTest.h"

/**
 * EN: Sync/async persistence parity contract. Equivalent input saved through the
 *     synchronous and asynchronous entry points must produce byte-equivalent files.
 *     Migration-chain parity requires a separate fixture with a registered migrator.
 * ES: Contrato de paridad de persistencia sync/async. El mismo input guardado por los
 *     entry points sincrono y asincrono debe producir archivos byte-equivalentes.
 *     La paridad de migracion requiere un fixture separado con migrator registrado.
 */

namespace
{
	// EN: Maximum wall-clock seconds to wait for an async save to drain.
	// ES: Segundos maximos de wall-clock para esperar que un save async drene.
	constexpr double GPGXAsyncSaveTimeoutSeconds = 5.0;
}

PGX_TEST_GAME(FPGXSave_SyncAsyncParity_SyncAsyncParity)
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

	const FGameplayTag TestContextTag = FGameplayTag::RequestGameplayTag(TEXT("PGX.Save.Context"));
	const FGameplayTag TestDomainTag = FGameplayTag::RequestGameplayTag(TEXT("PGX.Save.Domain"));

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
	TestConfig->bValidateChecksum = true;
	TestConfig->bCreateBackupBeforeSave = false;
	TestConfig->bCompressSaveData = false;

	FPGXSaveDomainEntry DomainEntry;
	DomainEntry.DomainTag = TestDomainTag;
	DomainEntry.SaveGameClass = UPGXSaveGame::StaticClass();
	DomainEntry.bRequired = true;
	TestConfig->SaveDomains.Add(DomainEntry);

	SaveSubsystem->InjectTestConfig(TestConfig);

	const FString SyncSlot = PGXSaveTestHelpers::MakeUniqueTestSlotName(TEXT("SyncAsyncParity_Sync"));
	const FString AsyncSlot = PGXSaveTestHelpers::MakeUniqueTestSlotName(TEXT("SyncAsyncParity_Async"));

	// EN: Phase 1 — sync save (canonical reference output).
	// ES: Phase 1 — sync save (output canonico de referencia).
	const EPGXSaveResult SyncResult = SaveSubsystem->SaveContext(TestContextTag, SyncSlot);
	if (SyncResult != EPGXSaveResult::Success)
	{
		AddError(FString::Printf(
			TEXT("Test setup: sync SaveContext failed result=%d"),
			static_cast<int32>(SyncResult)));
		SaveSubsystem->ClearTestConfigs();
		PGXSaveTestHelpers::TearDownLocalTestGameInstance(GameInstance);
		return false;
	}

	// EN: Phase 2 — kick off async save. We poll IsSaveInProgress via a latent
	//     command up to GPGXAsyncSaveTimeoutSeconds. Async result code is not
	//     surfaced through subsystem return (void); we infer success by file
	//     presence + use ComparePipelineOutcomes for byte equivalence.
	// ES: Phase 2 — disparar async save. Polleamos IsSaveInProgress via comando
	//     latente hasta GPGXAsyncSaveTimeoutSeconds. El codigo de resultado
	//     async no se superficiea por el retorno del subsistema (void); inferimos
	//     exito por presencia del archivo + usamos ComparePipelineOutcomes para
	//     equivalencia de bytes.
	SaveSubsystem->SaveContextAsync(TestContextTag, AsyncSlot);

	const double Deadline = FPlatformTime::Seconds() + GPGXAsyncSaveTimeoutSeconds;

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

	// EN: Phase 3 — async settled. Compare persisted bytes + cleanup.
	// ES: Phase 3 — async asentado. Comparar bytes persistidos + cleanup.
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand(
		[this, SaveSubsystem, GameInstance, TestContextTag, SyncSlot, AsyncSlot, SyncResult]() -> bool
		{
			TArray<uint8> SyncBytes;
			TArray<uint8> AsyncBytes;

			const FString SyncPath = PGXSaveTestHelpers::ResolveFirstDomainFilePath(
				SaveSubsystem, TestContextTag, SyncSlot);
			const FString AsyncPath = PGXSaveTestHelpers::ResolveFirstDomainFilePath(
				SaveSubsystem, TestContextTag, AsyncSlot);

			const bool bSyncRead = PGXSaveTestHelpers::LoadFileBytes(SyncPath, SyncBytes);
			const bool bAsyncRead = PGXSaveTestHelpers::LoadFileBytes(AsyncPath, AsyncBytes);

			TestTrue(TEXT("sync/async-parity sync save produced a readable file"), bSyncRead);
			TestTrue(TEXT("sync/async-parity async save produced a readable file (timeout/path)"), bAsyncRead);

			// EN: Infer async result from file presence — if async timed out the
			//     file may be absent or partial.
			// ES: Inferir el resultado async por presencia del archivo — si async
			//     timeout el archivo puede estar ausente o parcial.
			const EPGXSaveResult AsyncResult = bAsyncRead && AsyncBytes.Num() > 0
				? EPGXSaveResult::Success
				: EPGXSaveResult::Failed;

			// EN: Normalize only the timestamp stamped independently by each pipeline.
			// ES: Normalizar solo el timestamp sellado independientemente por cada pipeline.
			UPGXSaveGame* SyncSaveGame = bSyncRead
				? UPGXSaveSerializer::DeserializeFromMemory(SyncBytes)
				: nullptr;
			UPGXSaveGame* AsyncSaveGame = bAsyncRead
				? UPGXSaveSerializer::DeserializeFromMemory(AsyncBytes)
				: nullptr;

			const bool bDeserialized = SyncSaveGame != nullptr && AsyncSaveGame != nullptr;
			TestTrue(TEXT("sync/async-parity persisted payloads deserialize"), bDeserialized);

			TArray<uint8> CanonicalSyncBytes;
			TArray<uint8> CanonicalAsyncBytes;
			bool bSyncCanonicalized = false;
			bool bAsyncCanonicalized = false;
			if (bDeserialized)
			{
				SyncSaveGame->SaveTimestamp = FDateTime();
				AsyncSaveGame->SaveTimestamp = FDateTime();
				bSyncCanonicalized = UPGXSaveSerializer::SerializeToMemory(SyncSaveGame, CanonicalSyncBytes);
				bAsyncCanonicalized = UPGXSaveSerializer::SerializeToMemory(AsyncSaveGame, CanonicalAsyncBytes);
			}

			TestTrue(TEXT("sync/async-parity sync payload canonicalizes"), bSyncCanonicalized);
			TestTrue(TEXT("sync/async-parity async payload canonicalizes"), bAsyncCanonicalized);

			FString DiffSummary;
			const bool bEquivalent = PGXSaveTestHelpers::ComparePipelineOutcomes(
				SyncResult, AsyncResult, CanonicalSyncBytes, CanonicalAsyncBytes, DiffSummary);

			TestTrue(
				FString::Printf(
					TEXT("sync/async-parity: sync vs async save persisted bytes parity: %s"),
					*DiffSummary),
				bEquivalent);

			// EN: Cleanup — delete both slots + clear injected configs + tear down fixture.
			// ES: Cleanup — borrar ambos slots + limpiar configs inyectados + tear down del fixture.
			SaveSubsystem->DeleteSlot(TestContextTag, SyncSlot);
			SaveSubsystem->DeleteSlot(TestContextTag, AsyncSlot);
			SaveSubsystem->ClearTestConfigs();
			PGXSaveTestHelpers::TearDownLocalTestGameInstance(GameInstance);

			return true;
		}));

	return true;
#else
	AddWarning(TEXT("sync/async-parity test requires WITH_EDITOR (uses InjectTestConfig)"));
	return true;
#endif
}

#endif // WITH_DEV_AUTOMATION_TESTS
