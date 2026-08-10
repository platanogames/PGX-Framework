// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "EventHandler/PGXEventHandlerBase.h"
#include "PGXNotSavingCondition.generated.h"

/**
 * EN: Condition handler that succeeds only when the game is NOT saving.
 *     Used as a guard in ExecuteSequence to prevent actions during save operations.
 *     Lifecycle: Singleton. Category: Core.
 *     CONTRACT: Caller must add PGX.State.Saving to Context.SourceTags when saving
 *     is in progress. If the tag is absent, execution is ALLOWED (pass-through).
 * ES: Handler de condicion que tiene exito solo cuando el juego NO esta guardando.
 *     Usado como guard en ExecuteSequence para prevenir acciones durante operaciones de guardado.
 *     Ciclo de vida: Singleton. Categoria: Core.
 *     CONTRATO: El llamador debe agregar PGX.State.Saving a Context.SourceTags cuando
 *     saving esta en progreso. Si el tag no esta, la ejecucion es PERMITIDA (pass-through).
 */
UCLASS(BlueprintType)
class PGXCORERUNTIME_API UPGXNotSavingCondition : public UPGXEventHandlerBase
{
	GENERATED_BODY()

public:
	bool CanExecute_Implementation(const FPGXEventContext& Context) const override;
	EPGXEventResult Execute_Implementation(const FPGXEventContext& Context, const FInstancedStruct& Payload) override;
};
