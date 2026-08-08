// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PGXSaveSlotInfo.generated.h"

/**
 * EN: Metadata about a save slot.
 *     Contains display information, versioning, timing, and extensible
 *     custom metadata for UI display and slot enumeration.
 *
 * ES: Metadata sobre un slot de guardado.
 *     Contiene informacion de display, versionado, tiempos, y metadata
 *     custom extensible para display en UI y enumeracion de slots.
 */
USTRUCT(BlueprintType)
struct PGXSAVERUNTIME_API FPGXSaveSlotInfo
{
	GENERATED_BODY()

	// ========================================================================
	// EN: Identity
	// ES: Identidad
	// ========================================================================

	/** EN: Internal slot identifier (e.g. "Slot_01", "AutoSave_02") / ES: Identificador interno del slot */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Slot")
	FString SlotName;

	/** EN: User-facing display name (e.g. "Partida 1") / ES: Nombre para mostrar al usuario */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Slot")
	FText DisplayName;

	/** EN: Context tag identifying which dashboard owns this slot / ES: Tag de contexto que identifica que dashboard posee este slot */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Slot")
	FGameplayTag ContextTag;

	// ========================================================================
	// EN: Timing
	// ES: Tiempos
	// ========================================================================

	/** EN: Date and time when the save was made / ES: Fecha y hora en que se realizo el guardado */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Slot")
	FDateTime SaveDate;

	/** EN: Total accumulated play time / ES: Tiempo total jugado acumulado */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Slot")
	FTimespan PlayTime;

	// ========================================================================
	// EN: Game context
	// ES: Contexto de juego
	// ========================================================================

	/** EN: Gameplay tag representing where the player is (chapter, area, etc.) / ES: GameplayTag representando donde esta el jugador (capitulo, area, etc.) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Slot")
	FGameplayTag ChapterTag;

	/** EN: Name of the current map/level / ES: Nombre del mapa/nivel actual */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Slot")
	FName LevelName;

	// ========================================================================
	// EN: Versioning & integrity
	// ES: Versionado e integridad
	// ========================================================================

	/** EN: Save data version number / ES: Numero de version de los datos de guardado */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Slot")
	int32 SaveVersion = 0;

	/** EN: Number of domains stored in this slot / ES: Numero de dominios almacenados en este slot */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Slot")
	int32 DomainCount = 0;

	/** EN: Total file size in bytes / ES: Tamano total del archivo en bytes */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Slot")
	int64 TotalSizeBytes = 0;

	/** EN: True if the save data failed integrity validation / ES: True si los datos fallaron la validacion de integridad */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Slot")
	bool bIsCorrupted = false;

	// ========================================================================
	// EN: Save type flags
	// ES: Flags de tipo de guardado
	// ========================================================================

	/** EN: True if this slot was created by auto-save / ES: True si este slot fue creado por auto-guardado */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Slot")
	bool bIsAutoSave = false;

	/** EN: True if this slot is the quick-save slot / ES: True si este slot es el slot de quick-save */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Slot")
	bool bIsQuickSave = false;

	// ========================================================================
	// EN: Visual (v1.1 prepared)
	// ES: Visual (v1.1 preparado)
	// ========================================================================

	/** EN: Path to the thumbnail screenshot (empty until v1.1) / ES: Ruta al screenshot thumbnail (vacio hasta v1.1) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Slot")
	FString ThumbnailPath;

	// ========================================================================
	// EN: Extensibility
	// ES: Extensibilidad
	// ========================================================================

	/** EN: Project-specific custom metadata (key-value string pairs) / ES: Metadata custom especifica del proyecto (pares key-value string) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Slot")
	TMap<FName, FString> CustomMetadata;

	// ========================================================================
	// EN: Utility
	// ES: Utilidad
	// ========================================================================

	/** EN: Returns true if this slot info represents a valid slot / ES: Retorna true si esta info de slot representa un slot valido */
	bool IsValid() const { return !SlotName.IsEmpty(); }
};
