// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXDataAsset.h"
#include "GameplayTagContainer.h"
#include "Observability/PGXObservable.h"
#include "PGXItemDefinition.generated.h"

class AActor;
class UTexture2D;

/**
 * EN: Data asset defining an item type with properties.
 *     Contains static data shared by all instances of this item type.
 *
 * ES: Data asset que define un tipo de item con propiedades.
 *     Contiene datos estaticos compartidos por todas las instancias de este tipo de item.
 */
UCLASS(BlueprintType)
class PGXINVENTORYRUNTIME_API UPGXItemDefinition : public UPGXDataAsset, public IPGXObservable
{
	GENERATED_BODY()

public:
	static const FName SchemaVersion;

	//~ Begin IPGXObservable
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;
	//~ End IPGXObservable

	/** EN: GameplayTag identity for this item family / ES: Identidad GameplayTag de esta familia de item */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Inventory", meta = (Categories = "PGX.Inventory.Item"))
	FGameplayTag ItemTag;

	/** EN: Category tags used by filters/capacity policies / ES: Tags de categoria usadas por filtros/politicas de capacidad */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Inventory", meta = (Categories = "PGX.Inventory.Category"))
	FGameplayTagContainer CategoryTags;

	/** EN: Internal item name identifier retained for migration/readability / ES: Identificador interno conservado para migracion/legibilidad */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Inventory")
	FName ItemName;

	/** EN: Localized display name shown to the player / ES: Nombre localizado mostrado al jugador */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Inventory")
	FText DisplayName;

	/** EN: Localized item description / ES: Descripcion localizada del item */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Inventory")
	FText Description;

	/** EN: Item icon texture (soft reference for async loading) / ES: Textura de icono del item (referencia suave para carga asincrona) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Inventory")
	TSoftObjectPtr<UTexture2D> Icon;

	/** EN: Maximum number of items per stack / ES: Numero maximo de items por stack */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Inventory", meta = (ClampMin = "1"))
	int32 MaxStackSize = 1;

	/** EN: Weight per unit of this item / ES: Peso por unidad de este item */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Inventory", meta = (ClampMin = "0.0"))
	float Weight = 0.0f;

	/** EN: Require a valid actor instigator before OnUse succeeds / ES: Requerir actor instigador valido para que OnUse tenga exito */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Inventory|Use")
	bool bRequiresInstigatorForUse = false;

	/** EN: Extensible use hook for item definitions / ES: Hook extensible de uso para definiciones de item */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PGX|Inventory")
	bool OnUse(AActor* Instigator) const;
};

/** EN: Weapon-specific item definition / ES: Definicion de item especializada para armas */
UCLASS(BlueprintType)
class PGXINVENTORYRUNTIME_API UPGXWeaponItemDefinition : public UPGXItemDefinition
{
	GENERATED_BODY()

public:
	/** EN: Gameplay tag for weapon family / ES: GameplayTag de familia de arma */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Inventory|Weapon", meta = (Categories = "PGX.Inventory.Weapon"))
	FGameplayTag WeaponTag;

	/** EN: Baseline damage value for template logic / ES: Valor base de dano para logica de plantilla */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Inventory|Weapon", meta = (ClampMin = "0.0"))
	float BaseDamage = 0.0f;

	/** EN: Effective range in Unreal units / ES: Alcance efectivo en unidades Unreal */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Inventory|Weapon", meta = (ClampMin = "0.0"))
	float EffectiveRange = 0.0f;
};

/** EN: Consumable-specific item definition / ES: Definicion de item especializada para consumibles */
UCLASS(BlueprintType)
class PGXINVENTORYRUNTIME_API UPGXConsumableItemDefinition : public UPGXItemDefinition
{
	GENERATED_BODY()

public:
	/** EN: Gameplay tag for the authored use effect / ES: GameplayTag del efecto autorado de uso */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Inventory|Consumable", meta = (Categories = "PGX.Inventory.Effect"))
	FGameplayTag UseEffectTag;

	/** EN: Whether using this item consumes one stack unit / ES: Si usar este item consume una unidad del stack */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Inventory|Consumable")
	bool bConsumeOnUse = true;

	/** EN: Cooldown after use in seconds / ES: Enfriamiento tras uso en segundos */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Inventory|Consumable", meta = (ClampMin = "0.0"))
	float CooldownSeconds = 0.0f;
};
