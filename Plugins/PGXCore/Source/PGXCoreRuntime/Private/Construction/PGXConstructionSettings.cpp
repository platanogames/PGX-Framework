// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Construction/PGXConstructionSettings.h"
// EN: Full headers for all TSubclassOf<T> types — forward decl not enough for operator== (calls T::StaticClass())
// ES: Headers completos para todos los TSubclassOf<T> — forward decl no alcanza para operator== (llama T::StaticClass())
#include "Base/PGXGameModeBase.h"
#include "Base/PGXPlayerControllerBase.h"
#include "Base/PGXGameStateBase.h"
#include "Base/PGXPlayerStateBase.h"
#include "Base/PGXCharacterBase.h"
#include "Base/PGXPawnBase.h"
#include "GameFramework/HUD.h"
#include "GameMapsSettings.h"

#define LOCTEXT_NAMESPACE "PGXConstructionSettings"

FText UPGXConstructionSettings::GetSectionText() const
{
	return LOCTEXT("SectionText", "Framework Construction");
}

FText UPGXConstructionSettings::GetSectionDescription() const
{
	return LOCTEXT("SectionDesc",
		"Configure construction settings for PGX base classes. "
		"Each slot defines the class source (Default/C++/Blueprint) and a construction DA "
		"for component injections, system configs, and HUD layers applied during gameplay initialization.");
}

bool UPGXConstructionSettings::ValidateClassSources(TArray<FText>& OutWarnings, bool bIncludeMapsModesCheck) const
{
	// EN: Validate class source vs class assignment for each slot
	// ES: Validar class source vs asignacion de clase para cada slot
	struct FSlotCheck
	{
		const TCHAR* Name;
		EPGXClassSourceMode Source;
		bool bClassCPPNull;
		bool bClassBPNull;
		bool bDANull;
	};

	const FSlotCheck Slots[] =
	{
		{ TEXT("GameMode"),         GameModeClassSource,         GameModeClassCPP == nullptr,         GameModeClassBP.IsNull(),         GameModeConstruction.IsNull() },
		{ TEXT("PlayerController"), PlayerControllerClassSource, PlayerControllerClassCPP == nullptr, PlayerControllerClassBP.IsNull(), PlayerControllerConstruction.IsNull() },
		{ TEXT("GameState"),        GameStateClassSource,        GameStateClassCPP == nullptr,        GameStateClassBP.IsNull(),        GameStateConstruction.IsNull() },
		{ TEXT("PlayerState"),      PlayerStateClassSource,      PlayerStateClassCPP == nullptr,      PlayerStateClassBP.IsNull(),      PlayerStateConstruction.IsNull() },
		{ TEXT("Character"),        CharacterClassSource,        CharacterClassCPP == nullptr,        CharacterClassBP.IsNull(),        CharacterConstruction.IsNull() },
		{ TEXT("Pawn"),             PawnClassSource,             PawnClassCPP == nullptr,             PawnClassBP.IsNull(),             PawnConstruction.IsNull() },
		{ TEXT("HUD"),              HUDClassSource,              HUDClassCPP == nullptr,              HUDClassBP.IsNull(),              HUDConstruction.IsNull() },
	};

	for (const FSlotCheck& Slot : Slots)
	{
		if (Slot.Source == EPGXClassSourceMode::CppClass && Slot.bClassCPPNull)
		{
			OutWarnings.Add(FText::Format(
				LOCTEXT("CPPClassMissing", "{0}: Class Source is set to C++ but no C++ class is assigned"),
				FText::FromString(Slot.Name)
			));
		}
		else if (Slot.Source == EPGXClassSourceMode::Blueprint && Slot.bClassBPNull)
		{
			OutWarnings.Add(FText::Format(
				LOCTEXT("BPClassMissing", "{0}: Class Source is set to Blueprint but no Blueprint class is assigned"),
				FText::FromString(Slot.Name)
			));
		}
	}

	// EN: Cross-check GameMode in Maps & Modes against PGX base class
	// ES: Verificacion cruzada del GameMode en Maps & Modes contra la clase base PGX
	if (bIncludeMapsModesCheck)
	{
		const UGameMapsSettings* GameMapsSettings = GetDefault<UGameMapsSettings>();
		if (GameMapsSettings)
		{
			// EN: Use public getter — GlobalDefaultGameMode is private in UE 5.6
			// ES: Usar getter publico — GlobalDefaultGameMode es private en UE 5.6
			const FSoftClassPath& GlobalGM = GameMapsSettings->GetGlobalDefaultGameMode();
			if (GlobalGM.IsValid())
			{
				UClass* GMClass = GlobalGM.TryLoadClass<AGameModeBase>();
				if (GMClass && !GMClass->IsChildOf<APGXGameModeBase>())
				{
					OutWarnings.Add(FText::Format(
						LOCTEXT("MapsModesGMWarning",
							"Default GameMode in Maps & Modes ({0}) does not derive from APGXGameModeBase. "
							"PGX construction will not run."),
						FText::FromString(GMClass->GetName())
					));
				}
			}
		}
	}

	return OutWarnings.Num() == 0;
}

#undef LOCTEXT_NAMESPACE
