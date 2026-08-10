// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Logging/Renderers/PGXLogPSORenderer.h"

#define LOCTEXT_NAMESPACE "PGXLogDomain"

FLinearColor UPGXLogPSORenderer::GetDomainColor_Implementation() const
{
	return FLinearColor(0.0f, 0.737f, 0.831f);
}

FText UPGXLogPSORenderer::GetDomainDisplayName_Implementation() const
{
	return LOCTEXT("PSO", "PSO");
}

TArray<FPGXLogColumnDef> UPGXLogPSORenderer::GetColumnDefinitions_Implementation() const
{
	return {
		{ LOCTEXT("PipelineIdH", "PipelineId"), 100.0f, TEXT("PipelineId") },
		{ LOCTEXT("StatusH", "Status"), 70.0f, TEXT("Status") },
		{ LOCTEXT("DurationH", "Duration"), 70.0f, TEXT("Duration") },
		{ LOCTEXT("ContextH", "Context"), 80.0f, TEXT("Context") },
	};
}

#undef LOCTEXT_NAMESPACE
