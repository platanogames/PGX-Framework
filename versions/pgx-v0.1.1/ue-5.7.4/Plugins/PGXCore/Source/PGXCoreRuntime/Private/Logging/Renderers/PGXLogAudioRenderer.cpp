// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Logging/Renderers/PGXLogAudioRenderer.h"

#define LOCTEXT_NAMESPACE "PGXLogDomain"

FLinearColor UPGXLogAudioRenderer::GetDomainColor_Implementation() const
{
	return FLinearColor(1.0f, 0.596f, 0.0f);
}

FText UPGXLogAudioRenderer::GetDomainDisplayName_Implementation() const
{
	return LOCTEXT("Audio", "Audio");
}

TArray<FPGXLogColumnDef> UPGXLogAudioRenderer::GetColumnDefinitions_Implementation() const
{
	return {
		{ LOCTEXT("SoundEventH", "SoundEvent"), 100.0f, TEXT("SoundEvent") },
		{ LOCTEXT("ChannelH", "Channel"), 80.0f, TEXT("Channel") },
		{ LOCTEXT("VolumeH", "Volume"), 60.0f, TEXT("Volume") },
		{ LOCTEXT("BackendH", "Backend"), 70.0f, TEXT("Backend") },
	};
}

#undef LOCTEXT_NAMESPACE
