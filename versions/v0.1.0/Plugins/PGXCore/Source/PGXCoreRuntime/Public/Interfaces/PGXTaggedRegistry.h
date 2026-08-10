// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "PGXTaggedRegistry.generated.h"

/**
 * EN: Interface for registries whose entries are addressable by GameplayTag.
 *     Exists to collapse duplicated HasX(Tag) / GetCount / snapshot patterns
 *     across tagged PGX systems without forcing a concrete storage strategy.
 *
 * ES: Interfaz para registries cuyas entries se direccionan por GameplayTag.
 *     Existe para colapsar patrones duplicados HasX(Tag) / GetCount / snapshot
 *     en sistemas PGX tagged sin imponer una estrategia concreta de storage.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UPGXTaggedRegistry : public UInterface
{
	GENERATED_BODY()
};

class PGXCORERUNTIME_API IPGXTaggedRegistry
{
	GENERATED_BODY()

public:
	/**
	 * EN: True when at least one live entry exists for the tag.
	 * ES: True si existe al menos una entry viva para el tag.
	 */
	virtual bool HasEntryByTag(FGameplayTag Tag) const = 0;

	/**
	 * EN: Number of unique live entries in the registry.
	 * ES: Numero de entries vivas unicas en el registry.
	 */
	virtual int32 GetCount() const = 0;

	/**
	 * EN: Snapshot of tags that currently have at least one live entry.
	 *     OutTags is reset before population.
	 * ES: Snapshot de tags que actualmente tienen al menos una entry viva.
	 *     OutTags se resetea antes de poblarse.
	 */
	virtual void GetSnapshot(TArray<FGameplayTag>& OutTags) const = 0;
};
