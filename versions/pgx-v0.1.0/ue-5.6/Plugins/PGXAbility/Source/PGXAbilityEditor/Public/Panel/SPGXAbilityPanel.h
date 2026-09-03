// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class STextBlock;

/**
 * EN: PGX Ability observability panel. The following accessors are available on `UPGXAbilitySubsystem`:
 *     `GetActiveAbilityCount`, `GetRegisteredComponentCount`, `OnAbilityActivatedNative`,
 *     `OnComponentRegisteredNative`. `GetAttributeWatcherCount` is NOT implemented — no
 *     "attribute watcher" concept exists in the current architecture (PGXAttributeFacade is
 *     poll-based, not subscription-based).
 *
 *     This panel reads a one-shot snapshot on `Construct`/`Refresh` (button-driven), not a live
 *     reactive subscription to native delegates. Delegate-bound automatic refresh is outside
 *     this panel's read-only contract.
 *
 * ES: Panel de observabilidad PGX Ability. Los accessors indicados estan disponibles en el
 *     subsistema. `GetAttributeWatcherCount` NO esta implementado
 *     — no existe el concepto de "attribute watcher" en la arquitectura actual.
 *     Este panel lee un snapshot puntual en Construct/Refresh (boton), sin suscripcion reactiva
 *     a los delegates nativos.
 */
class PGXABILITYEDITOR_API SPGXAbilityPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXAbilityPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FReply OnRefreshClicked();
	FText GetStatusText() const;

	TSharedPtr<STextBlock> StatusTextBlock;
};
