// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "PGXDemoAssetGeneratorCommandlet.generated.h"

/**
 * EN: Generates the DA_Demo_*.uasset files declared in FPGXDemoRegistry. For each registry
 *     entry it resolves the UClass from ClassPath, creates the DataAsset under /Game/PGX_Demo,
 *     runs FPGXDemoPopulator (container with defaults if no populate case exists yet), and
 *     saves the package. Idempotent: existing assets are skipped unless -force.
 *     Run with the editor CLOSED.
 *
 *     Usage: UnrealEditor-Cmd.exe <ProjectPath> -run=PGXDemoAssetGenerator
 *            [-only=<DA_Demo_Name>[,<...>]] [-force] [-dryrun] [-savedir=/Game/PGX_Demo]
 *     Exit codes:
 *       0 = success (all targeted assets created or skipped)
 *       1 = one or more assets failed to create/populate
 *       2 = one or more assets failed to save
 *       3 = commandlet internal error
 *
 * ES: Genera los DA_Demo_*.uasset declarados en FPGXDemoRegistry. Por cada entrada resuelve
 *     la UClass desde ClassPath, crea el DataAsset bajo /Game/PGX_Demo, ejecuta
 *     FPGXDemoPopulator (contenedor con defaults si aun no hay caso de poblado) y guarda el
 *     paquete. Idempotente: los assets existentes se saltan salvo -force. Editor CERRADO.
 */
UCLASS()
class UPGXDemoAssetGeneratorCommandlet : public UCommandlet
{
	GENERATED_UCLASS_BODY()

	//~ Begin UCommandlet Interface
	int32 Main(const FString& Params) override;
	//~ End UCommandlet Interface
};
