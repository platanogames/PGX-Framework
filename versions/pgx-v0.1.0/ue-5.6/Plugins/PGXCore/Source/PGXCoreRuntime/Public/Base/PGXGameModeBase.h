// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Base/PGXBaseMessaging.h"
#include "PGXGameModeBase.generated.h"

class UPGXGameModeConstruction;
class UPGXDataAsset;
struct FPGXBridgeSaveNotification;
struct FPGXBridgeLevelTransition;
struct FPGXBridgeLoadingState;

/**
 * EN: Base game mode for all PGX projects. Provides standardized initialization,
 *     player spawning rules, construction DA resolution, and integration with PGX subsystems.
 *     Blueprint users can configure per-instance bridge wiring via UPROPERTY.
 * ES: Game mode base para todos los proyectos PGX. Proporciona inicializacion estandarizada,
 *     reglas de spawn de jugadores, resolucion de DA de construccion, e integracion con subsistemas PGX.
 *     Usuarios de Blueprint pueden configurar bridge wiring per-instancia via UPROPERTY.
 */
UCLASS(BlueprintType, Blueprintable)
class PGXCORERUNTIME_API APGXGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	APGXGameModeBase();

	/**
	 * EN: Human-readable display name for diagnostics and logging.
	 *     Override in derived classes or Blueprints for custom names.
	 * ES: Nombre legible para diagnosticos y logging.
	 *     Sobreescribir en clases derivadas o Blueprints para nombres custom.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|Identity")
	FString GetPGXDisplayName() const;

	//~ Begin AGameModeBase Interface
	void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	//~ End AGameModeBase Interface

	// ─── Lifecycle Event Helpers ───

	/** EN: Fire Match Started event via EventHandler / ES: Disparar evento Match Started via EventHandler */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow")
	void FireMatchStartedEvent();

	/** EN: Fire Match Ended event via EventHandler / ES: Disparar evento Match Ended via EventHandler */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow")
	void FireMatchEndedEvent();

	/** EN: Fire Player Joined event via EventHandler / ES: Disparar evento Player Joined via EventHandler */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow")
	void FirePlayerJoinedEvent(APlayerController* NewPlayer);

	/** EN: Fire Player Left event via EventHandler / ES: Disparar evento Player Left via EventHandler */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow")
	void FirePlayerLeftEvent(AController* Exiting);

	/**
	 * EN: Publish a GameFlow state change via bridge message.
	 *     Call when your GameMode transitions between game states.
	 * ES: Publicar un cambio de estado de GameFlow via mensaje bridge.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow")
	void PublishGameFlowState(FGameplayTag NewState);

	//~ Begin AActor Interface
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface

	// ─── Per-Instance Configuration (PGX Bridge) ───

	/** EN: Optional Construction DA override for this instance / ES: Override opcional de DA de construccion para esta instancia */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Construction",
		meta = (DisplayName = "Construction Override"))
	TSoftObjectPtr<UPGXGameModeConstruction> ConstructionOverride;

	/** EN: Listen to GameFlow state changes / ES: Escuchar cambios de estado de GameFlow */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|GameFlow",
		meta = (DisplayName = "Listen to GameFlow Changes"))
	bool bListenToGameFlowChanges = true;

	/** EN: Listen to level transition events / ES: Escuchar eventos de transicion de nivel */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|LevelFlow",
		meta = (DisplayName = "Listen to Level Transitions"))
	bool bListenToLevelTransitions = false;

	/** EN: Listen to loading screen state changes / ES: Escuchar cambios de estado de pantalla de carga */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Loading",
		meta = (DisplayName = "Listen to Loading Screen State"))
	bool bListenToLoadingScreenState = false;

protected:
	// ─── Construction DA ───

	/** EN: Apply configuration from Construction DA / ES: Aplicar configuracion del DA de construccion */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|Construction")
	void ApplyConstruction(const UPGXGameModeConstruction* Construction);

	/** EN: Apply platform-specific configuration / ES: Aplicar configuracion de plataforma */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|Platform")
	void ApplyPlatformProfile();

	// ─── Message Bus Integration ───

	/** EN: Broadcast a typed message on the PGX Message bus / ES: Publicar un mensaje tipado en el bus PGX */
	template<typename T>
	void BroadcastPGXMessage(FGameplayTag Channel, const T& Payload)
	{
		PGXBaseMessaging::Broadcast<T>(this, Channel, Payload);
	}

	/** EN: Listen for a typed message. Auto-cleaned in EndPlay / ES: Escuchar un mensaje tipado. Auto-limpiado en EndPlay */
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

	// ─── L2 Bridge Wiring ───

	/** EN: Setup Save bridge / ES: Configurar bridge Save */
	virtual void SetupSaveBridge(FGameplayTag InSaveDomain);

	/** EN: Teardown Save bridge (called from EndPlay) / ES: Desmontar bridge Save */
	virtual void TeardownSaveBridge();

	/** EN: Called before a save operation for this actor's domain / ES: Llamado antes de save */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|Save")
	void OnPGXPreSave(const FPGXBridgeSaveNotification& Notification);

	/** EN: Called after a load operation for this actor's domain / ES: Llamado despues de load */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|Save")
	void OnPGXPostLoad(const FPGXBridgeSaveNotification& Notification);

	/** EN: Called when GameFlow state changes / ES: Llamado cuando el estado de GameFlow cambia */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|GameFlow")
	void OnGameFlowStateChanged(FGameplayTag OldState, FGameplayTag NewState);

	/** EN: Called when a level transition starts / ES: Llamado al comenzar transicion de nivel */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|LevelFlow")
	void OnLevelTransitionStarted(const FPGXBridgeLevelTransition& Transition);

	/** EN: Called when a level transition completes / ES: Llamado al completar transicion de nivel */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|LevelFlow")
	void OnLevelTransitionCompleted(const FPGXBridgeLevelTransition& Transition);

	/** EN: Called when loading screen state changes / ES: Llamado al cambiar estado de pantalla de carga */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|Loading")
	void OnLoadingScreenStateChanged(const FPGXBridgeLoadingState& State);

	/** EN: Cached save domain from bridge setup / ES: Dominio de save cacheado del bridge */
	FGameplayTag CachedSaveDomain;

	/** EN: Cached GameFlow state for publishing transitions / ES: Estado GameFlow cacheado para publicar transiciones */
	FGameplayTag CachedGameFlowState;
};
