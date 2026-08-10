// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXViewportOverlayManager.h"
#include "Logging/PGXLogMacros.h"
#include "PGXLoadingRuntime.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameViewportClient.h"
#include "Engine/GameInstance.h"
#include "Widgets/SWeakWidget.h"
#include "Widgets/Layout/SBorder.h"

void UPGXViewportOverlayManager::ShowOverlay(int32 ZOrder, TSubclassOf<UUserWidget> WidgetClass)
{
	// EN: Prevent duplicate overlays / ES: Prevenir overlays duplicados
	if (bOverlayActive)
	{
		PGX_LOG_VERBOSE(LogPGXLoading, TEXT("OverlayManager: ShowOverlay called but overlay already active — ignoring."));
		return;
	}

	// EN: Get the GameInstance that owns us / ES: Obtener la GameInstance que nos posee
	UGameInstance* GI = GetTypedOuter<UGameInstance>();
	if (!GI)
	{
		// EN: Fallback: walk outer chain to find any UGameInstance
		// ES: Fallback: buscar cualquier UGameInstance en la cadena de outers
		UObject* CurrentOuter = GetOuter();
		while (CurrentOuter)
		{
			GI = Cast<UGameInstance>(CurrentOuter);
			if (GI) break;
			CurrentOuter = CurrentOuter->GetOuter();
		}
	}

	if (!GI)
	{
		PGX_LOG_ERROR(LogPGXLoading, TEXT("OverlayManager: Cannot find GameInstance — overlay not created."));
		return;
	}

	UGameViewportClient* ViewportClient = GI->GetGameViewportClient();
	if (!ViewportClient)
	{
		PGX_LOG_ERROR(LogPGXLoading, TEXT("OverlayManager: GameViewportClient is null — overlay not created."));
		return;
	}

	// EN: Create the widget — only if a concrete class is provided.
	//     UUserWidget is abstract and cannot be instantiated directly.
	//     When no class is given, the Slate fade handles the visual blackout.
	// ES: Crear el widget — solo si se provee una clase concreta.
	//     UUserWidget es abstracto y no se puede instanciar directamente.
	//     Cuando no hay clase, el fade de Slate maneja el blackout visual.
	if (WidgetClass)
	{
		ActiveWidget = CreateWidget<UUserWidget>(GI, WidgetClass);
	}

	// EN: Widget is optional — overlay can work without it (Slate fade only)
	// ES: Widget es opcional — el overlay puede funcionar sin el (solo fade Slate)
	if (ActiveWidget)
	{
		// EN: Get the Slate widget for viewport injection
		// ES: Obtener el widget Slate para inyeccion en viewport
		OverlaySlateWidget = ActiveWidget->TakeWidget();

		// EN: Inject into viewport — this survives OpenLevel because it's on the ViewportClient
		// ES: Inyectar en viewport — esto sobrevive OpenLevel porque esta en el ViewportClient
		ViewportClient->AddViewportWidgetContent(
			SNew(SWeakWidget).PossiblyNullContent(OverlaySlateWidget),
			ZOrder
		);

		ActiveWidget->SetRenderOpacity(1.0f);
	}

	bOverlayActive = true;

	PGX_LOG_INFO(LogPGXLoading, TEXT("OverlayManager: Overlay shown (ZOrder=%d, Class=%s)"),
		ZOrder, WidgetClass ? *WidgetClass->GetName() : TEXT("FadeOnly"));
}

void UPGXViewportOverlayManager::HideOverlay()
{
	if (!bOverlayActive)
	{
		return;
	}

	// EN: Remove from viewport
	// ES: Remover del viewport
	if (OverlaySlateWidget.IsValid())
	{
		UGameInstance* GI = GetTypedOuter<UGameInstance>();
		if (!GI)
		{
			UObject* CurrentOuter = GetOuter();
			while (CurrentOuter)
			{
				GI = Cast<UGameInstance>(CurrentOuter);
				if (GI) break;
				CurrentOuter = CurrentOuter->GetOuter();
			}
		}

		if (GI)
		{
			UGameViewportClient* ViewportClient = GI->GetGameViewportClient();
			if (ViewportClient)
			{
				ViewportClient->RemoveViewportWidgetContent(OverlaySlateWidget.ToSharedRef());
			}
		}

		OverlaySlateWidget.Reset();
	}

	// EN: Release widget reference
	// ES: Liberar referencia del widget
	if (ActiveWidget)
	{
		ActiveWidget->RemoveFromParent();
		ActiveWidget = nullptr;
	}

	bOverlayActive = false;

	PGX_LOG_INFO(LogPGXLoading, TEXT("OverlayManager: Overlay hidden."));
}

void UPGXViewportOverlayManager::SetOverlayOpacity(float Alpha)
{
	if (ActiveWidget)
	{
		ActiveWidget->SetRenderOpacity(FMath::Clamp(Alpha, 0.0f, 1.0f));
	}
}
