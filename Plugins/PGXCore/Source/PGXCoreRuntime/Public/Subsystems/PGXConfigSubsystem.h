// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Utils/PGXConfigResolution.h"
#include "PGXConfigSubsystem.generated.h"

/**
 * EN: Runtime configuration management system. Centralizes config loading via
 *     PGX::ResolveSingleConfig<T>() with optional caching, change notification,
 *     and hot-reload.
 *
 *     Usage:
 *       // In any subsystem that needs config:
 *       void UMySubsystem::LoadConfig()
 *       {
 *           UPGXConfigSubsystem* ConfigSys = UPGXConfigSubsystem::Get(this);
 *           UPGXMyConfig* Config = ConfigSys->GetActiveConfig<UPGXMyConfig>(
 *               Settings->ActiveConfig, TEXT("MySystem"));
 *           if (Config) { ApplyConfig(Config); }
 *       }
 *
 *       // Subscribe to config changes (optional):
 *       ConfigSys->OnPGXConfigChanged.AddUObject(this, &UMySubsystem::OnConfigReloaded);
 *
 *     GetActiveConfig<T>() wraps
 *     PGX::ResolveSingleConfig<T>() with in-memory caching and change notification.
 *     ReloadAll() clears cache and re-resolves on next GetActiveConfig call.
 *
 * ES: Sistema centralizado de gestion de configuracion en runtime. Centraliza carga
 *     via PGX::ResolveSingleConfig<T>() con cache opcional, notificacion de cambio,
 *     y hot-reload.
 *
 *     Implementacion con cache + notificacion.
 */
UCLASS()
class PGXCORERUNTIME_API UPGXConfigSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface

	/**
	 * EN: Resolve and cache a single typed config DA. On first call per SystemName,
	 *     delegates to PGX::ResolveSingleConfig<T>() and caches the result. Subsequent
	 *     calls return the cached pointer (fast, no re-resolution).
	 *
	 *     Pass bForceReload=true to bypass cache and re-resolve (fires OnPGXConfigChanged
	 *     so subscribers can react). This is the programmatic equivalent of ReloadAll()
	 *     for a single system.
	 *
	 *     Returns nullptr if no config is found or loading fails.
	 *
	 * ES: Resuelve y cachea un DA de config unico. Primera llamada por SystemName
	 *     delega a PGX::ResolveSingleConfig<T>() y cachea. Llamadas siguientes
	 *     retornan el puntero cacheado (rapido, sin re-resolucion).
	 *
	 *     Pasar bForceReload=true para saltar cache y re-resolver (dispara
	 *     OnPGXConfigChanged para que los suscriptores reaccionen).
	 */
	template<typename TConfig>
	TConfig* GetActiveConfig(const TSoftObjectPtr<TConfig>& ConfigRef,
	                         const FString& SystemName,
	                         bool bForceReload = false)
	{
		// ── Cache hit (fast path) ──
		if (!bForceReload)
		{
			TWeakObjectPtr<UObject>* Cached = ConfigCache.Find(SystemName);
			if (Cached)
			{
				return Cast<TConfig>(Cached->Get());
			}
		}

		// ── Resolve via PGXConfigResolution template ──
		TConfig* Resolved = PGX::ResolveSingleConfig<TConfig>(ConfigRef, SystemName);

		// ── Update cache ──
		ConfigCache.Add(SystemName, TWeakObjectPtr<UObject>(Resolved));

		// ── Notify subscribers ──
		OnPGXConfigChanged.Broadcast(SystemName, Resolved);

		return Resolved;
	}

	/**
	 * EN: Fired when configs are reloaded (via ReloadAll, GetActiveConfig with
	 *     bForceReload=true, or external trigger). Parameters: SystemName (FString)
	 *     and the Config object (UObject*) that was loaded (may be nullptr if loading
	 *     failed).
	 *
	 *     Multi-config consumers (PGX::ResolveMultiConfig users) subscribe to this
	 *     delegate to know when to re-resolve their DataTable-backed configs.
	 *
	 * ES: Se dispara cuando las configs se recargan (via ReloadAll, GetActiveConfig
	 *     con bForceReload=true, o trigger externo). Los consumidores multi-config
	 *     se suscriben para saber cuando re-resolver sus configs via DataTable.
	 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPGXConfigChanged, const FString& /*SystemName*/, UObject* /*Config*/);
	FOnPGXConfigChanged OnPGXConfigChanged;

	/**
	 * EN: Clear ALL cached configs and notify subscribers. The next GetActiveConfig
	 *     call per system will re-resolve from scratch. Call after config assets are
	 *     modified in the editor (hot-reload), or when the game instance transitions
	 *     between editor and PIE.
	 *
	 *     Each previously-cached system fires OnPGXConfigChanged(SystemName, nullptr)
	 *     — subscribers should re-fetch via GetActiveConfig.
	 *
	 * ES: Limpia TODOS los cache de config y notifica a suscriptores. La proxima
	 *     llamada a GetActiveConfig re-resolvera desde cero.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Config")
	void ReloadAll();

	/**
	 * EN: Get the ConfigSubsystem for the given WorldContext (UObject* or UWorld*).
	 *     Resolves World->GameInstance->GetSubsystem<UPGXConfigSubsystem>().
	 *     Returns nullptr if the chain fails at any point.
	 *
	 *     Usage:
	 *       UPGXConfigSubsystem* ConfigSys = UPGXConfigSubsystem::Get(this);
	 *
	 * ES: Obtiene el ConfigSubsystem para el WorldContext dado.
	 *     Resuelve World->GameInstance->GetSubsystem<UPGXConfigSubsystem>().
	 *     Retorna nullptr si la cadena falla en cualquier punto.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Config", meta = (WorldContext = "WorldContext"))
	static UPGXConfigSubsystem* Get(const UObject* WorldContext);

private:
	/** EN: Cache of resolved configs keyed by SystemName (weak, GC-safe).
	 *  ES: Cache de configs resueltas keyed por SystemName (weak, GC-safe). */
	TMap<FString, TWeakObjectPtr<UObject>> ConfigCache;
};
