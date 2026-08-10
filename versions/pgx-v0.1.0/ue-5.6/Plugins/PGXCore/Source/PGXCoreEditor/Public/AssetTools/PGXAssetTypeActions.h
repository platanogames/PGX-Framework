// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"

/**
 * EN: Generic asset type actions for all PGX DataAsset types.
 *     Single class instantiated with different parameters per asset type.
 *     Provides name, color, supported class, and category for Content Browser.
 *
 * ES: Acciones de tipo de asset genericas para todos los tipos de DataAsset PGX.
 *     Clase unica instanciada con diferentes parametros por tipo de asset.
 *     Provee nombre, color, clase soportada y categoria para el Content Browser.
 */
class PGXCOREEDITOR_API FPGXAssetTypeActions : public FAssetTypeActions_Base
{
public:
	FPGXAssetTypeActions(FText InName, FColor InColor, UClass* InClass, EAssetTypeCategories::Type InCategory, TArray<FText> InSubMenus = {})
		: AssetName(InName), TypeColor(InColor), AssetClass(InClass), PGXCategory(InCategory), SubMenuPath(MoveTemp(InSubMenus)) {}

	FText GetName() const override { return AssetName; }
	FColor GetTypeColor() const override { return TypeColor; }
	UClass* GetSupportedClass() const override { return AssetClass; }
	uint32 GetCategories() override { return PGXCategory; }
	const TArray<FText>& GetSubMenus() const override { return SubMenuPath; }

private:
	FText AssetName;
	FColor TypeColor;
	UClass* AssetClass;
	EAssetTypeCategories::Type PGXCategory;
	TArray<FText> SubMenuPath;
};
