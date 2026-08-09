// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Observability/PGXObservable.h"
#include "PGXScreenDefinition.generated.h"

class UUserWidget;

/**
 * EN: Authored Screen Definition Object DA. Declares the screen widget class,
 *     layer placement, screen tag, transition profile reference and presentation policy. **Runtime
 *     consumption is NOT CONSUMED AT RUNTIME at preview**: preview service object
 *     `UPGXScreenManager` push/pop accepts FGameplayTag + FString DebugName + int32 Layer directly
 *     via `PushScreen(...)` and does NOT load `UPGXScreenDefinition` assets today. This Object DA
 *     ships the SHAPE so projects can author screen definitions; the runtime integration
 *     (presentation policy resolution from Config DA / Object DA / Settings) will start consuming
 *     `WidgetClass` + `LayerTag` + `TransitionProfileTag` to drive screen pushes.
 *
 *     Validation rules by the documented validation rules:
 *     - Widget class soft reference valid (asset path resolves to a UUserWidget subclass).
 *     - `LayerTag` resolves to a `PGX.UI.Screen.Layer.*` tag.
 *     - `ScreenTag` resolves to a `PGX.UI.Screen.Type.*` tag.
 *
 * ES: Object DA autorada de Definicion de Pantalla. Declara la clase del widget,
 *     placement de capa, tag de pantalla, referencia a perfil de transicion y politica de presentacion.
 *     **El consumo en runtime NO esta cableado todavia** — UPGXScreenManager preview acepta valores
 *     directos via PushScreen(...) sin cargar definiciones autoradas. una futura actualizacion cableara el consumo.
 */
UCLASS(BlueprintType, meta = (DisplayName = "PGX Screen Definition"))
class PGXUIRUNTIME_API UPGXScreenDefinition : public UPrimaryDataAsset, public IPGXObservable
{
	GENERATED_BODY()

public:
	//~ Begin IPGXObservable
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;
	//~ End IPGXObservable

	// ========================================================================
	// EN: Identity (validation: ScreenTag resolves to PGX.UI.Screen.Type.*)
	// ES: Identidad
	// ========================================================================

	/** EN: Screen type tag — must resolve under `PGX.UI.Screen.Type.*`. Authoring identity. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|UI|Identity",
		meta = (Categories = "PGX.UI.Screen.Type"))
	FGameplayTag ScreenTag;

	/** EN: Editor-friendly display name. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|UI|Identity")
	FText DisplayName;

	// ========================================================================
	// EN: Widget binding (validation: soft reference valid)
	// ES: Binding del widget
	// ========================================================================

	/**
	 * EN: Soft reference to the UUserWidget subclass spawned for this screen. Soft pointer keeps
	 *     UMG asset cooked separately and avoids loading widget at module init. **NOT CONSUMED AT RUNTIME**:
	 *     preview service object does not load this; a current runtime does not.
	 * ES: Soft reference a la subclase UUserWidget spawneada para esta pantalla.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|UI|Widget",
		meta = (ToolTip = "Declares configuration metadata; runtime does not consume this setting."))
	TSoftClassPtr<UUserWidget> WidgetClass;

	// ========================================================================
	// EN: Layer + transition (validation: LayerTag resolves to PGX.UI.Screen.Layer.*)
	// ES: Capa y transicion
	// ========================================================================

	/** EN: Layer tag — must resolve under `PGX.UI.Screen.Layer.*` (e.g., HUD / Menu / Modal). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|UI|Layer",
		meta = (Categories = "PGX.UI.Screen.Layer"))
	FGameplayTag LayerTag;

	/** EN: Numeric layer override (used when project policy requires manual ordering). 0 = default. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|UI|Layer",
		meta = (ClampMin = "0", ClampMax = "100"))
	int32 LayerOrder = 0;

	/**
	 * EN: Reference tag to a transition profile (future `UPGXUITransitionProfile` Object DA update).
	 *     **NOT CONSUMED AT RUNTIME**: Transition profile DA resolution is not performed by the current runtime.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|UI|Transition",
		meta = (ToolTip = "Reserved for UPGXUITransitionProfile resolution update; not consumed at preview."))
	FGameplayTag TransitionProfileTag;

	// ========================================================================
	// EN: Presentation policy
	// ES: Politica de presentacion
	// ========================================================================

	/** EN: When true, this screen blocks input on screens beneath it (modal). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|UI|Policy")
	bool bIsModal = false;

	/** EN: When true, this screen pauses gameplay simulation while visible (per project policy). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|UI|Policy")
	bool bPausesGameplay = false;
};
