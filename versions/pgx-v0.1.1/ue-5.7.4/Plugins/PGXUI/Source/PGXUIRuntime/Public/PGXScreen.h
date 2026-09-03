// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGXScreen.generated.h"

/**
 * EN: Base widget class for PGX screens with lifecycle hooks.
 *     All managed screens should derive from this class.
 *
 * ES: Clase base de widget para pantallas PGX con hooks de ciclo de vida.
 *     Todas las pantallas gestionadas deben derivar de esta clase.
 */
UCLASS(Abstract, Blueprintable)
class PGXUIRUNTIME_API UPGXScreen : public UUserWidget
{
	GENERATED_BODY()

public:
	/** EN: Called when the screen is opened and displayed / ES: Se llama cuando la pantalla se abre y se muestra */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|UI")
	void OnScreenOpened();
	virtual void OnScreenOpened_Implementation();

	/** EN: Called when the screen is closed and removed / ES: Se llama cuando la pantalla se cierra y se remueve */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|UI")
	void OnScreenClosed();
	virtual void OnScreenClosed_Implementation();
};
