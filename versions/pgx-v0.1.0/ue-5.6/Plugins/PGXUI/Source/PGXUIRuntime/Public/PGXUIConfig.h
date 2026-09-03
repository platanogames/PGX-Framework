// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Data/PGXConfigDataAsset.h"
#include "Observability/PGXObservable.h"
#include "PGXUIConfig.generated.h"

/**
 * EN: Config DataAsset for the PGX UI system.
 *     Defines screen transitions, stack behavior, and widget pool settings.
 *
 * ES: Config DataAsset para el sistema de UI PGX.
 *     Define transiciones de pantalla, comportamiento del stack y configuracion del pool de widgets.
 */
UCLASS(BlueprintType)
class PGXUIRUNTIME_API UPGXUIConfig : public UPGXConfigDataAsset, public IPGXObservable
{
	GENERATED_BODY()

public:
	//~ Begin IPGXObservable
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;
	//~ End IPGXObservable

	/** EN: Default duration for screen transitions in seconds / ES: Duracion por defecto para transiciones de pantalla en segundos */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|UI", meta = (ClampMin = "0.0"))
	float DefaultTransitionDuration = 0.3f;

	/** EN: Maximum depth of the screen stack / ES: Profundidad maxima del stack de pantallas */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|UI", meta = (ClampMin = "1", ClampMax = "32"))
	int32 MaxScreenStackDepth = 10;

	/** EN: Show loading screen on level transitions / ES: Mostrar pantalla de carga en transiciones de nivel */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|UI")
	bool bShowLoadingScreenOnLevelChange = true;

	/** EN: Default notification display time in seconds / ES: Tiempo de display de notificacion por defecto en segundos */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|UI", meta = (ClampMin = "0.5"))
	float NotificationDisplayTime = 3.0f;

	/** EN: Initial size of the widget pool / ES: Tamano inicial del pool de widgets */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|UI", meta = (ClampMin = "0"))
	int32 WidgetPoolInitialSize = 16;
};
