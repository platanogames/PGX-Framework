// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXProfileTypes.generated.h"

// ============================================================================
// EN: Profile Types for PGX Project Profile System.
//     Defines the 5-layer model: Identity, Capabilities, Policies, Budgets, Features.
//     These types form the "constitution" of the framework — the single source of truth
//     that governs WHAT is allowed, HOW it should be done, and WHAT the limits are.
//
// ES: Tipos de Profile para el Sistema de Profile de Proyecto PGX.
//     Define el modelo de 5 capas: Identity, Capabilities, Policies, Budgets, Features.
//     Estos tipos forman la "constitucion" del framework — la fuente unica de verdad
//     que gobierna QUE esta permitido, COMO debe hacerse, y CUALES son los limites.
// ============================================================================

// ── Enums ───────────────────────────────────────────────────────────────────

/** EN: Project operational mode / ES: Modo operativo del proyecto */
UENUM(BlueprintType)
enum class EPGXProjectMode : uint8
{
	/** EN: Standard game project / ES: Proyecto de juego estandar */
	Game,

	/** EN: Virtual reality project / ES: Proyecto de realidad virtual */
	VR,

	/** EN: Enterprise/visualization application / ES: Aplicacion empresarial/visualizacion */
	Enterprise,

	/** EN: Tooling/editor extension project / ES: Proyecto de herramientas/extension de editor */
	Tooling
};

/** EN: Target platform for profile resolution / ES: Plataforma objetivo para resolucion de profile */
UENUM(BlueprintType)
enum class EPGXTargetPlatform : uint8
{
	/** EN: Desktop PC (Windows/Linux/Mac) / ES: PC de escritorio (Windows/Linux/Mac) */
	PC,

	/** EN: PlayStation consoles / ES: Consolas PlayStation */
	Console_PS,

	/** EN: Xbox consoles / ES: Consolas Xbox */
	Console_Xbox,

	/** EN: Nintendo Switch / ES: Nintendo Switch */
	Console_Switch,

	/** EN: iOS mobile devices / ES: Dispositivos moviles iOS */
	Mobile_iOS,

	/** EN: Android mobile devices / ES: Dispositivos moviles Android */
	Mobile_Android,

	/** EN: Extended reality devices (VR/AR/MR) / ES: Dispositivos de realidad extendida (VR/AR/MR) */
	XR
};

/** EN: Build configuration context / ES: Contexto de configuracion de build */
UENUM(BlueprintType)
enum class EPGXBuildContext : uint8
{
	/** EN: Development build with full debugging / ES: Build de desarrollo con debugging completo */
	Development,

	/** EN: Test/staging build / ES: Build de test/staging */
	Test,

	/** EN: Final shipping build / ES: Build final de shipping */
	Shipping
};

/** EN: Restriction level for profile enforcement / ES: Nivel de restriccion para enforcement del profile */
UENUM(BlueprintType)
enum class EPGXRestrictionLevel : uint8
{
	/** EN: Strict enforcement — violations produce errors / ES: Enforcement estricto — violaciones producen errores */
	Strict,

	/** EN: Relaxed enforcement — violations produce warnings / ES: Enforcement relajado — violaciones producen warnings */
	Relaxed
};

/** EN: Persistence backend for system data / ES: Backend de persistencia para datos del sistema */
UENUM(BlueprintType)
enum class EPGXPersistenceBackend : uint8
{
	/** EN: INI file-based persistence / ES: Persistencia basada en archivos INI */
	INI,

	/** EN: Save data system persistence / ES: Persistencia via sistema de save data */
	SaveData,

	/** EN: Hybrid INI + SaveData / ES: Hibrido INI + SaveData */
	Hybrid,

	/** EN: Persistence disabled / ES: Persistencia deshabilitada */
	Disabled
};

/** EN: Profile resolution state / ES: Estado de resolucion del profile */
UENUM(BlueprintType)
enum class EPGXProfileState : uint8
{
	/** EN: Profile not yet resolved / ES: Profile aun no resuelto */
	Unresolved,

	/** EN: Profile resolved and active / ES: Profile resuelto y activo */
	Resolved,

	/** EN: Profile is changing (re-resolution in progress) / ES: Profile cambiando (re-resolucion en progreso) */
	Changing,

	/** EN: Profile locked (Shipping build, immutable) / ES: Profile bloqueado (build Shipping, inmutable) */
	Locked
};

/** EN: Feature availability policy / ES: Politica de disponibilidad de feature */
UENUM(BlueprintType)
enum class EPGXFeaturePolicy : uint8
{
	/** EN: Feature is allowed and can be used / ES: Feature permitido y puede usarse */
	Allowed,

	/** EN: Feature is not allowed / ES: Feature no permitido */
	Disallowed,

	/** EN: Feature requires a fallback path / ES: Feature requiere una ruta de fallback */
	FallbackRequired
};

// ── Structs — Layer 1: Identity ─────────────────────────────────────────────

/**
 * EN: Project identity — defines WHAT this project is and WHERE it runs.
 * ES: Identidad del proyecto — define QUE es este proyecto y DONDE corre.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXProfileIdentity
{
	GENERATED_BODY()

	/** EN: Operational mode of the project / ES: Modo operativo del proyecto */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Identity")
	EPGXProjectMode ProjectMode = EPGXProjectMode::Game;

	/** EN: Target platforms this project supports / ES: Plataformas objetivo que soporta este proyecto */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Identity")
	TArray<EPGXTargetPlatform> ActiveTargets;

	/** EN: Build configuration context / ES: Contexto de configuracion de build */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Identity")
	EPGXBuildContext BuildContext = EPGXBuildContext::Development;

	/** EN: How strictly rules are enforced / ES: Cuan estrictamente se aplican las reglas */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Identity")
	EPGXRestrictionLevel RestrictionLevel = EPGXRestrictionLevel::Relaxed;
};

// ── Structs — Layer 2: Capabilities ─────────────────────────────────────────

/**
 * EN: Profile capabilities — defines WHAT the project is allowed to do.
 *     10 boolean flags organized by domain: Storage, Debug, Runtime.
 * ES: Capacidades del profile — define QUE tiene permitido hacer el proyecto.
 *     10 flags booleanos organizados por dominio: Storage, Debug, Runtime.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXProfileCapabilities
{
	GENERATED_BODY()

	// ── Storage / Almacenamiento ──

	/** EN: Allow INI-based persistence / ES: Permitir persistencia basada en INI */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Capabilities|Storage")
	bool bAllowINIPersistence = true;

	/** EN: Allow save data persistence / ES: Permitir persistencia de save data */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Capabilities|Storage")
	bool bAllowSaveData = true;

	/** EN: Allow writing to external paths (exports, logs) / ES: Permitir escritura a rutas externas (exports, logs) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Capabilities|Storage")
	bool bAllowExternalWrites = true;

	/** EN: Allow data exports (JSON, CSV) / ES: Permitir exportaciones de datos (JSON, CSV) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Capabilities|Storage")
	bool bAllowExports = true;

	// ── Debug / Depuracion ──

	/** EN: Allow console commands / ES: Permitir comandos de consola */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Capabilities|Debug")
	bool bAllowConsoleCommands = true;

	/** EN: Allow dangerous/destructive console commands / ES: Permitir comandos de consola peligrosos/destructivos */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Capabilities|Debug")
	bool bAllowDangerousCommands = true;

	/** EN: Allow profiling and performance instrumentation / ES: Permitir profiling e instrumentacion de rendimiento */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Capabilities|Debug")
	bool bAllowProfiling = true;

	// ── Runtime ──

	/** EN: Allow telemetry data collection / ES: Permitir recoleccion de datos de telemetria */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Capabilities|Runtime")
	bool bAllowTelemetry = true;

	/** EN: Allow persistent log files / ES: Permitir archivos de log persistentes */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Capabilities|Runtime")
	bool bAllowPersistentLogs = true;

	/** EN: Allow hot-reload of assets and code / ES: Permitir hot-reload de assets y codigo */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Capabilities|Runtime")
	bool bAllowHotReload = true;
};

// ── Structs — Layer 3: Policies ─────────────────────────────────────────────

/**
 * EN: Persistence policy — defines HOW each system persists data.
 * ES: Politica de persistencia — define COMO cada sistema persiste datos.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXPersistencePolicy
{
	GENERATED_BODY()

	/** EN: Backend for Config system / ES: Backend para sistema Config */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Policies|Persistence")
	EPGXPersistenceBackend ConfigBackend = EPGXPersistenceBackend::INI;

	/** EN: Backend for Log system / ES: Backend para sistema Log */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Policies|Persistence")
	EPGXPersistenceBackend LogBackend = EPGXPersistenceBackend::INI;

	/** EN: Backend for Save system / ES: Backend para sistema Save */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Policies|Persistence")
	EPGXPersistenceBackend SaveBackend = EPGXPersistenceBackend::SaveData;

	/** EN: Logical base path for persistent data / ES: Ruta base logica para datos persistentes */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Policies|Persistence")
	FString LogicalBasePath = TEXT("{ProjectSavedDir}");

	/** EN: Naming convention for save files / ES: Convencion de nombres para archivos de guardado */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Policies|Persistence")
	FString NamingRule = TEXT("{SystemName}_{SlotIndex}");
};

/**
 * EN: Security policy — defines security requirements for data handling.
 * ES: Politica de seguridad — define requerimientos de seguridad para manejo de datos.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXSecurityPolicy
{
	GENERATED_BODY()

	/** EN: Require encryption for save data / ES: Requerir encriptacion para save data */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Policies|Security")
	bool bRequireEncryption = false;

	/** EN: Require compression for save data / ES: Requerir compresion para save data */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Policies|Security")
	bool bRequireCompression = false;

	/** EN: Enable sensitive data handling / ES: Habilitar manejo de datos sensibles */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Policies|Security")
	bool bSensitiveDataHandling = false;

	/** EN: Strip debug strings in Shipping / ES: Eliminar strings de debug en Shipping */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Policies|Security")
	bool bStripDebugStrings = false;
};

/**
 * EN: Combined policies — persistence + security + restart behavior.
 * ES: Politicas combinadas — persistencia + seguridad + comportamiento de reinicio.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXProfilePolicies
{
	GENERATED_BODY()

	/** EN: Persistence routing policy / ES: Politica de enrutamiento de persistencia */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Policies")
	FPGXPersistencePolicy Persistence;

	/** EN: Security enforcement policy / ES: Politica de enforcement de seguridad */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Policies")
	FPGXSecurityPolicy Security;

	/** EN: Whether policy changes require application restart / ES: Si cambios de politica requieren reinicio de aplicacion */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Policies")
	bool bRequiresRestart = false;
};

// ── Structs — Layer 4: Budgets ──────────────────────────────────────────────

/**
 * EN: Resource budgets — defines hard limits for system resources.
 *     Value of 0 means unlimited (no budget enforcement).
 * ES: Presupuestos de recursos — define limites duros para recursos del sistema.
 *     Valor 0 significa ilimitado (sin enforcement de presupuesto).
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXProfileBudgets
{
	GENERATED_BODY()

	// ── Memory / Memoria ──

	/** EN: RAM budget in MB (0 = unlimited) / ES: Presupuesto de RAM en MB (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Budgets|Memory", meta = (ClampMin = "0"))
	int64 RAM_MB = 0;

	/** EN: VRAM budget in MB (0 = unlimited) / ES: Presupuesto de VRAM en MB (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Budgets|Memory", meta = (ClampMin = "0"))
	int64 VRAM_MB = 0;

	// ── Disk / Disco ──

	/** EN: Disk budget in MB (0 = unlimited) / ES: Presupuesto de disco en MB (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Budgets|Disk", meta = (ClampMin = "0"))
	int64 Disk_MB = 0;

	// ── Streaming / Streaming ──

	/** EN: Streaming pool budget in MB (0 = unlimited) / ES: Presupuesto de streaming pool en MB (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Budgets|Streaming", meta = (ClampMin = "0"))
	int64 StreamingPool_MB = 0;

	// ── Assets ──

	/** EN: Maximum texture dimension (0 = unlimited) / ES: Dimension maxima de textura (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Budgets|Assets", meta = (ClampMin = "0"))
	int32 TextureMaxDim = 0;

	/** EN: Maximum triangle count per mesh (0 = unlimited) / ES: Conteo maximo de triangulos por mesh (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Budgets|Assets", meta = (ClampMin = "0"))
	int32 MeshMaxTris = 0;

	// ── Pipeline ──

	/** EN: Shader variant budget (0 = unlimited) / ES: Presupuesto de variantes de shader (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Budgets|Pipeline", meta = (ClampMin = "0"))
	int32 ShaderBudget = 0;

	/** EN: PSO warm-up entry budget (0 = unlimited) / ES: Presupuesto de entradas de warm-up PSO (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Budgets|Pipeline", meta = (ClampMin = "0"))
	int32 PSOBudget = 0;

	// ── Audio ──

	/** EN: Audio memory budget in MB (0 = unlimited) / ES: Presupuesto de memoria de audio en MB (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Budgets|Audio", meta = (ClampMin = "0"))
	int32 AudioMemory_MB = 0;
};

// ── Structs — Layer 5: Feature Matrix ───────────────────────────────────────

/**
 * EN: Feature entry — single rendering/engine feature with its policy.
 * ES: Entrada de feature — feature individual de rendering/engine con su politica.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXFeatureEntry
{
	GENERATED_BODY()

	/** EN: Feature availability policy / ES: Politica de disponibilidad del feature */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Features")
	EPGXFeaturePolicy Policy = EPGXFeaturePolicy::Allowed;

	/** EN: Whether changing this feature requires restart / ES: Si cambiar este feature requiere reinicio */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Features")
	bool bRequiresRestart = false;
};

/**
 * EN: Feature matrix — rendering and engine feature availability.
 *     Each entry defines a policy (Allowed/Disallowed/FallbackRequired).
 * ES: Matriz de features — disponibilidad de features de rendering y engine.
 *     Cada entrada define una politica (Allowed/Disallowed/FallbackRequired).
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXProfileFeatureMatrix
{
	GENERATED_BODY()

	/** EN: Nanite virtualized geometry / ES: Geometria virtualizada Nanite */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Features")
	FPGXFeatureEntry Nanite;

	/** EN: Lumen global illumination / ES: Iluminacion global Lumen */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Features")
	FPGXFeatureEntry Lumen;

	/** EN: Hardware ray tracing / ES: Ray tracing por hardware */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Features")
	FPGXFeatureEntry RayTracing;

	/** EN: Virtual Shadow Maps / ES: Virtual Shadow Maps */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Features")
	FPGXFeatureEntry VSM;

	/** EN: Forward shading pipeline / ES: Pipeline de forward shading */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Features")
	FPGXFeatureEntry ForwardShading;

	/** EN: Mobile renderer / ES: Renderer movil */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Features")
	FPGXFeatureEntry MobileRenderer;

	/** EN: Virtual textures / ES: Texturas virtuales */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Features")
	FPGXFeatureEntry VirtualTextures;
};

// ── Resolved Profile ────────────────────────────────────────────────────────

/**
 * EN: The fully resolved profile — the single source of truth after resolution.
 *     Contains all 5 layers + state + timestamp + helper query methods.
 *     This is what subsystems receive and query against.
 *
 * ES: El profile completamente resuelto — la fuente unica de verdad tras la resolucion.
 *     Contiene las 5 capas + estado + timestamp + metodos de consulta auxiliares.
 *     Esto es lo que los subsistemas reciben y consultan.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXResolvedProfile
{
	GENERATED_BODY()

	// ── 5 Layers / 5 Capas ──

	/** EN: Layer 1 — Project identity / ES: Capa 1 — Identidad del proyecto */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Profile")
	FPGXProfileIdentity Identity;

	/** EN: Layer 2 — What is allowed / ES: Capa 2 — Que esta permitido */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Profile")
	FPGXProfileCapabilities Capabilities;

	/** EN: Layer 3 — How things should be done / ES: Capa 3 — Como deben hacerse las cosas */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Profile")
	FPGXProfilePolicies Policies;

	/** EN: Layer 4 — Resource limits / ES: Capa 4 — Limites de recursos */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Profile")
	FPGXProfileBudgets Budgets;

	/** EN: Layer 5 — Feature availability / ES: Capa 5 — Disponibilidad de features */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Profile")
	FPGXProfileFeatureMatrix Features;

	// ── State / Estado ──

	/** EN: Current resolution state / ES: Estado de resolucion actual */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Profile")
	EPGXProfileState State = EPGXProfileState::Unresolved;

	/** EN: Timestamp when profile was resolved (FDateTime::UtcNow) / ES: Timestamp de cuando se resolvio el profile */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Profile")
	FDateTime ResolvedTimestamp;

	// ── Query Helpers / Ayudantes de Consulta ──

	/**
	 * EN: Check if a named capability is allowed. Supports:
	 *     INIPersistence, SaveData, ExternalWrites, Exports,
	 *     ConsoleCommands, DangerousCommands, Profiling,
	 *     Telemetry, PersistentLogs, HotReload
	 * ES: Verificar si una capacidad nombrada esta permitida.
	 */
	bool IsCapabilityAllowed(FName CapabilityName) const;

	/**
	 * EN: Check if a named feature is allowed. Supports:
	 *     Nanite, Lumen, RayTracing, VSM, ForwardShading, MobileRenderer, VirtualTextures
	 * ES: Verificar si un feature nombrado esta permitido.
	 */
	bool IsFeatureAllowed(FName FeatureName) const;

	/**
	 * EN: Get a named budget value. Returns 0 if not found (unlimited).
	 *     Supports: RAM_MB, VRAM_MB, Disk_MB, StreamingPool_MB,
	 *     TextureMaxDim, MeshMaxTris, ShaderBudget, PSOBudget, AudioMemory_MB
	 * ES: Obtener un valor de presupuesto nombrado. Retorna 0 si no se encuentra (ilimitado).
	 */
	int64 GetBudget(FName BudgetName) const;

	/** EN: Get the feature policy for a named feature / ES: Obtener la politica de un feature nombrado */
	EPGXFeaturePolicy GetFeaturePolicy(FName FeatureName) const;

	/** EN: Get persistence backend for a named system / ES: Obtener backend de persistencia para un sistema nombrado */
	EPGXPersistenceBackend GetPersistenceBackend(FName SystemName) const;

	/** EN: Create a default permissive profile (all allowed, no budgets) / ES: Crear un profile permisivo por defecto */
	static FPGXResolvedProfile MakeDefaultPermissive();
};

// ── Dynamic Delegates (Blueprint-assignable) ────────────────────────────────

/** EN: Fired when the profile is first resolved / ES: Se dispara cuando el profile se resuelve por primera vez */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPGXProfileResolved, const FPGXResolvedProfile&, ResolvedProfile);

/** EN: Fired when the profile changes (old → new) / ES: Se dispara cuando el profile cambia (viejo → nuevo) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPGXProfileChanged, const FPGXResolvedProfile&, OldProfile, const FPGXResolvedProfile&, NewProfile);
