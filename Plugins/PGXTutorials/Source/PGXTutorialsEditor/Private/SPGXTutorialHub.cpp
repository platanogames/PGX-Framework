// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Tutorial Hub — panel with language toggle, agent type badges, and per-system cards.
// ES: Tutorial Hub — panel con toggle de idioma, badges de tipo agente, y cards por sistema.

#include "SPGXTutorialHub.h"
#include "PGXTutorialRunner.h"
#include "Logging/PGXLogMacros.h"
#include "Tutorials/PGXOnboardingTutorials.h"
#include "Tutorials/PGXSystemTutorials.h"
#include "Style/PGXVisualTokens.h"
#include "Widgets/SPGXPanelHeader.h"
#include "Widgets/SPGXSectionDivider.h"
#include "Widgets/SPGXFooterBar.h"

#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Images/SImage.h"
#include "Styling/AppStyle.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "PGXTutorialHub"

DEFINE_LOG_CATEGORY_STATIC(LogPGXTutorialHub, Log, All);

// ============================================================================
// EN: Tutorial card definitions — data-driven
// ES: Definiciones de cards de tutoriales — data-driven
// ============================================================================

struct FTutorialCardDef
{
	const TCHAR* Id;
	const TCHAR* TitleEN;
	const TCHAR* TitleES;
	const TCHAR* DescEN;
	const TCHAR* DescES;
	int32 StepCount;
	FLinearColor Color;
	EPGXTutorialAgentType AgentType;
};

// EN: Onboarding tutorials (T1-T5)
static const FTutorialCardDef GOnboardingCards[] =
{
	{ TEXT("T1"), TEXT("What is PGX?"), TEXT("\u00bfQu\u00e9 es PGX?"),
	  TEXT("Complete tour of every PGX editor tool"), TEXT("Tour completo de cada herramienta del editor PGX"),
	  11, PGX::Semantic::Info, EPGXTutorialAgentType::Guide },

	{ TEXT("T2"), TEXT("Zero-Config Game Loop"), TEXT("Game Loop Sin C\u00f3digo"),
	  TEXT("Set up a full game loop with 5 DataAssets"), TEXT("Configura un game loop completo con 5 DataAssets"),
	  10, PGX::System::GameFlow, EPGXTutorialAgentType::Constructor },

	{ TEXT("T3"), TEXT("Data-Driven Workflow"), TEXT("Flujo Data-Driven"),
	  TEXT("How everything in PGX is configured via DataAssets"), TEXT("C\u00f3mo todo en PGX se configura via DataAssets"),
	  8, PGX::System::Construction, EPGXTutorialAgentType::Constructor },

	{ TEXT("T4"), TEXT("Full Observability"), TEXT("Observabilidad Total"),
	  TEXT("Monitor every system in real time"), TEXT("Monitoriza cada sistema en tiempo real"),
	  9, PGX::System::Log, EPGXTutorialAgentType::Guide },

	{ TEXT("T5"), TEXT("Decoupled Architecture"), TEXT("Arquitectura Desacoplada"),
	  TEXT("How systems communicate without dependencies"), TEXT("C\u00f3mo los sistemas se comunican sin dependencias"),
	  8, PGX::System::Message, EPGXTutorialAgentType::Guide },
};

// EN: System tutorials (S1-S13)
static const FTutorialCardDef GSystemCards[] =
{
	{ TEXT("S1"), TEXT("Profile System"), TEXT("Sistema Profile"),
	  TEXT("Project constitution, platforms, budgets"), TEXT("Constituci\u00f3n del proyecto, plataformas, budgets"),
	  10, PGX::System::Profile, EPGXTutorialAgentType::Constructor },

	{ TEXT("S2"), TEXT("GameFlow System"), TEXT("Sistema GameFlow"),
	  TEXT("Game states, transitions, history"), TEXT("Estados de juego, transiciones, historial"),
	  10, PGX::System::GameFlow, EPGXTutorialAgentType::Constructor },

	{ TEXT("S3"), TEXT("Save System"), TEXT("Sistema Save"),
	  TEXT("Persistence, slots, domains, auto-save"), TEXT("Persistencia, slots, dominios, auto-guardado"),
	  10, PGX::System::Save, EPGXTutorialAgentType::Constructor },

	{ TEXT("S4"), TEXT("PSO System"), TEXT("Sistema PSO"),
	  TEXT("Pipeline State Objects, precache, recording"), TEXT("Pipeline State Objects, precache, recording"),
	  10, PGX::System::PSO, EPGXTutorialAgentType::Constructor },

	{ TEXT("S5"), TEXT("Loading System"), TEXT("Sistema Loading"),
	  TEXT("Loading screens, strategies, progress"), TEXT("Pantallas de carga, estrategias, progreso"),
	  10, PGX::System::Loading, EPGXTutorialAgentType::Constructor },

	{ TEXT("S6"), TEXT("LevelFlow System"), TEXT("Sistema LevelFlow"),
	  TEXT("Level management, streaming, transitions"), TEXT("Gesti\u00f3n de niveles, streaming, transiciones"),
	  9, PGX::System::LevelFlow, EPGXTutorialAgentType::Constructor },

	{ TEXT("S7"), TEXT("Audio System"), TEXT("Sistema Audio"),
	  TEXT("Dual-backend, channels, mix, ducking"), TEXT("Dual-backend, canales, mix, ducking"),
	  12, PGX::System::Audio, EPGXTutorialAgentType::Constructor },

	{ TEXT("S8"), TEXT("Log System"), TEXT("Sistema Log"),
	  TEXT("Polymorphic observability, sinks, domains"), TEXT("Observabilidad polim\u00f3rfica, sinks, dominios"),
	  10, PGX::System::Log, EPGXTutorialAgentType::Constructor },

	{ TEXT("S9"), TEXT("Data Registry"), TEXT("Data Registry"),
	  TEXT("Typed data catalog, queries, cache"), TEXT("Cat\u00e1logo de datos tipado, queries, cache"),
	  10, PGX::System::DataRegistry, EPGXTutorialAgentType::Constructor },

	{ TEXT("S10"), TEXT("Construction System"), TEXT("Sistema Construction"),
	  TEXT("Class override pattern, 7 DA types"), TEXT("Patr\u00f3n class override, 7 tipos de DA"),
	  9, PGX::System::Construction, EPGXTutorialAgentType::Constructor },

	{ TEXT("S11"), TEXT("Message System"), TEXT("Sistema Message"),
	  TEXT("Pub/sub bus, channels, listeners"), TEXT("Bus pub/sub, canales, listeners"),
	  10, PGX::System::Message, EPGXTutorialAgentType::Constructor },

	{ TEXT("S12"), TEXT("EventHandler System"), TEXT("Sistema EventHandler"),
	  TEXT("Behavior resolution, priorities, telemetry"), TEXT("Resoluci\u00f3n de comportamiento, prioridades, telemetr\u00eda"),
	  10, PGX::System::EventHandler, EPGXTutorialAgentType::Constructor },

	{ TEXT("S13"), TEXT("MGOS System"), TEXT("Sistema MGOS"),
	  TEXT("Memory & GC observability, profiles"), TEXT("Observabilidad de memoria y GC, perfiles"),
	  10, PGX::System::MGOS, EPGXTutorialAgentType::Constructor },

	{ TEXT("S14"), TEXT("Trade System"), TEXT("Sistema Trade"),
	  TEXT("Trade actors, offers, reputation, economy"), TEXT("Actores de comercio, ofertas, reputacion, economia"),
	  11, PGX::System::Tutorials, EPGXTutorialAgentType::Constructor },

	{ TEXT("S15"), TEXT("Crafting System"), TEXT("Sistema Crafting"),
	  TEXT("Recipe definitions, register-driven crafting"), TEXT("Definiciones de recetas, crafting register-driven"),
	  9, PGX::Semantic::Info, EPGXTutorialAgentType::Constructor },
};

// ============================================================================
// Construction
// ============================================================================

void SPGXTutorialHub::Construct(const FArguments& InArgs)
{
	Runner = InArgs._Runner;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush(TEXT("ToolPanel.GroupBorder")))
		.BorderBackgroundColor(PGX::Surface::Base)
		.Padding(0)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SAssignNew(ContentBox, SVerticalBox)
			]
		]
	];

	RebuildContent();
}

void SPGXTutorialHub::RebuildContent()
{
	if (!ContentBox.IsValid()) return;
	ContentBox->ClearChildren();

	const bool bSpanish = (FPGXTutorialRunner::GetLanguage() == EPGXTutorialLanguage::Spanish);

	// -- Panel header --
	ContentBox->AddSlot().AutoHeight()
	[
		SNew(SPGXPanelHeader)
		.SystemColor(PGX::Semantic::Info)
		.Title(LOCTEXT("HubTitle", "PGX Tutorials"))
		.Subtitle(bSpanish
			? LOCTEXT("HubSubES", "Tutoriales guiados para cada sistema del framework")
			: LOCTEXT("HubSubEN", "Guided tutorials for every framework system"))
	];

	// -- Language toggle row --
	ContentBox->AddSlot().AutoHeight().Padding(PGX::Spacing::XL, PGX::Spacing::LG, PGX::Spacing::XL, PGX::Spacing::MD)
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, PGX::Spacing::MD, 0)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("LangLabel", "Language:"))
			.Font(PGX::Font::Body())
			.ColorAndOpacity(PGX::Text::Secondary)
		]

		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, PGX::Spacing::SM, 0)
		[
			SNew(SButton)
			.Text(LOCTEXT("LangEN", "English"))
			.ButtonColorAndOpacity(!bSpanish ? PGX::Surface::Active : PGX::Surface::Raised)
			.OnClicked_Lambda([this]() -> FReply
			{
				FPGXTutorialRunner::SetLanguage(EPGXTutorialLanguage::English);
				RebuildContent();
				return FReply::Handled();
			})
		]

		+ SHorizontalBox::Slot().AutoWidth()
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("Espa\u00f1ol")))
			.ButtonColorAndOpacity(bSpanish ? PGX::Surface::Active : PGX::Surface::Raised)
			.OnClicked_Lambda([this]() -> FReply
			{
				FPGXTutorialRunner::SetLanguage(EPGXTutorialLanguage::Spanish);
				RebuildContent();
				return FReply::Handled();
			})
		]

		+ SHorizontalBox::Slot().FillWidth(1.0f)
		[
			SNew(SSpacer)
		]

		// EN: Keep hub open toggle / ES: Toggle mantener hub abierto
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(PGX::Spacing::XL, 0, 0, 0)
		[
			SNew(SCheckBox)
			.IsChecked(FPGXTutorialRunner::GetKeepHubOpen() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
			.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
			{
				FPGXTutorialRunner::SetKeepHubOpen(NewState == ECheckBoxState::Checked);
			})
		]

		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(PGX::Spacing::SM, 0, 0, 0)
		[
			SNew(STextBlock)
			.Text(bSpanish
				? LOCTEXT("KeepHubES", "Mantener hub abierto")
				: LOCTEXT("KeepHubEN", "Keep hub open"))
			.Font(PGX::Font::Caption())
			.ColorAndOpacity(PGX::Text::Secondary)
		]
	];

	// -- Onboarding section --
	ContentBox->AddSlot().AutoHeight().Padding(PGX::Spacing::XL, 0, PGX::Spacing::XL, PGX::Spacing::SM)
	[
		SNew(SPGXSectionDivider)
		.Title(LOCTEXT("OnboardingSection", "Onboarding"))
		.AccentColor(PGX::Semantic::Info)
	];

	// EN: Onboarding cards wrap box
	TSharedRef<SWrapBox> OnboardingWrap = SNew(SWrapBox).UseAllottedSize(true);
	for (const FTutorialCardDef& Card : GOnboardingCards)
	{
		const FText Title = bSpanish ? FText::FromString(Card.TitleES) : FText::FromString(Card.TitleEN);
		const FText Desc = bSpanish ? FText::FromString(Card.DescES) : FText::FromString(Card.DescEN);

		OnboardingWrap->AddSlot().Padding(0, 0, PGX::Spacing::MD, PGX::Spacing::MD)
		[
			BuildTutorialCard(Title, Desc, Card.StepCount, Card.Color, Card.Id, Card.AgentType)
		];
	}

	ContentBox->AddSlot().AutoHeight().Padding(PGX::Spacing::XL, 0, PGX::Spacing::XL, PGX::Spacing::LG)
	[
		OnboardingWrap
	];

	// -- System Deep Dives section --
	ContentBox->AddSlot().AutoHeight().Padding(PGX::Spacing::XL, 0, PGX::Spacing::XL, PGX::Spacing::SM)
	[
		SNew(SPGXSectionDivider)
		.Title(bSpanish
			? LOCTEXT("SystemSectionES", "Sistemas Individuales")
			: LOCTEXT("SystemSectionEN", "System Deep Dives"))
		.AccentColor(PGX::Semantic::Info)
	];

	// EN: System cards wrap box
	TSharedRef<SWrapBox> SystemWrap = SNew(SWrapBox).UseAllottedSize(true);
	for (const FTutorialCardDef& Card : GSystemCards)
	{
		const FText Title = bSpanish ? FText::FromString(Card.TitleES) : FText::FromString(Card.TitleEN);
		const FText Desc = bSpanish ? FText::FromString(Card.DescES) : FText::FromString(Card.DescEN);

		SystemWrap->AddSlot().Padding(0, 0, PGX::Spacing::MD, PGX::Spacing::MD)
		[
			BuildTutorialCard(Title, Desc, Card.StepCount, Card.Color, Card.Id, Card.AgentType)
		];
	}

	ContentBox->AddSlot().AutoHeight().Padding(PGX::Spacing::XL, 0, PGX::Spacing::XL, PGX::Spacing::LG)
	[
		SystemWrap
	];

	// -- Footer --
	const int32 TotalTutorials = UE_ARRAY_COUNT(GOnboardingCards) + UE_ARRAY_COUNT(GSystemCards);
	ContentBox->AddSlot().AutoHeight()
	[
		SNew(SPGXFooterBar)
		.StatusText(FText::Format(
			bSpanish
				? LOCTEXT("FooterES", "{0} tutoriales disponibles")
				: LOCTEXT("FooterEN", "{0} tutorials available"),
			FText::AsNumber(TotalTutorials)))
	];
}

TSharedRef<SWidget> SPGXTutorialHub::BuildTutorialCard(
	const FText& Title,
	const FText& Description,
	int32 StepCount,
	const FLinearColor& Color,
	const FString& TutorialId,
	EPGXTutorialAgentType AgentType)
{
	const bool bSpanish = (FPGXTutorialRunner::GetLanguage() == EPGXTutorialLanguage::Spanish);

	// EN: Estimate ~1 min per 2 steps
	const int32 EstMinutes = FMath::Max(1, (StepCount + 1) / 2);
	const FText MetaText = FText::Format(
		bSpanish
			? LOCTEXT("CardMetaES", "{0} pasos \u00b7 ~{1} min")
			: LOCTEXT("CardMetaEN", "{0} steps \u00b7 ~{1} min"),
		FText::AsNumber(StepCount),
		FText::AsNumber(EstMinutes));

	// EN: Agent type badge text and color
	// ES: Texto y color del badge de tipo de agente
	const bool bIsConstructor = (AgentType == EPGXTutorialAgentType::Constructor);
	const FText BadgeText = bIsConstructor
		? (bSpanish ? LOCTEXT("ConstructorES", "Constructor") : LOCTEXT("ConstructorEN", "Constructor"))
		: (bSpanish ? LOCTEXT("GuideES", "Gu\u00eda") : LOCTEXT("GuideEN", "Guide"));
	// EN: Badge background — dimmed variants of PGX::Semantic tokens (constructor=Good=green, guide=Info=blue) per Bucket VT fix 2026-05-21
	// ES: Fondo del badge — variantes dimmed de tokens PGX::Semantic (constructor=Good=verde, guide=Info=azul)
	FLinearColor BadgeBg = bIsConstructor
		? PGX::Semantic::Good * 0.4f
		: PGX::Semantic::Info * 0.4f;
	BadgeBg.A = 0.6f;
	const FLinearColor BadgeTextColor = bIsConstructor
		? PGX::Semantic::Good
		: PGX::Semantic::Info;

	return SNew(SBox)
		.WidthOverride(190.0f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush(TEXT("ToolPanel.DarkGroupBorder")))
			.BorderBackgroundColor(PGX::Surface::Raised)
			.Padding(0)
			[
				SNew(SVerticalBox)

				// -- Accent bar --
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SBox).HeightOverride(PGX::Height::AccentBar)
					[
						SNew(SImage)
						.Image(FAppStyle::GetBrush(TEXT("WhiteBrush")))
						.ColorAndOpacity(Color)
					]
				]

				// -- Card body --
				+ SVerticalBox::Slot().AutoHeight().Padding(PGX::Spacing::MD)
				[
					SNew(SVerticalBox)

					// Agent type badge
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, PGX::Spacing::SM)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth()
						[
							SNew(SBorder)
							.BorderImage(FAppStyle::GetBrush(TEXT("WhiteBrush")))
							.BorderBackgroundColor(BadgeBg)
							.Padding(FMargin(PGX::Spacing::SM, 2.0f))
							[
								SNew(STextBlock)
								.Text(BadgeText)
								.Font(PGX::Font::Badge())
								.ColorAndOpacity(BadgeTextColor)
							]
						]
					]

					// Title
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, PGX::Spacing::SM)
					[
						SNew(STextBlock)
						.Text(Title)
						.Font(PGX::Font::SubHeader())
						.ColorAndOpacity(Color)
					]

					// Description
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, PGX::Spacing::SM)
					[
						SNew(STextBlock)
						.Text(Description)
						.Font(PGX::Font::Caption())
						.ColorAndOpacity(PGX::Text::Secondary)
						.AutoWrapText(true)
					]

					// Step count + time
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, PGX::Spacing::MD)
					[
						SNew(STextBlock)
						.Text(MetaText)
						.Font(PGX::Font::Caption())
						.ColorAndOpacity(PGX::Text::Muted)
					]

					// Start button
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SButton)
						.Text(bSpanish ? LOCTEXT("StartES", "Iniciar") : LOCTEXT("StartEN", "Start"))
						.HAlign(HAlign_Center)
						.OnClicked_Lambda([this, TutorialId]() -> FReply
						{
							LaunchTutorial(TutorialId);
							return FReply::Handled();
						})
					]
				]
			]
		];
}

void SPGXTutorialHub::LaunchTutorial(const FString& TutorialId)
{
	if (!Runner)
	{
		PGX_LOG_WARNING(LogPGXTutorialHub, TEXT("Cannot launch tutorial '%s' — no runner."), *TutorialId);
		return;
	}

	// EN: Optionally close the hub tab before launching (user preference)
	// ES: Opcionalmente cerrar el tab del hub antes de lanzar (preferencia del usuario)
	if (!FPGXTutorialRunner::GetKeepHubOpen())
	{
		TSharedPtr<SDockTab> HubTab = FGlobalTabmanager::Get()->FindExistingLiveTab(FName(TEXT("PGXTutorialsHub")));
		if (HubTab.IsValid())
		{
			HubTab->RequestCloseTab();
		}
	}

	// EN: Dispatch to content provider by tutorial ID
	// ES: Despachar al proveedor de contenido por ID de tutorial
	const EPGXTutorialLanguage Lang = FPGXTutorialRunner::GetLanguage();
	TArray<FPGXTutorialStep> Steps;

	// -- Onboarding --
	if (TutorialId == TEXT("T1")) Steps = PGXOnboardingTutorials::GetT1_WhatIsPGX(Lang);
	else if (TutorialId == TEXT("T2")) Steps = PGXOnboardingTutorials::GetT2_ZeroConfigGameLoop(Lang);
	else if (TutorialId == TEXT("T3")) Steps = PGXOnboardingTutorials::GetT3_DataDrivenWorkflow(Lang);
	else if (TutorialId == TEXT("T4")) Steps = PGXOnboardingTutorials::GetT4_FullObservability(Lang);
	else if (TutorialId == TEXT("T5")) Steps = PGXOnboardingTutorials::GetT5_DecoupledArchitecture(Lang);
	// -- System --
	else if (TutorialId == TEXT("S1")) Steps = PGXSystemTutorials::GetS1_Profile(Lang);
	else if (TutorialId == TEXT("S2")) Steps = PGXSystemTutorials::GetS2_GameFlow(Lang);
	else if (TutorialId == TEXT("S3")) Steps = PGXSystemTutorials::GetS3_Save(Lang);
	else if (TutorialId == TEXT("S4")) Steps = PGXSystemTutorials::GetS4_PSO(Lang);
	else if (TutorialId == TEXT("S5")) Steps = PGXSystemTutorials::GetS5_Loading(Lang);
	else if (TutorialId == TEXT("S6")) Steps = PGXSystemTutorials::GetS6_LevelFlow(Lang);
	else if (TutorialId == TEXT("S7")) Steps = PGXSystemTutorials::GetS7_Audio(Lang);
	else if (TutorialId == TEXT("S8")) Steps = PGXSystemTutorials::GetS8_Log(Lang);
	else if (TutorialId == TEXT("S9")) Steps = PGXSystemTutorials::GetS9_DataRegistry(Lang);
	else if (TutorialId == TEXT("S10")) Steps = PGXSystemTutorials::GetS10_Construction(Lang);
	else if (TutorialId == TEXT("S11")) Steps = PGXSystemTutorials::GetS11_Message(Lang);
	else if (TutorialId == TEXT("S12")) Steps = PGXSystemTutorials::GetS12_EventHandler(Lang);
	else if (TutorialId == TEXT("S13")) Steps = PGXSystemTutorials::GetS13_MGOS(Lang);
	else if (TutorialId == TEXT("S14")) Steps = PGXSystemTutorials::GetS14_Trade(Lang);
	else if (TutorialId == TEXT("S15")) Steps = PGXSystemTutorials::GetS15_Crafting(Lang);

	if (Steps.Num() == 0)
	{
		PGX_LOG_WARNING(LogPGXTutorialHub,
			TEXT("Tutorial '%s' has no steps yet. Content not implemented."), *TutorialId);
	}
	else
	{
		Runner->StartTutorial(Steps);
	}
}

#undef LOCTEXT_NAMESPACE
