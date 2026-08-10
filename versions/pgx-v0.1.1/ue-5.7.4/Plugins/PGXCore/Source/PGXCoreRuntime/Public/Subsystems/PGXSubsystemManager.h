// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "PGXSubsystemManager.generated.h"

/**
 * EN: Central orchestrator for all PGX subsystems. Manages initialization order,
 *     dependency resolution, and provides a unified access point for subsystem queries.
 *
 *     Static template helpers GetWorldSubsystem<T>(World) and
 *     GetGameInstanceSubsystem<T>(GI) that wrap the UE 5.x templated
 *     GetSubsystem<T>() with explicit container + null-check semantics.
 *     Replaces the 18+ duplicated 'World->GI->GetSubsystem<T>() + null-check'
 *     call-sites scattered across the participating plugins.
 *
 * ES: Orquestador central de todos los subsistemas PGX. Gestiona orden de inicializacion,
 *     resolucion de dependencias, y proporciona un punto de acceso unificado para consultas de subsistemas.
 *
 *     Helpers estaticos templated GetWorldSubsystem<T>(World) y
 *     GetGameInstanceSubsystem<T>(GI) que envuelven el templated GetSubsystem<T>()
 *     de UE 5.x con semantica explicita de container + null-check.
 */
UCLASS()
class PGXCORERUNTIME_API UPGXSubsystemManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface

	/**
	 * EN: Resolve a UWorldSubsystem (or subclass) from a UWorld context, with
	 *     null-check on both the World and the subsystem. Returns nullptr on
	 *     any failure (does not crash).
	 *
	 *     Replaces the duplicated pattern:
	 *       if (World) {
	 *           if (auto* GI = World->GetGameInstance()) {
	 *               if (auto* Sub = GI->GetSubsystem<T>()) {
	 *                   // use Sub
	 *               }
	 *           }
	 *       }
	 *
	 * ES: Resuelve un UWorldSubsystem (o subclase) desde un UWorld, con
	 *     null-check en World y subsystem. Retorna nullptr en cualquier fallo.
	 */
	template<typename TSubsystemClass>
	static TSubsystemClass* GetWorldSubsystem(UWorld* World)
	{
		if (!World)
		{
			return nullptr;
		}
		return World->GetSubsystem<TSubsystemClass>();
	}

	/**
	 * EN: Resolve a UGameInstanceSubsystem (or subclass) from a UGameInstance
	 *     context, with null-check on both. Returns nullptr on any failure.
	 *
	 *     Replaces the duplicated pattern:
	 *       if (GI) {
	 *           if (auto* Sub = GI->GetSubsystem<T>()) { ... }
	 *       }
	 *
	 * ES: Resuelve un UGameInstanceSubsystem (o subclase) desde un UGameInstance,
	 *     con null-check en ambos. Retorna nullptr en cualquier fallo.
	 */
	template<typename TSubsystemClass>
	static TSubsystemClass* GetGameInstanceSubsystem(UGameInstance* GameInstance)
	{
		if (!GameInstance)
		{
			return nullptr;
		}
		return GameInstance->GetSubsystem<TSubsystemClass>();
	}

	/**
	 * EN: Convenience: World -> GameInstance -> Subsystem (one-shot). Same
	 *     null-check guarantees as the individual methods above. Returns
	 *     nullptr on any failure.
	 *
	 * ES: Conveniencia: World -> GameInstance -> Subsystem (one-shot). Mismas
	 *     garantias de null-check que los metodos individuales.
	 */
	template<typename TSubsystemClass>
	static TSubsystemClass* GetGISubsystem(UWorld* World)
	{
		UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
		return GetGameInstanceSubsystem<TSubsystemClass>(GI);
	}

	/**
	 * EN: Resolve a UGameInstanceSubsystem from any UObject-derived WorldContext.
	 *     Calls GetWorld() → GetGameInstance() → GetSubsystem<T>() with null
	 *     safety at each step. Returns nullptr on any null.
	 *
	 *     Use this in BlueprintLibrary functions or any UObject method that
	 *     receives a WorldContextObject parameter — no extra boilerplate.
	 *
	 *     This complements FPGXSubsystemResolver::GetFromContext<T>() with a
	 *     WorldContext entry point on UPGXSubsystemManager.
	 *
	 * ES: Resuelve un UGameInstanceSubsystem desde cualquier WorldContext UObject.
	 *     Null-safe en cada paso. Usar en funciones BlueprintLibrary o metodos
	 *     UObject que reciben un WorldContextObject.
	 */
	template<typename TSubsystemClass>
	static TSubsystemClass* GetSubsystemFromContext(const UObject* WorldContext)
	{
		if (!WorldContext)
		{
			return nullptr;
		}
		UWorld* World = WorldContext->GetWorld();
		return GetGISubsystem<TSubsystemClass>(World);
	}
};
