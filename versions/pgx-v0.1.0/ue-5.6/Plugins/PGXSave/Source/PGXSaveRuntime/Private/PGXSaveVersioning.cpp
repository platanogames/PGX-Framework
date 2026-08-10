// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXSaveVersioning.h"
#include "PGXSaveGame.h"
#include "Logging/PGXLogCategories.h"
#include "Logging/PGXLogMacros.h"

// EN: Save data versioning and migration implementation
// ES: Implementacion del sistema de versionado y migracion de datos de guardado

TArray<FPGXSaveVersioning::FMigrationStep> FPGXSaveVersioning::RegisteredMigrations;

// ============================================================================
// EN: Migration registration
// ES: Registro de migraciones
// ============================================================================

void FPGXSaveVersioning::RegisterMigration(int32 FromVersion, int32 ToVersion,
	TFunction<bool(UPGXSaveGame*)> MigrationFn)
{
	if (FromVersion >= ToVersion)
	{
		PGX_LOG_WARNING(LogPGXSave,
			TEXT("[Versioning] Invalid migration: FromVersion (%d) must be less than ToVersion (%d)"),
			FromVersion, ToVersion);
		return;
	}

	if (!MigrationFn)
	{
		PGX_LOG_WARNING(LogPGXSave,
			TEXT("[Versioning] Null migration function for %d -> %d"),
			FromVersion, ToVersion);
		return;
	}

	// EN: Check for duplicate
	// ES: Comprobar duplicado
	if (FindMigration(FromVersion, ToVersion) != nullptr)
	{
		PGX_LOG_WARNING(LogPGXSave,
			TEXT("[Versioning] Migration %d -> %d already registered, overwriting"),
			FromVersion, ToVersion);

		RegisteredMigrations.RemoveAll([FromVersion, ToVersion](const FMigrationStep& Step)
		{
			return Step.FromVersion == FromVersion && Step.ToVersion == ToVersion;
		});
	}

	FMigrationStep Step;
	Step.FromVersion = FromVersion;
	Step.ToVersion = ToVersion;
	Step.MigrationFn = MoveTemp(MigrationFn);
	RegisteredMigrations.Add(MoveTemp(Step));

	PGX_LOG_INFO(LogPGXSave, TEXT("[Versioning] Registered migration: v%d -> v%d (total: %d)"),
		FromVersion, ToVersion, RegisteredMigrations.Num());
}

// ============================================================================
// EN: Migration execution
// ES: Ejecucion de migracion
// ============================================================================

EPGXSaveResult FPGXSaveVersioning::MigrateToVersion(UPGXSaveGame* SaveGame, int32 TargetVersion)
{
	if (!SaveGame)
	{
		PGX_LOG_ERROR(LogPGXSave, TEXT("[Versioning] MigrateToVersion: SaveGame is null"));
		return EPGXSaveResult::Failed;
	}

	const int32 CurrentVersion = SaveGame->SaveFormatVersion;

	if (CurrentVersion == TargetVersion)
	{
		return EPGXSaveResult::Success; // EN: Already at target / ES: Ya esta en la version destino
	}

	if (CurrentVersion > TargetVersion)
	{
		// EN: Downgrade not supported
		// ES: Downgrade no soportado
		PGX_LOG_WARNING(LogPGXSave,
			TEXT("[Versioning] Downgrade not supported: save is v%d, target is v%d"),
			CurrentVersion, TargetVersion);
		return EPGXSaveResult::VersionMismatch;
	}

	// EN: Check if complete migration path exists before starting
	// ES: Comprobar si existe un camino completo de migracion antes de empezar
	if (!HasMigrationPath(CurrentVersion, TargetVersion))
	{
		PGX_LOG_ERROR(LogPGXSave,
			TEXT("[Versioning] No migration path from v%d to v%d"),
			CurrentVersion, TargetVersion);
		return EPGXSaveResult::VersionMismatch;
	}

	// EN: Execute migration chain step by step
	// ES: Ejecutar cadena de migracion paso a paso
	int32 RunningVersion = CurrentVersion;

	while (RunningVersion < TargetVersion)
	{
		// EN: Find the next step (try direct jump first, then +1 increment)
		// ES: Encontrar el siguiente paso (intentar salto directo primero, luego incremento +1)
		const FMigrationStep* Step = FindMigration(RunningVersion, RunningVersion + 1);
		if (!Step)
		{
			// EN: Try a larger jump (e.g., v1 -> v3 directly)
			// ES: Intentar un salto mas grande (ej. v1 -> v3 directamente)
			bool bFoundJump = false;
			for (int32 JumpTo = RunningVersion + 2; JumpTo <= TargetVersion; ++JumpTo)
			{
				Step = FindMigration(RunningVersion, JumpTo);
				if (Step)
				{
					bFoundJump = true;
					break;
				}
			}

			if (!bFoundJump)
			{
				PGX_LOG_ERROR(LogPGXSave,
					TEXT("[Versioning] Migration chain broken at v%d (target: v%d)"),
					RunningVersion, TargetVersion);
				return EPGXSaveResult::VersionMismatch;
			}
		}

		PGX_LOG_INFO(LogPGXSave, TEXT("[Versioning] Running migration: v%d -> v%d (class: %s)"),
			Step->FromVersion, Step->ToVersion, *SaveGame->GetClass()->GetName());

		if (!Step->MigrationFn(SaveGame))
		{
			PGX_LOG_ERROR(LogPGXSave,
				TEXT("[Versioning] Migration v%d -> v%d FAILED for '%s'"),
				Step->FromVersion, Step->ToVersion, *SaveGame->GetClass()->GetName());
			return EPGXSaveResult::Failed;
		}

		RunningVersion = Step->ToVersion;
		SaveGame->SaveFormatVersion = RunningVersion;
	}

	PGX_LOG_INFO(LogPGXSave, TEXT("[Versioning] Migration complete: v%d -> v%d for '%s'"),
		CurrentVersion, TargetVersion, *SaveGame->GetClass()->GetName());

	return EPGXSaveResult::Success;
}

// ============================================================================
// EN: Queries
// ES: Consultas
// ============================================================================

bool FPGXSaveVersioning::HasMigrationPath(int32 FromVersion, int32 ToVersion)
{
	if (FromVersion >= ToVersion)
	{
		return FromVersion == ToVersion;
	}

	// EN: Walk the chain to verify a complete path exists
	// ES: Recorrer la cadena para verificar que existe un camino completo
	int32 Current = FromVersion;
	int32 MaxIterations = ToVersion - FromVersion + 1; // EN: Safety limit / ES: Limite de seguridad

	while (Current < ToVersion && MaxIterations-- > 0)
	{
		bool bFoundNext = false;

		// EN: Try incremental step first, then larger jumps
		// ES: Intentar paso incremental primero, luego saltos mas grandes
		for (int32 JumpTo = Current + 1; JumpTo <= ToVersion; ++JumpTo)
		{
			if (FindMigration(Current, JumpTo))
			{
				Current = JumpTo;
				bFoundNext = true;
				break;
			}
		}

		if (!bFoundNext)
		{
			return false;
		}
	}

	return Current == ToVersion;
}

int32 FPGXSaveVersioning::GetRegisteredMigrationCount()
{
	return RegisteredMigrations.Num();
}

void FPGXSaveVersioning::ClearAllMigrations()
{
	RegisteredMigrations.Empty();
}

// ============================================================================
// EN: Internal helpers
// ES: Helpers internos
// ============================================================================

const FPGXSaveVersioning::FMigrationStep* FPGXSaveVersioning::FindMigration(int32 FromVersion, int32 ToVersion)
{
	for (const FMigrationStep& Step : RegisteredMigrations)
	{
		if (Step.FromVersion == FromVersion && Step.ToVersion == ToVersion)
		{
			return &Step;
		}
	}
	return nullptr;
}
