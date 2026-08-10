// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXAsyncLoader.h"
#include "Logging/PGXLogMacros.h"
#include "PGXLoadingRuntime.h"

// EN: UPGXAsyncLoader exposes an explicit unsupported probe so callers receive a
//     deterministic typed result and diagnostic instead of a silent no-op.
// ES: UPGXAsyncLoader expone un probe unsupported explicito para que los callers
//     reciban un resultado tipado y un diagnostico en lugar de un no-op silencioso.

FPGXLoadingResult UPGXAsyncLoader::ProbeUnsupported() const
{
	const FString Reason = GetUnsupportedReason();

	PGX_LOG_WARNING(LogPGXLoading, TEXT("[PGXAsyncLoader] %s"), *Reason);

	return FPGXLoadingResult::MakeFail(EPGXLoadingResultCode::Unsupported, Reason);
}
