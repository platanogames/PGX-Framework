// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Base/PGXBaseMessaging.h"
#include "PGXGameInstance.generated.h"

class UPGXDataAsset;

/**
 * EN: Base game instance for PGX projects. Provides standardized subsystem
 *     access, construction DA resolution, and lifecycle management.
 *     Blueprint users can configure per-instance bridge wiring via UPROPERTY.
 * ES: Game instance base para proyectos PGX. Proporciona acceso estandarizado
 *     a subsistemas, resolucion de DA de construccion y gestion del ciclo de vida.
 *     Usuarios de Blueprint pueden configurar bridge wiring per-instancia via UPROPERTY.
 */
UCLASS(BlueprintType, Blueprintable)
class PGXCORERUNTIME_API UPGXGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPGXGameInstance();

	/**
	 * EN: Human-readable display name for diagnostics and logging.
	 *     Override in derived classes or Blueprints for custom names.
	 * ES: Nombre legible para diagnosticos y logging.
	 *     Sobreescribir en clases derivadas o Blueprints para nombres custom.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|Identity")
	FString GetPGXDisplayName() const;

	//~ Begin UGameInstance Interface
	void Init() override;
	void Shutdown() override;
	//~ End UGameInstance Interface

	// ─── Per-Instance Configuration (PGX Bridge) ───

	/** EN: Listen to GameFlow state changes (default: true for GameInstance) / ES: Escuchar cambios de estado de GameFlow (default: true para GameInstance) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|GameFlow",
		meta = (DisplayName = "Listen to GameFlow Changes"))
	bool bListenToGameFlowChanges = true;

protected:
	/**
	 * EN: Called after Init to apply platform-specific configuration.
	 * ES: Llamado despues de Init para aplicar configuracion de plataforma.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|Platform")
	void ApplyPlatformProfile();

	// ─── Message Bus Integration ───

	/** EN: Broadcast a typed message on the PGX Message bus / ES: Publicar un mensaje tipado en el bus PGX */
	template<typename T>
	void BroadcastPGXMessage(FGameplayTag Channel, const T& Payload)
	{
		PGXBaseMessaging::Broadcast<T>(this, Channel, Payload);
	}

	/** EN: Listen for a typed message. Auto-cleaned in Shutdown / ES: Escuchar un mensaje tipado. Auto-limpiado en Shutdown */
	template<typename T>
	void ListenForPGXMessage(FGameplayTag Channel, TFunction<void(FGameplayTag, const T&)> Callback)
	{
		MessageListenerHandles.Add(PGXBaseMessaging::Listen<T>(this, Channel, MoveTemp(Callback)));
	}

	/** EN: Active message listener handles (auto-cleaned) / ES: Handles activos (auto-limpiados) */
	TArray<FPGXMessageListenerHandle> MessageListenerHandles;

	// ─── EventHandler Integration ───

	/** EN: Fire an event through the PGX EventHandler subsystem (C++ only — use BP Library) / ES: Disparar un evento via EventHandler (solo C++ — usar BP Library) */
	void FirePGXEvent(FGameplayTag EventTag);

	// ─── DataRegistry Integration ───

	/** EN: Query the PGX DataRegistry for a resolved asset / ES: Consultar el DataRegistry por un asset resuelto */
	UFUNCTION(BlueprintCallable, Category = "PGX|Registry")
	UPGXDataAsset* QueryPGXRegistry(FGameplayTag DatabaseTag, FGameplayTag EntryTag) const;

	/**
	 * EN: Called after Init when DataRegistry subsystem is available.
	 *     Override in derived classes or Blueprints for post-registry initialization.
	 * ES: Llamado despues de Init cuando el subsistema DataRegistry esta disponible.
	 *     Sobreescribir en clases derivadas o Blueprints para inicializacion post-registry.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|Registry")
	void OnRegistryReady();

	// ─── L2 Bridge Wiring ───

	/** EN: Called when GameFlow state changes / ES: Llamado cuando el estado de GameFlow cambia */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|GameFlow")
	void OnGameFlowStateChanged(FGameplayTag OldState, FGameplayTag NewState);
};
