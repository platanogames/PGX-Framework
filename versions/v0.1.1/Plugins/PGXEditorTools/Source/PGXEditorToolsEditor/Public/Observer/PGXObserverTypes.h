// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Style/PGXVisualTokens.h"

// ============================================================================
// EN: Types for the PGX System Observer Panel.
//     Used to snapshot framework state for diagnostic display.
//
// ES: Tipos para el Panel Observer del Sistema PGX.
//     Usados para capturar estado del framework para display diagnostico.
// ============================================================================

/**
 * EN: Health status for a monitored system or component.
 * ES: Estado de salud para un sistema o componente monitoreado.
 */
enum class EPGXSystemHealth : uint8
{
	Active,     // Green  — initialized and running
	Warning,    // Yellow — initialized with issues (stub, partial)
	Error,      // Red    — failed to initialize or runtime error
	Inactive,   // Gray   — not initialized / module not loaded
	Unknown     // ?      — can't determine state
};

/**
 * EN: Snapshot of a single PGX subsystem's state.
 * ES: Snapshot del estado de un unico subsistema PGX.
 */
struct FPGXSubsystemSnapshot
{
	FName SubsystemName;        // "PGXSave", "PGXLog", etc.
	FString ClassName;          // "UPGXSaveSubsystem"
	EPGXSystemHealth Health;    // Current health
	FString Version;            // "v1.0", "v2.2"
	FString StatusDetail;       // "3 contexts, 5 domains"
	bool bIsImplemented;        // true if beyond stub
	FGameplayTag SystemTag;     // TAG_PGX_System_Save
	float MetricValue = 0.0f;  // Per-system numeric metric for sparkline (log entries, sounds, etc.)
};

/**
 * EN: Snapshot of a core class instance (GameInstance, GameMode, etc.).
 * ES: Snapshot de una instancia de clase core (GameInstance, GameMode, etc.).
 */
struct FPGXCoreClassSnapshot
{
	FName Role;                 // "GameInstance", "GameMode", etc.
	FString ClassName;          // "BP_GameInstance_C"
	EPGXSystemHealth Health;    // Green if valid, Red if null
};

/**
 * EN: Snapshot of a plugin's load state.
 * ES: Snapshot del estado de carga de un plugin.
 */
struct FPGXPluginSnapshot
{
	FName PluginName;           // "PGXSave", "PGXAudio"
	FString ModuleName;         // "PGXSaveRuntime"
	bool bModuleLoaded;         // IModuleInterface accessible
	EPGXSystemHealth Health;
};

/**
 * EN: Get display color for a health status.
 * ES: Obtener color de display para un estado de salud.
 */
inline FLinearColor GetHealthColor(EPGXSystemHealth Health)
{
	// EN: Delegate to PGX Semantic tokens as the single source of truth.
	// ES: Delegar a tokens PGX Semantic como fuente unica de verdad.
	switch (Health)
	{
	case EPGXSystemHealth::Active:   return PGX::Semantic::Good;
	case EPGXSystemHealth::Warning:  return PGX::Semantic::Warn;
	case EPGXSystemHealth::Error:    return PGX::Semantic::Error;
	case EPGXSystemHealth::Inactive: return PGX::Semantic::Neutral;
	default:                         return PGX::Semantic::Neutral;
	}
}

/**
 * EN: Get display text for a health status.
 * ES: Obtener texto de display para un estado de salud.
 */
inline FString GetHealthText(EPGXSystemHealth Health)
{
	switch (Health)
	{
	case EPGXSystemHealth::Active:   return TEXT("Active");
	case EPGXSystemHealth::Warning:  return TEXT("Warning");
	case EPGXSystemHealth::Error:    return TEXT("Error");
	case EPGXSystemHealth::Inactive: return TEXT("Inactive");
	default:                         return TEXT("Unknown");
	}
}
