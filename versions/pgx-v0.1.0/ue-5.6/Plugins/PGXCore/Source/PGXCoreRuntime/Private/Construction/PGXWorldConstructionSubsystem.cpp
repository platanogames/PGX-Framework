// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Construction/PGXWorldConstructionSubsystem.h"
#include "Construction/PGXConstructionResolver.h"

void UPGXWorldConstructionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogPGXConstruction, Verbose, TEXT("WorldConstructionSubsystem initialized for world [%s]"),
		*GetWorld()->GetName());
}

void UPGXWorldConstructionSubsystem::Deinitialize()
{
	UE_LOG(LogPGXConstruction, Verbose, TEXT("WorldConstructionSubsystem deinitializing for world [%s]"),
		*GetWorld()->GetName());

	Super::Deinitialize();
}
