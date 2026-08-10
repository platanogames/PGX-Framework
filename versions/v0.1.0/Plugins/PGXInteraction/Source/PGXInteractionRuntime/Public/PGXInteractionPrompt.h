// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGXInteractionPrompt.generated.h"

/**
 * EN: Widget displaying contextual interaction prompt (e.g., "Press E to open").
 *     Subclass in Blueprints to customize the visual presentation.
 *
 * ES: Widget que muestra prompt de interaccion contextual (ej., "Presiona E para abrir").
 *     Crear subclase en Blueprints para personalizar la presentacion visual.
 */
UCLASS(Abstract, Blueprintable)
class PGXINTERACTIONRUNTIME_API UPGXInteractionPrompt : public UUserWidget
{
	GENERATED_BODY()

public:
	/** EN: Set the prompt text to display / ES: Establecer el texto de prompt a mostrar */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|Interaction")
	void SetPromptText(const FText& Text);
	virtual void SetPromptText_Implementation(const FText& Text);
};
