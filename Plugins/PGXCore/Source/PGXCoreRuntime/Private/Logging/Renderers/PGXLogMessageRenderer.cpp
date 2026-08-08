// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Logging/Renderers/PGXLogMessageRenderer.h"

#define LOCTEXT_NAMESPACE "PGXLogDomain"

FLinearColor UPGXLogMessageRenderer::GetDomainColor_Implementation() const
{
	return FLinearColor(0.706f, 0.314f, 0.863f);
}

FText UPGXLogMessageRenderer::GetDomainDisplayName_Implementation() const
{
	return LOCTEXT("Message", "Message");
}

TArray<FPGXLogColumnDef> UPGXLogMessageRenderer::GetColumnDefinitions_Implementation() const
{
	return {
		{ LOCTEXT("ChannelH", "Channel"), 90.0f, TEXT("Channel") },
		{ LOCTEXT("PayloadTypeH", "PayloadType"), 80.0f, TEXT("PayloadType") },
		{ LOCTEXT("SenderH", "Sender"), 80.0f, TEXT("Sender") },
		{ LOCTEXT("SizeH", "Size"), 60.0f, TEXT("Size") },
	};
}

#undef LOCTEXT_NAMESPACE
