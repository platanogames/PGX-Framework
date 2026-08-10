// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXEditorNotification.generated.h"

/**
 * EN: Editor notification utility for PGX framework.
 *     Provides consistent toast notifications, warnings, and messages in the editor.
 *
 * ES: Utilidad de notificaciones de editor para el framework PGX.
 *     Proporciona notificaciones toast, warnings y mensajes consistentes en el editor.
 */
UCLASS()
class PGXCOREEDITOR_API UPGXEditorNotification : public UObject
{
	GENERATED_BODY()

public:
	/** EN: Show an info notification / ES: Mostrar una notificacion informativa */
	UFUNCTION(BlueprintCallable, Category = "PGX|Editor")
	static void ShowInfo(const FText& Message);

	/** EN: Show a warning notification / ES: Mostrar una notificacion de warning */
	UFUNCTION(BlueprintCallable, Category = "PGX|Editor")
	static void ShowWarning(const FText& Message);

	/** EN: Show an error notification / ES: Mostrar una notificacion de error */
	UFUNCTION(BlueprintCallable, Category = "PGX|Editor")
	static void ShowError(const FText& Message);
};
