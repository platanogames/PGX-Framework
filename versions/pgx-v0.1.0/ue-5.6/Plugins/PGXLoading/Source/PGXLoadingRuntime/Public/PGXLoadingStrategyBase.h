// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "PGXLoadingTypes.h"
#include "PGXLoadingStrategyBase.generated.h"

class UPGXLoadingProfile;
class UPGXLoadingWidget;

/**
 * EN: Abstract base for loading screen visual strategies. Each strategy handles a specific visual
 *     type (Minimal, Image, Slideshow, Material, Video). The Subsystem delegates all visual work
 *     to the active Strategy, decoupling presentation from orchestration.
 *
 *     Lifecycle: InitializeStrategy → (wait IsReady) → Activate → UpdateVisual* → Deactivate
 *
 *     Strategies manage their own async asset loading via StreamableManager and report readiness.
 *     If a strategy fails to become ready within timeout, the Subsystem falls back to Minimal.
 *
 * ES: Base abstracta para estrategias visuales de pantalla de carga. Cada estrategia maneja un tipo
 *     visual especifico. El Subsistema delega todo el trabajo visual a la Strategy activa.
 *
 *     Ciclo: InitializeStrategy → (esperar IsReady) → Activate → UpdateVisual* → Deactivate
 */
UCLASS(Abstract, Blueprintable)
class PGXLOADINGRUNTIME_API UPGXLoadingStrategyBase : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * EN: Initialize with profile data. Start loading required resources.
	 * ES: Inicializar con datos del perfil. Comenzar carga de recursos necesarios.
	 */
	virtual void InitializeStrategy(UPGXLoadingProfile* Profile);

	/**
	 * EN: Activate visual on widget. Called when overlay is visible and ready.
	 * ES: Activar visual en widget. Llamado cuando el overlay es visible y esta listo.
	 */
	virtual void Activate(UPGXLoadingWidget* Widget);

	/**
	 * EN: Update visual state per tick (optional — not all strategies need this).
	 * ES: Actualizar estado visual por tick (opcional).
	 */
	virtual void UpdateVisual(float DeltaTime);

	/**
	 * EN: Deactivate and clean up all resources.
	 * ES: Desactivar y limpiar todos los recursos.
	 */
	virtual void Deactivate();

	/**
	 * EN: Are minimum resources loaded for display?
	 * ES: Estan cargados los recursos minimos para mostrar?
	 */
	virtual bool IsReady() const;

	/**
	 * EN: Receive progress update for display.
	 * ES: Recibir actualizacion de progreso para mostrar.
	 */
	virtual void OnProgressUpdated(const FPGXLoadingProgress& Progress);

	/**
	 * EN: Get current tip text (for strategies that rotate tips).
	 * ES: Obtener texto de consejo actual.
	 */
	virtual FText GetCurrentTip() const;

protected:
	UPROPERTY()
	TObjectPtr<UPGXLoadingProfile> CachedProfile;

	UPROPERTY()
	TObjectPtr<UPGXLoadingWidget> CachedWidget;

	TSharedPtr<FStreamableHandle> AssetLoadHandle;

	bool bReady = false;
	bool bActivated = false;
};
