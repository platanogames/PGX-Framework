// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Visual Showcase Tab — AAA standard validation of PGX visual system.
// ES: Tab de Visual Showcase — validacion estandar AAA del sistema visual PGX.
#include "Showcase/SPGXVisualShowcaseTab.h"

// EN: PGX Visual Components / ES: Componentes Visuales PGX
#include "Style/PGXVisualTokens.h"
#include "Widgets/SPGXPremiumShell.h"
#include "Widgets/SPGXAccentBar.h"
#include "Widgets/SPGXStatusBadge.h"
#include "Widgets/SPGXKPIChip.h"
#include "Widgets/SPGXSectionDivider.h"
#include "Widgets/SPGXFooterBar.h"
#include "Widgets/SPGXEmptyStateV2.h"
#include "Widgets/SPGXDataTable.h"
#include "Widgets/SPGXHealthDot.h"
#include "Widgets/SPGXBudgetBar.h"
#include "Widgets/SPGXTableRow.h"
#include "Utils/SPGXTelemetryGraph.h"

// EN: Slate includes / ES: Includes de Slate
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Style/PGXEditorStyle.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "PGXVisualShowcase"

// ============================================================================
// EN: Density & Theme Helpers / ES: Helpers de Densidad y Theme
// ============================================================================

float SPGXVisualShowcaseTab::GetRowHeight() const
{
	return CurrentDensity == EPGXShowcaseDensity::Compact
		? PGX::Height::TableRowCompact : PGX::Height::TableRow;
}

float SPGXVisualShowcaseTab::GetBodyFontSize() const
{
	return CurrentDensity == EPGXShowcaseDensity::Compact ? 9.0f : 10.0f;
}

float SPGXVisualShowcaseTab::GetPaddingScale() const
{
	return CurrentDensity == EPGXShowcaseDensity::Compact ? 0.75f : 1.0f;
}

FSlateFontInfo SPGXVisualShowcaseTab::GetBodyFont() const
{
	return FCoreStyle::GetDefaultFontStyle("Regular", GetBodyFontSize());
}

FSlateFontInfo SPGXVisualShowcaseTab::GetCaptionFont() const
{
	const float Size = CurrentDensity == EPGXShowcaseDensity::Compact ? 7.0f : 8.0f;
	return FCoreStyle::GetDefaultFontStyle("Regular", Size);
}

FLinearColor SPGXVisualShowcaseTab::GetSurfaceBase() const
{
	return CurrentTheme == EPGXShowcaseTheme::ValidationLight
		? FLinearColor(0.85f, 0.85f, 0.87f, 1.0f)
		: PGX::Surface::Base;
}

FLinearColor SPGXVisualShowcaseTab::GetSurfaceRaised() const
{
	return CurrentTheme == EPGXShowcaseTheme::ValidationLight
		? FLinearColor(0.92f, 0.92f, 0.93f, 1.0f)
		: PGX::Surface::Raised;
}

FLinearColor SPGXVisualShowcaseTab::GetTextPrimary() const
{
	return CurrentTheme == EPGXShowcaseTheme::ValidationLight
		? FLinearColor(0.1f, 0.1f, 0.1f, 1.0f)
		: PGX::Text::Primary;
}

FLinearColor SPGXVisualShowcaseTab::GetBorderDefault() const
{
	return CurrentTheme == EPGXShowcaseTheme::ValidationLight
		? FLinearColor(0.0f, 0.0f, 0.0f, 0.12f)
		: PGX::Border::Default;
}

int32 SPGXVisualShowcaseTab::CountByHealth(int32 SeverityOrder) const
{
	int32 Count = 0;
	for (const auto& Entry : SystemEntries)
	{
		if (Entry->SeverityOrder == SeverityOrder) { ++Count; }
	}
	return Count;
}

// ============================================================================
// EN: Filter Button Helper / ES: Helper de Boton de Filtro
// ============================================================================

TSharedRef<SWidget> SPGXVisualShowcaseTab::BuildFilterButton(const FText& Label, bool bActive, FOnClicked OnClick)
{
	return SNew(SBox)
		.Padding(FMargin(PGX::Spacing::SM * GetPaddingScale(), PGX::Spacing::XS))
		[
			SNew(SButton)
			.ButtonStyle(FAppStyle::Get(), "SimpleButton")
			.OnClicked(OnClick)
			[
				SNew(SBorder)
				.BorderBackgroundColor(bActive ? PGX::Semantic::Info : PGX::Border::Subtle)
				.Padding(FMargin(PGX::Spacing::MD, PGX::Spacing::XS))
				[
					SNew(STextBlock)
					.Text(Label)
					.Font(PGX::Font::Badge())
					.ColorAndOpacity(FSlateColor(bActive ? PGX::Text::OnColor : PGX::Text::Muted))
				]
			]
		];
}

// ============================================================================
// EN: Construct / ES: Construir
// ============================================================================

void SPGXVisualShowcaseTab::Construct(const FArguments& /*InArgs*/)
{
	PopulateFakeData();

	ChildSlot
	[
		SNew(SPGXPremiumShell)
		.SystemColor(PGX::System::MGOS)
		.Title(LOCTEXT("ShellTitle", "VISUAL SHOWCASE"))
		.Subtitle(LOCTEXT("ShellSubtitle", "AAA visual system validation"))
		.Icon(FPGXEditorStyle::Get().GetBrush("PGXEditor.Icon.VisualShowcase"))
		.bShowFooter(true)
		.StatusText(LOCTEXT("ShellFooter", "PGX v0.4.0 | Showcase Mode"))
		.Content()
		[
			SNew(SBorder)
			.BorderBackgroundColor_Lambda([this]() { return GetSurfaceBase(); })
			.Padding(0)
			[
				SAssignNew(RootContent, SVerticalBox)
			]
		]
	];

	RebuildContent();
	PopulateFakeGraphData();
}

void SPGXVisualShowcaseTab::RebuildContent()
{
	if (!RootContent.IsValid()) { return; }

	RootContent->ClearChildren();

	// EN: Command bar (theme/density toggles — no longer the header, SPGXPremiumShell handles that)
	// ES: Barra de comandos (toggles de theme/densidad — ya no es el header, SPGXPremiumShell lo maneja)
	RootContent->AddSlot()
		.AutoHeight()
		[BuildCommandHeader()];

	// EN: Health Summary Line / ES: Linea Resumen de Salud
	RootContent->AddSlot()
		.AutoHeight()
		.Padding(FMargin(0.0f, PGX::Spacing::XS, 0.0f, 0.0f))
		[BuildHealthSummaryLine()];

	// EN: KPI Strip / ES: Franja KPI
	RootContent->AddSlot()
		.AutoHeight()
		.Padding(FMargin(0.0f, PGX::Spacing::SM, 0.0f, PGX::Spacing::SM))
		[BuildKPIStrip()];

	// EN: Scrollable Content / ES: Contenido Scrollable
	const float HPad = PGX::Spacing::MD * GetPaddingScale();

	RootContent->AddSlot()
		.FillHeight(1.0f)
		[
			SNew(SScrollBox)

			+ SScrollBox::Slot()
			.Padding(FMargin(HPad, PGX::Spacing::SM, HPad, 0.0f))
			[BuildIncidentBlock()]

			+ SScrollBox::Slot()
			.Padding(FMargin(HPad, 0.0f, HPad, 0.0f))
			[BuildSystemHealthSection()]

			+ SScrollBox::Slot()
			.Padding(FMargin(HPad, 0.0f, HPad, 0.0f))
			[BuildTelemetrySection()]

			+ SScrollBox::Slot()
			.Padding(FMargin(HPad, 0.0f, HPad, 0.0f))
			[BuildRecentEventsSection()]

			+ SScrollBox::Slot()
			.Padding(FMargin(HPad, 0.0f, HPad, PGX::Spacing::XL))
			[BuildEmptyStateSection()]
		];

	// EN: Re-populate graph data after rebuild (graphs are recreated by SAssignNew)
	// ES: Re-poblar datos de graficas tras rebuild (las graficas se recrean con SAssignNew)
	PopulateFakeGraphData();
}

// ============================================================================
// EN: Command Header / ES: Cabecera de Comando
// ============================================================================

TSharedRef<SWidget> SPGXVisualShowcaseTab::BuildCommandHeader()
{
	return SNew(SVerticalBox)

		// EN: Accent bar / ES: Barra de acento
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SPGXAccentBar)
			.Color(PGX::Semantic::Info)
		]

		// EN: Command bar content / ES: Contenido de la barra de comando
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderBackgroundColor_Lambda([this]() { return GetSurfaceRaised(); })
			.Padding(FMargin(PGX::Spacing::MD, PGX::Spacing::SM))
			[
				SNew(SHorizontalBox)

				// EN: Left — Brand + PIE status / ES: Izquierda — Marca + estado PIE
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, PGX::Spacing::MD, 0.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Brand", "PGX VISUAL SHOWCASE"))
					.Font(PGX::Font::PanelTitle())
					.ColorAndOpacity_Lambda([this]() { return FSlateColor(GetTextPrimary()); })
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, PGX::Spacing::SM, 0.0f)
				[
					SNew(SPGXHealthDot)
					.State(EPGXHealthState::Active)
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, PGX::Spacing::XL, 0.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("PIEStatus", "PIE: Running"))
					.Font(GetCaptionFont())
					.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
				]

				// EN: Center — Version + Env Badge / ES: Centro — Version + Badge de Entorno
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, PGX::Spacing::MD, 0.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Version", "v0.4.0"))
					.Font(PGX::Font::Mono())
					.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, PGX::Spacing::XL, 0.0f)
				[
					SNew(SPGXStatusBadge)
					.Shape(EPGXBadgeShape::Pill)
					.Color(PGX::Semantic::Warn)
					.Label(LOCTEXT("EnvBadge", "DEV"))
				]

				// EN: Spacer / ES: Espaciador
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)

				// EN: Right — Theme Toggle / ES: Derecha — Toggle de Theme
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, PGX::Spacing::SM, 0.0f)
				[
					SNew(SButton)
					.ButtonStyle(FAppStyle::Get(), "SimpleButton")
					.OnClicked_Lambda([this]()
					{
						CurrentTheme = (CurrentTheme == EPGXShowcaseTheme::Obsidian)
							? EPGXShowcaseTheme::ValidationLight
							: EPGXShowcaseTheme::Obsidian;
						RebuildContent();
						return FReply::Handled();
					})
					[
						SNew(SBorder)
						.BorderBackgroundColor(PGX::Border::Subtle)
						.Padding(FMargin(PGX::Spacing::MD, PGX::Spacing::XS))
						[
							SNew(STextBlock)
							.Text_Lambda([this]()
							{
								return CurrentTheme == EPGXShowcaseTheme::Obsidian
									? LOCTEXT("ThemeObsidian", "Theme: Obsidian")
									: LOCTEXT("ThemeLight", "Theme: Light");
							})
							.Font(PGX::Font::Badge())
							.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
						]
					]
				]

				// EN: Density Toggle / ES: Toggle de Densidad
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SButton)
					.ButtonStyle(FAppStyle::Get(), "SimpleButton")
					.OnClicked_Lambda([this]()
					{
						CurrentDensity = (CurrentDensity == EPGXShowcaseDensity::Default)
							? EPGXShowcaseDensity::Compact
							: EPGXShowcaseDensity::Default;
						RebuildContent();
						return FReply::Handled();
					})
					[
						SNew(SBorder)
						.BorderBackgroundColor(PGX::Border::Subtle)
						.Padding(FMargin(PGX::Spacing::MD, PGX::Spacing::XS))
						[
							SNew(STextBlock)
							.Text_Lambda([this]()
							{
								return CurrentDensity == EPGXShowcaseDensity::Default
									? LOCTEXT("DensityDefault", "Density: Default")
									: LOCTEXT("DensityCompact", "Density: Compact");
							})
							.Font(PGX::Font::Badge())
							.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
						]
					]
				]
			]
		];
}

// ============================================================================
// EN: Health Summary Line / ES: Linea Resumen de Salud
// ============================================================================

TSharedRef<SWidget> SPGXVisualShowcaseTab::BuildHealthSummaryLine()
{
	const int32 OKCount = CountByHealth(2);
	const int32 WarnCount = CountByHealth(1);
	const int32 ErrorCount = CountByHealth(0);
	const bool bHasCritical = ErrorCount > 0;

	return SNew(SBorder)
		.BorderBackgroundColor(bHasCritical ? PGX::Semantic::ErrorBg : GetSurfaceRaised())
		.Padding(FMargin(PGX::Spacing::MD * GetPaddingScale(), PGX::Spacing::SM * GetPaddingScale()))
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, PGX::Spacing::MD, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("HealthLabel", "SYSTEM HEALTH:"))
				.Font(PGX::Font::Badge())
				.ColorAndOpacity_Lambda([this]() { return FSlateColor(GetTextPrimary()); })
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, PGX::Spacing::LG, 0.0f)
			[
				SNew(STextBlock)
				.Text(FText::Format(LOCTEXT("OKCount", "{0} OK"), FText::AsNumber(OKCount)))
				.Font(PGX::Font::Badge())
				.ColorAndOpacity(FSlateColor(PGX::Semantic::Good))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, PGX::Spacing::LG, 0.0f)
			[
				SNew(STextBlock)
				.Text(FText::Format(LOCTEXT("WarnCount", "{0} WARNING"), FText::AsNumber(WarnCount)))
				.Font(PGX::Font::Badge())
				.ColorAndOpacity(FSlateColor(PGX::Semantic::Warn))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::Format(LOCTEXT("CritCount", "{0} CRITICAL"), FText::AsNumber(ErrorCount)))
				.Font(PGX::Font::Badge())
				.ColorAndOpacity(FSlateColor(PGX::Semantic::Error))
			]
		];
}

// ============================================================================
// EN: Incident Block / ES: Bloque de Incidentes
// ============================================================================

TSharedRef<SWidget> SPGXVisualShowcaseTab::BuildIncidentBlock()
{
	if (Incidents.Num() == 0)
	{
		return SNew(SBox);
	}

	TSharedRef<SVerticalBox> Container = SNew(SVerticalBox);

	Container->AddSlot()
		.AutoHeight()
		.Padding(0.0f, PGX::Spacing::MD * GetPaddingScale(), 0.0f, PGX::Spacing::SM)
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("IncidentHeader", "ACTIVE INCIDENTS"))
			.AccentColor(PGX::Semantic::Error)
		];

	for (const FShowcaseIncident& Inc : Incidents)
	{
		Container->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, PGX::Spacing::XS)
			[
				SNew(SBorder)
				.BorderBackgroundColor(PGX::Semantic::ErrorBg)
				.Padding(FMargin(PGX::Spacing::MD * GetPaddingScale(), PGX::Spacing::SM * GetPaddingScale()))
				[
					SNew(SHorizontalBox)

					// EN: Left stripe / ES: Stripe izquierdo
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0.0f, 0.0f, PGX::Spacing::MD, 0.0f)
					[
						SNew(SBox)
						.WidthOverride(3.0f)
						[
							SNew(SBorder)
							.BorderBackgroundColor(Inc.Color)
						]
					]

					// EN: Incident info / ES: Info del incidente
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.0f, 0.0f, PGX::Spacing::MD, 0.0f)
					[
						SNew(SPGXHealthDot)
						.State(EPGXHealthState::Error)
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.0f, 0.0f, PGX::Spacing::MD, 0.0f)
					[
						SNew(STextBlock)
						.Text(Inc.System)
						.Font(PGX::Font::SubHeader())
						.ColorAndOpacity(FSlateColor(PGX::Semantic::Error))
					]

					// EN: Description — AutoWidth to keep CTA in proximity
					// ES: Descripcion — AutoWidth para mantener CTA en proximidad
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(Inc.Description)
						.Font(GetBodyFont())
						.ColorAndOpacity_Lambda([this]() { return FSlateColor(GetTextPrimary()); })
					]

					// EN: CTA button — immediately after description
					// ES: Boton CTA — inmediatamente despues de descripcion
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(PGX::Spacing::LG, 0.0f, 0.0f, 0.0f)
					[
						SNew(SButton)
						.ButtonStyle(FAppStyle::Get(), "SimpleButton")
						[
							SNew(STextBlock)
							.Text(LOCTEXT("OpenPanel", "Open Panel"))
							.Font(PGX::Font::Badge())
							.ColorAndOpacity(FSlateColor(PGX::Semantic::Info))
						]
					]

					// EN: Trailing spacer / ES: Espaciador final
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
				]
			];
	}

	return Container;
}

// ============================================================================
// EN: KPI Strip / ES: Franja KPI
// ============================================================================

TSharedRef<SWidget> SPGXVisualShowcaseTab::BuildKPIStrip()
{
	const float Pad = PGX::Spacing::SM * GetPaddingScale();

	// EN: Build trend arrow text / ES: Construir texto de flecha de tendencia
	auto TrendArrow = [](const FText& Value, const FText& ArrowText, const FLinearColor& ArrowColor) -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Bottom)
			[
				SNew(STextBlock)
				.Text(Value)
				.Font(PGX::Font::KPIValue())
				.ColorAndOpacity(FSlateColor(PGX::Text::Primary))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Bottom)
			.Padding(PGX::Spacing::XS, 0.0f, 0.0f, 2.0f)
			[
				SNew(STextBlock)
				.Text(ArrowText)
				.Font(PGX::Font::Badge())
				.ColorAndOpacity(FSlateColor(ArrowColor))
			];
	};

	// EN: Icon helper — 24x24 secondary SVG / ES: Helper de icono — SVG 24x24 secundario
	auto MakeIcon = [](const FName& BrushName) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.WidthOverride(24.0f)
			.HeightOverride(24.0f)
			[
				SNew(SImage)
				.Image(FPGXEditorStyle::Get().GetBrush(BrushName))
				.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
			];
	};

	return SNew(SBorder)
		.BorderBackgroundColor_Lambda([this]() { return GetSurfaceRaised(); })
		.Padding(FMargin(PGX::Spacing::MD * GetPaddingScale(), PGX::Spacing::SM * GetPaddingScale()))
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(0.0f, 0.0f, Pad, 0.0f)
			[
				SNew(SPGXKPIChip)
				.Label(LOCTEXT("KPIPlugins", "PLUGINS"))
				.Icon()
				[
					MakeIcon(FName("PGXEditor.Icon.Hub"))
				]
				.ValueWidget()
				[
					TrendArrow(
						FText::FromString(TEXT("23")),
						FText::FromString(TEXT("\x2192")),  // →
						PGX::Text::Muted)
				]
				.AccentColor(PGX::Semantic::Info)
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(Pad, 0.0f)
			[
				SNew(SPGXKPIChip)
				.Label(LOCTEXT("KPISystems", "SYSTEMS"))
				.Icon()
				[
					MakeIcon(FName("PGXEditor.Icon.SystemObserver"))
				]
				.ValueWidget()
				[
					TrendArrow(
						FText::FromString(TEXT("13")),
						FText::FromString(TEXT("\x2191")),  // ↑
						PGX::Semantic::Good)
				]
				.AccentColor(PGX::Semantic::Good)
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(Pad, 0.0f)
			[
				SNew(SPGXKPIChip)
				.Label(LOCTEXT("KPIErrors", "ERRORS"))
				.Icon()
				[
					MakeIcon(FName("PGXEditor.Icon.Validate"))
				]
				.Value(FText::FromString(TEXT("0")))
				.AccentColor(PGX::Semantic::Good)
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(Pad, 0.0f, 0.0f, 0.0f)
			[
				SNew(SPGXKPIChip)
				.Label(LOCTEXT("KPIUptime", "UPTIME"))
				.Icon()
				[
					MakeIcon(FName("PGXEditor.Icon.PlatformHealth"))
				]
				.Value(FText::FromString(TEXT("03:24:17")))
				.AccentColor(PGX::Semantic::Neutral)
			]
		];
}

// ============================================================================
// EN: System Health / ES: Salud del Sistema
// ============================================================================

TSharedRef<SWidget> SPGXVisualShowcaseTab::BuildSystemHealthSection()
{
	TSharedRef<SVerticalBox> Container = SNew(SVerticalBox);

	Container->AddSlot()
		.AutoHeight()
		.Padding(0.0f, PGX::Spacing::MD * GetPaddingScale(), 0.0f, 0.0f)
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("SectionHealth", "SYSTEM HEALTH"))
			.AccentColor(PGX::Semantic::Info)
			.RightContent()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()
				[
					BuildFilterButton(LOCTEXT("FilterAll", "All"),
						CurrentHealthFilter == EPGXHealthFilter::All,
						FOnClicked::CreateLambda([this]() { CurrentHealthFilter = EPGXHealthFilter::All; RebuildContent(); return FReply::Handled(); }))
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					BuildFilterButton(LOCTEXT("FilterActive", "Active"),
						CurrentHealthFilter == EPGXHealthFilter::Active,
						FOnClicked::CreateLambda([this]() { CurrentHealthFilter = EPGXHealthFilter::Active; RebuildContent(); return FReply::Handled(); }))
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					BuildFilterButton(LOCTEXT("FilterWarning", "Warning"),
						CurrentHealthFilter == EPGXHealthFilter::Warning,
						FOnClicked::CreateLambda([this]() { CurrentHealthFilter = EPGXHealthFilter::Warning; RebuildContent(); return FReply::Handled(); }))
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					BuildFilterButton(LOCTEXT("FilterError", "Error"),
						CurrentHealthFilter == EPGXHealthFilter::Error,
						FOnClicked::CreateLambda([this]() { CurrentHealthFilter = EPGXHealthFilter::Error; RebuildContent(); return FReply::Handled(); }))
				]
			]
		];

	// EN: Sort by severity (Error first) then build rows / ES: Ordenar por severidad (Error primero) luego construir filas
	TArray<TSharedPtr<FShowcaseSystemEntry>> Sorted = SystemEntries;
	Sorted.Sort([](const TSharedPtr<FShowcaseSystemEntry>& A, const TSharedPtr<FShowcaseSystemEntry>& B)
	{
		return A->SeverityOrder < B->SeverityOrder;
	});

	int32 VisibleIndex = 0;
	for (const TSharedPtr<FShowcaseSystemEntry>& Entry : Sorted)
	{
		// EN: Apply filter / ES: Aplicar filtro
		if (CurrentHealthFilter == EPGXHealthFilter::Active && Entry->SeverityOrder != 2) { continue; }
		if (CurrentHealthFilter == EPGXHealthFilter::Warning && Entry->SeverityOrder != 1) { continue; }
		if (CurrentHealthFilter == EPGXHealthFilter::Error && Entry->SeverityOrder != 0) { continue; }

		const EPGXHealthState DotState =
			Entry->SeverityOrder == 0 ? EPGXHealthState::Error :
			Entry->SeverityOrder == 1 ? EPGXHealthState::Warning :
			EPGXHealthState::Active;

		Container->AddSlot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(GetRowHeight())
				[
					SNew(SPGXTableRow)
					.SystemColor(Entry->Color)
					.bIsEvenRow(VisibleIndex % 2 == 0)
					.Content()
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(0.0f, 0.0f, PGX::Spacing::MD, 0.0f)
						[
							SNew(SPGXHealthDot)
							.State(DotState)
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(0.0f, 0.0f, PGX::Spacing::XL, 0.0f)
						[
							SNew(SBox)
							.WidthOverride(100.0f)
							[
								SNew(STextBlock)
								.Text(Entry->Name)
								.Font(GetBodyFont())
								.ColorAndOpacity_Lambda([this]() { return FSlateColor(GetTextPrimary()); })
							]
						]

						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.VAlign(VAlign_Center)
						[
							SNew(SPGXBudgetBar)
							.Label(LOCTEXT("Budget", "Budget:"))
							.Value(Entry->BudgetValue)
							.MaxValue(Entry->BudgetMax)
							.bUnlimited(Entry->bUnlimited)
						]
					]
				]
			];

		++VisibleIndex;
	}

	return Container;
}

// ============================================================================
// EN: Telemetry / ES: Telemetria
// ============================================================================

TSharedRef<SWidget> SPGXVisualShowcaseTab::BuildTelemetrySection()
{
	// EN: Build range buttons / ES: Construir botones de rango
	auto BuildRangeButtons = [this]() -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				BuildFilterButton(LOCTEXT("Range30s", "30s"),
					CurrentTelemetryRange == EPGXTelemetryRange::Seconds30,
					FOnClicked::CreateLambda([this]() { CurrentTelemetryRange = EPGXTelemetryRange::Seconds30; return FReply::Handled(); }))
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				BuildFilterButton(LOCTEXT("Range2m", "2m"),
					CurrentTelemetryRange == EPGXTelemetryRange::Minutes2,
					FOnClicked::CreateLambda([this]() { CurrentTelemetryRange = EPGXTelemetryRange::Minutes2; return FReply::Handled(); }))
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				BuildFilterButton(LOCTEXT("Range5m", "5m"),
					CurrentTelemetryRange == EPGXTelemetryRange::Minutes5,
					FOnClicked::CreateLambda([this]() { CurrentTelemetryRange = EPGXTelemetryRange::Minutes5; return FReply::Handled(); }))
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				BuildFilterButton(LOCTEXT("RangeLive", "Live"),
					CurrentTelemetryRange == EPGXTelemetryRange::Live,
					FOnClicked::CreateLambda([this]() { CurrentTelemetryRange = EPGXTelemetryRange::Live; return FReply::Handled(); }))
			];
	};

	// EN: Build legend / ES: Construir leyenda
	auto BuildLegend = [this](const TSharedPtr<SPGXTelemetryGraph>& Graph, const FLinearColor& MainColor) -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, PGX::Spacing::LG, 0.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, PGX::Spacing::XS, 0)
				[
					SNew(SBox).WidthOverride(10.0f).HeightOverride(2.0f)
					[SNew(SBorder).BorderBackgroundColor(MainColor)]
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text_Lambda([Graph]() {
						return FText::Format(LOCTEXT("LegendCurrent", "Current: {0}"),
							FText::AsNumber(FMath::RoundToInt(Graph.IsValid() ? Graph->GetCurrent() : 0.0f)));
					})
					.Font(GetCaptionFont())
					.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, PGX::Spacing::LG, 0.0f)
			[
				SNew(STextBlock)
				.Text_Lambda([Graph]() {
					return FText::Format(LOCTEXT("LegendAvg", "Avg: {0}"),
						FText::AsNumber(FMath::RoundToInt(Graph.IsValid() ? Graph->GetAverage() : 0.0f)));
				})
				.Font(GetCaptionFont())
				.ColorAndOpacity(FSlateColor(PGX::Semantic::Warn))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text_Lambda([Graph]() {
					return FText::Format(LOCTEXT("LegendPeak", "Peak: {0}"),
						FText::AsNumber(FMath::RoundToInt(Graph.IsValid() ? Graph->GetPeak() : 0.0f)));
				})
				.Font(GetCaptionFont())
				.ColorAndOpacity(FSlateColor(PGX::Semantic::Error))
			];
	};

	const float GraphHeight = CurrentDensity == EPGXShowcaseDensity::Compact ? 60.0f : 80.0f;

	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, PGX::Spacing::MD * GetPaddingScale(), 0.0f, 0.0f)
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("SectionTelemetry", "TELEMETRY"))
			.AccentColor(PGX::System::MGOS)
			.RightContent()
			[
				BuildRangeButtons()
			]
		]

		// EN: GC Cycle Time label / ES: Etiqueta Tiempo de Ciclo GC
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, PGX::Spacing::LG, 0.0f, PGX::Spacing::SM)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("GCLabel", "GC Cycle Time (ms)"))
			.Font(PGX::Font::SubHeader())
			.ColorAndOpacity_Lambda([this]() { return FSlateColor(GetTextPrimary()); })
		]

		// EN: GC graph / ES: Grafica GC
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)
			.HeightOverride(GraphHeight)
			[
				SAssignNew(GCGraph, SPGXTelemetryGraph)
				.BufferSize(128)
				.LineColor(PGX::System::MGOS)
				.MinValue(0.0f)
				.MaxValue(50.0f)
				.LineThickness(1.5f)
				.BackgroundColor(PGX::Surface::Void)
				.bShowGrid(true)
				.bShowAvgLine(true)
				.bShowPeakMarkers(true)
				.bShowLabels(true)
				.AvgLineColor(PGX::Semantic::Warn)
			]
		]

		// EN: GC legend / ES: Leyenda GC
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, PGX::Spacing::SM, 0.0f, PGX::Spacing::XXL)
		[BuildLegend(GCGraph, PGX::System::MGOS)]

		// EN: Frame Rate label / ES: Etiqueta Tasa de Frames
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, PGX::Spacing::SM)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("FPSLabel", "Frame Rate"))
			.Font(PGX::Font::SubHeader())
			.ColorAndOpacity_Lambda([this]() { return FSlateColor(GetTextPrimary()); })
		]

		// EN: FPS graph / ES: Grafica FPS
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)
			.HeightOverride(GraphHeight)
			[
				SAssignNew(FPSGraph, SPGXTelemetryGraph)
				.BufferSize(128)
				.LineColor(PGX::Semantic::Good)
				.MinValue(0.0f)
				.MaxValue(120.0f)
				.LineThickness(1.5f)
				.BackgroundColor(PGX::Surface::Void)
				.bShowGrid(true)
				.bShowAvgLine(true)
				.bShowPeakMarkers(false)
				.bShowLabels(true)
				.AvgLineColor(PGX::Semantic::Warn)
			]
		]

		// EN: FPS legend / ES: Leyenda FPS
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, PGX::Spacing::SM, 0.0f, 0.0f)
		[BuildLegend(FPSGraph, PGX::Semantic::Good)];
}

// ============================================================================
// EN: Recent Events / ES: Eventos Recientes
// ============================================================================

TSharedRef<SWidget> SPGXVisualShowcaseTab::BuildRecentEventsSection()
{
	// EN: Fixed EVENT width (200px) prevents extreme horizontal spread — proximity principle
	// ES: Ancho fijo de EVENT (200px) evita dispersion horizontal extrema — principio de proximidad
	TArray<FPGXColumnDef> Columns;
	Columns.Add({ FName(TEXT("Id")),     LOCTEXT("ColId", "#"),         30.0f,  HAlign_Right });
	Columns.Add({ FName(TEXT("System")), LOCTEXT("ColSystem", "SYSTEM"), 80.0f,  HAlign_Left });
	Columns.Add({ FName(TEXT("Event")),  LOCTEXT("ColEvent", "EVENT"),   200.0f, HAlign_Left });
	Columns.Add({ FName(TEXT("Time")),   LOCTEXT("ColTime", "TIME"),     60.0f,  HAlign_Right });
	Columns.Add({ FName(TEXT("Status")), LOCTEXT("ColStatus", "STATUS"), 80.0f,  HAlign_Center });

	TSharedRef<SVerticalBox> Container = SNew(SVerticalBox);

	Container->AddSlot()
		.AutoHeight()
		.Padding(0.0f, PGX::Spacing::MD * GetPaddingScale(), 0.0f, 0.0f)
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("SectionEvents", "RECENT EVENTS"))
			.AccentColor(PGX::System::EventHandler)
			.RightContent()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()
				[
					BuildFilterButton(LOCTEXT("EvtFilterAll", "All"),
						CurrentEventFilter == EPGXEventFilter::All,
						FOnClicked::CreateLambda([this]() { CurrentEventFilter = EPGXEventFilter::All; RebuildContent(); return FReply::Handled(); }))
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					BuildFilterButton(LOCTEXT("EvtFilterOK", "OK"),
						CurrentEventFilter == EPGXEventFilter::OK,
						FOnClicked::CreateLambda([this]() { CurrentEventFilter = EPGXEventFilter::OK; RebuildContent(); return FReply::Handled(); }))
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					BuildFilterButton(LOCTEXT("EvtFilterWarn", "Warning"),
						CurrentEventFilter == EPGXEventFilter::Warning,
						FOnClicked::CreateLambda([this]() { CurrentEventFilter = EPGXEventFilter::Warning; RebuildContent(); return FReply::Handled(); }))
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					BuildFilterButton(LOCTEXT("EvtFilterErr", "Error"),
						CurrentEventFilter == EPGXEventFilter::Error,
						FOnClicked::CreateLambda([this]() { CurrentEventFilter = EPGXEventFilter::Error; RebuildContent(); return FReply::Handled(); }))
				]
			]
		];

	Container->AddSlot()
		.AutoHeight()
		[
			SNew(SPGXDataTableHeader)
			.Columns(Columns)
		];

	int32 VisibleIndex = 0;
	for (const TSharedPtr<FShowcaseEventEntry>& Evt : EventEntries)
	{
		// EN: Apply event filter / ES: Aplicar filtro de eventos
		if (CurrentEventFilter == EPGXEventFilter::OK && Evt->SeverityOrder != 2) { continue; }
		if (CurrentEventFilter == EPGXEventFilter::Warning && Evt->SeverityOrder != 1) { continue; }
		if (CurrentEventFilter == EPGXEventFilter::Error && Evt->SeverityOrder != 0) { continue; }

		// EN: Error rows get subtle background / ES: Filas de error obtienen fondo sutil
		const bool bIsError = Evt->SeverityOrder == 0;

		Container->AddSlot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(GetRowHeight())
				[
					SNew(SBorder)
					.BorderBackgroundColor(bIsError ? PGX::Semantic::ErrorBg : FLinearColor::Transparent)
					.Padding(0)
					[
						SNew(SPGXTableRow)
						.SystemColor(Evt->StatusColor)
						.bIsEvenRow(VisibleIndex % 2 == 0)
						.Content()
						[
							SNew(SHorizontalBox)

							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SBox)
								.WidthOverride(30.0f)
								[
									SNew(STextBlock)
									.Text(FText::AsNumber(Evt->Id))
									.Font(PGX::Font::Mono())
									.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
									.Justification(ETextJustify::Right)
								]
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(PGX::Spacing::MD, 0.0f, 0.0f, 0.0f)
							[
								SNew(SBox)
								.WidthOverride(80.0f)
								[
									SNew(STextBlock)
									.Text(Evt->System)
									.Font(GetBodyFont())
									.ColorAndOpacity_Lambda([this]() { return FSlateColor(GetTextPrimary()); })
								]
							]

							// EN: EVENT — fixed 200px to keep TIME/STATUS in proximity
							// ES: EVENT — fijo 200px para mantener TIME/STATUS en proximidad
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(PGX::Spacing::MD, 0.0f, 0.0f, 0.0f)
							[
								SNew(SBox)
								.WidthOverride(200.0f)
								[
									SNew(STextBlock)
									.Text(Evt->Event)
									.Font(GetBodyFont())
									.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
								]
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(PGX::Spacing::MD, 0.0f, 0.0f, 0.0f)
							[
								SNew(SBox)
								.WidthOverride(60.0f)
								[
									SNew(STextBlock)
									.Text(Evt->Time)
									.Font(PGX::Font::Mono())
									.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
									.Justification(ETextJustify::Right)
								]
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(PGX::Spacing::MD, 0.0f, 0.0f, 0.0f)
							[
								SNew(SBox)
								.WidthOverride(80.0f)
								.HAlign(HAlign_Center)
								[
									SNew(SPGXStatusBadge)
									.Shape(EPGXBadgeShape::Pill)
									.Color(Evt->StatusColor)
									.Label(Evt->Status)
								]
							]
						]
					]
				]
			];

		++VisibleIndex;
	}

	return Container;
}

// ============================================================================
// EN: Empty State / ES: Estado Vacio
// ============================================================================

TSharedRef<SWidget> SPGXVisualShowcaseTab::BuildEmptyStateSection()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, PGX::Spacing::MD * GetPaddingScale(), 0.0f, 0.0f)
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("SectionEmpty", "EMPTY STATE DEMO"))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)
			.HeightOverride(180.0f)
			[
				SNew(SPGXEmptyStateV2)
				.Message(LOCTEXT("EmptyMsg", "No hay datos disponibles"))
				.Hint(LOCTEXT("EmptyHint", "Selecciona un sistema para ver detalles"))
				.ActionLabel(LOCTEXT("EmptyAction", "Abrir Hub Real"))
			]
		];
}

// ============================================================================
// EN: Footer Bar / ES: Barra de Pie
// ============================================================================

TSharedRef<SWidget> SPGXVisualShowcaseTab::BuildFooterBar()
{
	return SNew(SPGXFooterBar)
		.StatusText(LOCTEXT("FooterStatus", "PGX v0.4.0 | PC DEV"))
		.LeftContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, PGX::Spacing::MD, 0.0f)
			[
				SNew(SPGXHealthDot)
				.State(EPGXHealthState::Active)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					return FText::Format(LOCTEXT("FooterDensity", "{0} | {1}"),
						CurrentDensity == EPGXShowcaseDensity::Default
							? LOCTEXT("DDefault", "Default") : LOCTEXT("DCompact", "Compact"),
						CurrentTheme == EPGXShowcaseTheme::Obsidian
							? LOCTEXT("TObsidian", "Obsidian") : LOCTEXT("TLight", "Light"));
				})
				.Font(PGX::Font::Caption())
				.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
			]
		]
		.RightContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("FooterMode", "Showcase Mode"))
			.Font(PGX::Font::Caption())
			.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
		];
}

// ============================================================================
// EN: Fake Data / ES: Datos Falsos
// ============================================================================

void SPGXVisualShowcaseTab::PopulateFakeData()
{
	auto MakeSystem = [this](const FText& Name, const FLinearColor& Color,
		const FText& Status, const FLinearColor& StatusColor,
		float Value, float Max, bool bUnlimited, int32 Severity)
	{
		TSharedPtr<FShowcaseSystemEntry> E = MakeShared<FShowcaseSystemEntry>();
		E->Name = Name;
		E->Color = Color;
		E->Status = Status;
		E->StatusColor = StatusColor;
		E->BudgetValue = Value;
		E->BudgetMax = Max;
		E->bUnlimited = bUnlimited;
		E->SeverityOrder = Severity;
		SystemEntries.Add(E);
	};

	// EN: Severity: 0=Error, 1=Warning, 2=Active
	MakeSystem(LOCTEXT("SysMGOS",     "MGOS"),     PGX::System::MGOS,     LOCTEXT("StatusError",   "ERROR"),   PGX::Semantic::Error, 98.0f, 100.0f, false, 0);
	MakeSystem(LOCTEXT("SysSave",     "Save"),     PGX::System::Save,     LOCTEXT("StatusWarn",    "WARNING"), PGX::Semantic::Warn,  91.0f, 100.0f, false, 1);
	MakeSystem(LOCTEXT("SysProfile",  "Profile"),  PGX::System::Profile,  LOCTEXT("StatusActive",  "ACTIVE"),  PGX::Semantic::Good,  82.0f, 100.0f, false, 2);
	MakeSystem(LOCTEXT("SysGameFlow", "GameFlow"), PGX::System::GameFlow, LOCTEXT("StatusActive2", "ACTIVE"),  PGX::Semantic::Good,  0.0f,  100.0f, true,  2);
	MakeSystem(LOCTEXT("SysAudio",    "Audio"),    PGX::System::Audio,    LOCTEXT("StatusActive3", "ACTIVE"),  PGX::Semantic::Good,  60.0f, 100.0f, false, 2);

	// EN: Incidents (derived from error/warning entries)
	Incidents.Add({ LOCTEXT("IncMGOS", "MGOS"), LOCTEXT("IncMGOSDesc", "Memory Pressure (98%)"), PGX::Semantic::Error });
	Incidents.Add({ LOCTEXT("IncSave", "Save"), LOCTEXT("IncSaveDesc", "Budget Warning (91%)"), PGX::Semantic::Warn });

	// EN: Event entries with severity
	auto MakeEvent = [this](int32 Id, const FText& System, const FText& Event,
		const FText& Time, const FText& Status, const FLinearColor& Color, int32 Severity)
	{
		TSharedPtr<FShowcaseEventEntry> E = MakeShared<FShowcaseEventEntry>();
		E->Id = Id;
		E->System = System;
		E->Event = Event;
		E->Time = Time;
		E->Status = Status;
		E->StatusColor = Color;
		E->SeverityOrder = Severity;
		EventEntries.Add(E);
	};

	MakeEvent(1, LOCTEXT("EvtSave",     "Save"),     LOCTEXT("EvtAutoSave",  "AutoSave Complete"),  LOCTEXT("Evt2s",  "2s ago"),  LOCTEXT("EvtOK",      "OK"),      PGX::Semantic::Good,  2);
	MakeEvent(2, LOCTEXT("EvtGameFlow", "GameFlow"), LOCTEXT("EvtState",     "State: InGame"),      LOCTEXT("Evt5s",  "5s ago"),  LOCTEXT("EvtActive",  "ACTIVE"),  PGX::Semantic::Good,  2);
	MakeEvent(3, LOCTEXT("EvtAudio",    "Audio"),    LOCTEXT("EvtOverflow",  "Channel Overflow"),   LOCTEXT("Evt12s", "12s ago"), LOCTEXT("EvtWarn",    "WARNING"), PGX::Semantic::Warn,  1);
	MakeEvent(4, LOCTEXT("EvtMGOS",     "MGOS"),     LOCTEXT("EvtGCSpike",   "GC Spike 45ms"),      LOCTEXT("Evt18s", "18s ago"), LOCTEXT("EvtError",   "ERROR"),   PGX::Semantic::Error, 0);
	MakeEvent(5, LOCTEXT("EvtPSO",      "PSO"),      LOCTEXT("EvtPipeline",  "Pipeline Ready"),     LOCTEXT("Evt31s", "31s ago"), LOCTEXT("EvtOK2",     "OK"),      PGX::Semantic::Good,  2);
}

void SPGXVisualShowcaseTab::PopulateFakeGraphData()
{
	if (!GCGraph.IsValid() || !FPSGraph.IsValid())
	{
		return;
	}

	// EN: Generate fake GC cycle times (mostly low, some spikes)
	for (int32 i = 0; i < 100; ++i)
	{
		float BaseValue = 5.0f + FMath::FRandRange(-2.0f, 3.0f);
		if (i % 17 == 0) { BaseValue += FMath::FRandRange(15.0f, 35.0f); }
		GCGraph->PushValue(BaseValue);
	}

	// EN: Generate fake FPS data (mostly 60, some dips)
	for (int32 i = 0; i < 100; ++i)
	{
		float FPS = 60.0f + FMath::FRandRange(-3.0f, 3.0f);
		if (i % 23 == 0) { FPS -= FMath::FRandRange(10.0f, 25.0f); }
		FPSGraph->PushValue(FPS);
	}
}

#undef LOCTEXT_NAMESPACE
