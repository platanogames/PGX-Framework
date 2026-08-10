// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IPGXDocumentationProvider.generated.h"

/**
 * EN: Interface for plugins that want to register their own documentation
 *     with the PGX Documentation Viewer. Implement this interface in your
 *     plugin's module to auto-register docs at startup.
 *
 * ES: Interfaz para plugins que quieren registrar su propia documentacion
 *     con el Visor de Documentacion PGX. Implementar esta interfaz en el
 *     modulo de tu plugin para auto-registrar docs al inicio.
 */
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UPGXDocumentationProvider : public UInterface
{
	GENERATED_BODY()
};

class PGXDOCS_API IPGXDocumentationProvider
{
	GENERATED_BODY()

public:
	/**
	 * EN: Get the display name for this documentation module.
	 * ES: Obtener el nombre para mostrar de este modulo de documentacion.
	 */
	virtual FString GetDocModuleName() const = 0;

	/**
	 * EN: Get the root directory path(s) containing .md documentation files.
	 * ES: Obtener la(s) ruta(s) del directorio raiz con archivos .md de documentacion.
	 */
	virtual TArray<FString> GetDocRootPaths() const = 0;

	/**
	 * EN: Get the optional manifest file path (module.manifest.json).
	 *     Return empty string if no manifest.
	 * ES: Obtener la ruta opcional del archivo manifest (module.manifest.json).
	 *     Retornar string vacio si no hay manifest.
	 */
	virtual FString GetManifestPath() const { return FString(); }

	/**
	 * EN: Priority for ordering in the tree (lower = higher in tree).
	 * ES: Prioridad para ordenar en el arbol (menor = mas alto en arbol).
	 */
	virtual int32 GetDocPriority() const { return 100; }
};
