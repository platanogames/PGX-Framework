// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXStreamingManager.h"
#include "Logging/PGXLogMacros.h"
#include "PGXLoadingRuntime.h"

// EN: UPGXStreamingManager exposes an explicit unsupported probe so callers receive a
//     deterministic typed result and diagnostic instead of a silent no-op.
// ES: UPGXStreamingManager expone un probe unsupported explicito para que los callers
//     reciban un resultado tipado y un diagnostico en lugar de un no-op silencioso.

FPGXLoadingResult UPGXStreamingManager::ProbeUnsupported() const
{
	const FString Reason = GetUnsupportedReason();

	PGX_LOG_WARNING(LogPGXLoading, TEXT("[PGXStreamingManager] %s"), *Reason);

	return FPGXLoadingResult::MakeFail(EPGXLoadingResultCode::Unsupported, Reason);
}
