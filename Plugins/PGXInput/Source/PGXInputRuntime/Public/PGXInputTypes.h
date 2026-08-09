// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Observability/PGXValidationResult.h"
#include "PGXInputTypes.generated.h"

class UPGXInputContext;

/** EN: Active input device class / ES: Clase de dispositivo de input activo */
UENUM(BlueprintType)
enum class EPGXInputDeviceType : uint8
{
	Unknown = 0 UMETA(DisplayName = "Unknown"),
	KeyboardMouse = 1 UMETA(DisplayName = "Keyboard + Mouse"),
	Gamepad = 2 UMETA(DisplayName = "Gamepad"),
	Touch = 3 UMETA(DisplayName = "Touch")
};

/** EN: Context stack activation behavior / ES: Comportamiento de activacion en el stack de contextos */
UENUM(BlueprintType)
enum class EPGXInputContextActivationMode : uint8
{
	Additive = 0 UMETA(DisplayName = "Additive"),
	Exclusive = 1 UMETA(DisplayName = "Exclusive")
};

/** EN: Typed result for context stack operations / ES: Resultado tipado para operaciones de stack de contextos */
UENUM(BlueprintType)
enum class EPGXInputContextResultCode : uint8
{
	Success = 0 UMETA(DisplayName = "Success"),
	ContextNotFound = 1 UMETA(DisplayName = "Context Not Found"),
	AlreadyActive = 2 UMETA(DisplayName = "Already Active"),
	AlreadyInactive = 3 UMETA(DisplayName = "Already Inactive"),
	InvalidTag = 4 UMETA(DisplayName = "Invalid Tag"),
	MappingContextMissing = 5 UMETA(DisplayName = "Mapping Context Missing"),
	LocalPlayerNotReady = 6 UMETA(DisplayName = "Local Player Not Ready")
};

/** EN: Detailed result for context operations / ES: Resultado detallado para operaciones de contexto */
USTRUCT(BlueprintType)
struct PGXINPUTRUNTIME_API FPGXInputContextResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Input")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Input")
	EPGXInputContextResultCode Code = EPGXInputContextResultCode::ContextNotFound;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Input")
	FGameplayTag ContextTag;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Input")
	FString Message;

	static FPGXInputContextResult Success(FGameplayTag InContextTag, FString InMessage = FString());
	static FPGXInputContextResult Failure(EPGXInputContextResultCode InCode, FGameplayTag InContextTag, FString InMessage);

	/**
	 * EN: Convert to the canonical FPGXValidationResult for cross-cutting
	 *     concerns. Maps bSuccess -> bValid, EPGXInputContextResultCode ->
	 *     FName. The ContextTag is carried as the Path field (useful for
	 *     "which context failed" diagnostics).
	 *
	 *      Bridge to FPGXValidationResult.
	 *
	 * ES: Convertir al FPGXValidationResult canonico. Mapea bSuccess -> bValid,
	 *     EPGXInputContextResultCode -> FName. El ContextTag se lleva como el
	 *     Path (util para diagnostica de "que contexto fallo").
	 */
	FPGXValidationResult ToValidationResult() const
	{
		if (bSuccess)
		{
			return FPGXValidationResult::MakeValid();
		}
		const FName CodeName(*UEnum::GetValueAsString(Code));
		FPGXValidationResult R;
		R.AddError(CodeName, ContextTag.ToString(), FText::FromString(Message));
		return R;
	}
};

/** EN: Config reference to an input context / ES: Referencia de config a un contexto de input */
USTRUCT(BlueprintType)
struct PGXINPUTRUNTIME_API FPGXInputContextEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Input", meta = (Categories = "PGX.Input.Context"))
	FGameplayTag ContextTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Input")
	TSoftObjectPtr<UPGXInputContext> Context;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Input")
	int32 PriorityOverride = INDEX_NONE;
};

/** EN: Active context stack entry / ES: Entrada activa del stack de contextos */
USTRUCT(BlueprintType)
struct PGXINPUTRUNTIME_API FPGXActiveInputContextEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Input")
	FGameplayTag ContextTag;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Input")
	int32 Priority = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Input")
	EPGXInputContextActivationMode ActivationMode = EPGXInputContextActivationMode::Additive;

	TWeakObjectPtr<UPGXInputContext> Context;
};

/** EN: Single buffered input event / ES: Evento de input registrado en buffer */
USTRUCT(BlueprintType)
struct PGXINPUTRUNTIME_API FPGXInputBufferEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Input", meta = (Categories = "PGX.Input.Action"))
	FGameplayTag ActionTag;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Input")
	double Timestamp = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Input")
	FVector Value = FVector::ZeroVector;
};
