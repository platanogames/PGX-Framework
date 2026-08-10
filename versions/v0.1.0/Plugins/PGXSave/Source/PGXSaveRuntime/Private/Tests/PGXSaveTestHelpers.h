// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "PGXSaveTypes.h"

class UPGXSaveSubsystem;
class UPGXSaveConfig;
class UPGXSaveProvider;
class UGameInstance;

/**
 * EN: Focused helper utilities for isolated PGXSave automation fixtures,
 *     file mutation, byte comparison, and local GameInstance lifecycle.
 * ES: Helpers focalizados para fixtures aislados de PGXSave, mutacion de
 *     archivos, comparacion de bytes y ciclo de vida de GameInstance local.
 */
namespace PGXSaveTestHelpers
{
	/**
	 * EN: Create a transient UGameInstance for in-test fixturing, fully bootstrap
	 *     its subsystems via InitializeStandalone, and root it for the duration
	 *     of the test. Returns nullptr on failure (no GEngine, allocation failed).
	 *     Used because the shared test helper may not provide a GameInstance in this context.
	 * ES: Crear un UGameInstance transitorio para fixturing in-test, bootstrap
	 *     completo de sus subsistemas via InitializeStandalone, y rootearlo por
	 *     la duracion del test. Retorna nullptr en fallo (sin GEngine, alloc
	 *     fallido). Se usa porque el helper compartido puede no proporcionar GameInstance en este contexto.
	 */
	PGXSAVERUNTIME_API UGameInstance* CreateLocalTestGameInstance();

	/**
	 * EN: Tear down a UGameInstance previously created by CreateLocalTestGameInstance.
	 *     Calls Shutdown() to deinitialize subsystems and removes the root
	 *     reference. Safe to pass nullptr.
	 * ES: Tear down de un UGameInstance creado previamente por
	 *     CreateLocalTestGameInstance. Llama Shutdown() para deinicializar
	 *     subsistemas y remueve la referencia root. Seguro pasar nullptr.
	 */
	PGXSAVERUNTIME_API void TearDownLocalTestGameInstance(UGameInstance* GameInstance);

	/**
	 * EN: Build a unique test slot name to avoid collisions with player saves.
	 *     Format: "Test_<Prefix>_<GuidShort>".
	 * ES: Construir un nombre de slot de test unico para evitar colisiones con
	 *     saves del jugador. Formato: "Test_<Prefix>_<GuidShort>".
	 */
	PGXSAVERUNTIME_API FString MakeUniqueTestSlotName(const FString& Prefix);

	/**
	 * EN: Flip the last byte of a file on disk to simulate corruption. Used by
	 *     save-integrity corruption detection test.
	 * ES: Voltear el ultimo byte de un archivo en disco para simular corrupcion.
	 *     Usado por el test save-integrity de deteccion de corrupcion.
	 *
	 * @param FilePath  EN: Absolute path to a file (must exist) / ES: Path absoluto a un archivo (debe existir)
	 * @return          EN: True if file was read, mutated, and written back / ES: True si el archivo fue leido, mutado y escrito
	 */
	PGXSAVERUNTIME_API bool MutateLastByteOfFile(const FString& FilePath);

	/**
	 * EN: Load raw bytes from a file via FFileHelper. Returns false if the file
	 *     does not exist or cannot be read.
	 * ES: Cargar bytes crudos desde un archivo via FFileHelper. Retorna false si el
	 *     archivo no existe o no puede ser leido.
	 */
	PGXSAVERUNTIME_API bool LoadFileBytes(const FString& FilePath, TArray<uint8>& OutBytes);

	/**
	 * EN: Resolve the absolute file path for the first save domain in a context.
	 *     Used by tests that need direct file access without going through the
	 *     subsystem's high-level API. Returns empty if context/binding/provider
	 *     unresolved.
	 * ES: Resolver el path absoluto del archivo del primer dominio de save en un
	 *     contexto. Usado por tests que necesitan acceso directo al archivo sin
	 *     pasar por la API de alto nivel del subsistema. Retorna vacio si no se
	 *     resuelven contexto/binding/provider.
	 */
	PGXSAVERUNTIME_API FString ResolveFirstDomainFilePath(
		UPGXSaveSubsystem* Subsystem,
		FGameplayTag ContextTag,
		const FString& SlotName);

	/**
	 * EN: Compare outcomes of a sync save/load pair vs an async save/load pair
	 *     for the same context+config. Captures result-code mismatch and byte
	 *     divergence; populates OutDiffSummary with a one-line summary suitable
	 *     for AddError/AddInfo. Returns true iff fully equivalent (same result
	 *     and byte-equivalent persisted payload).
	 *
	 *     This helper scope: result-code + raw byte comparison only. Migration
	 *     parity is asserted indirectly (if migration runs in one path and not the
	 *     other, persisted bytes differ).
	 *
	 * ES: Comparar resultados de un par sync save/load vs un par async save/load
	 *     para el mismo contexto+config. Captura mismatch de codigo de resultado
	 *     y divergencia de bytes; popula OutDiffSummary con un resumen de una linea
	 *     adecuado para AddError/AddInfo. Retorna true sii son completamente
	 *     equivalentes (mismo resultado y payload persistido byte-equivalente).
	 *
	 *     Scope de este helper: solo comparacion de codigo de resultado + bytes
	 *     crudos. Paridad de migracion se asserta indirectamente (si migracion corre
	 *     en un path y no en otro, los bytes persistidos difieren).
	 */
	PGXSAVERUNTIME_API bool ComparePipelineOutcomes(
		EPGXSaveResult SyncResult,
		EPGXSaveResult AsyncResult,
		const TArray<uint8>& SyncPersistedBytes,
		const TArray<uint8>& AsyncPersistedBytes,
		FString& OutDiffSummary);
}

#endif // WITH_DEV_AUTOMATION_TESTS
