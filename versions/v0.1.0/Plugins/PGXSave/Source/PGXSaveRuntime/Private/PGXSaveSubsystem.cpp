// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXSaveSubsystem.h"
#include "PGXSaveConfig.h"
#include "PGXSaveSettings.h"
#include "Utils/PGXConfigResolution.h"
#include "PGXSaveGame.h"
#include "PGXSaveSerializer.h"
#include "PGXSaveBackupManager.h"
#include "PGXSaveVersioning.h"
#include "PGXSaveProvider.h"
#include "PGXSaveProvider_Default.h"
#include "Interfaces/PGXSaveable.h"
#include "Logging/PGXLogCategories.h"
#include "Logging/PGXLogMacros.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/AssetManager.h"
#include "HAL/IConsoleManager.h"
#include "Async/Async.h"
#include "Misc/FileHelper.h"
#include "Trace/PGXTraceHelper.h"
#include "Trace/PGXTraceTags.h"
#include "Profile/PGXProfileSubsystem.h"
#include "Profile/PGXPlatformConfig.h"

// EN: Save/load manager subsystem implementation
// ES: Implementacion del subsistema manager de guardado/carga

TWeakObjectPtr<UPGXSaveSubsystem> UPGXSaveSubsystem::CachedInstance;

// ============================================================================
// EN: Verbose pipeline logging macro (zero-cost in Shipping)
// ES: Macro de logging verbose del pipeline (zero-cost en Shipping)
// ============================================================================

#if !UE_BUILD_SHIPPING
#define PGX_SAVE_VERBOSE(Config, Step, Total, Format, ...) \
	do { if (Config && Config->bVerboseSaveDebug) { \
		PGX_LOG_INFO(LogPGXSave, TEXT("[SavePipeline] Step %d/%d: " Format), Step, Total, ##__VA_ARGS__); \
	} } while(0)
#else
#define PGX_SAVE_VERBOSE(Config, Step, Total, Format, ...)
#endif

// ============================================================================
// EN: Initialization / Deinitialize
// ES: Inicializacion / Deinicializacion
// ============================================================================

void UPGXSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CachedInstance = this;

	// EN: Resolve the operation-history cap from project settings and reserve up-front to avoid
	//     reallocations during steady-state ring-buffer rotation.
	// ES: Resolver cap del historial de operaciones desde project settings
	//     (data-driven, no literal en codigo). Reserve up-front
	//     para evitar realocaciones durante la rotacion en estado estable del
	//     ring buffer.
	if (const UPGXSaveSettings* SaveSettings = GetDefault<UPGXSaveSettings>())
	{
		MaxOperationHistory = FMath::Clamp(SaveSettings->MaxOperationHistory, 0, 10000);
	}
	OperationHistory.Reserve(MaxOperationHistory);

	DiscoverSaveConfigs();
	BuildDomainCache();
	CreateProvider();
	RegisterConsoleCommands();
	StartAllAutoSaveTimers();

	// EN: Register trace config from first discovered config (or default if none found)
	// ES: Registrar config de traza del primer config descubierto (o default si no se encontro)
	{
		FPGXTraceConfig SaveTraceConfig;
		if (DiscoveredConfigs.Num() > 0 && DiscoveredConfigs[0])
		{
			SaveTraceConfig = DiscoveredConfigs[0]->TraceConfig;
		}
		FPGXTraceHelper::RegisterSystemTraceConfig(TAG_PGX_System_Save, SaveTraceConfig);
	}

	// EN: Apply project profile constraints if available
	// ES: Aplicar restricciones del profile de proyecto si esta disponible
	if (auto* ProfileSS = GetGameInstance()->GetSubsystem<UPGXProfileSubsystem>())
	{
		if (ProfileSS->IsProfileResolved())
		{
			ApplyProfileConstraints(ProfileSS->GetResolvedProfile());
		}
		ProfileSS->OnProfileChangedNative.AddUObject(this, &ThisClass::HandleProfileChanged);
	}

	PGX_LOG_INFO(LogPGXSave, TEXT("[SaveSubsystem] Initialized: %d contexts, %d domains, provider: %s"),
		DiscoveredConfigs.Num(),
		DomainBindings.Num(),
		ActiveProvider ? *ActiveProvider->GetClass()->GetName() : TEXT("NONE"));
}

void UPGXSaveSubsystem::Deinitialize()
{
	// EN: Cleanup Profile delegate subscription / ES: Limpiar suscripcion a delegate de Profile
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UPGXProfileSubsystem* Profile = GI->GetSubsystem<UPGXProfileSubsystem>())
		{
			Profile->OnProfileChangedNative.RemoveAll(this);
		}
	}

	StopAllAutoSaveTimers();
	UnregisterConsoleCommands();
	FPGXTraceHelper::UnregisterSystemTraceConfig(TAG_PGX_System_Save);

	CachedInstance = nullptr;

	// EN: Clear all state
	// ES: Limpiar todo el estado
	RegisteredSaveables.Empty();
	DomainBindings.Empty();
	ContextConfigMap.Empty();
	ActiveSlots.Empty();
	AutoSaveRotationIndex.Empty();
	DiscoveredConfigs.Empty();
	ActiveProvider = nullptr;

	Super::Deinitialize();
}

// ============================================================================
// EN: Discovery & Cache
// ES: Descubrimiento y Cache
// ============================================================================

void UPGXSaveSubsystem::DiscoverSaveConfigs()
{
	const UPGXSaveSettings* Settings = GetDefault<UPGXSaveSettings>();

	if (!Settings->SaveContextTable.IsNull())
	{
		// EN: Load from DataTable (deterministic)
		// ES: Cargar desde DataTable (deterministico)
		UDataTable* Table = Settings->SaveContextTable.LoadSynchronous();
		if (IsValid(Table))
		{
			TArray<FPGXSaveContextRow*> Rows;
			Table->GetAllRows<FPGXSaveContextRow>(TEXT("SaveDiscovery"), Rows);

			for (const FPGXSaveContextRow* Row : Rows)
			{
				if (!Row || Row->ConfigRef.IsNull()) { continue; }

				UPGXSaveConfig* Config = Row->ConfigRef.LoadSynchronous();
				if (!IsValid(Config)) { continue; }

				if (!Config->ContextTag.IsValid())
				{
					PGX_LOG_WARNING(LogPGXSave, TEXT("[Discovery] SaveConfig '%s' has no ContextTag — skipped"),
						*Config->GetName());
					continue;
				}

				if (ContextConfigMap.Contains(Config->ContextTag))
				{
					PGX_LOG_WARNING(LogPGXSave, TEXT("[Discovery] Duplicate ContextTag '%s' in '%s' — already registered, skipped"),
						*Config->ContextTag.ToString(), *Config->GetName());
					continue;
				}

				DiscoveredConfigs.Add(Config);
				ContextConfigMap.Add(Config->ContextTag, Config);

				PGX_LOG_INFO(LogPGXSave, TEXT("[Discovery] Registered context '%s' (%s) — %d domains, mode: %s"),
					*Config->ContextTag.ToString(),
					*Config->GetName(),
					Config->SaveDomains.Num(),
					*UEnum::GetValueAsString(Config->SaveMode));
			}

			PGX_LOG_INFO(LogPGXSave, TEXT("[Save] %d contexts resolved from DataTable."), ContextConfigMap.Num());
			return;
		}
		PGX_LOG_WARNING(LogPGXSave, TEXT("[Save] DataTable assigned in Settings but failed to load: %s"),
			*Settings->SaveContextTable.ToString());
	}

	// EN: AssetRegistry fallback (deprecated)
	// ES: Fallback AssetRegistry (deprecated)
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	const IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	FARFilter Filter;
	Filter.ClassPaths.Add(UPGXSaveConfig::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true;

	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssets(Filter, AssetDataList);

	for (const FAssetData& AssetData : AssetDataList)
	{
		UPGXSaveConfig* Config = Cast<UPGXSaveConfig>(AssetData.GetAsset());
		if (!Config)
		{
			PGX_LOG_WARNING(LogPGXSave, TEXT("[Discovery] Failed to load SaveConfig: %s"), *AssetData.GetObjectPathString());
			continue;
		}

		if (!Config->ContextTag.IsValid())
		{
			PGX_LOG_WARNING(LogPGXSave, TEXT("[Discovery] SaveConfig '%s' has no ContextTag — skipped"),
				*Config->GetName());
			continue;
		}

		if (ContextConfigMap.Contains(Config->ContextTag))
		{
			PGX_LOG_WARNING(LogPGXSave, TEXT("[Discovery] Duplicate ContextTag '%s' in '%s' — already registered by another config, skipped"),
				*Config->ContextTag.ToString(), *Config->GetName());
			continue;
		}

		DiscoveredConfigs.Add(Config);
		ContextConfigMap.Add(Config->ContextTag, Config);

		PGX_LOG_INFO(LogPGXSave, TEXT("[Discovery] Registered context '%s' (%s) — %d domains, mode: %s"),
			*Config->ContextTag.ToString(),
			*Config->GetName(),
			Config->SaveDomains.Num(),
			*UEnum::GetValueAsString(Config->SaveMode));
	}

	if (ContextConfigMap.Num() > 0)
	{
		PGX_LOG_WARNING(LogPGXSave, TEXT("[Save] %d save configs auto-discovered from AssetRegistry. "
			"Configure a DataTable in Project Settings > PGX > Save System to remove this warning. "
			"Auto-discovery is deprecated and will be removed in v0.6.0."),
			ContextConfigMap.Num());
	}
}

void UPGXSaveSubsystem::BuildDomainCache()
{
	for (UPGXSaveConfig* Config : DiscoveredConfigs)
	{
		if (!Config) continue;

		for (const FPGXSaveDomainEntry& DomainEntry : Config->SaveDomains)
		{
			if (!DomainEntry.DomainTag.IsValid())
			{
				PGX_LOG_WARNING(LogPGXSave, TEXT("[DomainCache] Domain in '%s' has no tag — skipped"),
					*Config->GetName());
				continue;
			}

			if (!DomainEntry.SaveGameClass)
			{
				PGX_LOG_WARNING(LogPGXSave, TEXT("[DomainCache] Domain '%s' in '%s' has no SaveGameClass — skipped"),
					*DomainEntry.DomainTag.ToString(), *Config->GetName());
				continue;
			}

			if (DomainBindings.Contains(DomainEntry.DomainTag))
			{
				PGX_LOG_WARNING(LogPGXSave, TEXT("[DomainCache] Duplicate domain tag '%s' — already registered, skipped"),
					*DomainEntry.DomainTag.ToString());
				continue;
			}

			FPGXDomainBinding Binding;
			Binding.OwningConfig = Config;
			Binding.DomainEntry = DomainEntry;
			Binding.ActiveInstance = nullptr;

			DomainBindings.Add(DomainEntry.DomainTag, Binding);
		}
	}
}

void UPGXSaveSubsystem::CreateProvider()
{
	// EN: Use the first config's provider class, or Default if none specified
	// ES: Usar la clase provider del primer config, o Default si no se especifico
	TSubclassOf<UPGXSaveProvider> ProviderClass = nullptr;

	for (const UPGXSaveConfig* Config : DiscoveredConfigs)
	{
		if (Config && Config->SaveProviderClass)
		{
			ProviderClass = Config->SaveProviderClass;
			break;
		}
	}

	if (!ProviderClass)
	{
		ProviderClass = UPGXSaveProvider_Default::StaticClass();
	}

	ActiveProvider = NewObject<UPGXSaveProvider>(this, ProviderClass);
}

// ============================================================================
// EN: Internal helpers
// ES: Helpers internos
// ============================================================================

UPGXSaveConfig* UPGXSaveSubsystem::FindConfigByContextTag(FGameplayTag ContextTag) const
{
	UPGXSaveConfig* const* Found = ContextConfigMap.Find(ContextTag);
	return Found ? *Found : nullptr;
}

FPGXDomainBinding* UPGXSaveSubsystem::FindBindingByDomainTag(FGameplayTag DomainTag)
{
	return DomainBindings.Find(DomainTag);
}

const FPGXDomainBinding* UPGXSaveSubsystem::FindBindingByDomainTag(FGameplayTag DomainTag) const
{
	return DomainBindings.Find(DomainTag);
}

UPGXSaveGame* UPGXSaveSubsystem::EnsureSaveGameInstance(FPGXDomainBinding& Binding)
{
	if (Binding.ActiveInstance)
	{
		return Binding.ActiveInstance;
	}

	if (!Binding.DomainEntry.SaveGameClass)
	{
		PGX_LOG_ERROR(LogPGXSave, TEXT("[EnsureInstance] No SaveGameClass for domain '%s'"),
			*Binding.DomainEntry.DomainTag.ToString());
		return nullptr;
	}

	Binding.ActiveInstance = NewObject<UPGXSaveGame>(this, Binding.DomainEntry.SaveGameClass);
	if (Binding.ActiveInstance)
	{
		Binding.ActiveInstance->DomainTag = Binding.DomainEntry.DomainTag;
	}

	return Binding.ActiveInstance;
}

void UPGXSaveSubsystem::NotifySaveablesPreSave(FGameplayTag DomainTag, UPGXSaveGame* SaveGame)
{
	TArray<TWeakObjectPtr<UObject>> Saveables;
	RegisteredSaveables.MultiFind(DomainTag, Saveables);

	for (auto It = Saveables.CreateIterator(); It; ++It)
	{
		if (It->IsValid())
		{
			if (IPGXSaveable* Saveable = Cast<IPGXSaveable>(It->Get()))
			{
				Saveable->OnPreSave(SaveGame, DomainTag);
			}
		}
	}
}

void UPGXSaveSubsystem::NotifySaveablesPostLoad(FGameplayTag DomainTag, UPGXSaveGame* SaveGame)
{
	TArray<TWeakObjectPtr<UObject>> Saveables;
	RegisteredSaveables.MultiFind(DomainTag, Saveables);

	for (auto It = Saveables.CreateIterator(); It; ++It)
	{
		if (It->IsValid())
		{
			if (IPGXSaveable* Saveable = Cast<IPGXSaveable>(It->Get()))
			{
				Saveable->OnPostLoad(SaveGame, DomainTag);
			}
		}
	}
}

FPGXSaveSlotInfo UPGXSaveSubsystem::BuildSlotInfoForSave(const UPGXSaveConfig* Config,
	const FString& SlotName, int32 DomainCount, int64 TotalBytes) const
{
	FPGXSaveSlotInfo Info;
	Info.SlotName = SlotName;
	Info.DisplayName = FText::FromString(SlotName);
	Info.ContextTag = Config->ContextTag;
	Info.SaveDate = FDateTime::Now();
	Info.SaveVersion = Config->CurrentSaveVersion;
	Info.DomainCount = DomainCount;
	Info.TotalSizeBytes = TotalBytes;
	return Info;
}

// ============================================================================
// EN: SaveContext pipeline
// ES: Pipeline de SaveContext
// ============================================================================

EPGXSaveResult UPGXSaveSubsystem::SaveContext(FGameplayTag ContextTag, const FString& SlotName)
{
	if (bSaveInProgress)
	{
		PGX_LOG_WARNING(LogPGXSave, TEXT("[SaveContext] Save already in progress"));
		return EPGXSaveResult::InProgress;
	}

	// --- Step 1: Context lookup ---
	UPGXSaveConfig* Config = FindConfigByContextTag(ContextTag);
	if (!Config)
	{
		PGX_LOG_ERROR(LogPGXSave, TEXT("[SaveContext] Context not found: %s"), *ContextTag.ToString());
		return EPGXSaveResult::ContextNotFound;
	}

	if (!ActiveProvider)
	{
		PGX_LOG_ERROR(LogPGXSave, TEXT("[SaveContext] No active provider"));
		return EPGXSaveResult::ProviderError;
	}

	const int32 TotalSteps = 8;
	const FString EffectiveSlot = (Config->SaveMode == EPGXSaveMode::SingleSlot) ? TEXT("Save") : SlotName;

	bSaveInProgress = true;

	PGX_SAVE_VERBOSE(Config, 1, TotalSteps, TEXT("Context lookup — %s (%d domains)"),
		*Config->GetName(), Config->SaveDomains.Num());

	const FString BasePath = ActiveProvider->GetBaseSaveDirectory();
	int64 TotalBytes = 0;
	int32 DomainsSaved = 0;
	bool bAnyRequiredFailed = false;

	// --- Step 2: Backup existing files ---
	if (Config->bCreateBackupBeforeSave)
	{
		PGX_SAVE_VERBOSE(Config, 2, TotalSteps, TEXT("Creating backups for %d domains"), Config->SaveDomains.Num());

		for (const FPGXSaveDomainEntry& DomainEntry : Config->SaveDomains)
		{
			const FString FilePath = UPGXSaveSerializer::ResolveSavePath(Config, BasePath, EffectiveSlot, DomainEntry.DomainTag);
			FPGXSaveBackupManager::CreateBackup(FilePath, Config->MaxBackupsPerSlot);
		}
	}
	else
	{
		PGX_SAVE_VERBOSE(Config, 2, TotalSteps, TEXT("Backup disabled — skipped"));
	}

	// --- Step 3: OnPreSave callbacks ---
	PGX_SAVE_VERBOSE(Config, 3, TotalSteps, TEXT("OnPreSave callbacks"));

	for (const FPGXSaveDomainEntry& DomainEntry : Config->SaveDomains)
	{
		FPGXDomainBinding* Binding = FindBindingByDomainTag(DomainEntry.DomainTag);
		if (!Binding) continue;

		UPGXSaveGame* SaveGame = EnsureSaveGameInstance(*Binding);
		if (SaveGame)
		{
			NotifySaveablesPreSave(DomainEntry.DomainTag, SaveGame);
		}
	}

	// --- Steps 4-7: Serialize, compress, checksum, write (per domain) ---
	for (const FPGXSaveDomainEntry& DomainEntry : Config->SaveDomains)
	{
		FPGXDomainBinding* Binding = FindBindingByDomainTag(DomainEntry.DomainTag);
		if (!Binding || !Binding->ActiveInstance)
		{
			if (DomainEntry.bRequired)
			{
				PGX_LOG_ERROR(LogPGXSave, TEXT("[SaveContext] Required domain '%s' has no instance"),
					*DomainEntry.DomainTag.ToString());
				bAnyRequiredFailed = true;
			}
			continue;
		}

		UPGXSaveGame* SaveGame = Binding->ActiveInstance;

		// EN: Update metadata before serialize
		// ES: Actualizar metadata antes de serializar
		SaveGame->SaveTimestamp = FDateTime::Now();
		SaveGame->SaveFormatVersion = Config->CurrentSaveVersion;

		// Step 4: Serialize
		TArray<uint8> Bytes;
		if (!UPGXSaveSerializer::SerializeToMemory(SaveGame, Bytes))
		{
			PGX_LOG_ERROR(LogPGXSave, TEXT("[SaveContext] Serialize failed for domain '%s'"),
				*DomainEntry.DomainTag.ToString());
			if (DomainEntry.bRequired) bAnyRequiredFailed = true;
			continue;
		}

		PGX_SAVE_VERBOSE(Config, 4, TotalSteps, TEXT("Serialize — %s (%d bytes)"),
			*DomainEntry.DomainTag.ToString(), Bytes.Num());

		// Step 5: Compress (if enabled)
		TArray<uint8> FinalBytes;
		if (Config->bCompressSaveData)
		{
			if (UPGXSaveSerializer::Compress(Bytes, FinalBytes))
			{
				PGX_SAVE_VERBOSE(Config, 5, TotalSteps, TEXT("Compress — %d -> %d bytes (%d%% reduction)"),
					Bytes.Num(), FinalBytes.Num(),
					Bytes.Num() > 0 ? 100 - (FinalBytes.Num() * 100 / Bytes.Num()) : 0);
			}
			else
			{
				PGX_LOG_WARNING(LogPGXSave, TEXT("[SaveContext] Compression failed for '%s', saving uncompressed"),
					*DomainEntry.DomainTag.ToString());
				FinalBytes = MoveTemp(Bytes);
			}
		}
		else
		{
			FinalBytes = MoveTemp(Bytes);
			PGX_SAVE_VERBOSE(Config, 5, TotalSteps, TEXT("Compress — disabled, passthrough"));
		}

		// Step 6: Checksum
		const FString Checksum = UPGXSaveSerializer::GenerateChecksum(FinalBytes);

		PGX_SAVE_VERBOSE(Config, 6, TotalSteps, TEXT("Checksum — CRC32:%s"), *Checksum);

		// Step 7: Write to disk
		const FString FilePath = UPGXSaveSerializer::ResolveSavePath(Config, BasePath, EffectiveSlot, DomainEntry.DomainTag);
		const FString Directory = FPaths::GetPath(FilePath);
		ActiveProvider->EnsureDirectoryExists(Directory);

		if (!ActiveProvider->SaveBytes(FilePath, FinalBytes))
		{
			PGX_LOG_ERROR(LogPGXSave, TEXT("[SaveContext] Write failed for '%s' to '%s'"),
				*DomainEntry.DomainTag.ToString(), *FilePath);
			if (DomainEntry.bRequired) bAnyRequiredFailed = true;
			continue;
		}

		// EN: Write checksum sidecar file (companion to save data, not embedded in binary format)
		// ES: Escribir archivo sidecar de checksum (companero de datos de guardado, no embebido en formato binario)
		const FString ChecksumPath = FilePath + TEXT(".checksum");
		FFileHelper::SaveStringToFile(Checksum, *ChecksumPath);

		PGX_SAVE_VERBOSE(Config, 7, TotalSteps, TEXT("Write — %s (%d bytes)"),
			*FPaths::GetCleanFilename(FilePath), FinalBytes.Num());

		TotalBytes += FinalBytes.Num();
		DomainsSaved++;

		// EN: Log transaction
		// ES: Registrar transaccion
		const FString SlotDir = UPGXSaveSerializer::ResolveSlotDirectory(Config, BasePath, EffectiveSlot);
		const FString TransactionLogPath = FPGXSaveBackupManager::ResolveTransactionLogPath(
			FPaths::GetPath(UPGXSaveSerializer::ResolveIndexPath(Config, BasePath)));

		FPGXBackupEntry TransactionEntry;
		TransactionEntry.SlotName = EffectiveSlot;
		TransactionEntry.Timestamp = FDateTime::Now();
		TransactionEntry.Checksum = Checksum;
		TransactionEntry.Operation = EPGXSaveOperation::Save;
		TransactionEntry.Result = EPGXSaveResult::Success;
		TransactionEntry.FilePath = FilePath;
		FPGXSaveBackupManager::LogTransaction(TransactionLogPath, TransactionEntry);
	}

	// --- Step 8: Finalize ---
	const EPGXSaveResult Result = bAnyRequiredFailed ? EPGXSaveResult::Failed : EPGXSaveResult::Success;

	if (Result == EPGXSaveResult::Success)
	{
		SetActiveSlot(ContextTag, EffectiveSlot);
	}

	PGX_SAVE_VERBOSE(Config, 8, TotalSteps, TEXT("COMPLETE — %s saved (%d/%d domains, %lld bytes, result: %s)"),
		*EffectiveSlot, DomainsSaved, Config->SaveDomains.Num(), TotalBytes,
		*UEnum::GetValueAsString(Result));

	if (!Config->bVerboseSaveDebug)
	{
		PGX_LOG_INFO(LogPGXSave, TEXT("[SaveContext] %s — %s (%d domains, %lld bytes)"),
			*ContextTag.ToString(), *EffectiveSlot, DomainsSaved, TotalBytes);
	}

	bSaveInProgress = false;

	// EN: Broadcast delegates
	// ES: Broadcast de delegates
	OnSaveCompleted.Broadcast(EffectiveSlot, Result);
	OnSaveCompletedNative.Broadcast(EffectiveSlot, Result);

	return Result;
}

// ============================================================================
// EN: LoadContext pipeline
// ES: Pipeline de LoadContext
// ============================================================================

EPGXSaveResult UPGXSaveSubsystem::LoadContext(FGameplayTag ContextTag, const FString& SlotName)
{
	if (bLoadInProgress)
	{
		PGX_LOG_WARNING(LogPGXSave, TEXT("[LoadContext] Load already in progress"));
		return EPGXSaveResult::InProgress;
	}

	// --- Step 1: Context lookup ---
	UPGXSaveConfig* Config = FindConfigByContextTag(ContextTag);
	if (!Config)
	{
		PGX_LOG_ERROR(LogPGXSave, TEXT("[LoadContext] Context not found: %s"), *ContextTag.ToString());
		return EPGXSaveResult::ContextNotFound;
	}

	if (!ActiveProvider)
	{
		PGX_LOG_ERROR(LogPGXSave, TEXT("[LoadContext] No active provider"));
		return EPGXSaveResult::ProviderError;
	}

	const int32 TotalSteps = 8;
	const FString EffectiveSlot = (Config->SaveMode == EPGXSaveMode::SingleSlot) ? TEXT("Save") : SlotName;

	bLoadInProgress = true;

	PGX_SAVE_VERBOSE(Config, 1, TotalSteps, TEXT("Context lookup — %s (%d domains)"),
		*Config->GetName(), Config->SaveDomains.Num());

	const FString BasePath = ActiveProvider->GetBaseSaveDirectory();
	int32 DomainsLoaded = 0;
	bool bAnyRequiredFailed = false;
	// EN: Track corruption distinct from generic failure so the final result code
	//     can return Corrupted (not Failed) when integrity validation rejects bytes.
	//     the save-integrity contract.
	// ES: Trackear corrupcion distinta del fallo generico para que el codigo de
	//     resultado final pueda retornar Corrupted (no Failed) cuando la validacion
	//     de integridad rechaza los bytes. Contrato de integridad de save.
	bool bAnyRequiredCorrupted = false;

	// --- Steps 2-6: Read, checksum, decompress, deserialize, version migrate (per domain) ---
	for (const FPGXSaveDomainEntry& DomainEntry : Config->SaveDomains)
	{
		const FString FilePath = UPGXSaveSerializer::ResolveSavePath(Config, BasePath, EffectiveSlot, DomainEntry.DomainTag);

		// Step 2: Read bytes from disk
		TArray<uint8> RawBytes;
		if (!ActiveProvider->DoesFileExist(FilePath) || !ActiveProvider->LoadBytes(FilePath, RawBytes))
		{
			if (DomainEntry.bRequired)
			{
				PGX_LOG_ERROR(LogPGXSave, TEXT("[LoadContext] Required domain '%s' file not found: %s"),
					*DomainEntry.DomainTag.ToString(), *FilePath);

				// EN: Try backup recovery
				// ES: Intentar recuperacion de backup
				if (Config->bCreateBackupBeforeSave && FPGXSaveBackupManager::RestoreFromBackup(FilePath))
				{
					PGX_LOG_INFO(LogPGXSave, TEXT("[LoadContext] Restored from backup: %s"), *FilePath);
					if (!ActiveProvider->LoadBytes(FilePath, RawBytes))
					{
						bAnyRequiredFailed = true;
						continue;
					}
				}
				else
				{
					bAnyRequiredFailed = true;
					continue;
				}
			}
			else
			{
				PGX_SAVE_VERBOSE(Config, 2, TotalSteps, TEXT("Read — %s not found (optional, skipped)"),
					*DomainEntry.DomainTag.ToString());
				continue;
			}
		}

		PGX_SAVE_VERBOSE(Config, 2, TotalSteps, TEXT("Read — %s (%d bytes)"),
			*DomainEntry.DomainTag.ToString(), RawBytes.Num());

		// Step 3: Validate checksum against sidecar
		if (Config->bValidateChecksum)
		{
			const FString ChecksumPath = FilePath + TEXT(".checksum");
			FString ExpectedChecksum;
			if (FFileHelper::LoadFileToString(ExpectedChecksum, *ChecksumPath))
			{
				if (!UPGXSaveSerializer::ValidateChecksum(RawBytes, ExpectedChecksum))
				{
					PGX_LOG_ERROR(LogPGXSave, TEXT("[LoadContext] Checksum mismatch for '%s' — data corrupted"),
						*DomainEntry.DomainTag.ToString());
					if (DomainEntry.bRequired)
					{
						bAnyRequiredFailed = true;
						bAnyRequiredCorrupted = true;
					}
					continue;
				}
				PGX_SAVE_VERBOSE(Config, 3, TotalSteps, TEXT("Checksum — validated OK"));
			}
			else
			{
				// EN: No sidecar found (legacy save) — skip gracefully
				// ES: No se encontro sidecar (guardado legacy) — saltar con gracia
				PGX_LOG_INFO(LogPGXSave, TEXT("[LoadContext] No checksum sidecar for '%s' (legacy save, skipping validation)"),
					*DomainEntry.DomainTag.ToString());
			}
		}

		// Step 4: Decompress (if needed)
		TArray<uint8> DeserializeBytes;
		if (Config->bCompressSaveData)
		{
			if (UPGXSaveSerializer::Decompress(RawBytes, DeserializeBytes))
			{
				PGX_SAVE_VERBOSE(Config, 4, TotalSteps, TEXT("Decompress — %d -> %d bytes"),
					RawBytes.Num(), DeserializeBytes.Num());
			}
			else
			{
				PGX_LOG_ERROR(LogPGXSave, TEXT("[LoadContext] Decompression failed for '%s'"),
					*DomainEntry.DomainTag.ToString());
				if (DomainEntry.bRequired) bAnyRequiredFailed = true;
				continue;
			}
		}
		else
		{
			DeserializeBytes = MoveTemp(RawBytes);
		}

		// Step 5: Deserialize
		UPGXSaveGame* LoadedGame = UPGXSaveSerializer::DeserializeFromMemory(DeserializeBytes);
		if (!LoadedGame)
		{
			PGX_LOG_ERROR(LogPGXSave, TEXT("[LoadContext] Deserialize failed for '%s'"),
				*DomainEntry.DomainTag.ToString());
			if (DomainEntry.bRequired) bAnyRequiredFailed = true;
			continue;
		}

		PGX_SAVE_VERBOSE(Config, 5, TotalSteps, TEXT("Deserialize — %s (class: %s, %d KV entries)"),
			*DomainEntry.DomainTag.ToString(),
			*LoadedGame->GetClass()->GetName(),
			LoadedGame->GetKeyValueCount());

		// Step 6: Version migration (if needed)
		if (LoadedGame->SaveFormatVersion != Config->CurrentSaveVersion)
		{
			PGX_SAVE_VERBOSE(Config, 6, TotalSteps, TEXT("Version migrate — %s: v%d -> v%d"),
				*DomainEntry.DomainTag.ToString(),
				LoadedGame->SaveFormatVersion, Config->CurrentSaveVersion);

			const EPGXSaveResult MigrateResult = FPGXSaveVersioning::MigrateToVersion(
				LoadedGame, Config->CurrentSaveVersion);

			if (MigrateResult != EPGXSaveResult::Success)
			{
				PGX_LOG_ERROR(LogPGXSave,
					TEXT("[LoadContext] Version migration failed for '%s': v%d -> v%d (result: %s)"),
					*DomainEntry.DomainTag.ToString(),
					LoadedGame->SaveFormatVersion, Config->CurrentSaveVersion,
					*UEnum::GetValueAsString(MigrateResult));

				if (DomainEntry.bRequired) bAnyRequiredFailed = true;
				continue;
			}
		}
		else
		{
			PGX_SAVE_VERBOSE(Config, 6, TotalSteps, TEXT("Version migrate — %s: v%d (current, skip)"),
				*DomainEntry.DomainTag.ToString(), LoadedGame->SaveFormatVersion);
		}

		// EN: Store the loaded instance in the binding
		// ES: Almacenar la instancia cargada en el binding
		FPGXDomainBinding* Binding = FindBindingByDomainTag(DomainEntry.DomainTag);
		if (Binding)
		{
			Binding->ActiveInstance = LoadedGame;
		}

		DomainsLoaded++;
	}

	// --- Step 7: OnPostLoad callbacks ---
	PGX_SAVE_VERBOSE(Config, 7, TotalSteps, TEXT("OnPostLoad callbacks"));

	for (const FPGXSaveDomainEntry& DomainEntry : Config->SaveDomains)
	{
		const FPGXDomainBinding* Binding = FindBindingByDomainTag(DomainEntry.DomainTag);
		if (Binding && Binding->ActiveInstance)
		{
			NotifySaveablesPostLoad(DomainEntry.DomainTag, Binding->ActiveInstance);
		}
	}

	// --- Step 8: Finalize ---
	// EN: Corruption is reported distinctly from generic failure:
	//     mismatched-integrity payloads must surface as Corrupted, not Failed.
	// ES: La corrupcion se reporta de forma distinta al fallo generico:
	//     payloads con mismatch de integridad deben superficiar como Corrupted, no Failed.
	const EPGXSaveResult Result =
		bAnyRequiredCorrupted ? EPGXSaveResult::Corrupted :
		bAnyRequiredFailed    ? EPGXSaveResult::Failed    :
		                        EPGXSaveResult::Success;

	if (Result == EPGXSaveResult::Success)
	{
		SetActiveSlot(ContextTag, EffectiveSlot);
	}

	PGX_SAVE_VERBOSE(Config, 8, TotalSteps, TEXT("COMPLETE — %s loaded (%d/%d domains, result: %s)"),
		*EffectiveSlot, DomainsLoaded, Config->SaveDomains.Num(),
		*UEnum::GetValueAsString(Result));

	if (!Config->bVerboseSaveDebug)
	{
		PGX_LOG_INFO(LogPGXSave, TEXT("[LoadContext] %s — %s (%d domains)"),
			*ContextTag.ToString(), *EffectiveSlot, DomainsLoaded);
	}

	bLoadInProgress = false;

	// EN: Find first loaded SaveGame instance for broadcast
	// ES: Encontrar primera instancia de SaveGame cargada para broadcast
	UPGXSaveGame* BroadcastInstance = nullptr;
	if (Result == EPGXSaveResult::Success)
	{
		for (const FPGXSaveDomainEntry& DomainEntry : Config->SaveDomains)
		{
			const FPGXDomainBinding* Binding = FindBindingByDomainTag(DomainEntry.DomainTag);
			if (Binding && IsValid(Binding->ActiveInstance))
			{
				BroadcastInstance = Binding->ActiveInstance;
				break;
			}
		}
	}

	// EN: Broadcast delegates
	// ES: Broadcast de delegates
	OnLoadCompleted.Broadcast(EffectiveSlot, Result, BroadcastInstance);
	OnLoadCompletedNative.Broadcast(EffectiveSlot, Result, BroadcastInstance);

	return Result;
}

// ============================================================================
// EN: DeleteSlot
// ES: Eliminar slot
// ============================================================================

EPGXSaveResult UPGXSaveSubsystem::DeleteSlot(FGameplayTag ContextTag, const FString& SlotName)
{
	UPGXSaveConfig* Config = FindConfigByContextTag(ContextTag);
	if (!Config)
	{
		return EPGXSaveResult::ContextNotFound;
	}

	if (!ActiveProvider)
	{
		return EPGXSaveResult::ProviderError;
	}

	const FString BasePath = ActiveProvider->GetBaseSaveDirectory();
	bool bAllDeleted = true;

	for (const FPGXSaveDomainEntry& DomainEntry : Config->SaveDomains)
	{
		const FString FilePath = UPGXSaveSerializer::ResolveSavePath(Config, BasePath, SlotName, DomainEntry.DomainTag);

		if (ActiveProvider->DoesFileExist(FilePath))
		{
			// EN: Delete backups too
			// ES: Eliminar backups tambien
			FPGXSaveBackupManager::CleanupAllBackups(FilePath);

			if (!ActiveProvider->DeleteFile(FilePath))
			{
				bAllDeleted = false;
			}
		}
	}

	PGX_LOG_INFO(LogPGXSave, TEXT("[DeleteSlot] %s — %s (success: %s)"),
		*ContextTag.ToString(), *SlotName, bAllDeleted ? TEXT("true") : TEXT("false"));

	// EN: Clear active slot if it was the deleted one
	// ES: Limpiar slot activo si era el eliminado
	if (const FString* Active = ActiveSlots.Find(ContextTag))
	{
		if (*Active == SlotName)
		{
			ActiveSlots.Remove(ContextTag);
		}
	}

	OnSlotDeleted.Broadcast(SlotName);
	OnSlotDeletedNative.Broadcast(SlotName);

	return bAllDeleted ? EPGXSaveResult::Success : EPGXSaveResult::Failed;
}

// ============================================================================
// EN: CopySlot
// ES: Copiar slot
// ============================================================================

EPGXSaveResult UPGXSaveSubsystem::CopySlot(FGameplayTag ContextTag,
	const FString& SourceSlotName, const FString& DestSlotName)
{
	UPGXSaveConfig* Config = FindConfigByContextTag(ContextTag);
	if (!Config)
	{
		return EPGXSaveResult::ContextNotFound;
	}

	if (!ActiveProvider)
	{
		return EPGXSaveResult::ProviderError;
	}

	if (SourceSlotName == DestSlotName)
	{
		PGX_LOG_WARNING(LogPGXSave, TEXT("[CopySlot] Source and destination are the same: %s"), *SourceSlotName);
		return EPGXSaveResult::Failed;
	}

	const FString BasePath = ActiveProvider->GetBaseSaveDirectory();
	int32 FilesCopied = 0;
	bool bAnyFailed = false;

	for (const FPGXSaveDomainEntry& DomainEntry : Config->SaveDomains)
	{
		const FString SourcePath = UPGXSaveSerializer::ResolveSavePath(Config, BasePath, SourceSlotName, DomainEntry.DomainTag);
		const FString DestPath = UPGXSaveSerializer::ResolveSavePath(Config, BasePath, DestSlotName, DomainEntry.DomainTag);

		if (!ActiveProvider->DoesFileExist(SourcePath))
		{
			continue;
		}

		// EN: Read source bytes
		// ES: Leer bytes fuente
		TArray<uint8> Bytes;
		if (!ActiveProvider->LoadBytes(SourcePath, Bytes))
		{
			if (DomainEntry.bRequired) bAnyFailed = true;
			continue;
		}

		// EN: Write to destination
		// ES: Escribir en destino
		const FString DestDir = FPaths::GetPath(DestPath);
		ActiveProvider->EnsureDirectoryExists(DestDir);

		if (!ActiveProvider->SaveBytes(DestPath, Bytes))
		{
			if (DomainEntry.bRequired) bAnyFailed = true;
			continue;
		}

		FilesCopied++;
	}

	// EN: Log the transaction
	// ES: Registrar la transaccion
	const EPGXSaveResult Result = bAnyFailed ? EPGXSaveResult::Failed : EPGXSaveResult::Success;

	const FString TransactionLogPath = FPGXSaveBackupManager::ResolveTransactionLogPath(
		FPaths::GetPath(UPGXSaveSerializer::ResolveIndexPath(Config, BasePath)));

	FPGXBackupEntry TransactionEntry;
	TransactionEntry.SlotName = FString::Printf(TEXT("%s -> %s"), *SourceSlotName, *DestSlotName);
	TransactionEntry.Timestamp = FDateTime::Now();
	TransactionEntry.Operation = EPGXSaveOperation::Copy;
	TransactionEntry.Result = Result;
	FPGXSaveBackupManager::LogTransaction(TransactionLogPath, TransactionEntry);

	PGX_LOG_INFO(LogPGXSave, TEXT("[CopySlot] %s — %s -> %s (%d files, result: %s)"),
		*ContextTag.ToString(), *SourceSlotName, *DestSlotName, FilesCopied,
		*UEnum::GetValueAsString(Result));

	return Result;
}

// ============================================================================
// EN: Quick Save / Load
// ES: Guardado / Carga rapida
// ============================================================================

EPGXSaveResult UPGXSaveSubsystem::QuickSave(FGameplayTag ContextTag)
{
	const UPGXSaveConfig* Config = FindConfigByContextTag(ContextTag);
	if (!Config)
	{
		return EPGXSaveResult::ContextNotFound;
	}

	if (!Config->bEnableQuickSave)
	{
		PGX_LOG_WARNING(LogPGXSave, TEXT("[QuickSave] Quick save disabled for context: %s"),
			*ContextTag.ToString());
		return EPGXSaveResult::Failed;
	}

	return SaveContext(ContextTag, Config->QuickSaveSlotName);
}

EPGXSaveResult UPGXSaveSubsystem::QuickLoad(FGameplayTag ContextTag)
{
	const UPGXSaveConfig* Config = FindConfigByContextTag(ContextTag);
	if (!Config)
	{
		return EPGXSaveResult::ContextNotFound;
	}

	if (!Config->bEnableQuickSave)
	{
		PGX_LOG_WARNING(LogPGXSave, TEXT("[QuickLoad] Quick save disabled for context: %s"),
			*ContextTag.ToString());
		return EPGXSaveResult::Failed;
	}

	return LoadContext(ContextTag, Config->QuickSaveSlotName);
}

void UPGXSaveSubsystem::QuickSaveAsync(FGameplayTag ContextTag)
{
	const UPGXSaveConfig* Config = FindConfigByContextTag(ContextTag);
	if (!Config)
	{
		PGX_LOG_WARNING(LogPGXSave, TEXT("[QuickSaveAsync] Context not found: %s"), *ContextTag.ToString());
		return;
	}

	if (!Config->bEnableQuickSave)
	{
		PGX_LOG_WARNING(LogPGXSave, TEXT("[QuickSaveAsync] Quick save disabled for context: %s"),
			*ContextTag.ToString());
		return;
	}

	SaveContextAsync(ContextTag, Config->QuickSaveSlotName);
}

void UPGXSaveSubsystem::QuickLoadAsync(FGameplayTag ContextTag)
{
	const UPGXSaveConfig* Config = FindConfigByContextTag(ContextTag);
	if (!Config)
	{
		PGX_LOG_WARNING(LogPGXSave, TEXT("[QuickLoadAsync] Context not found: %s"), *ContextTag.ToString());
		return;
	}

	if (!Config->bEnableQuickSave)
	{
		PGX_LOG_WARNING(LogPGXSave, TEXT("[QuickLoadAsync] Quick save disabled for context: %s"),
			*ContextTag.ToString());
		return;
	}

	LoadContextAsync(ContextTag, Config->QuickSaveSlotName);
}

// ============================================================================
// EN: Slot queries
// ES: Consultas de slots
// ============================================================================

TArray<FPGXSaveSlotInfo> UPGXSaveSubsystem::GetAllSlots(FGameplayTag ContextTag) const
{
	TArray<FPGXSaveSlotInfo> Slots;

	const UPGXSaveConfig* Config = FindConfigByContextTag(ContextTag);
	if (!Config || !ActiveProvider)
	{
		return Slots;
	}

	const FString BasePath = ActiveProvider->GetBaseSaveDirectory();
	const FString ContextDir = Config->ContextTag.IsValid()
		? Config->ContextTag.ToString().Replace(TEXT("."), TEXT("/"))
		: TEXT("Default");
	const FString ContextPath = FPaths::Combine(BasePath, Config->BaseDirectory, ContextDir);

	if (Config->SaveMode == EPGXSaveMode::SingleSlot)
	{
		// EN: SingleSlot has one implicit slot
		// ES: SingleSlot tiene un slot implicito
		if (Config->SaveDomains.Num() > 0)
		{
			const FString FirstDomainPath = UPGXSaveSerializer::ResolveSavePath(
				Config, BasePath, TEXT("Save"), Config->SaveDomains[0].DomainTag);
			if (ActiveProvider->DoesFileExist(FirstDomainPath))
			{
				FPGXSaveSlotInfo Info;
				Info.SlotName = TEXT("Save");
				Info.DisplayName = FText::FromString(TEXT("Save"));
				Info.ContextTag = Config->ContextTag;
				Slots.Add(Info);
			}
		}
	}
	else
	{
		// EN: MultiSlot / SessionBased: enumerate subdirectories
		// ES: MultiSlot / SessionBased: enumerar subdirectorios
		TArray<FString> SubDirs;
		IFileManager::Get().FindFiles(SubDirs, *FPaths::Combine(ContextPath, TEXT("*")),
			/*bFiles=*/ false, /*bDirectories=*/ true);

		for (const FString& SubDir : SubDirs)
		{
			if (SubDir.StartsWith(TEXT("_"))) continue; // EN: Skip _Index, _Transactions / ES: Saltar _Index, _Transactions

			FPGXSaveSlotInfo Info;
			Info.SlotName = SubDir;
			Info.DisplayName = FText::FromString(SubDir);
			Info.ContextTag = Config->ContextTag;
			Slots.Add(Info);
		}
	}

	return Slots;
}

FPGXSaveSlotInfo UPGXSaveSubsystem::GetSlotInfo(FGameplayTag ContextTag, const FString& SlotName) const
{
	const TArray<FPGXSaveSlotInfo> AllSlots = GetAllSlots(ContextTag);
	for (const FPGXSaveSlotInfo& Info : AllSlots)
	{
		if (Info.SlotName == SlotName)
		{
			return Info;
		}
	}
	return FPGXSaveSlotInfo();
}

bool UPGXSaveSubsystem::DoesSlotExist(FGameplayTag ContextTag, const FString& SlotName) const
{
	const UPGXSaveConfig* Config = FindConfigByContextTag(ContextTag);
	if (!Config || !ActiveProvider)
	{
		return false;
	}

	// EN: Check if any domain file exists for this slot
	// ES: Comprobar si existe algun archivo de dominio para este slot
	const FString BasePath = ActiveProvider->GetBaseSaveDirectory();
	for (const FPGXSaveDomainEntry& DomainEntry : Config->SaveDomains)
	{
		const FString FilePath = UPGXSaveSerializer::ResolveSavePath(Config, BasePath, SlotName, DomainEntry.DomainTag);
		if (ActiveProvider->DoesFileExist(FilePath))
		{
			return true;
		}
	}

	return false;
}

FString UPGXSaveSubsystem::GetNextAvailableSlotName(FGameplayTag ContextTag) const
{
	const UPGXSaveConfig* Config = FindConfigByContextTag(ContextTag);
	if (!Config)
	{
		return TEXT("");
	}

	if (Config->SaveMode == EPGXSaveMode::SingleSlot)
	{
		return TEXT("Save");
	}

	if (Config->SaveMode == EPGXSaveMode::SessionBased)
	{
		return UPGXSaveSerializer::GenerateSessionSlotName();
	}

	// EN: MultiSlot: find next available number following the pattern
	// ES: MultiSlot: encontrar siguiente numero disponible siguiendo el patron
	for (int32 i = 1; i <= Config->MaxSaveSlots; ++i)
	{
		FString Candidate = Config->SlotNamePattern;
		Candidate = Candidate.Replace(TEXT("{NN}"), *FString::Printf(TEXT("%02d"), i));

		if (!DoesSlotExist(ContextTag, Candidate))
		{
			return Candidate;
		}
	}

	return TEXT(""); // EN: All slots full / ES: Todos los slots llenos
}

// ============================================================================
// EN: Direct SaveGame access
// ES: Acceso directo a SaveGame
// ============================================================================

UPGXSaveGame* UPGXSaveSubsystem::GetSaveGame(FGameplayTag DomainTag)
{
	FPGXDomainBinding* Binding = FindBindingByDomainTag(DomainTag);
	if (!Binding)
	{
		PGX_LOG_WARNING(LogPGXSave, TEXT("[GetSaveGame] Domain not found: %s"), *DomainTag.ToString());
		return nullptr;
	}

	return EnsureSaveGameInstance(*Binding);
}

bool UPGXSaveSubsystem::HasData(FGameplayTag DomainTag, FName Key) const
{
	const FPGXDomainBinding* Binding = FindBindingByDomainTag(DomainTag);
	if (!Binding || !Binding->ActiveInstance)
	{
		return false;
	}
	return Binding->ActiveInstance->HasKey(Key);
}

void UPGXSaveSubsystem::ClearDomain(FGameplayTag DomainTag)
{
	FPGXDomainBinding* Binding = FindBindingByDomainTag(DomainTag);
	if (Binding && Binding->ActiveInstance)
	{
		Binding->ActiveInstance->ClearAllKeyValueData();
	}
}

// ============================================================================
// EN: IPGXSaveable registration
// ES: Registro de IPGXSaveable
// ============================================================================

void UPGXSaveSubsystem::RegisterSaveable(UObject* Saveable, FGameplayTag DomainTag)
{
	if (!Saveable)
	{
		PGX_LOG_WARNING(LogPGXSave, TEXT("[RegisterSaveable] Null object"));
		return;
	}

	if (!Saveable->GetClass()->ImplementsInterface(UPGXSaveable::StaticClass()))
	{
		PGX_LOG_WARNING(LogPGXSave, TEXT("[RegisterSaveable] Object '%s' does not implement IPGXSaveable"),
			*Saveable->GetName());
		return;
	}

	if (!DomainTag.IsValid())
	{
		PGX_LOG_WARNING(LogPGXSave, TEXT("[RegisterSaveable] Invalid domain tag for '%s'"),
			*Saveable->GetName());
		return;
	}

	RegisteredSaveables.AddUnique(DomainTag, Saveable);

	PGX_LOG_VERBOSE(LogPGXSave, TEXT("[RegisterSaveable] '%s' -> domain '%s'"),
		*Saveable->GetName(), *DomainTag.ToString());
}

void UPGXSaveSubsystem::UnregisterSaveable(UObject* Saveable)
{
	if (!Saveable) return;

	// EN: Remove from all domains
	// ES: Remover de todos los dominios
	TArray<FGameplayTag> KeysToCheck;
	RegisteredSaveables.GetKeys(KeysToCheck);

	for (const FGameplayTag& Key : KeysToCheck)
	{
		RegisteredSaveables.RemoveSingle(Key, Saveable);
	}
}

// ============================================================================
// EN: Active state
// ES: Estado activo
// ============================================================================

FString UPGXSaveSubsystem::GetActiveSlotName(FGameplayTag ContextTag) const
{
	const FString* Found = ActiveSlots.Find(ContextTag);
	return Found ? *Found : TEXT("");
}

void UPGXSaveSubsystem::SetActiveSlot(FGameplayTag ContextTag, const FString& SlotName)
{
	ActiveSlots.FindOrAdd(ContextTag) = SlotName;
}

// ============================================================================
// EN: Auto-save control
// ES: Control de auto-guardado
// ============================================================================

void UPGXSaveSubsystem::SetAutoSaveEnabled(FGameplayTag ContextTag, bool bEnabled)
{
	UPGXSaveConfig* Config = FindConfigByContextTag(ContextTag);
	if (!Config)
	{
		return;
	}

	Config->bAutoSaveEnabled = bEnabled;

	if (bEnabled)
	{
		StartAutoSaveTimer(ContextTag);
	}
	else
	{
		StopAutoSaveTimer(ContextTag);
	}

	PGX_LOG_INFO(LogPGXSave, TEXT("[AutoSave] %s for context '%s'"),
		bEnabled ? TEXT("Enabled") : TEXT("Disabled"), *ContextTag.ToString());
}

void UPGXSaveSubsystem::TriggerAutoSave(FGameplayTag ContextTag)
{
	UPGXSaveConfig* Config = FindConfigByContextTag(ContextTag);
	if (!Config || !Config->bAutoSaveEnabled)
	{
		return;
	}

	// EN: Determine auto-save slot name via rotation
	// ES: Determinar nombre de slot de auto-guardado via rotacion
	int32& RotationIndex = AutoSaveRotationIndex.FindOrAdd(ContextTag, 0);
	RotationIndex = (RotationIndex % Config->MaxAutoSaveSlots) + 1;

	const FString AutoSlotName = FString::Printf(TEXT("AutoSave_%02d"), RotationIndex);

	PGX_LOG_INFO(LogPGXSave, TEXT("[AutoSave] Triggered for '%s' -> %s"),
		*ContextTag.ToString(), *AutoSlotName);

	OnAutoSaveTriggered.Broadcast(ContextTag);
	OnAutoSaveTriggeredNative.Broadcast(ContextTag);
	SaveContext(ContextTag, AutoSlotName);
}

bool UPGXSaveSubsystem::IsAutoSaveActive(FGameplayTag ContextTag) const
{
	return AutoSaveTickerHandles.Contains(ContextTag);
}

// ============================================================================
// EN: Auto-save timer management (FTSTicker, world-independent)
// ES: Gestion de timers de auto-guardado (FTSTicker, independiente de World)
// ============================================================================

void UPGXSaveSubsystem::StartAutoSaveTimer(FGameplayTag ContextTag)
{
	// EN: Don't double-register
	// ES: No registrar doble
	if (AutoSaveTickerHandles.Contains(ContextTag))
	{
		return;
	}

	const UPGXSaveConfig* Config = FindConfigByContextTag(ContextTag);
	if (!Config || !Config->bAutoSaveEnabled)
	{
		return;
	}

	const float Interval = Config->AutoSaveIntervalSeconds;

	FTSTicker::FDelegateHandle Handle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateWeakLambda(this, [this, ContextTag](float /*DeltaTime*/) -> bool
		{
			TriggerAutoSave(ContextTag);
			return true; // EN: Keep ticking / ES: Seguir ejecutando
		}),
		Interval
	);

	AutoSaveTickerHandles.Add(ContextTag, Handle);

	PGX_LOG_INFO(LogPGXSave, TEXT("[AutoSave] Timer started for '%s' (interval: %.0fs)"),
		*ContextTag.ToString(), Interval);
}

void UPGXSaveSubsystem::StopAutoSaveTimer(FGameplayTag ContextTag)
{
	FTSTicker::FDelegateHandle* Handle = AutoSaveTickerHandles.Find(ContextTag);
	if (Handle)
	{
		FTSTicker::GetCoreTicker().RemoveTicker(*Handle);
		AutoSaveTickerHandles.Remove(ContextTag);

		PGX_LOG_INFO(LogPGXSave, TEXT("[AutoSave] Timer stopped for '%s'"), *ContextTag.ToString());
	}
}

void UPGXSaveSubsystem::StartAllAutoSaveTimers()
{
	for (const UPGXSaveConfig* Config : DiscoveredConfigs)
	{
		if (Config && Config->bAutoSaveEnabled)
		{
			StartAutoSaveTimer(Config->ContextTag);
		}
	}
}

void UPGXSaveSubsystem::StopAllAutoSaveTimers()
{
	TArray<FGameplayTag> Keys;
	AutoSaveTickerHandles.GetKeys(Keys);

	for (const FGameplayTag& Key : Keys)
	{
		StopAutoSaveTimer(Key);
	}
}

// ============================================================================
// EN: Async context operations
// ES: Operaciones asincronas de contexto
// ============================================================================

void UPGXSaveSubsystem::SaveContextAsync(FGameplayTag ContextTag, const FString& SlotName)
{
	if (bSaveInProgress)
	{
		PGX_LOG_WARNING(LogPGXSave, TEXT("[SaveContextAsync] Save already in progress"));
		OnSaveCompleted.Broadcast(SlotName, EPGXSaveResult::InProgress);
		OnSaveCompletedNative.Broadcast(SlotName, EPGXSaveResult::InProgress);
		return;
	}

	// --- GameThread: Context lookup & serialization ---
	UPGXSaveConfig* Config = FindConfigByContextTag(ContextTag);
	if (!Config)
	{
		PGX_LOG_ERROR(LogPGXSave, TEXT("[SaveContextAsync] Context not found: %s"), *ContextTag.ToString());
		OnSaveCompleted.Broadcast(SlotName, EPGXSaveResult::ContextNotFound);
		OnSaveCompletedNative.Broadcast(SlotName, EPGXSaveResult::ContextNotFound);
		return;
	}

	if (!ActiveProvider)
	{
		OnSaveCompleted.Broadcast(SlotName, EPGXSaveResult::ProviderError);
		OnSaveCompletedNative.Broadcast(SlotName, EPGXSaveResult::ProviderError);
		return;
	}

	bSaveInProgress = true;

	const FString EffectiveSlot = (Config->SaveMode == EPGXSaveMode::SingleSlot) ? TEXT("Save") : SlotName;
	const FString BasePath = ActiveProvider->GetBaseSaveDirectory();

	// EN: OnPreSave callbacks (must be GameThread)
	// ES: Callbacks OnPreSave (deben ser GameThread)
	for (const FPGXSaveDomainEntry& DomainEntry : Config->SaveDomains)
	{
		FPGXDomainBinding* Binding = FindBindingByDomainTag(DomainEntry.DomainTag);
		if (!Binding) continue;

		UPGXSaveGame* SaveGame = EnsureSaveGameInstance(*Binding);
		if (SaveGame)
		{
			NotifySaveablesPreSave(DomainEntry.DomainTag, SaveGame);
		}
	}

	// EN: Serialize all domains to bytes on GameThread (UObject access required)
	// ES: Serializar todos los dominios a bytes en GameThread (acceso a UObject requerido)
	struct FDomainBytes
	{
		FString FilePath;
		TArray<uint8> Bytes;
		FString Checksum;
		bool bRequired = true;
	};

	TSharedPtr<TArray<FDomainBytes>> AllDomainBytes = MakeShared<TArray<FDomainBytes>>();
	bool bSerializationFailed = false;

	for (const FPGXSaveDomainEntry& DomainEntry : Config->SaveDomains)
	{
		FPGXDomainBinding* Binding = FindBindingByDomainTag(DomainEntry.DomainTag);
		if (!Binding || !Binding->ActiveInstance)
		{
			if (DomainEntry.bRequired) bSerializationFailed = true;
			continue;
		}

		UPGXSaveGame* SaveGame = Binding->ActiveInstance;
		SaveGame->SaveTimestamp = FDateTime::Now();
		SaveGame->SaveFormatVersion = Config->CurrentSaveVersion;

		// EN: Serialize
		// ES: Serializar
		TArray<uint8> Bytes;
		if (!UPGXSaveSerializer::SerializeToMemory(SaveGame, Bytes))
		{
			if (DomainEntry.bRequired) bSerializationFailed = true;
			continue;
		}

		// EN: Compress (if enabled)
		// ES: Comprimir (si habilitado)
		TArray<uint8> FinalBytes;
		if (Config->bCompressSaveData)
		{
			if (!UPGXSaveSerializer::Compress(Bytes, FinalBytes))
			{
				FinalBytes = MoveTemp(Bytes);
			}
		}
		else
		{
			FinalBytes = MoveTemp(Bytes);
		}

		// EN: Checksum
		// ES: Checksum
		const FString Checksum = UPGXSaveSerializer::GenerateChecksum(FinalBytes);

		FDomainBytes Entry;
		Entry.FilePath = UPGXSaveSerializer::ResolveSavePath(Config, BasePath, EffectiveSlot, DomainEntry.DomainTag);
		Entry.Bytes = MoveTemp(FinalBytes);
		Entry.Checksum = Checksum;
		Entry.bRequired = DomainEntry.bRequired;
		AllDomainBytes->Add(MoveTemp(Entry));
	}

	if (bSerializationFailed)
	{
		bSaveInProgress = false;
		OnSaveCompleted.Broadcast(EffectiveSlot, EPGXSaveResult::Failed);
		OnSaveCompletedNative.Broadcast(EffectiveSlot, EPGXSaveResult::Failed);
		return;
	}

	// EN: Dispatch I/O to background thread
	// ES: Despachar I/O a hilo de fondo
	TWeakObjectPtr<UPGXSaveSubsystem> WeakThis(this);
	TWeakObjectPtr<UPGXSaveProvider> WeakProvider(ActiveProvider);
	const FString CapturedSlot = EffectiveSlot; // NOLINT(performance-unnecessary-copy-initialization) — intentional copy for async lambda capture
	const FGameplayTag CapturedContextTag = ContextTag;
	const bool bCreateBackup = Config->bCreateBackupBeforeSave;
	const int32 MaxBackups = Config->MaxBackupsPerSlot;

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [WeakThis, WeakProvider, AllDomainBytes, CapturedSlot, CapturedContextTag, bCreateBackup, MaxBackups]()
	{
		bool bAnyFailed = false;

		for (FDomainBytes& DomainData : *AllDomainBytes)
		{
			UPGXSaveProvider* Provider = WeakProvider.Get();
			if (!Provider)
			{
				bAnyFailed = true;
				break;
			}

			// EN: Backup before write (file I/O, safe on background thread)
			// ES: Backup antes de escribir (I/O de archivo, seguro en hilo de fondo)
			if (bCreateBackup)
			{
				FPGXSaveBackupManager::CreateBackup(DomainData.FilePath, MaxBackups);
			}

			// EN: Ensure directory exists
			// ES: Asegurar que el directorio existe
			const FString Directory = FPaths::GetPath(DomainData.FilePath);
			Provider->EnsureDirectoryExists(Directory);

			// EN: Write bytes
			// ES: Escribir bytes
			if (!Provider->SaveBytes(DomainData.FilePath, DomainData.Bytes))
			{
				if (DomainData.bRequired) bAnyFailed = true;
			}
			else
			{
				// EN: Write checksum sidecar file / ES: Escribir archivo sidecar de checksum
				const FString ChecksumPath = DomainData.FilePath + TEXT(".checksum");
				FFileHelper::SaveStringToFile(DomainData.Checksum, *ChecksumPath);
			}
		}

		const EPGXSaveResult Result = bAnyFailed ? EPGXSaveResult::Failed : EPGXSaveResult::Success;

		// EN: Return to GameThread for delegate broadcast
		// ES: Retornar a GameThread para broadcast de delegates
		AsyncTask(ENamedThreads::GameThread, [WeakThis, CapturedSlot, CapturedContextTag, Result]()
		{
			if (UPGXSaveSubsystem* Self = WeakThis.Get())
			{
				Self->bSaveInProgress = false;

				if (Result == EPGXSaveResult::Success)
				{
					Self->SetActiveSlot(CapturedContextTag, CapturedSlot);
				}

				PGX_LOG_INFO(LogPGXSave, TEXT("[SaveContextAsync] Complete — %s (result: %s)"),
					*CapturedSlot, *UEnum::GetValueAsString(Result));

				Self->OnSaveCompleted.Broadcast(CapturedSlot, Result);
				Self->OnSaveCompletedNative.Broadcast(CapturedSlot, Result);
			}
		});
	});
}

void UPGXSaveSubsystem::LoadContextAsync(FGameplayTag ContextTag, const FString& SlotName)
{
	if (bLoadInProgress)
	{
		PGX_LOG_WARNING(LogPGXSave, TEXT("[LoadContextAsync] Load already in progress"));
		OnLoadCompleted.Broadcast(SlotName, EPGXSaveResult::InProgress, nullptr);
		OnLoadCompletedNative.Broadcast(SlotName, EPGXSaveResult::InProgress, nullptr);
		return;
	}

	// --- GameThread: Context lookup ---
	UPGXSaveConfig* Config = FindConfigByContextTag(ContextTag);
	if (!Config)
	{
		OnLoadCompleted.Broadcast(SlotName, EPGXSaveResult::ContextNotFound, nullptr);
		OnLoadCompletedNative.Broadcast(SlotName, EPGXSaveResult::ContextNotFound, nullptr);
		return;
	}

	if (!ActiveProvider)
	{
		OnLoadCompleted.Broadcast(SlotName, EPGXSaveResult::ProviderError, nullptr);
		OnLoadCompletedNative.Broadcast(SlotName, EPGXSaveResult::ProviderError, nullptr);
		return;
	}

	bLoadInProgress = true;

	const FString EffectiveSlot = (Config->SaveMode == EPGXSaveMode::SingleSlot) ? TEXT("Save") : SlotName;
	const FString BasePath = ActiveProvider->GetBaseSaveDirectory();

	// EN: Build file path list for background read
	// ES: Construir lista de paths de archivo para lectura en fondo
	struct FDomainReadRequest
	{
		FGameplayTag DomainTag;
		FString FilePath;
		bool bRequired = true;
		bool bCompressed = false;
	};

	TSharedPtr<TArray<FDomainReadRequest>> ReadRequests = MakeShared<TArray<FDomainReadRequest>>();

	for (const FPGXSaveDomainEntry& DomainEntry : Config->SaveDomains)
	{
		FDomainReadRequest Req;
		Req.DomainTag = DomainEntry.DomainTag;
		Req.FilePath = UPGXSaveSerializer::ResolveSavePath(Config, BasePath, EffectiveSlot, DomainEntry.DomainTag);
		Req.bRequired = DomainEntry.bRequired;
		Req.bCompressed = Config->bCompressSaveData;
		ReadRequests->Add(MoveTemp(Req));
	}

	// EN: Dispatch file reads to background thread
	// ES: Despachar lecturas de archivo a hilo de fondo
	struct FDomainReadResult
	{
		FGameplayTag DomainTag;
		TArray<uint8> DeserializeBytes;
		bool bSuccess = false;
		bool bRequired = true;
	};

	TWeakObjectPtr<UPGXSaveSubsystem> WeakThis(this);
	TWeakObjectPtr<UPGXSaveProvider> WeakProvider(ActiveProvider);
	const FString CapturedSlot = EffectiveSlot; // NOLINT(performance-unnecessary-copy-initialization) — intentional copy for async lambda capture
	const FGameplayTag CapturedContextTag = ContextTag;
	const bool bValidateChecksum = Config->bValidateChecksum;
	const int32 CurrentSaveVersion = Config->CurrentSaveVersion;

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [WeakThis, WeakProvider, ReadRequests, CapturedSlot, CapturedContextTag, bValidateChecksum, CurrentSaveVersion]()
	{
		TSharedPtr<TArray<FDomainReadResult>> ReadResults = MakeShared<TArray<FDomainReadResult>>();
		bool bAnyRequiredFailed = false;
		// EN: Track corruption distinct from generic failure for result-code
		//     contract (mirrors sync LoadContext logic).
		// ES: Trackear corrupcion distinta del fallo generico para el contrato de codigo
		//     de resultado (replica logica del LoadContext sincrono).
		bool bAnyRequiredCorrupted = false;

		for (const FDomainReadRequest& Req : *ReadRequests)
		{
			UPGXSaveProvider* Provider = WeakProvider.Get();
			if (!Provider)
			{
				bAnyRequiredFailed = true;
				break;
			}

			FDomainReadResult ReadResult;
			ReadResult.DomainTag = Req.DomainTag;
			ReadResult.bRequired = Req.bRequired;

			// EN: Read raw bytes
			// ES: Leer bytes crudos
			TArray<uint8> RawBytes;
			if (!Provider->DoesFileExist(Req.FilePath) || !Provider->LoadBytes(Req.FilePath, RawBytes))
			{
				if (Req.bRequired)
				{
					bAnyRequiredFailed = true;
				}
				ReadResults->Add(MoveTemp(ReadResult));
				continue;
			}

			// EN: Validate checksum sidecar (pure file I/O, safe on background thread)
			// ES: Validar sidecar de checksum (I/O puro de archivo, seguro en hilo de fondo)
			if (bValidateChecksum)
			{
				const FString ChecksumPath = Req.FilePath + TEXT(".checksum");
				FString ExpectedChecksum;
				if (FFileHelper::LoadFileToString(ExpectedChecksum, *ChecksumPath))
				{
					if (!UPGXSaveSerializer::ValidateChecksum(RawBytes, ExpectedChecksum))
					{
						PGX_LOG_ERROR(LogPGXSave, TEXT("[LoadContextAsync] Checksum mismatch for '%s' — data corrupted"),
							*Req.DomainTag.ToString());
						if (Req.bRequired)
						{
							bAnyRequiredFailed = true;
							bAnyRequiredCorrupted = true;
						}
						ReadResults->Add(MoveTemp(ReadResult));
						continue;
					}
				}
			}

			// EN: Decompress (if needed) — pure byte ops, safe on background thread
			// ES: Descomprimir (si necesario) — ops puras de bytes, seguro en hilo de fondo
			if (Req.bCompressed)
			{
				TArray<uint8> Decompressed;
				if (UPGXSaveSerializer::Decompress(RawBytes, Decompressed))
				{
					ReadResult.DeserializeBytes = MoveTemp(Decompressed);
					ReadResult.bSuccess = true;
				}
				else
				{
					if (Req.bRequired) bAnyRequiredFailed = true;
				}
			}
			else
			{
				ReadResult.DeserializeBytes = MoveTemp(RawBytes);
				ReadResult.bSuccess = true;
			}

			ReadResults->Add(MoveTemp(ReadResult));
		}

		// EN: Return to GameThread for deserialization (UObject creation)
		// ES: Retornar a GameThread para deserializacion (creacion de UObject)
		AsyncTask(ENamedThreads::GameThread, [WeakThis, ReadResults, CapturedSlot, CapturedContextTag, bAnyRequiredFailed, bAnyRequiredCorrupted, CurrentSaveVersion]()
		{
			UPGXSaveSubsystem* Self = WeakThis.Get();
			if (!Self)
			{
				return;
			}

			int32 DomainsLoaded = 0;
			bool bDeserializeFailed = bAnyRequiredFailed;
			// EN: Corruption flag flows from background read; deserialize/migrate failures
			//     do NOT escalate to corrupted (they are generic failures).
			// ES: La flag de corrupcion fluye desde la lectura en background; los fallos de
			//     deserializacion/migracion NO escalan a corrupted (son fallos genericos).
			const bool bDeserializeCorrupted = bAnyRequiredCorrupted;

			for (const FDomainReadResult& ReadResult : *ReadResults)
			{
				if (!ReadResult.bSuccess)
				{
					continue;
				}

				// EN: Deserialize on GameThread (UObject access)
				// ES: Deserializar en GameThread (acceso a UObject)
				UPGXSaveGame* LoadedGame = UPGXSaveSerializer::DeserializeFromMemory(ReadResult.DeserializeBytes);
				if (!LoadedGame)
				{
					if (ReadResult.bRequired) bDeserializeFailed = true;
					continue;
				}

				// EN: Version migration (if needed) — mirrors sync load path
				// ES: Migracion de version (si necesario) — replica el path de carga sincrona
				if (LoadedGame->SaveFormatVersion != CurrentSaveVersion)
				{
					const EPGXSaveResult MigrateResult = FPGXSaveVersioning::MigrateToVersion(
						LoadedGame, CurrentSaveVersion);

					if (MigrateResult != EPGXSaveResult::Success)
					{
						PGX_LOG_ERROR(LogPGXSave,
							TEXT("[LoadContextAsync] Version migration failed for '%s': v%d -> v%d"),
							*ReadResult.DomainTag.ToString(),
							LoadedGame->SaveFormatVersion, CurrentSaveVersion);
						if (ReadResult.bRequired) bDeserializeFailed = true;
						continue;
					}
				}

				// EN: Store in binding
				// ES: Almacenar en binding
				FPGXDomainBinding* Binding = Self->FindBindingByDomainTag(ReadResult.DomainTag);
				if (Binding)
				{
					Binding->ActiveInstance = LoadedGame;
				}

				// EN: PostLoad callbacks
				// ES: Callbacks PostLoad
				Self->NotifySaveablesPostLoad(ReadResult.DomainTag, LoadedGame);

				DomainsLoaded++;
			}

			// EN: Corruption-distinct result mapping mirrors sync LoadContext.
			//     Integrity-mismatch payloads return Corrupted.
			// ES: Mapping de resultado distinto-corrupcion replica LoadContext sincrono.
			//     Los payloads con mismatch de integridad retornan Corrupted.
			const EPGXSaveResult Result =
				bDeserializeCorrupted ? EPGXSaveResult::Corrupted :
				bDeserializeFailed    ? EPGXSaveResult::Failed    :
				                        EPGXSaveResult::Success;

			if (Result == EPGXSaveResult::Success)
			{
				Self->SetActiveSlot(CapturedContextTag, CapturedSlot);
			}

			Self->bLoadInProgress = false;

			// EN: Find first loaded SaveGame instance for broadcast
			// ES: Encontrar primera instancia de SaveGame cargada para broadcast
			UPGXSaveGame* BroadcastInstance = nullptr;
			if (Result == EPGXSaveResult::Success)
			{
				for (const FDomainReadResult& RR : *ReadResults)
				{
					if (RR.bSuccess)
					{
						FPGXDomainBinding* Binding = Self->FindBindingByDomainTag(RR.DomainTag);
						if (Binding && IsValid(Binding->ActiveInstance))
						{
							BroadcastInstance = Binding->ActiveInstance;
							break;
						}
					}
				}
			}

			PGX_LOG_INFO(LogPGXSave, TEXT("[LoadContextAsync] Complete — %s (%d domains, result: %s)"),
				*CapturedSlot, DomainsLoaded, *UEnum::GetValueAsString(Result));

			Self->OnLoadCompleted.Broadcast(CapturedSlot, Result, BroadcastInstance);
			Self->OnLoadCompletedNative.Broadcast(CapturedSlot, Result, BroadcastInstance);
		});
	});
}

// ============================================================================
// EN: Context queries
// ES: Consultas de contexto
// ============================================================================

TArray<FGameplayTag> UPGXSaveSubsystem::GetAllContextTags() const
{
	TArray<FGameplayTag> Tags;
	ContextConfigMap.GetKeys(Tags);
	return Tags;
}

const UPGXSaveConfig* UPGXSaveSubsystem::GetContextConfig(FGameplayTag ContextTag) const
{
	return FindConfigByContextTag(ContextTag);
}

// ============================================================================
// EN: Debug snapshot
// ES: Snapshot debug
// ============================================================================

FPGXSaveDebugSnapshot UPGXSaveSubsystem::GetDebugSnapshot() const
{
	FPGXSaveDebugSnapshot Snapshot;

	Snapshot.ContextCount = DiscoveredConfigs.Num();
	Snapshot.DomainBindingCount = DomainBindings.Num();
	Snapshot.ActiveSlotsByContext = ActiveSlots;
	Snapshot.bSaveInProgress = bSaveInProgress;
	Snapshot.bLoadInProgress = bLoadInProgress;
	Snapshot.RegisteredSaveableCount = RegisteredSaveables.Num();
	Snapshot.ActiveProviderClassName = ActiveProvider ? ActiveProvider->GetClass()->GetName() : FString();
	Snapshot.RecentOperations = OperationHistory;
	Snapshot.SnapshotTime = FPlatformTime::Seconds();

	return Snapshot;
}

// ============================================================================
// EN: Console commands (pgx.save.*)
// ES: Comandos de consola (pgx.save.*)
// ============================================================================

void UPGXSaveSubsystem::RegisterConsoleCommands()
{
	IConsoleManager& Console = IConsoleManager::Get();

	// --- pgx.save.stats ---
	RegisteredCommands.Add(Console.RegisterConsoleCommand(
		TEXT("pgx.save.stats"),
		TEXT("Show PGX Save system statistics"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			PGX_LOG_INFO(LogPGXSave, TEXT("=== PGX Save Stats ==="));
			PGX_LOG_INFO(LogPGXSave, TEXT("Contexts:     %d"), DiscoveredConfigs.Num());
			PGX_LOG_INFO(LogPGXSave, TEXT("Domains:      %d"), DomainBindings.Num());
			PGX_LOG_INFO(LogPGXSave, TEXT("Provider:     %s"), ActiveProvider ? *ActiveProvider->GetClass()->GetName() : TEXT("NONE"));
			PGX_LOG_INFO(LogPGXSave, TEXT("Saveables:    %d"), RegisteredSaveables.Num());
			PGX_LOG_INFO(LogPGXSave, TEXT("Active Slots: %d"), ActiveSlots.Num());
			PGX_LOG_INFO(LogPGXSave, TEXT("AutoTimers:   %d"), AutoSaveTickerHandles.Num());
			PGX_LOG_INFO(LogPGXSave, TEXT("Save InProg:  %s"), bSaveInProgress ? TEXT("YES") : TEXT("no"));
			PGX_LOG_INFO(LogPGXSave, TEXT("Load InProg:  %s"), bLoadInProgress ? TEXT("YES") : TEXT("no"));

			if (ActiveProvider)
			{
				const int64 DiskSpace = ActiveProvider->GetDiskSpaceAvailable();
				PGX_LOG_INFO(LogPGXSave, TEXT("Disk Free:    %lld MB"), DiskSpace / (1024 * 1024));
			}
		}),
		ECVF_Default
	));

	// --- pgx.save.list ---
	RegisteredCommands.Add(Console.RegisterConsoleCommand(
		TEXT("pgx.save.list"),
		TEXT("List all registered save contexts and their domains"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			PGX_LOG_INFO(LogPGXSave, TEXT("=== PGX Save Contexts ==="));
			for (const UPGXSaveConfig* Config : DiscoveredConfigs)
			{
				if (!Config) continue;

				const FString* ActiveSlot = ActiveSlots.Find(Config->ContextTag);
				const bool bTimerActive = AutoSaveTickerHandles.Contains(Config->ContextTag);

				PGX_LOG_INFO(LogPGXSave, TEXT("  [%s] '%s' — mode:%s, domains:%d, active:'%s', autosave:%s"),
					*Config->ContextTag.ToString(),
					*Config->GetName(),
					*UEnum::GetValueAsString(Config->SaveMode),
					Config->SaveDomains.Num(),
					ActiveSlot ? **ActiveSlot : TEXT("(none)"),
					bTimerActive ? TEXT("ON") : TEXT("off"));

				for (const FPGXSaveDomainEntry& Domain : Config->SaveDomains)
				{
					const FPGXDomainBinding* Binding = FindBindingByDomainTag(Domain.DomainTag);
					PGX_LOG_INFO(LogPGXSave, TEXT("    - %s -> %s [%s] %s"),
						*Domain.DomainTag.ToString(),
						Domain.SaveGameClass ? *Domain.SaveGameClass->GetName() : TEXT("NULL"),
						(Binding && Binding->ActiveInstance) ? TEXT("loaded") : TEXT("empty"),
						Domain.bRequired ? TEXT("(required)") : TEXT("(optional)"));
				}
			}
		}),
		ECVF_Default
	));

	// --- pgx.save.slots <ContextTag> ---
	RegisteredCommands.Add(Console.RegisterConsoleCommand(
		TEXT("pgx.save.slots"),
		TEXT("List all save slots for a context. Usage: pgx.save.slots <ContextTag>"),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& Args)
		{
			if (Args.Num() == 0)
			{
				PGX_LOG_INFO(LogPGXSave, TEXT("Usage: pgx.save.slots <ContextTag>"));
				return;
			}

			const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*Args[0]), false);
			if (!Tag.IsValid())
			{
				PGX_LOG_WARNING(LogPGXSave, TEXT("Invalid tag: %s"), *Args[0]);
				return;
			}

			const TArray<FPGXSaveSlotInfo> Slots = GetAllSlots(Tag);
			PGX_LOG_INFO(LogPGXSave, TEXT("=== Slots for '%s' (%d) ==="), *Args[0], Slots.Num());
			for (const FPGXSaveSlotInfo& Slot : Slots)
			{
				PGX_LOG_INFO(LogPGXSave, TEXT("  - %s"), *Slot.SlotName);
			}
		}),
		ECVF_Default
	));

	// --- pgx.save.save <ContextTag> <SlotName> ---
	RegisteredCommands.Add(Console.RegisterConsoleCommand(
		TEXT("pgx.save.save"),
		TEXT("Save a context to a slot. Usage: pgx.save.save <ContextTag> <SlotName>"),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& Args)
		{
			if (Args.Num() < 2)
			{
				PGX_LOG_INFO(LogPGXSave, TEXT("Usage: pgx.save.save <ContextTag> <SlotName>"));
				return;
			}

			const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*Args[0]), false);
			if (!Tag.IsValid())
			{
				PGX_LOG_WARNING(LogPGXSave, TEXT("Invalid tag: %s"), *Args[0]);
				return;
			}

			const EPGXSaveResult Result = SaveContext(Tag, Args[1]);
			PGX_LOG_INFO(LogPGXSave, TEXT("[Console] SaveContext -> %s"), *UEnum::GetValueAsString(Result));
		}),
		ECVF_Default
	));

	// --- pgx.save.load <ContextTag> <SlotName> ---
	RegisteredCommands.Add(Console.RegisterConsoleCommand(
		TEXT("pgx.save.load"),
		TEXT("Load a context from a slot. Usage: pgx.save.load <ContextTag> <SlotName>"),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& Args)
		{
			if (Args.Num() < 2)
			{
				PGX_LOG_INFO(LogPGXSave, TEXT("Usage: pgx.save.load <ContextTag> <SlotName>"));
				return;
			}

			const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*Args[0]), false);
			if (!Tag.IsValid())
			{
				PGX_LOG_WARNING(LogPGXSave, TEXT("Invalid tag: %s"), *Args[0]);
				return;
			}

			const EPGXSaveResult Result = LoadContext(Tag, Args[1]);
			PGX_LOG_INFO(LogPGXSave, TEXT("[Console] LoadContext -> %s"), *UEnum::GetValueAsString(Result));
		}),
		ECVF_Default
	));

	// --- pgx.save.delete <ContextTag> <SlotName> ---
	RegisteredCommands.Add(Console.RegisterConsoleCommand(
		TEXT("pgx.save.delete"),
		TEXT("Delete a save slot. Usage: pgx.save.delete <ContextTag> <SlotName>"),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& Args)
		{
			if (Args.Num() < 2)
			{
				PGX_LOG_INFO(LogPGXSave, TEXT("Usage: pgx.save.delete <ContextTag> <SlotName>"));
				return;
			}

			const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*Args[0]), false);
			if (!Tag.IsValid())
			{
				PGX_LOG_WARNING(LogPGXSave, TEXT("Invalid tag: %s"), *Args[0]);
				return;
			}

			const EPGXSaveResult Result = DeleteSlot(Tag, Args[1]);
			PGX_LOG_INFO(LogPGXSave, TEXT("[Console] DeleteSlot -> %s"), *UEnum::GetValueAsString(Result));
		}),
		ECVF_Default
	));

	// --- pgx.save.quicksave <ContextTag> ---
	RegisteredCommands.Add(Console.RegisterConsoleCommand(
		TEXT("pgx.save.quicksave"),
		TEXT("Quick save a context. Usage: pgx.save.quicksave <ContextTag>"),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& Args)
		{
			if (Args.Num() == 0)
			{
				PGX_LOG_INFO(LogPGXSave, TEXT("Usage: pgx.save.quicksave <ContextTag>"));
				return;
			}

			const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*Args[0]), false);
			if (!Tag.IsValid())
			{
				PGX_LOG_WARNING(LogPGXSave, TEXT("Invalid tag: %s"), *Args[0]);
				return;
			}

			const EPGXSaveResult Result = QuickSave(Tag);
			PGX_LOG_INFO(LogPGXSave, TEXT("[Console] QuickSave -> %s"), *UEnum::GetValueAsString(Result));
		}),
		ECVF_Default
	));

	// --- pgx.save.quickload <ContextTag> ---
	RegisteredCommands.Add(Console.RegisterConsoleCommand(
		TEXT("pgx.save.quickload"),
		TEXT("Quick load a context. Usage: pgx.save.quickload <ContextTag>"),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& Args)
		{
			if (Args.Num() == 0)
			{
				PGX_LOG_INFO(LogPGXSave, TEXT("Usage: pgx.save.quickload <ContextTag>"));
				return;
			}

			const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*Args[0]), false);
			if (!Tag.IsValid())
			{
				PGX_LOG_WARNING(LogPGXSave, TEXT("Invalid tag: %s"), *Args[0]);
				return;
			}

			const EPGXSaveResult Result = QuickLoad(Tag);
			PGX_LOG_INFO(LogPGXSave, TEXT("[Console] QuickLoad -> %s"), *UEnum::GetValueAsString(Result));
		}),
		ECVF_Default
	));

	// --- pgx.save.autosave <ContextTag> ---
	RegisteredCommands.Add(Console.RegisterConsoleCommand(
		TEXT("pgx.save.autosave"),
		TEXT("Trigger auto-save for a context. Usage: pgx.save.autosave <ContextTag>"),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& Args)
		{
			if (Args.Num() == 0)
			{
				PGX_LOG_INFO(LogPGXSave, TEXT("Usage: pgx.save.autosave <ContextTag>"));
				return;
			}

			const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*Args[0]), false);
			if (!Tag.IsValid())
			{
				PGX_LOG_WARNING(LogPGXSave, TEXT("Invalid tag: %s"), *Args[0]);
				return;
			}

			TriggerAutoSave(Tag);
		}),
		ECVF_Default
	));

	// --- pgx.save.info <ContextTag> <SlotName> ---
	RegisteredCommands.Add(Console.RegisterConsoleCommand(
		TEXT("pgx.save.info"),
		TEXT("Show detailed info for a slot. Usage: pgx.save.info <ContextTag> <SlotName>"),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& Args)
		{
			if (Args.Num() < 2)
			{
				PGX_LOG_INFO(LogPGXSave, TEXT("Usage: pgx.save.info <ContextTag> <SlotName>"));
				return;
			}

			const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*Args[0]), false);
			if (!Tag.IsValid())
			{
				PGX_LOG_WARNING(LogPGXSave, TEXT("Invalid tag: %s"), *Args[0]);
				return;
			}

			const UPGXSaveConfig* Config = FindConfigByContextTag(Tag);
			if (!Config)
			{
				PGX_LOG_WARNING(LogPGXSave, TEXT("Context not found: %s"), *Args[0]);
				return;
			}

			const bool bExists = DoesSlotExist(Tag, Args[1]);
			PGX_LOG_INFO(LogPGXSave, TEXT("=== Slot Info ==="));
			PGX_LOG_INFO(LogPGXSave, TEXT("Context:  %s"), *Args[0]);
			PGX_LOG_INFO(LogPGXSave, TEXT("Slot:     %s"), *Args[1]);
			PGX_LOG_INFO(LogPGXSave, TEXT("Exists:   %s"), bExists ? TEXT("YES") : TEXT("no"));
			PGX_LOG_INFO(LogPGXSave, TEXT("Mode:     %s"), *UEnum::GetValueAsString(Config->SaveMode));

			if (bExists && ActiveProvider)
			{
				const FString BasePath = ActiveProvider->GetBaseSaveDirectory();
				int64 TotalSize = 0;
				int32 FileCount = 0;

				for (const FPGXSaveDomainEntry& Domain : Config->SaveDomains)
				{
					const FString FilePath = UPGXSaveSerializer::ResolveSavePath(Config, BasePath, Args[1], Domain.DomainTag);
					if (ActiveProvider->DoesFileExist(FilePath))
					{
						TArray<uint8> Bytes;
						if (ActiveProvider->LoadBytes(FilePath, Bytes))
						{
							TotalSize += Bytes.Num();
							FileCount++;

							PGX_LOG_INFO(LogPGXSave, TEXT("  - %s: %d bytes"),
								*Domain.DomainTag.ToString(), Bytes.Num());
						}
					}
				}

				PGX_LOG_INFO(LogPGXSave, TEXT("Files:    %d"), FileCount);
				PGX_LOG_INFO(LogPGXSave, TEXT("Total:    %lld bytes"), TotalSize);
			}
		}),
		ECVF_Default
	));

	PGX_LOG_INFO(LogPGXSave, TEXT("[Console] Registered %d pgx.save.* commands"), RegisteredCommands.Num());
}

void UPGXSaveSubsystem::UnregisterConsoleCommands()
{
	IConsoleManager& Console = IConsoleManager::Get();
	for (IConsoleObject* Cmd : RegisteredCommands)
	{
		Console.UnregisterConsoleObject(Cmd);
	}
	RegisteredCommands.Empty();
}

// ============================================================================
// Profile Integration
// ============================================================================

void UPGXSaveSubsystem::ApplyProfileConstraints(const FPGXResolvedProfile& Profile)
{
	// EN: Enforce Save platform budgets from active PlatformConfig
	// ES: Aplicar presupuestos de plataforma Save desde PlatformConfig activa
	int32 EnforcedMaxFileSize_KB = 0;
	int32 EnforcedMaxStorage_MB = 0;
	int32 EnforcedMaxConcurrentIO = 0;

	if (auto* ProfileSS = UPGXProfileSubsystem::GetCachedInstance())
	{
		if (const UPGXPlatformConfig* PlatformCfg = ProfileSS->GetActivePlatformConfig())
		{
			const auto& B = PlatformCfg->SaveBudgets;
			EnforcedMaxFileSize_KB = B.MaxSaveFileSize_KB;
			EnforcedMaxStorage_MB = B.MaxTotalStorage_MB;
			EnforcedMaxConcurrentIO = B.MaxConcurrentIO;
		}
	}

	PGX_LOG_INFO(LogPGXSave, TEXT("[SaveSubsystem] Profile constraints enforced — MaxFile=%dKB, MaxStorage=%dMB, MaxIO=%d, SaveData=%d, Encrypt=%d, Compress=%d"),
		EnforcedMaxFileSize_KB, EnforcedMaxStorage_MB, EnforcedMaxConcurrentIO,
		Profile.Capabilities.bAllowSaveData,
		Profile.Policies.Security.bRequireEncryption,
		Profile.Policies.Security.bRequireCompression);
}

void UPGXSaveSubsystem::HandleProfileChanged(const FPGXResolvedProfile& /*OldProfile*/, const FPGXResolvedProfile& NewProfile)
{
	ApplyProfileConstraints(NewProfile);
}

// ============================================================================
// EN: Test injection API (editor only)
// ES: API de inyeccion de test (solo editor)
// ============================================================================

#if WITH_EDITOR
void UPGXSaveSubsystem::InjectTestConfig(UPGXSaveConfig* Config)
{
	if (!IsValid(Config))
	{
		PGX_LOG_WARNING(LogPGXSave, TEXT("[TestHarness] InjectTestConfig — invalid Config"));
		return;
	}

	if (!Config->ContextTag.IsValid())
	{
		PGX_LOG_WARNING(LogPGXSave, TEXT("[TestHarness] InjectTestConfig — Config '%s' has invalid ContextTag"),
			*Config->GetName());
		return;
	}

	// EN: Add to discovered configs + context map / ES: Agregar a configs descubiertos + mapa de contexto
	DiscoveredConfigs.AddUnique(Config);
	ContextConfigMap.Add(Config->ContextTag, Config);

	// EN: Build domain bindings for this config / ES: Construir domain bindings para este config
	for (const FPGXSaveDomainEntry& DomainEntry : Config->SaveDomains)
	{
		if (!DomainEntry.DomainTag.IsValid() || DomainBindings.Contains(DomainEntry.DomainTag))
		{
			continue;
		}

		FPGXDomainBinding Binding;
		Binding.OwningConfig = Config;
		Binding.DomainEntry = DomainEntry;
		Binding.ActiveInstance = nullptr;

		DomainBindings.Add(DomainEntry.DomainTag, Binding);
	}

	if (Config->SaveProviderClass)
	{
		ActiveProvider = NewObject<UPGXSaveProvider>(this, Config->SaveProviderClass);
	}

	PGX_LOG_INFO(LogPGXSave, TEXT("[TestHarness] Injected test SaveConfig: %s (context=%s, %d domains)"),
		*Config->GetName(), *Config->ContextTag.ToString(), Config->SaveDomains.Num());
}

void UPGXSaveSubsystem::ClearTestConfigs()
{
	const int32 Before = DiscoveredConfigs.Num();

	// EN: Collect transient config context tags for cleanup / ES: Recoger context tags de configs transient para limpieza
	TArray<FGameplayTag> TagsToRemove;

	for (const TObjectPtr<UPGXSaveConfig>& Cfg : DiscoveredConfigs)
	{
		if (IsValid(Cfg) && Cfg->HasAnyFlags(RF_Transient))
		{
			TagsToRemove.Add(Cfg->ContextTag);
		}
	}

	// EN: Remove context map entries for transient configs / ES: Remover entradas del mapa de contexto para configs transient
	for (const FGameplayTag& CtxTag : TagsToRemove)
	{
		ContextConfigMap.Remove(CtxTag);
	}

	// EN: Remove domain bindings only if their owning config is transient (ownership-safe)
	// ES: Remover domain bindings solo si su config dueño es transient (ownership-safe)
	TArray<FGameplayTag> DomainsToRemove;
	for (const auto& Pair : DomainBindings)
	{
		const UPGXSaveConfig* Owner = Pair.Value.OwningConfig.Get();
		if (IsValid(Owner) && Owner->HasAnyFlags(RF_Transient))
		{
			DomainsToRemove.Add(Pair.Key);
		}
	}
	for (const FGameplayTag& DomTag : DomainsToRemove)
	{
		DomainBindings.Remove(DomTag);
	}

	DiscoveredConfigs.RemoveAll([](const TObjectPtr<UPGXSaveConfig>& Cfg)
	{
		return IsValid(Cfg) && Cfg->HasAnyFlags(RF_Transient);
	});

	CreateProvider();

	PGX_LOG_INFO(LogPGXSave, TEXT("[TestHarness] ClearTestConfigs — removed %d transient configs, %d domain bindings"),
		Before - DiscoveredConfigs.Num(), DomainsToRemove.Num());
}
#endif

// EN: IPGXTaggedRegistry facade — exposes DomainBindings via the canonical
//     tag-keyed read contract. Behavior-preserving: same data, unified interface
//     so cross-plugin consumers can treat Save like any tagged registry.
// ES: Fachada IPGXTaggedRegistry — expone DomainBindings via el contrato
//     canonico de lectura keyed-por-tag. Behavior-preserving: mismos datos,
//     interfaz unificada para que consumers cross-plugin traten Save como
//     cualquier registry tagged.
// Keep validation output aligned with the shared PGX result contract.
bool UPGXSaveSubsystem::HasEntryByTag(FGameplayTag Tag) const
{
	return DomainBindings.Contains(Tag);
}

int32 UPGXSaveSubsystem::GetCount() const
{
	return DomainBindings.Num();
}

void UPGXSaveSubsystem::GetSnapshot(TArray<FGameplayTag>& OutTags) const
{
	DomainBindings.GetKeys(OutTags);
}

#undef PGX_SAVE_VERBOSE
