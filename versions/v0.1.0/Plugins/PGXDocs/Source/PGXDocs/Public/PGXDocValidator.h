// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXDocTypes.h"

class FDocRegistry;

/**
 * EN: Validates documentation cross-references, images, and structural integrity.
 *     Produces a validation report with errors and warnings.
 *
 * ES: Valida referencias cruzadas, imagenes e integridad estructural de la documentacion.
 *     Produce un reporte de validacion con errores y warnings.
 */
class FDocValidator
{
public:
	/**
	 * EN: Validate all documents in the registry.
	 * ES: Validar todos los documentos en el registry.
	 */
	FDocValidationReport ValidateAll(const FDocRegistry& Registry) const;

	/**
	 * EN: Validate a single document.
	 * ES: Validar un solo documento.
	 */
	TArray<FDocValidationEntry> ValidateDocument(const FString& DocId, const FString& Content, const FDocRegistry& Registry) const;

private:
	// EN: Check [[...]] crossref links resolve / ES: Verificar que links crossref [[...]] resuelven
	TArray<FDocValidationEntry> ValidateCrossRefs(const FString& DocId, const FString& Content, const FDocRegistry& Registry) const;

	// EN: Check image paths exist / ES: Verificar que rutas de imagenes existen
	TArray<FDocValidationEntry> ValidateImages(const FString& DocId, const FString& Content, const FString& DocPath) const;

	// EN: Check standard markdown links / ES: Verificar links markdown estandar
	TArray<FDocValidationEntry> ValidateLinks(const FString& DocId, const FString& Content, const FDocRegistry& Registry) const;
};
