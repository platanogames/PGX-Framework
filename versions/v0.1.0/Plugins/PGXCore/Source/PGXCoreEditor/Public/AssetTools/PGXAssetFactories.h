// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "PGXAssetFactories.generated.h"

// ============================================================================
// Core DataAsset Factories
// ============================================================================

/**
 * EN: Factory for creating PGX Config DataAssets from the Content Browser.
 * ES: Factory para crear PGX Config DataAssets desde el Content Browser.
 */
UCLASS()
class PGXCOREEDITOR_API UPGXConfigDataAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXConfigDataAssetFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

/**
 * EN: Factory for creating PGX Object DataAssets from the Content Browser.
 * ES: Factory para crear PGX Object DataAssets desde el Content Browser.
 */
UCLASS()
class PGXCOREEDITOR_API UPGXObjectDataAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXObjectDataAssetFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

// ============================================================================
// Per-Plugin DataAsset Factories
// ============================================================================

/** EN: Factory for PGX Flow Rules Config DataAssets / ES: Factory para Flow Rules Config DataAssets PGX */
UCLASS()
class PGXCOREEDITOR_API UPGXFlowRulesConfigFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXFlowRulesConfigFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

/** EN: Factory for PGX PSO Catalog DataAssets / ES: Factory para PSO Catalog DataAssets PGX */
UCLASS()
class PGXCOREEDITOR_API UPGXPSOCatalogFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXPSOCatalogFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

/** EN: Factory for PGX Save Config DataAssets / ES: Factory para Save Config DataAssets PGX */
UCLASS()
class PGXCOREEDITOR_API UPGXSaveConfigFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXSaveConfigFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

/** EN: Factory for PGX Level Flow Config DataAssets / ES: Factory para Level Flow Config DataAssets PGX */
UCLASS()
class PGXCOREEDITOR_API UPGXLevelFlowConfigFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXLevelFlowConfigFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

/** EN: Factory for PGX Level Profile DataAssets / ES: Factory para Level Profile DataAssets PGX */
UCLASS()
class PGXCOREEDITOR_API UPGXLevelProfileFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXLevelProfileFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

/** EN: Factory for PGX Loading Config DataAssets / ES: Factory para Loading Config DataAssets PGX */
UCLASS()
class PGXCOREEDITOR_API UPGXLoadingConfigFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXLoadingConfigFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

/** EN: Factory for PGX Loading Profile DataAssets / ES: Factory para Loading Profile DataAssets PGX */
UCLASS()
class PGXCOREEDITOR_API UPGXLoadingProfileFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXLoadingProfileFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

/** EN: Factory for PGX Project Profile Config DataAssets / ES: Factory para Project Profile Config DataAssets PGX */
UCLASS()
class PGXCOREEDITOR_API UPGXProjectProfileConfigFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXProjectProfileConfigFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

/** EN: Factory for PGX Platform Config DataAssets / ES: Factory para Platform Config DataAssets PGX */
UCLASS()
class PGXCOREEDITOR_API UPGXPlatformConfigFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXPlatformConfigFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

/** EN: Factory for PGX GC Observer Config DataAssets / ES: Factory para GC Observer Config DataAssets PGX */
UCLASS()
class PGXCOREEDITOR_API UPGXGCObserverConfigFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXGCObserverConfigFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

/** EN: Factory for PGX Docs Config DataAssets / ES: Factory para Docs Config DataAssets PGX */
UCLASS()
class PGXCOREEDITOR_API UPGXDocsConfigFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXDocsConfigFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

// ============================================================================
// Audio DataAsset Factories
// ============================================================================

/** EN: Factory for PGX Audio Config DataAssets / ES: Factory para Audio Config DataAssets PGX */
UCLASS()
class PGXCOREEDITOR_API UPGXAudioConfigFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXAudioConfigFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

/** EN: Factory for PGX Audio Channel Config DataAssets / ES: Factory para Audio Channel Config DataAssets PGX */
UCLASS()
class PGXCOREEDITOR_API UPGXAudioChannelConfigFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXAudioChannelConfigFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

/** EN: Factory for PGX Sound Definition DataAssets / ES: Factory para Sound Definition DataAssets PGX */
UCLASS()
class PGXCOREEDITOR_API UPGXSoundDefinitionFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXSoundDefinitionFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

/** EN: Factory for PGX Audio Profile DataAssets / ES: Factory para Audio Profile DataAssets PGX */
UCLASS()
class PGXCOREEDITOR_API UPGXAudioProfileFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXAudioProfileFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

/** EN: Factory for PGX Music Playlist DataAssets / ES: Factory para Music Playlist DataAssets PGX */
UCLASS()
class PGXCOREEDITOR_API UPGXMusicPlaylistFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXMusicPlaylistFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

/** EN: Factory for PGX Audio Ducking Config DataAssets / ES: Factory para Audio Ducking Config DataAssets PGX */
UCLASS()
class PGXCOREEDITOR_API UPGXAudioDuckingConfigFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXAudioDuckingConfigFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

/** EN: Factory for PGX Level Audio Config DataAssets / ES: Factory para Level Audio Config DataAssets PGX */
UCLASS()
class PGXCOREEDITOR_API UPGXLevelAudioConfigFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXLevelAudioConfigFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

// ============================================================================
// Data Registry Factories
// ============================================================================

/** EN: Factory for PGX DataTable Asset (UDataTable subclass) / ES: Factory para PGX DataTable Asset (subclase de UDataTable) */
UCLASS()
class PGXCOREEDITOR_API UPGXDataTableAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXDataTableAssetFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

/** EN: Factory for PGX Registry Definition DataAssets / ES: Factory para Registry Definition DataAssets PGX */
UCLASS()
class PGXCOREEDITOR_API UPGXRegistryDefinitionFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXRegistryDefinitionFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

// ============================================================================
// Message System Factories
// ============================================================================

/** EN: Factory for PGX Message Config DataAssets / ES: Factory para Message Config DataAssets PGX */
UCLASS()
class PGXCOREEDITOR_API UPGXMessageConfigFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXMessageConfigFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

// ============================================================================
// Event Handler Factories
// ============================================================================

/** EN: Factory for PGX EventHandler Config DataAssets / ES: Factory para EventHandler Config DataAssets PGX */
UCLASS()
class PGXCOREEDITOR_API UPGXEventHandlerConfigFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXEventHandlerConfigFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

// ============================================================================
// Construction DataAsset Factories
// ============================================================================

/** EN: Factory for GameMode Construction DataAssets / ES: Factory para GameMode Construction DataAssets */
UCLASS()
class PGXCOREEDITOR_API UPGXGameModeConstructionFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXGameModeConstructionFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

/** EN: Factory for PlayerController Construction DataAssets / ES: Factory para PlayerController Construction DataAssets */
UCLASS()
class PGXCOREEDITOR_API UPGXPlayerControllerConstructionFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXPlayerControllerConstructionFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

/** EN: Factory for GameState Construction DataAssets / ES: Factory para GameState Construction DataAssets */
UCLASS()
class PGXCOREEDITOR_API UPGXGameStateConstructionFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXGameStateConstructionFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

/** EN: Factory for PlayerState Construction DataAssets / ES: Factory para PlayerState Construction DataAssets */
UCLASS()
class PGXCOREEDITOR_API UPGXPlayerStateConstructionFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXPlayerStateConstructionFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

/** EN: Factory for Character Construction DataAssets / ES: Factory para Character Construction DataAssets */
UCLASS()
class PGXCOREEDITOR_API UPGXCharacterConstructionFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXCharacterConstructionFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

/** EN: Factory for Pawn Construction DataAssets / ES: Factory para Pawn Construction DataAssets */
UCLASS()
class PGXCOREEDITOR_API UPGXPawnConstructionFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXPawnConstructionFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

/** EN: Factory for HUD Construction DataAssets / ES: Factory para HUD Construction DataAssets */
UCLASS()
class PGXCOREEDITOR_API UPGXHUDConstructionFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXHUDConstructionFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

/** EN: Factory for Actor Construction DataAssets / ES: Factory para Actor Construction DataAssets */
UCLASS()
class PGXCOREEDITOR_API UPGXActorConstructionFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXActorConstructionFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};

// ============================================================================
// Log Domain DataAsset Factory
// ============================================================================

/** EN: Factory for Log Domain Config DataAssets / ES: Factory para Log Domain Config DataAssets */
UCLASS()
class PGXCOREEDITOR_API UPGXLogDomainConfigFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPGXLogDomainConfigFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool ShouldShowInNewMenu() const override { return false; }
};
