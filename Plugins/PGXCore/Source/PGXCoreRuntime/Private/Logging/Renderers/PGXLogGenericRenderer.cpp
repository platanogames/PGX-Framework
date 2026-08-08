// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Logging/Renderers/PGXLogGenericRenderer.h"

#define LOCTEXT_NAMESPACE "PGXLogDomain"

FLinearColor UPGXLogGenericRenderer::GetDomainColor_Implementation() const
{
	return FLinearColor(0.5f, 0.5f, 0.5f);
}

FText UPGXLogGenericRenderer::GetDomainDisplayName_Implementation() const
{
	return LOCTEXT("Generic", "Generic");
}

TArray<FPGXLogColumnDef> UPGXLogGenericRenderer::GetColumnDefinitions_Implementation() const
{
	return {};
}

#undef LOCTEXT_NAMESPACE
