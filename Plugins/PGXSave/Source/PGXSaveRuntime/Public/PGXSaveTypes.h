// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "PGXSaveTypes.generated.h"

class UPGXSaveGame;
class UPGXSaveConfig;

// ============================================================================
// EN: Enumerations
// ES: Enumeraciones
// ============================================================================

/**
 * EN: Defines how the save context manages its slots.
 * ES: Define como el contexto de guardado gestiona sus slots.
 */
UENUM(BlueprintType)
enum class EPGXSaveMode : uint8
{
	/** EN: One save, always overwritten / ES: Un guardado, siempre sobreescrito */
	SingleSlot		UMETA(DisplayName = "Single Slot"),

	/** EN: Multiple named slots managed by the player / ES: Multiples slots con nombre gestionados por el jugador */
	MultiSlot		UMETA(DisplayName = "Multi Slot"),

	/** EN: New save per session with auto-generated timestamp / ES: Nuevo guardado por sesion con timestamp auto-generado */
	SessionBased	UMETA(DisplayName = "Session Based")
};

/**
 * EN: Type of save operation being performed.
 * ES: Tipo de operacion de guardado siendo ejecutada.
 */
UENUM(BlueprintType)
enum class EPGXSaveOperation : uint8
{
	Save,
	Load,
	Delete,
	Copy,
	QuickSave,
	QuickLoad,
	AutoSave
};

/**
 * EN: Result of a save/load operation.
 * ES: Resultado de una operacion de guardado/carga.
 */
UENUM(BlueprintType)
enum class EPGXSaveResult : uint8
{
	/** EN: Operation completed successfully / ES: Operacion completada con exito */
	Success,

	/** EN: Operation failed (generic) / ES: Operacion fallida (generico) */
	Failed,

	/** EN: Requested slot does not exist / ES: El slot solicitado no existe */
	SlotNotFound,

	/** EN: Maximum slot count reached / ES: Numero maximo de slots alcanzado */
	SlotFull,

	/** EN: Save data failed integrity check / ES: Los datos fallaron la validacion de integridad */
	Corrupted,

	/** EN: Save version does not match current / ES: La version del guardado no coincide con la actual */
	VersionMismatch,

	/** EN: Another operation is already in progress / ES: Otra operacion ya esta en progreso */
	InProgress,

	/** EN: Operation was cancelled / ES: La operacion fue cancelada */
	Cancelled,

	/** EN: Platform save provider returned an error / ES: El proveedor de plataforma retorno un error */
	ProviderError,

	/** EN: The specified domain tag was not found in any config / ES: El tag de dominio no se encontro en ninguna config */
	DomainNotFound,

	/** EN: The specified context tag was not found / ES: El tag de contexto no se encontro */
	ContextNotFound
};

// ============================================================================
// EN: Structures
// ES: Estructuras
// ============================================================================

/**
 * EN: Defines a save domain entry within a SaveConfig dashboard.
 *     Each entry maps a GameplayTag to a SaveGame class that will be
 *     instantiated and managed by the save subsystem.
 *
 * ES: Define una entrada de dominio de guardado dentro de un dashboard SaveConfig.
 *     Cada entrada mapea un GameplayTag a una clase SaveGame que sera
 *     instanciada y gestionada por el subsistema de guardado.
 */
USTRUCT(BlueprintType)
struct PGXSAVERUNTIME_API FPGXSaveDomainEntry
{
	GENERATED_BODY()

	/** EN: Tag that uniquely identifies this domain / ES: Tag que identifica unicamente este dominio */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save", meta = (Categories = "PGX.Save"))
	FGameplayTag DomainTag;

	/** EN: SaveGame class to instantiate for this domain (BP or C++) / ES: Clase SaveGame a instanciar para este dominio (BP o C++) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save")
	TSubclassOf<UPGXSaveGame> SaveGameClass;

	/** EN: Display name for editor and debug / ES: Nombre para display en editor y debug */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save")
	FText DisplayName;

	/** EN: If true, context load fails if this domain cannot load / ES: Si true, la carga del contexto falla si este dominio no puede cargar */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Save")
	bool bRequired = true;
};

/**
 * EN: Internal binding that maps a domain tag to its owning config
 *     and active SaveGame instance. Not exposed to Blueprint.
 *
 * ES: Binding interno que mapea un tag de dominio a su config propietaria
 *     e instancia activa de SaveGame. No expuesto a Blueprint.
 */
USTRUCT()
struct PGXSAVERUNTIME_API FPGXDomainBinding
{
	GENERATED_BODY()

	/** EN: The config DataAsset that owns this domain / ES: El DataAsset config que posee este dominio */
	UPROPERTY()
	TWeakObjectPtr<UPGXSaveConfig> OwningConfig;

	/** EN: Copy of the domain entry from the config / ES: Copia de la entrada de dominio desde la config */
	UPROPERTY()
	FPGXSaveDomainEntry DomainEntry;

	/** EN: Active SaveGame instance (null until first load/create) / ES: Instancia activa de SaveGame (null hasta primer load/create) */
	UPROPERTY()
	TObjectPtr<UPGXSaveGame> ActiveInstance = nullptr;
};

/**
 * EN: Record of a backup transaction for the transaction log (.ini).
 * ES: Registro de una transaccion de backup para el log de transacciones (.ini).
 */
USTRUCT()
struct PGXSAVERUNTIME_API FPGXBackupEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FString SlotName;

	UPROPERTY()
	FDateTime Timestamp;

	UPROPERTY()
	FString Checksum;

	UPROPERTY()
	EPGXSaveOperation Operation = EPGXSaveOperation::Save;

	UPROPERTY()
	EPGXSaveResult Result = EPGXSaveResult::Success;

	UPROPERTY()
	FString FilePath;
};

/**
 * EN: Per-operation diagnostic record. Captures the lifecycle of save, load,
 *     delete, and copy operations for the inspector, bounded history, and tests.
 * ES: Registro diagnostico por operacion. Captura el ciclo de vida de save, load,
 *     delete y copy para el inspector, el historial acotado y los tests.
 */
USTRUCT(BlueprintType)
struct PGXSAVERUNTIME_API FPGXSaveOperationRecord
{
	GENERATED_BODY()

	/** EN: Stable identifier for this operation instance / ES: Identificador estable para esta instancia de operacion */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Operation")
	FGuid OperationHandle;

	/** EN: Context tag this operation belongs to / ES: Tag de contexto al que pertenece esta operacion */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Operation")
	FGameplayTag ContextTag;

	/** EN: Slot name targeted by the operation / ES: Nombre del slot al que apunta la operacion */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Operation")
	FString SlotName;

	/** EN: Operation kind (Save / Load / Delete / Copy / Quick* / AutoSave) / ES: Tipo de operacion (Save / Load / Delete / Copy / Quick* / AutoSave) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Operation")
	EPGXSaveOperation Operation = EPGXSaveOperation::Save;

	/**
	 * EN: Pipeline phase tag (PGX.Save.Operation.Phase.* or PGX.Save.Result.*).
	 *     Empty until the subsystem stamps a phase. Used for inspector + tests.
	 * ES: Tag de fase del pipeline (PGX.Save.Operation.Phase.* o PGX.Save.Result.*).
	 *     Vacio hasta que el subsystem estampa una fase. Usado por inspector + tests.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Operation")
	FGameplayTag PhaseTag;

	/** EN: Final typed result (Success or specific failure reason) / ES: Resultado tipado final (Success o razon de fallo especifica) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Operation")
	EPGXSaveResult Result = EPGXSaveResult::Failed;

	/** EN: GameThread-monotonic seconds at operation start / ES: Segundos GameThread-monotonic al inicio de la operacion */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Operation")
	double StartTime = 0.0;

	/** EN: GameThread-monotonic seconds at operation end (0 while in-flight) / ES: Segundos GameThread-monotonic al final de la operacion (0 mientras en vuelo) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Operation")
	double EndTime = 0.0;

	/** EN: True while the operation has not reached terminal phase / ES: True mientras la operacion no haya alcanzado fase terminal */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Operation")
	bool bInFlight = false;

	/** EN: Optional caller provenance string (subsystem name, system, automation test id) / ES: String opcional de provenance del caller (nombre de subsystem, sistema, id de test de automation) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Operation")
	FString CallerProvenance;

	/** EN: Bytes written/read by this operation (0 if not measured) / ES: Bytes escritos/leidos por esta operacion (0 si no medido) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Operation")
	int64 TotalBytes = 0;

	/** EN: Domains touched by this operation (in deterministic config order) / ES: Dominios tocados por esta operacion (en orden determinista de config) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Operation")
	TArray<FGameplayTag> DomainsTouched;
};

/**
 * EN: Aggregate snapshot of save-subsystem runtime state for inspector, debug snapshot
 *     console command (`pgx.save.status`) and tests. Read-only at consume time.
 *     Fields remain extensible without breaking existing callers.
 *
 * ES: Snapshot agregado del estado runtime del subsistema de save para inspector, comando
 *     debug snapshot (`pgx.save.status`) y tests. Read-only al consumir.
 *     Los campos permanecen extensibles sin romper callers existentes.
 */
USTRUCT(BlueprintType)
struct PGXSAVERUNTIME_API FPGXSaveDebugSnapshot
{
	GENERATED_BODY()

	/** EN: Total contexts discovered at initialization / ES: Total de contextos descubiertos en inicializacion */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Debug")
	int32 ContextCount = 0;

	/** EN: Total domain bindings cached / ES: Total de domain bindings cacheados */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Debug")
	int32 DomainBindingCount = 0;

	/** EN: Active-slot map snapshot (context tag -> slot name) / ES: Snapshot del mapa de slot activo (context tag -> nombre de slot) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Debug")
	TMap<FGameplayTag, FString> ActiveSlotsByContext;

	/** EN: True while any save operation is in progress / ES: True mientras cualquier operacion de save este en progreso */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Debug")
	bool bSaveInProgress = false;

	/** EN: True while any load operation is in progress / ES: True mientras cualquier operacion de load este en progreso */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Debug")
	bool bLoadInProgress = false;

	/** EN: Registered IPGXSaveable count (across all domains) / ES: Total de IPGXSaveable registrados (atravesando todos los dominios) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Debug")
	int32 RegisteredSaveableCount = 0;

	/** EN: Class name of the active provider (or empty if none bound) / ES: Nombre de clase del provider activo (o vacio si ninguno bound) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Debug")
	FString ActiveProviderClassName;

	/** EN: Last N operation records in chronological order (oldest first) / ES: Ultimos N records de operacion en orden cronologico (mas antiguo primero) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Debug")
	TArray<FPGXSaveOperationRecord> RecentOperations;

	/** EN: GameThread-monotonic seconds at the moment this snapshot was taken / ES: Segundos GameThread-monotonic en el momento que se tomo este snapshot */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Save|Debug")
	double SnapshotTime = 0.0;
};

// ============================================================================
// DataTable Row Structs — Config Resolution via Project Settings
// ============================================================================

/**
 * EN: DataTable row for save context resolution via Project Settings.
 *     Maps a context GameplayTag to a UPGXSaveConfig DA.
 * ES: Fila de DataTable para resolucion de contexto de save via Project Settings.
 *     Mapea un GameplayTag de contexto a un DA UPGXSaveConfig.
 */
USTRUCT(BlueprintType)
struct PGXSAVERUNTIME_API FPGXSaveContextRow : public FTableRowBase
{
	GENERATED_BODY()

	/** EN: Context tag for this save config / ES: Tag de contexto para este config de save */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Save")
	FGameplayTag ContextTag;

	/** EN: Save config DA / ES: DA de config de save */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Save")
	TSoftObjectPtr<UPGXSaveConfig> ConfigRef;

	/** EN: Optional description / ES: Descripcion opcional */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Save")
	FText Description;
};
