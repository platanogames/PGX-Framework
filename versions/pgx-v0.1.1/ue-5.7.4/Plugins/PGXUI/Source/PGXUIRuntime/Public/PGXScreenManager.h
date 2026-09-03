// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PGXUITypes.h"
#include "PGXScreenManager.generated.h"

/**
 * EN: Screen stack manager.
 *     Handles presentation-only screen push/pop state and bounded stack policy.
 *
 * ES: Manager de stack de pantallas.
 *     Maneja estado de push/pop solo de presentacion y politica de stack acotado.
 */
UCLASS(BlueprintType)
class PGXUIRUNTIME_API UPGXScreenManager : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(int32 InMaxStackDepth);

	UFUNCTION(BlueprintCallable, Category = "PGX|UI")
	FPGXUIResult PushScreen(FGameplayTag ScreenTag, FString DebugName, int32 Layer = 0);

	UFUNCTION(BlueprintCallable, Category = "PGX|UI")
	FPGXUIResult PopScreen();

	UFUNCTION(BlueprintCallable, Category = "PGX|UI")
	FPGXUIResult CloseScreen(FPGXUIHandle ScreenHandle);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|UI")
	bool HasScreen(FPGXUIHandle ScreenHandle) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|UI")
	int32 GetOpenScreenCount() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|UI")
	int32 GetMaxStackDepth() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|UI")
	TArray<FPGXUIScreenEntry> GetScreenStackSnapshot() const;

	void Clear();

private:
	FPGXUIScreenEntry* FindOpenScreen(FPGXUIHandle ScreenHandle);
	const FPGXUIScreenEntry* FindOpenScreen(FPGXUIHandle ScreenHandle) const;

	UPROPERTY(Transient)
	TArray<FPGXUIScreenEntry> ScreenStack;

	UPROPERTY(Transient)
	int32 MaxStackDepth = 10;
};