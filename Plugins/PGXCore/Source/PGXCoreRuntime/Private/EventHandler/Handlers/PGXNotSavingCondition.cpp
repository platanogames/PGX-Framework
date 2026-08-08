// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "EventHandler/Handlers/PGXNotSavingCondition.h"
#include "EventHandler/PGXEventHandlerLog.h"

bool UPGXNotSavingCondition::CanExecute_Implementation(const FPGXEventContext& Context) const
{
	// EN: Check if the "PGX.State.Saving" tag is present in SourceTags.
	//     The Save L2 subsystem is responsible for adding this tag to contexts
	//     during save operations. If not present, we assume not saving.
	// ES: Verificar si el tag "PGX.State.Saving" esta presente en SourceTags.
	//     El subsistema L2 Save se encarga de agregar este tag a los contextos
	//     durante operaciones de guardado. Si no esta, asumimos que no guarda.
	const FGameplayTag SavingStateTag = FGameplayTag::RequestGameplayTag(FName("PGX.State.Saving"), false);
	if (!SavingStateTag.IsValid())
	{
		return true;
	}

	return !Context.SourceTags.HasTag(SavingStateTag);
}

EPGXEventResult UPGXNotSavingCondition::Execute_Implementation(const FPGXEventContext& Context, const FInstancedStruct& /*Payload*/)
{
	if (CanExecute(Context))
	{
		return EPGXEventResult::Success;
	}

	UE_LOG(LogPGXEventHandler, Log, TEXT("NotSavingCondition: Blocked — saving state tag present in context"));
	return EPGXEventResult::Skipped;
}
