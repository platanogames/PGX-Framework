// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXTradeRuntime.h"
#include "Logging/PGXLogMacros.h"

DEFINE_LOG_CATEGORY(LogPGXTrade);

#define LOCTEXT_NAMESPACE "FPGXTradeRuntimeModule"

void FPGXTradeRuntimeModule::StartupModule()
{
	PGX_LOG_INFO(LogPGXTrade, TEXT("PGXTradeRuntime: Module started."));
}

void FPGXTradeRuntimeModule::ShutdownModule()
{
	PGX_LOG_INFO(LogPGXTrade, TEXT("PGXTradeRuntime: Module shut down."));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXTradeRuntimeModule, PGXTradeRuntime)
