// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Data/PGXConfigDataAsset.h"
#include "GameplayTagContainer.h"
#include "Observability/PGXObservable.h"
#include "PGXSaveTypes.h"
#include "PGXSaveProvider.h"
#include "Path/PGXPathTypes.h"
#include "Trace/PGXTraceTypes.h"
#include "PGXSaveConfig.generated.h"

/**
 * EN: Config DataAsset dashboard for the PGX Save system.
 *     Each instance represents an independent save context with its own mode,
 *     domains, slots, and operational parameters. Multiple dashboards can
 *     coexist in the same project.
 *
 *     Create instances from Content Browser: PGX Framework > Save Config
 *     Examples: DA_SaveConfig_Gameplay, DA_SaveConfig_Settings, DA_SaveConfig_Dev
 *
 * ES: DataAsset dashboard de configuracion para el sistema de guardado PGX.
 *     Cada instancia representa un contexto de guardado independiente con su
 *     propio modo, dominios, slots y parametros operacionales. Multiples
 *     dashboards pueden coexistir en el mismo proyecto.
 *
 *     Crear instancias desde Content Browser: PGX Framework > Save Config
 *     Ejemplos: DA_SaveConfig_Gameplay, DA_SaveConfig_Settings, DA_SaveConfig_Dev
 */
UCLASS(BlueprintType)
class PGXSAVERUNTIME_API UPGXSaveConfig : public UPGXConfigDataAsset, public IPGXObservable
{
	GENERATED_BODY()

public:
	//~ Begin IPGXObservable
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;
	//~ End IPGXObservable

	// ========================================================================
	// EN: Context identity
	// ES: Identidad de contexto
	// ========================================================================

	/** EN: Unique tag identifying this save context / ES: Tag unico que identifica este contexto de guardado */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save|Context", meta = (Categories = "PGX.Save"))
	FGameplayTag ContextTag;

	/** EN: Display name for this context (UI, editor, debug) / ES: Nombre para display de este contexto (UI, editor, debug) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save|Context")
	FText ContextDisplayName;

	// ========================================================================
	// EN: Save mode
	// ES: Modo de guardado
	// ========================================================================

	/** EN: How this context manages its save slots / ES: Como este contexto gestiona sus slots de guardado */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save|Mode")
	EPGXSaveMode SaveMode = EPGXSaveMode::MultiSlot;

	// ========================================================================
	// EN: Domains (the SaveGame classes registered in this context)
	// ES: Dominios (las clases SaveGame registradas en este contexto)
	// ========================================================================

	/** EN: Save domains managed by this context. Each maps a GameplayTag to a SaveGame class. / ES: Dominios de guardado gestionados por este contexto. Cada uno mapea un GameplayTag a una clase SaveGame. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save|Domains")
	TArray<FPGXSaveDomainEntry> SaveDomains;

	// ========================================================================
	// EN: Slot configuration
	// ES: Configuracion de slots
	// ========================================================================

	/** EN: Maximum number of player-created save slots / ES: Numero maximo de slots de guardado creados por el jugador */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save|Slots",
		meta = (AdvancedDisplay, EditCondition = "SaveMode == EPGXSaveMode::MultiSlot", ClampMin = "1", ClampMax = "100"))
	int32 MaxSaveSlots = 10;

	/** EN: Pattern for auto-generated slot names (NN = zero-padded number) / ES: Patron para nombres de slots auto-generados (NN = numero con zero-pad) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save|Slots",
		meta = (AdvancedDisplay, EditCondition = "SaveMode == EPGXSaveMode::MultiSlot"))
	FString SlotNamePattern = TEXT("Slot_{NN}");

	// ========================================================================
	// EN: Auto-save configuration
	// ES: Configuracion de auto-guardado
	// ========================================================================

	/** EN: Enable automatic saving at intervals / ES: Habilitar guardado automatico por intervalos */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save|AutoSave")
	bool bAutoSaveEnabled = true;

	/** EN: Interval in seconds between auto-saves / ES: Intervalo en segundos entre auto-guardados */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save|AutoSave",
		meta = (AdvancedDisplay, EditCondition = "bAutoSaveEnabled", ClampMin = "10.0"))
	float AutoSaveIntervalSeconds = 300.0f;

	/** EN: Number of rotating auto-save slots / ES: Numero de slots de auto-guardado en rotacion */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save|AutoSave",
		meta = (AdvancedDisplay, EditCondition = "bAutoSaveEnabled", ClampMin = "1", ClampMax = "10"))
	int32 MaxAutoSaveSlots = 3;

	/** EN: GameplayTag that triggers a checkpoint save when received / ES: GameplayTag que dispara un guardado de checkpoint al ser recibido */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save|AutoSave", meta = (AdvancedDisplay, Categories = "PGX.Save"))
	FGameplayTag AutoSaveCheckpointTag;

	// ========================================================================
	// EN: Quick save configuration
	// ES: Configuracion de quick save
	// ========================================================================

	/** EN: Enable quick save/load functionality / ES: Habilitar funcionalidad de guardado/carga rapida */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save|QuickSave", meta = (AdvancedDisplay))
	bool bEnableQuickSave = true;

	/** EN: Slot name used for quick save / ES: Nombre del slot usado para quick save */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save|QuickSave",
		meta = (AdvancedDisplay, EditCondition = "bEnableQuickSave"))
	FString QuickSaveSlotName = TEXT("QuickSave");

	// ========================================================================
	// EN: Serialization & integrity
	// ES: Serializacion e integridad
	// ========================================================================

	/** EN: Compress save data to reduce file size (recommended for shipping) / ES: Comprimir datos de guardado para reducir tamano de archivo (recomendado para shipping) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save|Serialization", meta = (AdvancedDisplay))
	bool bCompressSaveData = false;

	/** EN: Validate data integrity via checksum on load / ES: Validar integridad de datos via checksum al cargar */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save|Serialization", meta = (AdvancedDisplay))
	bool bValidateChecksum = true;

	/** EN: Current save format version (increment when data schema changes) / ES: Version actual del formato de guardado (incrementar cuando cambia el esquema de datos) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save|Serialization",
		meta = (AdvancedDisplay, ClampMin = "1"))
	int32 CurrentSaveVersion = 1;

	/** EN: Save file extension / ES: Extension del archivo de guardado */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save|Serialization", meta = (AdvancedDisplay))
	FString SaveFileExtension = TEXT(".sav");

	// ========================================================================
	// EN: Backup & recovery
	// ES: Backup y recuperacion
	// ========================================================================

	/** EN: Create a backup copy before overwriting an existing save / ES: Crear una copia de backup antes de sobreescribir un guardado existente */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save|Backup", meta = (AdvancedDisplay))
	bool bCreateBackupBeforeSave = true;

	/** EN: Maximum number of backup copies per slot / ES: Numero maximo de copias de backup por slot */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save|Backup",
		meta = (AdvancedDisplay, EditCondition = "bCreateBackupBeforeSave", ClampMin = "1", ClampMax = "10"))
	int32 MaxBackupsPerSlot = 2;

	// ========================================================================
	// EN: Paths
	// ES: Rutas
	// ========================================================================

	/** EN: Base directory for save files (relative to ProjectSavedDir) / ES: Directorio base para archivos de guardado (relativo a ProjectSavedDir) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save|Paths", meta = (AdvancedDisplay))
	FString BaseDirectory = TEXT("SaveGames");

	/** EN: Path auto-generation strategy (used by FPGXPathResolver) / ES: Estrategia de auto-generacion de paths (usada por FPGXPathResolver) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save|Paths", meta = (AdvancedDisplay))
	EPGXPathStrategy PathStrategy = EPGXPathStrategy::NamedSlot;

	/** EN: Prefix for auto-generated file names / ES: Prefijo para nombres de archivo auto-generados */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save|Paths", meta = (AdvancedDisplay))
	FString FileNamePrefix = TEXT("Save");

	/** EN: Subfolder name for Session saves / ES: Nombre de subcarpeta para guardados de sesion */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save|Paths",
		meta = (AdvancedDisplay, EditCondition = "PathStrategy == EPGXPathStrategy::Session"))
	FString SessionFolderName = TEXT("Sessions");

	/** EN: Subfolder name for Profile saves / ES: Nombre de subcarpeta para guardados de perfil */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save|Paths",
		meta = (AdvancedDisplay, EditCondition = "PathStrategy == EPGXPathStrategy::Profile"))
	FString ProfileFolderName = TEXT("Profiles");

	// ========================================================================
	// EN: Traceability
	// ES: Trazabilidad
	// ========================================================================

	/** EN: Trace configuration for automatic C++ operation tracing. Toggle from here, no code changes needed. / ES: Configuracion de traza para trazado automatico de operaciones C++. Toglear desde aqui, sin cambios de codigo. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save|Trace", meta = (AdvancedDisplay))
	FPGXTraceConfig TraceConfig;

	// ========================================================================
	// EN: Thumbnail (v1.1 - prepared, not yet implemented)
	// ES: Thumbnail (v1.1 - preparado, aun no implementado)
	// ========================================================================

	/** EN: Capture a screenshot when saving (v1.1) / ES: Capturar screenshot al guardar (v1.1) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save|Thumbnail", meta = (AdvancedDisplay))
	bool bCaptureThumbnail = false;

	/** EN: Thumbnail resolution / ES: Resolucion del thumbnail */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save|Thumbnail",
		meta = (AdvancedDisplay, EditCondition = "bCaptureThumbnail"))
	FIntPoint ThumbnailResolution = FIntPoint(320, 180);

	// ========================================================================
	// EN: Platform
	// ES: Plataforma
	// ========================================================================

	/** EN: Save provider class override (nullptr = default platform provider) / ES: Override de clase provider de guardado (nullptr = provider por defecto de plataforma) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save|Platform", meta = (AdvancedDisplay))
	TSubclassOf<UPGXSaveProvider> SaveProviderClass;

	// ========================================================================
	// EN: Debug & operability
	// ES: Debug y operabilidad
	// ========================================================================

	/** EN: Enable step-by-step pipeline logging for debugging (Development only, zero-cost in Shipping) / ES: Habilitar logging paso a paso del pipeline para debugging (solo Development, zero-cost en Shipping) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save|Debug", meta = (AdvancedDisplay))
	bool bVerboseSaveDebug = false;
};
