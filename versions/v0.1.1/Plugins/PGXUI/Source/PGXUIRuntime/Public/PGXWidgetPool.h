// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PGXUITypes.h"
#include "PGXWidgetPool.generated.h"

/**
 * EN: State-only widget pool for lists, grids, and repetitive UI elements.
 *     Tracks acquire/release policy without spawning widgets.
 *
 * ES: Pool de widgets solo de estado para listas, grids y elementos repetitivos.
 *     Registra politica de acquire/release sin crear widgets.
 */
UCLASS(BlueprintType)
class PGXUIRUNTIME_API UPGXWidgetPool : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(int32 InCapacity);

	UFUNCTION(BlueprintCallable, Category = "PGX|UI")
	FPGXUIResult AcquireWidget(TSubclassOf<UUserWidget> WidgetClass, FString DebugName);

	UFUNCTION(BlueprintCallable, Category = "PGX|UI")
	FPGXUIResult ReleaseWidget(FPGXUIHandle WidgetHandle);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|UI")
	bool HasAcquiredWidget(FPGXUIHandle WidgetHandle) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|UI")
	int32 GetCapacity() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|UI")
	int32 GetAcquiredCount() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|UI")
	int32 GetAvailableCount() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|UI")
	TArray<FPGXWidgetPoolEntry> GetPoolSnapshot() const;

	void Clear();

private:
	FPGXWidgetPoolEntry* FindPoolEntry(FPGXUIHandle WidgetHandle);
	const FPGXWidgetPoolEntry* FindPoolEntry(FPGXUIHandle WidgetHandle) const;

	UPROPERTY(Transient)
	TArray<FPGXWidgetPoolEntry> PoolEntries;

	UPROPERTY(Transient)
	int32 Capacity = 16;
};