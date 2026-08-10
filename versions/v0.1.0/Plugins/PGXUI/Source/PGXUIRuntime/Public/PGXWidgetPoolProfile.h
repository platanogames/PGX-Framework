// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Observability/PGXObservable.h"
#include "PGXWidgetPoolProfile.generated.h"

class UUserWidget;

/**
 * EN: Authored Widget Pool Profile Object DA. Declares the pool type identity,
 *     widget class binding, capacity policy, and reset/release validator policy. Runtime
 *     does not consume this profile: `UPGXWidgetPool` accepts
 *     `TSubclassOf<UUserWidget>` + `int32 Capacity` directly via
 *     `Initialize(...)` + `AcquireWidget(...)` and does NOT load `UPGXWidgetPoolProfile` assets
 *     today and is state-only; it does not spawn widgets. The profile remains an authoring
 *     and validation surface for per-pool policy.
 *
 *     Validation rules by the documented validation rules:
 *     - Widget class policy present (`WidgetClass` soft reference valid OR `bIsAbstractPool` true).
 *     - Reset validator policy explicit (`bResetOnRelease` + `MaxReuseCount`).
 *
 * ES: Object DA autorada de Perfil de Pool de Widgets. Declara la identidad
 *     del tipo de pool, binding de clase de widget, politica de capacidad y politica de reset/
 *     release. UPGXWidgetPool no consume este perfil y solo mantiene estado; no instancia widgets.
 */
UCLASS(BlueprintType, meta = (DisplayName = "PGX Widget Pool Profile"))
class PGXUIRUNTIME_API UPGXWidgetPoolProfile : public UPrimaryDataAsset, public IPGXObservable
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
	// EN: Identity (validation: PoolTypeTag resolves under PGX.UI.WidgetPool.Type.*)
	// ES: Identidad
	// ========================================================================

	/** EN: Pool type tag — must resolve under `PGX.UI.WidgetPool.Type.*`. Authoring identity. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|UI|Identity",
		meta = (Categories = "PGX.UI.WidgetPool.Type"))
	FGameplayTag PoolTypeTag;

	/** EN: Editor-friendly display name. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|UI|Identity")
	FText DisplayName;

	// ========================================================================
	// EN: Widget binding (validation: WidgetClass present OR bIsAbstractPool)
	// ES: Binding del widget
	// ========================================================================

	/**
	 * EN: Soft reference to the UUserWidget subclass managed by this pool. Soft pointer keeps
	 *     UMG asset cooked separately. May be null IF `bIsAbstractPool == true`. **NOT CONSUMED AT RUNTIME**:
	 *     the pool is state-only and does not spawn widgets.
	 * ES: Soft reference a la clase UUserWidget gestionada. Puede ser null si bIsAbstractPool=true.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|UI|Widget",
		meta = (ToolTip = "Declares pool policy metadata; runtime does not consume this setting."))
	TSoftClassPtr<UUserWidget> WidgetClass;

	/**
	 * EN: When true, this is an abstract pool (e.g., a parent shape projects subclass). `WidgetClass`
	 *     may be null in this case; concrete child profiles bind real classes.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|UI|Widget")
	bool bIsAbstractPool = false;

	// ========================================================================
	// EN: Capacity policy
	// ES: Politica de capacidad
	// ========================================================================

	/** EN: Initial pool capacity at subsystem warm-up. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|UI|Capacity",
		meta = (ClampMin = "0", ClampMax = "1024"))
	int32 InitialCapacity = 16;

	/** EN: Maximum capacity (hard cap). When pool is full, AcquireWidget returns PoolExhausted. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|UI|Capacity",
		meta = (ClampMin = "1", ClampMax = "4096"))
	int32 MaxCapacity = 64;

	// ========================================================================
	// EN: Reset / release validator policy (validation: explicit by the documented validation rules)
	// ES: Politica de reset y release
	// ========================================================================

	/**
	 * EN: When true, the pool runs a reset validator on each released widget before returning it
	 *     to the available set (clears state, restores defaults). **NOT CONSUMED AT RUNTIME**: validator
	 *     callback hook is reserved for the spawning update.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|UI|Reset")
	bool bResetOnRelease = true;

	/** EN: Maximum number of reuses for a single pooled instance before forced re-creation. 0 = unlimited. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|UI|Reset",
		meta = (ClampMin = "0", ClampMax = "10000"))
	int32 MaxReuseCount = 0;
};
