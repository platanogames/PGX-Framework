// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PSOInspector/SPGXPSOInspectorTab.h"
#include "PGXPSOSubsystem.h"
#include "Logging/PGXLogMacros.h"
#include "PGXPSOWarmUpConfig.h"
#include "Profile/PGXProfileSubsystem.h"
#include "Profile/PGXPlatformConfig.h"
#include "ShaderPipelineCache.h"

#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/STableRow.h"
#include "Styling/AppStyle.h"
#include "Utils/PGXEditorUtils.h"
#include "Style/PGXVisualTokens.h"
#include "Style/PGXStatusPalette.h"
#include "Style/PGXEditorStyle.h"
#include "Widgets/SPGXPremiumShell.h"
#include "Widgets/SPGXSectionDivider.h"
#include "Widgets/SPGXKPIChip.h"
#include "Widgets/SPGXStatusBadge.h"
#include "Editor.h"
#include "Engine/GameInstance.h"

#define LOCTEXT_NAMESPACE "PGXPSOInspector"
DEFINE_LOG_CATEGORY_STATIC(LogPGXPSOInspector, Log, All);

// EN: PSO system color (Cyan) / ES: Color del sistema PSO (Cyan)
namespace { static const FLinearColor GPSOColor = PGX::System::PSO; }

// ============================================================================
// EN: Construction / Destruction
// ES: Construccion / Destruccion
// ============================================================================

void SPGXPSOInspectorTab::Construct(const FArguments& /*InArgs*/)
{
	PGX_LOG_INFO(LogPGXPSOInspector, TEXT("[PSO] Construct"));

	ChildSlot
	[
		SNew(SPGXPremiumShell)
		.SystemColor(PGX::System::PSO)
		.Title(LOCTEXT("PanelTitle", "PGX PSO INSPECTOR"))
		.Subtitle(LOCTEXT("PanelSubtitle", "Pipeline State Object warm-up monitor"))
		.Icon(FPGXEditorStyle::Get().GetBrush("PGXEditor.Icon.PSO"))
		.TitleRightContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("Refresh", "Refresh"))
				.OnClicked(this, &SPGXPSOInspectorTab::OnRefreshClicked)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f)
			[
				SNew(SButton)
				.OnClicked(this, &SPGXPSOInspectorTab::OnRecordToggleClicked)
				[
					SAssignNew(RecordButtonText, STextBlock)
					.Text(LOCTEXT("RecordStart", "Record"))
				]
			]
		]
		.FooterLeftContent()
		[
			SAssignNew(StatusText, STextBlock)
			.Text(LOCTEXT("StatusDisconnected", "Not connected - Start PIE to inspect PSO pipeline"))
			.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
		]
		.Content()
		[
			SNew(SVerticalBox)

			// EN: KPI chips row / ES: Fila de chips KPI
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4.0f)
			[
				SNew(SHorizontalBox)

				// EN: KPI chip: State / ES: Chip KPI: Estado
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4.0f, 0.0f)
				[
					SNew(SPGXKPIChip)
					.Label(LOCTEXT("KPIStateLabel", "STATE"))
					.AccentColor(PGX::System::PSO)
					.ValueWidget()
					[
						SAssignNew(KPIStateChip, STextBlock)
						.Text(LOCTEXT("KPIStateInit", "Idle"))
						.Font(PGX::Font::Mono())
						.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
					]
				]

				// EN: KPI chip: Configs / ES: Chip KPI: Configs
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4.0f, 0.0f)
				[
					SNew(SPGXKPIChip)
					.Label(LOCTEXT("KPIConfigsLabel", "CONFIGS"))
					.AccentColor(PGX::System::PSO)
					.ValueWidget()
					[
						SAssignNew(KPIConfigsChip, STextBlock)
						.Text(LOCTEXT("KPIConfigsInit", "0"))
						.Font(PGX::Font::Mono())
						.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
					]
				]

				// EN: KPI chip: Cache status badge / ES: Chip KPI: Badge de estado de cache
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4.0f, 0.0f)
				[
					SAssignNew(KPICacheBadge, SBorder)
					.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
					.BorderBackgroundColor(PGX::Surface::Elevated)
					.Padding(FMargin(6.0f, 2.0f))
					[
						SAssignNew(KPICacheChip, STextBlock)
						.Text(LOCTEXT("KPICacheInit", "Cache: --"))
						.Font(PGX::Font::CaptionBold())
						.ColorAndOpacity(FSlateColor(FLinearColor::White))
					]
				]
			]

			// EN: Main content area (scrollable) / ES: Area de contenido principal (scrollable)
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SScrollBox)

				// EN: Panel 1 - Warm-Up Status / ES: Panel 1 - Estado de Warm-Up
				+ SScrollBox::Slot()
				.Padding(4.0f)
				[
					BuildWarmUpStatusPanel()
				]

				+ SScrollBox::Slot()
				.Padding(4.0f, 0.0f)
				[
					SNew(SSeparator)
				]

				// EN: Panel 2 - Discovered Configs / ES: Panel 2 - Configs Descubiertos
				+ SScrollBox::Slot()
				.Padding(4.0f)
				[
					BuildDiscoveredConfigsPanel()
				]

				+ SScrollBox::Slot()
				.Padding(4.0f, 0.0f)
				[
					SNew(SSeparator)
				]

				// EN: Panel 3 - Recording Session / ES: Panel 3 - Sesion de Grabacion
				+ SScrollBox::Slot()
				.Padding(4.0f)
				[
					BuildRecordingPanel()
				]
			]

			// EN: Platform Budgets section / ES: Seccion de Presupuestos de Plataforma
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4.0f, 4.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SPGXSectionDivider)
					.Title(LOCTEXT("PlatformBudgets", "PLATFORM BUDGETS"))
					.AccentColor(PGX::System::PSO)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8.0f, 4.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text_Lambda([]() -> FText
					{
						if (UPGXProfileSubsystem* ProfileSS = UPGXProfileSubsystem::GetCachedInstance())
						{
							if (const UPGXPlatformConfig* Cfg = ProfileSS->GetActivePlatformConfig())
							{
								const auto& B = Cfg->PSOBudgets;
								return FText::Format(
									LOCTEXT("PSOBudgetsFmt", "Shader Permutations: {0} | WarmUp Entries: {1} | WarmUp Time: {2}ms"),
									B.MaxShaderPermutations > 0 ? FText::AsNumber(B.MaxShaderPermutations) : LOCTEXT("Unlim1", "Unlimited"),
									B.MaxWarmUpEntries > 0 ? FText::AsNumber(B.MaxWarmUpEntries) : LOCTEXT("Unlim2", "Unlimited"),
									B.WarmUpTimeBudgetMs > 0 ? FText::AsNumber(B.WarmUpTimeBudgetMs) : LOCTEXT("Unlim3", "Unlimited"));
							}
						}
						return LOCTEXT("NoPlatformCfg", "No platform config active");
					})
					.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
				]
			]
		]
	];

	BindPIEDelegates();

	// EN: If PIE is already running when panel opens, bind immediately
	// ES: Si PIE ya esta corriendo cuando se abre el panel, enlazar inmediatamente
	if (GEditor && GEditor->PlayWorld)
	{
		PGX_LOG_INFO(LogPGXPSOInspector, TEXT("[PSO] PIE already running at Construct — binding"));
		OnPIEStarted(false);
	}
}

SPGXPSOInspectorTab::~SPGXPSOInspectorTab()
{
	PGX_LOG_INFO(LogPGXPSOInspector, TEXT("[PSO] Destructor — cleaning up"));
	StopPolling();
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

// EN: BuildToolbar — now inlined into Construct via SPGXPremiumShell + SPGXKPIChip
// ES: BuildToolbar — ahora inline en Construct via SPGXPremiumShell + SPGXKPIChip

TSharedRef<SWidget> SPGXPSOInspectorTab::BuildWarmUpStatusPanel()
{
	return SNew(SVerticalBox)

		// EN: Section header / ES: Cabecera de seccion
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 4.0f)
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("WarmUpStatusHeader", "WARM-UP STATUS"))
			.AccentColor(PGX::System::PSO)
		]

		// EN: State badge / ES: Badge de estado
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 4.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("StateLabel", "State: "))
				.Font(PGX::Font::SubHeader())
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SAssignNew(StateBadgeText, STextBlock)
				.Text(LOCTEXT("StateIdle", "Idle"))
				.Font(PGX::Font::SubHeader())
				.ColorAndOpacity(FSlateColor(GetStateColor(EPGXPSOWarmUpState::Idle)))
			]
		]

		// EN: Progress / ES: Progreso
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 2.0f)
		[
			SAssignNew(ProgressText, STextBlock)
			.Text(LOCTEXT("ProgressInit", "Progress: 0.0%"))
			.Font(PGX::Font::Mono())
		]

		// EN: Progress bar / ES: Barra de progreso
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 2.0f)
		[
			SNew(SBox)
			.HeightOverride(6.0f)
			[
				SNew(SProgressBar)
				.Percent_Lambda([this]() -> TOptional<float>
				{
					UPGXPSOSubsystem* Sub = BoundSubsystem.Get();
					if (!Sub) { return 0.0f; }
					return Sub->GetWarmUpProgress().PercentComplete;
				})
				.FillColorAndOpacity(FLinearColor(0.0f, 0.737f, 0.831f))
			]
		]

		// EN: Stats grid / ES: Grid de stats
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 2.0f)
		[
			SAssignNew(StatsText, STextBlock)
			.Text(LOCTEXT("StatsInit", "Total: 0 | Completed: 0 | Failed: 0 | Dedup: 0 | Elapsed: 0.0s | Peak Materials: 0"))
			.Font(PGX::Font::Mono())
			.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
		]

		// EN: Active contexts / ES: Contextos activos
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 2.0f)
		[
			SAssignNew(ContextsText, STextBlock)
			.Text(LOCTEXT("ContextsInit", "Active Contexts: (none)"))
			.Font(PGX::Font::Mono())
			.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
		]

		// EN: Action buttons / ES: Botones de accion
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 6.0f, 0.0f, 2.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("ForceWarmUp", "Force Warm-Up All"))
				.ToolTipText(LOCTEXT("ForceWarmUpTip", "Request warm-up for all discovered configs"))
				.OnClicked(this, &SPGXPSOInspectorTab::OnForceWarmUpClicked)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("PauseResume", "Pause/Resume"))
				.ToolTipText(LOCTEXT("PauseResumeTip", "Pause or resume warm-up pipeline"))
				.OnClicked(this, &SPGXPSOInspectorTab::OnPauseResumeClicked)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("SaveCache", "Save Cache"))
				.ToolTipText(LOCTEXT("SaveCacheTip", "Save PSO cache to disk"))
				.OnClicked(this, &SPGXPSOInspectorTab::OnSaveCacheClicked)
			]
		];
}

TSharedRef<SWidget> SPGXPSOInspectorTab::BuildDiscoveredConfigsPanel()
{
	return SNew(SVerticalBox)

		// EN: Section header / ES: Cabecera de seccion
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 4.0f)
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("ConfigsHeader", "DISCOVERED CONFIGS"))
			.AccentColor(PGX::System::PSO)
		]

		// EN: Configs list / ES: Lista de configs
		+ SVerticalBox::Slot()
		.AutoHeight()
		.MaxHeight(200.0f)
		[
			SAssignNew(ConfigListView, SListView<TSharedPtr<FString>>)
			.ListItemsSource(&ConfigDisplayNames)
			.OnGenerateRow(this, &SPGXPSOInspectorTab::OnGenerateConfigRow)
			.SelectionMode(ESelectionMode::None)
		];
}

TSharedRef<SWidget> SPGXPSOInspectorTab::BuildRecordingPanel()
{
	return SNew(SVerticalBox)

		// EN: Section header / ES: Cabecera de seccion
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 4.0f)
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("RecordingHeader", "RECORDING SESSION"))
			.AccentColor(PGX::System::PSO)
		]

		// EN: Action buttons / ES: Botones de accion
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 2.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("ExportJSON", "Export JSON"))
				.OnClicked(this, &SPGXPSOInspectorTab::OnExportJsonClicked)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("ClearRecording", "Clear"))
				.OnClicked(this, &SPGXPSOInspectorTab::OnClearRecordingClicked)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("ConvertToCatalog", "Convert to Catalog"))
				.ToolTipText(LOCTEXT("ConvertTip", "Convert recorded entries to catalog entries for a new config DA"))
				.OnClicked(this, &SPGXPSOInspectorTab::OnConvertRecordingClicked)
			]
		]

		// EN: Recording entries list / ES: Lista de entradas de grabacion
		+ SVerticalBox::Slot()
		.AutoHeight()
		.MaxHeight(300.0f)
		[
			SAssignNew(RecordingListView, SListView<TSharedPtr<FPGXPSORecordedEntry>>)
			.ListItemsSource(&RecordingEntries)
			.OnGenerateRow(this, &SPGXPSOInspectorTab::OnGenerateRecordingRow)
			.SelectionMode(ESelectionMode::None)
		];
}

// ============================================================================
// EN: SListView Row Generators
// ES: Generadores de Filas SListView
// ============================================================================

TSharedRef<ITableRow> SPGXPSOInspectorTab::OnGenerateConfigRow(
	TSharedPtr<FString> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
		.Padding(FMargin(4.0f, 1.0f))
		[
			SNew(STextBlock)
			.Text(FText::FromString(Item.IsValid() ? *Item : TEXT("(null)")))
			.Font(PGX::Font::Mono())
		];
}

TSharedRef<ITableRow> SPGXPSOInspectorTab::OnGenerateRecordingRow(
	TSharedPtr<FPGXPSORecordedEntry> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	if (!Item.IsValid())
	{
		return SNew(STableRow<TSharedPtr<FPGXPSORecordedEntry>>, OwnerTable)
			[
				SNew(STextBlock).Text(LOCTEXT("NullEntry", "(null)"))
			];
	}

	const FString MaterialName = FPaths::GetBaseFilename(Item->MaterialPath);
	const FLinearColor HitchColor = Item->bCausedHitch
		? FLinearColor(1.0f, 0.3f, 0.3f)
		: FLinearColor(0.7f, 0.7f, 0.7f);

	return SNew(STableRow<TSharedPtr<FPGXPSORecordedEntry>>, OwnerTable)
		.Padding(FMargin(4.0f, 1.0f))
		[
			SNew(SHorizontalBox)

			// EN: Material name / ES: Nombre de material
			+ SHorizontalBox::Slot()
			.FillWidth(0.35f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(MaterialName))
				.Font(PGX::Font::BodySmall())
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			]

			// EN: VF name / ES: Nombre VF
			+ SHorizontalBox::Slot()
			.FillWidth(0.2f)
			.VAlign(VAlign_Center)
			.Padding(4.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->VertexFactoryName.ToString()))
				.Font(PGX::Font::Mono())
				.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			]

			// EN: Pass / ES: Pase
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4.0f, 0.0f)
			[
				SNew(SBox)
				.WidthOverride(80.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Item->MeshPassName))
					.Font(PGX::Font::Mono())
					.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
				]
			]

			// EN: Time (ms) / ES: Tiempo (ms)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4.0f, 0.0f)
			[
				SNew(SBox)
				.WidthOverride(60.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(FString::Printf(TEXT("%.1fms"), Item->CompilationTimeMs)))
					.Font(PGX::Font::Mono())
					.ColorAndOpacity(FSlateColor(HitchColor))
				]
			]

			// EN: Hitch indicator / ES: Indicador de hitch
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(Item->bCausedHitch ? LOCTEXT("Hitch", "HITCH") : FText::GetEmpty())
				.Font(PGX::Font::CaptionBold())
				.ColorAndOpacity(FSlateColor(PGX::Semantic::Error))
			]

			// EN: Frame / ES: Frame
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("F:%lld"), Item->FrameNumber)))
				.Font(PGX::Font::Mono())
				.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
			]
		];
}

// ============================================================================
// EN: Refresh Methods
// ES: Metodos de Refresh
// ============================================================================

void SPGXPSOInspectorTab::RefreshAllData()
{
	RefreshStatusPanel();
	RefreshConfigsList();
	RefreshRecordingList();
}

void SPGXPSOInspectorTab::RefreshStatusPanel()
{
	UPGXPSOSubsystem* Sub = BoundSubsystem.Get();

	if (!Sub)
	{
		if (StateBadgeText.IsValid())
		{
			StateBadgeText->SetText(LOCTEXT("StateNotConnected", "Not Connected"));
			StateBadgeText->SetColorAndOpacity(FSlateColor(PGX::Text::Muted));
		}
		return;
	}

	const EPGXPSOWarmUpState State = Sub->GetWarmUpState();
	const FPGXPSOWarmUpProgress Progress = Sub->GetWarmUpProgress();

	// EN: State badge / ES: Badge de estado
	if (StateBadgeText.IsValid())
	{
		StateBadgeText->SetText(FText::FromString(GetStateName(State)));
		StateBadgeText->SetColorAndOpacity(FSlateColor(GetStateColor(State)));
	}

	// EN: Progress / ES: Progreso
	if (ProgressText.IsValid())
	{
		ProgressText->SetText(FText::FromString(
			FString::Printf(TEXT("Progress: %.1f%%"), Progress.PercentComplete * 100.0f)));
	}

	// EN: Stats grid / ES: Grid de stats
	if (StatsText.IsValid())
	{
		StatsText->SetText(FText::FromString(
			FString::Printf(TEXT("Total: %d | Completed: %d | Failed: %d | Dedup: %d | Elapsed: %.1fs | Peak Materials: %d"),
				Progress.TotalEntries,
				Progress.CompletedEntries,
				Progress.FailedEntries,
				Progress.DeduplicatedEntries,
				Progress.ElapsedTimeSeconds,
				Progress.PeakLoadedMaterials)));
	}

	// EN: Active contexts / ES: Contextos activos
	if (ContextsText.IsValid())
	{
		TArray<FGameplayTag> Contexts = Sub->GetActiveContexts();
		if (Contexts.IsEmpty())
		{
			ContextsText->SetText(LOCTEXT("ContextsNone", "Active Contexts: (none)"));
		}
		else
		{
			FString CtxList;
			for (const FGameplayTag& Ctx : Contexts)
			{
				if (!CtxList.IsEmpty()) CtxList += TEXT(", ");
				CtxList += PGXEditorUtils::TagToLeafName(Ctx);
			}
			ContextsText->SetText(FText::FromString(FString::Printf(TEXT("Active Contexts: %s"), *CtxList)));
		}
	}
}

void SPGXPSOInspectorTab::RefreshConfigsList()
{
	UPGXPSOSubsystem* Sub = BoundSubsystem.Get();
	ConfigDisplayNames.Empty();

	if (!Sub)
	{
		if (ConfigListView.IsValid())
		{
			ConfigListView->RequestListRefresh();
		}
		return;
	}

	const int32 Count = Sub->GetDiscoveredConfigCount();

	if (Count == 0)
	{
		ConfigDisplayNames.Add(MakeShared<FString>(TEXT("(no configs discovered)")));
	}
	else
	{
		ConfigDisplayNames.Add(MakeShared<FString>(
			FString::Printf(TEXT("%d config(s) discovered"), Count)));

		// EN: Show active contexts as additional entries / ES: Mostrar contextos activos como entradas adicionales
		TArray<FGameplayTag> Contexts = Sub->GetActiveContexts();
		for (const FGameplayTag& Ctx : Contexts)
		{
			ConfigDisplayNames.Add(MakeShared<FString>(
				FString::Printf(TEXT("  Active Context: %s"), *PGXEditorUtils::TagToLeafName(Ctx))));
		}
	}

	if (ConfigListView.IsValid())
	{
		ConfigListView->RequestListRefresh();
	}
}

void SPGXPSOInspectorTab::RefreshRecordingList()
{
	RecordingEntries.Empty();

#if WITH_EDITOR
	UPGXPSOSubsystem* Sub = BoundSubsystem.Get();
	if (Sub)
	{
		const TArray<FPGXPSORecordedEntry>& Entries = Sub->GetRecordedEntries();
		for (const FPGXPSORecordedEntry& Entry : Entries)
		{
			RecordingEntries.Add(MakeShared<FPGXPSORecordedEntry>(Entry));
		}

		// EN: Update record button text / ES: Actualizar texto del boton de grabacion
		if (RecordButtonText.IsValid())
		{
			RecordButtonText->SetText(Sub->IsRecording()
				? LOCTEXT("RecordStop", "Stop Recording")
				: LOCTEXT("RecordStart", "Record"));
		}
	}
#endif

	if (RecordingListView.IsValid())
	{
		RecordingListView->RequestListRefresh();
	}
}

// ============================================================================
// EN: PIE Lifecycle
// ES: Ciclo de Vida PIE
// ============================================================================

void SPGXPSOInspectorTab::BindPIEDelegates()
{
	PGX_LOG_INFO(LogPGXPSOInspector, TEXT("[PSO] PIE delegates bound"));
	PIEStartedHandle = FEditorDelegates::PostPIEStarted.AddSP(
		SharedThis(this), &SPGXPSOInspectorTab::OnPIEStarted);
	PIEEndedHandle = FEditorDelegates::EndPIE.AddSP(
		SharedThis(this), &SPGXPSOInspectorTab::OnPIEEnded);
}

void SPGXPSOInspectorTab::OnPIEStarted(bool bIsSimulating)
{
	PGX_LOG_INFO(LogPGXPSOInspector, TEXT("[PSO] PIE started (simulating=%d)"), bIsSimulating);
	bIsPIEActive = true;
	BindToSubsystem();
	StartPolling();
}

void SPGXPSOInspectorTab::OnPIEEnded(bool /*bIsSimulating*/)
{
	PGX_LOG_INFO(LogPGXPSOInspector, TEXT("[PSO] PIE ended — retaining %d recording entries"), RecordingEntries.Num());
	bIsPIEActive = false;
	StopPolling();
	UnbindFromSubsystem();

	if (StatusText.IsValid())
	{
		StatusText->SetText(FText::Format(
			LOCTEXT("StatusDisconnectedRetain", "Disconnected - {0} recording entries retained"),
			FText::AsNumber(RecordingEntries.Num())));
		StatusText->SetColorAndOpacity(FSlateColor(PGX::Text::Muted));
	}
}

void SPGXPSOInspectorTab::BindToSubsystem()
{
	UPGXPSOSubsystem* Sub = nullptr;

	if (GEditor)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::PIE && Context.World())
			{
				UGameInstance* GI = Context.World()->GetGameInstance();
				if (GI)
				{
					Sub = GI->GetSubsystem<UPGXPSOSubsystem>();
					if (Sub) break;
				}
			}
		}
	}

	if (!Sub)
	{
		if (StatusText.IsValid())
		{
			StatusText->SetText(LOCTEXT("StatusNoSubsystem", "PIE active but PSOSubsystem not found"));
			StatusText->SetColorAndOpacity(FSlateColor(PGX::Semantic::Warn));
		}
		return;
	}

	BoundSubsystem = Sub;

	PGX_LOG_INFO(LogPGXPSOInspector, TEXT("[PSO] Bound to subsystem"));

	// EN: Bind to native delegates / ES: Enlazar a delegados nativos
	StateChangedHandle = Sub->OnStateChangedNative.AddSP(
		SharedThis(this), &SPGXPSOInspectorTab::OnPSOStateChanged);
	WarmUpBeginHandle = Sub->OnWarmUpBeginNative.AddSP(
		SharedThis(this), &SPGXPSOInspectorTab::OnWarmUpBegin);
	WarmUpProgressHandle = Sub->OnWarmUpProgressNative.AddSP(
		SharedThis(this), &SPGXPSOInspectorTab::OnWarmUpProgress);
	WarmUpCompleteHandle = Sub->OnWarmUpCompleteNative.AddSP(
		SharedThis(this), &SPGXPSOInspectorTab::OnWarmUpComplete);

	if (StatusText.IsValid())
	{
		StatusText->SetText(LOCTEXT("StatusConnected", "Connected to PSOSubsystem - Live updates active"));
		StatusText->SetColorAndOpacity(FSlateColor(PGX::Semantic::Good));
	}

	RefreshAllData();
	RefreshKPIChips();
}

void SPGXPSOInspectorTab::UnbindFromSubsystem()
{
	PGX_LOG_INFO(LogPGXPSOInspector, TEXT("[PSO] Unbinding from subsystem"));
	UPGXPSOSubsystem* Sub = BoundSubsystem.Get();
	if (Sub)
	{
		if (StateChangedHandle.IsValid()) Sub->OnStateChangedNative.Remove(StateChangedHandle);
		if (WarmUpBeginHandle.IsValid()) Sub->OnWarmUpBeginNative.Remove(WarmUpBeginHandle);
		if (WarmUpProgressHandle.IsValid()) Sub->OnWarmUpProgressNative.Remove(WarmUpProgressHandle);
		if (WarmUpCompleteHandle.IsValid()) Sub->OnWarmUpCompleteNative.Remove(WarmUpCompleteHandle);
	}
	StateChangedHandle.Reset();
	WarmUpBeginHandle.Reset();
	WarmUpProgressHandle.Reset();
	WarmUpCompleteHandle.Reset();
	BoundSubsystem.Reset();
}

void SPGXPSOInspectorTab::StartPolling()
{
	if (PollingHandle.IsValid())
	{
		return;
	}

	PollingHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateLambda([this](float /*DeltaTime*/) -> bool
		{
			if (!bIsPIEActive)
			{
				return false;
			}
			RefreshAllData();
			return true;
		}),
		1.0f); // EN: 1 second interval / ES: Intervalo de 1 segundo
}

void SPGXPSOInspectorTab::StopPolling()
{
	if (PollingHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(PollingHandle);
		PollingHandle.Reset();
	}
}

// ============================================================================
// EN: Delegate Callbacks
// ES: Callbacks de Delegados
// ============================================================================

void SPGXPSOInspectorTab::OnPSOStateChanged(EPGXPSOWarmUpState NewState)
{
	PGX_LOG_INFO(LogPGXPSOInspector, TEXT("[PSO] State changed to: %s"), *GetStateName(NewState));
	RefreshStatusPanel();
	RefreshKPIChips();
}

void SPGXPSOInspectorTab::OnWarmUpBegin()
{
	PGX_LOG_INFO(LogPGXPSOInspector, TEXT("[PSO] Warm-up begin"));
	RefreshStatusPanel();
	RefreshConfigsList();
	RefreshKPIChips();
}

void SPGXPSOInspectorTab::OnWarmUpProgress(int32 /*Completed*/, int32 /*Total*/, float /*Percent*/)
{
	RefreshStatusPanel();
	RefreshKPIChips();
}

void SPGXPSOInspectorTab::OnWarmUpComplete()
{
	PGX_LOG_INFO(LogPGXPSOInspector, TEXT("[PSO] Warm-up complete"));
	RefreshAllData();
	RefreshKPIChips();
}

// ============================================================================
// EN: Actions
// ES: Acciones
// ============================================================================

FReply SPGXPSOInspectorTab::OnRefreshClicked()
{
	RefreshAllData();
	return FReply::Handled();
}

FReply SPGXPSOInspectorTab::OnRecordToggleClicked()
{
#if WITH_EDITOR
	UPGXPSOSubsystem* Sub = BoundSubsystem.Get();
	if (Sub)
	{
		if (Sub->IsRecording())
		{
			Sub->StopRecording();
		}
		else
		{
			Sub->StartRecording(TEXT("InspectorSession"));
		}
		RefreshRecordingList();
	}
#endif
	return FReply::Handled();
}

FReply SPGXPSOInspectorTab::OnExportJsonClicked()
{
#if WITH_EDITOR
	UPGXPSOSubsystem* Sub = BoundSubsystem.Get();
	if (Sub)
	{
		Sub->ExportRecordingToJson(TEXT(""));
	}
#endif
	return FReply::Handled();
}

FReply SPGXPSOInspectorTab::OnClearRecordingClicked()
{
#if WITH_EDITOR
	UPGXPSOSubsystem* Sub = BoundSubsystem.Get();
	if (Sub)
	{
		Sub->ClearRecording();
		RefreshRecordingList();
	}
#endif
	return FReply::Handled();
}

FReply SPGXPSOInspectorTab::OnForceWarmUpClicked()
{
	if (BoundSubsystem.IsValid())
	{
		PGX_LOG_INFO(LogPGXPSOInspector, TEXT("[PSO] Force Warm-Up All requested"));
		BoundSubsystem->RequestWarmUpAll();
	}
	return FReply::Handled();
}

FReply SPGXPSOInspectorTab::OnPauseResumeClicked()
{
	UPGXPSOSubsystem* Sub = BoundSubsystem.Get();
	if (Sub)
	{
		const EPGXPSOWarmUpState State = Sub->GetWarmUpState();
		if (State == EPGXPSOWarmUpState::Paused)
		{
			PGX_LOG_INFO(LogPGXPSOInspector, TEXT("[PSO] Resuming warm-up"));
			Sub->ResumeWarmUp();
		}
		else if (State == EPGXPSOWarmUpState::Loading || State == EPGXPSOWarmUpState::Compiling)
		{
			PGX_LOG_INFO(LogPGXPSOInspector, TEXT("[PSO] Pausing warm-up"));
			Sub->PauseWarmUp();
		}
	}
	return FReply::Handled();
}

FReply SPGXPSOInspectorTab::OnSaveCacheClicked()
{
	if (BoundSubsystem.IsValid())
	{
		PGX_LOG_INFO(LogPGXPSOInspector, TEXT("[PSO] Save cache to disk"));
		BoundSubsystem->SaveCacheToDisk();
		RefreshKPIChips();
	}
	return FReply::Handled();
}

FReply SPGXPSOInspectorTab::OnConvertRecordingClicked()
{
#if WITH_EDITOR
	UPGXPSOSubsystem* Sub = BoundSubsystem.Get();
	if (Sub)
	{
		TArray<FPGXPSOEntry> Entries = Sub->ConvertRecordingToCatalogEntries(FGameplayTag());
		PGX_LOG_INFO(LogPGXPSOInspector, TEXT("[PSO] Converted %d entries to catalog format"), Entries.Num());
		if (StatusText.IsValid())
		{
			StatusText->SetText(FText::Format(
				LOCTEXT("ConvertResult", "Converted {0} recording entries to catalog format"),
				FText::AsNumber(Entries.Num())));
			StatusText->SetColorAndOpacity(FSlateColor(PGX::Semantic::Good));
		}
	}
#endif
	return FReply::Handled();
}

void SPGXPSOInspectorTab::RefreshKPIChips()
{
	UPGXPSOSubsystem* Sub = BoundSubsystem.Get();

	if (KPIStateChip.IsValid())
	{
		if (Sub)
		{
			const EPGXPSOWarmUpState State = Sub->GetWarmUpState();
			KPIStateChip->SetText(FText::FromString(GetStateName(State)));
			KPIStateChip->SetColorAndOpacity(FSlateColor(GetStateColor(State)));
		}
		else
		{
			KPIStateChip->SetText(LOCTEXT("KPIStateDisc", "--"));
			KPIStateChip->SetColorAndOpacity(FSlateColor(PGX::Text::Muted));
		}
	}

	if (KPIConfigsChip.IsValid())
	{
		const int32 Count = Sub ? Sub->GetDiscoveredConfigCount() : 0;
		KPIConfigsChip->SetText(FText::AsNumber(Count));
	}

	if (KPICacheBadge.IsValid() && KPICacheChip.IsValid())
	{
		if (Sub)
		{
			const bool bDirty = Sub->IsCacheDirty();
			KPICacheChip->SetText(bDirty ? LOCTEXT("CacheDirty", "UNSAVED") : LOCTEXT("CacheClean", "CLEAN"));
			KPICacheBadge->SetBorderBackgroundColor(bDirty
				? FLinearColor(0.8f, 0.6f, 0.0f)   // Orange = dirty
				: FLinearColor(0.2f, 0.6f, 0.2f));  // Green = clean
		}
		else
		{
			KPICacheChip->SetText(LOCTEXT("CacheNA", "Cache: --"));
			KPICacheBadge->SetBorderBackgroundColor(PGX::Surface::Elevated);
		}
	}
}

// ============================================================================
// EN: Helpers
// ES: Helpers
// ============================================================================

FLinearColor SPGXPSOInspectorTab::GetStateColor(EPGXPSOWarmUpState State)
{
	switch (State)
	{
	case EPGXPSOWarmUpState::Idle:      return PGX::GetStatusPaletteColor(PGX::EStatusPalette::Idle);
	case EPGXPSOWarmUpState::Loading:   return PGX::GetStatusPaletteColor(PGX::EStatusPalette::Loading);
	case EPGXPSOWarmUpState::Compiling: return PGX::GetStatusPaletteColor(PGX::EStatusPalette::Preparing);
	case EPGXPSOWarmUpState::Complete:  return PGX::GetStatusPaletteColor(PGX::EStatusPalette::Complete);
	case EPGXPSOWarmUpState::Failed:    return PGX::GetStatusPaletteColor(PGX::EStatusPalette::Failed);
	case EPGXPSOWarmUpState::Paused:    return PGX::GetStatusPaletteColor(PGX::EStatusPalette::Paused);
	default:                            return PGX::GetStatusPaletteColor(PGX::EStatusPalette::Unknown);
	}
}

FString SPGXPSOInspectorTab::GetStateName(EPGXPSOWarmUpState State)
{
	switch (State)
	{
	case EPGXPSOWarmUpState::Idle:      return TEXT("Idle");
	case EPGXPSOWarmUpState::Loading:   return TEXT("Loading");
	case EPGXPSOWarmUpState::Compiling: return TEXT("Compiling");
	case EPGXPSOWarmUpState::Complete:  return TEXT("Complete");
	case EPGXPSOWarmUpState::Failed:    return TEXT("Failed");
	case EPGXPSOWarmUpState::Paused:    return TEXT("Paused");
	default:                            return TEXT("Unknown");
	}
}

#undef LOCTEXT_NAMESPACE
