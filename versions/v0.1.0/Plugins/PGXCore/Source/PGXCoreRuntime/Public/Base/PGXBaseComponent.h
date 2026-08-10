// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Base/PGXBaseMessaging.h"
#include "PGXBaseComponent.generated.h"

class UPGXDataAsset;
struct FPGXBridgeSaveNotification;

/**
 * EN: Base component for all PGX components. Provides common initialization patterns,
 *     activation/deactivation hooks, and debug integration.
 *     Blueprint users can configure per-instance save bridge wiring via UPROPERTY.
 * ES: Componente base para todos los componentes PGX. Proporciona patrones comunes de inicializacion,
 *     hooks de activacion/desactivacion, e integracion de debug.
 *     Usuarios de Blueprint pueden configurar bridge wiring de save per-instancia via UPROPERTY.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(PGX), meta=(BlueprintSpawnableComponent))
class PGXCORERUNTIME_API UPGXBaseComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPGXBaseComponent();

	/**
	 * EN: Human-readable display name for diagnostics and logging.
	 *     Override in derived classes or Blueprints for custom names.
	 * ES: Nombre legible para diagnosticos y logging.
	 *     Sobreescribir en clases derivadas o Blueprints para nombres custom.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|Identity")
	FString GetPGXDisplayName() const;

	//~ Begin UActorComponent Interface
	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End UActorComponent Interface

	// ─── Per-Instance Configuration (PGX Bridge) ───

	/** EN: Whether this component participates in Save system / ES: Si este componente participa en el sistema Save */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Save",
		meta = (DisplayName = "Participate in Save"))
	bool bParticipateInSave = false;

	/** EN: Save domain for this component (only used if bParticipateInSave) / ES: Dominio de save (solo si bParticipateInSave) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Save",
		meta = (DisplayName = "Save Domain", EditCondition = "bParticipateInSave", Categories = "PGX.Save.Domain"))
	FGameplayTag SaveDomain;

protected:
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

	// ─── L2 Bridge Wiring (Save) ───

	/** EN: Setup Save bridge / ES: Configurar bridge Save */
	virtual void SetupSaveBridge(FGameplayTag InSaveDomain);

	/** EN: Teardown Save bridge (called from EndPlay) / ES: Desmontar bridge Save */
	virtual void TeardownSaveBridge();

	/** EN: Called before a save operation for this component's domain / ES: Llamado antes de save */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|Save")
	void OnPGXPreSave(const FPGXBridgeSaveNotification& Notification);

	/** EN: Called after a load operation for this component's domain / ES: Llamado despues de load */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|Save")
	void OnPGXPostLoad(const FPGXBridgeSaveNotification& Notification);

	/** EN: Cached save domain from bridge setup / ES: Dominio de save cacheado del bridge */
	FGameplayTag CachedSaveDomain;
};
