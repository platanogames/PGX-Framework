// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Validation/PGXRegistryValidationTypes.h"

class UDataTable;

/**
 * EN: Safe autofix service for PGX Registry validation issues.
 *     Only performs low-risk, deterministic fixes with confirmation.
 *
 *     Allowed: RDT012 (tag normalization), RDT030 (null removal), RDT033 (redirector fixup).
 *     Forbidden: auto-renaming tags, changing conflict policy, rewiring assets.
 *
 * ES: Servicio de autofix seguro para problemas de validacion del PGX Registry.
 *     Solo realiza correcciones de bajo riesgo y deterministicas con confirmacion.
 *
 *     Permitido: RDT012 (normalizacion de tags), RDT030 (remocion de nulos), RDT033 (fixup de redirectores).
 *     Prohibido: auto-renombrar tags, cambiar politica de conflicto, reconectar assets.
 */
class FPGXRegistryFixService
{
public:
	/**
	 * EN: Apply safe fixes for all auto-fixable issues in the given list.
	 *     Returns the number of issues successfully fixed.
	 * ES: Aplicar correcciones seguras para todos los problemas auto-corregibles en la lista.
	 *     Retorna el numero de problemas corregidos exitosamente.
	 */
	int32 ApplySafeFixes(const TArray<FPGXRegistryValidationIssue>& Issues, TArray<FString>& OutLog);

	/**
	 * EN: Attempt to fix a single issue. Returns true if fixed.
	 * ES: Intentar corregir un problema individual. Retorna true si se corrigio.
	 */
	bool FixIssue(const FPGXRegistryValidationIssue& Issue, FString& OutLog);

private:
	/**
	 * EN: RDT030 — Remove null entries from a DataTable's ItemsByTag maps.
	 * ES: RDT030 — Remover entradas nulas de los mapas ItemsByTag de un DataTable.
	 */
	bool FixNullAssetRefs(const FString& TableName, FName RowName, FString& OutLog);

	/**
	 * EN: RDT033 — Trigger UE's redirector fixup for the affected asset.
	 * ES: RDT033 — Disparar el fixup de redirectores de UE para el asset afectado.
	 */
	bool FixRedirector(const FString& TableName, FName RowName, const FString& AssetPath, FString& OutLog);
};
