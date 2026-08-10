// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PGXConstructionTypes.generated.h"

/**
 * EN: Declares how the user provides the class for a construction slot.
 *     The DA (construction DataAsset) is always available regardless of class source.
 * ES: Declara como el usuario provee la clase para un slot de construccion.
 *     El DA (DataAsset de construccion) siempre esta disponible independiente del class source.
 */
UENUM(BlueprintType)
enum class EPGXClassSourceMode : uint8
{
	/** EN: Use PGX base class — no custom override / ES: Usar clase base PGX — sin override */
	Default     UMETA(DisplayName = "Default (PGX Base)"),

	/** EN: User provides a C++ class derived from PGX base / ES: Usuario provee clase C++ derivada de base PGX */
	CppClass    UMETA(DisplayName = "C++ Class"),

	/** EN: User provides a Blueprint class derived from PGX base / ES: Usuario provee clase BP derivada de base PGX */
	Blueprint   UMETA(DisplayName = "Blueprint Class")
};

/**
 * EN: Describes a component to inject into an actor during construction.
 *     Supports conditional injection based on Profile capabilities and
 *     network relevance filtering.
 * ES: Describe un componente a inyectar en un actor durante la construccion.
 *     Soporta inyeccion condicional basada en capabilities de Profile y
 *     filtrado de relevancia de red.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXComponentInjection
{
	GENERATED_BODY()

	/** EN: Component class to inject / ES: Clase de componente a inyectar */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Construction")
	TSoftClassPtr<UActorComponent> ComponentClass;

	/** EN: Optional custom name for the component / ES: Nombre personalizado opcional */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Construction")
	FName ComponentName;

	/** EN: Only inject if this capability is enabled in Profile / ES: Solo inyectar si esta capability esta habilitada en Profile */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Construction", meta = (Categories = "PGX.Profile.Capability"))
	FGameplayTag RequiredCapability;

	/** EN: Only inject on clients / ES: Solo inyectar en clientes */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Construction")
	bool bClientOnly = false;

	/** EN: Only inject on server / ES: Solo inyectar en servidor */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Construction")
	bool bServerOnly = false;
};

/**
 * EN: Entry describing a HUD layer to create during construction.
 * ES: Entrada que describe una capa de HUD a crear durante la construccion.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXHUDLayerEntry
{
	GENERATED_BODY()

	/** EN: Widget class to create / ES: Clase de widget a crear */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Construction")
	TSoftClassPtr<UUserWidget> WidgetClass;

	/** EN: Layer Z-order / ES: Orden Z de la capa */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Construction")
	int32 ZOrder = 0;

	/** EN: Display tag for identification / ES: Tag de display para identificacion */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Construction")
	FGameplayTag LayerTag;
};
