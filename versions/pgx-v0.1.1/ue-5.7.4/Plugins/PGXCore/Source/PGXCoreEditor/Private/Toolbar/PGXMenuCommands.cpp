// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Toolbar/PGXMenuCommands.h"

#define LOCTEXT_NAMESPACE "FPGXMenuCommands"

// EN: PGX menu commands implementation
// ES: Implementacion de comandos de menu PGX

FPGXMenuCommands::FPGXMenuCommands()
	: TCommands<FPGXMenuCommands>(
		TEXT("PGXEditor"),
		LOCTEXT("PGXEditorCommands", "PGX Editor"),
		NAME_None,
		FAppStyle::GetAppStyleSetName())
{
}

void FPGXMenuCommands::RegisterCommands()
{
	UI_COMMAND(OpenHub, "PGX Hub", "Open the PGX Hub dashboard", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control | EModifierKey::Shift, EKeys::P));
	UI_COMMAND(ValidateFramework, "Validate Framework", "Run PGX framework validation", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenSettings, "PGX Settings", "Open PGX section in Project Settings", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenDocs, "Documentation", "Open PGX documentation folder", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(RestartEditor, "Restart Editor", "Save and restart the editor safely", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control | EModifierKey::Shift, EKeys::R));
}

#undef LOCTEXT_NAMESPACE
