// Copyright PGX Framework. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PGXAbilityTestUtility.generated.h"

class UPGXAbilitySubsystem;

/**
 * EN: Test utility for the PGX Ability baseline. Canonical PGX contract — same shape as
 *     `UPGXAITestUtility`:
 *
 *         static bool TestX(WCO, TArray<FString>& OutIssues, ...)
 *
 *     Returns true on full pass; populates OutIssues with [PASS]/[FAIL] lines.
 *
 * ES: Utilidad de pruebas para el baseline PGX Ability. Mismo contrato canonico que
 *     `UPGXAITestUtility`: bool + OutIssues con lineas [PASS]/[FAIL].
 */
UCLASS()
class PGXABILITYRUNTIME_API UPGXAbilityTestUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** EN: Subsystem accessible from world; initial registry empty. / ES: Subsistema accesible; registro inicial vacio. */
	UFUNCTION(BlueprintCallable, Category = "PGX|Ability|Test", meta = (WorldContext = "WorldContextObject"))
	static bool SubsystemInitializeTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: Component BeginPlay resolves/creates ASC, registers with subsystem; EndPlay unregisters. / ES: BeginPlay resuelve ASC y se registra; EndPlay desregistra. */
	UFUNCTION(BlueprintCallable, Category = "PGX|Ability|Test", meta = (WorldContext = "WorldContextObject"))
	static bool ComponentLifecycleTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: GetAbilityFacade/GetAttributeFacade/GetEffectFacade return valid, stable instances. / ES: Las 3 facades resuelven instancias validas y estables. */
	UFUNCTION(BlueprintCallable, Category = "PGX|Ability|Test", meta = (WorldContext = "WorldContextObject"))
	static bool FacadeResolutionTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: UPGXAbilitySettings reachable via GetDefault; ActiveConfig accessor reachable. / ES: Settings alcanzable; accessor ActiveConfig alcanzable. */
	UFUNCTION(BlueprintCallable, Category = "PGX|Ability|Test", meta = (WorldContext = "WorldContextObject"))
	static bool ConfigResolutionTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: GrantAbility/RevokeAbility contract — typed result, idempotency, NotFound on miss. / ES: Contrato Grant/Revoke — resultado tipado, idempotencia, NotFound en miss. */
	UFUNCTION(BlueprintCallable, Category = "PGX|Ability|Test", meta = (WorldContext = "WorldContextObject"))
	static bool AbilityGrantRevokeTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

private:
	static UPGXAbilitySubsystem* GetSubsystem(const UObject* WorldContextObject);
	static void RecordResult(TArray<FString>& OutIssues, const FString& TestName, bool bPassed, const FString& Details = TEXT(""));
};
