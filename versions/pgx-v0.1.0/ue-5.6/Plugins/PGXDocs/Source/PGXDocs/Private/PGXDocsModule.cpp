// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXDocsModule.h"
#include "Logging/PGXLogMacros.h"

DEFINE_LOG_CATEGORY(LogPGXDocs);

#define LOCTEXT_NAMESPACE "FPGXDocsModule"

void FPGXDocsModule::StartupModule()
{
	PGX_LOG_INFO(LogPGXDocs, TEXT("PGXDocs: Shared module loaded"));
}

void FPGXDocsModule::ShutdownModule()
{
	PGX_LOG_INFO(LogPGXDocs, TEXT("PGXDocs: Shared module unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXDocsModule, PGXDocs)
