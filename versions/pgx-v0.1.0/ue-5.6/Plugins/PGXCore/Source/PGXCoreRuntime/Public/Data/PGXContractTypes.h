// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXContractTypes.generated.h"

// EN: Widget slot contract types — Displayed read-only in DA Details panels.
//     When a DA requires a UMG Widget, these structs describe the expected named slots
//     so the developer knows exactly which widget names to use in their Widget Blueprint.
//     Zero documentation needed — the contract IS the documentation.
//
// ES: Tipos de contrato de slots de Widget — Se muestran read-only en los paneles Details del DA.
//     Cuando un DA requiere un Widget UMG, estos structs describen los slots con nombre esperados
//     para que el desarrollador sepa exactamente que nombres de widget usar en su Widget Blueprint.
//     Zero documentacion necesaria — el contrato ES la documentacion.

/**
 * EN: Describes a single expected widget slot (name binding) in a UMG Widget.
 * ES: Describe un slot de widget esperado (name binding) en un Widget UMG.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXWidgetSlotInfo
{
	GENERATED_BODY()

	/** EN: Slot name — must match the widget's BindWidget name / ES: Nombre del slot — debe coincidir con el BindWidget del widget */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Contract")
	FName SlotName;

	/** EN: Human-readable description / ES: Descripcion legible */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Contract")
	FText Description;

	/** EN: Expected UMG widget class name (e.g. "UImage", "UProgressBar") / ES: Nombre de clase UMG esperada */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Contract")
	FName WidgetType;

	/** EN: Whether this slot is required (true) or optional (false) / ES: Si el slot es requerido u opcional */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Contract")
	bool bRequired = false;

	FPGXWidgetSlotInfo() = default;

	FPGXWidgetSlotInfo(FName InSlotName, const FText& InDescription, FName InWidgetType, bool bInRequired)
		: SlotName(InSlotName), Description(InDescription), WidgetType(InWidgetType), bRequired(bInRequired)
	{
	}
};

/**
 * EN: Describes the complete widget contract — all expected slots for a DA that requires a UMG Widget.
 *     Embed this in a DA as VisibleAnywhere so the dev sees the expected slot names in the Details panel.
 * ES: Describe el contrato completo de widget — todos los slots esperados para un DA que requiere un Widget UMG.
 *     Embeder esto en un DA como VisibleAnywhere para que el dev vea los nombres de slots en el panel Details.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXWidgetContract
{
	GENERATED_BODY()

	/** EN: All expected widget slots / ES: Todos los slots de widget esperados */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Contract")
	TArray<FPGXWidgetSlotInfo> ExpectedSlots;
};
