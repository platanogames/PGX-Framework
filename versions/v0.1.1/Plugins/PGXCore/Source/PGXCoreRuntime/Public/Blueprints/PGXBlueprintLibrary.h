// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "PGXBlueprintLibrary.generated.h"

/**
 * EN: Blueprint function library exposing common PGX operations.
 *     Provides Blueprint-friendly access to Config, Data Registry, Messages,
 *     and framework utilities.
 * ES: Libreria de funciones Blueprint exponiendo operaciones comunes de PGX.
 *     Proporciona acceso Blueprint-friendly a Config, Data Registry, Messages
 *     y utilidades del framework.
 */
UCLASS()
class PGXCORERUNTIME_API UPGXBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// EN: Check if PGX framework is fully initialized / ES: Verificar si el framework PGX esta completamente inicializado
	UFUNCTION(BlueprintPure, Category = "PGX|Core", meta = (WorldContext = "WorldContextObject"))
	static bool IsPGXReady(const UObject* WorldContextObject);

	// EN: Deprecated — use UPGXMessageBlueprintLibrary::BroadcastPGXMessage instead.
	// ES: Deprecada — usar UPGXMessageBlueprintLibrary::BroadcastPGXMessage.
	UFUNCTION(BlueprintCallable, Category = "PGX|Messages", meta = (WorldContext = "WorldContextObject",
		DeprecatedFunction, DeprecationMessage = "Use Broadcast PGX Message from PGX|Messages instead."))
	static void SendPGXMessage(const UObject* WorldContextObject, FGameplayTag MessageTag, UObject* Payload);

	// EN: Get data from the Data Registry by ID / ES: Obtener datos del Data Registry por ID
	UFUNCTION(BlueprintCallable, Category = "PGX|DataRegistry", meta = (WorldContext = "WorldContextObject"))
	static UObject* GetPGXData(const UObject* WorldContextObject, FName DataId);

	// EN: Check if data is loaded in the registry cache / ES: Verificar si los datos estan cargados en el cache del registry
	UFUNCTION(BlueprintPure, Category = "PGX|DataRegistry", meta = (WorldContext = "WorldContextObject"))
	static bool IsPGXDataLoaded(const UObject* WorldContextObject, FName DataId);

	// EN: Validate a PGX asset / ES: Validar un asset PGX
	UFUNCTION(BlueprintCallable, Category = "PGX|Validation")
	static bool ValidatePGXAsset(UObject* Asset, TArray<FText>& OutErrors);
};
