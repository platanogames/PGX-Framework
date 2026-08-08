// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "IDirectoryWatcher.h"
#include "Containers/Ticker.h"

/**
 * EN: Watches documentation directories for file changes.
 *     Uses FDirectoryWatcher with debounce (300ms) and batch processing.
 *     Notifies FDocSystem when .md files change.
 *
 * ES: Observa directorios de documentacion para cambios de archivos.
 *     Usa FDirectoryWatcher con debounce (300ms) y procesamiento batch.
 *     Notifica a FDocSystem cuando cambian archivos .md.
 */
class FDocWatcher
{
public:
	~FDocWatcher();

	/** EN: Start watching the given directories / ES: Empezar a observar los directorios dados */
	void Start(const TArray<FString>& Directories);

	/** EN: Stop watching all directories / ES: Dejar de observar todos los directorios */
	void Stop();

	/** EN: Whether the watcher is active / ES: Si el watcher esta activo */
	bool IsActive() const { return bIsActive; }

private:
	// EN: Callback from DirectoryWatcher / ES: Callback de DirectoryWatcher
	void OnDirectoryChanged(const TArray<FFileChangeData>& FileChanges);

	// EN: Process debounced changes / ES: Procesar cambios con debounce
	void ProcessPendingChanges();

	bool bIsActive = false;

	/** EN: Registered watcher delegates / ES: Delegados de watcher registrados */
	TArray<FDelegateHandle> WatcherHandles;

	/** EN: Directories being watched / ES: Directorios siendo observados */
	TArray<FString> WatchedDirectories;

	/** EN: Pending changes accumulated during debounce / ES: Cambios pendientes acumulados durante debounce */
	TArray<FString> PendingChanges;

	/** EN: Debounce ticker handle — must be cancelled on Stop/destroy to prevent use-after-free
	  * ES: Handle de ticker debounce — debe cancelarse en Stop/destroy para prevenir use-after-free */
	FTSTicker::FDelegateHandle DebounceTickerHandle;
};
