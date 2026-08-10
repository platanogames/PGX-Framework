// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

class UWorld;

#include "CoreMinimal.h"
#include "Base/PGXGameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Containers/Ticker.h"
#include "Interfaces/PGXTaggedRegistry.h"
#include "PGXSaveTypes.h"
#include "PGXSaveDelegates.h"
#include "PGXSaveSlotInfo.h"
#include "PGXSaveSubsystem.generated.h"

class UPGXSaveConfig;
class UPGXSaveGame;
class UPGXSaveProvider;
class IPGXSaveable;
class IConsoleObject;

/**
 * EN: Central save/load manager subsystem.
 *     Discovers UPGXSaveConfig DAs at initialization and builds an internal cache
 *     mapping GameplayTags to domain bindings for O(1) lookup. Orchestrates the
 *     full save/load pipeline: callbacks -> serialize -> compress -> checksum -> write.
 *
 *     All public API functions receive GameplayTags as identifiers. The subsystem
 *     resolves internally which dashboard, domain, and SaveGame class to use.
 *
 *     Lifecycle: GameInstance scope (persists across level transitions).
 *
 * ES: Subsistema central de gestion de guardado/carga.
 *     Descubre UPGXSaveConfig DAs en inicializacion y construye un cache interno
 *     mapeando GameplayTags a domain bindings para lookup O(1). Orquesta el pipeline
 *     completo de save/load: callbacks -> serializar -> comprimir -> checksum -> escribir.
 *
 *     Todas las funciones de la API publica reciben GameplayTags como identificadores.
 *     El subsistema resuelve internamente que dashboard, dominio y clase SaveGame usar.
 *
 *     Ciclo de vida: scope de GameInstance (persiste entre transiciones de nivel).
 */
UCLASS(BlueprintType)
class PGXSAVERUNTIME_API UPGXSaveSubsystem : public UPGXGameInstanceSubsystem, public IPGXTaggedRegistry
{
	GENERATED_BODY()

public:
	// ========================================================================
	// EN: USubsystem Interface
	// ES: Interfaz USubsystem
	// ========================================================================

	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	// ========================================================================
	// EN: Context operations (operate on an entire dashboard)
	// ES: Operaciones de contexto (operan sobre un dashboard completo)
	// ========================================================================

	/**
	 * EN: Save all domains in a context to the specified slot (synchronous).
	 * ES: Guardar todos los dominios de un contexto en el slot especificado (sincrono).
	 *
	 * @param ContextTag  EN: Tag identifying the SaveConfig DA / ES: Tag que identifica el DA de SaveConfig
	 * @param SlotName    EN: Target slot name (ignored for SingleSlot mode) / ES: Nombre del slot destino (ignorado para modo SingleSlot)
	 * @return            EN: Result of the operation / ES: Resultado de la operacion
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save")
	EPGXSaveResult SaveContext(FGameplayTag ContextTag, const FString& SlotName);

	/**
	 * EN: Load all domains in a context from the specified slot (synchronous).
	 * ES: Cargar todos los dominios de un contexto desde el slot especificado (sincrono).
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save")
	EPGXSaveResult LoadContext(FGameplayTag ContextTag, const FString& SlotName);

	/**
	 * EN: Delete a save slot and all its domain files.
	 * ES: Eliminar un slot de guardado y todos sus archivos de dominio.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save")
	EPGXSaveResult DeleteSlot(FGameplayTag ContextTag, const FString& SlotName);

	/**
	 * EN: Copy all domain files from one slot to another.
	 * ES: Copiar todos los archivos de dominio de un slot a otro.
	 *
	 * @param ContextTag      EN: Context to operate on / ES: Contexto sobre el que operar
	 * @param SourceSlotName  EN: Slot to copy from / ES: Slot desde el que copiar
	 * @param DestSlotName    EN: Slot to copy to / ES: Slot al que copiar
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save")
	EPGXSaveResult CopySlot(FGameplayTag ContextTag, const FString& SourceSlotName, const FString& DestSlotName);

	// ========================================================================
	// EN: Async context operations
	// ES: Operaciones asincronas de contexto
	// ========================================================================

	/**
	 * EN: Save all domains asynchronously. Serializes on GameThread, writes on background thread.
	 *     Broadcasts OnSaveCompleted when done.
	 * ES: Guardar todos los dominios asincronamente. Serializa en GameThread, escribe en hilo de fondo.
	 *     Dispara OnSaveCompleted al completar.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save")
	void SaveContextAsync(FGameplayTag ContextTag, const FString& SlotName);

	/**
	 * EN: Load all domains asynchronously. Reads on background thread, deserializes on GameThread.
	 *     Broadcasts OnLoadCompleted when done.
	 * ES: Cargar todos los dominios asincronamente. Lee en hilo de fondo, deserializa en GameThread.
	 *     Dispara OnLoadCompleted al completar.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save")
	void LoadContextAsync(FGameplayTag ContextTag, const FString& SlotName);

	// ========================================================================
	// EN: Quick Save / Load
	// ES: Guardado / Carga rapida
	// ========================================================================

	/** EN: Quick save the context using the configured QuickSave slot name / ES: Guardado rapido del contexto usando el nombre de slot QuickSave configurado */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save")
	EPGXSaveResult QuickSave(FGameplayTag ContextTag);

	/** EN: Quick load the context from the QuickSave slot / ES: Carga rapida del contexto desde el slot QuickSave */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save")
	EPGXSaveResult QuickLoad(FGameplayTag ContextTag);

	/** EN: Quick save asynchronously using configured QuickSave slot / ES: Guardado rapido asincrono usando slot QuickSave configurado */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save")
	void QuickSaveAsync(FGameplayTag ContextTag);

	/** EN: Quick load asynchronously from configured QuickSave slot / ES: Carga rapida asincrona desde slot QuickSave configurado */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save")
	void QuickLoadAsync(FGameplayTag ContextTag);

	// ========================================================================
	// EN: Slot queries
	// ES: Consultas de slots
	// ========================================================================

	/** EN: Get metadata for all slots in a context / ES: Obtener metadata de todos los slots en un contexto */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save")
	TArray<FPGXSaveSlotInfo> GetAllSlots(FGameplayTag ContextTag) const;

	/** EN: Get metadata for a specific slot / ES: Obtener metadata de un slot especifico */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save")
	FPGXSaveSlotInfo GetSlotInfo(FGameplayTag ContextTag, const FString& SlotName) const;

	/** EN: Check if a slot exists / ES: Comprobar si un slot existe */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save")
	bool DoesSlotExist(FGameplayTag ContextTag, const FString& SlotName) const;

	/** EN: Get the next available slot name following the pattern / ES: Obtener el siguiente nombre de slot disponible siguiendo el patron */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save")
	FString GetNextAvailableSlotName(FGameplayTag ContextTag) const;

	// ========================================================================
	// EN: Direct SaveGame access
	// ES: Acceso directo a SaveGame
	// ========================================================================

	/**
	 * EN: Get the active SaveGame instance for a domain. Creates it if it doesn't exist yet.
	 * ES: Obtener la instancia activa de SaveGame para un dominio. La crea si aun no existe.
	 *
	 * @param DomainTag  EN: Domain identifier / ES: Identificador de dominio
	 * @return           EN: The SaveGame instance (null if domain not found) / ES: La instancia de SaveGame (null si dominio no encontrado)
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save")
	UPGXSaveGame* GetSaveGame(FGameplayTag DomainTag);

	/**
	 * EN: Get the SaveGame instance cast to a specific subclass (C++ only).
	 * ES: Obtener la instancia de SaveGame cast a una subclase especifica (solo C++).
	 */
	template<typename T>
	T* GetSaveGameAs(FGameplayTag DomainTag);

	// ========================================================================
	// EN: Data access (convenience wrappers)
	// ES: Acceso a datos (wrappers de conveniencia)
	// ========================================================================

	/** EN: Check if a key exists in any typed map of the domain's SaveGame / ES: Comprobar si una key existe en algun mapa tipado del SaveGame del dominio */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save")
	bool HasData(FGameplayTag DomainTag, FName Key) const;

	/** EN: Clear all key-value data in a domain's SaveGame / ES: Limpiar todos los datos key-value en el SaveGame de un dominio */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save")
	void ClearDomain(FGameplayTag DomainTag);

	// ========================================================================
	// EN: IPGXSaveable registration (C++ objects participating in save/load)
	// ES: Registro de IPGXSaveable (objetos C++ que participan en save/load)
	// ========================================================================

	/**
	 * EN: Register an object implementing IPGXSaveable to receive save/load callbacks.
	 * ES: Registrar un objeto que implementa IPGXSaveable para recibir callbacks de save/load.
	 *
	 * @param Saveable   EN: The UObject implementing IPGXSaveable / ES: El UObject que implementa IPGXSaveable
	 * @param DomainTag  EN: The domain this saveable belongs to / ES: El dominio al que pertenece este saveable
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save")
	void RegisterSaveable(UObject* Saveable, FGameplayTag DomainTag);

	/** EN: Unregister a previously registered saveable / ES: Des-registrar un saveable previamente registrado */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save")
	void UnregisterSaveable(UObject* Saveable);

	// ========================================================================
	// EN: Active state
	// ES: Estado activo
	// ========================================================================

	/** EN: Get the currently active slot name for a context / ES: Obtener el nombre del slot activo actual para un contexto */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save")
	FString GetActiveSlotName(FGameplayTag ContextTag) const;

	/** EN: Set the active slot for a context (does NOT trigger load) / ES: Establecer el slot activo para un contexto (NO dispara carga) */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save")
	void SetActiveSlot(FGameplayTag ContextTag, const FString& SlotName);

	/** EN: Returns true if a save operation is currently in progress / ES: Retorna true si una operacion de guardado esta en progreso */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save")
	bool IsSaveInProgress() const { return bSaveInProgress; }

	/** EN: Returns true if a load operation is currently in progress / ES: Retorna true si una operacion de carga esta en progreso */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save")
	bool IsLoadInProgress() const { return bLoadInProgress; }

	// ========================================================================
	// EN: Auto-save control
	// ES: Control de auto-guardado
	// ========================================================================

	/** EN: Enable or disable auto-save for a context / ES: Habilitar o deshabilitar auto-guardado para un contexto */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save")
	void SetAutoSaveEnabled(FGameplayTag ContextTag, bool bEnabled);

	/** EN: Manually trigger an auto-save for a context / ES: Disparar manualmente un auto-guardado para un contexto */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save")
	void TriggerAutoSave(FGameplayTag ContextTag);

	/** EN: Check if auto-save is currently active for a context / ES: Comprobar si el auto-guardado esta activo para un contexto */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save")
	bool IsAutoSaveActive(FGameplayTag ContextTag) const;

	// ========================================================================
	// EN: Context queries
	// ES: Consultas de contexto
	// ========================================================================

	/** EN: Get all discovered context tags / ES: Obtener todos los tags de contexto descubiertos */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save")
	TArray<FGameplayTag> GetAllContextTags() const;

	/** EN: Get the config DA for a context / ES: Obtener el DA de config para un contexto */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save")
	const UPGXSaveConfig* GetContextConfig(FGameplayTag ContextTag) const;

	/** EN: Get the number of discovered contexts / ES: Obtener el numero de contextos descubiertos */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save")
	int32 GetContextCount() const { return DiscoveredConfigs.Num(); }

	// ========================================================================
	// EN: Debug snapshot and operation history
	// ES: Snapshot debug e historial de operaciones
	// ========================================================================

	/**
	 * EN: Build an aggregate debug snapshot of current subsystem state.
	 *     Read-only; safe to call from inspector, console, or automation tests.
	 *     RecentOperations slice is bounded by MaxOperationHistory.
	 * ES: Construir un snapshot debug agregado del estado actual del subsistema.
	 *     Read-only; seguro llamarlo desde inspector, consola o tests de automation.
	 *     El slice RecentOperations esta acotado por MaxOperationHistory.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save")
	FPGXSaveDebugSnapshot GetDebugSnapshot() const;

	/** EN: Maximum number of operation records retained in the ring buffer / ES: Numero maximo de records de operacion retenidos en el ring buffer */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Save")
	int32 GetMaxOperationHistory() const { return MaxOperationHistory; }

	// ========================================================================
	// EN: Delegates (Blueprint-assignable)
	// ES: Delegates (asignables desde Blueprint)
	// ========================================================================

	/** EN: Fired when a save operation completes / ES: Disparado cuando una operacion de guardado se completa */
	UPROPERTY(BlueprintAssignable, Category = "PGX|Save")
	FOnPGXSaveCompleted OnSaveCompleted;

	/** EN: Fired when a load operation completes / ES: Disparado cuando una operacion de carga se completa */
	UPROPERTY(BlueprintAssignable, Category = "PGX|Save")
	FOnPGXLoadCompleted OnLoadCompleted;

	/** EN: Fired when a slot is deleted / ES: Disparado cuando un slot es eliminado */
	UPROPERTY(BlueprintAssignable, Category = "PGX|Save")
	FOnPGXSlotDeleted OnSlotDeleted;

	/** EN: Fired when auto-save triggers / ES: Disparado cuando se activa el auto-guardado */
	UPROPERTY(BlueprintAssignable, Category = "PGX|Save")
	FOnPGXAutoSaveTriggered OnAutoSaveTriggered;

	/** EN: Fired to report save/load progress / ES: Disparado para reportar progreso de save/load */
	UPROPERTY(BlueprintAssignable, Category = "PGX|Save")
	FOnPGXSaveProgress OnSaveProgress;

	// EN: Native C++ delegates (no reflection overhead, for Slate/subsystem listeners)
	// ES: Delegates nativos C++ (sin overhead de reflexion, para listeners Slate/subsistema)
	FOnPGXSaveCompletedNative OnSaveCompletedNative;
	FOnPGXLoadCompletedNative OnLoadCompletedNative;
	FOnPGXSlotDeletedNative OnSlotDeletedNative;
	FOnPGXAutoSaveTriggeredNative OnAutoSaveTriggeredNative;

	// ========================================================================
	// EN: IPGXTaggedRegistry adoption — DomainBindings facade
	// ES: Adopcion IPGXTaggedRegistry — fachada de DomainBindings
	// ========================================================================

	/** EN: True if DomainBindings contains the tag / ES: True si DomainBindings contiene el tag. */
	bool HasEntryByTag(FGameplayTag Tag) const override;

	/** EN: Number of registered save domains in the bindings cache / ES: Numero de dominios de save registrados en el cache de bindings. */
	int32 GetCount() const override;

	/** EN: Snapshot of registered save-domain tags from the bindings cache / ES: Snapshot de tags de save-domain registrados del cache de bindings. */
	void GetSnapshot(TArray<FGameplayTag>& OutTags) const override;

#if WITH_EDITOR
	// ========================================================================
	// EN: Test injection API (editor only)
	// ES: API de inyeccion de test (solo editor)
	// ========================================================================

	/** EN: Inject a test config into discovered configs + caches (test harness) / ES: Inyectar un config de test en configs descubiertos + caches (harness de test) */
	void InjectTestConfig(UPGXSaveConfig* Config);

	/** EN: Remove all transient (test-injected) configs and their bindings / ES: Remover todos los configs transient (inyectados por test) y sus bindings */
	void ClearTestConfigs();
#endif

private:
	// ========================================================================
	// EN: Profile integration
	// ES: Integracion con Profile
	// ========================================================================

	/** EN: Apply constraints from the resolved project profile / ES: Aplicar restricciones del profile de proyecto resuelto */
	void ApplyProfileConstraints(const struct FPGXResolvedProfile& Profile);

	/** EN: Handle profile change at runtime / ES: Manejar cambio de profile en runtime */
	void HandleProfileChanged(const struct FPGXResolvedProfile& OldProfile, const struct FPGXResolvedProfile& NewProfile);

	// ========================================================================
	// EN: Discovery & initialization
	// ES: Descubrimiento e inicializacion
	// ========================================================================

	/** EN: Scan AssetRegistry for all UPGXSaveConfig DAs / ES: Escanear AssetRegistry por todos los DAs UPGXSaveConfig */
	void DiscoverSaveConfigs();

	/** EN: Build the domain binding cache from discovered configs / ES: Construir el cache de domain bindings desde configs descubiertas */
	void BuildDomainCache();

	/** EN: Create the platform save provider instance / ES: Crear la instancia del provider de guardado de plataforma */
	void CreateProvider();

	// ========================================================================
	// EN: Console commands
	// ES: Comandos de consola
	// ========================================================================

	/** EN: Register all pgx.save.* console commands / ES: Registrar todos los comandos de consola pgx.save.* */
	/** EN: Unregister console commands / ES: Des-registrar comandos de consola */
	// ========================================================================
	// EN: Auto-save timer management
	// ES: Gestion de timers de auto-guardado
	// ========================================================================

	/** EN: Start auto-save ticker for a context (FTSTicker, world-independent) / ES: Iniciar ticker de auto-guardado para un contexto (FTSTicker, independiente de World) */
	void StartAutoSaveTimer(FGameplayTag ContextTag);

	/** EN: Stop auto-save ticker for a context / ES: Detener ticker de auto-guardado para un contexto */
	void StopAutoSaveTimer(FGameplayTag ContextTag);

	/** EN: Start auto-save timers for all contexts that have it enabled / ES: Iniciar timers de auto-guardado para todos los contextos que lo tengan habilitado */
	void StartAllAutoSaveTimers();

	/** EN: Stop all auto-save timers / ES: Detener todos los timers de auto-guardado */
	void StopAllAutoSaveTimers();

	// ========================================================================
	// EN: Internal helpers
	// ES: Helpers internos
	// ========================================================================

	/** EN: Find config DA by context tag / ES: Encontrar DA de config por context tag */
	UPGXSaveConfig* FindConfigByContextTag(FGameplayTag ContextTag) const;

	/** EN: Find domain binding by domain tag (mutable) / ES: Encontrar domain binding por domain tag (mutable) */
	FPGXDomainBinding* FindBindingByDomainTag(FGameplayTag DomainTag);

	/** EN: Find domain binding by domain tag (const) / ES: Encontrar domain binding por domain tag (const) */
	const FPGXDomainBinding* FindBindingByDomainTag(FGameplayTag DomainTag) const;

	/** EN: Ensure a SaveGame instance exists for a binding, create if needed / ES: Asegurar que exista instancia SaveGame para un binding, crear si es necesario */
	UPGXSaveGame* EnsureSaveGameInstance(FPGXDomainBinding& Binding);

	/** EN: Notify all registered IPGXSaveables for a domain before save / ES: Notificar a todos los IPGXSaveable registrados para un dominio antes de guardar */
	void NotifySaveablesPreSave(FGameplayTag DomainTag, UPGXSaveGame* SaveGame);

	/** EN: Notify all registered IPGXSaveables for a domain after load / ES: Notificar a todos los IPGXSaveable registrados para un dominio despues de cargar */
	void NotifySaveablesPostLoad(FGameplayTag DomainTag, UPGXSaveGame* SaveGame);

	/** EN: Build SlotInfo metadata for a slot being saved / ES: Construir metadata SlotInfo para un slot que se esta guardando */
	FPGXSaveSlotInfo BuildSlotInfoForSave(const UPGXSaveConfig* Config, const FString& SlotName, int32 DomainCount, int64 TotalBytes) const;

	// ========================================================================
	// EN: State
	// ES: Estado
	// ========================================================================

	/** EN: All discovered SaveConfig DAs (strong GC references) / ES: Todos los DAs SaveConfig descubiertos (referencias fuertes de GC) */
	UPROPERTY()
	TArray<TObjectPtr<UPGXSaveConfig>> DiscoveredConfigs;

	/** EN: Active platform save provider / ES: Provider de plataforma activo */
	UPROPERTY()
	TObjectPtr<UPGXSaveProvider> ActiveProvider;

	/** EN: Domain bindings cache (domain tag -> binding with active SaveGame instance) / ES: Cache de domain bindings (domain tag -> binding con instancia activa de SaveGame) */
	UPROPERTY()
	TMap<FGameplayTag, FPGXDomainBinding> DomainBindings;

	/** EN: Context tag -> Config DA lookup (no ownership, refs held by DiscoveredConfigs) / ES: Context tag -> lookup de Config DA (sin ownership, refs mantenidas por DiscoveredConfigs) */
	TMap<FGameplayTag, UPGXSaveConfig*> ContextConfigMap;

	/** EN: Active slot name per context / ES: Nombre del slot activo por contexto */
	TMap<FGameplayTag, FString> ActiveSlots;

	/** EN: Registered IPGXSaveable objects per domain tag / ES: Objetos IPGXSaveable registrados por domain tag */
	TMultiMap<FGameplayTag, TWeakObjectPtr<UObject>> RegisteredSaveables;

	/** EN: Auto-save rotation index per context / ES: Indice de rotacion de auto-guardado por contexto */
	TMap<FGameplayTag, int32> AutoSaveRotationIndex;

	/** EN: True while a save operation is executing / ES: True mientras una operacion de guardado esta ejecutando */
	bool bSaveInProgress = false;

	/** EN: True while a load operation is executing / ES: True mientras una operacion de carga esta ejecutando */
	bool bLoadInProgress = false;

	/** EN: Registered console commands for cleanup / ES: Comandos de consola registrados para cleanup */
	/** EN: Auto-save ticker handles per context (FTSTicker, world-independent) / ES: Handles de ticker de auto-guardado por contexto (FTSTicker, independiente de World) */
	TMap<FGameplayTag, FTSTicker::FDelegateHandle> AutoSaveTickerHandles;

	/** EN: Cached singleton instance for fast access / ES: Instancia singleton cacheada para acceso rapido */
	static TWeakObjectPtr<UPGXSaveSubsystem> CachedInstance;

	// ========================================================================
	// EN: Operation history ring buffer populated by completed operations
	//     Capacity resolved at Initialize from UPGXSaveSettings::MaxOperationHistory
	//     (data-driven through project settings).
	// ES: Ring buffer de historial de operaciones poblado por operaciones completadas
	//     Capacidad resuelta al Initialize desde UPGXSaveSettings::MaxOperationHistory
	//     (data-driven mediante project settings).
	// ========================================================================

	/** EN: Resolved cap from UPGXSaveSettings (set at Initialize) / ES: Cap resuelto desde UPGXSaveSettings (set al Initialize) */
	int32 MaxOperationHistory = 0;

	/** EN: Ring buffer of recent operation records (chronological, oldest first) / ES: Ring buffer de records de operacion recientes (cronologico, mas antiguo primero) */
	TArray<FPGXSaveOperationRecord> OperationHistory;

private:
	friend class FPGXSaveRuntimeModule;
	void ExecuteConsoleCommand(const FString& CommandName, const TArray<FString>& Args, UWorld* World);
};

// ============================================================================
// EN: Template implementations
// ES: Implementaciones de templates
// ============================================================================

template<typename T>
T* UPGXSaveSubsystem::GetSaveGameAs(FGameplayTag DomainTag)
{
	return Cast<T>(GetSaveGame(DomainTag));
}
