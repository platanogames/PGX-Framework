// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "EventHandler/Handlers/PGXFlushConfigHandler.h"
#include "EventHandler/PGXEventHandlerLog.h"
#include "Misc/ConfigCacheIni.h"

EPGXEventResult UPGXFlushConfigHandler::Execute_Implementation(const FPGXEventContext& /*Context*/, const FInstancedStruct& /*Payload*/)
{
	if (GConfig)
	{
		GConfig->Flush(false);
		UE_LOG(LogPGXEventHandler, Log, TEXT("FlushConfigHandler: Config flushed successfully"));
		return EPGXEventResult::Success;
	}

	UE_LOG(LogPGXEventHandler, Warning, TEXT("FlushConfigHandler: GConfig not available"));
	return EPGXEventResult::Failed;
}
