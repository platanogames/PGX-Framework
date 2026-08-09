// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PGXScreen.h"
#include "PGXLoadingScreen.generated.h"

/**
 * EN: Loading screen base with progress tracking and tip display.
 *     Derive to create custom loading screens with project-specific visuals.
 *
 * ES: Base de pantalla de carga con tracking de progreso y display de tips.
 *     Derivar para crear pantallas de carga personalizadas con visuales del proyecto.
 */
UCLASS(Abstract, Blueprintable)
class PGXUIRUNTIME_API UPGXLoadingScreen : public UPGXScreen
{
	GENERATED_BODY()

public:
	/** EN: Updates the loading progress (0.0 to 1.0) / ES: Actualiza el progreso de carga (0.0 a 1.0) */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|UI")
	void UpdateProgress(float Progress);
	virtual void UpdateProgress_Implementation(float Progress);
};
