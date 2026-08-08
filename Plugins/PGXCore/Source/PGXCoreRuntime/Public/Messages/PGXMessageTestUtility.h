// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "Messages/PGXMessage.h"
#include "PGXMessageTestUtility.generated.h"

/**
 * EN: Test utility for the PGX Message System.
 *     Provides Blueprint-callable test/debug helpers for validating
 *     message broadcasting, listener management, and system health.
 *     Designed for PIE testing and automation.
 * ES: Utilidad de test para el Sistema de Mensajes PGX.
 *     Provee helpers de test/debug llamables desde Blueprint para validar
 *     broadcasting de mensajes, gestion de listeners y salud del sistema.
 *     Disenado para testing PIE y automatizacion.
 */
UCLASS()
class PGXCORERUNTIME_API UPGXMessageTestUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** EN: Quick system health check / ES: Verificacion rapida de salud del sistema */
	UFUNCTION(BlueprintCallable, Category = "PGX|Messages|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool QuickTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: Test broadcast and receive pipeline / ES: Probar pipeline de broadcast y recepcion */
	UFUNCTION(BlueprintCallable, Category = "PGX|Messages|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool BroadcastReceiveTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: Test message filtering / ES: Probar filtrado de mensajes */
	UFUNCTION(BlueprintCallable, Category = "PGX|Messages|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool FilterTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: Test match types (exact vs partial) / ES: Probar tipos de coincidencia (exacta vs parcial) */
	UFUNCTION(BlueprintCallable, Category = "PGX|Messages|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool MatchTypeTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: Test message history recording / ES: Probar grabacion de historial de mensajes */
	UFUNCTION(BlueprintCallable, Category = "PGX|Messages|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool HistoryTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: Test listener unregistration and cleanup / ES: Probar desregistro y limpieza de listeners */
	UFUNCTION(BlueprintCallable, Category = "PGX|Messages|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool UnregisterTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: Run all message system tests / ES: Ejecutar todas las pruebas del sistema de mensajes */
	UFUNCTION(BlueprintCallable, Category = "PGX|Messages|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool RunAllTests(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	// ============================================================
	// Additional message-system compatibility tests
	// ============================================================

	/** EN: Type mismatch broadcast does not fire the listener. */
	UFUNCTION(BlueprintCallable, Category = "PGX|Messages|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool TypeMismatchRejectedTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: Repeated unregister of the same handle is safe. */
	UFUNCTION(BlueprintCallable, Category = "PGX|Messages|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool DoubleUnregisterSafeTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: Listener self-unregistering inside a callback does not break fan-out iteration. */
	UFUNCTION(BlueprintCallable, Category = "PGX|Messages|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool CallbackRemovalSafeTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	// Partial matching
	/** EN: Exact-match listener on a parent tag does NOT receive a child-tag broadcast. */
	UFUNCTION(BlueprintCallable, Category = "PGX|Messages|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool ExactParentNoChildTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: With Config.bEnablePartialMatching=false, a partial parent listener does NOT receive a child broadcast. WITH_EDITOR. */
	UFUNCTION(BlueprintCallable, Category = "PGX|Messages|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool PartialMatchGlobalDisabledTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: Listeners on disjoint channels do not cross-fire. */
	UFUNCTION(BlueprintCallable, Category = "PGX|Messages|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool ChannelIsolationTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	// Payload backward compatibility
	/** EN: Listener with a parent-struct filter receives a child-struct broadcast. */
	UFUNCTION(BlueprintCallable, Category = "PGX|Messages|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool PayloadBackwardCompatTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	// Configuration and history — WITH_EDITOR (uses InjectTestConfig)
	/** EN: History is trimmed at the MaxMessageHistory bound. WITH_EDITOR. */
	UFUNCTION(BlueprintCallable, Category = "PGX|Messages|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool HistoryBoundedByPolicyTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: ClearHistory empties records but leaves listeners untouched. WITH_EDITOR. */
	UFUNCTION(BlueprintCallable, Category = "PGX|Messages|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool ClearHistoryAdditionalTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: Settings.EmergencyHistoryFallback is used when CachedConfig is invalid. WITH_EDITOR. */
	UFUNCTION(BlueprintCallable, Category = "PGX|Messages|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool EmergencyHistoryFallbackTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	// Telemetry
	/** EN: Stats.MaxFanOutOnSingleBroadcast tracks the listener-count high-water mark. */
	UFUNCTION(BlueprintCallable, Category = "PGX|Messages|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool FanOutTelemetryTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: Parent fan-out completes before a nested broadcast begins. */
	UFUNCTION(BlueprintCallable, Category = "PGX|Messages|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool NestedBroadcastOrderingTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: Record.Timestamp uses FPlatformTime::Seconds (wall time, never simulation time per the wall-clock timestamp policy). */
	UFUNCTION(BlueprintCallable, Category = "PGX|Messages|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool TimeAccelBoundaryReadOnlyTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	// ============================================================
	// FPGXBridgeGameFlowChanged appended-field compatibility
	// ============================================================

	/** EN: New fields exist on FPGXBridgeGameFlowChanged with default-construct values. */
	UFUNCTION(BlueprintCallable, Category = "PGX|Messages|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool BridgePayloadExtensionPresenceTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: Populated payload roundtrips through Message bus; consumer reads new fields intact. */
	UFUNCTION(BlueprintCallable, Category = "PGX|Messages|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool BridgePayloadExtensionRoundtripTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: Default-only payload (legacy publisher style) broadcasts; consumer reads default-empty new fields without crash. */
	UFUNCTION(BlueprintCallable, Category = "PGX|Messages|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool BridgePayloadExtensionBackwardCompatTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: Caller-supplied RequestId preserved through broadcast; default-construct yields invalid FGuid. */
	UFUNCTION(BlueprintCallable, Category = "PGX|Messages|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool BridgePayloadExtensionRequestIdTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);
};

/**
 * EN: Test-only USTRUCT child of FPGXMessage. Used by PayloadBackwardCompatTest to verify
 *     listener-filter compatibility: a parent type accepts a child broadcast.
 *     Adds a single field over base; UStruct inheritance prefix invariant guarantees safe
 *     parent-layout copy from child bytes.
 * ES: USTRUCT solo-test hijo de FPGXMessage. Usado por PayloadBackwardCompatTest para verificar
 *     compatibilidad del filtro: un parent type acepta un broadcast hijo.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXTestChildMessage : public FPGXMessage
{
	GENERATED_BODY()

	/** EN: Test-only field that exists in child but not parent. */
	UPROPERTY(BlueprintReadWrite)
	int32 ChildOnlyValue = 0;
};
