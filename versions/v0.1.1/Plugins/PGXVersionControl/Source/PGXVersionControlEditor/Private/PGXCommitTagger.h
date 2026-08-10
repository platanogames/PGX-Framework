// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"

/**
 * EN: Detects PGX systems from file paths and generates commit prefixes/templates.
 *     Maps 23+ plugin directory names to human-readable system tags.
 *
 * ES: Detecta sistemas PGX de rutas de archivos y genera prefijos/templates de commit.
 *     Mapea 23+ nombres de directorio de plugins a tags de sistema legibles.
 */
class FPGXCommitTagger
{
public:
	FPGXCommitTagger();

	/** EN: Detect PGX systems touched by the given file paths / ES: Detectar sistemas PGX tocados por las rutas */
	TArray<FString> DetectSystems(const TArray<FString>& InFilePaths) const;

	/** EN: Build commit prefix from tags (e.g. "[Audio][Save]") / ES: Construir prefijo de commit de tags */
	FString BuildCommitPrefix(const TArray<FString>& InTags) const;

	/** EN: Generate full commit template using settings template format / ES: Generar template completo de commit */
	FString GetCommitTemplate(const FString& InChangelistName, const TArray<FString>& InTags) const;

private:
	/** EN: Directory fragment -> system display name / ES: Fragmento de directorio -> nombre de sistema */
	TMap<FString, FString> SystemMap;

	void InitializeSystemMap();
};
