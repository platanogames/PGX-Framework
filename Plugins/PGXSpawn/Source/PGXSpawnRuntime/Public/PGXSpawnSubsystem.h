// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXWorldSubsystem.h"
#include "Containers/Ticker.h"
#include "HAL/IConsoleManager.h"
#include "PGXSpawnTypes.h"
#include "PGXSpawnSubsystem.generated.h"

class UPGXSpawnConfig;
class UPGXWaveDefinition;
class APGXSpawnVolume;
class UWorld;

/**
 * EN: World spawn manager. Manages spawn request validation and local active records.
 * ES: Manager de spawn por mundo. Gestiona validacion de peticiones y registros activos locales.
 */
UCLASS(BlueprintType)
class PGXSPAWNRUNTIME_API UPGXSpawnSubsystem : public UPGXWorldSubsystem
{
	GENERATED_BODY()

public:
	/** EN: Called when the subsystem is initialized. / ES: Llamado cuando el subsistema se inicializa. */
	void Initialize(FSubsystemCollectionBase& Collection) override;

	/** EN: Called when the subsystem is deinitialized. / ES: Llamado cuando el subsistema se desinicializa. */
	void Deinitialize() override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Spawn")
	FPGXSpawnResult ValidateSpawnRequest(const FPGXSpawnRequest& Request) const;

	UFUNCTION(BlueprintCallable, Category = "PGX|Spawn")
	FPGXSpawnResult ExecuteSpawnRequest(const FPGXSpawnRequest& Request);

	UFUNCTION(BlueprintCallable, Category = "PGX|Spawn")
	FPGXSpawnResult RegisterSpawnRecord(const FPGXSpawnRequest& Request, AActor* SpawnedActor = nullptr);

	UFUNCTION(BlueprintCallable, Category = "PGX|Spawn")
	FPGXSpawnResult CompleteSpawnRecord(FPGXSpawnRequestHandle Handle, EPGXSpawnResultCode ResultCode = EPGXSpawnResultCode::Success, AActor* SpawnedActor = nullptr, FString Message = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "PGX|Spawn")
	FPGXSpawnResult CancelSpawnRecord(FPGXSpawnRequestHandle Handle, FString Message = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "PGX|Spawn")
	int32 CleanupInactiveSpawnRecords();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Spawn")
	bool HasSpawnRecord(FPGXSpawnRequestHandle Handle) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Spawn")
	int32 GetActiveSpawnCount() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Spawn")
	int32 GetTotalSpawnRecordCount() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Spawn")
	TArray<FPGXSpawnRecord> GetSpawnRecordsSnapshot() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Spawn")
	TArray<FPGXSpawnRecord> GetLastCleanedSpawnRecordsSnapshot() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Spawn")
	UPGXSpawnConfig* GetActiveSpawnConfig() const;

	// ========================================================================
	// Wave scheduling and bookkeeping
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "PGX|Spawn|Wave")
	FPGXSpawnResult StartWave(UPGXWaveDefinition* WaveDef);

	UFUNCTION(BlueprintCallable, Category = "PGX|Spawn|Wave")
	FPGXSpawnResult CancelWave(FGameplayTag WaveTag);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Spawn|Wave")
	TArray<FPGXSpawnRecord> GetActiveWavesSnapshot() const;

	// ========================================================================
	// EN: Budget / Pool delegates
	// ES: Delegates de budget / pool
	// ========================================================================

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPGXBudgetWarning, int32, CurrentCount, int32, MaxCount);

	UPROPERTY(BlueprintAssignable, Category = "PGX|Spawn|Delegates")
	FOnPGXBudgetWarning OnBudgetWarning;

	// ========================================================================
	// Volume registration. Automatic point discovery remains limited.
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "PGX|Spawn|Volume")
	int32 RegisterVolume(APGXSpawnVolume* Volume);

	// ========================================================================
	// Runtime diagnostics snapshot
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "PGX|Spawn|Debug")
	FPGXSpawnDebugSnapshot GetDebugSnapshot() const;

	// ========================================================================
	// Spawn condition evaluator
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "PGX|Spawn|Condition")
	bool EvaluateConditions(const FPGXSpawnRequest& Request, const TArray<FPGXSpawnConditionDefinition>& Conditions) const;

	UFUNCTION(BlueprintCallable, Category = "PGX|Spawn|Condition")
	bool EvaluateCondition(const FPGXSpawnConditionDefinition& Condition, const FPGXSpawnRequest& Request) const;

	// ========================================================================
	// Actor pool API
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "PGX|Spawn|Pool")
	void ReturnActorToPool(AActor* Actor);

#if WITH_DEV_AUTOMATION_TESTS
	void InjectTestSpawnConfig(UPGXSpawnConfig* InConfig);
	void ClearSpawnRecordsForTesting();
	FPGXSpawnResult ExecuteSpawnRequestForTesting(UWorld* World, const FPGXSpawnRequest& Request);
	void SetForceNextSpawnActorFailureForTesting(bool bInForceFailure);
#endif

private:
	// ========================================================================
	// EN: Wave runtime state (wave implementation — internal)
	// ES: Estado runtime de oleadas (wave implementation — interno)
	// ========================================================================
	struct FPGXSpawnWaveRuntimeState
	{
		FPGXSpawnRequestHandle WaveHandle;
		TWeakObjectPtr<UPGXWaveDefinition> WaveDef;
		FGameplayTag WaveTag;
		int32 SpawnedSoFar = 0;
		double StartedTime = 0.0;
		double LastSpawnTime = 0.0;
		EPGXSpawnRequestStatus Status = EPGXSpawnRequestStatus::None;
		TArray<FPGXSpawnRequestHandle> SpawnedRecordHandles;
	};

	void EnsureRuntimeObjects() const;
	bool IsRecordActive(const FPGXSpawnRecord& Record) const;
	int32 GetMaxConcurrentSpawnBudget() const;
	FPGXSpawnResult ExecuteSpawnRequestInWorld(UWorld* World, const FPGXSpawnRequest& Request);
	void AppendLifecycleEvent(FPGXSpawnRecord& Record, EPGXSpawnLifecycleEventType EventType, EPGXSpawnResultCode ResultCode, const FString& Message) const;

	// EN: FTSTicker callback for wave cadence. Returns true to keep ticker alive.
	bool OnWaveTick(float DeltaTime);

	// EN: Try to acquire a pooled actor of the given class. Returns nullptr if pool disabled, empty, or all stale.
	AActor* AcquireFromPool(TSubclassOf<AActor> SpawnClass);

	// EN: Return an actor to the pool. If pool disabled, actor is destroyed (or remains in world per config).
	void ReleaseToPool(AActor* Actor);

	// EN: Track peak concurrent spawn count + fire OnBudgetWarning when threshold crossed.
	void UpdateBudgetTracking();

	UPROPERTY(Transient)
	mutable TObjectPtr<UPGXSpawnConfig> SpawnConfig;

	UPROPERTY(Transient)
	TMap<FGuid, FPGXSpawnRecord> SpawnRecords;

	UPROPERTY(Transient)
	TArray<FPGXSpawnRecord> LastCleanedSpawnRecords;

	// EN: Active waves (wave implementation). Index 0 = oldest, last = newest. No UPROPERTY (runtime-only, transient by design).
	TArray<FPGXSpawnWaveRuntimeState> ActiveWaves;

	// EN: FTSTicker handle for wave cadence. Set on first StartWave, cleared on Deinitialize or when ActiveWaves becomes empty.
	FTSTicker::FDelegateHandle WaveTickerHandle;

	// EN: Object pool (pool implementation). Keyed by spawn class. Values are weak refs to hidden/disabled actors awaiting reuse.
	TMap<TSubclassOf<AActor>, TArray<TWeakObjectPtr<AActor>>> ObjectPool;

	// EN: Peak concurrent spawn count (pool implementation). Exposed via GetDebugSnapshot.
	int32 PeakConcurrentActors = 0;

	// EN: Console commands registered in Initialize, unregistered in Deinitialize (console diagnostics).
	TArray<IConsoleCommand*> RegisteredConsoleCommands;

	// EN: Console command handlers (console diagnostics)
	void HandleConsoleList(const TArray<FString>& Args);
	void HandleConsoleCleanup(const TArray<FString>& Args);
	void HandleConsoleBudget(const TArray<FString>& Args);
	void HandleConsoleWaves(const TArray<FString>& Args);
	void HandleConsoleTriggerPoint(const TArray<FString>& Args);
	void HandleConsolePoolClear(const TArray<FString>& Args);

#if WITH_DEV_AUTOMATION_TESTS
	bool bForceNextSpawnActorFailureForTesting = false;
#endif
};
