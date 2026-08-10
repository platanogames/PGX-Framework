// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "EventHandler/Handlers/PGXNotLoadingCondition.h"
#include "EventHandler/PGXEventHandlerLog.h"

bool UPGXNotLoadingCondition::CanExecute_Implementation(const FPGXEventContext& Context) const
{
	// EN: Check if the "PGX.State.Loading" tag is present in SourceTags.
	//     The Loading L2 subsystem is responsible for adding this tag to contexts
	//     during level transitions. If not present, we assume not loading.
	// ES: Verificar si el tag "PGX.State.Loading" esta presente en SourceTags.
	//     El subsistema L2 Loading se encarga de agregar este tag a los contextos
	//     durante transiciones de nivel. Si no esta, asumimos que no carga.
	const FGameplayTag LoadingStateTag = FGameplayTag::RequestGameplayTag(FName("PGX.State.Loading"), false);
	if (!LoadingStateTag.IsValid())
	{
		return true;
	}

	return !Context.SourceTags.HasTag(LoadingStateTag);
}

EPGXEventResult UPGXNotLoadingCondition::Execute_Implementation(const FPGXEventContext& Context, const FInstancedStruct& /*Payload*/)
{
	// EN: Condition handlers validate — execution is a pass-through
	// ES: Handlers de condicion validan — la ejecucion es pass-through
	if (CanExecute(Context))
	{
		return EPGXEventResult::Success;
	}

	UE_LOG(LogPGXEventHandler, Log, TEXT("NotLoadingCondition: Blocked — loading state tag present in context"));
	return EPGXEventResult::Skipped;
}
