// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Logging/Renderers/PGXLogDataRegistryRenderer.h"

#define LOCTEXT_NAMESPACE "PGXLogDomain"

FLinearColor UPGXLogDataRegistryRenderer::GetDomainColor_Implementation() const
{
	return FLinearColor(0.0f, 0.8f, 1.0f);
}

FText UPGXLogDataRegistryRenderer::GetDomainDisplayName_Implementation() const
{
	return LOCTEXT("DataRegistry", "DataRegistry");
}

TArray<FPGXLogColumnDef> UPGXLogDataRegistryRenderer::GetColumnDefinitions_Implementation() const
{
	return {
		{ LOCTEXT("DatabaseH", "Database"), 100.0f, TEXT("Database") },
		{ LOCTEXT("ItemTagH", "ItemTag"), 90.0f, TEXT("ItemTag") },
		{ LOCTEXT("ActionH", "Action"), 70.0f, TEXT("Action") },
		{ LOCTEXT("VersionH", "Version"), 60.0f, TEXT("Version") },
	};
}

#undef LOCTEXT_NAMESPACE
