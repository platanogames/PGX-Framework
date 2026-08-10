// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Logging/Renderers/PGXLogProfileRenderer.h"

#define LOCTEXT_NAMESPACE "PGXLogDomain"

FLinearColor UPGXLogProfileRenderer::GetDomainColor_Implementation() const
{
	return FLinearColor(1.0f, 0.757f, 0.027f);
}

FText UPGXLogProfileRenderer::GetDomainDisplayName_Implementation() const
{
	return LOCTEXT("Profile", "Profile");
}

TArray<FPGXLogColumnDef> UPGXLogProfileRenderer::GetColumnDefinitions_Implementation() const
{
	return {
		{ LOCTEXT("PlatformH", "Platform"), 80.0f, TEXT("Platform") },
		{ LOCTEXT("CapabilityH", "Capability"), 90.0f, TEXT("Capability") },
		{ LOCTEXT("BudgetH", "Budget"), 80.0f, TEXT("Budget") },
		{ LOCTEXT("ValueH", "Value"), 70.0f, TEXT("Value") },
	};
}

#undef LOCTEXT_NAMESPACE
