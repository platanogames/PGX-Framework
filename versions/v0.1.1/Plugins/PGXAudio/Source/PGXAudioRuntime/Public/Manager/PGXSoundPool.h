// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PGXAudioTypes.h"
#include "PGXSoundPool.generated.h"

class UAudioComponent;

/**
 * EN: Sound instance pool — manages reusable UAudioComponent instances
 *     to reduce allocation overhead for high-frequency sounds.
 *     Grows dynamically up to MaxCapacity. When exhausted, reclaims
 *     lowest-priority active sounds.
 *
 * ES: Pool de instancias de sonido — gestiona instancias reutilizables de UAudioComponent
 *     para reducir overhead de allocacion para sonidos de alta frecuencia.
 *     Crece dinamicamente hasta MaxCapacity. Cuando se agota, reclama
 *     sonidos activos de menor prioridad.
 */
UCLASS()
class PGXAUDIORUNTIME_API UPGXSoundPool : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * EN: Initialize pool with capacity settings.
	 * ES: Inicializar pool con settings de capacidad.
	 * @param InitialSize Initial pre-allocated component count.
	 * @param InMaxCapacity Maximum pool size (0 = unlimited).
	 */
	void Initialize(int32 InitialSize, int32 InMaxCapacity);

	/** EN: Shutdown and destroy all pooled components / ES: Apagar y destruir todos los componentes del pool */
	void Deinitialize();

	/**
	 * EN: Acquire a component from the pool.
	 * ES: Adquirir un componente del pool.
	 * @param WorldContextObject World context for component creation.
	 * @return Audio component ready for use, or nullptr if pool exhausted.
	 */
	UAudioComponent* AcquireComponent(UObject* WorldContextObject);

	/**
	 * EN: Return a component to the pool for reuse.
	 * ES: Devolver un componente al pool para reutilizacion.
	 */
	void ReleaseComponent(UAudioComponent* Component);

	/** EN: Get pool statistics / ES: Obtener estadisticas del pool */
	FPGXSoundPoolStats GetStats() const;

	/** EN: Get memory info from pool / ES: Obtener info de memoria del pool */
	FPGXAudioMemoryInfo GetMemoryInfo() const;

private:
	/** EN: Grow pool by creating new components / ES: Crecer pool creando nuevos componentes */
	void GrowPool(int32 Count, UObject* WorldContextObject);

	/** EN: Available (idle) components / ES: Componentes disponibles (inactivos) */
	UPROPERTY()
	TArray<TObjectPtr<UAudioComponent>> AvailableComponents;

	/** EN: Currently in-use components / ES: Componentes actualmente en uso */
	UPROPERTY()
	TArray<TObjectPtr<UAudioComponent>> ActiveComponents;

	/** EN: Maximum pool capacity / ES: Capacidad maxima del pool */
	int32 MaxCapacity = 128;

	/** EN: Peak usage watermark / ES: Marca de agua de uso pico */
	int32 PeakUsage = 0;

	/** EN: Times pool was empty when a request came / ES: Veces que el pool estaba vacio cuando llego una peticion */
	int32 MissCount = 0;

	/** EN: Times pool grew dynamically / ES: Veces que el pool crecio dinamicamente */
	int32 GrowthEvents = 0;
};
