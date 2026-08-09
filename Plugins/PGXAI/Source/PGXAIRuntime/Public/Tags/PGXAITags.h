// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

// EN: Native gameplay tags for the PGX AI system. Development Preview declares the canonical
//     branches that PGXAI owns. Definitions in Private/Tags/PGXAITags.cpp via UE_DEFINE_GAMEPLAY_TAG.
//     Branches:
//       PGX.AI                        — root branch
//       PGX.AI.Perception             — stimulus categorization
//       PGX.AI.Perception.Sight
//       PGX.AI.Perception.Hearing
//       PGX.AI.Perception.Damage
//       PGX.AI.Alert                  — alert/detection state surfaces
//       PGX.AI.Alert.Calm
//       PGX.AI.Alert.Investigating
//       PGX.AI.Alert.Combat
//       PGX.AI.Squad                  — squad coordination surfaces
//       PGX.AI.Squad.Member
//       PGX.AI.Squad.Leader
//       PGX.AI.Task                   — task execution surfaces
//       PGX.AI.Task.Idle
//       PGX.AI.Task.Patrol
//       PGX.AI.Task.Engage
//
// ES: Tags nativos de gameplay para el sistema PGXAI. Development Preview declara las ramas
//     canonicas que PGXAI posee.

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_AI);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_AI_Perception);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_AI_Perception_Sight);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_AI_Perception_Hearing);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_AI_Perception_Damage);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_AI_Alert);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_AI_Alert_Calm);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_AI_Alert_Investigating);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_AI_Alert_Combat);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_AI_Squad);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_AI_Squad_Member);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_AI_Squad_Leader);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_AI_Task);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_AI_Task_Idle);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_AI_Task_Patrol);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_AI_Task_Engage);
