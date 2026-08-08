// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Logging/Renderers/PGXLogSaveRenderer.h"

#define LOCTEXT_NAMESPACE "PGXLogDomain"

FLinearColor UPGXLogSaveRenderer::GetDomainColor_Implementation() const
{
	return FLinearColor(0.298f, 0.686f, 0.314f);
}

FText UPGXLogSaveRenderer::GetDomainDisplayName_Implementation() const
{
	return LOCTEXT("Save", "Save");
}

TArray<FPGXLogColumnDef> UPGXLogSaveRenderer::GetColumnDefinitions_Implementation() const
{
	return {
		{ LOCTEXT("SlotNameH", "SlotName"), 90.0f, TEXT("SlotName") },
		{ LOCTEXT("DomainH", "Domain"), 80.0f, TEXT("Domain") },
		{ LOCTEXT("ByteSizeH", "ByteSize"), 70.0f, TEXT("ByteSize") },
		{ LOCTEXT("OperationH", "Operation"), 80.0f, TEXT("Operation") },
	};
}

#undef LOCTEXT_NAMESPACE
