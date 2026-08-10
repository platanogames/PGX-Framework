// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Data/PGXConfigDataAsset.h"
#include "Construction/PGXConstructionTypes.h"
#include "GameplayTagContainer.h"
#include "PGXClassConstruction.generated.h"

/**
 * EN: Abstract base for ALL construction DataAssets. Defines components to inject,
 *     system configs to apply, and gameplay tags to grant. Extensible by inheritance
 *     (PGX base -> Vertical -> Studio).
 * ES: Base abstracta para TODOS los DataAssets de construccion. Define componentes a
 *     inyectar, configs de sistema a aplicar, y tags de gameplay a otorgar. Extensible
 *     por herencia (PGX base -> Vertical -> Studio).
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class PGXCORERUNTIME_API UPGXClassConstruction : public UPGXConfigDataAsset
{
	GENERATED_BODY()

public:
	// ─── Component Injection ───

	/** EN: Components to inject during construction / ES: Componentes a inyectar durante la construccion */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Construction")
	TArray<FPGXComponentInjection> Components;

	/** EN: Required components that must already exist / ES: Componentes requeridos que ya deben existir */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Construction", meta = (AdvancedDisplay))
	TArray<TSoftClassPtr<UActorComponent>> RequiredComponents;

	// ─── System Configs ───

	/** EN: System configuration assets to apply (keyed by system tag) / ES: Assets de config de sistema a aplicar */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Construction", meta = (AdvancedDisplay, Categories = "PGX.System"))
	TMap<FGameplayTag, TSoftObjectPtr<UPGXConfigDataAsset>> SystemConfigs;

	// ─── Tags ───

	/** EN: Gameplay tags to grant to the constructed actor / ES: Tags de gameplay a otorgar al actor construido */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Construction")
	FGameplayTagContainer GrantedTags;

	// ─── Save Integration ───

	/** EN: Enable save system participation for the constructed actor / ES: Habilitar participacion en save */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save", meta = (AdvancedDisplay))
	bool bParticipateInSave = false;

	/** EN: Save domain for this actor type / ES: Dominio de save para este tipo de actor */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save",
		meta = (AdvancedDisplay, EditCondition = "bParticipateInSave", Categories = "PGX.Save.Domain"))
	FGameplayTag SaveDomain;

	// ─── Bridge Subscriptions ───

	/** EN: Subscribe to GameFlow state changes via Message bus / ES: Suscribirse a cambios de GameFlow */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Bridge", meta = (AdvancedDisplay))
	bool bListenToGameFlowChanges = false;

	/** EN: Subscribe to LevelFlow transition notifications / ES: Suscribirse a transiciones de LevelFlow */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Bridge", meta = (AdvancedDisplay))
	bool bListenToLevelTransitions = false;

	/** EN: Subscribe to Loading screen state changes / ES: Suscribirse a cambios de pantalla de carga */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Bridge", meta = (AdvancedDisplay))
	bool bListenToLoadingScreenState = false;

	// ─── Validation ───

	/**
	 * EN: Validate the construction DA. Override in subclasses for type-specific validation.
	 * ES: Validar el DA de construccion. Override en subclases para validacion especifica.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Construction")
	virtual bool Validate(TArray<FText>& OutErrors) const;

	/**
	 * EN: Get a display name for this construction DA.
	 * ES: Obtener un nombre para mostrar de este DA de construccion.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Construction")
	virtual FText GetConstructionDisplayName() const;
};
