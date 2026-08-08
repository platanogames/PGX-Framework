// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXLoadingStrategy_Material.h"
#include "Logging/PGXLogMacros.h"
#include "PGXLoadingRuntime.h"
#include "PGXLoadingProfile.h"
#include "PGXLoadingWidget.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"

void UPGXLoadingStrategy_Material::InitializeStrategy(UPGXLoadingProfile* Profile)
{
	Super::InitializeStrategy(Profile);

	LoadedMaterial = nullptr;
	DynamicMaterial = nullptr;
	AnimationTime = 0.0f;
	CurrentTipIndex = 0;
	TipTimer = 0.0f;

	if (!Profile || Profile->BackgroundMaterials.Num() == 0 || Profile->BackgroundMaterials[0].IsNull())
	{
		PGX_LOG_WARNING(LogPGXLoading, TEXT("Strategy_Material: No background materials — ready without material."));
		bReady = true;
		return;
	}

	// EN: Async load the first material / ES: Carga async del primer material
	FSoftObjectPath MaterialPath = Profile->BackgroundMaterials[0].ToSoftObjectPath();

	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
	AssetLoadHandle = StreamableManager.RequestAsyncLoad(
		MaterialPath,
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnMaterialLoaded),
		FStreamableManager::AsyncLoadHighPriority
	);

	PGX_LOG_INFO(LogPGXLoading, TEXT("Strategy_Material: Async loading '%s'..."), *MaterialPath.ToString());
}

void UPGXLoadingStrategy_Material::OnMaterialLoaded()
{
	if (CachedProfile && CachedProfile->BackgroundMaterials.Num() > 0)
	{
		LoadedMaterial = CachedProfile->BackgroundMaterials[0].Get();
	}

	bReady = true;

	if (LoadedMaterial)
	{
		PGX_LOG_INFO(LogPGXLoading, TEXT("Strategy_Material: Material loaded: %s"), *LoadedMaterial->GetName());

		// EN: If already activated, create DynamicMaterial and apply / ES: Si ya activado, crear y aplicar
		if (bActivated && CachedWidget)
		{
			DynamicMaterial = UMaterialInstanceDynamic::Create(LoadedMaterial, this);
			CachedWidget->SetBackgroundMaterial(DynamicMaterial);
		}
	}
	else
	{
		PGX_LOG_WARNING(LogPGXLoading, TEXT("Strategy_Material: Material load returned null."));
	}
}

void UPGXLoadingStrategy_Material::Activate(UPGXLoadingWidget* Widget)
{
	Super::Activate(Widget);

	if (!CachedWidget) return;

	// EN: Create dynamic material instance for animation / ES: Crear instancia dinamica para animacion
	if (LoadedMaterial)
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(LoadedMaterial, this);
		CachedWidget->SetBackgroundMaterial(DynamicMaterial);
	}

	CachedWidget->SetProgressBarVisible(CachedProfile ? CachedProfile->bShowProgressBar : true);
	CachedWidget->SetSkipButtonVisible(CachedProfile ? CachedProfile->bAllowSkip : false);

	if (CachedProfile && CachedProfile->Tips.Num() > 0)
	{
		CachedWidget->SetTipText(CachedProfile->Tips[0]);
		CachedWidget->OnTipChanged(CachedProfile->Tips[0]);
	}

	AnimationTime = 0.0f;

	PGX_LOG_VERBOSE(LogPGXLoading, TEXT("Strategy_Material: Activated."));
}

void UPGXLoadingStrategy_Material::UpdateVisual(float DeltaTime)
{
	if (!bActivated) return;

	// EN: Drive material animation via "Time" parameter / ES: Animar material via parametro "Time"
	if (DynamicMaterial)
	{
		AnimationTime += DeltaTime;
		DynamicMaterial->SetScalarParameterValue(FName("Time"), AnimationTime);
	}

	// EN: Tip rotation / ES: Rotacion de consejos
	if (CachedProfile && CachedProfile->Tips.Num() > 1)
	{
		TipTimer += DeltaTime;
		if (TipTimer >= CachedProfile->TipRotationInterval)
		{
			CurrentTipIndex = (CurrentTipIndex + 1) % CachedProfile->Tips.Num();
			if (CachedWidget)
			{
				CachedWidget->SetTipText(CachedProfile->Tips[CurrentTipIndex]);
				CachedWidget->OnTipChanged(CachedProfile->Tips[CurrentTipIndex]);
			}
			TipTimer = 0.0f;
		}
	}
}

void UPGXLoadingStrategy_Material::Deactivate()
{
	DynamicMaterial = nullptr;
	LoadedMaterial = nullptr;

	PGX_LOG_VERBOSE(LogPGXLoading, TEXT("Strategy_Material: Deactivated."));
	Super::Deactivate();
}

FText UPGXLoadingStrategy_Material::GetCurrentTip() const
{
	if (!CachedProfile || CachedProfile->Tips.Num() == 0)
	{
		return FText::GetEmpty();
	}
	return CachedProfile->Tips[CurrentTipIndex % CachedProfile->Tips.Num()];
}
