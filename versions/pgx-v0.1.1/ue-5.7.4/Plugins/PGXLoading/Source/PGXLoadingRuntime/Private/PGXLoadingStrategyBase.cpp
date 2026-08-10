// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXLoadingStrategyBase.h"
#include "PGXLoadingRuntime.h"
#include "PGXLoadingProfile.h"
#include "PGXLoadingWidget.h"

void UPGXLoadingStrategyBase::InitializeStrategy(UPGXLoadingProfile* Profile)
{
	CachedProfile = Profile;
	bReady = false;
	bActivated = false;
}

void UPGXLoadingStrategyBase::Activate(UPGXLoadingWidget* Widget)
{
	CachedWidget = Widget;
	bActivated = true;
}

void UPGXLoadingStrategyBase::UpdateVisual(float DeltaTime)
{
	// EN: Default: no-op. Subclasses override for animated strategies.
	// ES: Default: no-op. Subclases sobrecargan para estrategias animadas.
}

void UPGXLoadingStrategyBase::Deactivate()
{
	// EN: Cancel any pending asset load / ES: Cancelar cualquier carga pendiente
	if (AssetLoadHandle.IsValid())
	{
		AssetLoadHandle->CancelHandle();
		AssetLoadHandle.Reset();
	}

	CachedWidget = nullptr;
	CachedProfile = nullptr;
	bActivated = false;
	bReady = false;
}

bool UPGXLoadingStrategyBase::IsReady() const
{
	return bReady;
}

void UPGXLoadingStrategyBase::OnProgressUpdated(const FPGXLoadingProgress& Progress)
{
	// EN: Forward to widget if available / ES: Reenviar al widget si disponible
	if (CachedWidget)
	{
		CachedWidget->SetProgressValue(Progress.TotalProgress);
		CachedWidget->SetStatusText(Progress.StatusMessage);
		CachedWidget->OnProgressChanged(Progress.TotalProgress, Progress.StatusMessage);
	}
}

FText UPGXLoadingStrategyBase::GetCurrentTip() const
{
	return FText::GetEmpty();
}
