// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Engine/LocalPlayer.h"
#include "Interfaces/PGXTaggedRegistry.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/ObjectKey.h"
#include "PGXInputTypes.h"
#include "PGXInputSubsystem.generated.h"

class UPGXInputBuffer;
class UPGXInputConfig;
class UPGXInputContext;
class UEnhancedInputLocalPlayerSubsystem;
class UInputMappingContext;

/**
 * EN: Input context manager subsystem.
 *     Activates/deactivates GameplayTag addressed input contexts deterministically.
 *
 * ES: Subsistema manager de contextos de input.
 *     Activa/desactiva contextos de input direccionados por GameplayTag de forma determinista.
 */
UCLASS(BlueprintType)
class PGXINPUTRUNTIME_API UPGXInputSubsystem : public UGameInstanceSubsystem, public IPGXTaggedRegistry
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface

	UFUNCTION(BlueprintCallable, Category = "PGX|Input")
	FPGXInputContextResult ActivateContext(FGameplayTag ContextTag, int32 PriorityOverride = -1);

	UFUNCTION(BlueprintCallable, Category = "PGX|Input")
	FPGXInputContextResult ActivateContextForLocalPlayer(FGameplayTag ContextTag, ULocalPlayer* LocalPlayer, int32 PriorityOverride = -1);

	UFUNCTION(BlueprintCallable, Category = "PGX|Input")
	FPGXInputContextResult DeactivateContext(FGameplayTag ContextTag);

	UFUNCTION(BlueprintCallable, Category = "PGX|Input")
	FPGXInputContextResult DeactivateContextForLocalPlayer(FGameplayTag ContextTag, ULocalPlayer* LocalPlayer);

	UFUNCTION(BlueprintCallable, Category = "PGX|Input")
	void DeactivateAllContexts();

	UFUNCTION(BlueprintCallable, Category = "PGX|Input")
	void DeactivateAllContextsForLocalPlayer(ULocalPlayer* LocalPlayer);

	UFUNCTION(BlueprintPure, Category = "PGX|Input")
	bool IsContextActive(FGameplayTag ContextTag) const;

	UFUNCTION(BlueprintPure, Category = "PGX|Input")
	TArray<FPGXActiveInputContextEntry> GetActiveContexts() const;

	UFUNCTION(BlueprintPure, Category = "PGX|Input")
	int32 GetActiveContextCount() const;

	UFUNCTION(BlueprintPure, Category = "PGX|Input")
	UPGXInputContext* FindContextAsset(FGameplayTag ContextTag) const;

	UFUNCTION(BlueprintPure, Category = "PGX|Input")
	UPGXInputConfig* GetActiveInputConfig() const;

	UFUNCTION(BlueprintPure, Category = "PGX|Input")
	UPGXInputBuffer* GetInputBuffer() const;

	// EN: IPGXTaggedRegistry integration — input context cache facade.
	// ES: Adopcion IPGXTaggedRegistry — fachada del cache de contextos de input.
	bool HasEntryByTag(FGameplayTag Tag) const override;
	int32 GetCount() const override;
	void GetSnapshot(TArray<FGameplayTag>& OutTags) const override;

#if WITH_DEV_AUTOMATION_TESTS
	void InjectTestInputConfig(UPGXInputConfig* InConfig);
	void InjectTestContext(UPGXInputContext* InContext);
	void ClearTestContexts();
#endif

private:
	void EnsureRuntimeObjects();
	void RebuildContextCache();
	void SortActiveContexts();
	int32 ResolvePriority(const UPGXInputContext* Context, int32 PriorityOverride) const;
	FPGXInputContextResult ResolveEnhancedInputApplyTargets(FGameplayTag ContextTag, ULocalPlayer* LocalPlayer, UPGXInputContext*& OutContext, UInputMappingContext*& OutMappingContext, UEnhancedInputLocalPlayerSubsystem*& OutEnhancedInputSubsystem) const;
	void RemoveAppliedContextFromLocalPlayer(ULocalPlayer* LocalPlayer, FGameplayTag ContextTag);
	void RemoveAppliedContextsPrunedByExclusive(int32 ResolvedPriority);

	UPROPERTY(Transient)
	TObjectPtr<UPGXInputConfig> InputConfig;

	UPROPERTY(Transient)
	TObjectPtr<UPGXInputBuffer> InputBuffer;

	UPROPERTY(Transient)
	TArray<FPGXActiveInputContextEntry> ActiveContexts;

	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UPGXInputContext>> ContextCache;

	TMap<TObjectKey<ULocalPlayer>, TSet<FGameplayTag>> AppliedContextsByLocalPlayer;
};
