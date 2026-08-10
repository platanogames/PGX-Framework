// Copyright PGX Framework. All Rights Reserved.
// Blueprint facade — BlueprintLibrary with static accessors for PGXSpawn.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PGXSpawnTypes.h"
#include "UPGXSpawnBlueprintLibrary.generated.h"

class UPGXSpawnSubsystem;

/**
 * EN: Static accessors and factory functions for the PGXSpawn system.
 *     Provides BP-friendly helpers that don't require holding a UPGXSpawnSubsystem reference.
 * ES: Accesores estaticos y funciones factory para el sistema PGXSpawn.
 *     Provee helpers BP-friendly que no requieren tener referencia a UPGXSpawnSubsystem.
 */
UCLASS()
class PGXSPAWNRUNTIME_API UPGXSpawnBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * EN: Retrieve the UPGXSpawnSubsystem for a given world context. Returns nullptr if no World or no subsystem.
	 * ES: Recupera el UPGXSpawnSubsystem para un contexto de mundo. Retorna nullptr si no hay World o subsystem.
	 */
	UFUNCTION(BlueprintPure, Category = "PGX|Spawn|Query", meta = (WorldContext = "WorldContext"))
	static UPGXSpawnSubsystem* GetSpawnSubsystem(const UObject* WorldContext);

	/**
	 * EN: Quick validity check (non-subsystem): SpawnClass non-null, concrete actor class, Transform not NaN/zero-scale.
	 *     For full validation (including budget + conditions), use UPGXSpawnSubsystem::ValidateSpawnRequest.
	 * ES: Check de validez rapido (sin subsystem): SpawnClass no-null, clase concreta, Transform sin NaN/escala-cero.
	 *     Para validacion completa (incluyendo budget + conditions), usa UPGXSpawnSubsystem::ValidateSpawnRequest.
	 */
	UFUNCTION(BlueprintPure, Category = "PGX|Spawn|Query")
	static bool IsValidSpawnRequest(const FPGXSpawnRequest& Request);

	/**
	 * EN: Factory for FPGXSpawnRequest. Convenience for BP graph construction.
	 * ES: Factory para FPGXSpawnRequest. Conveniencia para construccion en BP graph.
	 */
	UFUNCTION(BlueprintPure, Category = "PGX|Spawn|Build")
	static FPGXSpawnRequest MakeSpawnRequest(TSubclassOf<AActor> Class, FTransform Transform, FGameplayTag SourceTag, int32 Priority);

	/**
	 * EN: Factory for FPGXSpawnClassEntry (used in wave weighted-class selection).
	 * ES: Factory para FPGXSpawnClassEntry (usado en seleccion ponderada de clases de oleada).
	 */
	UFUNCTION(BlueprintPure, Category = "PGX|Spawn|Build")
	static FPGXSpawnClassEntry MakeSpawnClassEntry(TSubclassOf<AActor> Class, float Weight, int32 MinCount, int32 MaxCount);

	/**
	 * EN: Convert EPGXSpawnResultCode to human-readable string.
	 * ES: Convierte EPGXSpawnResultCode a string legible.
	 */
	UFUNCTION(BlueprintPure, Category = "PGX|Spawn|Result")
	static FString ResultCodeToString(EPGXSpawnResultCode Code);

	/**
	 * EN: Convert EPGXSpawnRequestStatus to human-readable string.
	 * ES: Convierte EPGXSpawnRequestStatus a string legible.
	 */
	UFUNCTION(BlueprintPure, Category = "PGX|Spawn|Result")
	static FString RequestStatusToString(EPGXSpawnRequestStatus Status);

	/**
	 * EN: Filter a list of spawn records by status.
	 * ES: Filtra una lista de records de spawn por status.
	 */
	UFUNCTION(BlueprintPure, Category = "PGX|Spawn|Filter")
	static TArray<FPGXSpawnRecord> FilterRecordsByStatus(const TArray<FPGXSpawnRecord>& Records, EPGXSpawnRequestStatus Status);
};
