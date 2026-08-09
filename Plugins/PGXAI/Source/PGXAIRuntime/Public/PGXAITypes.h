// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTree.h"
#include "GameplayTagContainer.h"
#include "PGXAITypes.generated.h"

class AAIController;

/**
 * EN: Typed result enum for PGXAI public API surfaces. Mirrors the result-policy convention used
 *     across PGXFlowResult / PGXSaveResult — uniform success/failure surface so callers can
 *     branch on enum without parsing strings or interpreting nullptrs. Development Preview primitive.
 * ES: Enum de resultado tipado para superficies API publicas de PGXAI. Sigue la convencion
 *     uniforme PGXFlowResult / PGXSaveResult.
 */
UENUM(BlueprintType)
enum class EPGXAIResultCode : uint8
{
	/** EN: Operation succeeded. / ES: Operacion exitosa. */
	Success                 UMETA(DisplayName = "Success"),

	/** EN: Subsystem not initialized or not available. / ES: Subsystem no inicializado o no disponible. */
	SubsystemUnavailable    UMETA(DisplayName = "Subsystem Unavailable"),

	/** EN: Required input was invalid (null pointer, invalid handle, malformed tag, etc.). / ES: Input invalido. */
	InvalidInput            UMETA(DisplayName = "Invalid Input"),

	/** EN: Lookup failed — agent / profile / config not found. / ES: Lookup fallo — agente / profile / config no encontrado. */
	NotFound                UMETA(DisplayName = "Not Found"),

	/** EN: Operation rejected by policy (auth gate, dedup gate, profile constraint). / ES: Rechazado por policy. */
	PolicyRejected          UMETA(DisplayName = "Policy Rejected"),

	/** EN: Controller world does not match subsystem world. / ES: El mundo del controller no coincide con el subsystem. */
	InvalidWorld            UMETA(DisplayName = "Invalid World"),

	/** EN: Behavior Tree asset is missing or invalid. / ES: Asset Behavior Tree ausente o invalido. */
	BehaviorTreeUnavailable UMETA(DisplayName = "Behavior Tree Unavailable"),

	/** EN: AIController::RunBehaviorTree returned failure. / ES: RunBehaviorTree fallo. */
	BehaviorTreeRunFailed   UMETA(DisplayName = "Behavior Tree Run Failed"),

	/** EN: Generic failure with diagnostic message (avoid when a more specific code applies). / ES: Fallo generico. */
	Failure                 UMETA(DisplayName = "Failure")
};

/**
 * EN: Typed result for PGXAI public API. `bSucceeded` is the canonical success predicate;
 *     `Code` carries the enum reason; `DiagnosticMessage` is human-readable detail for logs
 *     and inspector telemetry. Default-constructed value represents an inconclusive
 *     "no-op" outcome (Code=Success, bSucceeded=true).
 * ES: Resultado tipado para API publica PGXAI. `bSucceeded` es el predicado canonico de exito;
 *     `Code` lleva la razon enum; `DiagnosticMessage` es texto humano-legible para logs e
 *     inspector.
 */
USTRUCT(BlueprintType)
struct PGXAIRUNTIME_API FPGXAIResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bSucceeded = true;

	UPROPERTY(BlueprintReadOnly)
	EPGXAIResultCode Code = EPGXAIResultCode::Success;

	UPROPERTY(BlueprintReadOnly)
	FString DiagnosticMessage;

	static FPGXAIResult MakeSuccess()
	{
		return FPGXAIResult{};
	}

	static FPGXAIResult MakeFailure(EPGXAIResultCode InCode, const FString& InMessage = TEXT(""))
	{
		FPGXAIResult R;
		R.bSucceeded = false;
		R.Code = InCode;
		R.DiagnosticMessage = InMessage;
		return R;
	}
};

/**
 * EN: Stable handle for a registered AI agent. Carries the subsystem-assigned `AgentId` plus a
 *     weak pointer back to the controller for safe dereference. Default-constructed (AgentId==0)
 *     handles are invalid; subsystem assigns ids starting at 1. Development Preview primitive — full
 *     lifecycle integration with perception, Behavior Trees and squads is not included.
 * ES: Handle estable para un agente AI registrado. Lleva el `AgentId` asignado por el subsystem
 *     mas un weak pointer al controller para deref seguro. Handle por defecto (AgentId==0) es
 *     invalido; el subsystem asigna ids empezando en 1.
 */
USTRUCT(BlueprintType)
struct PGXAIRUNTIME_API FPGXAIAgentHandle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 AgentId = 0;

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AAIController> Controller;

	bool IsValid() const { return AgentId != 0; }

	bool operator==(const FPGXAIAgentHandle& Other) const { return AgentId == Other.AgentId; }
	bool operator!=(const FPGXAIAgentHandle& Other) const { return !(*this == Other); }
};

FORCEINLINE uint32 GetTypeHash(const FPGXAIAgentHandle& Handle)
{
	return GetTypeHash(Handle.AgentId);
}

/**
 * EN: Typed perception event shape. The Development Preview ships the SHAPE (USTRUCT layout) so
 *     downstream consumers can compile against it; the actual emission pipeline (UE stimulus
 *     normalization, profile filtering and message broadcast) is not part of the current runtime contract
 *     per perception pipeline. Default-construct yields invalid tags / zero confidence / wall-time stamp.
 * ES: Forma tipada del evento de percepcion. Development Preview entrega solo el SHAPE; el pipeline
 *     real de emision se implementa en una version futura.
 */
USTRUCT(BlueprintType)
struct PGXAIRUNTIME_API FPGXAIPerceptionEvent
{
	GENERATED_BODY()

	/** EN: Tag identifying the kind of stimulus (sight / hearing / damage / custom). */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag StimulusTag;

	/** EN: Agent that observed the stimulus. */
	UPROPERTY(BlueprintReadOnly)
	FPGXAIAgentHandle Observer;

	/** EN: Object that produced the stimulus (other agent, projectile, sound source, etc.). */
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<UObject> Source;

	/** EN: Last-known location of the stimulus source, or ZeroVector when N/A. */
	UPROPERTY(BlueprintReadOnly)
	FVector LastKnownLocation = FVector::ZeroVector;

	/** EN: Confidence value in [0,1] — 0 = ignore, 1 = certain. */
	UPROPERTY(BlueprintReadOnly)
	float Confidence = 0.f;

	/** EN: Wall-time timestamp captured by the producer. */
	UPROPERTY(BlueprintReadOnly)
	double Timestamp = 0.0;
};


/**
 * EN: Last observed Behavior Tree run status for a registered agent. This is a lightweight
 *     source-visible seam for runtime diagnostics and Path B tests; it is not a replacement
 *     for UE Behavior Tree execution telemetry.
 * ES: Ultimo estado observado de ejecucion Behavior Tree para un agente registrado.
 */
USTRUCT(BlueprintType)
struct PGXAIRUNTIME_API FPGXAIBehaviorTreeRunStatus
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FPGXAIAgentHandle AgentHandle;

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(BlueprintReadOnly)
	bool bRunAttempted = false;

	UPROPERTY(BlueprintReadOnly)
	bool bRunSucceeded = false;

	UPROPERTY(BlueprintReadOnly)
	EPGXAIResultCode LastResultCode = EPGXAIResultCode::Success;

	UPROPERTY(BlueprintReadOnly)
	FString DiagnosticMessage;

	UPROPERTY(BlueprintReadOnly)
	double Timestamp = 0.0;
};
