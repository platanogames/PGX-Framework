// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "PGXUITestUtility.generated.h"

class UPGXScreenDefinition;
class UPGXNotificationProfile;
class UPGXWidgetPoolProfile;

/**
 * EN: Test utility for PGXUI surfaces. Five BPL validators use the bool+OutIssues
 *     shared result contract used by PGXBridge,
 *     PGXMessage, PGXInspector, PGXPSO, PGXAI, PGXColony, PGXUI):
 *
 *         static bool ValidateX(..., TArray<FString>& OutIssues)
 *
 *     Returns true on full pass; populates OutIssues with [PASS]/[FAIL] lines so Automation wrappers
 *     surface every issue via FAutomationTestBase::AddInfo.
 *
 *     Scope (settings, tags, and DataAsset surfaces):
 *     - ValidateScreenDefinition (UPGXScreenDefinition validation rules)
 *     - ValidateNotificationProfile (UPGXNotificationProfile validation rules)
 *     - ValidateWidgetPoolProfile (UPGXWidgetPoolProfile validation rules)
 *     - ValidateTagInNamespace (tag generic helper — check FGameplayTag is descendant of namespace root)
 *     - ValidateSettingsAccessor (settings UPGXUISettings GetDefault + canonical surface accessibility)
 *
 *     Outside the current test utility:
 *     - Reset / GetSnapshot / InjectFakeConfig / BroadcastTest / ListenAndCapture / RunIntegration
 *       / DumpDiagnostic are not included because they require runtime subsystem state
 *       and cross-plugin Message channel wiring.
 *
 * ES: Utilidad de pruebas para superficies PGXUI. Cinco validadores BPL usan el contrato
 *     bool+OutIssues compartido. Reset, GetSnapshot, InjectFakeConfig, BroadcastTest,
 *     ListenAndCapture, RunIntegration y DumpDiagnostic no forman parte de esta utilidad.
 */
UCLASS()
class PGXUIRUNTIME_API UPGXUITestUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ========================================================================
	// EN: Five validators using the bool+OutIssues contract.
	// ES: Cinco validadores con el contrato bool+OutIssues.
	// ========================================================================

	/**
	 * EN: Validate UPGXScreenDefinition Object DA by the documented validation rules: ScreenTag resolves under
	 *     `PGX.UI.Screen.Type.*`, LayerTag resolves under `PGX.UI.Screen.Layer.*`, WidgetClass
	 *     soft reference valid (asset path non-null).
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|UI|Test")
	static bool ValidateScreenDefinition(const UPGXScreenDefinition* Definition, TArray<FString>& OutIssues);

	/**
	 * EN: Validate UPGXNotificationProfile Object DA by the documented validation rules: CategoryTag resolves under
	 *     `PGX.UI.Notification.Category.*`, PriorityTag resolves under `PGX.UI.Notification.Priority.*`,
	 *     coalescing policy explicit, dismissal policy explicit.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|UI|Test")
	static bool ValidateNotificationProfile(const UPGXNotificationProfile* Profile, TArray<FString>& OutIssues);

	/**
	 * EN: Validate UPGXWidgetPoolProfile Object DA by the documented validation rules: PoolTypeTag resolves under
	 *     `PGX.UI.WidgetPool.Type.*`, widget class policy present (WidgetClass valid OR
	 *     bIsAbstractPool), reset validator policy explicit, capacity ordering coherent
	 *     (InitialCapacity <= MaxCapacity).
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|UI|Test")
	static bool ValidateWidgetPoolProfile(const UPGXWidgetPoolProfile* Profile, TArray<FString>& OutIssues);

	/**
	 * EN: Validate that `Tag` is a valid descendant (or equal) of `NamespaceRoot`. Useful generic
	 *     guard for cross-plugin tag boundary enforcement (e.g., assert a tag belongs to the
	 *     PGX.UI.Screen.Type.* branch before forwarding to UPGXScreenManager).
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|UI|Test")
	static bool ValidateTagInNamespace(FGameplayTag Tag, FGameplayTag NamespaceRoot, TArray<FString>& OutIssues);

	/**
	 * EN: Validate UPGXUISettings GetDefault accessor surface (settings): Settings non-null, GetCategoryName
	 *     returns "PGX", DiscoveryMode default is AssetRegistryScan, ActiveConfig accessor reachable
	 *     (TSoftObjectPtr soft-null OK at-baseline since runtime consumption NOT CONSUMED AT RUNTIME).
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|UI|Test")
	static bool ValidateSettingsAccessor(TArray<FString>& OutIssues);

private:
	/** EN: Append [PASS]/[FAIL] line to OutIssues + UE_LOG mirror for live debug visibility. */
	static void RecordResult(TArray<FString>& OutIssues, const FString& TestName, bool bPassed, const FString& Details = TEXT(""));
};
