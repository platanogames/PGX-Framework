// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Async/PGXAsyncAction_QueryLogs.h"
#include "Subsystems/PGXLogSubsystem.h"
#include "Tasks/Task.h"

// EN: Async Blueprint action for querying PGX logs with filters
// ES: Accion async de Blueprint para consultar logs PGX con filtros

UPGXAsyncAction_QueryLogs* UPGXAsyncAction_QueryLogs::QueryLogs(
	UObject* WorldContextObject,
	EPGXLogVerbosity MinVerbosity,
	FName CategoryFilter,
	int32 MaxResults)
{
	UPGXAsyncAction_QueryLogs* Action = NewObject<UPGXAsyncAction_QueryLogs>();
	Action->QueryMinVerbosity = MinVerbosity;
	Action->QueryCategoryFilter = CategoryFilter;
	Action->QueryMaxResults = FMath::Max(MaxResults, 1);
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UPGXAsyncAction_QueryLogs::Activate()
{
	UPGXLogSubsystem* LogSub = UPGXLogSubsystem::GetCached();
	if (!LogSub)
	{
		OnResults.Broadcast(TArray<FPGXLogEntry>());
		SetReadyToDestroy();
		return;
	}

	// EN: Snapshot (short lock on ring buffer) then filter on background thread
	// ES: Snapshot (lock corto en ring buffer) luego filtrar en hilo background
	TArray<FPGXLogEntry> AllEntries = LogSub->GetEntriesFiltered(QueryMinVerbosity, QueryCategoryFilter);

	TWeakObjectPtr<UPGXAsyncAction_QueryLogs> WeakThis(this);
	const int32 MaxRes = QueryMaxResults;

	UE::Tasks::Launch(TEXT("PGXAsyncQueryLogs"),
		[WeakThis, Entries = MoveTemp(AllEntries), MaxRes]()
		{
			// EN: Trim to max results (keep most recent = tail)
			// ES: Recortar a max results (mantener mas recientes = cola)
			TArray<FPGXLogEntry> Result;
			if (Entries.Num() > MaxRes)
			{
				const int32 StartIndex = Entries.Num() - MaxRes;
				Result.Reserve(MaxRes);
				for (int32 i = StartIndex; i < Entries.Num(); ++i)
				{
					Result.Add(Entries[i]);
				}
			}
			else
			{
				Result = Entries;
			}

			// EN: Callback on game thread
			// ES: Callback en el hilo principal
			AsyncTask(ENamedThreads::GameThread, [WeakThis, Result = MoveTemp(Result)]()
			{
				if (UPGXAsyncAction_QueryLogs* Self = WeakThis.Get())
				{
					Self->OnResults.Broadcast(Result);
					Self->SetReadyToDestroy();
				}
			});
		});
}
