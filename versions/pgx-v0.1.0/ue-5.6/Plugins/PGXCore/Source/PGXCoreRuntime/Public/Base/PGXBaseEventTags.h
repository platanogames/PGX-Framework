// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"

/**
 * EN: Base class lifecycle event tags for EventHandler integration.
 *     Used by GameMode helpers to fire standard match/player events.
 * ES: Tags de eventos de ciclo de vida para integracion con EventHandler.
 *     Usados por helpers del GameMode para disparar eventos estandar.
 */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Event_Match_Started);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Event_Match_Ended);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Event_Player_Joined);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Event_Player_Left);
