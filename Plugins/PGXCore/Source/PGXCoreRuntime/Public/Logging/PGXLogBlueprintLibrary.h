// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Logging/PGXLogTypes.h"
#include "GameplayTagContainer.h"
#include "PGXLogBlueprintLibrary.generated.h"

// ═══════════════════════════════════════════════════════════════
// EN: Blueprint Function Library for writing PGX structured logs.
//     Provides convenience nodes for all verbosity levels,
//     plus full-featured versions with typed params and GameplayTags.
//     All logs go to both UE Output Log AND the PGX ring buffer.
//
// ES: Libreria de Funciones Blueprint para escribir logs PGX estructurados.
//     Provee nodos de conveniencia para todos los niveles de verbosity,
//     mas versiones completas con params tipados y GameplayTags.
//     Todos los logs van tanto al Output Log de UE COMO al ring buffer PGX.
// ═══════════════════════════════════════════════════════════════

UCLASS()
class PGXCORERUNTIME_API UPGXLogBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ═══════════════════════════════════════
	// Generic / Generico
	// ═══════════════════════════════════════

	/**
	 * EN: Log a message with the specified verbosity level.
	 *     Goes to both UE Output Log (LogPGX) and PGX ring buffer.
	 * ES: Registrar un mensaje con el nivel de verbosity especificado.
	 *     Va tanto al Output Log de UE (LogPGX) como al ring buffer PGX.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Log", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "Category"))
	static void PGXLog(const UObject* WorldContextObject, EPGXLogVerbosity Verbosity, const FString& Message, FName Category = "LogPGX");

	// ═══════════════════════════════════════
	// Convenience per-verbosity / Conveniencia por verbosity
	// ═══════════════════════════════════════

	/** EN: Log an Info message / ES: Registrar un mensaje Info */
	UFUNCTION(BlueprintCallable, Category = "PGX|Log", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "Category"))
	static void PGXLogInfo(const UObject* WorldContextObject, const FString& Message, FName Category = "LogPGX");

	/** EN: Log a Warning message / ES: Registrar un mensaje Warning */
	UFUNCTION(BlueprintCallable, Category = "PGX|Log", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "Category"))
	static void PGXLogWarning(const UObject* WorldContextObject, const FString& Message, FName Category = "LogPGX");

	/** EN: Log an Error message / ES: Registrar un mensaje Error */
	UFUNCTION(BlueprintCallable, Category = "PGX|Log", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "Category"))
	static void PGXLogError(const UObject* WorldContextObject, const FString& Message, FName Category = "LogPGX");

	/** EN: Log a Debug message (stripped in Shipping) / ES: Registrar un mensaje Debug (eliminado en Shipping) */
	UFUNCTION(BlueprintCallable, Category = "PGX|Log", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "Category"))
	static void PGXLogDebug(const UObject* WorldContextObject, const FString& Message, FName Category = "LogPGX");

	/** EN: Log a Verbose message (stripped in Shipping) / ES: Registrar un mensaje Verbose (eliminado en Shipping) */
	UFUNCTION(BlueprintCallable, Category = "PGX|Log", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "Category"))
	static void PGXLogVerbose(const UObject* WorldContextObject, const FString& Message, FName Category = "LogPGX");

	// ═══════════════════════════════════════
	// With Parameters / Con Parametros
	// ═══════════════════════════════════════

	/**
	 * EN: Log a message with key-value parameters. Params appear in the Log Viewer
	 *     expanded panel and in JSON exports.
	 * ES: Registrar un mensaje con parametros clave-valor. Los params aparecen en
	 *     el panel expandido del Log Viewer y en exportaciones JSON.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Log", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "Category"))
	static void PGXLogWithParams(const UObject* WorldContextObject, EPGXLogVerbosity Verbosity, const FString& Message, const TMap<FString, FString>& Params, FName Category = "LogPGX");

	// ═══════════════════════════════════════
	// Full (Params + Tags) / Completo (Params + Tags)
	// ═══════════════════════════════════════

	/**
	 * EN: Full-featured log with parameters AND GameplayTags.
	 *     Custom tags are appended to auto-generated verbosity/context tags.
	 *     Use this for logs that need tag-based filtering in the Log Viewer.
	 * ES: Log completo con parametros Y GameplayTags.
	 *     Tags custom se agregan a los tags auto-generados de verbosity/contexto.
	 *     Usar para logs que necesiten filtrado por tags en el Log Viewer.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Log", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "Category"))
	static void PGXLogFull(const UObject* WorldContextObject, EPGXLogVerbosity Verbosity, const FString& Message, const TMap<FString, FString>& Params, FGameplayTagContainer Tags, FName Category = "LogPGX");

	// ═══════════════════════════════════════
	// Domain-Aware (v3.0) / Con Dominio (v3.0)
	// ═══════════════════════════════════════

	/**
	 * EN: Log a message with a specific domain tag. Used for domain-specific rendering in the Log Viewer.
	 * ES: Registrar un mensaje con un tag de dominio especifico. Usado para rendering de dominio en el Log Viewer.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Log|Domain", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "Category"))
	static void PGXLogDomain(const UObject* WorldContextObject, EPGXLogVerbosity Verbosity, const FString& Message, FGameplayTag DomainTag, FName Category = "LogPGX");

	/**
	 * EN: Log with domain tag AND key-value parameters.
	 * ES: Log con tag de dominio Y parametros clave-valor.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Log|Domain", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "Category"))
	static void PGXLogDomainWithParams(const UObject* WorldContextObject, EPGXLogVerbosity Verbosity, const FString& Message, FGameplayTag DomainTag, const TMap<FString, FString>& Params, FName Category = "LogPGX");

	/**
	 * EN: Get all registered domain tags.
	 * ES: Obtener todos los tags de dominio registrados.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Log|Domain", meta = (WorldContext = "WorldContextObject"))
	static TArray<FGameplayTag> GetRegisteredDomains(const UObject* WorldContextObject);

	// ═══════════════════════════════════════
	// Query / Consulta
	// ═══════════════════════════════════════

	/** EN: Get all entries from the current session / ES: Obtener todas las entradas de la sesion actual */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Log|Query", meta = (WorldContext = "WorldContextObject"))
	static TArray<FPGXLogEntry> GetCurrentSessionEntries(const UObject* WorldContextObject);

	/** EN: Get current session metadata / ES: Obtener metadatos de la sesion actual */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Log|Query", meta = (WorldContext = "WorldContextObject"))
	static FPGXLogSession GetCurrentSession(const UObject* WorldContextObject);

	/** EN: Get total entry count in ring buffer / ES: Obtener conteo total en el ring buffer */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Log|Query", meta = (WorldContext = "WorldContextObject"))
	static int32 GetEntryCount(const UObject* WorldContextObject);

private:
	/**
	 * EN: Internal: Submit an entry to the PGX Log Subsystem + UE_LOG bridge.
	 * ES: Interno: Enviar una entrada al PGX Log Subsystem + puente UE_LOG.
	 */
	static void SubmitEntry(const UObject* WorldContextObject, EPGXLogVerbosity Verbosity, const FString& Message, FName Category, const TMap<FString, FString>* Params = nullptr, const FGameplayTagContainer* CustomTags = nullptr, const FGameplayTag* InDomainTag = nullptr);
};
