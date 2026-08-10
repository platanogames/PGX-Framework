// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Construction/PGXClassConstruction.h"

bool UPGXClassConstruction::Validate(TArray<FText>& OutErrors) const
{
	bool bValid = true;

	// EN: Validate component injections / ES: Validar inyecciones de componentes
	for (int32 i = 0; i < Components.Num(); ++i)
	{
		const FPGXComponentInjection& Comp = Components[i];
		if (Comp.ComponentClass.IsNull())
		{
			OutErrors.Add(FText::FromString(FString::Printf(
				TEXT("Component injection [%d] has null ComponentClass"), i)));
			bValid = false;
		}
		if (Comp.bClientOnly && Comp.bServerOnly)
		{
			OutErrors.Add(FText::FromString(FString::Printf(
				TEXT("Component injection [%d] is marked both ClientOnly and ServerOnly"), i)));
			bValid = false;
		}
	}

	return bValid;
}

FText UPGXClassConstruction::GetConstructionDisplayName() const
{
	if (!ConfigDisplayName.IsEmpty())
	{
		return ConfigDisplayName;
	}
	return FText::FromString(GetName());
}
