// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Observability/PGXObservable.h"
#include "PGXNotificationProfile.generated.h"

/**
 * EN: Authored Notification Profile Object DA. Declares the notification
 *     category identity, priority policy, default display duration, coalescing policy and
 *     dismissal policy. **Runtime consumption is NOT CONSUMED AT RUNTIME at preview**:
 *     preview service object `UPGXNotificationManager` accepts `FPGXUINotificationRequest` (ad-hoc
 *     tag + message + priority + display time) directly via `EnqueueNotification(...)` and does
 *     NOT load `UPGXNotificationProfile` assets today. This Object DA ships the SHAPE so projects
 *     can author per-category notification policy; the runtime integration (notification
 *     queueing / coalescing / priority policy) will start consuming the profile.
 *
 *     Validation rules by the documented validation rules:
 *     - `CategoryTag` resolves under `PGX.UI.Notification.Category.*`.
 *     - Coalescing policy explicit (`bAllowCoalescing` + `MaxQueueDepth`).
 *     - Dismissal policy explicit (`bDismissOnHide` + `DefaultDisplayTimeSeconds`).
 *
 * ES: Object DA autorada de Perfil de Notificacion. Declara la identidad
 *     de la categoria, politica de prioridad, duracion de display, politica de coalescing y
 *     politica de dismissal. **El consumo en runtime NO esta cableado todavia** — preview acepta
 *     FPGXUINotificationRequest directo. una futura actualizacion cableara el consumo.
 */
UCLASS(BlueprintType, meta = (DisplayName = "PGX Notification Profile"))
class PGXUIRUNTIME_API UPGXNotificationProfile : public UPrimaryDataAsset, public IPGXObservable
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
	// EN: Identity (validation: CategoryTag resolves under PGX.UI.Notification.Category.*)
	// ES: Identidad
	// ========================================================================

	/** EN: Category tag — must resolve under `PGX.UI.Notification.Category.*`. Authoring identity. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|UI|Identity",
		meta = (Categories = "PGX.UI.Notification.Category"))
	FGameplayTag CategoryTag;

	/** EN: Editor-friendly display name. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|UI|Identity")
	FText DisplayName;

	// ========================================================================
	// EN: Priority policy
	// ES: Politica de prioridad
	// ========================================================================

	/** EN: Priority tag — must resolve under `PGX.UI.Notification.Priority.*` (Low / Normal / High). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|UI|Priority",
		meta = (Categories = "PGX.UI.Notification.Priority"))
	FGameplayTag PriorityTag;

	/**
	 * EN: Numeric priority for sorting in the queue. Higher = served sooner. Authored alongside
	 *     `PriorityTag` for runtime sort efficiency; the runtime integration will reconcile
	 *     tag-vs-numeric authoring policy.
	 * ES: Prioridad numerica para ordenacion. update planned work reconcilia politica tag-vs-numerica.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|UI|Priority",
		meta = (ClampMin = "0", ClampMax = "100"))
	int32 PriorityNumeric = 50;

	// ========================================================================
	// EN: Display + dismissal policy (validation: explicit by the documented validation rules)
	// ES: Politica de display y dismissal
	// ========================================================================

	/** EN: Default display time in seconds when this category fires. 0 = uses subsystem default. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|UI|Display",
		meta = (ClampMin = "0.0", ClampMax = "60.0"))
	float DefaultDisplayTimeSeconds = 3.0f;

	/** EN: When true, repeated notifications of this category coalesce into the latest entry. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|UI|Coalescing")
	bool bAllowCoalescing = false;

	/** EN: Maximum queue depth this category can occupy before older entries drop. 0 = unlimited. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|UI|Coalescing",
		meta = (ClampMin = "0", ClampMax = "32"))
	int32 MaxQueueDepth = 8;

	/** EN: When true, the notification dismisses automatically when its host UI hides. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|UI|Dismissal")
	bool bDismissOnHide = true;
};
