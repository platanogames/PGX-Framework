// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Construction/PGXGameModeConstruction.h"

UPGXGameModeConstruction::UPGXGameModeConstruction()
{
	// EN: GameMode listens to level and loading by default (natural coordinator)
	// ES: GameMode escucha level y loading por defecto (coordinador natural)
	bListenToLevelTransitions = true;
	bListenToLoadingScreenState = true;
}

bool UPGXGameModeConstruction::Validate(TArray<FText>& OutErrors) const
{
	bool bValid = Super::Validate(OutErrors);

	// EN: GameMode construction should have at least a pawn class
	// ES: La construccion de GameMode deberia tener al menos una clase de pawn
	if (DefaultPawnClass.IsNull())
	{
		OutErrors.Add(FText::FromString(TEXT("GameMode Construction: DefaultPawnClass is not set")));
		// EN: Not a hard error — still valid but warn
		// ES: No es un error grave — sigue valido pero se advierte
	}

	return bValid;
}
