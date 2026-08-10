// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

/**
 * EN: Detail customization for PGX actors in the viewport.
 *     Adds a prominent "PGX Data" section at the top showing database reference,
 *     row name, load status, and quick action buttons.
 * ES: Customizacion de detalles para actores PGX en el viewport.
 *     Anade una seccion "PGX Data" prominente arriba mostrando referencia a database,
 *     row name, estado de carga y botones de accion rapida.
 */
class PGXCOREEDITOR_API FPGXActorCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
