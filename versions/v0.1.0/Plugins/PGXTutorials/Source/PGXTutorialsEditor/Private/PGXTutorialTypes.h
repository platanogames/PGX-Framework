// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Tutorial types — step definition, action types, agent types.
// ES: Tipos de tutorial — definicion de paso, tipos de accion, tipos de agente.
#pragma once

#include "CoreMinimal.h"

/**
 * EN: Language selection for bilingual tutorial content.
 * ES: Seleccion de idioma para contenido tutorial bilingue.
 */
enum class EPGXTutorialLanguage : uint8
{
	English,
	Spanish
};

/**
 * EN: Action type that a tutorial step can execute (Agent system).
 * ES: Tipo de accion que un paso de tutorial puede ejecutar (sistema de acciones).
 */
enum class EPGXTutorialAction : uint8
{
	None,            // EN: Display only (Guide Agent) / ES: Solo mostrar (Agente Guia)
	CreateFolder,    // EN: Create folder in Content Browser / ES: Crear carpeta en Content Browser
	CreateAsset,     // EN: Create DataAsset and open it / ES: Crear DataAsset y abrirlo
	OpenAsset,       // EN: Open existing asset (created in previous step) / ES: Abrir asset existente
	NavigateCB,      // EN: Navigate Content Browser to a folder / ES: Navegar Content Browser a una carpeta
	ConfigBasePath,  // EN: Show base path configuration (Step 0 of Constructor tutorials) / ES: Configuracion de ruta base (Paso 0)
};

/**
 * EN: Agent type for tutorial classification in the Hub.
 *     Guide = shows existing panels, explains. Constructor = creates folders/assets.
 * ES: Tipo de agente para clasificacion de tutoriales en el Hub.
 *     Guia = muestra paneles existentes, explica. Constructor = crea carpetas/assets.
 */
enum class EPGXTutorialAgentType : uint8
{
	Guide,       // EN: Shows existing panels, explains / ES: Muestra paneles existentes, explica
	Constructor, // EN: Creates folders, assets, opens them / ES: Crea carpetas, assets, los abre
};

/**
 * EN: Result of a tutorial action execution.
 * ES: Resultado de la ejecucion de una accion de tutorial.
 */
struct FPGXTutorialActionResult
{
	bool bSuccess = false;
	FText FeedbackText;
};

/**
 * EN: A single step in a guided tutorial sequence.
 *     Supports both Guide steps (display-only) and Constructor steps (create assets/folders).
 * ES: Un paso individual en una secuencia de tutorial guiado.
 *     Soporta pasos Guia (solo mostrar) y Constructor (crear assets/carpetas).
 */
struct FPGXTutorialStep
{
	/** EN: Tab ID to highlight (NAME_None = no target, callout centered) / ES: Tab ID a resaltar */
	FName TargetTabId = NAME_None;

	/** EN: Open the tab if not visible? / ES: Abrir el tab si no esta visible? */
	bool bOpenTab = true;

	/** EN: Step title / ES: Titulo del paso */
	FText Title;

	/** EN: Step description / ES: Descripcion del paso */
	FText Description;

	/** EN: Accent color for highlight (defaults to Tutorials system token; per-step override allowed) / ES: Color de acento para el highlight (defaults al token PGX::System::Tutorials; override per-step permitido) */
	// Default value applied via canonical token at construction site (see SPGXTutorialOverlay / SPGXTutorialHub consumers).
	// Hardcoded literal preserved here to avoid header include of PGXVisualTokens.h (header-only consume to PGXCoreEditor — keeps PGXTutorialsEditor Build.cs surface minimal).
	// Consumers should apply PGX::System::Tutorials at construction; this literal is a fallback that avoids a public header dependency.
	FLinearColor AccentColor = FLinearColor(0.95f, 0.55f, 0.15f, 1.0f); // matches PGX::System::Tutorials (warm orange-amber)

	// --- Tutorial action system / Sistema de acciones de tutorial ---

	/** EN: Action to execute when this step is applied / ES: Accion a ejecutar al aplicar este paso */
	EPGXTutorialAction Action = EPGXTutorialAction::None;

	/** EN: Relative path to tutorial BasePath (e.g., "Profile", "GameFlow/Configs") / ES: Ruta relativa al BasePath */
	FString ActionPath;

	/** EN: Asset class path for CreateAsset (e.g., "/Script/PGXCoreRuntime.PGXProjectProfileConfig") / ES: Ruta de clase del asset */
	FString AssetClass;

	/** EN: Asset name for CreateAsset/OpenAsset (e.g., "MyProfileConfig") / ES: Nombre del asset */
	FString AssetName;
};
