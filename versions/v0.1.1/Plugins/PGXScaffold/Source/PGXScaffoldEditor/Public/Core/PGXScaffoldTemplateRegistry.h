// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Core/PGXScaffoldTypes.h"

/**
 * EN: Registry for scaffold templates. Holds built-in and user-registered templates.
 *     Templates are registered during module startup.
 *
 * ES: Registro de templates de scaffold. Contiene templates built-in y registrados por el usuario.
 *     Los templates se registran durante el startup del modulo.
 */
class PGXSCAFFOLDEDITOR_API FPGXScaffoldTemplateRegistry
{
public:
	/** EN: Register a template / ES: Registrar un template */
	void RegisterTemplate(const FPGXScaffoldTemplate& Template);

	/** EN: Get all registered templates / ES: Obtener todos los templates registrados */
	const TArray<FPGXScaffoldTemplate>& GetAllTemplates() const { return Templates; }

	/** EN: Find template by ID / ES: Buscar template por ID */
	const FPGXScaffoldTemplate* FindTemplate(FName TemplateId) const;

	/** EN: Get unique categories / ES: Obtener categorias unicas */
	TArray<FName> GetCategories() const;

	/** EN: Get templates in a category / ES: Obtener templates de una categoria */
	TArray<const FPGXScaffoldTemplate*> GetTemplatesByCategory(FName Category) const;

	/** EN: Singleton access / ES: Acceso singleton */
	static FPGXScaffoldTemplateRegistry& Get();

private:
	TArray<FPGXScaffoldTemplate> Templates;
	static TUniquePtr<FPGXScaffoldTemplateRegistry> Instance;
};
