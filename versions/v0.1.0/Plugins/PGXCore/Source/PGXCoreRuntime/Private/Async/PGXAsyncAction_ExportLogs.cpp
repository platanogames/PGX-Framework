// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Async/PGXAsyncAction_ExportLogs.h"
#include "Subsystems/PGXLogSubsystem.h"
#include "Logging/PGXLogSerializer.h"
#include "Logging/PGXLogCategories.h"
#include "Tasks/Task.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/GameInstance.h"

// EN: Async Blueprint action for exporting PGX logs to JSON
// ES: Accion async de Blueprint para exportar logs PGX a JSON

UPGXAsyncAction_ExportLogs* UPGXAsyncAction_ExportLogs::ExportLogsToJson(
	UObject* WorldContextObject,
	const FString& ExportPath)
{
	UPGXAsyncAction_ExportLogs* Action = NewObject<UPGXAsyncAction_ExportLogs>();
	Action->RequestedPath = ExportPath;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UPGXAsyncAction_ExportLogs::Activate()
{
	UPGXLogSubsystem* LogSub = UPGXLogSubsystem::GetCached();
	if (!LogSub)
	{
		OnFailure.Broadcast(TEXT("PGX Log Subsystem not available"));
		SetReadyToDestroy();
		return;
	}

	// EN: Snapshot (short lock) → release → serialize async
	// ES: Snapshot (lock corto) → release → serializar async
	TArray<FPGXLogEntry> Entries = LogSub->GetCurrentSessionEntries();
	FPGXLogSession Session = LogSub->GetCurrentSession();

	FString ExportPath = RequestedPath;
	if (ExportPath.IsEmpty())
	{
		ExportPath = FPaths::Combine(
			FPaths::ProjectSavedDir(),
			TEXT("Logs/PGX"),
			FString::Printf(TEXT("PGXLog_Export_%s.json"),
				*FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"))));
	}

	// EN: Weak reference to self for callback safety
	// ES: Referencia debil a self para seguridad del callback
	TWeakObjectPtr<UPGXAsyncAction_ExportLogs> WeakThis(this);

	UE::Tasks::Launch(TEXT("PGXAsyncExportLogs"),
		[WeakThis, Entries = MoveTemp(Entries), Session, ExportPath]()
		{
			const FString JsonString = FPGXLogSerializer::ToJson(Entries, Session);

			const FString Dir = FPaths::GetPath(ExportPath);
			IFileManager::Get().MakeDirectory(*Dir, true);
			const bool bSuccess = FFileHelper::SaveStringToFile(
				JsonString, *ExportPath, FFileHelper::EEncodingOptions::ForceUTF8);

			// EN: Callback on game thread
			// ES: Callback en el hilo principal
			AsyncTask(ENamedThreads::GameThread, [WeakThis, bSuccess, ExportPath]()
			{
				if (UPGXAsyncAction_ExportLogs* Self = WeakThis.Get())
				{
					if (bSuccess)
					{
						Self->OnSuccess.Broadcast(ExportPath);
					}
					else
					{
						Self->OnFailure.Broadcast(FString::Printf(TEXT("Failed to write: %s"), *ExportPath));
					}
					Self->SetReadyToDestroy();
				}
			});
		});
}
