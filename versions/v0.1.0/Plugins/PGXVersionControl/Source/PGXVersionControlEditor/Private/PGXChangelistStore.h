// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXVersionControlTypes.h"
#include "HAL/CriticalSection.h"

/**
 * EN: Persistent storage for PGX changelists. Serializes to JSON at
 *     {ProjectDir}/Saved/PGX/changelists.json with atomic write (tmp+rename).
 *     Thread-safe via FCriticalSection (expected game-thread only, guard is defensive).
 *
 * ES: Almacenamiento persistente para changelists PGX. Serializa a JSON en
 *     {ProjectDir}/Saved/PGX/changelists.json con escritura atomica (tmp+rename).
 *     Thread-safe via FCriticalSection (se espera solo game-thread, guard es defensivo).
 */
class FPGXChangelistStore
{
public:
	FPGXChangelistStore();
	explicit FPGXChangelistStore(const FString& InSavePathOverride);

	/** EN: Load changelists from disk. Creates default CL if file doesn't exist. / ES: Cargar changelists de disco. */
	void Load();

	/** EN: Save changelists to disk with atomic write. / ES: Guardar changelists a disco con escritura atomica. */
	void Save();
	FPGXVersionControlOperationResult SaveWithResult();

	// ─── CRUD ───

	const TArray<FPGXChangelist>& GetChangelists() const { return Changelists; }
	const FPGXChangelist* FindChangelist(const FGuid& InGuid) const;
	FPGXChangelist* FindChangelistMutable(const FGuid& InGuid);

	/** EN: Create a new changelist. Returns its Guid. / ES: Crear una nueva changelist. Retorna su Guid. */
	FGuid CreateChangelist(const FString& InName, const FString& InDescription);

	/** EN: Delete a changelist (moves files to Default). Returns false if it's the default CL. / ES: Borrar changelist. */
	bool DeleteChangelist(const FGuid& InGuid);

	/** EN: Rename a changelist. / ES: Renombrar una changelist. */
	bool RenameChangelist(const FGuid& InGuid, const FString& InNewName);

	/** EN: Move a file to a target changelist. Removes from current CL if present. / ES: Mover archivo a CL objetivo. */
	bool MoveFileToChangelist(const FString& InFilePath, const FGuid& InTargetGuid);

	/** EN: Get the default changelist Guid. / ES: Obtener el Guid de la changelist por defecto. */
	FGuid GetDefaultChangelistGuid() const;

	FPGXVersionControlOperationResult GetLastOperationResult() const { return LastOperationResult; }

	/** EN: Broadcast when changelists change / ES: Broadcast cuando changelists cambian */
	FOnPGXChangelistsChanged OnChangelistsChanged;

private:
	FString GetSavePath() const;
	void EnsureDefaultChangelist();
	void SetLastOperationResult(EPGXVersionControlOperationStatus InStatus, const FString& InMessage, const FString& InContextPath = FString());

	TArray<FPGXChangelist> Changelists;
	FString SavePathOverride;
	FPGXVersionControlOperationResult LastOperationResult;
	mutable FCriticalSection CriticalSection;

	static constexpr int32 JsonVersion = 1;
};
