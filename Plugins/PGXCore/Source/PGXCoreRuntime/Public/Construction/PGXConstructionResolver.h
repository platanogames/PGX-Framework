// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Construction/PGXClassConstruction.h"
#include "Construction/PGXConstructionTypes.h"
#include "Construction/PGXWorldConstructionSubsystem.h"
#include "Construction/PGXConstructionSettings.h"
#include "Engine/World.h"

PGXCORERUNTIME_API DECLARE_LOG_CATEGORY_EXTERN(LogPGXConstruction, Log, All);

/**
 * EN: Static utility for resolving and applying construction DataAssets.
 *     Resolution order: WorldOverride >> ProjectSettings >> nullptr.
 * ES: Utilidad estatica para resolver y aplicar DataAssets de construccion.
 *     Orden de resolucion: WorldOverride >> ProjectSettings >> nullptr.
 */
class PGXCORERUNTIME_API FPGXConstructionResolver
{
public:
	/**
	 * EN: Resolve a construction DA for a given type.
	 *     Checks WorldConstructionSubsystem first, then Project Settings.
	 * ES: Resolver un DA de construccion para un tipo dado.
	 *     Verifica WorldConstructionSubsystem primero, luego Project Settings.
	 */
	template<typename T>
	static const T* Resolve(UWorld* World);

	/**
	 * EN: Inject components from a construction DA into an actor.
	 *     Respects Profile capabilities and network relevance.
	 * ES: Inyectar componentes de un DA de construccion en un actor.
	 *     Respeta capabilities de Profile y relevancia de red.
	 */
	static void InjectComponents(AActor* Actor, const UPGXClassConstruction* Construction);

	/**
	 * EN: Apply granted tags from a construction DA (for future tag-aware actors).
	 * ES: Aplicar tags otorgados de un DA de construccion.
	 */
	static FGameplayTagContainer GetGrantedTags(const UPGXClassConstruction* Construction);

private:
	/** EN: Check if we should inject based on network role / ES: Verificar si debemos inyectar segun rol de red */
	static bool ShouldInjectForNetwork(const AActor* Actor, const FPGXComponentInjection& Injection);

	/** EN: Check if a Profile capability is met / ES: Verificar si una capability de Profile se cumple */
	static bool IsCapabilityMet(const UWorld* World, const FGameplayTag& CapabilityTag);

	/** EN: Get the settings-based soft ptr for a type / ES: Obtener el soft ptr de settings para un tipo */
	template<typename T>
	static TSoftObjectPtr<T> GetSettingsPtr();
};

// ═══════════════════════════════════════════════════════════════
// Template Implementation / Implementacion de Template
// ═══════════════════════════════════════════════════════════════

template<typename T>
const T* FPGXConstructionResolver::Resolve(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	// EN: Step 1: Check world override / ES: Paso 1: Verificar override de mundo
	if (UPGXWorldConstructionSubsystem* WorldSub = World->GetSubsystem<UPGXWorldConstructionSubsystem>())
	{
		const T* Override = WorldSub->GetOverride<T>();
		if (Override)
		{
			return Override;
		}
	}

	// EN: Step 2: Fall back to Project Settings / ES: Paso 2: Fallback a Project Settings
	TSoftObjectPtr<T> SoftPtr = GetSettingsPtr<T>();
	if (!SoftPtr.IsNull())
	{
		return SoftPtr.LoadSynchronous();
	}

	// EN: No DA found — verbose log for debugging if enabled
	// ES: No se encontro DA — log verbose para debugging si esta habilitado
	if (GetDefault<UPGXConstructionSettings>()->bVerboseConstructionLog)
	{
		UE_LOG(LogPGXConstruction, Verbose, TEXT("Resolve<%s>: No DA found (no world override, no project setting). Using base class defaults."), *T::StaticClass()->GetName());
	}

	return nullptr;
}

// ─── Settings Ptr Specializations ───

class UPGXActorConstruction;
class UPGXGameModeConstruction;
class UPGXPlayerControllerConstruction;
class UPGXGameStateConstruction;
class UPGXPlayerStateConstruction;
class UPGXCharacterConstruction;
class UPGXPawnConstruction;
class UPGXHUDConstruction;

template<> inline TSoftObjectPtr<UPGXActorConstruction> FPGXConstructionResolver::GetSettingsPtr<UPGXActorConstruction>()
{
	return GetDefault<UPGXConstructionSettings>()->ActorConstruction;
}

template<> inline TSoftObjectPtr<UPGXGameModeConstruction> FPGXConstructionResolver::GetSettingsPtr<UPGXGameModeConstruction>()
{
	return GetDefault<UPGXConstructionSettings>()->GameModeConstruction;
}

template<> inline TSoftObjectPtr<UPGXPlayerControllerConstruction> FPGXConstructionResolver::GetSettingsPtr<UPGXPlayerControllerConstruction>()
{
	return GetDefault<UPGXConstructionSettings>()->PlayerControllerConstruction;
}

template<> inline TSoftObjectPtr<UPGXGameStateConstruction> FPGXConstructionResolver::GetSettingsPtr<UPGXGameStateConstruction>()
{
	return GetDefault<UPGXConstructionSettings>()->GameStateConstruction;
}

template<> inline TSoftObjectPtr<UPGXPlayerStateConstruction> FPGXConstructionResolver::GetSettingsPtr<UPGXPlayerStateConstruction>()
{
	return GetDefault<UPGXConstructionSettings>()->PlayerStateConstruction;
}

template<> inline TSoftObjectPtr<UPGXCharacterConstruction> FPGXConstructionResolver::GetSettingsPtr<UPGXCharacterConstruction>()
{
	return GetDefault<UPGXConstructionSettings>()->CharacterConstruction;
}

template<> inline TSoftObjectPtr<UPGXPawnConstruction> FPGXConstructionResolver::GetSettingsPtr<UPGXPawnConstruction>()
{
	return GetDefault<UPGXConstructionSettings>()->PawnConstruction;
}

template<> inline TSoftObjectPtr<UPGXHUDConstruction> FPGXConstructionResolver::GetSettingsPtr<UPGXHUDConstruction>()
{
	return GetDefault<UPGXConstructionSettings>()->HUDConstruction;
}
