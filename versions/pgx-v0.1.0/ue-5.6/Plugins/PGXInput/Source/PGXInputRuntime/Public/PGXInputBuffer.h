// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PGXInputTypes.h"
#include "PGXInputBuffer.generated.h"

/**
 * EN: Input buffer for combo detection and queued action support.
 *     Records recent inputs within a configurable time window.
 *
 * ES: Buffer de input para deteccion de combos y soporte de acciones en cola.
 *     Registra inputs recientes dentro de una ventana de tiempo configurable.
 */
UCLASS(BlueprintType)
class PGXINPUTRUNTIME_API UPGXInputBuffer : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "PGX|Input")
	void Configure(int32 InCapacity, double InWindowSeconds);

	UFUNCTION(BlueprintCallable, Category = "PGX|Input")
	void RecordInput(FGameplayTag ActionTag, FVector Value, double Timestamp = -1.0);

	UFUNCTION(BlueprintPure, Category = "PGX|Input")
	bool ContainsRecentInput(FGameplayTag ActionTag, double CurrentTime = -1.0) const;

	UFUNCTION(BlueprintCallable, Category = "PGX|Input")
	bool ConsumeRecentInput(FGameplayTag ActionTag, double CurrentTime = -1.0);

	UFUNCTION(BlueprintCallable, Category = "PGX|Input")
	void Clear();

	UFUNCTION(BlueprintPure, Category = "PGX|Input")
	TArray<FPGXInputBufferEntry> GetBufferedInputs() const;

	UFUNCTION(BlueprintPure, Category = "PGX|Input")
	int32 Num() const;

	UFUNCTION(BlueprintPure, Category = "PGX|Input")
	int32 GetCapacity() const;

	UFUNCTION(BlueprintPure, Category = "PGX|Input")
	double GetWindowSeconds() const;

private:
	void TrimExpired(double CurrentTime);
	void EnforceCapacity();
	static double ResolveTime(double MaybeTime);

	UPROPERTY(Transient)
	TArray<FPGXInputBufferEntry> Entries;

	UPROPERTY(Transient)
	int32 Capacity = 16;

	UPROPERTY(Transient)
	double WindowSeconds = 0.15;
};
