// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Logging/Renderers/PGXLogLevelFlowRenderer.h"

#define LOCTEXT_NAMESPACE "PGXLogDomain"

FLinearColor UPGXLogLevelFlowRenderer::GetDomainColor_Implementation() const
{
	return FLinearColor(0.129f, 0.588f, 0.953f);
}

FText UPGXLogLevelFlowRenderer::GetDomainDisplayName_Implementation() const
{
	return LOCTEXT("LevelFlow", "LevelFlow");
}

TArray<FPGXLogColumnDef> UPGXLogLevelFlowRenderer::GetColumnDefinitions_Implementation() const
{
	return {
		{ LOCTEXT("LevelNameH", "LevelName"), 100.0f, TEXT("LevelName") },
		{ LOCTEXT("ActionH", "Action"), 70.0f, TEXT("Action") },
		{ LOCTEXT("SubLevelH", "SubLevel"), 90.0f, TEXT("SubLevel") },
		{ LOCTEXT("ReasonH", "Reason"), 80.0f, TEXT("Reason") },
	};
}

#undef LOCTEXT_NAMESPACE
