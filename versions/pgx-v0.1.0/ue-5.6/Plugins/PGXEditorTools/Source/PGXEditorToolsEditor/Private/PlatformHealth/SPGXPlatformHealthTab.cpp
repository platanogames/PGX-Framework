// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "PlatformHealth/SPGXPlatformHealthTab.h"
#include "Profile/PGXProfileSubsystem.h"
#include "Profile/PGXProfileDelegates.h"
#include "Profile/PGXPlatformConfig.h"
#include "Profile/PGXProjectProfileConfig.h"
#include "Profile/PGXPlatformBudgets.h"
#include "Logging/PGXLogMacros.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/SBoxPanel.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "Utils/PGXEditorUtils.h"
#include "Style/PGXVisualTokens.h"
#include "Style/PGXEditorStyle.h"
#include "Widgets/SPGXPremiumShell.h"
#include "Widgets/SPGXSectionDivider.h"
#include "Widgets/PGXButtonHelpers.h"
#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "AssetRegistry/AssetRegistryModule.h"

#define LOCTEXT_NAMESPACE "PGXPlatformHealth"
DEFINE_LOG_CATEGORY_STATIC(LogPGXPlatformHealth, Log, All);

// EN: Profile system color (Yellow) / ES: Color del sistema Profile (Amarillo)
namespace { static const FLinearColor GPlatformHealthColor = PGX::System::Profile; }

// ============================================================
// Construct / Destruct
// ============================================================

void SPGXPlatformHealthTab::Construct(const FArguments& /*InArgs*/)
{
	PGX_LOG_INFO(LogPGXPlatformHealth, TEXT("[PlatformHealth] Construct"));

	// EN: Populate platform options from the enum
	// ES: Poblar opciones de plataforma desde el enum
	const UEnum* PlatformEnum = StaticEnum<EPGXTargetPlatform>();
	if (PlatformEnum)
	{
		for (int32 i = 0; i < PlatformEnum->NumEnums() - 1; ++i)
		{
			FString Name = PlatformEnum->GetNameStringByIndex(i);
			PlatformOptions.Add(MakeShared<FString>(Name));
		}
	}

	if (PlatformOptions.Num() > 0)
	{
		SelectedPlatform = PlatformOptions[0];
	}

	BindPIEDelegates();

	ChildSlot
	[
		// EN: Premium Shell — covers UE chrome, provides title bar + footer
		// ES: Shell Premium — cubre chrome de UE, provee title bar + footer
		SNew(SPGXPremiumShell)
		.SystemColor(PGX::System::Profile)
		.Title(LOCTEXT("PanelTitle", "Platform Health"))
		.Subtitle(LOCTEXT("PanelSubtitle", "Platform-aware budget dashboard"))
		.Icon(FPGXEditorStyle::Get().GetBrush("PGXEditor.Icon.Platform"))
		.TitleRightContent()
		[
			SNew(SHorizontalBox)

			// EN: Platform selector / ES: Selector de plataforma
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PlatformLabel", "Platform:"))
				.Font(PGX::Font::BodySmall())
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(SBox)
				.WidthOverride(200.0f)
				[
					SNew(SComboBox<TSharedPtr<FString>>)
					.OptionsSource(&PlatformOptions)
					.OnSelectionChanged(this, &SPGXPlatformHealthTab::OnPlatformSelected)
					.InitiallySelectedItem(SelectedPlatform)
					.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
					{
						return SNew(STextBlock).Text(FText::FromString(*Item));
					})
					[
						SNew(STextBlock)
						.Text_Lambda([this]() -> FText
						{
							return SelectedPlatform.IsValid() ? FText::FromString(*SelectedPlatform) : LOCTEXT("NoPlatform2", "None");
						})
					]
				]
			]

			// EN: Refresh button / ES: Boton de refresco
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("Refresh", "Refresh"))
				.ToolTipText(LOCTEXT("RefreshTip", "Refresh platform health data"))
				.OnClicked(this, &SPGXPlatformHealthTab::OnRefreshClicked)
			]

			// EN: Simulate button / ES: Boton de simular
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("SimulatePlatform", "Simulate Platform"))
				.ToolTipText(LOCTEXT("SimulateTip", "Apply selected platform config via Profile subsystem (requires PIE)"))
				.IsEnabled_Lambda([this]() { return bIsPIEActive; })
				.OnClicked(this, &SPGXPlatformHealthTab::OnSimulatePlatformClicked)
			]
		]
		.bShowFooter(true)
		.FooterLeftContent()
		[
			SAssignNew(StatusText, STextBlock)
			.Text(LOCTEXT("StatusReady", "Platform Health — Waiting for platform selection"))
			.Font(PGX::Font::BodySmall())
			.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
		]
		.Content()
		[
			// EN: Scrollable content / ES: Contenido scrollable
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SAssignNew(ContentArea, SVerticalBox)
			]
		]
	];

	// EN: If PIE is already running when panel opens, bind immediately
	// ES: Si PIE ya esta corriendo cuando se abre el panel, enlazar inmediatamente
	if (GEditor && GEditor->PlayWorld)
	{
		PGX_LOG_INFO(LogPGXPlatformHealth, TEXT("[PlatformHealth] PIE already running at Construct — binding"));
		OnPIEStarted(false);
	}

	RefreshData();
}

SPGXPlatformHealthTab::~SPGXPlatformHealthTab()
{
	PGX_LOG_INFO(LogPGXPlatformHealth, TEXT("[PlatformHealth] Destructor — cleaning up"));
	if (PIEStartedHandle.IsValid())
	{
		FEditorDelegates::PostPIEStarted.Remove(PIEStartedHandle);
	}
	if (PIEEndedHandle.IsValid())
	{
		FEditorDelegates::EndPIE.Remove(PIEEndedHandle);
	}

	// EN: Clean up profile delegate / ES: Limpiar delegado de profile
	if (ProfileChangedHandle.IsValid())
	{
		if (UPGXProfileSubsystem* SS = UPGXProfileSubsystem::GetCachedInstance())
		{
			SS->OnProfileChangedNative.Remove(ProfileChangedHandle);
		}
		ProfileChangedHandle.Reset();
	}
}

// EN: BuildToolbar() — Removed: integrated into SPGXPremiumShell TitleRightContent in Construct()
// ES: BuildToolbar() — Eliminado: integrado en SPGXPremiumShell TitleRightContent en Construct()

// ============================================================
// Traits Grid
// ============================================================

TSharedRef<SWidget> SPGXPlatformHealthTab::BuildTraitsGrid(const UPGXPlatformConfig* Config)
{
	if (!Config) return SNullWidget::NullWidget;

	const auto& T = Config->Traits;

	// EN: Lambda to create a trait chip / ES: Lambda para crear un chip de trait
	auto MakeTraitChip = [](const FText& Label, bool bEnabled) -> TSharedRef<SWidget>
	{
		const FLinearColor Color = bEnabled
			? PGX::Semantic::Good   // Green — trait enabled
			: PGX::Semantic::Neutral;  // Gray — trait disabled
		return SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
			.Padding(FMargin(8.0f, 3.0f))
			[
				SNew(STextBlock)
				.Text(FText::Format(LOCTEXT("TraitFmt", "{0}: {1}"), Label,
					bEnabled ? LOCTEXT("Yes", "Yes") : LOCTEXT("No", "No")))
				.Font(PGX::Font::BodySmall())
				.ColorAndOpacity(FSlateColor(Color))
			];
	};

	return SNew(SWrapBox)
		.UseAllottedSize(true)
		+ SWrapBox::Slot().Padding(2.0f)[ MakeTraitChip(LOCTEXT("TCert", "Cert Compliance"), T.bRequiresCertCompliance) ]
		+ SWrapBox::Slot().Padding(2.0f)[ MakeTraitChip(LOCTEXT("TStorage", "Limited Storage"), T.bLimitedStorage) ]
		+ SWrapBox::Slot().Padding(2.0f)[ MakeTraitChip(LOCTEXT("TVTex", "Virtual Textures"), T.bSupportsVirtualTextures) ]
		+ SWrapBox::Slot().Padding(2.0f)[ MakeTraitChip(LOCTEXT("TRT", "Ray Tracing"), T.bSupportsRayTracing) ]
		+ SWrapBox::Slot().Padding(2.0f)[ MakeTraitChip(LOCTEXT("TAsync", "Async I/O"), T.bAsyncIONative) ]
		+ SWrapBox::Slot().Padding(2.0f)[ MakeTraitChip(LOCTEXT("TSave", "Custom Save Paths"), T.bCustomSavePaths) ];
}

// ============================================================
// Global Budgets Section
// ============================================================

TSharedRef<SWidget> SPGXPlatformHealthTab::BuildGlobalBudgetsSection()
{
	const UPGXPlatformConfig* Config = GetSelectedPlatformConfig();
	if (!Config)
	{
		return SNew(STextBlock)
			.Text(LOCTEXT("NoPlatformConfig", "No platform config found for selected platform."))
			.ColorAndOpacity(FSlateColor(PGX::Semantic::Warn));
	}

	const auto& G = Config->GlobalBudgets;

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(2.0f)
		[ BuildBudgetRow(LOCTEXT("RAM", "RAM (MB)"), static_cast<int32>(G.RAM_MB)) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(2.0f)
		[ BuildBudgetRow(LOCTEXT("VRAM", "VRAM (MB)"), static_cast<int32>(G.VRAM_MB)) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(2.0f)
		[ BuildBudgetRow(LOCTEXT("Disk", "Disk (MB)"), static_cast<int32>(G.Disk_MB)) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(2.0f)
		[ BuildBudgetRow(LOCTEXT("StreamingPool", "Streaming Pool (MB)"), static_cast<int32>(G.StreamingPool_MB)) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(2.0f)
		[ BuildBudgetRow(LOCTEXT("TextureMaxDim", "Texture Max Dim"), G.TextureMaxDim) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(2.0f)
		[ BuildBudgetRow(LOCTEXT("MeshMaxTris", "Mesh Max Tris"), G.MeshMaxTris) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(2.0f)
		[ BuildBudgetRow(LOCTEXT("ShaderBudget", "Shader Budget"), G.ShaderBudget) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(2.0f)
		[ BuildBudgetRow(LOCTEXT("PSOBudget", "PSO Budget"), G.PSOBudget) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(2.0f)
		[ BuildBudgetRow(LOCTEXT("AudioMemory", "Audio Memory (MB)"), G.AudioMemory_MB) ];
}

// ============================================================
// System Budgets Grid
// ============================================================

TSharedRef<SWidget> SPGXPlatformHealthTab::BuildSystemBudgetsGrid()
{
	const UPGXPlatformConfig* Config = GetSelectedPlatformConfig();
	if (!Config)
	{
		return SNullWidget::NullWidget;
	}

	TSharedRef<SWrapBox> Grid = SNew(SWrapBox).UseAllottedSize(true);

	// Audio
	{
		TArray<TPair<FText, int32>> Budgets;
		Budgets.Add(MakeTuple(LOCTEXT("AudioMem", "Memory (MB)"), Config->AudioBudgets.AudioMemory_MB));
		Budgets.Add(MakeTuple(LOCTEXT("AudioConcurrent", "Concurrent Sounds"), Config->AudioBudgets.MaxConcurrentSounds));
		Budgets.Add(MakeTuple(LOCTEXT("AudioPool", "Sound Pool Size"), Config->AudioBudgets.SoundPoolMaxSize));
		Grid->AddSlot().Padding(4.0f)[ BuildSystemCard(LOCTEXT("Audio", "Audio"), PGX::System::Audio, Budgets) ];
	}

	// PSO
	{
		TArray<TPair<FText, int32>> Budgets;
		Budgets.Add(MakeTuple(LOCTEXT("PSOPermutations", "Shader Permutations"), Config->PSOBudgets.MaxShaderPermutations));
		Budgets.Add(MakeTuple(LOCTEXT("PSOWarmUp", "WarmUp Entries"), Config->PSOBudgets.MaxWarmUpEntries));
		Budgets.Add(MakeTuple(LOCTEXT("PSOTimeBudget", "WarmUp Time (ms)"), Config->PSOBudgets.WarmUpTimeBudgetMs));
		Grid->AddSlot().Padding(4.0f)[ BuildSystemCard(LOCTEXT("PSO", "PSO"), PGX::System::PSO, Budgets) ];
	}

	// Save
	{
		TArray<TPair<FText, int32>> Budgets;
		Budgets.Add(MakeTuple(LOCTEXT("SaveFileSize", "Max File Size (KB)"), Config->SaveBudgets.MaxSaveFileSize_KB));
		Budgets.Add(MakeTuple(LOCTEXT("SaveTotalStorage", "Total Storage (MB)"), Config->SaveBudgets.MaxTotalStorage_MB));
		Budgets.Add(MakeTuple(LOCTEXT("SaveConcurrentIO", "Concurrent IO"), Config->SaveBudgets.MaxConcurrentIO));
		Grid->AddSlot().Padding(4.0f)[ BuildSystemCard(LOCTEXT("Save", "Save"), PGX::System::Save, Budgets) ];
	}

	// Loading
	{
		TArray<TPair<FText, int32>> Budgets;
		Budgets.Add(MakeTuple(LOCTEXT("LoadingAsync", "Concurrent Async Loads"), Config->LoadingBudgets.MaxConcurrentAsyncLoads));
		Budgets.Add(MakeTuple(LOCTEXT("LoadingMinScreen", "Min Loading Screen (s)"), Config->LoadingBudgets.MinLoadingScreenDuration));
		Grid->AddSlot().Padding(4.0f)[ BuildSystemCard(LOCTEXT("Loading", "Loading"), PGX::System::Loading, Budgets) ];
	}

	// LevelFlow
	{
		TArray<TPair<FText, int32>> Budgets;
		Budgets.Add(MakeTuple(LOCTEXT("LFStreamingPool", "Streaming Pool (MB)"), Config->LevelFlowBudgets.StreamingPool_MB));
		Budgets.Add(MakeTuple(LOCTEXT("LFMaxSublevels", "Max Loaded SubLevels"), Config->LevelFlowBudgets.MaxLoadedSubLevels));
		Grid->AddSlot().Padding(4.0f)[ BuildSystemCard(LOCTEXT("LevelFlow", "LevelFlow"), PGX::System::LevelFlow, Budgets) ];
	}

	// GameFlow
	{
		TArray<TPair<FText, int32>> Budgets;
		Budgets.Add(MakeTuple(LOCTEXT("GFChannels", "Max Flow Channels"), Config->GameFlowBudgets.MaxFlowChannels));
		Budgets.Add(MakeTuple(LOCTEXT("GFHistory", "Max Transition History"), Config->GameFlowBudgets.MaxTransitionHistory));
		Grid->AddSlot().Padding(4.0f)[ BuildSystemCard(LOCTEXT("GameFlow", "GameFlow"), PGX::System::GameFlow, Budgets) ];
	}

	// Log
	{
		TArray<TPair<FText, int32>> Budgets;
		Budgets.Add(MakeTuple(LOCTEXT("LogRingBuffer", "Ring Buffer Capacity"), Config->LogBudgets.RingBufferCapacity));
		Budgets.Add(MakeTuple(LOCTEXT("LogMaxFiles", "Max Log Files"), Config->LogBudgets.MaxLogFiles));
		Grid->AddSlot().Padding(4.0f)[ BuildSystemCard(LOCTEXT("Log", "Log"), PGX::System::Log, Budgets) ];
	}

	// MGOS
	{
		TArray<TPair<FText, int32>> Budgets;
		Budgets.Add(MakeTuple(LOCTEXT("MGOSHistory", "History Window"), Config->MGOSBudgets.HistoryWindowSize));
		Budgets.Add(MakeTuple(LOCTEXT("MGOSTracked", "Max Tracked Classes"), Config->MGOSBudgets.MaxTrackedClasses));
		Budgets.Add(MakeTuple(LOCTEXT("MGOSIncidents", "Max Incident History"), Config->MGOSBudgets.MaxIncidentHistory));
		Grid->AddSlot().Padding(4.0f)[ BuildSystemCard(LOCTEXT("MGOS", "MGOS"), PGX::System::MGOS, Budgets) ];
	}

	// Message
	{
		TArray<TPair<FText, int32>> Budgets;
		Budgets.Add(MakeTuple(LOCTEXT("MsgHistory", "Max Message History"), Config->MessageBudgets.MaxMessageHistory));
		Grid->AddSlot().Padding(4.0f)[ BuildSystemCard(LOCTEXT("Message", "Message"), PGX::System::Message, Budgets) ];
	}

	// EventHandler
	{
		TArray<TPair<FText, int32>> Budgets;
		Budgets.Add(MakeTuple(LOCTEXT("EHCache", "Max Cached Handlers"), Config->EventHandlerBudgets.MaxCachedHandlers));
		Budgets.Add(MakeTuple(LOCTEXT("EHBlackbox", "Blackbox Buffer Size"), Config->EventHandlerBudgets.BlackboxBufferSize));
		Grid->AddSlot().Padding(4.0f)[ BuildSystemCard(LOCTEXT("EventHandler", "Event Handler"), PGX::System::EventHandler, Budgets) ];
	}

	return Grid;
}

// ============================================================
// System Card Widget
// ============================================================

TSharedRef<SWidget> SPGXPlatformHealthTab::BuildSystemCard(const FText& SystemName, const FLinearColor& SystemColor, const TArray<TPair<FText, int32>>& BudgetValues)
{
	TSharedRef<SVerticalBox> CardContent = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 4.0f)
		[
			SNew(STextBlock)
			.Text(SystemName)
			.Font(PGX::Font::SubHeader())
			.ColorAndOpacity(FSlateColor(SystemColor))
		];

	for (const auto& Budget : BudgetValues)
	{
		CardContent->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 1.0f)
			[
				BuildBudgetRow(Budget.Key, Budget.Value)
			];
	}

	return SNew(SBox)
		.WidthOverride(260.0f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
			.Padding(8.0f)
			[
				CardContent
			]
		];
}

// ============================================================
// Budget Row Widget
// ============================================================

TSharedRef<SWidget> SPGXPlatformHealthTab::BuildBudgetRow(const FText& Label, int32 Value)
{
	const bool bIsConfigured = (Value > 0);
	const FLinearColor ValueColor = bIsConfigured
		? PGX::Semantic::Good
		: PGX::Semantic::Neutral;

	const FText ValueText = bIsConfigured
		? FText::AsNumber(Value)
		: LOCTEXT("Unlimited", "Unlimited");

	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(Label)
			.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(8.0f, 0.0f, 0.0f, 0.0f)
		[
			SNew(STextBlock)
			.Text(ValueText)
			.Font(PGX::Font::Badge())
			.ColorAndOpacity(FSlateColor(ValueColor))
		];
}

// ============================================================
// Data
// ============================================================

void SPGXPlatformHealthTab::RefreshData()
{
	if (!ContentArea.IsValid())
	{
		return;
	}

	ContentArea->ClearChildren();

	const UPGXPlatformConfig* Config = GetSelectedPlatformConfig();

	if (Config)
	{
		// EN: Simulation banner (if active) / ES: Banner de simulacion (si activa)
#if WITH_EDITOR
		if (UPGXProfileSubsystem* ProfileSS = UPGXProfileSubsystem::GetCachedInstance())
		{
			if (ProfileSS->HasSimulationOverrides())
			{
				ContentArea->AddSlot()
					.AutoHeight()
					.Padding(0.0f, 4.0f)
					[
						SNew(SBorder)
						.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(PGX::Semantic::Warn).CopyWithNewOpacity(0.3f))
						.Padding(8.0f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(FText::Format(LOCTEXT("SimBanner", "SIMULATING: {0}"), Config->DisplayName))
								.Font(PGX::Font::SubHeader())
								.ColorAndOpacity(PGX::Semantic::Warn)
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SButton)
								.Text(LOCTEXT("ClearSim", "Clear Simulation"))
								.OnClicked_Lambda([this]()
								{
									PGX_LOG_INFO(LogPGXPlatformHealth, TEXT("[PlatformHealth] Clear Simulation requested"));
									if (UPGXProfileSubsystem* SS = UPGXProfileSubsystem::GetCachedInstance())
									{
										SS->ClearSimulationOverrides();
									}
									RefreshData();
									return FReply::Handled();
								})
							]
						]
					];
			}
		}
#endif

		// EN: Platform info header / ES: Header de info de plataforma
		ContentArea->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 4.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(FText::Format(LOCTEXT("PlatformInfo", "Platform: {0}"), Config->DisplayName))
					.Font(PGX::Font::SectionHeader())
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::Format(LOCTEXT("ThresholdInfo", "Warning: {0}% | Error: {1}%"),
						FText::AsNumber(Config->WarningThresholdPercent),
						FText::AsNumber(Config->ErrorThresholdPercent)))
					.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
				]
			];

		// EN: Traits as structured chips / ES: Traits como chips estructurados
		ContentArea->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 4.0f)
			[
				SNew(SPGXSectionDivider)
				.Title(LOCTEXT("TraitsHeader", "PLATFORM TRAITS"))
				.AccentColor(PGX::System::Profile)
			];

		ContentArea->AddSlot()
			.AutoHeight()
			.Padding(8.0f, 4.0f)
			[
				BuildTraitsGrid(Config)
			];

		// EN: Global Budgets Section / ES: Seccion de Presupuestos Globales
		ContentArea->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 4.0f)
			[
				SNew(SPGXSectionDivider)
				.Title(LOCTEXT("GlobalBudgets", "GLOBAL BUDGETS"))
				.AccentColor(PGX::System::Profile)
			];

		ContentArea->AddSlot()
			.AutoHeight()
			.Padding(8.0f, 4.0f)
			[
				BuildGlobalBudgetsSection()
			];

		// EN: Per-System Budgets / ES: Presupuestos por Sistema
		ContentArea->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 12.0f, 0.0f, 4.0f)
			[
				SNew(SPGXSectionDivider)
				.Title(LOCTEXT("SystemBudgets", "PER-SYSTEM BUDGETS"))
				.AccentColor(PGX::System::Profile)
			];

		ContentArea->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 4.0f)
			[
				BuildSystemBudgetsGrid()
			];

		if (StatusText.IsValid())
		{
			FText PIEState = bIsPIEActive ? LOCTEXT("PIEActive", "PIE Active") : LOCTEXT("EditorOnly", "Editor Only");
			StatusText->SetText(FText::Format(
				LOCTEXT("StatusLoaded", "Platform Health — {0} | {1} | Warning: {2}% | Error: {3}%"),
				Config->DisplayName,
				PIEState,
				FText::AsNumber(Config->WarningThresholdPercent),
				FText::AsNumber(Config->ErrorThresholdPercent)));
			StatusText->SetColorAndOpacity(FSlateColor(bIsPIEActive
				? PGX::Semantic::Good
				: PGX::Semantic::Neutral));
		}
	}
	else
	{
		ContentArea->AddSlot()
			.AutoHeight()
			.Padding(8.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("NoConfig", "No UPGXPlatformConfig found for the selected platform.\nCreate one in Content Browser: PGX FRAMEWORK > Profile > Platform Config"))
				.ColorAndOpacity(FSlateColor(PGX::Semantic::Warn))
			];

		if (StatusText.IsValid())
		{
			StatusText->SetText(LOCTEXT("StatusNoConfig", "No platform config available."));
		}
	}
}

const UPGXPlatformConfig* SPGXPlatformHealthTab::GetSelectedPlatformConfig() const
{
	if (!SelectedPlatform.IsValid())
	{
		return nullptr;
	}

	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	TArray<FAssetData> FoundAssets;
	AssetRegistry.GetAssetsByClass(UPGXPlatformConfig::StaticClass()->GetClassPathName(), FoundAssets);

	const UEnum* PlatformEnum = StaticEnum<EPGXTargetPlatform>();
	if (!PlatformEnum)
	{
		return nullptr;
	}

	int64 SelectedEnumValue = PlatformEnum->GetValueByNameString(*SelectedPlatform);
	if (SelectedEnumValue == INDEX_NONE)
	{
		return nullptr;
	}

	EPGXTargetPlatform SelectedTarget = static_cast<EPGXTargetPlatform>(SelectedEnumValue);

	for (const FAssetData& AssetData : FoundAssets)
	{
		if (UPGXPlatformConfig* PlatformConfig = Cast<UPGXPlatformConfig>(AssetData.GetAsset()))
		{
			if (PlatformConfig->TargetPlatform == SelectedTarget)
			{
				return PlatformConfig;
			}
		}
	}

	return nullptr;
}

// ============================================================
// Actions
// ============================================================

FReply SPGXPlatformHealthTab::OnRefreshClicked()
{
	PGX_LOG_INFO(LogPGXPlatformHealth, TEXT("[PlatformHealth] Manual refresh"));
	RefreshData();
	return FReply::Handled();
}

FReply SPGXPlatformHealthTab::OnSimulatePlatformClicked()
{
	if (!bIsPIEActive || !SelectedPlatform.IsValid())
	{
		return FReply::Handled();
	}

	PGX_LOG_INFO(LogPGXPlatformHealth, TEXT("[PlatformHealth] Simulate Platform: %s"), **SelectedPlatform);

	const FString Cmd = FString::Printf(TEXT("pgx.profile.simulate.platform %s"), **SelectedPlatform);
	GEngine->Exec(GEditor->GetEditorWorldContext().World(), *Cmd);

	RefreshData();
	return FReply::Handled();
}

void SPGXPlatformHealthTab::OnPlatformSelected(TSharedPtr<FString> NewSelection, ESelectInfo::Type /*SelectInfo*/)
{
	SelectedPlatform = NewSelection;
	PGX_LOG_INFO(LogPGXPlatformHealth, TEXT("[PlatformHealth] Platform selected: %s"),
		SelectedPlatform.IsValid() ? **SelectedPlatform : TEXT("(none)"));
	RefreshData();
}

// ============================================================
// PIE Lifecycle
// ============================================================

void SPGXPlatformHealthTab::BindPIEDelegates()
{
	PGX_LOG_INFO(LogPGXPlatformHealth, TEXT("[PlatformHealth] PIE delegates bound"));
	PIEStartedHandle = FEditorDelegates::PostPIEStarted.AddSP(SharedThis(this), &SPGXPlatformHealthTab::OnPIEStarted);
	PIEEndedHandle = FEditorDelegates::EndPIE.AddSP(SharedThis(this), &SPGXPlatformHealthTab::OnPIEEnded);
}

void SPGXPlatformHealthTab::OnPIEStarted(bool bIsSimulating)
{
	PGX_LOG_INFO(LogPGXPlatformHealth, TEXT("[PlatformHealth] PIE started (simulating=%s)"),
		bIsSimulating ? TEXT("true") : TEXT("false"));
	bIsPIEActive = true;

	// EN: Bind to profile changes for auto-refresh / ES: Bindear a cambios de perfil para auto-refresh
	for (const FWorldContext& Ctx : GEngine->GetWorldContexts())
	{
		if (Ctx.WorldType == EWorldType::PIE && Ctx.World())
		{
			if (UGameInstance* GI = Ctx.World()->GetGameInstance())
			{
				if (UPGXProfileSubsystem* ProfileSS = GI->GetSubsystem<UPGXProfileSubsystem>())
				{
					ProfileChangedHandle = ProfileSS->OnProfileChangedNative.AddSP(
						SharedThis(this), &SPGXPlatformHealthTab::OnProfileChanged);
					PGX_LOG_INFO(LogPGXPlatformHealth, TEXT("[PlatformHealth] Bound OnProfileChangedNative"));
					break;
				}
			}
		}
	}

	RefreshData();
}

void SPGXPlatformHealthTab::OnPIEEnded(bool /*bIsSimulating*/)
{
	PGX_LOG_INFO(LogPGXPlatformHealth, TEXT("[PlatformHealth] PIE ended"));
	bIsPIEActive = false;

	// EN: Unbind profile delegate / ES: Desvincular delegado de profile
	if (ProfileChangedHandle.IsValid())
	{
		if (UPGXProfileSubsystem* SS = UPGXProfileSubsystem::GetCachedInstance())
		{
			SS->OnProfileChangedNative.Remove(ProfileChangedHandle);
		}
		ProfileChangedHandle.Reset();
		PGX_LOG_INFO(LogPGXPlatformHealth, TEXT("[PlatformHealth] Unbound OnProfileChangedNative"));
	}

	RefreshData();
}

// ============================================================
// Delegate Callbacks
// ============================================================

void SPGXPlatformHealthTab::OnProfileChanged(const FPGXResolvedProfile& /*OldProfile*/, const FPGXResolvedProfile& /*NewProfile*/)
{
	PGX_LOG_INFO(LogPGXPlatformHealth, TEXT("[PlatformHealth] Profile changed — auto-refresh"));
	RefreshData();
}

#undef LOCTEXT_NAMESPACE
