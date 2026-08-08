// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Messages/PGXMessageConfig.h"

UPGXMessageConfig::UPGXMessageConfig()
{
	ConfigDisplayName = FText::FromString(TEXT("Message System Config"));
	ConfigDescription = FText::FromString(TEXT("Configuration for the PGX Message System (pub/sub bus)."));
}
