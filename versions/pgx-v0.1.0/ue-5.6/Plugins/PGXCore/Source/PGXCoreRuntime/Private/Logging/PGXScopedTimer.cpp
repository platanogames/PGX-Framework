// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Logging/PGXScopedTimer.h"
#include "Logging/PGXLogCategories.h"

// EN: Scoped timer implementation - logs elapsed ms on destruction
// ES: Implementacion del temporizador de scope - loguea ms transcurridos al destruirse

FPGXScopedTimer::FPGXScopedTimer(FName InCategory, const TCHAR* InLabel)
	: Category(InCategory)
	, Label(InLabel)
	, StartTime(FPlatformTime::Seconds())
{
}

FPGXScopedTimer::~FPGXScopedTimer()
{
	const double ElapsedMs = (FPlatformTime::Seconds() - StartTime) * 1000.0;

	// EN: Use UE_LOG directly to avoid circular dependency with macros/subsystem
	// ES: Usar UE_LOG directamente para evitar dependencia circular con macros/subsistema
	UE_LOG(LogPGX, Log, TEXT("[INF][%s] %s completed in %.2fms"), *Category.ToString(), *Label, ElapsedMs);
}
