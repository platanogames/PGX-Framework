// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

/**
 * EN: Exported native gameplay tag handles for the Environment runtime module.
 *     Variable and Zone are extensible parent branches; Severity and Result
 *     provide framework-defined classification tags.
 * ES: Handles nativos exportados del modulo runtime de Environment.
 *     Variable y Zone son ramas padre extensibles; Severity y Result aportan
 *     tags de clasificacion definidos por el framework.
 */

// EN: Parent branches (developer-extensible).
// ES: Ramas padre (developer-extensible).
extern PGXENVIRONMENTRUNTIME_API FNativeGameplayTag TAG_PGX_Environment_Variable;
extern PGXENVIRONMENTRUNTIME_API FNativeGameplayTag TAG_PGX_Environment_Zone;

// EN: Severity branch (framework-defined).
// ES: Rama Severity (framework-defined).
extern PGXENVIRONMENTRUNTIME_API FNativeGameplayTag TAG_PGX_Environment_Severity;
extern PGXENVIRONMENTRUNTIME_API FNativeGameplayTag TAG_PGX_Environment_Severity_None;
extern PGXENVIRONMENTRUNTIME_API FNativeGameplayTag TAG_PGX_Environment_Severity_Minor;
extern PGXENVIRONMENTRUNTIME_API FNativeGameplayTag TAG_PGX_Environment_Severity_Moderate;
extern PGXENVIRONMENTRUNTIME_API FNativeGameplayTag TAG_PGX_Environment_Severity_Severe;
extern PGXENVIRONMENTRUNTIME_API FNativeGameplayTag TAG_PGX_Environment_Severity_Critical;

// EN: Result branch (framework-defined parent).
// ES: Rama Result (padre definido por el framework).
extern PGXENVIRONMENTRUNTIME_API FNativeGameplayTag TAG_PGX_Environment_Result;
