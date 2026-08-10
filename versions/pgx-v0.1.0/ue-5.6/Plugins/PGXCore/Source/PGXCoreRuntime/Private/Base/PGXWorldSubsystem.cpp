// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Base/PGXWorldSubsystem.h"
#include "UObject/UObjectIterator.h"

// EN: Base class for all PGX World subsystems implementation
// ES: Implementacion de la clase base para todos los subsistemas PGX de World

void UPGXWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UPGXWorldSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UPGXWorldSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// EN: Class Override Pattern — if a developer creates a non-abstract subclass of any PGX
	//     subsystem, the PGX base class automatically yields. This prevents duplicate instances.
	// ES: Patron Class Override — si un desarrollador crea una subclase no-abstracta de cualquier
	//     subsistema PGX, la clase base PGX cede automaticamente. Previene instancias duplicadas.
	TArray<UClass*> DerivedClasses;
	GetDerivedClasses(GetClass(), DerivedClasses, true);
	for (const UClass* Subclass : DerivedClasses)
	{
		if (!Subclass->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists))
		{
			return false;
		}
	}
	return Super::ShouldCreateSubsystem(Outer);
}
