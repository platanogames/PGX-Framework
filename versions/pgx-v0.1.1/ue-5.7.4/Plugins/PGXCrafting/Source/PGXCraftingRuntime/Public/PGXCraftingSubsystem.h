// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/PGXTaggedRegistry.h"
#include "PGXCraftingTypes.h"
#include "PGXCraftingSubsystem.generated.h"

/**
 * EN: Baseline crafting subsystem. Owns local recipe registry and craft job simulation state.
 * ES: Subsistema base de crafting. Posee registro local de recetas y estado de simulacion de trabajos.
 */
UCLASS(BlueprintType)
class PGXCRAFTINGRUNTIME_API UPGXCraftingSubsystem : public UGameInstanceSubsystem, public IPGXTaggedRegistry
{
	GENERATED_BODY()

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "PGX|Crafting")
	FPGXCraftingResult RegisterRecipe(const FPGXCraftingRecipeDefinition& Recipe);

	UFUNCTION(BlueprintCallable, Category = "PGX|Crafting")
	FPGXCraftingResult RegisterRecipeAsset(const UPGXRecipeDefinition* RecipeAsset);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Crafting")
	FPGXCraftingResult ValidateRecipeDefinition(const FPGXCraftingRecipeDefinition& Recipe) const;

	// EN: Not BlueprintCallable — returns a raw struct pointer, which UHT cannot
	//     expose to Blueprint without changing the signature (out-param + bool).
	//     Out of scope for this BP-exposure pass (signature-only change).
	// ES: No BlueprintCallable — retorna un puntero crudo a struct, que UHT no
	//     puede exponer a Blueprint sin cambiar la firma (out-param + bool).
	//     Fuera de scope para este pase de exposicion BP (cambio solo de specifiers).
	const FPGXCraftingRecipeDefinition* FindRecipe(FGameplayTag RecipeTag) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Crafting")
	bool HasRecipe(FGameplayTag RecipeTag) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Crafting")
	int32 GetRegisteredRecipeCount() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Crafting")
	TArray<FPGXCraftingRecipeDefinition> GetRegisteredRecipesSnapshot() const;

	// EN: IPGXTaggedRegistry adoption — recipe registry facade.
	// ES: Adopcion IPGXTaggedRegistry — fachada del registry de recetas.
	bool HasEntryByTag(FGameplayTag Tag) const override;
	int32 GetCount() const override;
	void GetSnapshot(TArray<FGameplayTag>& OutTags) const override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Crafting")
	FPGXCraftingResult ValidateCraftRequest(const FPGXCraftRequest& Request) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Crafting")
	FPGXCraftingResult SimulateCraft(const FPGXCraftRequest& Request) const;

	UFUNCTION(BlueprintCallable, Category = "PGX|Crafting")
	FPGXCraftingResult StartCraft(const FPGXCraftRequest& Request);

	// EN: Testing-only helper (name + intent). Not exposed to Blueprint.
	// ES: Helper solo-testing (nombre + intencion). No expuesto a Blueprint.
	FPGXCraftingResult CompleteCraftForTesting(FPGXCraftJobHandle Handle, FString Message = FString());

	// EN: UHT does not parse `FString = FString()` default args on UFUNCTIONs
	//     (same root cause as PGXUI PushScreen — runtime-safety 2026-06-30). Default
	//     dropped so this compiles as BlueprintCallable.
	// ES: UHT no parsea defaults `FString = FString()` en UFUNCTIONs (misma
	//     causa raiz que PGXUI PushScreen — runtime-safety 2026-06-30). Default
	//     eliminado para que compile como BlueprintCallable.
	UFUNCTION(BlueprintCallable, Category = "PGX|Crafting")
	FPGXCraftingResult CancelCraft(FPGXCraftJobHandle Handle, FString Message);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Crafting")
	bool HasCraftJob(FPGXCraftJobHandle Handle) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Crafting")
	bool GetCraftJob(FPGXCraftJobHandle Handle, FPGXCraftJobRecord& OutRecord) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Crafting")
	int32 GetActiveCraftJobCount() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Crafting")
	int32 GetCraftJobCount() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Crafting")
	TArray<FPGXCraftJobRecord> GetCraftJobsSnapshot() const;

	UFUNCTION(BlueprintCallable, Category = "PGX|Crafting")
	int32 CleanupResolvedCraftJobs();

#if WITH_DEV_AUTOMATION_TESTS
	void ClearCraftingStateForTesting();
	void SetMaxActiveCraftJobsForTesting(int32 InMaxActiveCraftJobs);
#endif

private:
	FPGXCraftJobRecord* FindCraftJobMutable(FPGXCraftJobHandle Handle);
	const FPGXCraftJobRecord* FindCraftJob(FPGXCraftJobHandle Handle) const;
	bool IsCraftJobActive(const FPGXCraftJobRecord& Record) const;

	UPROPERTY(Transient)
	TArray<FPGXCraftingRecipeDefinition> Recipes;

	UPROPERTY(Transient)
	TArray<FPGXCraftJobRecord> CraftJobs;

	UPROPERTY(Transient)
	int32 MaxActiveCraftJobs = 8;
};
