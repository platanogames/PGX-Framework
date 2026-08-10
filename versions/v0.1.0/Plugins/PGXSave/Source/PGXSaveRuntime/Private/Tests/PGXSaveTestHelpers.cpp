// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/PGXSaveTestHelpers.h"
#include "PGXSaveSubsystem.h"
#include "PGXSaveConfig.h"
#include "PGXSaveProvider.h"
#include "PGXSaveSerializer.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/Paths.h"

namespace PGXSaveTestHelpers
{
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

		// EN: Root the instance so it survives GC during the latent test chain.
		// ES: Rootear la instancia para que sobreviva GC durante la cadena de
		//     comandos latentes del test.
		GameInstance->AddToRoot();

		// EN: InitializeStandalone bootstraps the GameInstance and triggers
		//     UGameInstanceSubsystem creation via UPGXGameInstanceSubsystem
		//     -> UPGXSaveSubsystem::Initialize. After this call,
		//     GetSubsystem<UPGXSaveSubsystem>() returns a live instance.
		// ES: InitializeStandalone hace bootstrap del GameInstance y dispara
		//     la creacion de UGameInstanceSubsystem via UPGXGameInstanceSubsystem
		//     -> UPGXSaveSubsystem::Initialize. Despues de esta llamada,
		//     GetSubsystem<UPGXSaveSubsystem>() retorna una instancia viva.
		GameInstance->InitializeStandalone();

		return GameInstance;
	}

	void TearDownLocalTestGameInstance(UGameInstance* GameInstance)
	{
		if (!GameInstance)
		{
			return;
		}

		GameInstance->Shutdown();
		GameInstance->RemoveFromRoot();
	}

	FString MakeUniqueTestSlotName(const FString& Prefix)
	{
		const FString GuidShort = FGuid::NewGuid().ToString(EGuidFormats::Short).Left(8);
		return FString::Printf(TEXT("Test_%s_%s"), *Prefix, *GuidShort);
	}

	bool MutateLastByteOfFile(const FString& FilePath)
	{
		TArray<uint8> Bytes;
		if (!FFileHelper::LoadFileToArray(Bytes, *FilePath))
		{
			return false;
		}
		if (Bytes.Num() == 0)
		{
			return false;
		}
		// EN: Flip last byte deterministically (XOR with 0xFF) — guaranteed mutation.
		// ES: Voltear ultimo byte deterministicamente (XOR con 0xFF) — mutacion garantizada.
		Bytes.Last() ^= 0xFF;
		return FFileHelper::SaveArrayToFile(Bytes, *FilePath);
	}

	bool LoadFileBytes(const FString& FilePath, TArray<uint8>& OutBytes)
	{
		OutBytes.Reset();
		if (!FPaths::FileExists(FilePath))
		{
			return false;
		}
		return FFileHelper::LoadFileToArray(OutBytes, *FilePath);
	}

	FString ResolveFirstDomainFilePath(
		UPGXSaveSubsystem* Subsystem,
		FGameplayTag ContextTag,
		const FString& SlotName)
	{
		if (!Subsystem)
		{
			return FString();
		}

		const UPGXSaveConfig* Config = Subsystem->GetContextConfig(ContextTag);
		if (!Config || Config->SaveDomains.Num() == 0)
		{
			return FString();
		}

		// EN: We do NOT have a public BaseSaveDirectory accessor on the subsystem.
		//     The provider exposes it via GetBaseSaveDirectory(); we obtain the active
		//     provider class name from a debug snapshot but the provider instance is
		//     private. For automation tests we resolve via the default project save dir
		//     pattern used by UPGXSaveProvider_Default — tests run in editor/dev
		//     contexts where this resolves to FPaths::ProjectSavedDir() / "SaveGames".
		//     Providers with alternate paths require an explicit subsystem helper; this helper
		//     intentionally covers the default provider.
		// ES: NO tenemos un accesor publico de BaseSaveDirectory en el subsistema.
		//     El provider lo expone via GetBaseSaveDirectory(); obtenemos el nombre de
		//     clase del provider activo via debug snapshot pero la instancia del
		//     provider es privada. Para tests de automation resolvemos via el patron default
		//     del project save dir usado por UPGXSaveProvider_Default — los tests
		//     corren en contextos editor/dev donde eso resuelve a
		//     FPaths::ProjectSavedDir() / "SaveGames". Providers con rutas alternativas
		//     requieren un helper explicito; este helper cubre el provider por defecto.
		const FString BasePath = FPaths::ProjectSavedDir() / TEXT("SaveGames");

		const FPGXSaveDomainEntry& FirstDomain = Config->SaveDomains[0];
		return UPGXSaveSerializer::ResolveSavePath(
			Config,
			BasePath,
			SlotName,
			FirstDomain.DomainTag);
	}

	bool ComparePipelineOutcomes(
		EPGXSaveResult SyncResult,
		EPGXSaveResult AsyncResult,
		const TArray<uint8>& SyncPersistedBytes,
		const TArray<uint8>& AsyncPersistedBytes,
		FString& OutDiffSummary)
	{
		const bool bResultMatch = (SyncResult == AsyncResult);
		const bool bSizeMatch = (SyncPersistedBytes.Num() == AsyncPersistedBytes.Num());
		bool bByteMatch = bSizeMatch;

		if (bSizeMatch)
		{
			for (int32 Idx = 0; Idx < SyncPersistedBytes.Num(); ++Idx)
			{
				if (SyncPersistedBytes[Idx] != AsyncPersistedBytes[Idx])
				{
					bByteMatch = false;
					break;
				}
			}
		}

		if (bResultMatch && bByteMatch)
		{
			OutDiffSummary = TEXT("equivalent (results + bytes match)");
			return true;
		}

		OutDiffSummary = FString::Printf(
			TEXT("DIVERGENT: SyncResult=%d AsyncResult=%d SyncBytes=%d AsyncBytes=%d ResultMatch=%s ByteMatch=%s"),
			static_cast<int32>(SyncResult),
			static_cast<int32>(AsyncResult),
			SyncPersistedBytes.Num(),
			AsyncPersistedBytes.Num(),
			bResultMatch ? TEXT("yes") : TEXT("no"),
			bByteMatch ? TEXT("yes") : TEXT("no"));
		return false;
	}
}

#endif // WITH_DEV_AUTOMATION_TESTS
