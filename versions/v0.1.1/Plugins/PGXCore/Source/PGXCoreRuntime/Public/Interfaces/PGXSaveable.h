// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "PGXSaveable.generated.h"

class UPGXSaveGame;

/**
 * EN: Contract for objects that participate in the save/load lifecycle.
 *     Objects prepare their state before save and restore it after load.
 *     The actual data lives in UPGXSaveGame subclasses (as UPROPERTYs or
 *     key-value entries), not serialized directly via FArchive.
 *
 *     Usage:
 *     - Implement OnPreSave() to push your runtime state into the SaveGame
 *     - Implement OnPostLoad() to pull your state back from the SaveGame
 *     - Implement GetSaveDomainTag() to declare which domain you belong to
 *     - Register with SaveSubsystem->RegisterSaveable(this, DomainTag)
 *
 * ES: Contrato para objetos que participan en el ciclo de vida de save/load.
 *     Los objetos preparan su estado antes de guardar y lo restauran despues
 *     de cargar. Los datos reales viven en subclases de UPGXSaveGame (como
 *     UPROPERTYs o entradas key-value), no serializados directamente via FArchive.
 *
 *     Uso:
 *     - Implementar OnPreSave() para empujar tu estado runtime al SaveGame
 *     - Implementar OnPostLoad() para extraer tu estado del SaveGame
 *     - Implementar GetSaveDomainTag() para declarar a que dominio perteneces
 *     - Registrarse con SaveSubsystem->RegisterSaveable(this, DomainTag)
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UPGXSaveable : public UInterface
{
	GENERATED_BODY()
};

class PGXCORERUNTIME_API IPGXSaveable
{
	GENERATED_BODY()

public:
	/**
	 * EN: Called before the save operation. Push your runtime state into the SaveGame.
	 * ES: Llamado antes de la operacion de guardado. Empuja tu estado runtime al SaveGame.
	 *
	 * @param SaveGame   EN: The SaveGame instance to write data into / ES: La instancia de SaveGame donde escribir datos
	 * @param DomainTag  EN: The domain being saved / ES: El dominio que se esta guardando
	 */
	virtual void OnPreSave(UPGXSaveGame* SaveGame, FGameplayTag DomainTag) = 0;

	/**
	 * EN: Called after load completes. Pull your state from the SaveGame.
	 * ES: Llamado despues de completar la carga. Extrae tu estado del SaveGame.
	 *
	 * @param SaveGame   EN: The SaveGame instance to read data from / ES: La instancia de SaveGame de donde leer datos
	 * @param DomainTag  EN: The domain that was loaded / ES: El dominio que fue cargado
	 */
	virtual void OnPostLoad(UPGXSaveGame* SaveGame, FGameplayTag DomainTag) = 0;

	/**
	 * EN: Returns the domain tag this saveable belongs to.
	 *     Used by the subsystem to match saveables with their SaveGame instances.
	 * ES: Retorna el tag de dominio al que pertenece este saveable.
	 *     Usado por el subsistema para emparejar saveables con sus instancias de SaveGame.
	 */
	virtual FGameplayTag GetSaveDomainTag() const = 0;
};
