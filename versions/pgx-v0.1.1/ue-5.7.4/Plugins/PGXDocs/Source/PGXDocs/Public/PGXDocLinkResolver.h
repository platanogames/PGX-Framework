// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"

class FDocRegistry;

/**
 * EN: Resolves [[Type:Value]] crossref links in documentation.
 *     Types: Doc (navigate), Class (open C++ source), BP (open Blueprint asset).
 *     When no type prefix, tries Doc first, then Class.
 *
 * ES: Resuelve links crossref [[Tipo:Valor]] en documentacion.
 *     Tipos: Doc (navegar), Class (abrir fuente C++), BP (abrir asset Blueprint).
 *     Sin prefijo de tipo, intenta Doc primero, luego Class.
 */
class PGXDOCS_API FDocLinkResolver
{
public:
	// EN: Result of resolving a link / ES: Resultado de resolver un link
	enum class ELinkType : uint8
	{
		Doc,        // EN: Navigate to another doc / ES: Navegar a otro doc
		Class,      // EN: Open C++ class in IDE / ES: Abrir clase C++ en IDE
		Blueprint,  // EN: Open Blueprint asset / ES: Abrir asset Blueprint
		External,   // EN: External URL / ES: URL externa
		Unknown     // EN: Could not resolve / ES: No se pudo resolver
	};

	struct FResolvedLink
	{
		ELinkType Type = ELinkType::Unknown;
		FString Value;       // EN: DocId, ClassName, AssetPath, or URL / ES: DocId, NombreClase, AssetPath, o URL
		FString DisplayText; // EN: Text to show for the link / ES: Texto a mostrar para el link
	};

	/**
	 * EN: Parse and resolve a [[...]] crossref string.
	 * ES: Parsear y resolver un string crossref [[...]].
	 */
	FResolvedLink Resolve(const FString& CrossRef, const FDocRegistry& Registry) const;

	/**
	 * EN: Convert all [[...]] patterns in HTML to clickable links.
	 * ES: Convertir todos los patrones [[...]] en HTML a links clickeables.
	 */
	FString ProcessCrossRefsInHtml(const FString& Html, const FDocRegistry& Registry) const;

	/**
	 * EN: Execute a resolved link (navigate, open file, etc.)
	 * ES: Ejecutar un link resuelto (navegar, abrir archivo, etc.)
	 */
	void ExecuteLink(const FResolvedLink& Link) const;
};
