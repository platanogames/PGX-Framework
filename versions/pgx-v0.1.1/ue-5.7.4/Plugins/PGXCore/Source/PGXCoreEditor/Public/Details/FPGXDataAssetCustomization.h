// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

/**
 * EN: Detail customization for PGX DataAssets (both Config and Object).
 *     Adds a custom header with type info and validation status,
 *     organizes properties by category with PGX-specific widgets.
 * ES: Customizacion de detalles para PGX DataAssets (tanto Config como Object).
 *     Anade un header custom con info de tipo y estado de validacion,
 *     organiza propiedades por categoria con widgets especificos de PGX.
 */
class PGXCOREEDITOR_API FPGXDataAssetCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
