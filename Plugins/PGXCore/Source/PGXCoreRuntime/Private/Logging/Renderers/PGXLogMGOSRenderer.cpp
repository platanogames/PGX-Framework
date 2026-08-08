// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Logging/Renderers/PGXLogMGOSRenderer.h"

#define LOCTEXT_NAMESPACE "PGXLogDomain"

FLinearColor UPGXLogMGOSRenderer::GetDomainColor_Implementation() const
{
	return FLinearColor(0.498f, 0.0f, 1.0f);
}

FText UPGXLogMGOSRenderer::GetDomainDisplayName_Implementation() const
{
	return LOCTEXT("MGOS", "MGOS");
}

TArray<FPGXLogColumnDef> UPGXLogMGOSRenderer::GetColumnDefinitions_Implementation() const
{
	return {
		{ LOCTEXT("PoolNameH", "PoolName"), 100.0f, TEXT("PoolName") },
		{ LOCTEXT("AllocCountH", "AllocCount"), 80.0f, TEXT("AllocCount") },
		{ LOCTEXT("MemUsedH", "MemUsed"), 70.0f, TEXT("MemUsed") },
		{ LOCTEXT("DeltaH", "Delta"), 70.0f, TEXT("Delta") },
	};
}

#undef LOCTEXT_NAMESPACE
