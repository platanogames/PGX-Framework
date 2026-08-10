// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Logging/Renderers/PGXLogEventHandlerRenderer.h"

#define LOCTEXT_NAMESPACE "PGXLogDomain"

FLinearColor UPGXLogEventHandlerRenderer::GetDomainColor_Implementation() const
{
	return FLinearColor(0.863f, 0.235f, 0.235f);
}

FText UPGXLogEventHandlerRenderer::GetDomainDisplayName_Implementation() const
{
	return LOCTEXT("EventHandler", "EventHandler");
}

TArray<FPGXLogColumnDef> UPGXLogEventHandlerRenderer::GetColumnDefinitions_Implementation() const
{
	return {
		{ LOCTEXT("EventTagH", "EventTag"), 100.0f, TEXT("EventTag") },
		{ LOCTEXT("ResolutionH", "Resolution"), 80.0f, TEXT("Resolution") },
		{ LOCTEXT("HandlerH", "Handler"), 90.0f, TEXT("Handler") },
		{ LOCTEXT("TimeH", "Time"), 60.0f, TEXT("Time") },
	};
}

#undef LOCTEXT_NAMESPACE
