// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Base/PGXBaseMessaging.h"
#include "PGXPawnBase.generated.h"

class UPGXPawnConstruction;
class UPGXDataAsset;
struct FPGXBridgeSaveNotification;
struct FPGXBridgeLevelTransition;
struct FPGXBridgeLoadingState;

/**
 * EN: Base pawn for PGX projects. Supports construction DA resolution,
 *     component injection, and PGX subsystem hooks.
 *     Blueprint users can configure per-instance bridge wiring via UPROPERTY.
 * ES: Pawn base para proyectos PGX. Soporta resolucion de DA de construccion,
 *     inyeccion de componentes, y hooks a subsistemas PGX.
 *     Usuarios de Blueprint pueden configurar bridge wiring per-instancia via UPROPERTY.
 */
UCLASS(BlueprintType, Blueprintable)
class PGXCORERUNTIME_API APGXPawnBase : public APawn
{
	GENERATED_BODY()

public:
	APGXPawnBase();

	/**
	 * EN: Human-readable display name for diagnostics and logging.
	 *     Override in derived classes or Blueprints for custom names.
	 * ES: Nombre legible para diagnosticos y logging.
	 *     Sobreescribir en clases derivadas o Blueprints para nombres custom.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|Identity")
	FString GetPGXDisplayName() const;

	//~ Begin AActor Interface
	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface

	// ─── Per-Instance Configuration (PGX Bridge) ───

	/** EN: Optional Construction DA override for this instance / ES: Override opcional de DA de construccion para esta instancia */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Construction",
		meta = (DisplayName = "Construction Override"))
	TSoftObjectPtr<UPGXPawnConstruction> ConstructionOverride;

	/** EN: Whether this pawn participates in Save system / ES: Si este pawn participa en el sistema Save */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Save",
		meta = (DisplayName = "Participate in Save"))
	bool bParticipateInSave = false;

	/** EN: Save domain for this pawn (only used if bParticipateInSave) / ES: Dominio de save (solo si bParticipateInSave) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Save",
		meta = (DisplayName = "Save Domain", EditCondition = "bParticipateInSave", Categories = "PGX.Save.Domain"))
	FGameplayTag SaveDomain;

	/** EN: Listen to GameFlow state changes / ES: Escuchar cambios de estado de GameFlow */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|GameFlow",
		meta = (DisplayName = "Listen to GameFlow Changes"))
	bool bListenToGameFlowChanges = false;

	/** EN: Listen to level transition events / ES: Escuchar eventos de transicion de nivel */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|LevelFlow",
		meta = (DisplayName = "Listen to Level Transitions"))
	bool bListenToLevelTransitions = false;

	/** EN: Listen to loading screen state changes / ES: Escuchar cambios de estado de pantalla de carga */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Loading",
		meta = (DisplayName = "Listen to Loading Screen State"))
	bool bListenToLoadingScreenState = false;

	/** EN: Default sound event tag for this pawn / ES: Tag de evento de sonido por defecto para este pawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Audio",
		meta = (DisplayName = "Default Sound Event", Categories = "PGX.Audio"))
	FGameplayTag DefaultSoundEventTag;

protected:
	// ─── Construction DA ───

	/** EN: Apply configuration from Construction DA / ES: Aplicar configuracion del DA de construccion */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|Construction")
	void ApplyConstruction(const UPGXPawnConstruction* Construction);

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

	/** EN: Called before a save operation for this pawn's domain / ES: Llamado antes de save */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|Save")
	void OnPGXPreSave(const FPGXBridgeSaveNotification& Notification);

	/** EN: Called after a load operation for this pawn's domain / ES: Llamado despues de load */
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

	// ─── Audio Bridge ───

	/** EN: Request a sound play through PGX Audio bridge / ES: Solicitar sonido via bridge Audio PGX */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio")
	void RequestPGXSound(FGameplayTag SoundEventTag, FVector Location = FVector::ZeroVector);

	/** EN: Request to stop a sound through PGX Audio bridge / ES: Solicitar detener sonido via bridge Audio */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio")
	void StopPGXSound(FGameplayTag SoundEventTag);
};
