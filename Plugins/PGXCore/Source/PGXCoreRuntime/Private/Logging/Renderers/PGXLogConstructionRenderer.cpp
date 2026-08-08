// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Logging/Renderers/PGXLogConstructionRenderer.h"

#define LOCTEXT_NAMESPACE "PGXLogDomain"

FLinearColor UPGXLogConstructionRenderer::GetDomainColor_Implementation() const
{
	return FLinearColor(0.0f, 0.502f, 0.502f);
}

FText UPGXLogConstructionRenderer::GetDomainDisplayName_Implementation() const
{
	return LOCTEXT("Construction", "Construction");
}

TArray<FPGXLogColumnDef> UPGXLogConstructionRenderer::GetColumnDefinitions_Implementation() const
{
	return {
		{ LOCTEXT("SlotTypeH", "SlotType"), 90.0f, TEXT("SlotType") },
		{ LOCTEXT("ClassNameH", "ClassName"), 100.0f, TEXT("ClassName") },
		{ LOCTEXT("SourceModeH", "SourceMode"), 80.0f, TEXT("SourceMode") },
	};
}

#undef LOCTEXT_NAMESPACE
