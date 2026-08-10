// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Testing/PGXTestResultCollector.h"
#include "Utils/FPGXSubsystemResolver.h"

class FPGXTestSubsystemFixture;
class UWorld;

/**
 * EN: Stable lifecycle state for a PGX test-runner instance.
 * ES: Estado de ciclo de vida estable para una instancia de test-runner PGX.
 */
enum class EPGXTestRunnerState : uint8
{
	NotStarted,
	Running,
	Passed,
	Failed,
	Cancelled
};

/**
 * EN: Options for a single FPGXTestRunner execution.
 *     The runner is instance-local on purpose: no static maps, no cross-PIE
 *     leakage, and deterministic cleanup via EndPlay().
 *
 * ES: Opciones para una ejecucion de FPGXTestRunner.
 *     El runner es local a la instancia a proposito: sin mapas static, sin
 *     fugas cross-PIE, y cleanup determinista via EndPlay().
 */
struct FPGXTestRunnerOptions
{
	FString TestName = TEXT("PGXTestRunner");
	bool bUseSubsystemFixture = false;
	bool bRecordResult = true;
};

/**
 * EN: Summary of a single runner execution, compatible with
 *     FPGXTestResultCollector.
 * ES: Resumen de una ejecucion del runner, compatible con
 *     FPGXTestResultCollector.
 */
struct FPGXTestRunnerResult
{
	FString SystemName;
	FString SystemVersion;
	FString TestName;
	bool bPassed = false;
	TArray<FString> Issues;
	FDateTime Timestamp;
	double DurationMs = 0.0;
	EPGXTestRunnerState State = EPGXTestRunnerState::NotStarted;

	FPGXTestResult ToCollectorResult() const;
};

/**
 * EN: Base runner that unifies the existing PGX test utilities instead of
 *     replacing them. It wraps:
 *       - FPGXTestSubsystemFixture for optional lifecycle fixture ownership.
 *       - FPGXTestResultCollector for centralized PASS/FAIL recording.
 *       - FPGXSubsystemResolver for null-safe subsystem access.
 *
 *     Subclasses override RunInternal() or callers use RunLambda() for small
 *     adoption shims around existing U*TestUtility::RunAllTests functions.
 *     BeginPlay()/EndPlay() are deterministic and instance-local, so a runner
 *     cannot leak static state between PIE sessions.
 *
 * ES: Runner base que unifica las utilidades de test PGX existentes en vez de
 *     reemplazarlas. Envuelve FPGXTestSubsystemFixture, FPGXTestResultCollector
 *     y FPGXSubsystemResolver. BeginPlay()/EndPlay() son deterministas y
 *     locales a la instancia para evitar fugas entre sesiones PIE.
 */
class PGXCOREDEVELOPER_API FPGXTestRunner
{
public:
	explicit FPGXTestRunner(FString InSystemName, FString InSystemVersion = TEXT(""));
	virtual ~FPGXTestRunner();

	FPGXTestRunner(const FPGXTestRunner&) = delete;
	FPGXTestRunner& operator=(const FPGXTestRunner&) = delete;

	FPGXTestRunner(FPGXTestRunner&&) = delete;
	FPGXTestRunner& operator=(FPGXTestRunner&&) = delete;

	bool BeginPlay(const UObject* WorldContextObject, const FPGXTestRunnerOptions& Options = FPGXTestRunnerOptions());
	void EndPlay();

	bool Run(const UObject* WorldContextObject, const FPGXTestRunnerOptions& Options = FPGXTestRunnerOptions());
	bool RunLambda(const UObject* WorldContextObject, TFunctionRef<bool(const UObject*, TArray<FString>&)> TestBody, const FPGXTestRunnerOptions& Options = FPGXTestRunnerOptions());

	void Pass(const FString& TestName, const FString& Details = TEXT(""));
	void Fail(const FString& TestName, const FString& Issue);
	bool ExpectTrue(bool bCondition, const FString& TestName, const FString& FailureIssue);
	bool ExpectNotNull(const void* Pointer, const FString& TestName, const FString& FailureIssue);

	template<typename TSubsystem>
	TSubsystem* GetGISubsystem() const
	{
		static_assert(TIsDerivedFrom<TSubsystem, UGameInstanceSubsystem>::Value, "TSubsystem must derive from UGameInstanceSubsystem");
		return GetGISubsystem<TSubsystem>(GetActiveWorld());
	}

	template<typename TSubsystem>
	TSubsystem* GetGISubsystem(const UWorld* World) const
	{
		static_assert(TIsDerivedFrom<TSubsystem, UGameInstanceSubsystem>::Value, "TSubsystem must derive from UGameInstanceSubsystem");
		return FPGXTestRunner::ResolveGISubsystem<TSubsystem>(World);
	}

	template<typename TSubsystem>
	static TSubsystem* ResolveGISubsystem(const UWorld* World)
	{
		static_assert(TIsDerivedFrom<TSubsystem, UGameInstanceSubsystem>::Value, "TSubsystem must derive from UGameInstanceSubsystem");
		return FPGXSubsystemResolver::GetGISubsystem<TSubsystem>(World);
	}

	const FPGXTestRunnerResult& GetLastResult() const { return LastResult; }
	const TArray<FPGXTestRunnerResult>& GetHistory() const { return History; }
	EPGXTestRunnerState GetState() const { return State; }
	int32 GetPassedCount() const;
	int32 GetFailedCount() const;
	void Reset();

protected:
	virtual bool RunInternal(TArray<FString>& OutIssues);

	UWorld* GetActiveWorld() const;
	const UObject* GetWorldContextObject() const;
	void AddIssue(const FString& Issue);

private:
	void CompleteRun(bool bPassed, const TArray<FString>& Issues, double StartSeconds);

	FString SystemName;
	FString SystemVersion;
	FPGXTestRunnerOptions ActiveOptions;
	EPGXTestRunnerState State = EPGXTestRunnerState::NotStarted;
	TWeakObjectPtr<UObject> WorldContextWeak;
	TWeakObjectPtr<UWorld> ActiveWorldWeak;
	TUniquePtr<FPGXTestSubsystemFixture> Fixture;
	FPGXTestRunnerResult LastResult;
	TArray<FPGXTestRunnerResult> History;
};
