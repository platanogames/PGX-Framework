// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXEnvironmentSettings.h"

// EN: Settings UCLASS — no constructor logic at baseline; UPROPERTY
//     defaults supply the data-driven values. The current implementation does not include
//     PostEditChangeProperty validation hooks if Project Settings UI
//     needs to gate invalid edits.
// ES: UCLASS de Settings — sin logica de constructor en baseline; los
//     defaults UPROPERTY proveen los valores data-driven. versiones futuras
//     pueden añadir hooks PostEditChangeProperty de validacion si la UI
//     de Project Settings necesita gatear ediciones invalidas.
