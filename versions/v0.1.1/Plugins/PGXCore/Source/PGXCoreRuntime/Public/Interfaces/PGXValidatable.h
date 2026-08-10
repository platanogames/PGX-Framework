// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PGXValidatable.generated.h"

/**
 * EN: Contract for objects/assets that support validation
 *     in editor and runtime.
 * ES: Contrato para objetos/assets que soportan validacion
 *     en editor y runtime.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UPGXValidatable : public UInterface
{
	GENERATED_BODY()
};

class PGXCORERUNTIME_API IPGXValidatable
{
	GENERATED_BODY()

public:
	/**
	 * EN: Validate the object and return errors if any.
	 *     Returns true if valid, false if errors were found.
	 * ES: Validar el objeto y retornar errores si los hay.
	 *     Retorna true si es valido, false si se encontraron errores.
	 */
	virtual bool Validate(TArray<FText>& OutErrors) const = 0;
};
