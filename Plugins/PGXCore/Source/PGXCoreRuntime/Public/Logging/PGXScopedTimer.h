// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Logging/PGXLogTypes.h"

/**
 * EN: RAII scope timer that logs elapsed time when the scope exits.
 *     Created by PGX_LOG_SCOPE_TIMER macro.
 *     Compiled out in Shipping builds.
 *
 * ES: Temporizador RAII de scope que loguea el tiempo transcurrido al salir del scope.
 *     Creado por la macro PGX_LOG_SCOPE_TIMER.
 *     Se elimina en builds Shipping.
 *
 * Usage / Uso:
 *     void UPGXSaveSubsystem::SaveSlot(const FString& SlotName)
 *     {
 *         PGX_LOG_SCOPE_TIMER(LogPGXSave, "SaveSlot");
 *         // ... save logic ...
 *     } // Output: [INF][LogPGXSave] SaveSlot completed in 12.34ms
 */
class PGXCORERUNTIME_API FPGXScopedTimer
{
public:
	FPGXScopedTimer(FName InCategory, const TCHAR* InLabel);
	~FPGXScopedTimer();

	// EN: Non-copyable, non-movable / ES: No copiable, no movible
	FPGXScopedTimer(const FPGXScopedTimer&) = delete;
	FPGXScopedTimer& operator=(const FPGXScopedTimer&) = delete;

private:
	FName Category;
	FString Label;
	double StartTime;
};
