// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "AssetTools/PGXBlueprintFactoryBase.h"
#include "PGXBlueprintFactories.generated.h"

/**
 * EN: 17 Blueprint factories for PGX base classes. Exposed via the Content Browser
 *     context menu "PGX FRAMEWORK" extension section (PGXContentBrowserExtension).
 *     NOT via the top "Add New" menu. Each factory auto-hides via ShouldShowInNewMenu()
 *     if its L2 module is not loaded.
 *
 * ES: 17 factories de Blueprint para clases base PGX. Expuestas via la seccion
 *     "PGX FRAMEWORK" del menu contextual del Content Browser (PGXContentBrowserExtension).
 *     NO via el menu "Add New" superior. Cada factory se auto-oculta via ShouldShowInNewMenu()
 *     si su modulo L2 no esta cargado.
 */

// ═══════════════════════════════════════════
// Core (6)
// ═══════════════════════════════════════════

UCLASS()
class UPGXActorBlueprintFactory : public UPGXBlueprintFactoryBase
{
	GENERATED_BODY()
public:
	UPGXActorBlueprintFactory();
};

UCLASS()
class UPGXCharacterBlueprintFactory : public UPGXBlueprintFactoryBase
{
	GENERATED_BODY()
public:
	UPGXCharacterBlueprintFactory();
};

UCLASS()
class UPGXGameModeBlueprintFactory : public UPGXBlueprintFactoryBase
{
	GENERATED_BODY()
public:
	UPGXGameModeBlueprintFactory();
};

UCLASS()
class UPGXPlayerControllerBlueprintFactory : public UPGXBlueprintFactoryBase
{
	GENERATED_BODY()
public:
	UPGXPlayerControllerBlueprintFactory();
};

UCLASS()
class UPGXGameStateBlueprintFactory : public UPGXBlueprintFactoryBase
{
	GENERATED_BODY()
public:
	UPGXGameStateBlueprintFactory();
};

UCLASS()
class UPGXComponentBlueprintFactory : public UPGXBlueprintFactoryBase
{
	GENERATED_BODY()
public:
	UPGXComponentBlueprintFactory();
};

// ═══════════════════════════════════════════
// UI (1)
// ═══════════════════════════════════════════

UCLASS()
class UPGXHUDBlueprintFactory : public UPGXBlueprintFactoryBase
{
	GENERATED_BODY()
public:
	UPGXHUDBlueprintFactory();
};

// ═══════════════════════════════════════════
// Camera (1)
// ═══════════════════════════════════════════

UCLASS()
class UPGXCameraModifierBlueprintFactory : public UPGXBlueprintFactoryBase
{
	GENERATED_BODY()
public:
	UPGXCameraModifierBlueprintFactory();
};

// ═══════════════════════════════════════════
// AI (3)
// ═══════════════════════════════════════════

UCLASS()
class UPGXAIControllerBlueprintFactory : public UPGXBlueprintFactoryBase
{
	GENERATED_BODY()
public:
	UPGXAIControllerBlueprintFactory();
};

UCLASS()
class UPGXBTTaskBlueprintFactory : public UPGXBlueprintFactoryBase
{
	GENERATED_BODY()
public:
	UPGXBTTaskBlueprintFactory();
};

UCLASS()
class UPGXBTDecoratorBlueprintFactory : public UPGXBlueprintFactoryBase
{
	GENERATED_BODY()
public:
	UPGXBTDecoratorBlueprintFactory();
};

// ═══════════════════════════════════════════
// Animation (3)
// ═══════════════════════════════════════════

UCLASS()
class UPGXAnimInstanceBlueprintFactory : public UPGXBlueprintFactoryBase
{
	GENERATED_BODY()
public:
	UPGXAnimInstanceBlueprintFactory();
};

UCLASS()
class UPGXAnimNotifyBlueprintFactory : public UPGXBlueprintFactoryBase
{
	GENERATED_BODY()
public:
	UPGXAnimNotifyBlueprintFactory();
};

UCLASS()
class UPGXAnimNotifyStateBlueprintFactory : public UPGXBlueprintFactoryBase
{
	GENERATED_BODY()
public:
	UPGXAnimNotifyStateBlueprintFactory();
};

// ═══════════════════════════════════════════
// Multiplayer (2)
// ═══════════════════════════════════════════

UCLASS()
class UPGXReplicatedActorBlueprintFactory : public UPGXBlueprintFactoryBase
{
	GENERATED_BODY()
public:
	UPGXReplicatedActorBlueprintFactory();
};

UCLASS()
class UPGXReplicatedCharacterBlueprintFactory : public UPGXBlueprintFactoryBase
{
	GENERATED_BODY()
public:
	UPGXReplicatedCharacterBlueprintFactory();
};

// ═══════════════════════════════════════════
// Loading (1)
// ═══════════════════════════════════════════

UCLASS()
class UPGXLoadingStrategyBlueprintFactory : public UPGXBlueprintFactoryBase
{
	GENERATED_BODY()
public:
	UPGXLoadingStrategyBlueprintFactory();
};
