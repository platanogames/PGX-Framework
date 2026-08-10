// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXLoadingRuntime.h"
#include "Logging/PGXLogMacros.h"

#define LOCTEXT_NAMESPACE "FPGXLoadingRuntimeModule"

DEFINE_LOG_CATEGORY(LogPGXLoading);

void FPGXLoadingRuntimeModule::StartupModule()
{
	PGX_LOG_INFO(LogPGXLoading, TEXT("PGXLoadingRuntime: Module started."));
}

void FPGXLoadingRuntimeModule::ShutdownModule()
{
	PGX_LOG_INFO(LogPGXLoading, TEXT("PGXLoadingRuntime: Module shut down."));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXLoadingRuntimeModule, PGXLoadingRuntime)
