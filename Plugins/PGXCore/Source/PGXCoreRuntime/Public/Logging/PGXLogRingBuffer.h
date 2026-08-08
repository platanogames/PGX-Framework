// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Logging/PGXLogTypes.h"

/**
 * EN: Thread-safe circular buffer for log entries.
 *     Overwrites oldest entries when full. Capacity configured by Config DA profile.
 *
 *     v2.0: Uses FCriticalSection (mutex) for thread safety.
 *     v2.1 upgrade path: MPSC lock-free queue (multi-producer=worker threads,
 *     single-consumer=GameThread). Documented here for future optimization.
 *
 * ES: Buffer circular thread-safe para entradas de log.
 *     Sobrescribe las mas antiguas cuando esta lleno. Capacidad configurada por perfil Config DA.
 *
 *     v2.0: Usa FCriticalSection (mutex) para thread safety.
 *     v2.1 upgrade path: Cola MPSC lock-free (multi-producer=worker threads,
 *     single-consumer=GameThread). Documentado aqui para optimizacion futura.
 */
class PGXCORERUNTIME_API FPGXLogRingBuffer
{
public:
	/**
	 * EN: Construct with initial capacity / ES: Construir con capacidad inicial
	 * @param InCapacity Number of entries before overwrite (default 4096)
	 */
	explicit FPGXLogRingBuffer(int32 InCapacity = 4096);

	/**
	 * EN: Push a new entry. Overwrites oldest if buffer is full. Thread-safe.
	 * ES: Insertar nueva entrada. Sobrescribe la mas antigua si el buffer esta lleno. Thread-safe.
	 */
	void Push(FPGXLogEntry&& Entry);

	/**
	 * EN: Thread-safe snapshot of all entries in chronological order.
	 *     Use this for async operations: snapshot (short lock) → release → process.
	 * ES: Snapshot thread-safe de todas las entradas en orden cronologico.
	 *     Usar para operaciones async: snapshot (lock corto) → release → procesar.
	 */
	TArray<FPGXLogEntry> Snapshot() const;

	/**
	 * EN: Filtered snapshot by minimum verbosity and/or category. Thread-safe.
	 * ES: Snapshot filtrado por verbosity minima y/o categoria. Thread-safe.
	 * @param MinLevel Minimum verbosity to include
	 * @param CategoryFilter If not NAME_None, only include this category
	 */
	TArray<FPGXLogEntry> SnapshotFiltered(EPGXLogVerbosity MinLevel, FName CategoryFilter = NAME_None) const;

	/**
	 * EN: Snapshot filtered by domain tag. Thread-safe.
	 * ES: Snapshot filtrado por tag de dominio. Thread-safe.
	 */
	TArray<FPGXLogEntry> SnapshotByDomain(const FGameplayTag& DomainTag) const;

	/** EN: Current entry count / ES: Conteo actual de entradas */
	int32 Num() const;

	/** EN: Clear all entries. Thread-safe. / ES: Limpiar todas las entradas. Thread-safe. */
	void Clear();

	/**
	 * EN: Resize the buffer. Clears existing entries. Thread-safe.
	 * ES: Redimensionar el buffer. Limpia entradas existentes. Thread-safe.
	 */
	void Resize(int32 NewCapacity);

private:
	TArray<FPGXLogEntry> Buffer;
	int32 Head = 0;
	int32 Count = 0;
	int32 Capacity;

	/** EN: Mutex for thread safety (v2.0) / ES: Mutex para thread safety (v2.0) */
	mutable FCriticalSection Lock;
};
