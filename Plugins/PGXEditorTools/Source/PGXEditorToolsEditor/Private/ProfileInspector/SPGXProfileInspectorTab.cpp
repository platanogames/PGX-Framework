// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "ProfileInspector/SPGXProfileInspectorTab.h"
#include "Profile/PGXProfileSubsystem.h"
#include "Logging/PGXLogMacros.h"

#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "Utils/PGXEditorUtils.h"
#include "Style/PGXVisualTokens.h"
#include "Widgets/SPGXPremiumShell.h"
#include "Widgets/PGXButtonHelpers.h"
#include "Widgets/SPGXSectionDivider.h"
#include "Widgets/SPGXKPIChip.h"
#include "Widgets/SPGXStatusBadge.h"
#include "Style/PGXEditorStyle.h"
#include "Editor.h"
#include "Engine/GameInstance.h"

#define LOCTEXT_NAMESPACE "PGXProfileInspector"
DEFINE_LOG_CATEGORY_STATIC(LogPGXProfileInspector, Log, All);

// EN: Profile system color (Yellow) / ES: Color del sistema Profile (Amarillo)
namespace { static const FLinearColor GProfileColor = PGX::System::Profile; }

// ============================================================================
// EN: Capability definitions / ES: Definiciones de capacidades
// ============================================================================

struct FCapabilityDef
{
	const TCHAR* Label;
	int32 FieldOffset; // offset into FPGXProfileCapabilities (bool)
};

// EN: 10 capability fields in order / ES: 10 campos de capability en orden
static const FCapabilityDef GCapabilities[] = {
	{ TEXT("INI"),          0 },  // bAllowINIPersistence
	{ TEXT("SaveData"),     1 },  // bAllowSaveData
	{ TEXT("ExtWrites"),    2 },  // bAllowExternalWrites
	{ TEXT("Exports"),      3 },  // bAllowExports
	{ TEXT("Console"),      4 },  // bAllowConsoleCommands
	{ TEXT("Dangerous"),    5 },  // bAllowDangerousCommands
	{ TEXT("Profiling"),    6 },  // bAllowProfiling
	{ TEXT("Telemetry"),    7 },  // bAllowTelemetry
	{ TEXT("PersistLogs"),  8 },  // bAllowPersistentLogs
	{ TEXT("HotReload"),    9 },  // bAllowHotReload
};

// EN: Feature definitions / ES: Definiciones de features
struct FFeatureDef
{
	const TCHAR* Label;
};

static const FFeatureDef GFeatures[] = {
	{ TEXT("Nanite") },
	{ TEXT("Lumen") },
	{ TEXT("RayTracing") },
	{ TEXT("VSM") },
	{ TEXT("ForwardShading") },
	{ TEXT("MobileRenderer") },
	{ TEXT("VirtualTextures") },
};

// ============================================================================
// EN: Construction / Destruction
// ES: Construccion / Destruccion
// ============================================================================

void SPGXProfileInspectorTab::Construct(const FArguments& /*InArgs*/)
{
	PGX_LOG_INFO(LogPGXProfileInspector, TEXT("[Profile] Construct"));

	ChildSlot
	[
		// EN: Premium Shell — covers UE chrome, provides title bar + footer
		// ES: Shell Premium — cubre chrome de UE, provee title bar + footer
		SNew(SPGXPremiumShell)
		.SystemColor(PGX::System::Profile)
		.Title(LOCTEXT("PanelTitle", "Profile Inspector"))
		.Subtitle(LOCTEXT("PanelSubtitle", "Platform-aware configuration live viewer"))
		.Icon(FPGXEditorStyle::Get().GetBrush("PGXEditor.Icon.Profile"))
		.bShowFooter(true)
		.TitleRightContent()
		[
			SNew(SButton)
			.Text(LOCTEXT("Refresh", "Refresh"))
			.OnClicked(this, &SPGXProfileInspectorTab::OnRefreshClicked)
		]
		.StatusText_Lambda([this]() -> FText
		{
			if (StatusBarText.IsValid())
			{
				return StatusBarText->GetText();
			}
			return LOCTEXT("StatusDisconnected2", "Not connected");
		})
		.Content()
		[
			SNew(SVerticalBox)

			// EN: KPI chips row / ES: Fila de chips KPI
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4.0f)
			[
				SNew(SHorizontalBox)

				// EN: KPI chip: Mode / ES: Chip KPI: Modo
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4.0f, 0.0f)
				[
					SNew(SPGXKPIChip)
					.Label(LOCTEXT("KPIModeLabel", "MODE"))
					.AccentColor(PGX::System::Profile)
					.ValueWidget()
					[
						SAssignNew(KPIModeChip, STextBlock)
						.Text(LOCTEXT("KPIModeInit", "---"))
						.Font(PGX::Font::BodySmall())
						.ColorAndOpacity(PGX::Text::Secondary)
					]
				]

				// EN: KPI chip: State (colored badge) / ES: Chip KPI: Estado (badge coloreado)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4.0f, 0.0f)
				[
					SAssignNew(KPIStateBadge, SBorder)
					.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
					.BorderBackgroundColor(PGX::Semantic::Neutral)
					.Padding(FMargin(6.0f, 2.0f))
					[
						SAssignNew(KPIStateChip, STextBlock)
						.Text(LOCTEXT("KPIStateInit", "---"))
						.Font(PGX::Font::Badge())
						.ColorAndOpacity(PGX::Text::OnColor)
					]
				]

				// EN: KPI chip: Platform / ES: Chip KPI: Plataforma
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4.0f, 0.0f)
				[
					SNew(SPGXKPIChip)
					.Label(LOCTEXT("KPIPlatformLabel", "PLATFORM"))
					.AccentColor(PGX::System::Profile)
					.ValueWidget()
					[
						SAssignNew(KPIPlatformChip, STextBlock)
						.Text(LOCTEXT("KPIPlatformInit", "---"))
						.Font(PGX::Font::BodySmall())
						.ColorAndOpacity(PGX::Text::Secondary)
					]
				]
			]

			// EN: Main content area (scrollable) / ES: Area de contenido principal (scrollable)
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SScrollBox)

				+ SScrollBox::Slot().Padding(4.0f) [ BuildIdentityPanel() ]

				+ SScrollBox::Slot().Padding(4.0f) [ BuildCapabilitiesPanel() ]

				+ SScrollBox::Slot().Padding(4.0f) [ BuildPoliciesPanel() ]

				+ SScrollBox::Slot().Padding(4.0f) [ BuildBudgetsPanel() ]

				+ SScrollBox::Slot().Padding(4.0f) [ BuildFeaturesPanel() ]

				+ SScrollBox::Slot().Padding(4.0f) [ BuildSimulationPanel() ]
			]
		]
	];

	// EN: Hidden text block for footer status (read by StatusText lambda)
	// ES: Text block oculto para estado del footer (leido por lambda StatusText)
	SAssignNew(StatusBarText, STextBlock)
		.Text(LOCTEXT("StatusDisconnected", "Not connected — Start PIE to inspect Profile"));

	BindPIEDelegates();

	// EN: If PIE is already running when the panel opens
	// ES: Si PIE ya esta corriendo cuando se abre el panel
	if (GEditor && GEditor->PlayWorld)
	{
		OnPIEStarted(false);
	}

	PGX_LOG_INFO(LogPGXProfileInspector, TEXT("[Profile] Construct complete — PIE delegates bound"));
}

SPGXProfileInspectorTab::~SPGXProfileInspectorTab()
{
	PGX_LOG_INFO(LogPGXProfileInspector, TEXT("[Profile] Destructor — cleaning up delegates"));

	UnbindFromSubsystem();

	if (PIEStartedHandle.IsValid())
	{
		FEditorDelegates::PostPIEStarted.Remove(PIEStartedHandle);
	}
	if (PIEEndedHandle.IsValid())
	{
		FEditorDelegates::EndPIE.Remove(PIEEndedHandle);
	}
}

// ============================================================================
// EN: UI Build
// ES: Construccion de UI
// ============================================================================

// EN: BuildToolbar — now inlined into Construct via SPGXPanelHeader + SPGXKPIChip
// ES: BuildToolbar — ahora inline en Construct via SPGXPanelHeader + SPGXKPIChip

TSharedRef<SWidget> SPGXProfileInspectorTab::BuildIdentityPanel()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("IdentityHeader", "IDENTITY"))
			.AccentColor(PGX::System::Profile)
		]

		// EN: Project Mode / ES: Modo de Proyecto
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 2.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SBox).WidthOverride(120.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ModeLabel", "Project Mode:"))
					.Font(PGX::Font::SubHeader())
				]
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(ModeText, STextBlock)
				.Text(LOCTEXT("ModeInit", "---"))
			]
		]

		// EN: Active Targets / ES: Targets Activos
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 2.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SBox).WidthOverride(120.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("TargetsLabel", "Active Targets:"))
					.Font(PGX::Font::SubHeader())
				]
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f)
			[
				SAssignNew(TargetsText, STextBlock)
				.Text(LOCTEXT("TargetsInit", "---"))
				.AutoWrapText(true)
			]
		]

		// EN: Build Context / ES: Contexto de Build
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 2.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SBox).WidthOverride(120.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("BuildLabel", "Build Context:"))
					.Font(PGX::Font::SubHeader())
				]
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(BuildText, STextBlock)
				.Text(LOCTEXT("BuildInit", "---"))
			]
		]

		// EN: Restriction Level / ES: Nivel de Restriccion
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 2.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SBox).WidthOverride(120.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("RestrictionLabel", "Restriction:"))
					.Font(PGX::Font::SubHeader())
				]
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(RestrictionText, STextBlock)
				.Text(LOCTEXT("RestrictionInit", "---"))
			]
		];
}

// EN: Change 2 — Capabilities as visual grid with colored badges
// ES: Cambio 2 — Capacidades como grid visual con badges coloreados
TSharedRef<SWidget> SPGXProfileInspectorTab::BuildCapabilitiesPanel()
{
	TSharedRef<SWrapBox> WrapBox = SNew(SWrapBox).UseAllottedSize(true);

	for (int32 i = 0; i < NumCapabilities; ++i)
	{
		WrapBox->AddSlot()
		.Padding(2.0f)
		[
			SAssignNew(CapBadges[i], SBorder)
			.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.3f, 0.3f, 0.3f)) // Default gray
			.Padding(FMargin(8.0f, 4.0f))
			[
				SNew(STextBlock)
				.Text(FText::FromString(GCapabilities[i].Label))
				.Font(PGX::Font::Badge())
				.ColorAndOpacity(PGX::Text::OnColor)
			]
		];
	}

	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("CapabilitiesHeader", "CAPABILITIES"))
			.AccentColor(PGX::System::Profile)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 4.0f)
		[
			WrapBox
		];
}

// EN: Change 5 — Policies as structured table
// ES: Cambio 5 — Politicas como tabla estructurada
TSharedRef<SWidget> SPGXProfileInspectorTab::BuildPoliciesPanel()
{
	// EN: Helper to create a bool badge
	// ES: Helper para crear un badge booleano
	auto MakeBoolBadge = [](TSharedPtr<SBorder>& OutBadge, const FText& Label) -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SBox).WidthOverride(120.0f)
				[
					SNew(STextBlock)
					.Text(Label)
					.Font(PGX::Font::Badge())
				]
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(OutBadge, SBorder)
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.3f, 0.3f, 0.3f))
				.Padding(FMargin(8.0f, 2.0f))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("BoolInit", "---"))
					.Font(PGX::Font::CaptionBold())
					.ColorAndOpacity(PGX::Text::OnColor)
				]
			];
	};

	// EN: Helper to create a text row / ES: Helper para crear fila de texto
	auto MakeTextRow = [](TSharedPtr<STextBlock>& OutText, const FText& Label) -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SBox).WidthOverride(120.0f)
				[
					SNew(STextBlock)
					.Text(Label)
					.Font(PGX::Font::Badge())
				]
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f)
			[
				SAssignNew(OutText, STextBlock)
				.Text(LOCTEXT("PolTextInit", "---"))
				.ColorAndOpacity(PGX::Text::Secondary)
			];
	};

	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("PoliciesHeader", "POLICIES"))
			.AccentColor(PGX::System::Profile)
		]

		// EN: Sub-header: Persistence Routing / ES: Sub-encabezado: Routing de Persistencia
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 6.0f, 0.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("PersistenceRouting", "Persistence Routing"))
			.Font(PGX::Font::SubHeader())
			.ColorAndOpacity(PGX::Semantic::Warn)
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(12.0f, 2.0f) [ MakeTextRow(PolicyConfigBackend, LOCTEXT("PolConfig", "Config:")) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(12.0f, 2.0f) [ MakeTextRow(PolicyLogBackend, LOCTEXT("PolLog", "Log:")) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(12.0f, 2.0f) [ MakeTextRow(PolicySaveBackend, LOCTEXT("PolSave", "Save:")) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(12.0f, 2.0f) [ MakeTextRow(PolicyBasePath, LOCTEXT("PolPath", "Base Path:")) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(12.0f, 2.0f) [ MakeTextRow(PolicyNaming, LOCTEXT("PolNaming", "Naming:")) ]

		// EN: Sub-header: Security / ES: Sub-encabezado: Seguridad
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 6.0f, 0.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SecurityFlags", "Security"))
			.Font(PGX::Font::SubHeader())
			.ColorAndOpacity(PGX::Semantic::Error)
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(12.0f, 2.0f) [ MakeBoolBadge(PolicyEncryptionBadge, LOCTEXT("PolEncrypt", "Encryption:")) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(12.0f, 2.0f) [ MakeBoolBadge(PolicyCompressionBadge, LOCTEXT("PolCompress", "Compression:")) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(12.0f, 2.0f) [ MakeBoolBadge(PolicySensitiveBadge, LOCTEXT("PolSensitive", "Sensitive Data:")) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(12.0f, 2.0f) [ MakeBoolBadge(PolicyStripDebugBadge, LOCTEXT("PolStripDbg", "Strip Debug:")) ]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(12.0f, 4.0f)
		[
			SAssignNew(PolicyRestartText, STextBlock)
			.Text(LOCTEXT("PolRestartInit", ""))
			.ColorAndOpacity(PGX::Semantic::Warn)
		];
}

// EN: Change 3 — Budgets with progress bars
// ES: Cambio 3 — Budgets con barras de progreso
TSharedRef<SWidget> SPGXProfileInspectorTab::BuildBudgetsPanel()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("BudgetsHeader", "BUDGETS"))
			.AccentColor(PGX::System::Profile)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 4.0f)
		[
			SAssignNew(BudgetsBox, SVerticalBox)

			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("BudInit", "Waiting for profile..."))
				.ColorAndOpacity(PGX::Text::Muted)
			]
		];
}

// EN: Change 4 — Features with colored policy badges
// ES: Cambio 4 — Features con badges de policy coloreados
TSharedRef<SWidget> SPGXProfileInspectorTab::BuildFeaturesPanel()
{
	TSharedRef<SVerticalBox> FeatBox = SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("FeaturesHeader", "FEATURES"))
			.AccentColor(PGX::System::Profile)
		];

	for (int32 i = 0; i < NumFeatures; ++i)
	{
		FeatBox->AddSlot()
		.AutoHeight()
		.Padding(8.0f, 2.0f)
		[
			SNew(SHorizontalBox)

			// EN: Feature name / ES: Nombre de feature
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox).WidthOverride(140.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(GFeatures[i].Label))
					.Font(PGX::Font::SubHeader())
				]
			]

			// EN: Policy badge (colored) / ES: Badge de policy (coloreado)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SAssignNew(FeaturePolicyBadges[i], SBorder)
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderBackgroundColor(PGX::Semantic::Neutral)
				.Padding(FMargin(8.0f, 2.0f))
				[
					SAssignNew(FeaturePolicyTexts[i], STextBlock)
					.Text(LOCTEXT("FeatPolInit", "---"))
					.Font(PGX::Font::CaptionBold())
					.ColorAndOpacity(PGX::Text::OnColor)
				]
			]

			// EN: Restart indicator / ES: Indicador de restart
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(8.0f, 0.0f)
			[
				SAssignNew(FeatureRestartTexts[i], STextBlock)
				.Text(FText::GetEmpty())
				.Font(PGX::Font::Caption())
				.ColorAndOpacity(PGX::Semantic::Warn)
			]
		];
	}

	return FeatBox;
}

TSharedRef<SWidget> SPGXProfileInspectorTab::BuildSimulationPanel()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("SimulationHeader", "SIMULATION"))
			.AccentColor(PGX::System::Profile)
		]

		// EN: Simulation action buttons / ES: Botones de accion de simulacion
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 4.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("SimPC", "Sim: PC"))
				.ToolTipText(LOCTEXT("SimPCTip", "Simulate PC platform"))
				.OnClicked(this, &SPGXProfileInspectorTab::OnSimulatePlatformClicked)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("SimSwitch", "Sim: Switch"))
				.ToolTipText(LOCTEXT("SimSwitchTip", "Simulate Console_Switch platform"))
				.OnClicked_Lambda([this]()
				{
					if (BoundSubsystem.IsValid())
					{
						BoundSubsystem->SimulatePlatform(EPGXTargetPlatform::Console_Switch);
					}
					return FReply::Handled();
				})
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("SimMobile", "Sim: Mobile"))
				.ToolTipText(LOCTEXT("SimMobileTip", "Simulate Mobile_Android platform"))
				.OnClicked_Lambda([this]()
				{
					if (BoundSubsystem.IsValid())
					{
						BoundSubsystem->SimulatePlatform(EPGXTargetPlatform::Mobile_Android);
					}
					return FReply::Handled();
				})
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("SimShipping", "Sim: Shipping"))
				.ToolTipText(LOCTEXT("SimShippingTip", "Simulate Shipping build context"))
				.OnClicked(this, &SPGXProfileInspectorTab::OnSimulateBuildClicked)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("ClearSim", "Clear Simulation"))
				.ToolTipText(LOCTEXT("ClearSimTip", "Restore original profile"))
				.OnClicked(this, &SPGXProfileInspectorTab::OnClearSimulationClicked)
			]
		]

		// EN: Current simulation status / ES: Estado de simulacion actual
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 2.0f)
		[
			SAssignNew(SimulationStatusText, STextBlock)
			.Text(LOCTEXT("SimStatusInit", "Simulation: Inactive"))
			.Font(PGX::Font::BodySmall())
			.ColorAndOpacity(PGX::Text::Secondary)
		];
}

// ============================================================================
// EN: Refresh
// ES: Refrescar
// ============================================================================

void SPGXProfileInspectorTab::RefreshAllData()
{
	if (BoundSubsystem.IsValid())
	{
		RefreshFromProfile(BoundSubsystem->GetResolvedProfile());

#if WITH_EDITOR
		if (BoundSubsystem->HasSimulationOverrides())
		{
			SimulationStatusText->SetText(FText::FromString(TEXT("Simulation: ACTIVE")));
			SimulationStatusText->SetColorAndOpacity(PGX::Semantic::Warn);
		}
		else
		{
			SimulationStatusText->SetText(FText::FromString(TEXT("Simulation: Inactive")));
			SimulationStatusText->SetColorAndOpacity(PGX::Semantic::Neutral);
		}
#endif
	}
}

void SPGXProfileInspectorTab::RefreshFromProfile(const FPGXResolvedProfile& Profile)
{
	CachedProfile = Profile;

	PGX_LOG_VERBOSE(LogPGXProfileInspector, TEXT("[Profile] RefreshFromProfile — State: %s"),
		*GetStateName(Profile.State));

	// EN: KPI Chips (values only — labels are in SPGXKPIChip) / ES: Chips KPI (solo valores — labels estan en SPGXKPIChip)
	if (KPIModeChip.IsValid())
	{
		KPIModeChip->SetText(FText::FromString(GetModeName(Profile.Identity.ProjectMode)));
	}
	if (KPIStateChip.IsValid() && KPIStateBadge.IsValid())
	{
		KPIStateChip->SetText(FText::FromString(GetStateName(Profile.State)));
		KPIStateBadge->SetBorderBackgroundColor(GetStateColor(Profile.State));
	}
	if (KPIPlatformChip.IsValid())
	{
		FString PlatformStr;
		if (Profile.Identity.ActiveTargets.Num() > 0)
		{
			PlatformStr = GetPlatformName(Profile.Identity.ActiveTargets[0]);
			if (Profile.Identity.ActiveTargets.Num() > 1)
			{
				PlatformStr += FString::Printf(TEXT(" +%d"), Profile.Identity.ActiveTargets.Num() - 1);
			}
		}
		else
		{
			PlatformStr = TEXT("(none)");
		}
		KPIPlatformChip->SetText(FText::FromString(PlatformStr));
	}

	// ---- Identity ----
	ModeText->SetText(FText::FromString(GetModeName(Profile.Identity.ProjectMode)));

	FString TargetList;
	for (const EPGXTargetPlatform& T : Profile.Identity.ActiveTargets)
	{
		if (!TargetList.IsEmpty()) TargetList += TEXT(", ");
		TargetList += GetPlatformName(T);
	}
	if (TargetList.IsEmpty()) TargetList = TEXT("(none)");
	TargetsText->SetText(FText::FromString(TargetList));

	BuildText->SetText(FText::FromString(GetBuildName(Profile.Identity.BuildContext)));
	RestrictionText->SetText(FText::FromString(GetRestrictionName(Profile.Identity.RestrictionLevel)));

	// ---- Change 2: Capabilities Grid ----
	{
		const FPGXProfileCapabilities& Cap = Profile.Capabilities;
		const bool CapValues[NumCapabilities] = {
			Cap.bAllowINIPersistence, Cap.bAllowSaveData,
			Cap.bAllowExternalWrites, Cap.bAllowExports,
			Cap.bAllowConsoleCommands, Cap.bAllowDangerousCommands,
			Cap.bAllowProfiling,
			Cap.bAllowTelemetry, Cap.bAllowPersistentLogs,
			Cap.bAllowHotReload
		};

		const FLinearColor EnabledColor = PGX::Semantic::Good;   // Green
		const FLinearColor DisabledColor = PGX::Semantic::Error;  // Red

		for (int32 i = 0; i < NumCapabilities; ++i)
		{
			if (CapBadges[i].IsValid())
			{
				CapBadges[i]->SetBorderBackgroundColor(CapValues[i] ? EnabledColor : DisabledColor);
			}
		}
	}

	// ---- Change 5: Policies ----
	{
		const FPGXProfilePolicies& Pol = Profile.Policies;

		if (PolicyConfigBackend.IsValid())
			PolicyConfigBackend->SetText(FText::FromString(GetBackendName(Pol.Persistence.ConfigBackend)));
		if (PolicyLogBackend.IsValid())
			PolicyLogBackend->SetText(FText::FromString(GetBackendName(Pol.Persistence.LogBackend)));
		if (PolicySaveBackend.IsValid())
			PolicySaveBackend->SetText(FText::FromString(GetBackendName(Pol.Persistence.SaveBackend)));
		if (PolicyBasePath.IsValid())
			PolicyBasePath->SetText(FText::FromString(Pol.Persistence.LogicalBasePath));
		if (PolicyNaming.IsValid())
			PolicyNaming->SetText(FText::FromString(Pol.Persistence.NamingRule));

		const FLinearColor BoolOn = PGX::Semantic::Good;
		const FLinearColor BoolOff = PGX::Semantic::Error;

		auto UpdateBoolBadge = [&](TSharedPtr<SBorder>& Badge, bool bValue)
		{
			if (Badge.IsValid())
			{
				Badge->SetBorderBackgroundColor(bValue ? BoolOn : BoolOff);
				// EN: Update child text / ES: Actualizar texto hijo
				TSharedPtr<SWidget> Content = Badge->GetContent();
				if (TSharedPtr<STextBlock> Text = StaticCastSharedPtr<STextBlock>(Content))
				{
					Text->SetText(bValue ? LOCTEXT("BoolYes", "YES") : LOCTEXT("BoolNo", "NO"));
				}
			}
		};

		UpdateBoolBadge(PolicyEncryptionBadge, Pol.Security.bRequireEncryption);
		UpdateBoolBadge(PolicyCompressionBadge, Pol.Security.bRequireCompression);
		UpdateBoolBadge(PolicySensitiveBadge, Pol.Security.bSensitiveDataHandling);
		UpdateBoolBadge(PolicyStripDebugBadge, Pol.Security.bStripDebugStrings);

		if (PolicyRestartText.IsValid())
		{
			PolicyRestartText->SetText(Pol.bRequiresRestart
				? LOCTEXT("PolRestart", "Requires Restart: YES")
				: FText::GetEmpty());
		}
	}

	// ---- Change 3: Budgets with Progress Bars ----
	if (BudgetsBox.IsValid())
	{
		BudgetsBox->ClearChildren();

		const FPGXProfileBudgets& Bud = Profile.Budgets;

		struct FBudgetDef
		{
			const TCHAR* Label;
			int64 Value;
			int64 MaxBaseline;
			const TCHAR* Unit;
		};

		const FBudgetDef Budgets[] = {
			{ TEXT("RAM"),         Bud.RAM_MB,            32768, TEXT("MB") },
			{ TEXT("VRAM"),        Bud.VRAM_MB,           24576, TEXT("MB") },
			{ TEXT("Disk"),        Bud.Disk_MB,           65536, TEXT("MB") },
			{ TEXT("StreamPool"),  Bud.StreamingPool_MB,  4096,  TEXT("MB") },
			{ TEXT("TextureMax"),  Bud.TextureMaxDim,     8192,  TEXT("px") },
			{ TEXT("MeshTris"),    Bud.MeshMaxTris,       10000000, TEXT("") },
			{ TEXT("Shaders"),     Bud.ShaderBudget,      50000, TEXT("") },
			{ TEXT("PSO"),         Bud.PSOBudget,         100000, TEXT("") },
			{ TEXT("AudioMem"),    Bud.AudioMemory_MB,    1024,  TEXT("MB") },
		};

		for (const FBudgetDef& Def : Budgets)
		{
			const bool bUnlimited = (Def.Value == 0);
			const float Percent = bUnlimited ? 1.0f : FMath::Clamp(static_cast<float>(Def.Value) / static_cast<float>(Def.MaxBaseline), 0.0f, 1.0f);
			const FLinearColor BarColor = bUnlimited
				? PGX::Semantic::Good  // Green = unlimited
				: (Percent > 0.7f ? FLinearColor(0.2f, 0.6f, 0.8f) : FLinearColor(1.0f, 0.6f, 0.0f)); // Blue = high, Orange = low

			const FString ValueStr = bUnlimited
				? TEXT("Unlimited")
				: FString::Printf(TEXT("%lld %s"), Def.Value, Def.Unit);

			BudgetsBox->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 1.0f)
			[
				SNew(SHorizontalBox)

				// EN: Label / ES: Etiqueta
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBox).WidthOverride(100.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(Def.Label))
						.Font(PGX::Font::Badge())
					]
				]

				// EN: Value / ES: Valor
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBox).WidthOverride(110.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(ValueStr))
						.Font(PGX::Font::BodySmall())
						.ColorAndOpacity(bUnlimited ? PGX::Semantic::Good : PGX::Text::Secondary)
					]
				]

				// EN: Progress bar / ES: Barra de progreso
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				.Padding(8.0f, 0.0f)
				[
					SNew(SProgressBar)
					.Percent(Percent)
					.FillColorAndOpacity(BarColor)
				]
			];
		}
	}

	// ---- Change 4: Features with Policy Badges ----
	{
		const FPGXProfileFeatureMatrix& Feat = Profile.Features;
		const FPGXFeatureEntry* Features[NumFeatures] = {
			&Feat.Nanite, &Feat.Lumen, &Feat.RayTracing, &Feat.VSM,
			&Feat.ForwardShading, &Feat.MobileRenderer, &Feat.VirtualTextures
		};

		for (int32 i = 0; i < NumFeatures; ++i)
		{
			const FPGXFeatureEntry& Entry = *Features[i];

			if (FeaturePolicyBadges[i].IsValid())
			{
				FeaturePolicyBadges[i]->SetBorderBackgroundColor(GetFeaturePolicyColor(Entry.Policy));
			}
			if (FeaturePolicyTexts[i].IsValid())
			{
				FeaturePolicyTexts[i]->SetText(FText::FromString(GetFeaturePolicyName(Entry.Policy)));
			}
			if (FeatureRestartTexts[i].IsValid())
			{
				FeatureRestartTexts[i]->SetText(Entry.bRequiresRestart
					? LOCTEXT("FeatRestart", "(restart)")
					: FText::GetEmpty());
			}
		}
	}

	// ---- Status Bar ----
	const FLinearColor StateCol = GetStateColor(Profile.State);
	FString StatusStr = FString::Printf(TEXT("State: %s | Resolved: %s"),
		*GetStateName(Profile.State),
		*Profile.ResolvedTimestamp.ToString(TEXT("%Y-%m-%d %H:%M:%S")));
	StatusBarText->SetText(FText::FromString(StatusStr));
	StatusBarText->SetColorAndOpacity(StateCol);
}

// ============================================================================
// EN: PIE Lifecycle
// ES: Ciclo de Vida PIE
// ============================================================================

void SPGXProfileInspectorTab::BindPIEDelegates()
{
	PIEStartedHandle = FEditorDelegates::PostPIEStarted.AddSP(SharedThis(this), &SPGXProfileInspectorTab::OnPIEStarted);
	PIEEndedHandle = FEditorDelegates::EndPIE.AddSP(SharedThis(this), &SPGXProfileInspectorTab::OnPIEEnded);
	PGX_LOG_INFO(LogPGXProfileInspector, TEXT("[Profile] PIE delegates bound"));
}

void SPGXProfileInspectorTab::OnPIEStarted(bool /*bIsSimulating*/)
{
	bIsPIEActive = true;
	PGX_LOG_INFO(LogPGXProfileInspector, TEXT("[Profile] PIE started"));
	BindToSubsystem();
	RefreshAllData();

	StatusBarText->SetText(LOCTEXT("StatusConnected", "Connected to PIE — Profile live"));
	StatusBarText->SetColorAndOpacity(PGX::Semantic::Good);
}

void SPGXProfileInspectorTab::OnPIEEnded(bool /*bIsSimulating*/)
{
	bIsPIEActive = false;
	PGX_LOG_INFO(LogPGXProfileInspector, TEXT("[Profile] PIE ended — showing last snapshot"));
	UnbindFromSubsystem();

	StatusBarText->SetText(LOCTEXT("StatusRetained", "PIE ended — Showing last snapshot"));
	StatusBarText->SetColorAndOpacity(PGX::Semantic::Neutral);
}

void SPGXProfileInspectorTab::BindToSubsystem()
{
	if (!GEditor || !GEditor->GetPIEWorldContext()) return;

	UWorld* PIEWorld = GEditor->GetPIEWorldContext()->World();
	if (!PIEWorld) return;

	UGameInstance* GI = PIEWorld->GetGameInstance();
	if (!GI) return;

	UPGXProfileSubsystem* ProfileSS = GI->GetSubsystem<UPGXProfileSubsystem>();
	if (!ProfileSS) return;

	BoundSubsystem = ProfileSS;

	ProfileChangedHandle = ProfileSS->OnProfileChangedNative.AddSP(
		SharedThis(this), &SPGXProfileInspectorTab::HandleProfileChanged);

	PGX_LOG_INFO(LogPGXProfileInspector, TEXT("[Profile] Bound to subsystem"));
}

void SPGXProfileInspectorTab::UnbindFromSubsystem()
{
	if (BoundSubsystem.IsValid() && ProfileChangedHandle.IsValid())
	{
		BoundSubsystem->OnProfileChangedNative.Remove(ProfileChangedHandle);
		ProfileChangedHandle.Reset();
		PGX_LOG_INFO(LogPGXProfileInspector, TEXT("[Profile] Unbound from subsystem"));
	}
	BoundSubsystem = nullptr;
}

// ============================================================================
// EN: Delegate Callbacks
// ES: Callbacks de Delegados
// ============================================================================

void SPGXProfileInspectorTab::HandleProfileChanged(
	const FPGXResolvedProfile& /*OldProfile*/, const FPGXResolvedProfile& NewProfile)
{
	PGX_LOG_INFO(LogPGXProfileInspector, TEXT("[Profile] Profile changed — refreshing"));
	RefreshFromProfile(NewProfile);

#if WITH_EDITOR
	if (BoundSubsystem.IsValid() && BoundSubsystem->HasSimulationOverrides())
	{
		SimulationStatusText->SetText(FText::FromString(TEXT("Simulation: ACTIVE")));
		SimulationStatusText->SetColorAndOpacity(PGX::Semantic::Warn);
	}
	else
	{
		SimulationStatusText->SetText(FText::FromString(TEXT("Simulation: Inactive")));
		SimulationStatusText->SetColorAndOpacity(PGX::Semantic::Neutral);
	}
#endif
}

// ============================================================================
// EN: Actions
// ES: Acciones
// ============================================================================

FReply SPGXProfileInspectorTab::OnRefreshClicked()
{
	PGX_LOG_INFO(LogPGXProfileInspector, TEXT("[Profile] Manual refresh"));
	RefreshAllData();
	return FReply::Handled();
}

FReply SPGXProfileInspectorTab::OnSimulatePlatformClicked()
{
#if WITH_EDITOR
	if (BoundSubsystem.IsValid())
	{
		PGX_LOG_INFO(LogPGXProfileInspector, TEXT("[Profile] Simulate PC platform"));
		BoundSubsystem->SimulatePlatform(EPGXTargetPlatform::PC);
	}
#endif
	return FReply::Handled();
}

FReply SPGXProfileInspectorTab::OnSimulateBuildClicked()
{
#if WITH_EDITOR
	if (BoundSubsystem.IsValid())
	{
		PGX_LOG_INFO(LogPGXProfileInspector, TEXT("[Profile] Simulate Shipping build"));
		BoundSubsystem->SimulateBuildContext(EPGXBuildContext::Shipping);
	}
#endif
	return FReply::Handled();
}

FReply SPGXProfileInspectorTab::OnClearSimulationClicked()
{
#if WITH_EDITOR
	if (BoundSubsystem.IsValid())
	{
		PGX_LOG_INFO(LogPGXProfileInspector, TEXT("[Profile] Clear simulation overrides"));
		BoundSubsystem->ClearSimulationOverrides();
	}
#endif
	return FReply::Handled();
}

// ============================================================================
// EN: Helpers
// ES: Helpers
// ============================================================================

FString SPGXProfileInspectorTab::GetModeName(EPGXProjectMode Mode)
{
	switch (Mode)
	{
	case EPGXProjectMode::Game:       return TEXT("Game");
	case EPGXProjectMode::VR:         return TEXT("VR");
	case EPGXProjectMode::Enterprise: return TEXT("Enterprise");
	case EPGXProjectMode::Tooling:    return TEXT("Tooling");
	default:                          return TEXT("Unknown");
	}
}

FString SPGXProfileInspectorTab::GetPlatformName(EPGXTargetPlatform Platform)
{
	switch (Platform)
	{
	case EPGXTargetPlatform::PC:              return TEXT("PC");
	case EPGXTargetPlatform::Console_PS:      return TEXT("Console_PS");
	case EPGXTargetPlatform::Console_Xbox:    return TEXT("Console_Xbox");
	case EPGXTargetPlatform::Console_Switch:  return TEXT("Console_Switch");
	case EPGXTargetPlatform::Mobile_iOS:      return TEXT("Mobile_iOS");
	case EPGXTargetPlatform::Mobile_Android:  return TEXT("Mobile_Android");
	case EPGXTargetPlatform::XR:              return TEXT("XR");
	default:                                  return TEXT("Unknown");
	}
}

FString SPGXProfileInspectorTab::GetBuildName(EPGXBuildContext Context)
{
	switch (Context)
	{
	case EPGXBuildContext::Development: return TEXT("Development");
	case EPGXBuildContext::Test:        return TEXT("Test");
	case EPGXBuildContext::Shipping:    return TEXT("Shipping");
	default:                           return TEXT("Unknown");
	}
}

FString SPGXProfileInspectorTab::GetRestrictionName(EPGXRestrictionLevel Level)
{
	switch (Level)
	{
	case EPGXRestrictionLevel::Strict:  return TEXT("Strict");
	case EPGXRestrictionLevel::Relaxed: return TEXT("Relaxed");
	default:                            return TEXT("Unknown");
	}
}

FString SPGXProfileInspectorTab::GetStateName(EPGXProfileState State)
{
	switch (State)
	{
	case EPGXProfileState::Unresolved: return TEXT("Unresolved");
	case EPGXProfileState::Resolved:   return TEXT("Resolved");
	case EPGXProfileState::Changing:   return TEXT("Changing");
	case EPGXProfileState::Locked:     return TEXT("Locked");
	default:                           return TEXT("Unknown");
	}
}

FString SPGXProfileInspectorTab::GetBackendName(EPGXPersistenceBackend Backend)
{
	switch (Backend)
	{
	case EPGXPersistenceBackend::INI:      return TEXT("INI");
	case EPGXPersistenceBackend::SaveData: return TEXT("SaveData");
	case EPGXPersistenceBackend::Hybrid:   return TEXT("Hybrid");
	case EPGXPersistenceBackend::Disabled: return TEXT("Disabled");
	default:                               return TEXT("Unknown");
	}
}

FString SPGXProfileInspectorTab::GetFeaturePolicyName(EPGXFeaturePolicy Policy)
{
	switch (Policy)
	{
	case EPGXFeaturePolicy::Allowed:          return TEXT("ALLOWED");
	case EPGXFeaturePolicy::Disallowed:       return TEXT("DENIED");
	case EPGXFeaturePolicy::FallbackRequired: return TEXT("FALLBACK");
	default:                                  return TEXT("UNKNOWN");
	}
}

FLinearColor SPGXProfileInspectorTab::GetStateColor(EPGXProfileState State)
{
	switch (State)
	{
	case EPGXProfileState::Resolved: return PGX::Semantic::Good; // Green
	case EPGXProfileState::Locked:   return PGX::Semantic::Error; // Red
	case EPGXProfileState::Changing: return PGX::Semantic::Warn; // Yellow
	default:                         return PGX::Semantic::Neutral; // Gray
	}
}

FLinearColor SPGXProfileInspectorTab::GetFeaturePolicyColor(EPGXFeaturePolicy Policy)
{
	switch (Policy)
	{
	case EPGXFeaturePolicy::Allowed:          return PGX::Semantic::Good; // Green
	case EPGXFeaturePolicy::Disallowed:       return PGX::Semantic::Error; // Red
	case EPGXFeaturePolicy::FallbackRequired: return PGX::Semantic::Warn; // Yellow
	default:                                  return PGX::Semantic::Neutral; // Gray
	}
}

#undef LOCTEXT_NAMESPACE
