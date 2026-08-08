// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Logging/Renderers/PGXLogLoadingRenderer.h"

#define LOCTEXT_NAMESPACE "PGXLogDomain"

FLinearColor UPGXLogLoadingRenderer::GetDomainColor_Implementation() const
{
	return FLinearColor(0.914f, 0.118f, 0.388f);
}

FText UPGXLogLoadingRenderer::GetDomainDisplayName_Implementation() const
{
	return LOCTEXT("Loading", "Loading");
}

TArray<FPGXLogColumnDef> UPGXLogLoadingRenderer::GetColumnDefinitions_Implementation() const
{
	return {
		{ LOCTEXT("PhaseH", "Phase"), 80.0f, TEXT("Phase") },
		{ LOCTEXT("ProgressH", "Progress"), 60.0f, TEXT("Progress") },
		{ LOCTEXT("ContextH", "Context"), 80.0f, TEXT("Context") },
		{ LOCTEXT("DurationH", "Duration"), 70.0f, TEXT("Duration") },
	};
}

#undef LOCTEXT_NAMESPACE
