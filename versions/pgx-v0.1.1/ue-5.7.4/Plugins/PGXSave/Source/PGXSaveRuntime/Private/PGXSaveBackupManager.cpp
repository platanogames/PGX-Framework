// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXSaveBackupManager.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "Logging/PGXLogCategories.h"
#include "Logging/PGXLogMacros.h"

// EN: Backup manager and transaction log implementation
// ES: Implementacion del gestor de backups y transaction log

// ============================================================================
// EN: Backup operations
// ES: Operaciones de backup
// ============================================================================

bool FPGXSaveBackupManager::CreateBackup(const FString& FilePath, int32 MaxBackups)
{
	// EN: If the original file doesn't exist, nothing to back up
	// ES: Si el archivo original no existe, nada que respaldar
	if (!IFileManager::Get().FileExists(*FilePath))
	{
		return true;
	}

	if (MaxBackups <= 0)
	{
		return true;
	}

	// EN: Rotate existing backups: .bak.N -> .bak.(N+1), delete if > MaxBackups
	// ES: Rotar backups existentes: .bak.N -> .bak.(N+1), eliminar si > MaxBackups
	for (int32 i = MaxBackups; i >= 1; --i)
	{
		const FString CurrentBackup = GetBackupPath(FilePath, i);

		if (i == MaxBackups)
		{
			// EN: Delete the oldest backup if it exists
			// ES: Eliminar el backup mas antiguo si existe
			if (IFileManager::Get().FileExists(*CurrentBackup))
			{
				IFileManager::Get().Delete(*CurrentBackup, /*bRequireExists=*/ false, /*bEvenReadOnly=*/ true);
			}
		}
		else
		{
			// EN: Shift backup to next index: .bak.i -> .bak.(i+1)
			// ES: Desplazar backup al siguiente indice: .bak.i -> .bak.(i+1)
			if (IFileManager::Get().FileExists(*CurrentBackup))
			{
				const FString NextBackup = GetBackupPath(FilePath, i + 1);
				IFileManager::Get().Move(*NextBackup, *CurrentBackup, /*bReplace=*/ true);
			}
		}
	}

	// EN: Copy original file to .bak.1
	// ES: Copiar archivo original a .bak.1
	const FString NewestBackup = GetBackupPath(FilePath, 1);
	const uint32 CopyResult = IFileManager::Get().Copy(*NewestBackup, *FilePath, /*bReplace=*/ true);

	if (CopyResult != COPY_OK)
	{
		PGX_LOG_WARNING(LogPGXSave, TEXT("[BackupManager] Failed to create backup: %s -> %s"),
			*FilePath, *NewestBackup);
		return false;
	}

	return true;
}

bool FPGXSaveBackupManager::RestoreFromBackup(const FString& FilePath)
{
	// EN: Search for the most recent backup (.bak.1, .bak.2, etc.)
	// ES: Buscar el backup mas reciente (.bak.1, .bak.2, etc.)
	for (int32 i = 1; i <= 10; ++i) // EN: Hard limit 10 to prevent infinite search / ES: Limite duro 10 para prevenir busqueda infinita
	{
		const FString BackupPath = GetBackupPath(FilePath, i);

		if (IFileManager::Get().FileExists(*BackupPath))
		{
			const uint32 CopyResult = IFileManager::Get().Copy(*FilePath, *BackupPath, /*bReplace=*/ true);

			if (CopyResult == COPY_OK)
			{
				PGX_LOG_INFO(LogPGXSave, TEXT("[BackupManager] Restored from backup: %s -> %s"),
					*BackupPath, *FilePath);
				return true;
			}

			PGX_LOG_WARNING(LogPGXSave, TEXT("[BackupManager] Failed to restore from backup: %s"), *BackupPath);
			return false;
		}
	}

	PGX_LOG_WARNING(LogPGXSave, TEXT("[BackupManager] No backup found for: %s"), *FilePath);
	return false;
}

TArray<FString> FPGXSaveBackupManager::GetBackupFiles(const FString& FilePath)
{
	TArray<FString> Backups;

	for (int32 i = 1; i <= 10; ++i)
	{
		const FString BackupPath = GetBackupPath(FilePath, i);
		if (IFileManager::Get().FileExists(*BackupPath))
		{
			Backups.Add(BackupPath);
		}
		else
		{
			break; // EN: Backups are sequential, stop at first gap / ES: Los backups son secuenciales, parar en el primer hueco
		}
	}

	return Backups;
}

void FPGXSaveBackupManager::CleanupAllBackups(const FString& FilePath)
{
	for (int32 i = 1; i <= 10; ++i)
	{
		const FString BackupPath = GetBackupPath(FilePath, i);
		if (IFileManager::Get().FileExists(*BackupPath))
		{
			IFileManager::Get().Delete(*BackupPath, /*bRequireExists=*/ false, /*bEvenReadOnly=*/ true);
		}
		else
		{
			break;
		}
	}
}

// ============================================================================
// EN: Transaction log
// ES: Transaction log
// ============================================================================

void FPGXSaveBackupManager::LogTransaction(const FString& TransactionLogPath, const FPGXBackupEntry& Entry)
{
	// EN: Read existing entries to determine next index
	// ES: Leer entradas existentes para determinar el siguiente indice
	TArray<FPGXBackupEntry> Existing = GetTransactionHistory(TransactionLogPath, 0);

	const int32 NextIndex = Existing.Num() + 1;

	// EN: Format: SlotName|ISO8601|Checksum|Operation|Result|FilePath
	// ES: Formato: SlotName|ISO8601|Checksum|Operation|Result|FilePath
	const FString EntryStr = FString::Printf(TEXT("%s|%s|%s|%s|%s|%s"),
		*Entry.SlotName,
		*Entry.Timestamp.ToIso8601(),
		*Entry.Checksum,
		*OperationToString(Entry.Operation),
		*ResultToString(Entry.Result),
		*Entry.FilePath);

	const FString Key = FString::Printf(TEXT("Entry_%03d"), NextIndex);
	const FString Section = TEXT("PGXSave.TransactionLog");

	// EN: Append to ini file
	// ES: Agregar al archivo ini
	GConfig->SetString(*Section, *Key, *EntryStr, TransactionLogPath);
	GConfig->Flush(false, TransactionLogPath);
}

TArray<FPGXBackupEntry> FPGXSaveBackupManager::GetTransactionHistory(const FString& TransactionLogPath, int32 MaxEntries)
{
	TArray<FPGXBackupEntry> Entries;

	if (!IFileManager::Get().FileExists(*TransactionLogPath))
	{
		return Entries;
	}

	const FString Section = TEXT("PGXSave.TransactionLog");
	TArray<FString> Keys;

	// EN: Read all keys from the section
	// ES: Leer todas las keys de la seccion
	FConfigFile* ConfigFile = GConfig->FindConfigFile(TransactionLogPath);
	if (!ConfigFile)
	{
		GConfig->LoadFile(TransactionLogPath);
		ConfigFile = GConfig->FindConfigFile(TransactionLogPath);
	}

	if (!ConfigFile)
	{
		return Entries;
	}

	const FConfigSection* ConfigSection = ConfigFile->FindSection(*Section);
	if (!ConfigSection)
	{
		return Entries;
	}

	for (const auto& Pair : *ConfigSection)
	{
		const FString EntryStr = Pair.Value.GetValue();

		// EN: Parse: SlotName|ISO8601|Checksum|Operation|Result|FilePath
		// ES: Parsear: SlotName|ISO8601|Checksum|Operation|Result|FilePath
		TArray<FString> Parts;
		EntryStr.ParseIntoArray(Parts, TEXT("|"), /*bCullEmpty=*/ false);

		if (Parts.Num() >= 6)
		{
			FPGXBackupEntry Entry;
			Entry.SlotName = Parts[0];
			FDateTime::ParseIso8601(*Parts[1], Entry.Timestamp);
			Entry.Checksum = Parts[2];
			Entry.Operation = StringToOperation(Parts[3]);
			Entry.Result = StringToResult(Parts[4]);
			Entry.FilePath = Parts[5];
			Entries.Add(Entry);
		}
	}

	// EN: Reverse to get newest first
	// ES: Invertir para obtener el mas nuevo primero
	Algo::Reverse(Entries);

	// EN: Trim to MaxEntries if specified
	// ES: Recortar a MaxEntries si se especifico
	if (MaxEntries > 0 && Entries.Num() > MaxEntries)
	{
		Entries.SetNum(MaxEntries);
	}

	return Entries;
}

FString FPGXSaveBackupManager::ResolveTransactionLogPath(const FString& ContextDirectoryPath)
{
	return FPaths::Combine(ContextDirectoryPath, TEXT("_Transactions.ini"));
}

// ============================================================================
// EN: Internal helpers
// ES: Helpers internos
// ============================================================================

FString FPGXSaveBackupManager::GetBackupPath(const FString& OriginalPath, int32 BackupIndex)
{
	return FString::Printf(TEXT("%s.bak.%d"), *OriginalPath, BackupIndex);
}

FString FPGXSaveBackupManager::OperationToString(EPGXSaveOperation Operation)
{
	switch (Operation)
	{
	case EPGXSaveOperation::Save:		return TEXT("Save");
	case EPGXSaveOperation::Load:		return TEXT("Load");
	case EPGXSaveOperation::Delete:		return TEXT("Delete");
	case EPGXSaveOperation::Copy:		return TEXT("Copy");
	case EPGXSaveOperation::QuickSave:	return TEXT("QuickSave");
	case EPGXSaveOperation::QuickLoad:	return TEXT("QuickLoad");
	case EPGXSaveOperation::AutoSave:	return TEXT("AutoSave");
	default:							return TEXT("Unknown");
	}
}

EPGXSaveOperation FPGXSaveBackupManager::StringToOperation(const FString& Str)
{
	if (Str == TEXT("Save"))		return EPGXSaveOperation::Save;
	if (Str == TEXT("Load"))		return EPGXSaveOperation::Load;
	if (Str == TEXT("Delete"))		return EPGXSaveOperation::Delete;
	if (Str == TEXT("Copy"))		return EPGXSaveOperation::Copy;
	if (Str == TEXT("QuickSave"))	return EPGXSaveOperation::QuickSave;
	if (Str == TEXT("QuickLoad"))	return EPGXSaveOperation::QuickLoad;
	if (Str == TEXT("AutoSave"))	return EPGXSaveOperation::AutoSave;
	return EPGXSaveOperation::Save;
}

FString FPGXSaveBackupManager::ResultToString(EPGXSaveResult Result)
{
	switch (Result)
	{
	case EPGXSaveResult::Success:			return TEXT("Success");
	case EPGXSaveResult::Failed:			return TEXT("Failed");
	case EPGXSaveResult::SlotNotFound:		return TEXT("SlotNotFound");
	case EPGXSaveResult::SlotFull:			return TEXT("SlotFull");
	case EPGXSaveResult::Corrupted:			return TEXT("Corrupted");
	case EPGXSaveResult::VersionMismatch:	return TEXT("VersionMismatch");
	case EPGXSaveResult::InProgress:		return TEXT("InProgress");
	case EPGXSaveResult::Cancelled:			return TEXT("Cancelled");
	case EPGXSaveResult::ProviderError:		return TEXT("ProviderError");
	case EPGXSaveResult::DomainNotFound:	return TEXT("DomainNotFound");
	case EPGXSaveResult::ContextNotFound:	return TEXT("ContextNotFound");
	default:								return TEXT("Unknown");
	}
}

EPGXSaveResult FPGXSaveBackupManager::StringToResult(const FString& Str)
{
	if (Str == TEXT("Success"))			return EPGXSaveResult::Success;
	if (Str == TEXT("Failed"))			return EPGXSaveResult::Failed;
	if (Str == TEXT("SlotNotFound"))		return EPGXSaveResult::SlotNotFound;
	if (Str == TEXT("SlotFull"))			return EPGXSaveResult::SlotFull;
	if (Str == TEXT("Corrupted"))		return EPGXSaveResult::Corrupted;
	if (Str == TEXT("VersionMismatch"))	return EPGXSaveResult::VersionMismatch;
	if (Str == TEXT("InProgress"))		return EPGXSaveResult::InProgress;
	if (Str == TEXT("Cancelled"))		return EPGXSaveResult::Cancelled;
	if (Str == TEXT("ProviderError"))	return EPGXSaveResult::ProviderError;
	if (Str == TEXT("DomainNotFound"))	return EPGXSaveResult::DomainNotFound;
	if (Str == TEXT("ContextNotFound"))	return EPGXSaveResult::ContextNotFound;
	return EPGXSaveResult::Failed;
}
