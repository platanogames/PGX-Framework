// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once

class UWorld;
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Logging/PGXLogTypes.h"
#include "Logging/PGXLogRingBuffer.h"
#include "PGXLogSubsystem.generated.h"

class UPGXLogConfig;
class UPGXLogDomainConfig;
class UPGXLogDomainRendererBase;

/**
 * EN: Centralized log management subsystem.
 *     Provides: structured log capture, ring buffer storage, runtime filtering,
 *     per-category verbosity control, on-screen display, async persistence,
 *     delegate broadcast, and console commands.
 *
 *     Access: Use PGX_LOG() macros from gameplay code. Never call methods directly.
 *     Static access via GetCached() is reserved for internal macro/builder use.
 *
 * ES: Subsistema de gestion centralizada de logs.
 *     Proporciona: captura de logs estructurados, almacenamiento en ring buffer,
 *     filtrado en runtime, control de verbosity por categoria, display en pantalla,
 *     persistencia asincrona, broadcast de delegados, y comandos de consola.
 *
 *     Acceso: Usar macros PGX_LOG() desde codigo gameplay. Nunca llamar metodos directamente.
 *     Acceso estatico via GetCached() reservado para uso interno de macros/builder.
 */
UCLASS()
class PGXCORERUNTIME_API UPGXLogSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface

	// ═══════════════════════════════════════
	// Static Access (for macros/builder)
	// Acceso Estatico (para macros/builder)
	// ═══════════════════════════════════════

	/**
	 * EN: Fast cached access for macros. Returns nullptr if subsystem not yet initialized.
	 * ES: Acceso cacheado rapido para macros. Retorna nullptr si el subsistema no esta inicializado.
	 */
	static UPGXLogSubsystem* GetCached();

	// ═══════════════════════════════════════
	// Entry (called by macro/builder)
	// Entrada (llamada por macro/builder)
	// ═══════════════════════════════════════

	/**
	 * EN: Submit a structured log entry. Called by PGX_LOG macros and FPGXLogEntryBuilder.
	 *     Flow: filter check → ring buffer push → update counters → screen → broadcast.
	 * ES: Enviar una entrada de log estructurada. Llamada por macros PGX_LOG y FPGXLogEntryBuilder.
	 *     Flujo: check de filtro → push al ring buffer → actualizar contadores → pantalla → broadcast.
	 */
	void AddEntry(FPGXLogEntry&& Entry);

	// ═══════════════════════════════════════
	// Query (read-only, for Widget Developer)
	// Consulta (solo lectura, para Widget Developer)
	// ═══════════════════════════════════════

	/** EN: Get all entries from current session / ES: Obtener todas las entradas de la sesion actual */
	TArray<FPGXLogEntry> GetCurrentSessionEntries() const;

	/** EN: Get current session metadata / ES: Obtener metadatos de la sesion actual */
	FPGXLogSession GetCurrentSession() const;

	/** EN: Get total entry count in ring buffer / ES: Obtener conteo total en el ring buffer */
	int32 GetEntryCount() const;

	/**
	 * EN: Get filtered entries by verbosity and/or category.
	 * ES: Obtener entradas filtradas por verbosity y/o categoria.
	 */
	TArray<FPGXLogEntry> GetEntriesFiltered(
		EPGXLogVerbosity MinLevel = EPGXLogVerbosity::Verbose,
		FName CategoryFilter = NAME_None) const;

	/**
	 * EN: Get entries filtered by domain tag.
	 * ES: Obtener entradas filtradas por tag de dominio.
	 */
	TArray<FPGXLogEntry> GetEntriesForDomain(const FGameplayTag& DomainTag) const;

	// ═══════════════════════════════════════
	// Domain Registry (v3.0)
	// Registro de Dominios (v3.0)
	// ═══════════════════════════════════════

	/**
	 * EN: Get all registered domain configs. Returns the auto-discovered DAs.
	 * ES: Obtener todas las configs de dominio registradas.
	 */
	const TMap<FGameplayTag, TObjectPtr<const UPGXLogDomainConfig>>& GetAllDomains() const { return DomainRegistry; }

	/**
	 * EN: Find a domain config by tag. Returns nullptr if not registered.
	 * ES: Encontrar config de dominio por tag.
	 */
	const UPGXLogDomainConfig* FindDomainConfig(const FGameplayTag& DomainTag) const;

	/**
	 * EN: Get or create a cached renderer instance for a domain tag.
	 *     Returns the generic renderer if no domain-specific one is configured.
	 * ES: Obtener o crear una instancia cacheada de renderer para un tag de dominio.
	 */
	UPGXLogDomainRendererBase* GetRendererForDomain(const FGameplayTag& DomainTag);

	/**
	 * EN: Get the default (generic) renderer instance.
	 * ES: Obtener la instancia del renderer default (generico).
	 */
	UPGXLogDomainRendererBase* GetGenericRenderer();

	/** EN: Fired when a new domain is registered / ES: Disparado cuando un nuevo dominio se registra */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnPGXLogDomainRegistered, const FGameplayTag& /*DomainTag*/);
	FOnPGXLogDomainRegistered OnDomainRegistered;

	// ═══════════════════════════════════════
	// IO (async)
	// ═══════════════════════════════════════

	/**
	 * EN: Export current buffer to JSON asynchronously.
	 *     Pattern: snapshot (short lock) → release → serialize on background thread.
	 * ES: Exportar buffer actual a JSON asincronamente.
	 *     Patron: snapshot (lock corto) → release → serializar en hilo background.
	 */
	void ExportToJsonAsync(const FString& Path = TEXT(""));

	/** EN: Slot persistence is unavailable in this preview; use JSON export. / ES: La persistencia por slots no esta disponible; usa la exportacion JSON. */
	void SaveToSlotAsync();

	/** EN: Returns false because slot loading is unavailable in this preview. / ES: Devuelve false porque la carga por slots no esta disponible. */
	bool LoadFromSlot(int32 SessionId = -1);

	// ═══════════════════════════════════════
	// Runtime Filtering / Filtrado en Runtime
	// ═══════════════════════════════════════

	/** EN: Set global minimum verbosity / ES: Establecer verbosity minima global */
	void SetGlobalMinVerbosity(EPGXLogVerbosity Level);

	/** EN: Set per-category verbosity override / ES: Establecer override de verbosity por categoria */
	void SetCategoryVerbosity(FName Category, EPGXLogVerbosity Level);

	/** EN: Toggle on-screen display / ES: Alternar display en pantalla */
	void SetScreenPrintEnabled(bool bEnabled);

	// ═══════════════════════════════════════
	// Config / Configuracion
	// ═══════════════════════════════════════

	/**
	 * EN: Apply a Config DA profile. Called by UPGXConfigSubsystem after loading.
	 * ES: Aplicar un perfil Config DA. Llamado por UPGXConfigSubsystem despues de cargar.
	 */
	void ApplyConfig(const UPGXLogConfig* Config);

	// ═══════════════════════════════════════
	// Delegates / Delegados
	// ═══════════════════════════════════════

	/** EN: Fired when a new log entry is added (Blueprint) / ES: Disparado cuando se agrega una nueva entrada (Blueprint) */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPGXLogEntryAdded, const FPGXLogEntry&, Entry);

	UPROPERTY(BlueprintAssignable, Category = "PGX|Log")
	FOnPGXLogEntryAdded OnLogEntryAdded;

	/**
	 * EN: Native C++ delegate for log entry notifications. More efficient than dynamic delegate.
	 *     Used by Slate editor widgets (SPGXLogViewerTab) that can't easily bind to DYNAMIC_MULTICAST.
	 * ES: Delegado nativo C++ para notificaciones de entradas de log. Mas eficiente que delegado dinamico.
	 *     Usado por widgets Slate del editor (SPGXLogViewerTab) que no pueden enlazar facilmente a DYNAMIC_MULTICAST.
	 */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnPGXLogEntryAddedNative, const FPGXLogEntry&);
	FOnPGXLogEntryAddedNative OnLogEntryAddedNative;

	/** EN: Fired when session state changes / ES: Disparado cuando cambia el estado de sesion */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPGXSessionUpdated);

	UPROPERTY(BlueprintAssignable, Category = "PGX|Log")
	FOnPGXSessionUpdated OnSessionUpdated;

private:
	// ═══════════════════════════════════════
	// Profile Integration / Integracion con Profile
	// ═══════════════════════════════════════

	void ApplyProfileConstraints(const struct FPGXResolvedProfile& Profile);
	void HandleProfileChanged(const struct FPGXResolvedProfile& OldProfile, const struct FPGXResolvedProfile& NewProfile);

	// ═══════════════════════════════════════
	// Internal Methods / Metodos Internos
	// ═══════════════════════════════════════
	void PrintToScreen(const FPGXLogEntry& Entry);
	void StartNewSession();
	void EndCurrentSession();
	void ScheduleAutoSave(float IntervalSeconds);
	FString GetDefaultJsonExportPath() const;

	/**
	 * EN: Check if an entry passes the current filter configuration.
	 * ES: Verificar si una entrada pasa la configuracion actual de filtros.
	 */
	bool PassesFilter(const FPGXLogEntry& Entry) const;

	// ═══════════════════════════════════════
	// State / Estado
	// ═══════════════════════════════════════

	FPGXLogRingBuffer RingBuffer;
	FPGXLogSession CurrentSession;
	/** EN: Protects session counters for thread-safe AddEntry / ES: Protege contadores de sesion para AddEntry thread-safe */
	FCriticalSection SessionCounterLock;
	TMap<FName, EPGXLogVerbosity> CategoryVerbosityOverrides;
	EPGXLogVerbosity GlobalMinVerbosity = EPGXLogVerbosity::Verbose;
	bool bScreenPrintEnabled = true;
	bool bBroadcastEnabled = true;
	EPGXLogVerbosity BroadcastMinVerbosity = EPGXLogVerbosity::Info;
	EPGXLogVerbosity ScreenMinVerbosity = EPGXLogVerbosity::Warning;
	float ScreenDisplayDuration = 5.0f;
	int32 NextSessionId = 1;

	// EN: Config reference / ES: Referencia a configuracion
	UPROPERTY()
	TObjectPtr<const UPGXLogConfig> ActiveConfig;

	// EN: Timer for auto-save / ES: Timer para auto-guardado
	FTimerHandle AutoSaveTimerHandle;

	// EN: Static cache for fast macro access / ES: Cache estatico para acceso rapido de macros
	static TWeakObjectPtr<UPGXLogSubsystem> CachedInstance;

	// EN: Registered console commands / ES: Comandos de consola registrados
	// EN: Category whitelist (empty = all) / ES: Whitelist de categorias (vacio = todas)
	TArray<FName> CategoryWhitelist;

	// ═══════════════════════════════════════
	// Domain Registry State (v3.0)
	// ═══════════════════════════════════════

	void DiscoverDomainConfigs();
	void RegisterDomain(const UPGXLogDomainConfig* Config);
	void AutoPopulateDomainTag(FPGXLogEntry& Entry) const;

	/** EN: DomainTag → Config DA mapping / ES: Mapeo DomainTag → Config DA */
	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<const UPGXLogDomainConfig>> DomainRegistry;

	/** EN: Category → DomainTag auto-infer map (built from DA AutoInferCategories) / ES: Mapa auto-infer de Category → DomainTag */
	TMap<FName, FGameplayTag> CategoryToDomainMap;

	/** EN: Cached renderer instances per domain / ES: Instancias de renderer cacheadas por dominio */
	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UPGXLogDomainRendererBase>> RendererCache;

	/** EN: Generic fallback renderer / ES: Renderer generico de fallback */
	UPROPERTY()
	TObjectPtr<UPGXLogDomainRendererBase> GenericRendererInstance;

private:
	friend class FPGXCoreRuntimeModule;
	void ExecuteConsoleCommand(const FString& CommandName, const TArray<FString>& Args, UWorld* World);
};
