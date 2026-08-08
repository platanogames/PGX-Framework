// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "PGXSaveTypes.h"
#include "PGXSaveSlotInfo.h"
#include "PGXSaveBlueprintLibrary.generated.h"

class UPGXSaveSubsystem;
class UPGXSaveGame;

/**
 * EN: Blueprint function library for the PGX Save system.
 *     Provides 40+ nodes wrapping the SaveSubsystem API for easy Blueprint access.
 *     Each node resolves the subsystem internally via WorldContext.
 *
 *     Categories:
 *     - PGX|Save           : Core operations (Save, Load — auto-resolve slot)
 *     - PGX|Save|Advanced  : Slot operations (SaveToSlot, LoadFromSlot, Delete, Copy, AutoSave, Versioning)
 *     - PGX|Save|Query     : Slot/context queries
 *     - PGX|Save|Data      : SaveGame access (GetSaveGame, HasSaveData, ClearDomainData)
 *     - PGX|Save|Data|Read : Typed key-value read from SaveGame
 *     - PGX|Save|Data|Write: Typed key-value write to SaveGame
 *     - PGX|Save|Debug     : In-progress state checks
 *
 * ES: Biblioteca de funciones Blueprint para el sistema PGX Save.
 *     Proporciona 40+ nodos que envuelven la API del SaveSubsystem para acceso facil desde Blueprint.
 *     Cada nodo resuelve el subsistema internamente via WorldContext.
 */
UCLASS()
class PGXSAVERUNTIME_API UPGXSaveBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ========================================================================
	// EN: Core operations (auto-resolve slot from config)
	// ES: Operaciones core (auto-resuelven slot desde config)
	// ========================================================================

	/** EN: Save all domains in a context using the configured slot / ES: Guardar todos los dominios de un contexto usando el slot configurado */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save",
		meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "CallerName"))
	static EPGXSaveResult Save(const UObject* WorldContextObject, FGameplayTag ContextTag,
		FName CallerName = NAME_None);

	/** EN: Load all domains in a context from the configured slot / ES: Cargar todos los dominios de un contexto desde el slot configurado */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save",
		meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "CallerName"))
	static EPGXSaveResult Load(const UObject* WorldContextObject, FGameplayTag ContextTag,
		FName CallerName = NAME_None);

	// ========================================================================
	// EN: Slot operations (explicit slot name)
	// ES: Operaciones de slot (nombre de slot explicito)
	// ========================================================================

	/** EN: Save all domains in a context to a specific slot / ES: Guardar todos los dominios de un contexto en un slot especifico */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save|Advanced",
		meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "CallerName"))
	static EPGXSaveResult SaveToSlot(const UObject* WorldContextObject,
		FGameplayTag ContextTag, const FString& SlotName, FName CallerName = NAME_None);

	/** EN: Load all domains in a context from a specific slot / ES: Cargar todos los dominios de un contexto desde un slot especifico */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save|Advanced",
		meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "CallerName"))
	static EPGXSaveResult LoadFromSlot(const UObject* WorldContextObject,
		FGameplayTag ContextTag, const FString& SlotName, FName CallerName = NAME_None);

	/** EN: Delete a save slot and all its files / ES: Eliminar un slot de guardado y todos sus archivos */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save|Advanced",
		meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "CallerName"))
	static EPGXSaveResult DeleteSlot(const UObject* WorldContextObject,
		FGameplayTag ContextTag, const FString& SlotName, FName CallerName = NAME_None);

	/** EN: Copy all domain files from one slot to another / ES: Copiar todos los archivos de dominio de un slot a otro */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save|Advanced",
		meta = (WorldContext = "WorldContextObject"))
	static EPGXSaveResult CopySlot(const UObject* WorldContextObject,
		FGameplayTag ContextTag, const FString& SourceSlotName, const FString& DestSlotName);

	// ========================================================================
	// EN: Slot queries
	// ES: Consultas de slots
	// ========================================================================

	/** EN: Get all save slots for a context / ES: Obtener todos los slots de guardado para un contexto */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save|Query",
		meta = (WorldContext = "WorldContextObject"))
	static TArray<FPGXSaveSlotInfo> GetAllSlots(const UObject* WorldContextObject, FGameplayTag ContextTag);

	/** EN: Get metadata for a specific slot / ES: Obtener metadata de un slot especifico */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save|Query",
		meta = (WorldContext = "WorldContextObject"))
	static FPGXSaveSlotInfo GetSlotInfo(const UObject* WorldContextObject,
		FGameplayTag ContextTag, const FString& SlotName);

	/** EN: Check if a slot exists / ES: Comprobar si un slot existe */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save|Query",
		meta = (WorldContext = "WorldContextObject"))
	static bool DoesSlotExist(const UObject* WorldContextObject,
		FGameplayTag ContextTag, const FString& SlotName);

	/** EN: Get the next available slot name / ES: Obtener el siguiente nombre de slot disponible */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save|Query",
		meta = (WorldContext = "WorldContextObject"))
	static FString GetNextAvailableSlotName(const UObject* WorldContextObject, FGameplayTag ContextTag);

	// ========================================================================
	// EN: Data|Write — typed key-value onto a domain's SaveGame
	// ES: Data|Write — key-value tipados al SaveGame de un dominio
	// ========================================================================

	/** EN: Write a string value / ES: Escribir un valor string */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save|Data|Write",
		meta = (WorldContext = "WorldContextObject"))
	static void WriteSaveDataString(const UObject* WorldContextObject,
		FGameplayTag DomainTag, FName Key, const FString& Value);

	/** EN: Write an integer value / ES: Escribir un valor entero */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save|Data|Write",
		meta = (WorldContext = "WorldContextObject"))
	static void WriteSaveDataInt(const UObject* WorldContextObject,
		FGameplayTag DomainTag, FName Key, int32 Value);

	/** EN: Write a float value / ES: Escribir un valor float */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save|Data|Write",
		meta = (WorldContext = "WorldContextObject"))
	static void WriteSaveDataFloat(const UObject* WorldContextObject,
		FGameplayTag DomainTag, FName Key, float Value);

	/** EN: Write a boolean value / ES: Escribir un valor booleano */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save|Data|Write",
		meta = (WorldContext = "WorldContextObject"))
	static void WriteSaveDataBool(const UObject* WorldContextObject,
		FGameplayTag DomainTag, FName Key, bool Value);

	/** EN: Write a vector value / ES: Escribir un valor vector */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save|Data|Write",
		meta = (WorldContext = "WorldContextObject"))
	static void WriteSaveDataVector(const UObject* WorldContextObject,
		FGameplayTag DomainTag, FName Key, FVector Value);

	/** EN: Write a rotator value / ES: Escribir un valor rotator */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save|Data|Write",
		meta = (WorldContext = "WorldContextObject"))
	static void WriteSaveDataRotator(const UObject* WorldContextObject,
		FGameplayTag DomainTag, FName Key, FRotator Value);

	/** EN: Write a transform value / ES: Escribir un valor transform */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save|Data|Write",
		meta = (WorldContext = "WorldContextObject"))
	static void WriteSaveDataTransform(const UObject* WorldContextObject,
		FGameplayTag DomainTag, FName Key, const FTransform& Value);

	/** EN: Write a gameplay tag value / ES: Escribir un valor gameplay tag */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save|Data|Write",
		meta = (WorldContext = "WorldContextObject"))
	static void WriteSaveDataTag(const UObject* WorldContextObject,
		FGameplayTag DomainTag, FName Key, FGameplayTag Value);

	// ========================================================================
	// EN: Data|Read — typed key-value from a domain's SaveGame
	// ES: Data|Read — key-value tipados desde el SaveGame de un dominio
	// ========================================================================

	/** EN: Read a string value / ES: Leer un valor string */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save|Data|Read",
		meta = (WorldContext = "WorldContextObject"))
	static FString ReadSaveDataString(const UObject* WorldContextObject,
		FGameplayTag DomainTag, FName Key, const FString& DefaultValue);

	/** EN: Read an integer value / ES: Leer un valor entero */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save|Data|Read",
		meta = (WorldContext = "WorldContextObject"))
	static int32 ReadSaveDataInt(const UObject* WorldContextObject,
		FGameplayTag DomainTag, FName Key, int32 DefaultValue = 0);

	/** EN: Read a float value / ES: Leer un valor float */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save|Data|Read",
		meta = (WorldContext = "WorldContextObject"))
	static float ReadSaveDataFloat(const UObject* WorldContextObject,
		FGameplayTag DomainTag, FName Key, float DefaultValue = 0.0f);

	/** EN: Read a boolean value / ES: Leer un valor booleano */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save|Data|Read",
		meta = (WorldContext = "WorldContextObject"))
	static bool ReadSaveDataBool(const UObject* WorldContextObject,
		FGameplayTag DomainTag, FName Key, bool DefaultValue = false);

	/** EN: Read a vector value / ES: Leer un valor vector */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save|Data|Read",
		meta = (WorldContext = "WorldContextObject"))
	static FVector ReadSaveDataVector(const UObject* WorldContextObject,
		FGameplayTag DomainTag, FName Key);

	/** EN: Read a rotator value / ES: Leer un valor rotator */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save|Data|Read",
		meta = (WorldContext = "WorldContextObject"))
	static FRotator ReadSaveDataRotator(const UObject* WorldContextObject,
		FGameplayTag DomainTag, FName Key);

	/** EN: Read a transform value / ES: Leer un valor transform */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save|Data|Read",
		meta = (WorldContext = "WorldContextObject"))
	static FTransform ReadSaveDataTransform(const UObject* WorldContextObject,
		FGameplayTag DomainTag, FName Key);

	/** EN: Read a gameplay tag value / ES: Leer un valor gameplay tag */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save|Data|Read",
		meta = (WorldContext = "WorldContextObject"))
	static FGameplayTag ReadSaveDataTag(const UObject* WorldContextObject,
		FGameplayTag DomainTag, FName Key);

	// ========================================================================
	// EN: Data utility
	// ES: Utilidad de datos
	// ========================================================================

	/** EN: Check if a key exists in any typed map / ES: Comprobar si una key existe en algun mapa tipado */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save|Data",
		meta = (WorldContext = "WorldContextObject"))
	static bool HasSaveData(const UObject* WorldContextObject, FGameplayTag DomainTag, FName Key);

	/** EN: Clear all key-value data in a domain / ES: Limpiar todos los datos key-value en un dominio */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save|Data",
		meta = (WorldContext = "WorldContextObject"))
	static void ClearDomainData(const UObject* WorldContextObject, FGameplayTag DomainTag);

	/** EN: Get the active SaveGame instance for a domain / ES: Obtener la instancia activa de SaveGame para un dominio */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save|Data",
		meta = (WorldContext = "WorldContextObject"))
	static UPGXSaveGame* GetSaveGame(const UObject* WorldContextObject, FGameplayTag DomainTag);

	/**
	 * EN: Get the active SaveGame cast to a specific subclass — Blueprint node pin shows the expected class.
	 * ES: Obtener el SaveGame activo cast a una subclase especifica — el pin del nodo Blueprint muestra la clase esperada.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save|Data",
		meta = (WorldContext = "WorldContextObject", DeterminesOutputType = "ExpectedClass",
			DisplayName = "Get Save Game (Typed)"))
	static UPGXSaveGame* GetSaveGameTyped(const UObject* WorldContextObject,
		FGameplayTag DomainTag, TSubclassOf<UPGXSaveGame> ExpectedClass);

	// ========================================================================
	// EN: Auto-save control
	// ES: Control de auto-guardado
	// ========================================================================

	/** EN: Enable or disable auto-save for a context / ES: Habilitar o deshabilitar auto-guardado para un contexto */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save|Advanced",
		meta = (WorldContext = "WorldContextObject"))
	static void SetAutoSaveEnabled(const UObject* WorldContextObject,
		FGameplayTag ContextTag, bool bEnabled);

	/** EN: Manually trigger auto-save / ES: Disparar manualmente auto-guardado */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save|Advanced",
		meta = (WorldContext = "WorldContextObject"))
	static void TriggerAutoSave(const UObject* WorldContextObject, FGameplayTag ContextTag);

	/** EN: Check if auto-save timer is active / ES: Comprobar si el timer de auto-guardado esta activo */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save|Advanced",
		meta = (WorldContext = "WorldContextObject"))
	static bool IsAutoSaveActive(const UObject* WorldContextObject, FGameplayTag ContextTag);

	// ========================================================================
	// EN: Active state
	// ES: Estado activo
	// ========================================================================

	/** EN: Get the currently active slot name / ES: Obtener el nombre del slot activo actual */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save|Query",
		meta = (WorldContext = "WorldContextObject"))
	static FString GetActiveSlotName(const UObject* WorldContextObject, FGameplayTag ContextTag);

	/** EN: Set the active slot (does NOT trigger load) / ES: Establecer el slot activo (NO dispara carga) */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save|Advanced",
		meta = (WorldContext = "WorldContextObject"))
	static void SetActiveSlot(const UObject* WorldContextObject,
		FGameplayTag ContextTag, const FString& SlotName);

	/** EN: Is a save operation in progress? / ES: Hay una operacion de guardado en progreso? */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save|Debug",
		meta = (WorldContext = "WorldContextObject"))
	static bool IsSaveInProgress(const UObject* WorldContextObject);

	/** EN: Is a load operation in progress? / ES: Hay una operacion de carga en progreso? */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save|Debug",
		meta = (WorldContext = "WorldContextObject"))
	static bool IsLoadInProgress(const UObject* WorldContextObject);

	// ========================================================================
	// EN: Context queries
	// ES: Consultas de contexto
	// ========================================================================

	/** EN: Get all discovered context tags / ES: Obtener todos los tags de contexto descubiertos */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save|Query",
		meta = (WorldContext = "WorldContextObject"))
	static TArray<FGameplayTag> GetAllContextTags(const UObject* WorldContextObject);

	/** EN: Get the number of registered save contexts / ES: Obtener el numero de contextos de guardado registrados */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save|Query",
		meta = (WorldContext = "WorldContextObject"))
	static int32 GetContextCount(const UObject* WorldContextObject);

	// ========================================================================
	// EN: Versioning queries
	// ES: Consultas de versionado
	// ========================================================================

	/** EN: Get the format version of a loaded SaveGame (0 if not loaded) / ES: Obtener la version de formato de un SaveGame cargado (0 si no esta cargado) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save|Advanced",
		meta = (WorldContext = "WorldContextObject"))
	static int32 GetSaveFormatVersion(const UObject* WorldContextObject, FGameplayTag DomainTag);

	/** EN: Get the current save format version from a context's config / ES: Obtener la version actual de formato de guardado desde la config de un contexto */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save|Advanced",
		meta = (WorldContext = "WorldContextObject"))
	static int32 GetCurrentSaveVersion(const UObject* WorldContextObject, FGameplayTag ContextTag);

	/** EN: Check if a migration path exists between two versions / ES: Comprobar si existe un camino de migracion entre dos versiones */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save|Advanced")
	static bool HasMigrationPath(int32 FromVersion, int32 ToVersion);

	/** EN: Get the number of registered migration steps / ES: Obtener el numero de pasos de migracion registrados */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save|Advanced")
	static int32 GetMigrationCount();

private:
	// ========================================================================
	// EN: Internal helper
	// ES: Helper interno
	// ========================================================================

	/** EN: Resolve the SaveSubsystem from a WorldContext / ES: Resolver el SaveSubsystem desde un WorldContext */
	static UPGXSaveSubsystem* GetSaveSubsystem(const UObject* WorldContextObject);
};
