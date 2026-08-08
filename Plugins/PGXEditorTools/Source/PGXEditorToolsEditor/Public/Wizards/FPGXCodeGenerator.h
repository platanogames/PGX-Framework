// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"

/**
 * EN: Code generator for PGX Data-Driven types.
 *     Generates C++ source files from templates for new Object DA categories,
 *     Config DAs, structs, interfaces, and factories.
 * ES: Generador de codigo para tipos Data-Driven de PGX.
 *     Genera archivos fuente C++ desde templates para nuevas categorias de Object DA,
 *     Config DAs, structs, interfaces y factories.
 */
class PGXEDITORTOOLSEDITOR_API FPGXCodeGenerator
{
public:
	// EN: Generate files for a new Object DA category / ES: Generar archivos para nueva categoria de Object DA
	static bool GenerateObjectDACategory(const FString& CategoryName, const FString& OutputPath);

	// EN: Generate files for a new Config DA / ES: Generar archivos para nuevo Config DA
	static bool GenerateConfigDA(const FString& ConfigName, const FString& OutputPath);
};
