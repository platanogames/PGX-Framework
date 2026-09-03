// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXInteractionCondition.h"

// EN: Interaction condition base implementation
// ES: Implementacion base de condicion de interaccion

bool UPGXInteractionCondition::EvaluateCondition_Implementation(AActor* Interactor) const
{
	// EN: Base implementation always returns true / ES: Implementacion base siempre retorna true
	return true;
}
