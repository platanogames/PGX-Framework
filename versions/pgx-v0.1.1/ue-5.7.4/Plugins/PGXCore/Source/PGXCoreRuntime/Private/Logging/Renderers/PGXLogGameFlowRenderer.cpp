// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Logging/Renderers/PGXLogGameFlowRenderer.h"

#define LOCTEXT_NAMESPACE "PGXLogDomain"

FLinearColor UPGXLogGameFlowRenderer::GetDomainColor_Implementation() const
{
	return FLinearColor(1.0f, 0.596f, 0.0f);
}

FText UPGXLogGameFlowRenderer::GetDomainDisplayName_Implementation() const
{
	return LOCTEXT("GameFlow", "GameFlow");
}

TArray<FPGXLogColumnDef> UPGXLogGameFlowRenderer::GetColumnDefinitions_Implementation() const
{
	return {
		{ LOCTEXT("FromStateH", "FromState"), 90.0f, TEXT("FromState") },
		{ LOCTEXT("ToStateH", "ToState"), 90.0f, TEXT("ToState") },
		{ LOCTEXT("TriggerH", "Trigger"), 80.0f, TEXT("Trigger") },
		{ LOCTEXT("PhaseH", "Phase"), 70.0f, TEXT("Phase") },
	};
}

#undef LOCTEXT_NAMESPACE
