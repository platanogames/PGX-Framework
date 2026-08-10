// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "PGXRegistryValidateCommandlet.generated.h"

/**
 * EN: CI/Automation commandlet for PGX Registry validation.
 *     Scans all DataTables, runs the full 18-rule validator, outputs summary
 *     to stdout and writes a JSON report to Saved/PGXRegistry/validation_report.json.
 *
 *     Usage:  UnrealEditor-Cmd.exe <ProjectPath> -run=PGXRegistryValidate [-strict] [-csv] [-noexport] [-report=<file>]
 *     Exit codes:
 *       0 = no blocking validation failures
 *       1 = validation failed (errors, or warnings under -strict)
 *       2 = report export failed
 *       3 = commandlet internal error
 *
 * ES: Commandlet de CI/Automatizacion para validacion del PGX Registry.
 *     Escanea todos los DataTables, ejecuta el validador completo de 18 reglas,
 *     imprime resumen en stdout y escribe reporte JSON a Saved/PGXRegistry/validation_report.json.
 *
 *     Uso:    UnrealEditor-Cmd.exe <RutaProyecto> -run=PGXRegistryValidate [-strict] [-csv] [-noexport] [-report=<archivo>]
 *     Codigos de salida:
 *       0 = sin fallos bloqueantes de validacion
 *       1 = validacion fallida (errores, o warnings con -strict)
 *       2 = fallo exportando reporte
 *       3 = error interno del commandlet
 */
UCLASS()
class UPGXRegistryValidateCommandlet : public UCommandlet
{
	GENERATED_UCLASS_BODY()

	//~ Begin UCommandlet Interface
	int32 Main(const FString& Params) override;
	//~ End UCommandlet Interface
};
