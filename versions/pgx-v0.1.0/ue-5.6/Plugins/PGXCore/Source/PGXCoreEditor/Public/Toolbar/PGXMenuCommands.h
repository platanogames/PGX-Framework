// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"

/**
 * EN: Defines all PGX editor menu commands and keyboard shortcuts.
 *     Registered in FPGXToolbarBuilder and mapped to editor actions.
 * ES: Define todos los comandos de menu y atajos de teclado del editor PGX.
 *     Registrados en FPGXToolbarBuilder y mapeados a acciones del editor.
 */
class PGXCOREEDITOR_API FPGXMenuCommands : public TCommands<FPGXMenuCommands>
{
public:
	FPGXMenuCommands();

	// EN: Register all commands / ES: Registrar todos los comandos
	void RegisterCommands() override;

	// EN: Command to open PGX Hub / ES: Comando para abrir PGX Hub
	TSharedPtr<FUICommandInfo> OpenHub;

	// EN: Command to validate framework / ES: Comando para validar el framework
	TSharedPtr<FUICommandInfo> ValidateFramework;

	// EN: Command to open PGX Settings / ES: Comando para abrir PGX Settings
	TSharedPtr<FUICommandInfo> OpenSettings;

	// EN: Command to open documentation / ES: Comando para abrir documentacion
	TSharedPtr<FUICommandInfo> OpenDocs;

	// EN: Command to restart the editor safely / ES: Comando para reiniciar el editor de forma segura
	TSharedPtr<FUICommandInfo> RestartEditor;
};
