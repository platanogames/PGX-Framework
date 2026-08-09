// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "Observability/PGXObservable.h"
#include "PGXCraftingTypes.generated.h"

/** EN: Typed outcome codes for crafting operations / ES: Codigos tipados para operaciones de crafting */
UENUM(BlueprintType)
enum class EPGXCraftingResultCode : uint8
{
	Success = 0 UMETA(DisplayName = "Success"),
	InvalidRecipe = 1 UMETA(DisplayName = "Invalid Recipe"),
	DuplicateRecipe = 2 UMETA(DisplayName = "Duplicate Recipe"),
	RecipeNotFound = 3 UMETA(DisplayName = "Recipe Not Found"),
	InvalidRequest = 4 UMETA(DisplayName = "Invalid Request"),
	Unsupported = 5 UMETA(DisplayName = "Unsupported"),
	JobNotFound = 6 UMETA(DisplayName = "Job Not Found"),
	AlreadyResolved = 7 UMETA(DisplayName = "Already Resolved"),
	InternalError = 8 UMETA(DisplayName = "Internal Error")
};

/** EN: Baseline craft job lifecycle state / ES: Estado base del ciclo de vida de trabajo de crafting */
UENUM(BlueprintType)
enum class EPGXCraftJobState : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Validated = 1 UMETA(DisplayName = "Validated"),
	Active = 2 UMETA(DisplayName = "Active"),
	Completed = 3 UMETA(DisplayName = "Completed"),
	Cancelled = 4 UMETA(DisplayName = "Cancelled"),
	Failed = 5 UMETA(DisplayName = "Failed")
};

/** EN: Stable opaque craft job handle / ES: Handle opaco estable de trabajo de crafting */
USTRUCT(BlueprintType)
struct PGXCRAFTINGRUNTIME_API FPGXCraftJobHandle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Crafting")
	FGuid Id;

	bool IsValid() const { return Id.IsValid(); }
	static FPGXCraftJobHandle NewHandle();
};

/** EN: Generic ingredient or output requirement / ES: Requisito generico de ingrediente o resultado */
USTRUCT(BlueprintType)
struct PGXCRAFTINGRUNTIME_API FPGXCraftingResourceQuantity
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Crafting", meta = (Categories = "PGX.Crafting.Resource"))
	FGameplayTag ResourceTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Crafting", meta = (ClampMin = "1"))
	int32 Quantity = 1;

	bool IsValid() const { return ResourceTag.IsValid() && Quantity > 0; }
};

/** EN: Runtime-safe recipe definition snapshot / ES: Snapshot de definicion de receta seguro para runtime */
USTRUCT(BlueprintType)
struct PGXCRAFTINGRUNTIME_API FPGXCraftingRecipeDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Crafting", meta = (Categories = "PGX.Crafting.Recipe"))
	FGameplayTag RecipeTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Crafting", meta = (Categories = "PGX.Crafting.Category"))
	FGameplayTag CategoryTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Crafting")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Crafting")
	TArray<FPGXCraftingResourceQuantity> Inputs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Crafting")
	TArray<FPGXCraftingResourceQuantity> Outputs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Crafting", meta = (ClampMin = "0.0"))
	float CraftDurationSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Crafting")
	bool bIsSupportedByBaseline = true;

	bool IsValid() const;
};

/** EN: DataAsset wrapper for authored recipes / ES: DataAsset contenedor para recetas authoradas */
UCLASS(BlueprintType)
class PGXCRAFTINGRUNTIME_API UPGXRecipeDefinition : public UDataAsset, public IPGXObservable
{
	GENERATED_BODY()

public:
	static const FName SchemaVersion;

	//~ Begin IPGXObservable
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;
	//~ End IPGXObservable

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Crafting")
	FPGXCraftingRecipeDefinition Recipe;
};

/** EN: Craft request against a registered recipe / ES: Peticion de crafting contra una receta registrada */
USTRUCT(BlueprintType)
struct PGXCRAFTINGRUNTIME_API FPGXCraftRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Crafting", meta = (Categories = "PGX.Crafting.Recipe"))
	FGameplayTag RecipeTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Crafting", meta = (ClampMin = "1"))
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Crafting", meta = (Categories = "PGX.Crafting.Source"))
	FGameplayTag SourceTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Crafting")
	bool bSimulateOnly = false;

	bool IsValid() const { return RecipeTag.IsValid() && Quantity > 0; }
};

/** EN: Craft job record snapshot / ES: Snapshot de registro de trabajo de crafting */
USTRUCT(BlueprintType)
struct PGXCRAFTINGRUNTIME_API FPGXCraftJobRecord
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Crafting")
	FPGXCraftJobHandle Handle;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Crafting")
	FPGXCraftRequest Request;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Crafting")
	FPGXCraftingRecipeDefinition Recipe;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Crafting")
	EPGXCraftJobState State = EPGXCraftJobState::None;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Crafting")
	EPGXCraftingResultCode ResultCode = EPGXCraftingResultCode::InternalError;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Crafting")
	FString Message;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Crafting")
	double CreatedTimeSeconds = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Crafting")
	double ResolvedTimeSeconds = 0.0;
};

/** EN: Typed craft operation result / ES: Resultado tipado de operacion de crafting */
USTRUCT(BlueprintType)
struct PGXCRAFTINGRUNTIME_API FPGXCraftingResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Crafting")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Crafting")
	EPGXCraftingResultCode Code = EPGXCraftingResultCode::InternalError;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Crafting")
	EPGXCraftJobState State = EPGXCraftJobState::None;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Crafting")
	FPGXCraftJobHandle Handle;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Crafting", meta = (Categories = "PGX.Crafting.Recipe"))
	FGameplayTag RecipeTag;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Crafting")
	FString Message;

	static FPGXCraftingResult Success(FGameplayTag InRecipeTag, FPGXCraftJobHandle InHandle, EPGXCraftJobState InState, FString InMessage = FString());
	static FPGXCraftingResult Failure(EPGXCraftingResultCode InCode, EPGXCraftJobState InState, FString InMessage, FGameplayTag InRecipeTag = FGameplayTag(), FPGXCraftJobHandle InHandle = FPGXCraftJobHandle());
};