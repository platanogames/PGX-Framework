// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PGXDependencyDiagnostics.generated.h"

/**
 * EN: Diagnostics tool for PGX system dependencies. Validates dependency graphs,
 *     detects circular dependencies, reports initialization order issues,
 *     and provides runtime health checks for all PGX subsystems.
 * ES: Herramienta de diagnostico para dependencias de sistemas PGX. Valida grafos de dependencias,
 *     detecta dependencias circulares, reporta problemas de orden de inicializacion,
 *     y proporciona health checks en runtime para todos los subsistemas PGX.
 */
UCLASS()
class PGXCORERUNTIME_API UPGXDependencyDiagnostics : public UObject
{
	GENERATED_BODY()
};
