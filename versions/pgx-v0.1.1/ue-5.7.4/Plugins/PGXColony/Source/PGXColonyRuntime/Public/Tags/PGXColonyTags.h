// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

// EN: Native gameplay tags for the PGX Colony system. Development Preview declares the canonical
//     branches that PGXColony owns. Definitions in Private/Tags/PGXColonyTags.cpp via
//     UE_DEFINE_GAMEPLAY_TAG. Branches:
//
//       PGX.Colony                       — root branch
//       PGX.Colony.Role                  — role categorization
//       PGX.Colony.Role.Worker
//       PGX.Colony.Role.Scout
//       PGX.Colony.Role.Leader
//       PGX.Colony.Task                  — task categorization
//       PGX.Colony.Task.Idle
//       PGX.Colony.Task.Gather
//       PGX.Colony.Task.Build
//       PGX.Colony.Need                  — need categorization
//       PGX.Colony.Need.Hunger
//       PGX.Colony.Need.Rest
//       PGX.Colony.Need.Social
//       PGX.Colony.Event                 — event categorization
//       PGX.Colony.Event.Recruitment
//       PGX.Colony.Event.Conflict
//
//     Authored definitions (UPGXSurvivorRoleDefinition / UPGXColonyTaskDefinition /
//     UPGXColonyNeedDefinition / UPGXColonyEventDefinition Object DAs) consume these tags
//     outside the current product boundary.
// ES: Tags nativos de gameplay para el sistema PGXColony. Development Preview declara las ramas
//     canonicas que PGXColony posee.

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Colony);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Colony_Role);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Colony_Role_Worker);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Colony_Role_Scout);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Colony_Role_Leader);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Colony_Task);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Colony_Task_Idle);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Colony_Task_Gather);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Colony_Task_Build);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Colony_Need);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Colony_Need_Hunger);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Colony_Need_Rest);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Colony_Need_Social);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Colony_Event);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Colony_Event_Recruitment);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Colony_Event_Conflict);
