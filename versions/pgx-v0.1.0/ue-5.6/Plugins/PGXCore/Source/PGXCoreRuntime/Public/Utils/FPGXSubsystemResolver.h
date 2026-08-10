// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "UObject/Class.h"

/**
 * EN: Null-safe subsystem resolver for UGameInstanceSubsystem and
 *     UWorldSubsystem types. Replaces the 94+ duplicated GetSubsystem<T>()
 *     patterns across all PGX plugins while preserving deterministic lookup order.
 *
 *     Typical usage — before (94+ occurrences across 15 plugins):
 *
 *         UPGXAbilitySubsystem* Sub = GameInstance
 *             ? GameInstance->GetSubsystem<UPGXAbilitySubsystem>()
 *             : nullptr;
 *
 *         // or with WorldContext:
 *         UWorld* World = GEngine->GetWorldFromContextObject(Context);
 *         UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
 *         UPGXAbilitySubsystem* Sub = GI
 *             ? GI->GetSubsystem<UPGXAbilitySubsystem>()
 *             : nullptr;
 *
 *     After — one call, no null-chain boilerplate:
 *
 *         UPGXAbilitySubsystem* Sub =
 *             FPGXSubsystemResolver::GetGISubsystem<UPGXAbilitySubsystem>(World);
 *
 *         // from any UObject-derived WorldContext:
 *         UPGXAbilitySubsystem* Sub =
 *             FPGXSubsystemResolver::GetFromContext<UPGXAbilitySubsystem>(this);
 *
 *     Null safety: every public method returns nullptr if any intermediate
 *     pointer in the chain (World→GameInstance→Subsystem) is null.
 *     Use the Checked() variant if a missing subsystem is a logic error
 *     (asserts, not recommended for production paths).
 *
 * ES: Resolvedor null-safe de subsistemas para tipos UGameInstanceSubsystem
 *     y UWorldSubsystem. Reemplaza los 94+ patrones GetSubsystem<T>()
 *     duplicados en todos los plugins PGX.
 */
class PGXCORERUNTIME_API FPGXSubsystemResolver
{
public:
	// =========================================================================
	// EN: UGameInstanceSubsystem resolver
	// ES: Resolvedor de UGameInstanceSubsystem
	// =========================================================================

	/**
	 * EN: Resolve T* from UWorld (World → GameInstance → Subsystem).
	 *     Returns nullptr if World, GameInstance, or Subsystem is null.
	 *
	 * ES: Resuelve T* desde UWorld (World → GameInstance → Subsystem).
	 *     Retorna nullptr si World, GameInstance o Subsystem es null.
	 */
	template <typename T>
	static T* GetGISubsystem(const UWorld* World)
	{
		static_assert(TIsDerivedFrom<T, UGameInstanceSubsystem>::Value,
			"T must derive from UGameInstanceSubsystem");
		if (!World)
		{
			return nullptr;
		}
		UGameInstance* GI = World->GetGameInstance();
		return GI ? GI->GetSubsystem<T>() : nullptr;
	}

	/**
	 * EN: Resolve T* from UGameInstance. Returns nullptr if either
	 *     GameInstance or the Subsystem is null.
	 *
	 * ES: Resuelve T* desde UGameInstance. Retorna nullptr si
	 *     GameInstance o el Subsystem es null.
	 */
	template <typename T>
	static T* GetGISubsystem(UGameInstance* GameInstance)
	{
		static_assert(TIsDerivedFrom<T, UGameInstanceSubsystem>::Value,
			"T must derive from UGameInstanceSubsystem");
		return GameInstance ? GameInstance->GetSubsystem<T>() : nullptr;
	}

	/**
	 * EN: Resolve T* from UWorld, asserting if World, GameInstance, or
	 *     Subsystem is null. Use only when the subsystem is guaranteed to
	 *     exist (initialization paths, not on hot frames).
	 *
	 * ES: Resuelve T* desde UWorld con assert si World, GameInstance o
	 *     Subsystem es null. Usar solo cuando el subsystem existe seguro.
	 */
	template <typename T>
	static T& GetGISubsystemChecked(const UWorld* World)
	{
		static_assert(TIsDerivedFrom<T, UGameInstanceSubsystem>::Value,
			"T must derive from UGameInstanceSubsystem");
		check(World);
		UGameInstance* GI = World->GetGameInstance();
		check(GI);
		T* Sub = GI->GetSubsystem<T>();
		check(Sub);
		return *Sub;
	}

	/**
	 * EN: Resolve T* from UGameInstance, asserting if either is null.
	 *
	 * ES: Resuelve T* desde UGameInstance con assert si es null.
	 */
	template <typename T>
	static T& GetGISubsystemChecked(UGameInstance* GameInstance)
	{
		static_assert(TIsDerivedFrom<T, UGameInstanceSubsystem>::Value,
			"T must derive from UGameInstanceSubsystem");
		check(GameInstance);
		T* Sub = GameInstance->GetSubsystem<T>();
		check(Sub);
		return *Sub;
	}

	// =========================================================================
	// EN: UWorldSubsystem resolver
	// ES: Resolvedor de UWorldSubsystem
	// =========================================================================

	/**
	 * EN: Resolve a UWorldSubsystem T* from UWorld. Returns nullptr if
	 *     World or the Subsystem is null.
	 *
	 * ES: Resuelve un UWorldSubsystem T* desde UWorld. Retorna nullptr si
	 *     World o el Subsystem es null.
	 */
	template <typename T>
	static T* GetWorldSubsystem(const UWorld* World)
	{
		static_assert(TIsDerivedFrom<T, UWorldSubsystem>::Value,
			"T must derive from UWorldSubsystem");
		return World ? World->GetSubsystem<T>() : nullptr;
	}

	/**
	 * EN: Resolve a UWorldSubsystem T* from UWorld, asserting on null.
	 *
	 * ES: Resuelve un UWorldSubsystem T* desde UWorld con assert si null.
	 */
	template <typename T>
	static T& GetWorldSubsystemChecked(const UWorld* World)
	{
		static_assert(TIsDerivedFrom<T, UWorldSubsystem>::Value,
			"T must derive from UWorldSubsystem");
		check(World);
		T* Sub = World->GetSubsystem<T>();
		check(Sub);
		return *Sub;
	}

	// =========================================================================
	// EN: WorldContext-based resolver (any UObject-derived WorldContext)
	// ES: Resolvedor basado en WorldContext (cualquier UObject derivado)
	// =========================================================================

	/**
	 * EN: Resolve a UGameInstanceSubsystem T* from any UObject WorldContext.
	 *     Calls GetWorld() → GetGameInstance() → GetSubsystem<T>() with null
	 *     safety at each step. Returns nullptr on any null.
	 *
	 *     Preferred entry point when writing code inside a UObject method
	 *     (Actor, Component, Subsystem, etc.) because it needs zero
	 *     boilerplate — just pass `this`.
	 *
	 * ES: Resuelve un UGameInstanceSubsystem T* desde cualquier WorldContext
	 *     UObject. Null-safe en cada paso. Entrada preferida cuando se
	 *     escribe codigo dentro de un metodo UObject.
	 */
	template <typename T>
	static T* GetFromContext(const UObject* WorldContext)
	{
		static_assert(TIsDerivedFrom<T, UGameInstanceSubsystem>::Value,
			"T must derive from UGameInstanceSubsystem");
		if (!WorldContext)
		{
			return nullptr;
		}
		UWorld* World = WorldContext->GetWorld();
		if (!World)
		{
			return nullptr;
		}
		UGameInstance* GI = World->GetGameInstance();
		return GI ? GI->GetSubsystem<T>() : nullptr;
	}

	/**
	 * EN: Resolve a UGameInstanceSubsystem T& from WorldContext, asserting
	 *     on any null in the chain.
	 *
	 * ES: Resuelve un UGameInstanceSubsystem T& desde WorldContext con
	 *     assert si cualquier puntero en la cadena es null.
	 */
	template <typename T>
	static T& GetFromContextChecked(const UObject* WorldContext)
	{
		static_assert(TIsDerivedFrom<T, UGameInstanceSubsystem>::Value,
			"T must derive from UGameInstanceSubsystem");
		check(WorldContext);
		UWorld* World = WorldContext->GetWorld();
		check(World);
		UGameInstance* GI = World->GetGameInstance();
		check(GI);
		T* Sub = GI->GetSubsystem<T>();
		check(Sub);
		return *Sub;
	}

	// =========================================================================
	// EN: Convenience — resolve UWorld from WorldContext
	// ES: Conveniencia — resolver UWorld desde WorldContext
	// =========================================================================

	/**
	 * EN: Safely resolve a UWorld* from any UObject WorldContext.
	 *     Returns nullptr if the context is null or GetWorld() fails.
	 *
	 * ES: Resuelve UWorld* desde cualquier WorldContext UObject de forma
	 *     segura. Retorna nullptr si el contexto es null o GetWorld() falla.
	 */
	static UWorld* GetWorldFromContext(const UObject* WorldContext)
	{
		return WorldContext ? WorldContext->GetWorld() : nullptr;
	}

private:
	// EN: Static-only class — no instantiation.
	// ES: Clase estatica — no instanciable.
	FPGXSubsystemResolver() = delete;
};
