// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Testing/FPGXTestRunner.h"

#include "Engine/World.h"
#include "Testing/PGXTestSubsystemFixture.h"
#include "UObject/Object.h"

FPGXTestResult FPGXTestRunnerResult::ToCollectorResult() const
{
	FPGXTestResult Result;
	Result.SystemName = SystemName;
	Result.SystemVersion = SystemVersion;
	Result.TestName = TestName;
	Result.bPassed = bPassed;
	Result.Issues = Issues;
	Result.Timestamp = Timestamp;
	Result.DurationMs = DurationMs;
	return Result;
}

FPGXTestRunner::FPGXTestRunner(FString InSystemName, FString InSystemVersion)
	: SystemName(MoveTemp(InSystemName))
	, SystemVersion(MoveTemp(InSystemVersion))
{
}

FPGXTestRunner::~FPGXTestRunner()
{
	EndPlay();
}

bool FPGXTestRunner::BeginPlay(const UObject* WorldContextObject, const FPGXTestRunnerOptions& Options)
{
	EndPlay();
	ActiveOptions = Options;
	LastResult = FPGXTestRunnerResult();
	State = EPGXTestRunnerState::Running;

	WorldContextWeak = const_cast<UObject*>(WorldContextObject);
	UWorld* ResolvedWorld = FPGXSubsystemResolver::GetWorldFromContext(WorldContextObject);

	if (!ResolvedWorld && Options.bUseSubsystemFixture)
	{
		Fixture = MakeUnique<FPGXTestSubsystemFixture>();
		Fixture->Setup();
		ResolvedWorld = Fixture->GetWorld();
	}

	ActiveWorldWeak = ResolvedWorld;

	// EN: A null world is accepted so pure/static TestUtilities can still run;
	//     subsystem access helpers simply return nullptr.
	// ES: World null es aceptado para TestUtilities puros/static; los helpers de
	//     subsistema retornan nullptr de forma segura.
	return true;
}

void FPGXTestRunner::EndPlay()
{
	Fixture.Reset();
	ActiveWorldWeak.Reset();
	WorldContextWeak.Reset();

	if (State == EPGXTestRunnerState::Running)
	{
		State = EPGXTestRunnerState::Cancelled;
	}
}

bool FPGXTestRunner::Run(const UObject* WorldContextObject, const FPGXTestRunnerOptions& Options)
{
	const double StartSeconds = FPlatformTime::Seconds();
	BeginPlay(WorldContextObject, Options);

	TArray<FString> Issues;
	const bool bPassed = RunInternal(Issues);
	CompleteRun(bPassed, Issues, StartSeconds);
	EndPlay();
	return bPassed;
}

bool FPGXTestRunner::RunLambda(const UObject* WorldContextObject, TFunctionRef<bool(const UObject*, TArray<FString>&)> TestBody, const FPGXTestRunnerOptions& Options)
{
	const double StartSeconds = FPlatformTime::Seconds();
	BeginPlay(WorldContextObject, Options);

	TArray<FString> Issues;
	const bool bPassed = TestBody(WorldContextObject, Issues);
	CompleteRun(bPassed, Issues, StartSeconds);
	EndPlay();
	return bPassed;
}

void FPGXTestRunner::Pass(const FString& TestName, const FString& Details)
{
	LastResult.Issues.Add(Details.IsEmpty()
		? FString::Printf(TEXT("[PASS] %s"), *TestName)
		: FString::Printf(TEXT("[PASS] %s — %s"), *TestName, *Details));
}

void FPGXTestRunner::Fail(const FString& TestName, const FString& Issue)
{
	LastResult.Issues.Add(FString::Printf(TEXT("[FAIL] %s — %s"), *TestName, *Issue));
}

bool FPGXTestRunner::ExpectTrue(bool bCondition, const FString& TestName, const FString& FailureIssue)
{
	if (bCondition)
	{
		Pass(TestName);
		return true;
	}

	Fail(TestName, FailureIssue);
	return false;
}

bool FPGXTestRunner::ExpectNotNull(const void* Pointer, const FString& TestName, const FString& FailureIssue)
{
	return ExpectTrue(Pointer != nullptr, TestName, FailureIssue);
}

int32 FPGXTestRunner::GetPassedCount() const
{
	int32 Count = 0;
	for (const FPGXTestRunnerResult& Result : History)
	{
		if (Result.bPassed)
		{
			++Count;
		}
	}
	return Count;
}

int32 FPGXTestRunner::GetFailedCount() const
{
	int32 Count = 0;
	for (const FPGXTestRunnerResult& Result : History)
	{
		if (!Result.bPassed)
		{
			++Count;
		}
	}
	return Count;
}

void FPGXTestRunner::Reset()
{
	EndPlay();
	LastResult = FPGXTestRunnerResult();
	History.Reset();
	State = EPGXTestRunnerState::NotStarted;
}

bool FPGXTestRunner::RunInternal(TArray<FString>& OutIssues)
{
	OutIssues.Add(TEXT("[PASS] Default FPGXTestRunner::RunInternal"));
	return true;
}

UWorld* FPGXTestRunner::GetActiveWorld() const
{
	return ActiveWorldWeak.Get();
}

const UObject* FPGXTestRunner::GetWorldContextObject() const
{
	return WorldContextWeak.Get();
}

void FPGXTestRunner::AddIssue(const FString& Issue)
{
	LastResult.Issues.Add(Issue);
}

void FPGXTestRunner::CompleteRun(bool bPassed, const TArray<FString>& Issues, double StartSeconds)
{
	LastResult.SystemName = SystemName;
	LastResult.SystemVersion = SystemVersion;
	LastResult.TestName = ActiveOptions.TestName;
	LastResult.bPassed = bPassed;
	LastResult.Issues.Append(Issues);
	LastResult.Timestamp = FDateTime::UtcNow();
	LastResult.DurationMs = (FPlatformTime::Seconds() - StartSeconds) * 1000.0;
	LastResult.State = bPassed ? EPGXTestRunnerState::Passed : EPGXTestRunnerState::Failed;

	State = LastResult.State;
	History.Add(LastResult);

	if (ActiveOptions.bRecordResult)
	{
		FPGXTestResultCollector::Get().RecordResult(LastResult.ToCollectorResult());
	}
}
