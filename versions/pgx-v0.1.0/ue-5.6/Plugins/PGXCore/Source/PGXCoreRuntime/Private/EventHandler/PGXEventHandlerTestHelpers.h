// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "EventHandler/PGXEventHandlerBase.h"
#include "PGXEventHandlerTestHelpers.generated.h"

/**
 * EN: Internal test handler that always succeeds. Not for external use.
 * ES: Handler de test interno que siempre tiene exito. No para uso externo.
 */
UCLASS(Hidden, NotBlueprintable)
class UPGXTestSuccessHandler : public UPGXEventHandlerBase
{
	GENERATED_BODY()
public:
	EPGXEventResult Execute_Implementation(const FPGXEventContext& /*Context*/, const FInstancedStruct& /*Payload*/) override
	{
		return EPGXEventResult::Success;
	}
};

/**
 * EN: Internal test handler that always fails. Not for external use.
 * ES: Handler de test interno que siempre falla. No para uso externo.
 */
UCLASS(Hidden, NotBlueprintable)
class UPGXTestFailHandler : public UPGXEventHandlerBase
{
	GENERATED_BODY()
public:
	EPGXEventResult Execute_Implementation(const FPGXEventContext& /*Context*/, const FInstancedStruct& /*Payload*/) override
	{
		return EPGXEventResult::Failed;
	}
};

/**
 * EN: Internal test handler with conditional execution. Not for external use.
 *     Only executes if "PGX.Test.Allow" tag is present in context SourceTags.
 * ES: Handler de test interno con ejecucion condicional. No para uso externo.
 *     Solo ejecuta si el tag "PGX.Test.Allow" esta presente en SourceTags del contexto.
 */
UCLASS(Hidden, NotBlueprintable)
class UPGXTestConditionHandler : public UPGXEventHandlerBase
{
	GENERATED_BODY()
public:
	bool CanExecute_Implementation(const FPGXEventContext& Context) const override
	{
		const FGameplayTag AllowTag = FGameplayTag::RequestGameplayTag(FName("PGX.Test.Allow"), false);
		return AllowTag.IsValid() && Context.SourceTags.HasTag(AllowTag);
	}

	EPGXEventResult Execute_Implementation(const FPGXEventContext& /*Context*/, const FInstancedStruct& /*Payload*/) override
	{
		return EPGXEventResult::Success;
	}
};
