// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXSaveTypes.h"

class UPGXSaveGame;

/**
 * EN: Save data versioning and migration system.
 *     Allows projects to register migration steps that transform SaveGame data
 *     from one format version to the next. The load pipeline calls MigrateToVersion()
 *     automatically when a loaded SaveGame's version differs from the Config's current version.
 *
 *     Usage (register in your GameInstance::Init or similar startup):
 *       FPGXSaveVersioning::RegisterMigration(1, 2, [](UPGXSaveGame* SG) {
 *           // Transform v1 data to v2 format
 *           FString OldValue = SG->ReadString(FName("OldKey"));
 *           SG->WriteString(FName("NewKey"), OldValue);
 *           return true;
 *       });
 *
 *     Migrations are chained: to go from v1 to v3, register v1->v2 and v2->v3.
 *     The system runs them sequentially: v1->v2, then v2->v3.
 *
 * ES: Sistema de versionado y migracion de datos de guardado.
 *     Permite a los proyectos registrar pasos de migracion que transforman datos de SaveGame
 *     de una version de formato a la siguiente. El pipeline de carga llama MigrateToVersion()
 *     automaticamente cuando la version del SaveGame cargado difiere de la version actual del Config.
 *
 *     Uso (registrar en tu GameInstance::Init o startup similar):
 *       FPGXSaveVersioning::RegisterMigration(1, 2, [](UPGXSaveGame* SG) {
 *           // Transformar datos v1 a formato v2
 *           FString OldValue = SG->ReadString(FName("OldKey"));
 *           SG->WriteString(FName("NewKey"), OldValue);
 *           return true;
 *       });
 *
 *     Las migraciones se encadenan: para ir de v1 a v3, registrar v1->v2 y v2->v3.
 *     El sistema las ejecuta secuencialmente: v1->v2, luego v2->v3.
 */
struct PGXSAVERUNTIME_API FPGXSaveVersioning
{
	FPGXSaveVersioning() = delete;

	// ========================================================================
	// EN: Migration registration
	// ES: Registro de migraciones
	// ========================================================================

	/**
	 * EN: Register a migration step from one version to the next.
	 * ES: Registrar un paso de migracion de una version a la siguiente.
	 *
	 * @param FromVersion   EN: Source format version / ES: Version de formato origen
	 * @param ToVersion     EN: Target format version (should be FromVersion + 1) / ES: Version de formato destino (deberia ser FromVersion + 1)
	 * @param MigrationFn   EN: Function that transforms the SaveGame data, returns true on success / ES: Funcion que transforma los datos del SaveGame, retorna true en exito
	 */
	static void RegisterMigration(int32 FromVersion, int32 ToVersion,
		TFunction<bool(UPGXSaveGame*)> MigrationFn);

	// ========================================================================
	// EN: Migration execution
	// ES: Ejecucion de migracion
	// ========================================================================

	/**
	 * EN: Migrate a SaveGame from its current version to the target version.
	 *     Runs all registered migration steps sequentially.
	 *     Updates SaveGame->SaveFormatVersion on success.
	 *
	 * ES: Migrar un SaveGame desde su version actual a la version destino.
	 *     Ejecuta todos los pasos de migracion registrados secuencialmente.
	 *     Actualiza SaveGame->SaveFormatVersion en exito.
	 *
	 * @param SaveGame       EN: The SaveGame to migrate / ES: El SaveGame a migrar
	 * @param TargetVersion  EN: Desired format version / ES: Version de formato deseada
	 * @return               EN: Success if all steps succeeded, VersionMismatch if no path exists / ES: Success si todos los pasos fueron exitosos, VersionMismatch si no existe camino
	 */
	static EPGXSaveResult MigrateToVersion(UPGXSaveGame* SaveGame, int32 TargetVersion);

	// ========================================================================
	// EN: Queries
	// ES: Consultas
	// ========================================================================

	/**
	 * EN: Check if a migration path exists from one version to another.
	 * ES: Comprobar si existe un camino de migracion de una version a otra.
	 */
	static bool HasMigrationPath(int32 FromVersion, int32 ToVersion);

	/** EN: Get the number of registered migration steps / ES: Obtener el numero de pasos de migracion registrados */
	static int32 GetRegisteredMigrationCount();

	/** EN: Clear all registered migrations / ES: Limpiar todas las migraciones registradas */
	static void ClearAllMigrations();

private:
	/** EN: Internal migration step / ES: Paso de migracion interno */
	struct FMigrationStep
	{
		int32 FromVersion;
		int32 ToVersion;
		TFunction<bool(UPGXSaveGame*)> MigrationFn;
	};

	/** EN: Find migration step for a specific version transition / ES: Encontrar paso de migracion para una transicion de version especifica */
	static const FMigrationStep* FindMigration(int32 FromVersion, int32 ToVersion);

	/** EN: All registered migration steps / ES: Todos los pasos de migracion registrados */
	static TArray<FMigrationStep> RegisteredMigrations;
};
