// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "PGXInteractionTypes.h"
#include "PGXInteractionComponent.generated.h"

class UPGXInteractionCondition;

/**
 * EN: Component that owns local interaction target registration and action state.
 *     No detector is provided; tick remains disabled.
 *
 * ES: Componente que posee registro local de targets de interaccion y estado de acciones.
 *     La deteccion queda diferida; tick permanece desactivado hasta implementar detector data-driven.
 */
UCLASS(ClassGroup=(PGX), BlueprintType, meta=(BlueprintSpawnableComponent))
class PGXINTERACTIONRUNTIME_API UPGXInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPGXInteractionComponent();

	/** EN: Maximum distance for interaction detection / ES: Distancia maxima para deteccion de interaccion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Interaction", meta = (ClampMin = "0.0"))
	float InteractionRange = 200.0f;

	UFUNCTION(BlueprintCallable, Category = "PGX|Interaction")
	FPGXInteractionResult RegisterTarget(AActor* TargetActor, FGameplayTag TargetTag, FText PromptText, int32 Priority = 0);

	UFUNCTION(BlueprintCallable, Category = "PGX|Interaction")
	FPGXInteractionResult UnregisterTarget(FPGXInteractionHandle TargetHandle);

	UFUNCTION(BlueprintCallable, Category = "PGX|Interaction")
	FPGXInteractionResult ValidateTarget(FPGXInteractionHandle TargetHandle) const;

	UFUNCTION(BlueprintCallable, Category = "PGX|Interaction")
	FPGXInteractionResult BeginInteraction(FPGXInteractionHandle TargetHandle, FGameplayTag ActionTag);

	UFUNCTION(BlueprintCallable, Category = "PGX|Interaction")
	FPGXInteractionResult CompleteInteraction(FPGXInteractionHandle ActionHandle);

	UFUNCTION(BlueprintCallable, Category = "PGX|Interaction")
	FPGXInteractionResult CancelInteraction(FPGXInteractionHandle ActionHandle);

	UFUNCTION(BlueprintCallable, Category = "PGX|Interaction")
	int32 CleanupResolvedInteractions();

	UFUNCTION(BlueprintCallable, Category = "PGX|Interaction")
	FPGXInteractionResult EvaluateConditionTyped(const UPGXInteractionCondition* Condition, AActor* Interactor) const;

	UFUNCTION(BlueprintCallable, Category = "PGX|Interaction|Query")
	FPGXInteractionQueryResult QueryBestTargetFromOwner(float MaxRange = -1.0f, bool bRequireInteractableInterface = true) const;

	UFUNCTION(BlueprintCallable, Category = "PGX|Interaction|Query")
	FPGXInteractionQueryResult QueryBestTargetFromLocation(AActor* Interactor, FVector Origin, float MaxRange = -1.0f, bool bRequireInteractableInterface = true) const;

	UFUNCTION(BlueprintCallable, Category = "PGX|Interaction|Prompt")
	FPGXInteractionQueryResult BuildPromptSnapshot(FPGXInteractionHandle TargetHandle, FGameplayTag ActionTag, float Distance = 0.0f) const;

	UFUNCTION(BlueprintPure, Category = "PGX|Interaction")
	bool HasTarget(FPGXInteractionHandle TargetHandle) const;

	UFUNCTION(BlueprintPure, Category = "PGX|Interaction")
	int32 GetRegisteredTargetCount() const;

	UFUNCTION(BlueprintPure, Category = "PGX|Interaction")
	int32 GetActiveInteractionCount() const;

	UFUNCTION(BlueprintPure, Category = "PGX|Interaction")
	int32 GetInteractionRecordCount() const;

	UFUNCTION(BlueprintPure, Category = "PGX|Interaction")
	TArray<FPGXInteractableTarget> GetTargetsSnapshot() const;

	UFUNCTION(BlueprintPure, Category = "PGX|Interaction")
	TArray<FPGXInteractionRecord> GetInteractionRecordsSnapshot() const;

private:
	FPGXInteractableTarget* FindTargetMutable(FPGXInteractionHandle TargetHandle);
	const FPGXInteractableTarget* FindTarget(FPGXInteractionHandle TargetHandle) const;
	FPGXInteractionRecord* FindRecordMutable(FPGXInteractionHandle ActionHandle);
	const FPGXInteractionRecord* FindRecord(FPGXInteractionHandle ActionHandle) const;
	bool IsRecordActive(const FPGXInteractionRecord& Record) const;

	UPROPERTY(Transient)
	TArray<FPGXInteractableTarget> RegisteredTargets;

	UPROPERTY(Transient)
	TArray<FPGXInteractionRecord> InteractionRecords;
};
