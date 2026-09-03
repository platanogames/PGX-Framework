// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/PGXSaveable.h"
#include "PGXSaveGame.h"

#include "PGXHarnessSaveableStub.generated.h"

/**
 * EN: Private compatibility fixture implementing IPGXSaveable for lifecycle verification.
 *     Writes a marker key on OnPreSave and reads it back on OnPostLoad.
 *
 * ES: Fixture privado de compatibilidad que implementa IPGXSaveable para verificar su ciclo de vida.
 *     Escribe una clave marcador en OnPreSave y la lee de vuelta en OnPostLoad.
 */
UCLASS(Transient)
class UPGXHarnessSaveableStub : public UObject, public IPGXSaveable
{
	GENERATED_BODY()

public:
	FGameplayTag DomainTag;
	FString TestData = TEXT("HarnessFixtureData");

	void OnPreSave(UPGXSaveGame* SaveGame, FGameplayTag /*InDomainTag*/) override
	{
		if (SaveGame)
		{
			SaveGame->WriteString(FName("HarnessFixtureMarker"), TestData);
		}
	}

	void OnPostLoad(UPGXSaveGame* SaveGame, FGameplayTag /*InDomainTag*/) override
	{
		if (SaveGame)
		{
			TestData = SaveGame->ReadString(FName("HarnessFixtureMarker"));
		}
	}

	FGameplayTag GetSaveDomainTag() const override
	{
		return DomainTag;
	}
};
