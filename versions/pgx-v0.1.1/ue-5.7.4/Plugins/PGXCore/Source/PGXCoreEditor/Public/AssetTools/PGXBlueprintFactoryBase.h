// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "PGXBlueprintFactoryBase.generated.h"

/**
 * EN: Base factory for creating Blueprints derived from PGX base classes.
 *     Subclasses set ParentClassPath, FactoryDisplayName, and SubMenus in their constructor.
 *     Classes are resolved at runtime via FindObject/LoadObject — no compile-time L2 dependencies.
 *     Factories with unloaded parent classes are automatically hidden from the Content Browser.
 *
 * ES: Factory base para crear Blueprints derivados de clases base PGX.
 *     Las subclases configuran ParentClassPath, FactoryDisplayName y SubMenus en su constructor.
 *     Las clases se resuelven en runtime via FindObject/LoadObject — sin dependencias L2 en compilacion.
 *     Las factories con clases padre no cargadas se ocultan automaticamente del Content Browser.
 */
UCLASS(Abstract)
class PGXCOREEDITOR_API UPGXBlueprintFactoryBase : public UFactory
{
	GENERATED_BODY()

public:
	UPGXBlueprintFactoryBase();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override;
	uint32 GetMenuCategories() const override;
	const TArray<FText>& GetMenuCategorySubMenus() const override;
	FText GetDisplayName() const override;

protected:
	// EN: Full class path (e.g. "/Script/PGXCoreRuntime.PGXActorBase")
	// ES: Path completo de clase (ej: "/Script/PGXCoreRuntime.PGXActorBase")
	FString ParentClassPath;

	// EN: Display name shown in Content Browser menu
	// ES: Nombre mostrado en el menu del Content Browser
	FText FactoryDisplayName;

	// EN: Sub-menu path in Basic section (e.g. {"PGX Framework", "Core", "Blueprints"})
	// ES: Ruta de sub-menu en seccion Basic (ej: {"PGX Framework", "Core", "Blueprints"})
	TArray<FText> SubMenus;
};
