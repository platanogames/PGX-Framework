// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Tutorial Hub NomadTab — central panel for discovering and launching all PGX tutorials.
// ES: NomadTab del Tutorial Hub — panel central para descubrir y lanzar todos los tutoriales PGX.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "PGXTutorialTypes.h"

class FPGXTutorialRunner;

/**
 * EN: Tutorial Hub — shows Onboarding and System tutorial cards with language toggle.
 *     Click "Start" on any card to launch that tutorial with the selected language.
 * ES: Tutorial Hub — muestra cards de tutoriales Onboarding y Sistema con toggle de idioma.
 *     Click "Start" en cualquier card para lanzar ese tutorial con el idioma seleccionado.
 */
class SPGXTutorialHub : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXTutorialHub) {}
		SLATE_ARGUMENT(FPGXTutorialRunner*, Runner)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	/** EN: Rebuild the full panel (called on language change) / ES: Reconstruir panel completo */
	void RebuildContent();

	/** EN: Build a single tutorial card with agent type badge / ES: Construir una card con badge de tipo agente */
	TSharedRef<SWidget> BuildTutorialCard(
		const FText& Title,
		const FText& Description,
		int32 StepCount,
		const FLinearColor& Color,
		const FString& TutorialId,
		EPGXTutorialAgentType AgentType
	);

	/** EN: Launch a tutorial by ID / ES: Lanzar un tutorial por ID */
	void LaunchTutorial(const FString& TutorialId);

	FPGXTutorialRunner* Runner = nullptr;

	/** EN: Content box for dynamic rebuild / ES: Caja de contenido para rebuild dinamico */
	TSharedPtr<SVerticalBox> ContentBox;
};
