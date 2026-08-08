// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXSaveTypes.h"

/**
 * EN: Static utility for save file backup management and transaction logging.
 *     Before each save, the system can create a .bak copy of the existing file.
 *     Maintains a transaction log (.ini) recording all save operations for auditing.
 *     If a save file is corrupted, the system can restore from the most recent backup.
 *
 *     Backup naming: {OriginalPath}.bak.1, .bak.2, etc. (newest = .bak.1)
 *     Transaction log: _Transactions.ini in the context root directory.
 *
 * ES: Utilidad estatica para gestion de backups de archivos de guardado y transaction logging.
 *     Antes de cada guardado, el sistema puede crear una copia .bak del archivo existente.
 *     Mantiene un transaction log (.ini) registrando todas las operaciones para auditoria.
 *     Si un archivo de guardado esta corrupto, el sistema puede restaurar del backup mas reciente.
 *
 *     Nomenclatura de backup: {OriginalPath}.bak.1, .bak.2, etc. (mas nuevo = .bak.1)
 *     Transaction log: _Transactions.ini en el directorio raiz del contexto.
 */
struct PGXSAVERUNTIME_API FPGXSaveBackupManager
{
	// ========================================================================
	// EN: Backup operations
	// ES: Operaciones de backup
	// ========================================================================

	/**
	 * EN: Create a backup of the specified file by rotating existing backups.
	 *     Newest backup is always .bak.1. Older backups are shifted: .bak.1 -> .bak.2, etc.
	 *     If MaxBackups is exceeded, the oldest backup is deleted.
	 *
	 * ES: Crear un backup del archivo especificado rotando los backups existentes.
	 *     El backup mas nuevo siempre es .bak.1. Los backups mas antiguos se desplazan.
	 *     Si se excede MaxBackups, el backup mas antiguo se elimina.
	 *
	 * @param FilePath    EN: Full path to the file to back up / ES: Path completo al archivo a respaldar
	 * @param MaxBackups  EN: Maximum number of backup copies to keep / ES: Numero maximo de copias de backup a mantener
	 * @return            EN: True if backup was created or file didn't exist / ES: True si se creo el backup o el archivo no existia
	 */
	static bool CreateBackup(const FString& FilePath, int32 MaxBackups);

	/**
	 * EN: Restore the most recent backup copy to the original file path.
	 *     Searches for .bak.1 first, then .bak.2, etc.
	 *
	 * ES: Restaurar la copia de backup mas reciente al path original del archivo.
	 *     Busca .bak.1 primero, luego .bak.2, etc.
	 *
	 * @param FilePath  EN: Original file path to restore to / ES: Path original del archivo a restaurar
	 * @return          EN: True if a backup was found and restored / ES: True si se encontro y restauro un backup
	 */
	static bool RestoreFromBackup(const FString& FilePath);

	/**
	 * EN: Get all backup file paths for a given file, ordered newest first.
	 * ES: Obtener todos los paths de backup para un archivo dado, ordenados del mas nuevo al mas viejo.
	 */
	static TArray<FString> GetBackupFiles(const FString& FilePath);

	/**
	 * EN: Delete all backup files for a given file path.
	 * ES: Eliminar todos los archivos de backup para un path de archivo dado.
	 */
	static void CleanupAllBackups(const FString& FilePath);

	// ========================================================================
	// EN: Transaction log
	// ES: Transaction log
	// ========================================================================

	/**
	 * EN: Append a transaction entry to the .ini transaction log.
	 *     Entries are stored as numbered keys in [PGXSave.TransactionLog].
	 *     Format: SlotName|ISO8601|Checksum|Operation|Result|FilePath
	 *
	 * ES: Agregar una entrada de transaccion al transaction log .ini.
	 *     Las entradas se almacenan como keys numeradas en [PGXSave.TransactionLog].
	 *     Formato: SlotName|ISO8601|Checksum|Operation|Result|FilePath
	 *
	 * @param TransactionLogPath  EN: Full path to the _Transactions.ini file / ES: Path completo al archivo _Transactions.ini
	 * @param Entry               EN: Transaction entry data / ES: Datos de la entrada de transaccion
	 */
	static void LogTransaction(const FString& TransactionLogPath, const FPGXBackupEntry& Entry);

	/**
	 * EN: Read transaction history from the .ini log file.
	 * ES: Leer historial de transacciones desde el archivo .ini de log.
	 *
	 * @param TransactionLogPath  EN: Full path to the _Transactions.ini file / ES: Path completo al archivo _Transactions.ini
	 * @param MaxEntries          EN: Maximum entries to return (0 = all) / ES: Maximo de entradas a retornar (0 = todas)
	 * @return                    EN: Array of entries, newest first / ES: Array de entradas, mas nueva primero
	 */
	static TArray<FPGXBackupEntry> GetTransactionHistory(const FString& TransactionLogPath, int32 MaxEntries = 50);

	/**
	 * EN: Resolve the transaction log path for a given context directory.
	 * ES: Resolver el path del transaction log para un directorio de contexto dado.
	 */
	static FString ResolveTransactionLogPath(const FString& ContextDirectoryPath);

private:
	FPGXSaveBackupManager() = delete;

	/** EN: Get backup path for a specific index / ES: Obtener path de backup para un indice especifico */
	static FString GetBackupPath(const FString& OriginalPath, int32 BackupIndex);

	/** EN: Convert operation enum to string / ES: Convertir enum de operacion a string */
	static FString OperationToString(EPGXSaveOperation Operation);

	/** EN: Convert string back to operation enum / ES: Convertir string de vuelta a enum de operacion */
	static EPGXSaveOperation StringToOperation(const FString& Str);

	/** EN: Convert result enum to string / ES: Convertir enum de resultado a string */
	static FString ResultToString(EPGXSaveResult Result);

	/** EN: Convert string back to result enum / ES: Convertir string de vuelta a enum de resultado */
	static EPGXSaveResult StringToResult(const FString& Str);
};
