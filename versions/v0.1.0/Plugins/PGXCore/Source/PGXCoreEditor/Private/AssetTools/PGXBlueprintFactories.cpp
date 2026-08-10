// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "AssetTools/PGXBlueprintFactories.h"

#define LOCTEXT_NAMESPACE "PGXBlueprintFactories"

// ═══════════════════════════════════════════
// Core (6)
// ═══════════════════════════════════════════

UPGXActorBlueprintFactory::UPGXActorBlueprintFactory()
{
	FactoryDisplayName = LOCTEXT("PGXActor", "PGX Actor");
	ParentClassPath = TEXT("/Script/PGXCoreRuntime.PGXActorBase");
	SubMenus = { LOCTEXT("Fw", "PGX Framework"), LOCTEXT("Core", "Core"), LOCTEXT("BPG", "Blueprints") };
}

UPGXCharacterBlueprintFactory::UPGXCharacterBlueprintFactory()
{
	FactoryDisplayName = LOCTEXT("PGXCharacter", "PGX Character");
	ParentClassPath = TEXT("/Script/PGXCoreRuntime.PGXCharacterBase");
	SubMenus = { LOCTEXT("Fw2", "PGX Framework"), LOCTEXT("Core2", "Core"), LOCTEXT("BPG2", "Blueprints") };
}

UPGXGameModeBlueprintFactory::UPGXGameModeBlueprintFactory()
{
	FactoryDisplayName = LOCTEXT("PGXGameMode", "PGX GameMode");
	ParentClassPath = TEXT("/Script/PGXCoreRuntime.PGXGameModeBase");
	SubMenus = { LOCTEXT("Fw3", "PGX Framework"), LOCTEXT("Core3", "Core"), LOCTEXT("BPG3", "Blueprints") };
}

UPGXPlayerControllerBlueprintFactory::UPGXPlayerControllerBlueprintFactory()
{
	FactoryDisplayName = LOCTEXT("PGXPlayerController", "PGX PlayerController");
	ParentClassPath = TEXT("/Script/PGXCoreRuntime.PGXPlayerControllerBase");
	SubMenus = { LOCTEXT("Fw4", "PGX Framework"), LOCTEXT("Core4", "Core"), LOCTEXT("BPG4", "Blueprints") };
}

UPGXGameStateBlueprintFactory::UPGXGameStateBlueprintFactory()
{
	FactoryDisplayName = LOCTEXT("PGXGameState", "PGX GameState");
	ParentClassPath = TEXT("/Script/PGXCoreRuntime.PGXGameStateBase");
	SubMenus = { LOCTEXT("Fw5", "PGX Framework"), LOCTEXT("Core5", "Core"), LOCTEXT("BPG5", "Blueprints") };
}

UPGXComponentBlueprintFactory::UPGXComponentBlueprintFactory()
{
	FactoryDisplayName = LOCTEXT("PGXComponent", "PGX Component");
	ParentClassPath = TEXT("/Script/PGXCoreRuntime.PGXBaseComponent");
	SubMenus = { LOCTEXT("Fw6", "PGX Framework"), LOCTEXT("Core6", "Core"), LOCTEXT("BPG6", "Blueprints") };
}

// ═══════════════════════════════════════════
// UI (1)
// ═══════════════════════════════════════════

UPGXHUDBlueprintFactory::UPGXHUDBlueprintFactory()
{
	FactoryDisplayName = LOCTEXT("PGXHUD", "PGX HUD");
	ParentClassPath = TEXT("/Script/PGXUIRuntime.PGXHUDBase");
	SubMenus = { LOCTEXT("Fw7", "PGX Framework"), LOCTEXT("UI", "UI"), LOCTEXT("BPG7", "Blueprints") };
}

// ═══════════════════════════════════════════
// Camera (1)
// ═══════════════════════════════════════════

UPGXCameraModifierBlueprintFactory::UPGXCameraModifierBlueprintFactory()
{
	FactoryDisplayName = LOCTEXT("PGXCameraModifier", "PGX Camera Modifier");
	ParentClassPath = TEXT("/Script/PGXCameraRuntime.PGXCameraModifier_Base");
	SubMenus = { LOCTEXT("Fw8", "PGX Framework"), LOCTEXT("Camera", "Camera"), LOCTEXT("BPG8", "Blueprints") };
}

// ═══════════════════════════════════════════
// AI (3)
// ═══════════════════════════════════════════

UPGXAIControllerBlueprintFactory::UPGXAIControllerBlueprintFactory()
{
	FactoryDisplayName = LOCTEXT("PGXAIController", "PGX AI Controller");
	ParentClassPath = TEXT("/Script/PGXAIRuntime.PGXAIController_Base");
	SubMenus = { LOCTEXT("Fw9", "PGX Framework"), LOCTEXT("AI", "AI"), LOCTEXT("BPG9", "Blueprints") };
}

UPGXBTTaskBlueprintFactory::UPGXBTTaskBlueprintFactory()
{
	FactoryDisplayName = LOCTEXT("PGXBTTask", "PGX BT Task");
	ParentClassPath = TEXT("/Script/PGXAIRuntime.PGXBTTask_Base");
	SubMenus = { LOCTEXT("Fw10", "PGX Framework"), LOCTEXT("AI2", "AI"), LOCTEXT("BPG10", "Blueprints") };
}

UPGXBTDecoratorBlueprintFactory::UPGXBTDecoratorBlueprintFactory()
{
	FactoryDisplayName = LOCTEXT("PGXBTDecorator", "PGX BT Decorator");
	ParentClassPath = TEXT("/Script/PGXAIRuntime.PGXBTDecorator_Base");
	SubMenus = { LOCTEXT("Fw11", "PGX Framework"), LOCTEXT("AI3", "AI"), LOCTEXT("BPG11", "Blueprints") };
}

// ═══════════════════════════════════════════
// Animation (3)
// ═══════════════════════════════════════════

UPGXAnimInstanceBlueprintFactory::UPGXAnimInstanceBlueprintFactory()
{
	FactoryDisplayName = LOCTEXT("PGXAnimInstance", "PGX Anim Instance");
	ParentClassPath = TEXT("/Script/PGXAnimationRuntime.PGXAnimInstanceBase");
	SubMenus = { LOCTEXT("Fw12", "PGX Framework"), LOCTEXT("Animation", "Animation"), LOCTEXT("BPG12", "Blueprints") };
}

UPGXAnimNotifyBlueprintFactory::UPGXAnimNotifyBlueprintFactory()
{
	FactoryDisplayName = LOCTEXT("PGXAnimNotify", "PGX Anim Notify");
	ParentClassPath = TEXT("/Script/PGXAnimationRuntime.PGXAnimNotifyBase");
	SubMenus = { LOCTEXT("Fw13", "PGX Framework"), LOCTEXT("Animation2", "Animation"), LOCTEXT("BPG13", "Blueprints") };
}

UPGXAnimNotifyStateBlueprintFactory::UPGXAnimNotifyStateBlueprintFactory()
{
	FactoryDisplayName = LOCTEXT("PGXAnimNotifyState", "PGX Anim Notify State");
	ParentClassPath = TEXT("/Script/PGXAnimationRuntime.PGXAnimNotifyStateBase");
	SubMenus = { LOCTEXT("Fw14", "PGX Framework"), LOCTEXT("Animation3", "Animation"), LOCTEXT("BPG14", "Blueprints") };
}

// ═══════════════════════════════════════════
// Multiplayer (2)
// ═══════════════════════════════════════════

UPGXReplicatedActorBlueprintFactory::UPGXReplicatedActorBlueprintFactory()
{
	FactoryDisplayName = LOCTEXT("PGXReplicatedActor", "PGX Replicated Actor");
	ParentClassPath = TEXT("/Script/PGXMultiplayerRuntime.PGXReplicatedActorBase");
	SubMenus = { LOCTEXT("Fw15", "PGX Framework"), LOCTEXT("Multiplayer", "Multiplayer"), LOCTEXT("BPG15", "Blueprints") };
}

UPGXReplicatedCharacterBlueprintFactory::UPGXReplicatedCharacterBlueprintFactory()
{
	FactoryDisplayName = LOCTEXT("PGXReplicatedCharacter", "PGX Replicated Character");
	ParentClassPath = TEXT("/Script/PGXMultiplayerRuntime.PGXReplicatedCharacterBase");
	SubMenus = { LOCTEXT("Fw16", "PGX Framework"), LOCTEXT("Multiplayer2", "Multiplayer"), LOCTEXT("BPG16", "Blueprints") };
}

// ═══════════════════════════════════════════
// Loading (1)
// ═══════════════════════════════════════════

UPGXLoadingStrategyBlueprintFactory::UPGXLoadingStrategyBlueprintFactory()
{
	FactoryDisplayName = LOCTEXT("PGXLoadingStrategy", "PGX Loading Strategy");
	ParentClassPath = TEXT("/Script/PGXLoadingRuntime.PGXLoadingStrategyBase");
	SubMenus = { LOCTEXT("Fw17", "PGX Framework"), LOCTEXT("Loading", "Loading"), LOCTEXT("BPG17", "Blueprints") };
}

#undef LOCTEXT_NAMESPACE
