// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "AssetTools/PGXBlueprintFactoryBase.h"
#include "PGXCoreEditor.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Engine/Blueprint.h"

#define LOCTEXT_NAMESPACE "PGXBlueprintFactory"

UPGXBlueprintFactoryBase::UPGXBlueprintFactoryBase()
{
	SupportedClass = UBlueprint::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UPGXBlueprintFactoryBase::FactoryCreateNew(
	UClass* /*InClass*/, UObject* InParent, FName InName,
	EObjectFlags /*Flags*/, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	// EN: Resolve the parent class at runtime (avoids compile-time L2 dependency)
	// ES: Resolver la clase padre en runtime (evita dependencia L2 en compilacion)
	UClass* ParentClass = LoadObject<UClass>(nullptr, *ParentClassPath);
	if (!ParentClass)
	{
		UE_LOG(LogPGXCoreEditor, Error, TEXT("PGXBlueprintFactory: Could not load class '%s'"), *ParentClassPath);
		return nullptr;
	}

	return FKismetEditorUtilities::CreateBlueprint(
		ParentClass, InParent, InName, BPTYPE_Normal,
		UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
}

bool UPGXBlueprintFactoryBase::ShouldShowInNewMenu() const
{
	// EN: Hidden from automatic menu — PGX entries are shown via custom UToolMenus section
	//     (see FPGXContentBrowserExtension). Factories are still usable for programmatic creation.
	// ES: Oculto del menu automatico — entradas PGX se muestran via seccion UToolMenus custom
	//     (ver FPGXContentBrowserExtension). Las factories aun se pueden usar para creacion programatica.
	return false;
}

uint32 UPGXBlueprintFactoryBase::GetMenuCategories() const
{
	// EN: Place in the top Basic (CREATE) section — PGX Framework submenu nesting is provided via SubMenus
	// ES: Ubica en la seccion Basic (CREATE) superior — anidamiento de submenu PGX Framework via SubMenus
	return EAssetTypeCategories::Basic;
}

const TArray<FText>& UPGXBlueprintFactoryBase::GetMenuCategorySubMenus() const
{
	return SubMenus;
}

FText UPGXBlueprintFactoryBase::GetDisplayName() const
{
	return FactoryDisplayName;
}

#undef LOCTEXT_NAMESPACE
