// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "AssetTools/PGXAssetFactories.h"


#include "Engine/DataAsset.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	UClass* ResolveExtensionDataAssetClass(const TCHAR* ClassPath)
	{
		UClass* ResolvedClass = LoadObject<UClass>(nullptr, ClassPath);
		return ResolvedClass && ResolvedClass->IsChildOf(UDataAsset::StaticClass())
			? ResolvedClass
			: nullptr;
	}

	UObject* CreateExtensionDataAsset(
		const TCHAR* ClassPath,
		UClass* InClass,
		UObject* InParent,
		FName InName,
		EObjectFlags Flags)
	{
		UClass* ResolvedClass = ResolveExtensionDataAssetClass(ClassPath);
		UClass* ClassToCreate = InClass ? InClass : ResolvedClass;
		if (!ResolvedClass || !ClassToCreate || !InParent
			|| !ClassToCreate->IsChildOf(ResolvedClass)
			|| !ClassToCreate->IsChildOf(UDataAsset::StaticClass()))
		{
			return nullptr;
		}

		return NewObject<UObject>(InParent, ClassToCreate, InName, Flags);
	}
}

// Core DataAsset types
#include "Data/PGXConfigDataAsset.h"
#include "Data/PGXObjectDataAsset.h"

// Per-plugin DataAsset types
#include "Profile/PGXProjectProfileConfig.h"
#include "Profile/PGXPlatformConfig.h"

// Audio DataAsset types

// Data Registry types
#include "Tables/PGXDataTableAsset.h"
#include "Registry/PGXRegistryDefinition.h"

// Message System types
#include "Messages/PGXMessageConfig.h"

// Event Handler types
#include "EventHandler/PGXEventHandlerConfig.h"

// Log Domain types
#include "Data/PGXLogDomainConfig.h"

// Construction DataAsset types
#include "Construction/PGXGameModeConstruction.h"
#include "Construction/PGXPlayerControllerConstruction.h"
#include "Construction/PGXGameStateConstruction.h"
#include "Construction/PGXPlayerStateConstruction.h"
#include "Construction/PGXCharacterConstruction.h"
#include "Construction/PGXPawnConstruction.h"
#include "Construction/PGXHUDConstruction.h"
#include "Construction/PGXActorConstruction.h"

// ============================================================================
// Core DataAsset Factories
// ============================================================================

UPGXConfigDataAssetFactory::UPGXConfigDataAssetFactory()
{
	SupportedClass = UPGXConfigDataAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UPGXConfigDataAssetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return NewObject<UPGXConfigDataAsset>(InParent, InClass, InName, Flags);
}

UPGXObjectDataAssetFactory::UPGXObjectDataAssetFactory()
{
	SupportedClass = UPGXObjectDataAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UPGXObjectDataAssetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return NewObject<UPGXObjectDataAsset>(InParent, InClass, InName, Flags);
}

// ============================================================================
// Per-Plugin DataAsset Factories
// ============================================================================

// --- Flow Rules Config ---

UPGXFlowRulesConfigFactory::UPGXFlowRulesConfigFactory()
{
	SupportedClass = ResolveExtensionDataAssetClass(TEXT("/Script/PGXGameFlowRuntime.PGXFlowRulesConfig"));
	bCreateNew = SupportedClass != nullptr;
	bEditAfterNew = bCreateNew;
}

UObject* UPGXFlowRulesConfigFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return CreateExtensionDataAsset(TEXT("/Script/PGXGameFlowRuntime.PGXFlowRulesConfig"), InClass, InParent, InName, Flags);
}

// --- PSO WarmUp Config ---

UPGXPSOCatalogFactory::UPGXPSOCatalogFactory()
{
	SupportedClass = ResolveExtensionDataAssetClass(TEXT("/Script/PGXPSORuntime.PGXPSOWarmUpConfig"));
	bCreateNew = SupportedClass != nullptr;
	bEditAfterNew = bCreateNew;
}

UObject* UPGXPSOCatalogFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return CreateExtensionDataAsset(TEXT("/Script/PGXPSORuntime.PGXPSOWarmUpConfig"), InClass, InParent, InName, Flags);
}

// --- Save Config ---

UPGXSaveConfigFactory::UPGXSaveConfigFactory()
{
	SupportedClass = ResolveExtensionDataAssetClass(TEXT("/Script/PGXSaveRuntime.PGXSaveConfig"));
	bCreateNew = SupportedClass != nullptr;
	bEditAfterNew = bCreateNew;
}

UObject* UPGXSaveConfigFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return CreateExtensionDataAsset(TEXT("/Script/PGXSaveRuntime.PGXSaveConfig"), InClass, InParent, InName, Flags);
}

// --- Level Flow Config ---

UPGXLevelFlowConfigFactory::UPGXLevelFlowConfigFactory()
{
	SupportedClass = ResolveExtensionDataAssetClass(TEXT("/Script/PGXLoadingRuntime.PGXLevelFlowConfig"));
	bCreateNew = SupportedClass != nullptr;
	bEditAfterNew = bCreateNew;
}

UObject* UPGXLevelFlowConfigFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return CreateExtensionDataAsset(TEXT("/Script/PGXLoadingRuntime.PGXLevelFlowConfig"), InClass, InParent, InName, Flags);
}

// --- Level Profile ---

UPGXLevelProfileFactory::UPGXLevelProfileFactory()
{
	SupportedClass = ResolveExtensionDataAssetClass(TEXT("/Script/PGXLoadingRuntime.PGXLevelProfile"));
	bCreateNew = SupportedClass != nullptr;
	bEditAfterNew = bCreateNew;
}

UObject* UPGXLevelProfileFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return CreateExtensionDataAsset(TEXT("/Script/PGXLoadingRuntime.PGXLevelProfile"), InClass, InParent, InName, Flags);
}

// --- Loading Config ---

UPGXLoadingConfigFactory::UPGXLoadingConfigFactory()
{
	SupportedClass = ResolveExtensionDataAssetClass(TEXT("/Script/PGXLoadingRuntime.PGXLoadingConfig"));
	bCreateNew = SupportedClass != nullptr;
	bEditAfterNew = bCreateNew;
}

UObject* UPGXLoadingConfigFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return CreateExtensionDataAsset(TEXT("/Script/PGXLoadingRuntime.PGXLoadingConfig"), InClass, InParent, InName, Flags);
}

// --- Loading Profile ---

UPGXLoadingProfileFactory::UPGXLoadingProfileFactory()
{
	SupportedClass = ResolveExtensionDataAssetClass(TEXT("/Script/PGXLoadingRuntime.PGXLoadingProfile"));
	bCreateNew = SupportedClass != nullptr;
	bEditAfterNew = bCreateNew;
}

UObject* UPGXLoadingProfileFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return CreateExtensionDataAsset(TEXT("/Script/PGXLoadingRuntime.PGXLoadingProfile"), InClass, InParent, InName, Flags);
}

// --- Project Profile Config ---

UPGXProjectProfileConfigFactory::UPGXProjectProfileConfigFactory()
{
	SupportedClass = UPGXProjectProfileConfig::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UPGXProjectProfileConfigFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return NewObject<UPGXProjectProfileConfig>(InParent, InClass, InName, Flags);
}

// --- Platform Config ---

UPGXPlatformConfigFactory::UPGXPlatformConfigFactory()
{
	SupportedClass = UPGXPlatformConfig::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UPGXPlatformConfigFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return NewObject<UPGXPlatformConfig>(InParent, InClass, InName, Flags);
}

// --- GC Observer Config ---

UPGXGCObserverConfigFactory::UPGXGCObserverConfigFactory()
{
	SupportedClass = ResolveExtensionDataAssetClass(TEXT("/Script/PGXMGOSRuntime.PGXGCObserverConfig"));
	bCreateNew = SupportedClass != nullptr;
	bEditAfterNew = bCreateNew;
}

UObject* UPGXGCObserverConfigFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return CreateExtensionDataAsset(TEXT("/Script/PGXMGOSRuntime.PGXGCObserverConfig"), InClass, InParent, InName, Flags);
}

// --- Docs Config ---

UPGXDocsConfigFactory::UPGXDocsConfigFactory()
{
	SupportedClass = ResolveExtensionDataAssetClass(TEXT("/Script/PGXDocs.PGXDocsConfig"));
	bCreateNew = SupportedClass != nullptr;
	bEditAfterNew = bCreateNew;
}

UObject* UPGXDocsConfigFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return CreateExtensionDataAsset(TEXT("/Script/PGXDocs.PGXDocsConfig"), InClass, InParent, InName, Flags);
}

// ============================================================================
// Audio DataAsset Factories
// ============================================================================

// --- Audio Config ---

UPGXAudioConfigFactory::UPGXAudioConfigFactory()
{
	SupportedClass = ResolveExtensionDataAssetClass(TEXT("/Script/PGXAudioRuntime.PGXAudioConfig"));
	bCreateNew = SupportedClass != nullptr;
	bEditAfterNew = bCreateNew;
}

UObject* UPGXAudioConfigFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return CreateExtensionDataAsset(TEXT("/Script/PGXAudioRuntime.PGXAudioConfig"), InClass, InParent, InName, Flags);
}

// --- Audio Channel Config ---

UPGXAudioChannelConfigFactory::UPGXAudioChannelConfigFactory()
{
	SupportedClass = ResolveExtensionDataAssetClass(TEXT("/Script/PGXAudioRuntime.PGXAudioChannelConfig"));
	bCreateNew = SupportedClass != nullptr;
	bEditAfterNew = bCreateNew;
}

UObject* UPGXAudioChannelConfigFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return CreateExtensionDataAsset(TEXT("/Script/PGXAudioRuntime.PGXAudioChannelConfig"), InClass, InParent, InName, Flags);
}

// --- Sound Definition ---

UPGXSoundDefinitionFactory::UPGXSoundDefinitionFactory()
{
	SupportedClass = ResolveExtensionDataAssetClass(TEXT("/Script/PGXAudioRuntime.PGXSoundDefinition"));
	bCreateNew = SupportedClass != nullptr;
	bEditAfterNew = bCreateNew;
}

UObject* UPGXSoundDefinitionFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return CreateExtensionDataAsset(TEXT("/Script/PGXAudioRuntime.PGXSoundDefinition"), InClass, InParent, InName, Flags);
}

// --- Audio Profile ---

UPGXAudioProfileFactory::UPGXAudioProfileFactory()
{
	SupportedClass = ResolveExtensionDataAssetClass(TEXT("/Script/PGXAudioRuntime.PGXAudioProfile"));
	bCreateNew = SupportedClass != nullptr;
	bEditAfterNew = bCreateNew;
}

UObject* UPGXAudioProfileFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return CreateExtensionDataAsset(TEXT("/Script/PGXAudioRuntime.PGXAudioProfile"), InClass, InParent, InName, Flags);
}

// --- Music Playlist ---

UPGXMusicPlaylistFactory::UPGXMusicPlaylistFactory()
{
	SupportedClass = ResolveExtensionDataAssetClass(TEXT("/Script/PGXAudioRuntime.PGXMusicPlaylist"));
	bCreateNew = SupportedClass != nullptr;
	bEditAfterNew = bCreateNew;
}

UObject* UPGXMusicPlaylistFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return CreateExtensionDataAsset(TEXT("/Script/PGXAudioRuntime.PGXMusicPlaylist"), InClass, InParent, InName, Flags);
}

// --- Audio Ducking Config ---

UPGXAudioDuckingConfigFactory::UPGXAudioDuckingConfigFactory()
{
	SupportedClass = ResolveExtensionDataAssetClass(TEXT("/Script/PGXAudioRuntime.PGXAudioDuckingConfig"));
	bCreateNew = SupportedClass != nullptr;
	bEditAfterNew = bCreateNew;
}

UObject* UPGXAudioDuckingConfigFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return CreateExtensionDataAsset(TEXT("/Script/PGXAudioRuntime.PGXAudioDuckingConfig"), InClass, InParent, InName, Flags);
}

// --- Level Audio Config ---

UPGXLevelAudioConfigFactory::UPGXLevelAudioConfigFactory()
{
	SupportedClass = ResolveExtensionDataAssetClass(TEXT("/Script/PGXAudioRuntime.PGXLevelAudioConfig"));
	bCreateNew = SupportedClass != nullptr;
	bEditAfterNew = bCreateNew;
}

UObject* UPGXLevelAudioConfigFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return CreateExtensionDataAsset(TEXT("/Script/PGXAudioRuntime.PGXLevelAudioConfig"), InClass, InParent, InName, Flags);
}

// ============================================================================
// Data Registry Factories
// ============================================================================

// --- PGX DataTable Asset ---

UPGXDataTableAssetFactory::UPGXDataTableAssetFactory()
{
	SupportedClass = UPGXDataTableAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UPGXDataTableAssetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return NewObject<UPGXDataTableAsset>(InParent, InClass, InName, Flags);
}

// --- Registry Definition ---

UPGXRegistryDefinitionFactory::UPGXRegistryDefinitionFactory()
{
	SupportedClass = UPGXRegistryDefinition::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UPGXRegistryDefinitionFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return NewObject<UPGXRegistryDefinition>(InParent, InClass, InName, Flags);
}

// ============================================================================
// Message System Factories
// ============================================================================

// --- Message Config ---

UPGXMessageConfigFactory::UPGXMessageConfigFactory()
{
	SupportedClass = UPGXMessageConfig::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UPGXMessageConfigFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return NewObject<UPGXMessageConfig>(InParent, InClass, InName, Flags);
}

// ============================================================================
// Event Handler Factories
// ============================================================================

// --- EventHandler Config ---

UPGXEventHandlerConfigFactory::UPGXEventHandlerConfigFactory()
{
	SupportedClass = UPGXEventHandlerConfig::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UPGXEventHandlerConfigFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return NewObject<UPGXEventHandlerConfig>(InParent, InClass, InName, Flags);
}

// ============================================================================
// Construction DataAsset Factories
// ============================================================================

// --- GameMode Construction ---

UPGXGameModeConstructionFactory::UPGXGameModeConstructionFactory()
{
	SupportedClass = UPGXGameModeConstruction::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UPGXGameModeConstructionFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return NewObject<UPGXGameModeConstruction>(InParent, InClass, InName, Flags);
}

// --- PlayerController Construction ---

UPGXPlayerControllerConstructionFactory::UPGXPlayerControllerConstructionFactory()
{
	SupportedClass = UPGXPlayerControllerConstruction::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UPGXPlayerControllerConstructionFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return NewObject<UPGXPlayerControllerConstruction>(InParent, InClass, InName, Flags);
}

// --- GameState Construction ---

UPGXGameStateConstructionFactory::UPGXGameStateConstructionFactory()
{
	SupportedClass = UPGXGameStateConstruction::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UPGXGameStateConstructionFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return NewObject<UPGXGameStateConstruction>(InParent, InClass, InName, Flags);
}

// --- PlayerState Construction ---

UPGXPlayerStateConstructionFactory::UPGXPlayerStateConstructionFactory()
{
	SupportedClass = UPGXPlayerStateConstruction::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UPGXPlayerStateConstructionFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return NewObject<UPGXPlayerStateConstruction>(InParent, InClass, InName, Flags);
}

// --- Character Construction ---

UPGXCharacterConstructionFactory::UPGXCharacterConstructionFactory()
{
	SupportedClass = UPGXCharacterConstruction::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UPGXCharacterConstructionFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return NewObject<UPGXCharacterConstruction>(InParent, InClass, InName, Flags);
}

// --- Pawn Construction ---

UPGXPawnConstructionFactory::UPGXPawnConstructionFactory()
{
	SupportedClass = UPGXPawnConstruction::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UPGXPawnConstructionFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return NewObject<UPGXPawnConstruction>(InParent, InClass, InName, Flags);
}

// --- HUD Construction ---

UPGXHUDConstructionFactory::UPGXHUDConstructionFactory()
{
	SupportedClass = UPGXHUDConstruction::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UPGXHUDConstructionFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return NewObject<UPGXHUDConstruction>(InParent, InClass, InName, Flags);
}

// --- Actor Construction ---

UPGXActorConstructionFactory::UPGXActorConstructionFactory()
{
	SupportedClass = UPGXActorConstruction::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UPGXActorConstructionFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return NewObject<UPGXActorConstruction>(InParent, InClass, InName, Flags);
}

// ============================================================================
// Log Domain DataAsset Factory
// ============================================================================

UPGXLogDomainConfigFactory::UPGXLogDomainConfigFactory()
{
	SupportedClass = UPGXLogDomainConfig::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UPGXLogDomainConfigFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* /*Context*/, FFeedbackContext* /*Warn*/)
{
	return NewObject<UPGXLogDomainConfig>(InParent, InClass, InName, Flags);
}
