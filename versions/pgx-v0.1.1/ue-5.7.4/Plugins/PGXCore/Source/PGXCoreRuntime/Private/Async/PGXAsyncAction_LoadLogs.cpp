// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Async/PGXAsyncAction_LoadLogs.h"
#include "Subsystems/PGXLogSubsystem.h"

// EN: Async Blueprint action for loading PGX logs from .sav slot
// ES: Accion async de Blueprint para cargar logs PGX desde slot .sav

UPGXAsyncAction_LoadLogs* UPGXAsyncAction_LoadLogs::LoadLogsFromSlot(
	UObject* WorldContextObject,
	int32 SessionId)
{
	UPGXAsyncAction_LoadLogs* Action = NewObject<UPGXAsyncAction_LoadLogs>();
	Action->RequestedSessionId = SessionId;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UPGXAsyncAction_LoadLogs::Activate()
{
	UPGXLogSubsystem* LogSub = UPGXLogSubsystem::GetCached();
	if (!LogSub)
	{
		OnFailure.Broadcast(TEXT("PGX Log Subsystem not available"));
		SetReadyToDestroy();
		return;
	}

	// EN: .sav loading is unavailable while slot persistence remains unsupported.
	// ES: La carga .sav no esta disponible mientras no haya persistencia por slots.
	const bool bSuccess = LogSub->LoadFromSlot(RequestedSessionId);
	if (bSuccess)
	{
		OnLogsLoaded.Broadcast(LogSub->GetCurrentSessionEntries());
	}
	else
	{
		OnFailure.Broadcast(TEXT(".sav slot loading not yet implemented"));
	}

	SetReadyToDestroy();
}
