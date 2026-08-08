// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXPSOBlueprintLibrary.h"
#include "PGXPSOSubsystem.h"
#include "ShaderPipelineCache.h"
#include "Engine/GameInstance.h"

// ============================================================================
// EN: Helper
// ES: Helper
// ============================================================================

UPGXPSOSubsystem* UPGXPSOBlueprintLibrary::GetSubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return nullptr;
	}

	UGameInstance* GI = World->GetGameInstance();
	return GI ? GI->GetSubsystem<UPGXPSOSubsystem>() : nullptr;
}

// ============================================================================
// EN: Warm-Up Control
// ES: Control de Warm-Up
// ============================================================================

bool UPGXPSOBlueprintLibrary::RequestPSOWarmUp(const UObject* WorldContextObject)
{
	UPGXPSOSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->RequestWarmUpAll() : false;
}

bool UPGXPSOBlueprintLibrary::RequestPSOWarmUpForContext(const UObject* WorldContextObject, FGameplayTag ContextTag)
{
	UPGXPSOSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->RequestWarmUp(ContextTag) : false;
}

void UPGXPSOBlueprintLibrary::PausePSOWarmUp(const UObject* WorldContextObject)
{
	UPGXPSOSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (Sub)
	{
		Sub->PauseWarmUp();
	}
}

void UPGXPSOBlueprintLibrary::ResumePSOWarmUp(const UObject* WorldContextObject)
{
	UPGXPSOSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (Sub)
	{
		Sub->ResumeWarmUp();
	}
}

void UPGXPSOBlueprintLibrary::CancelPSOWarmUp(const UObject* WorldContextObject)
{
	UPGXPSOSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (Sub)
	{
		Sub->CancelWarmUp();
	}
}

EPGXPSOWarmUpState UPGXPSOBlueprintLibrary::GetPSOWarmUpState(const UObject* WorldContextObject)
{
	UPGXPSOSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->GetWarmUpState() : EPGXPSOWarmUpState::Idle;
}

FPGXPSOWarmUpProgress UPGXPSOBlueprintLibrary::GetPSOProgress(const UObject* WorldContextObject)
{
	UPGXPSOSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->GetWarmUpProgress() : FPGXPSOWarmUpProgress();
}

float UPGXPSOBlueprintLibrary::GetPSOProgressPercent(const UObject* WorldContextObject)
{
	UPGXPSOSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		return 0.0f;
	}
	return Sub->GetWarmUpProgress().PercentComplete * 100.0f;
}

bool UPGXPSOBlueprintLibrary::IsPSOWarmUpComplete(const UObject* WorldContextObject)
{
	UPGXPSOSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? (Sub->GetWarmUpState() == EPGXPSOWarmUpState::Complete) : false;
}

bool UPGXPSOBlueprintLibrary::IsPSOCacheDirty(const UObject* WorldContextObject)
{
	UPGXPSOSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->IsCacheDirty() : false;
}

// ============================================================================
// EN: Context Management
// ES: Gestion de Contexto
// ============================================================================

void UPGXPSOBlueprintLibrary::AddPSOContext(const UObject* WorldContextObject, FGameplayTag ContextTag)
{
	UPGXPSOSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (Sub)
	{
		Sub->AddPSOContext(ContextTag);
	}
}

void UPGXPSOBlueprintLibrary::RemovePSOContext(const UObject* WorldContextObject, FGameplayTag ContextTag)
{
	UPGXPSOSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (Sub)
	{
		Sub->RemovePSOContext(ContextTag);
	}
}

TArray<FGameplayTag> UPGXPSOBlueprintLibrary::GetActivePSOContexts(const UObject* WorldContextObject)
{
	UPGXPSOSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->GetActiveContexts() : TArray<FGameplayTag>();
}

// ============================================================================
// EN: Cache Operations
// ES: Operaciones de Cache
// ============================================================================

void UPGXPSOBlueprintLibrary::SavePSOCacheToDisk(const UObject* WorldContextObject)
{
	UPGXPSOSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (Sub)
	{
		Sub->SaveCacheToDisk();
	}
}

int32 UPGXPSOBlueprintLibrary::GetDiscoveredPSOConfigCount(const UObject* WorldContextObject)
{
	UPGXPSOSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->GetDiscoveredConfigCount() : 0;
}

int32 UPGXPSOBlueprintLibrary::GetPendingPSOCompilations()
{
	return static_cast<int32>(FShaderPipelineCache::NumPrecompilesRemaining());
}

// ============================================================================
// EN: Native Shader Cache
// ES: Cache de Shaders Nativo
// ============================================================================

void UPGXPSOBlueprintLibrary::PauseShaderCacheBatching()
{
	FShaderPipelineCache::PauseBatching();
}

void UPGXPSOBlueprintLibrary::ResumeShaderCacheBatching()
{
	FShaderPipelineCache::ResumeBatching();
}
