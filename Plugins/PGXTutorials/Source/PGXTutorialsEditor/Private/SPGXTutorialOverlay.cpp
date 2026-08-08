// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Tutorial overlay — arrow indicator + callout + action feedback + navigation UI.
// ES: Overlay de tutorial — indicador flecha + callout + feedback de acciones + UI de navegacion.

#include "SPGXTutorialOverlay.h"
#include "PGXTutorialRunner.h"
#include "PGXTutorialTypes.h"
#include "PGXTutorialActionExecutor.h"
#include "Style/PGXVisualTokens.h"

#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"
#include "Rendering/DrawElements.h"
#include "Styling/AppStyle.h"
#include "Framework/Application/SlateApplication.h"

void SPGXTutorialOverlay::Construct(const FArguments& InArgs)
{
	Runner = InArgs._Runner;
	SetVisibility(EVisibility::SelfHitTestInvisible);

	// EN: Initialize pending base path from runner
	// ES: Inicializar ruta base pendiente desde el runner
	PendingBasePath = FPGXTutorialRunner::GetBasePath();

	// EN: Canvas for positioning the callout dynamically
	// ES: Canvas para posicionar el callout dinamicamente
	ChildSlot
	[
		SAssignNew(CalloutCanvas, SCanvas)
	];

	// EN: Listen for step changes
	// ES: Escuchar cambios de paso
	if (Runner)
	{
		Runner->OnStepChanged.BindRaw(this, &SPGXTutorialOverlay::RebuildCallout);
		RebuildCallout();
	}
}

void SPGXTutorialOverlay::RebuildCallout()
{
	if (!Runner || !Runner->IsActive())
	{
		if (CalloutCanvas.IsValid())
		{
			CalloutCanvas->ClearChildren();
		}
		CalloutWidget.Reset();
		return;
	}

	const FPGXTutorialStep* Step = Runner->GetCurrentStep();
	if (!Step) return;

	// EN: When step changes, wait for layout if targeting a tab
	// ES: Al cambiar paso, esperar layout si apunta a un tab
	if (Step->TargetTabId != NAME_None)
	{
		bWaitingForLayout = true;
		LayoutWaitFrames = 2;
	}

	const FLinearColor Accent = Step->AccentColor;
	const int32 StepIdx = Runner->GetCurrentStepIndex() + 1;
	const int32 StepTotal = Runner->GetStepCount();
	const float Progress = Runner->GetProgress();
	const bool bIsLast = Runner->IsLastStep();
	const bool bIsFirst = Runner->IsFirstStep();
	const bool bIsConfigStep = (Step->Action == EPGXTutorialAction::ConfigBasePath);
	const bool bSpanish = (FPGXTutorialRunner::GetLanguage() == EPGXTutorialLanguage::Spanish);

	// EN: Get action result for feedback display
	// ES: Obtener resultado de accion para mostrar feedback
	const FPGXTutorialActionResult& ActionResult = Runner->GetLastActionResult();
	const bool bHasFeedback = !ActionResult.FeedbackText.IsEmpty();

	// EN: Step counter text "Step 3 of 8"
	FText CounterText = FText::Format(
		NSLOCTEXT("PGXTutorials", "StepCounter", "Step {0} of {1}"),
		FText::AsNumber(StepIdx),
		FText::AsNumber(StepTotal)
	);

	// EN: Build the callout content vertical box
	// ES: Construir el vertical box del contenido del callout
	TSharedRef<SVerticalBox> ContentVBox = SNew(SVerticalBox)

		// -- Accent bar --
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, PGX::Spacing::LG)
		[
			SNew(SBox).HeightOverride(PGX::Height::AccentBar)
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush(TEXT("WhiteBrush")))
				.ColorAndOpacity(Accent)
			]
		]

		// -- Title + Step counter inline --
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, PGX::Spacing::MD)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.0f)
			[
				SNew(STextBlock)
				.Text(Step->Title)
				.Font(PGX::Font::PanelTitle())
				.ColorAndOpacity(Accent)
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(CounterText)
				.Font(PGX::Font::Caption())
				.ColorAndOpacity(PGX::Text::Muted)
			]
		];

	// -- Target tab indicator (shows which panel to look at) --
	if (Step->TargetTabId != NAME_None)
	{
		FString TabLabel;
		TSharedPtr<SDockTab> TargetTab = FGlobalTabmanager::Get()->FindExistingLiveTab(Step->TargetTabId);
		if (TargetTab.IsValid())
		{
			TabLabel = TargetTab->GetTabLabel().ToString();
		}
		else
		{
			TabLabel = Step->TargetTabId.ToString();
		}

		ContentVBox->AddSlot().AutoHeight().Padding(0, 0, 0, PGX::Spacing::MD)
		[
			SNew(STextBlock)
			.Text(FText::Format(
				NSLOCTEXT("PGXTutorials", "TargetIndicator", "\u25B6 {0}"),
				FText::FromString(TabLabel)))
			.Font(PGX::Font::Caption())
			.ColorAndOpacity(Accent)
		];
	}

	// -- Separator between title section and content --
	ContentVBox->AddSlot().AutoHeight().Padding(0, PGX::Spacing::SM, 0, PGX::Spacing::LG)
	[
		SNew(SBox).HeightOverride(1.0f)
		[
			SNew(SImage)
			.Image(FAppStyle::GetBrush(TEXT("WhiteBrush")))
			.ColorAndOpacity(PGX::Border::Subtle)
		]
	];

	// -- Action feedback (between title and description) --
	if (bHasFeedback)
	{
		// EN: Feedback background — dimmed variants of PGX::Semantic tokens (success=Good, fail=Error) per Bucket VT fix 2026-05-21
		// ES: Fondo del feedback — variantes dimmed de tokens PGX::Semantic (success=Good, fail=Error)
		FLinearColor FeedbackBg = ActionResult.bSuccess
			? PGX::Semantic::Good * 0.5f
			: PGX::Semantic::Error * 0.625f;
		FeedbackBg.A = 0.8f;
		const FLinearColor FeedbackText = ActionResult.bSuccess
			? PGX::Semantic::Good
			: PGX::Semantic::Error;

		ContentVBox->AddSlot().AutoHeight().Padding(0, 0, 0, PGX::Spacing::SM)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush(TEXT("WhiteBrush")))
			.BorderBackgroundColor(FeedbackBg)
			.Padding(FMargin(PGX::Spacing::MD, PGX::Spacing::MD))
			[
				SNew(STextBlock)
				.Text(ActionResult.FeedbackText)
				.Font(PGX::Font::Caption())
				.ColorAndOpacity(FeedbackText)
				.AutoWrapText(true)
				.LineHeightPercentage(1.4f)
			]
		];
	}

	// -- Description --
	ContentVBox->AddSlot().AutoHeight().Padding(0, 0, 0, 32.0f)
	[
		SNew(STextBlock)
		.Text(Step->Description)
		.Font(PGX::Font::Body())
		.ColorAndOpacity(PGX::Text::Primary)
		.AutoWrapText(true)
		.LineHeightPercentage(1.5f)
	];

	// -- ConfigBasePath: editable text box for base path --
	if (bIsConfigStep)
	{
		PendingBasePath = FPGXTutorialRunner::GetBasePath();

		ContentVBox->AddSlot().AutoHeight().Padding(0, 0, 0, PGX::Spacing::MD)
		[
			SNew(STextBlock)
			.Text(bSpanish
				? NSLOCTEXT("PGXTutorials", "BasePathLabelES", "Ruta base:")
				: NSLOCTEXT("PGXTutorials", "BasePathLabelEN", "Base path:"))
			.Font(PGX::Font::Caption())
			.ColorAndOpacity(PGX::Text::Secondary)
		];

		ContentVBox->AddSlot().AutoHeight().Padding(0, 0, 0, PGX::Spacing::XL)
		[
			SNew(SEditableTextBox)
			.Text(FText::FromString(PendingBasePath))
			.Font(PGX::Font::Body())
			.OnTextChanged_Lambda([this](const FText& NewText)
			{
				PendingBasePath = NewText.ToString();
			})
		];
	}

	// -- Separator before progress --
	ContentVBox->AddSlot().AutoHeight().Padding(0, 0, 0, PGX::Spacing::LG)
	[
		SNew(SBox).HeightOverride(1.0f)
		[
			SNew(SImage)
			.Image(FAppStyle::GetBrush(TEXT("WhiteBrush")))
			.ColorAndOpacity(PGX::Border::Subtle)
		]
	];

	// -- Progress bar --
	ContentVBox->AddSlot().AutoHeight().Padding(0, 0, 0, PGX::Spacing::XL)
	[
		SNew(SBox).HeightOverride(6.0f)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush(TEXT("WhiteBrush")))
				.ColorAndOpacity(PGX::Surface::Active)
			]
			+ SOverlay::Slot()
			[
				SNew(SBox)
				.WidthOverride(CalloutWidth * Progress)
				.HAlign(HAlign_Left)
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush(TEXT("WhiteBrush")))
					.ColorAndOpacity(Accent)
				]
			]
		]
	];

	// -- Navigation buttons --
	TSharedRef<SHorizontalBox> NavButtons = SNew(SHorizontalBox)

		// Back button
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, PGX::Spacing::MD, 0)
		[
			SNew(SButton)
			.Text(bSpanish
				? NSLOCTEXT("PGXTutorials", "BackES", "Atr\u00e1s")
				: NSLOCTEXT("PGXTutorials", "Back", "Back"))
			.IsEnabled(!bIsFirst)
			.OnClicked_Lambda([this]() -> FReply
			{
				if (Runner) Runner->PrevStep();
				return FReply::Handled();
			})
		]

		+ SHorizontalBox::Slot().FillWidth(1.0f)
		[
			SNew(SSpacer)
		];

	// -- Cleanup button (last step only, if assets were created) --
	if (bIsLast && FPGXTutorialActionExecutor::HasCreatedAssets())
	{
		NavButtons->AddSlot().AutoWidth().Padding(0, 0, PGX::Spacing::SM, 0)
		[
			SNew(SButton)
			.Text(bSpanish
				? NSLOCTEXT("PGXTutorials", "CleanupES", "Limpiar assets")
				: NSLOCTEXT("PGXTutorials", "CleanupEN", "Clean up assets"))
			.OnClicked_Lambda([this]() -> FReply
			{
				FPGXTutorialActionExecutor::CleanupTutorialAssets();
				// EN: Rebuild callout to update cleanup button state
				// ES: Reconstruir callout para actualizar estado del boton
				RebuildCallout();
				return FReply::Handled();
			})
		];
	}

	// -- Close button --
	NavButtons->AddSlot().AutoWidth().Padding(0, 0, PGX::Spacing::MD, 0)
	[
		SNew(SButton)
		.Text(bSpanish
			? NSLOCTEXT("PGXTutorials", "CloseES", "Cerrar")
			: NSLOCTEXT("PGXTutorials", "Close", "Close"))
		.OnClicked_Lambda([this]() -> FReply
		{
			if (Runner) Runner->Close();
			return FReply::Handled();
		})
	];

	// -- Next/Finish button --
	FText NextText;
	if (bIsConfigStep)
	{
		NextText = bSpanish
			? NSLOCTEXT("PGXTutorials", "ContinueES", "Continuar")
			: NSLOCTEXT("PGXTutorials", "ContinueEN", "Continue");
	}
	else if (bIsLast)
	{
		NextText = bSpanish
			? NSLOCTEXT("PGXTutorials", "FinishES", "Finalizar")
			: NSLOCTEXT("PGXTutorials", "Finish", "Finish");
	}
	else
	{
		NextText = bSpanish
			? NSLOCTEXT("PGXTutorials", "NextES", "Siguiente")
			: NSLOCTEXT("PGXTutorials", "Next", "Next");
	}

	NavButtons->AddSlot().AutoWidth()
	[
		SNew(SButton)
		.Text(NextText)
		.OnClicked_Lambda([this]() -> FReply
		{
			if (Runner)
			{
				// EN: If config step, save the base path before advancing
				// ES: Si es paso de config, guardar ruta base antes de avanzar
				const FPGXTutorialStep* CurrentStep = Runner->GetCurrentStep();
				if (CurrentStep && CurrentStep->Action == EPGXTutorialAction::ConfigBasePath
					&& !PendingBasePath.IsEmpty())
				{
					FPGXTutorialRunner::SetBasePath(PendingBasePath);
				}
				Runner->NextStep();
			}
			return FReply::Handled();
		})
	];

	ContentVBox->AddSlot().AutoHeight()
	[
		NavButtons
	];

	// EN: Wrap in callout box
	// ES: Envolver en caja del callout
	CalloutWidget =
		SNew(SBox)
		.WidthOverride(CalloutWidth)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush(TEXT("ToolPanel.GroupBorder")))
			.BorderBackgroundColor(PGX::Surface::Elevated)
			.Padding(FMargin(PGX::Spacing::XXL, PGX::Spacing::XXL, PGX::Spacing::XXL, PGX::Spacing::XL))
			[
				ContentVBox
			]
		];

	// EN: Add to canvas
	// ES: Agregar al canvas
	if (CalloutCanvas.IsValid())
	{
		CalloutCanvas->ClearChildren();
		CalloutCanvas->AddSlot()
			.Position(TAttribute<FVector2D>::CreateLambda([this]() { return CalloutPosition; }))
			.Size(FVector2D(CalloutWidth, CalloutHeight))
			[
				CalloutWidget.ToSharedRef()
			];
	}
}

void SPGXTutorialOverlay::UpdateTargetGeometry()
{
	bHasValidTarget = false;

	if (!Runner || !Runner->IsActive()) return;

	// EN: If waiting for a newly opened tab to layout, count down frames
	// ES: Si esperamos que un tab recien abierto se layoutee, contar frames
	if (bWaitingForLayout)
	{
		if (LayoutWaitFrames > 0)
		{
			LayoutWaitFrames--;
			return;
		}
		bWaitingForLayout = false;
	}

	const FPGXTutorialStep* Step = Runner->GetCurrentStep();
	if (!Step || Step->TargetTabId == NAME_None) return;

	TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->FindExistingLiveTab(Step->TargetTabId);
	if (!Tab.IsValid()) return;

	// EN: Try tab content first (the panel body), fall back to the SDockTab itself
	// ES: Intentar contenido del tab primero (el cuerpo del panel), fallback al SDockTab mismo
	TSharedRef<SWidget> TabContent = Tab->GetContent();
	const FVector2D ContentSize = TabContent->GetTickSpaceGeometry().GetLocalSize();

	if (ContentSize.SizeSquared() > 0)
	{
		CachedTargetGeometry = TabContent->GetTickSpaceGeometry();
		bHasValidTarget = true;
	}
	else
	{
		// EN: Fallback — use the dock tab widget itself (includes the tab bar area)
		// ES: Fallback — usar el widget del dock tab (incluye el area de la barra de tabs)
		const FVector2D TabSize = Tab->GetTickSpaceGeometry().GetLocalSize();
		if (TabSize.SizeSquared() > 0)
		{
			CachedTargetGeometry = Tab->GetTickSpaceGeometry();
			bHasValidTarget = true;
		}
	}
}

void SPGXTutorialOverlay::PositionCallout(const FGeometry& AllottedGeometry)
{
	const FVector2D WindowSize = AllottedGeometry.GetLocalSize();
	constexpr float Margin = PGX::Spacing::XXL; // 24px

	// EN: Fixed right-side positioning, vertically centered — never overlaps with target panels
	// ES: Posicionamiento fijo a la derecha, centrado verticalmente — nunca se solapa con paneles target
	CalloutPosition = FVector2D(
		WindowSize.X - CalloutWidth - Margin,
		(WindowSize.Y - CalloutHeight) * 0.5f);

	// EN: Arrow always points left (toward target panels)
	// ES: Flecha siempre apunta a la izquierda (hacia los paneles target)
	if (bHasValidTarget)
	{
		ArrowDir = EArrowDirection::Left;
		ArrowBasePosition = FVector2D(
			CalloutPosition.X - Margin * 0.5f,
			CalloutPosition.Y + CalloutHeight * 0.35f); // upper-third
	}
	else
	{
		ArrowDir = EArrowDirection::None;
		ArrowBasePosition = FVector2D::ZeroVector;
	}
}

void SPGXTutorialOverlay::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (!Runner || !Runner->IsActive()) return;

	UpdateTargetGeometry();
	PositionCallout(AllottedGeometry);
	ArrowTime += InDeltaTime;
}

int32 SPGXTutorialOverlay::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
	int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	if (!Runner || !Runner->IsActive())
	{
		return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	}

	// EN: Paint children first (the callout canvas), then draw arrow on top
	// ES: Pintar hijos primero (el canvas del callout), luego dibujar flecha encima
	int32 ChildLayerId = SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	// EN: Draw animated arrow chevron pointing toward the target tab
	// ES: Dibujar flecha chevron animada apuntando al tab objetivo
	if (bHasValidTarget && ArrowDir != EArrowDirection::None)
	{
		ChildLayerId++;
		const float SinOffset = FMath::Sin(ArrowTime * ArrowBounceSpeed * 2.0f * UE_PI) * ArrowBounceAmplitude;

		const FPGXTutorialStep* Step = Runner->GetCurrentStep();
		const FLinearColor AccentColor = Step ? Step->AccentColor : FLinearColor::White;

		FVector2D OffsetDir = FVector2D::ZeroVector;
		TArray<FVector2D> ChevronPoints;

		switch (ArrowDir)
		{
		case EArrowDirection::Right:
			OffsetDir = FVector2D(SinOffset, 0.0f);
			ChevronPoints = { FVector2D(-ArrowChevronSize, -ArrowChevronSize), FVector2D(0.0f, 0.0f), FVector2D(-ArrowChevronSize, ArrowChevronSize) };
			break;
		case EArrowDirection::Left:
			OffsetDir = FVector2D(-SinOffset, 0.0f);
			ChevronPoints = { FVector2D(ArrowChevronSize, -ArrowChevronSize), FVector2D(0.0f, 0.0f), FVector2D(ArrowChevronSize, ArrowChevronSize) };
			break;
		case EArrowDirection::Up:
			OffsetDir = FVector2D(0.0f, -SinOffset);
			ChevronPoints = { FVector2D(-ArrowChevronSize, ArrowChevronSize), FVector2D(0.0f, 0.0f), FVector2D(ArrowChevronSize, ArrowChevronSize) };
			break;
		case EArrowDirection::Down:
			OffsetDir = FVector2D(0.0f, SinOffset);
			ChevronPoints = { FVector2D(-ArrowChevronSize, -ArrowChevronSize), FVector2D(0.0f, 0.0f), FVector2D(ArrowChevronSize, -ArrowChevronSize) };
			break;
		default:
			break;
		}

		// EN: Translate chevron to arrow base position + sine offset
		// ES: Trasladar chevron a posicion base de la flecha + offset sine
		const FVector2D FinalPos = ArrowBasePosition + OffsetDir;
		for (FVector2D& Pt : ChevronPoints)
		{
			Pt += FinalPos;
		}

		FSlateDrawElement::MakeLines(
			OutDrawElements, ChildLayerId,
			AllottedGeometry.ToPaintGeometry(),
			ChevronPoints,
			ESlateDrawEffect::None,
			AccentColor, true, ArrowLineThickness);
	}

	return ChildLayerId;
}

FReply SPGXTutorialOverlay::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	// The overlay is intentionally non-modal: clicks pass through to the editor
	// while the tutorial annotates the active workflow. Construct registers the
	// widget as SelfHitTestInvisible, so both branches return Unhandled.
	//
	// El overlay es no modal: los clicks pasan al editor mientras el tutorial
	// anota el flujo activo. Por eso ambas ramas devuelven Unhandled.
	return FReply::Unhandled();
}
