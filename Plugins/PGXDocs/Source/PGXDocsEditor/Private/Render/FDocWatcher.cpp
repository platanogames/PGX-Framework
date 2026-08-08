// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Render/FDocWatcher.h"
#include "PGXDocsModule.h"
#include "Logging/PGXLogMacros.h"
#include "PGXDocSystem.h"
#include "DirectoryWatcherModule.h"
#include "IDirectoryWatcher.h"
#include "Modules/ModuleManager.h"
#include "Misc/Paths.h"
#include "Containers/Ticker.h"

FDocWatcher::~FDocWatcher()
{
	Stop();
}

void FDocWatcher::Start(const TArray<FString>& Directories)
{
	Stop();

	FDirectoryWatcherModule& DirWatcherModule = FModuleManager::LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
	IDirectoryWatcher* DirWatcher = DirWatcherModule.Get();

	if (!DirWatcher)
	{
		PGX_LOG_WARNING(LogPGXDocs, TEXT("PGXDocs: DirectoryWatcher not available"));
		return;
	}

	WatchedDirectories = Directories;

	for (const FString& Dir : Directories)
	{
		if (!FPaths::DirectoryExists(Dir))
		{
			continue;
		}

		FDelegateHandle Handle;
		DirWatcher->RegisterDirectoryChangedCallback_Handle(
			Dir,
			IDirectoryWatcher::FDirectoryChanged::CreateRaw(this, &FDocWatcher::OnDirectoryChanged),
			Handle,
			IDirectoryWatcher::WatchOptions::IncludeDirectoryChanges
		);

		WatcherHandles.Add(Handle);
	}

	bIsActive = true;
	PGX_LOG_INFO(LogPGXDocs, TEXT("PGXDocs: Watcher started for %d directories"), Directories.Num());
}

void FDocWatcher::Stop()
{
	if (!bIsActive)
	{
		return;
	}

	// EN: Use GetModulePtr instead of LoadModuleChecked — module may already be unloaded during shutdown
	// ES: Usar GetModulePtr en vez de LoadModuleChecked — el modulo puede estar descargado durante shutdown
	FDirectoryWatcherModule* DirWatcherModule = FModuleManager::GetModulePtr<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
	IDirectoryWatcher* DirWatcher = DirWatcherModule ? DirWatcherModule->Get() : nullptr;

	if (DirWatcher)
	{
		for (int32 i = 0; i < WatcherHandles.Num() && i < WatchedDirectories.Num(); ++i)
		{
			DirWatcher->UnregisterDirectoryChangedCallback_Handle(WatchedDirectories[i], WatcherHandles[i]);
		}
	}

	// EN: Cancel pending debounce ticker / ES: Cancelar ticker de debounce pendiente
	if (DebounceTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(DebounceTickerHandle);
		DebounceTickerHandle.Reset();
	}

	WatcherHandles.Empty();
	WatchedDirectories.Empty();
	PendingChanges.Empty();
	bIsActive = false;

	PGX_LOG_INFO(LogPGXDocs, TEXT("PGXDocs: Watcher stopped"));
}

void FDocWatcher::OnDirectoryChanged(const TArray<FFileChangeData>& FileChanges)
{
	for (const FFileChangeData& Change : FileChanges)
	{
		// EN: Only care about .md files / ES: Solo interesa archivos .md
		if (Change.Filename.EndsWith(TEXT(".md")))
		{
			PendingChanges.AddUnique(Change.Filename);
		}
	}

	// EN: Debounce — process after 300ms of quiet / ES: Debounce — procesar despues de 300ms de silencio
	if (PendingChanges.Num() > 0)
	{
		// EN: Cancel previous debounce if still pending, then schedule new one
		// ES: Cancelar debounce anterior si aun esta pendiente, luego programar nuevo
		if (DebounceTickerHandle.IsValid())
		{
			FTSTicker::GetCoreTicker().RemoveTicker(DebounceTickerHandle);
		}

		DebounceTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateLambda([this](float /*DeltaTime*/) -> bool
			{
				DebounceTickerHandle.Reset();
				ProcessPendingChanges();
				return false; // EN: One-shot / ES: Una sola vez
			}),
			0.3f // EN: 300ms delay / ES: 300ms de retraso
		);
	}
}

void FDocWatcher::ProcessPendingChanges()
{
	if (PendingChanges.Num() == 0)
	{
		return;
	}

	TArray<FString> ChangesToProcess = MoveTemp(PendingChanges);
	PendingChanges.Empty();

	PGX_LOG_VERBOSE(LogPGXDocs, TEXT("PGXDocs: Watcher processing %d changed files"), ChangesToProcess.Num());

	// EN: Notify the doc system / ES: Notificar al sistema de docs
	FDocSystem::Get().OnFilesChanged(ChangesToProcess);
}
