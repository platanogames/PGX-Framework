// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "SaveInspector/SPGXSaveInspectorTab.h"
#include "PGXSaveSubsystem.h"
#include "Logging/PGXLogMacros.h"
#include "PGXSaveConfig.h"
#include "PGXSaveGame.h"
#include "Profile/PGXProfileSubsystem.h"
#include "Profile/PGXPlatformConfig.h"

#include "Widgets/SPGXPremiumShell.h"
#include "Widgets/SPGXSectionDivider.h"
#include "Style/PGXEditorStyle.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Utils/PGXEditorUtils.h"
#include "Widgets/SPGXEmptyStateV2.h"
#include "Styling/AppStyle.h"
#include "Style/PGXVisualTokens.h"
#include "Editor.h"
#include "Engine/GameInstance.h"

// EN: Save Inspector - Real-time visualization of the PGX Save system state
// ES: Save Inspector - Visualizacion en tiempo real del estado del sistema PGX Save

#define LOCTEXT_NAMESPACE "PGXSaveInspector"
DEFINE_LOG_CATEGORY_STATIC(LogPGXSaveInspector, Log, All);

// EN: System color constant — Green — via PGX Visual Tokens
// ES: Color del sistema constante — Verde — via PGX Visual Tokens
static const FLinearColor GSaveInspectorColor = PGX::System::Save;

// ============================================================================
// EN: SMultiColumnTableRow — Context table
// ES: SMultiColumnTableRow — Tabla de contextos
// ============================================================================

class SPGXContextTableRow : public SMultiColumnTableRow<TSharedPtr<FPGXSaveContextEntry>>
{
public:
	SLATE_BEGIN_ARGS(SPGXContextTableRow) {}
		SLATE_ARGUMENT(TSharedPtr<FPGXSaveContextEntry>, Item)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable)
	{
		Item = InArgs._Item;
		SMultiColumnTableRow::Construct(FSuperRowType::FArguments(), InOwnerTable);
	}

	TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
	{
		if (!Item.IsValid()) return SNullWidget::NullWidget;

		if (ColumnName == "Context")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock)
				.Text(Item->DisplayName)
				.Font(PGX::Font::BodySmall())
			];
		}
		if (ColumnName == "Mode")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->SaveModeName))
				.ColorAndOpacity(FSlateColor(PGX::Semantic::Info))
			];
		}
		if (ColumnName == "Domains")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock)
				.Text(FText::AsNumber(Item->DomainCount))
			];
		}
		if (ColumnName == "Slots")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock)
				.Text(FText::AsNumber(Item->SlotCount))
			];
		}
		if (ColumnName == "ActiveSlot")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->ActiveSlotName.IsEmpty() ? TEXT("---") : Item->ActiveSlotName))
				.ColorAndOpacity(Item->ActiveSlotName.IsEmpty()
					? FSlateColor(PGX::Text::Muted)
					: FSlateColor(PGX::Semantic::Good))
			];
		}
		if (ColumnName == "AutoSave")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock)
				.Text(Item->bAutoSaveActive
					? LOCTEXT("AutoOn", "ON")
					: LOCTEXT("AutoOff", "OFF"))
				.ColorAndOpacity(Item->bAutoSaveActive
					? FSlateColor(PGX::Semantic::Good)
					: FSlateColor(PGX::Text::Muted))
			];
		}
		return SNullWidget::NullWidget;
	}

private:
	TSharedPtr<FPGXSaveContextEntry> Item;
};

// ============================================================================
// EN: SMultiColumnTableRow — Pipeline Log table
// ES: SMultiColumnTableRow — Tabla de Pipeline Log
// ============================================================================

class SPGXPipelineTableRow : public SMultiColumnTableRow<TSharedPtr<FPGXSavePipelineEntry>>
{
public:
	SLATE_BEGIN_ARGS(SPGXPipelineTableRow) {}
		SLATE_ARGUMENT(TSharedPtr<FPGXSavePipelineEntry>, Item)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable)
	{
		Item = InArgs._Item;
		SMultiColumnTableRow::Construct(FSuperRowType::FArguments(), InOwnerTable);
	}

	TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
	{
		if (!Item.IsValid()) return SNullWidget::NullWidget;

		// EN: Color-code by result / ES: Color segun resultado
		FLinearColor RowColor = FLinearColor::White;
		if (Item->Result == EPGXSaveResult::Success)
		{
			RowColor = PGX::Semantic::Good;
		}
		else if (Item->Result == EPGXSaveResult::Failed)
		{
			RowColor = PGX::Semantic::Error;
		}
		else
		{
			RowColor = PGX::Semantic::Warn;
		}

		if (ColumnName == "Time")
		{
			return SNew(SBox).Padding(FMargin(4, 1))
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->Timestamp.ToString(TEXT("%H:%M:%S"))))
				.ToolTipText(FText::FromString(Item->Timestamp.ToString()))
				.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
			];
		}
		if (ColumnName == "Operation")
		{
			return SNew(SBox).Padding(FMargin(4, 1))
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->OperationType))
				.ColorAndOpacity(FSlateColor(RowColor))
				.Font(PGX::Font::Badge())
			];
		}
		if (ColumnName == "Context")
		{
			return SNew(SBox).Padding(FMargin(4, 1))
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->ContextName))
			];
		}
		if (ColumnName == "Slot")
		{
			return SNew(SBox).Padding(FMargin(4, 1))
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->SlotName))
			];
		}
		if (ColumnName == "Result")
		{
			const FString ResultStr = (Item->Result == EPGXSaveResult::Success) ? TEXT("OK")
				: (Item->Result == EPGXSaveResult::Failed) ? TEXT("FAILED")
				: TEXT("WARN");

			return SNew(SBox).Padding(FMargin(4, 1))
			[
				SNew(STextBlock)
				.Text(FText::FromString(ResultStr))
				.ColorAndOpacity(FSlateColor(RowColor))
			];
		}
		return SNullWidget::NullWidget;
	}

private:
	TSharedPtr<FPGXSavePipelineEntry> Item;
};

// ============================================================================
// EN: SMultiColumnTableRow — Slot Browser table
// ES: SMultiColumnTableRow — Tabla de Slot Browser
// ============================================================================

class SPGXSlotTableRow : public SMultiColumnTableRow<TSharedPtr<FPGXSaveSlotInfo>>
{
public:
	SLATE_BEGIN_ARGS(SPGXSlotTableRow) {}
		SLATE_ARGUMENT(TSharedPtr<FPGXSaveSlotInfo>, Item)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable)
	{
		Item = InArgs._Item;
		SMultiColumnTableRow::Construct(FSuperRowType::FArguments(), InOwnerTable);
	}

	TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
	{
		if (!Item.IsValid()) return SNullWidget::NullWidget;

		if (ColumnName == "Slot")
		{
			return SNew(SBox).Padding(FMargin(4, 1))
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->SlotName))
				.Font(PGX::Font::Badge())
			];
		}
		if (ColumnName == "Domains")
		{
			return SNew(SBox).Padding(FMargin(4, 1))
			[
				SNew(STextBlock)
				.Text(FText::AsNumber(Item->DomainCount))
			];
		}
		if (ColumnName == "SaveDate")
		{
			return SNew(SBox).Padding(FMargin(4, 1))
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->SaveDate.ToString(TEXT("%Y-%m-%d %H:%M"))))
				.ToolTipText(FText::FromString(Item->SaveDate.ToString()))
				.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
			];
		}
		if (ColumnName == "Size")
		{
			return SNew(SBox).Padding(FMargin(4, 1))
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("%.1f KB"), static_cast<double>(Item->TotalSizeBytes) / 1024.0)))
			];
		}
		if (ColumnName == "Version")
		{
			return SNew(SBox).Padding(FMargin(4, 1))
			[
				SNew(STextBlock)
				.Text(FText::Format(LOCTEXT("SlotVersionFmt", "v{0}"), FText::AsNumber(Item->SaveVersion)))
			];
		}
		return SNullWidget::NullWidget;
	}

private:
	TSharedPtr<FPGXSaveSlotInfo> Item;
};

// ============================================================================
// EN: SMultiColumnTableRow — Domain Detail table
// ES: SMultiColumnTableRow — Tabla de Detalle de Dominio
// ============================================================================

class SPGXDomainDetailTableRow : public SMultiColumnTableRow<TSharedPtr<FPGXDomainDetailRow>>
{
public:
	SLATE_BEGIN_ARGS(SPGXDomainDetailTableRow) {}
		SLATE_ARGUMENT(TSharedPtr<FPGXDomainDetailRow>, Item)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable)
	{
		Item = InArgs._Item;
		SMultiColumnTableRow::Construct(FSuperRowType::FArguments(), InOwnerTable);
	}

	TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
	{
		if (!Item.IsValid()) return SNullWidget::NullWidget;

		if (ColumnName == "Domain")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->DomainLeafTag))
				.ToolTipText(FText::FromString(Item->DomainFullTag))
				.Font(PGX::Font::Badge())
			];
		}
		if (ColumnName == "Class")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->SaveGameClassName))
				.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
			];
		}
		if (ColumnName == "Status")
		{
			const FText StatusTxt = Item->bHasInstance
				? FText::Format(LOCTEXT("InstanceValid", "v{0}"), FText::AsNumber(Item->SaveFormatVersion))
				: LOCTEXT("InstanceNone", "---");
			const FLinearColor StatusColor = Item->bHasInstance
				? PGX::Semantic::Good
				: PGX::Semantic::Neutral;

			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock)
				.Text(StatusTxt)
				.ColorAndOpacity(FSlateColor(StatusColor))
			];
		}
		if (ColumnName == "Keys")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock)
				.Text(FText::AsNumber(Item->TotalKeys))
				.ToolTipText(FText::FromString(Item->KeyBreakdown))
			];
		}
		if (ColumnName == "Required")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock)
				.Text(Item->bRequired ? LOCTEXT("ReqYes", "Yes") : LOCTEXT("ReqNo", "No"))
				.ColorAndOpacity(Item->bRequired
					? FSlateColor(FLinearColor::White)
					: FSlateColor(PGX::Text::Muted))
			];
		}
		return SNullWidget::NullWidget;
	}

private:
	TSharedPtr<FPGXDomainDetailRow> Item;
};

// ============================================================================
// EN: Construction / Destruction
// ES: Construccion / Destruccion
// ============================================================================

void SPGXSaveInspectorTab::Construct(const FArguments& /*InArgs*/)
{
	PGX_LOG_INFO(LogPGXSaveInspector, TEXT("[SaveInspector] Construct"));

	ChildSlot
	[
		SNew(SPGXPremiumShell)
		.SystemColor(PGX::System::Save)
		.Title(LOCTEXT("HeaderTitle", "PGX SAVE INSPECTOR"))
		.Icon(FPGXEditorStyle::Get().GetBrush("PGXEditor.Icon.SaveInspector"))
		.TitleRightContent()
		[
			SNew(SHorizontalBox)
			// EN: Activity indicator / ES: Indicador de actividad
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0, 0, 12, 0)
			[
				SAssignNew(ActivityIndicator, STextBlock)
				.Text(LOCTEXT("Idle", ""))
				.Font(PGX::Font::Badge())
				.Visibility(EVisibility::Collapsed)
			]
			// EN: Quick Save button / ES: Boton de Quick Save
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 4, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("QuickSave", "Quick Save"))
				.ToolTipText(LOCTEXT("QuickSaveTip", "Quick save the selected context"))
				.IsEnabled_Lambda([this]() -> bool
				{
					return SelectedContextTag.IsValid() && BoundSubsystem.IsValid();
				})
				.OnClicked_Lambda([this]() -> FReply
				{
					UPGXSaveSubsystem* Sub = BoundSubsystem.Get();
					if (IsValid(Sub) && SelectedContextTag.IsValid())
					{
						PGX_LOG_INFO(LogPGXSaveInspector, TEXT("[SaveInspector] Quick Save: %s"), *SelectedContextTag.ToString());
						Sub->QuickSave(SelectedContextTag);
					}
					return FReply::Handled();
				})
			]
			// EN: Save Now button / ES: Boton de Save Now
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 4, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("ForceSave", "Save Now"))
				.ToolTipText(LOCTEXT("ForceSaveTip", "Save the selected context to its active slot"))
				.IsEnabled_Lambda([this]() -> bool
				{
					UPGXSaveSubsystem* Sub = BoundSubsystem.Get();
					return IsValid(Sub) && SelectedContextTag.IsValid()
						&& !Sub->GetActiveSlotName(SelectedContextTag).IsEmpty();
				})
				.OnClicked_Lambda([this]() -> FReply
				{
					UPGXSaveSubsystem* Sub = BoundSubsystem.Get();
					if (IsValid(Sub) && SelectedContextTag.IsValid())
					{
						const FString SlotName = Sub->GetActiveSlotName(SelectedContextTag);
						PGX_LOG_INFO(LogPGXSaveInspector, TEXT("[SaveInspector] Save Now: %s -> %s"),
							*SelectedContextTag.ToString(), *SlotName);
						Sub->SaveContext(SelectedContextTag, SlotName);
					}
					return FReply::Handled();
				})
			]
			// EN: Refresh button / ES: Boton de Refresh
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("Refresh", "Refresh"))
				.ToolTipText(LOCTEXT("RefreshTooltip", "Refresh all panels from subsystem state"))
				.OnClicked(this, &SPGXSaveInspectorTab::OnRefreshClicked)
			]
		]
		.FooterLeftContent()
		[
			SAssignNew(FooterStatusText, STextBlock)
			.Text(LOCTEXT("FooterDisconnected", "Disconnected"))
			.Font(PGX::Font::BodySmall())
			.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
		]
		.Content()
		[
			SAssignNew(ContentSwitcher, SWidgetSwitcher)
			.WidgetIndex(0)

			// ── Slot 0: Empty state (no PIE) ──
			+ SWidgetSwitcher::Slot()
			[
				SNew(SPGXEmptyStateV2)
				.Message(LOCTEXT("EmptyReason", "No PIE session active"))
				.Hint(LOCTEXT("EmptySuggestion", "Start Play-In-Editor to inspect save system state"))
			]

			// ── Slot 1: Live content ──
			+ SWidgetSwitcher::Slot()
			[
				SNew(SVerticalBox)

				// EN: Main content area (scrollable) / ES: Area de contenido principal
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					SNew(SScrollBox)

					// EN: Panel 1 - Active Contexts / ES: Panel 1 - Contextos Activos
					+ SScrollBox::Slot()
					.Padding(4.0f)
					[
						BuildContextPanel()
					]

					+ SScrollBox::Slot()
					.Padding(4.0f, 0.0f)
					[
						SNew(SSeparator)
					]

					// EN: Panel 2 - Domain Detail / ES: Panel 2 - Detalle de Dominio
					+ SScrollBox::Slot()
					.Padding(4.0f)
					[
						BuildDomainDetailPanel()
					]

					+ SScrollBox::Slot()
					.Padding(4.0f, 0.0f)
					[
						SNew(SSeparator)
					]

					// EN: Panel 3 - Pipeline Log / ES: Panel 3 - Pipeline Log
					+ SScrollBox::Slot()
					.Padding(4.0f)
					[
						BuildPipelineLogPanel()
					]

					+ SScrollBox::Slot()
					.Padding(4.0f, 0.0f)
					[
						SNew(SSeparator)
					]

					// EN: Panel 4 - Slot Browser / ES: Panel 4 - Slot Browser
					+ SScrollBox::Slot()
					.Padding(4.0f)
					[
						BuildSlotBrowserPanel()
					]
				]

				// EN: Platform Budgets / ES: Presupuestos de Plataforma
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
						.AccentColor(PGX::System::Save)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(4.0f, 4.0f, 4.0f, 0.0f)
					[
						SNew(STextBlock)
						.Text_Lambda([]() -> FText
						{
							if (UPGXProfileSubsystem* ProfileSS = UPGXProfileSubsystem::GetCachedInstance())
							{
								if (const UPGXPlatformConfig* Cfg = ProfileSS->GetActivePlatformConfig())
								{
									const auto& B = Cfg->SaveBudgets;
									return FText::Format(
										LOCTEXT("SaveBudgetsFmt", "Max File Size: {0} KB | Total Storage: {1} MB | Concurrent IO: {2}"),
										B.MaxSaveFileSize_KB > 0 ? FText::AsNumber(B.MaxSaveFileSize_KB) : LOCTEXT("Unlim1", "Unlimited"),
										B.MaxTotalStorage_MB > 0 ? FText::AsNumber(B.MaxTotalStorage_MB) : LOCTEXT("Unlim2", "Unlimited"),
										B.MaxConcurrentIO > 0 ? FText::AsNumber(B.MaxConcurrentIO) : LOCTEXT("Unlim3", "Unlimited"));
								}
							}
							return LOCTEXT("NoPlatformCfg", "No platform config active");
						})
						.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
					]
				]
			]
		]
	];

	BindPIEDelegates();

	// EN: If PIE is already running when the panel opens, bind immediately
	// ES: Si PIE ya esta corriendo cuando se abre el panel, bindear inmediatamente
	if (GEditor && GEditor->PlayWorld)
	{
		OnPIEStarted(false);
	}
}

SPGXSaveInspectorTab::~SPGXSaveInspectorTab()
{
	PGX_LOG_INFO(LogPGXSaveInspector, TEXT("[SaveInspector] Destructor — unbinding all delegates"));

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
// EN: Tick — Activity Indicator polling
// ES: Tick — Polling del indicador de actividad
// ============================================================================

void SPGXSaveInspectorTab::Tick(
	const FGeometry& AllottedGeometry,
	const double InCurrentTime,
	const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	UPGXSaveSubsystem* Sub = BoundSubsystem.Get();
	if (!IsValid(Sub) || !ActivityIndicator.IsValid())
	{
		return;
	}

	if (Sub->IsSaveInProgress())
	{
		ActivityIndicator->SetText(LOCTEXT("Saving", "SAVING..."));
		ActivityIndicator->SetColorAndOpacity(FSlateColor(PGX::Semantic::Warn));
		ActivityIndicator->SetVisibility(EVisibility::Visible);
	}
	else if (Sub->IsLoadInProgress())
	{
		ActivityIndicator->SetText(LOCTEXT("Loading", "LOADING..."));
		ActivityIndicator->SetColorAndOpacity(FSlateColor(PGX::Semantic::Info));
		ActivityIndicator->SetVisibility(EVisibility::Visible);
	}
	else
	{
		ActivityIndicator->SetVisibility(EVisibility::Collapsed);
	}
}

// ============================================================================
// EN: UI Build
// ES: Construccion de UI
// ============================================================================

// EN: BuildToolbar() — Removed: integrated into SPGXPremiumShell in Construct()
// ES: BuildToolbar() — Eliminado: integrado en SPGXPremiumShell en Construct()

TSharedRef<SWidget> SPGXSaveInspectorTab::BuildContextPanel()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("ContextsHeader", "ACTIVE CONTEXTS"))
			.AccentColor(PGX::System::Save)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.MaxHeight(150.0f)
		[
			SAssignNew(ContextListView, SListView<TSharedPtr<FPGXSaveContextEntry>>)
			.ListItemsSource(&ContextRows)
			.OnGenerateRow(this, &SPGXSaveInspectorTab::OnGenerateContextRow)
			.OnSelectionChanged(this, &SPGXSaveInspectorTab::OnContextSelected)
			.SelectionMode(ESelectionMode::Single)
			.HeaderRow
			(
				SNew(SHeaderRow)
				+ SHeaderRow::Column("Context")
					.DefaultLabel(LOCTEXT("CtxColName", "Context"))
					.FillWidth(1.0f)
				+ SHeaderRow::Column("Mode")
					.DefaultLabel(LOCTEXT("CtxColMode", "Mode"))
					.FillWidth(0.5f)
				+ SHeaderRow::Column("Domains")
					.DefaultLabel(LOCTEXT("CtxColDomains", "Domains"))
					.FillWidth(0.35f)
				+ SHeaderRow::Column("Slots")
					.DefaultLabel(LOCTEXT("CtxColSlots", "Slots"))
					.FillWidth(0.3f)
				+ SHeaderRow::Column("ActiveSlot")
					.DefaultLabel(LOCTEXT("CtxColActive", "Active"))
					.FillWidth(0.5f)
				+ SHeaderRow::Column("AutoSave")
					.DefaultLabel(LOCTEXT("CtxColAuto", "Auto"))
					.FillWidth(0.25f)
			)
		];
}

TSharedRef<SWidget> SPGXSaveInspectorTab::BuildDomainDetailPanel()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("DomainHeader", "DOMAIN DETAIL"))
			.AccentColor(PGX::System::Save)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(DomainNoSelectionText, STextBlock)
			.Text(LOCTEXT("DomainSelectPrompt", "Select a context above to view domain details"))
			.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
			.Visibility(EVisibility::Visible)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.MaxHeight(200.0f)
		[
			SAssignNew(DomainListView, SListView<TSharedPtr<FPGXDomainDetailRow>>)
			.ListItemsSource(&DomainDetailItems)
			.OnGenerateRow(this, &SPGXSaveInspectorTab::OnGenerateDomainDetailRow)
			.Visibility(EVisibility::Collapsed)
			.HeaderRow
			(
				SNew(SHeaderRow)
				+ SHeaderRow::Column("Domain")
					.DefaultLabel(LOCTEXT("DDColDomain", "Domain"))
					.FillWidth(1.0f)
				+ SHeaderRow::Column("Class")
					.DefaultLabel(LOCTEXT("DDColClass", "SaveGame Class"))
					.FillWidth(0.8f)
				+ SHeaderRow::Column("Status")
					.DefaultLabel(LOCTEXT("DDColStatus", "Status"))
					.FillWidth(0.3f)
				+ SHeaderRow::Column("Keys")
					.DefaultLabel(LOCTEXT("DDColKeys", "Keys"))
					.FillWidth(0.3f)
				+ SHeaderRow::Column("Required")
					.DefaultLabel(LOCTEXT("DDColReq", "Req"))
					.FillWidth(0.2f)
			)
		];
}

TSharedRef<SWidget> SPGXSaveInspectorTab::BuildPipelineLogPanel()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("PipelineHeader", "PIPELINE LOG (LIVE)"))
			.AccentColor(PGX::System::Save)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.MaxHeight(200.0f)
		[
			SAssignNew(PipelineListView, SListView<TSharedPtr<FPGXSavePipelineEntry>>)
			.ListItemsSource(&PipelineEntries)
			.OnGenerateRow(this, &SPGXSaveInspectorTab::OnGeneratePipelineRow)
			.SelectionMode(ESelectionMode::None)
			.HeaderRow
			(
				SNew(SHeaderRow)
				+ SHeaderRow::Column("Time")
					.DefaultLabel(LOCTEXT("PipeColTime", "Time"))
					.FillWidth(0.4f)
				+ SHeaderRow::Column("Operation")
					.DefaultLabel(LOCTEXT("PipeColOp", "Operation"))
					.FillWidth(0.4f)
				+ SHeaderRow::Column("Context")
					.DefaultLabel(LOCTEXT("PipeColCtx", "Context"))
					.FillWidth(0.6f)
				+ SHeaderRow::Column("Slot")
					.DefaultLabel(LOCTEXT("PipeColSlot", "Slot"))
					.FillWidth(0.5f)
				+ SHeaderRow::Column("Result")
					.DefaultLabel(LOCTEXT("PipeColResult", "Result"))
					.FillWidth(0.3f)
			)
		];
}

TSharedRef<SWidget> SPGXSaveInspectorTab::BuildSlotBrowserPanel()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("SlotHeader", "SLOT BROWSER"))
			.AccentColor(PGX::System::Save)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.MaxHeight(200.0f)
		[
			SAssignNew(SlotListView, SListView<TSharedPtr<FPGXSaveSlotInfo>>)
			.ListItemsSource(&SlotRows)
			.OnGenerateRow(this, &SPGXSaveInspectorTab::OnGenerateSlotRow)
			.SelectionMode(ESelectionMode::None)
			.HeaderRow
			(
				SNew(SHeaderRow)
				+ SHeaderRow::Column("Slot")
					.DefaultLabel(LOCTEXT("SlotColName", "Slot"))
					.FillWidth(0.6f)
				+ SHeaderRow::Column("Domains")
					.DefaultLabel(LOCTEXT("SlotColDomains", "Domains"))
					.FillWidth(0.35f)
				+ SHeaderRow::Column("SaveDate")
					.DefaultLabel(LOCTEXT("SlotColDate", "Save Date"))
					.FillWidth(0.6f)
				+ SHeaderRow::Column("Size")
					.DefaultLabel(LOCTEXT("SlotColSize", "Size"))
					.FillWidth(0.35f)
				+ SHeaderRow::Column("Version")
					.DefaultLabel(LOCTEXT("SlotColVersion", "Version"))
					.FillWidth(0.35f)
			)
		];
}

// EN: BuildFooter() — Removed: integrated into SPGXPremiumShell in Construct()
// ES: BuildFooter() — Eliminado: integrado en SPGXPremiumShell en Construct()

// ============================================================================
// EN: SListView row generators (delegating to SMultiColumnTableRow)
// ES: Generadores de filas SListView (delegando a SMultiColumnTableRow)
// ============================================================================

TSharedRef<ITableRow> SPGXSaveInspectorTab::OnGenerateContextRow(
	TSharedPtr<FPGXSaveContextEntry> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SPGXContextTableRow, OwnerTable)
		.Item(Item);
}

TSharedRef<ITableRow> SPGXSaveInspectorTab::OnGeneratePipelineRow(
	TSharedPtr<FPGXSavePipelineEntry> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SPGXPipelineTableRow, OwnerTable)
		.Item(Item);
}

TSharedRef<ITableRow> SPGXSaveInspectorTab::OnGenerateSlotRow(
	TSharedPtr<FPGXSaveSlotInfo> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SPGXSlotTableRow, OwnerTable)
		.Item(Item);
}

TSharedRef<ITableRow> SPGXSaveInspectorTab::OnGenerateDomainDetailRow(
	TSharedPtr<FPGXDomainDetailRow> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SPGXDomainDetailTableRow, OwnerTable)
		.Item(Item);
}

// ============================================================================
// EN: Context selection
// ES: Seleccion de contexto
// ============================================================================

void SPGXSaveInspectorTab::OnContextSelected(TSharedPtr<FPGXSaveContextEntry> Item, ESelectInfo::Type /*SelectInfo*/)
{
	if (!Item.IsValid())
	{
		SelectedContextTag = FGameplayTag();
		PGX_LOG_VERBOSE(LogPGXSaveInspector, TEXT("[SaveInspector] Context deselected"));
	}
	else
	{
		SelectedContextTag = Item->ContextTag;
		PGX_LOG_VERBOSE(LogPGXSaveInspector, TEXT("[SaveInspector] Context selected: %s"), *SelectedContextTag.ToString());
	}

	RefreshDomainDetail();
	RefreshSlotBrowser();
}

void SPGXSaveInspectorTab::RefreshContextList()
{
	ContextRows.Empty();

	UPGXSaveSubsystem* Sub = BoundSubsystem.Get();
	if (!IsValid(Sub))
	{
		if (ContextListView.IsValid())
		{
			ContextListView->RequestListRefresh();
		}
		return;
	}

	TArray<FGameplayTag> ContextTags = Sub->GetAllContextTags();
	for (const FGameplayTag& CtxTag : ContextTags)
	{
		const UPGXSaveConfig* Config = Sub->GetContextConfig(CtxTag);
		if (!Config)
		{
			continue;
		}

		TSharedPtr<FPGXSaveContextEntry> Row = MakeShared<FPGXSaveContextEntry>();
		Row->ContextTag = CtxTag;
		Row->DisplayName = Config->ContextDisplayName.IsEmpty()
			? FText::FromString(CtxTag.ToString())
			: Config->ContextDisplayName;

		switch (Config->SaveMode)
		{
		case EPGXSaveMode::SingleSlot:    Row->SaveModeName = TEXT("SingleSlot");    break;
		case EPGXSaveMode::MultiSlot:     Row->SaveModeName = TEXT("MultiSlot");     break;
		case EPGXSaveMode::SessionBased:  Row->SaveModeName = TEXT("SessionBased");  break;
		default:                          Row->SaveModeName = TEXT("Unknown");        break;
		}

		Row->DomainCount = Config->SaveDomains.Num();

		TArray<FPGXSaveSlotInfo> Slots = Sub->GetAllSlots(CtxTag);
		Row->SlotCount = Slots.Num();

		// EN: New fields: ActiveSlotName + AutoSave state / ES: Campos nuevos
		Row->ActiveSlotName = Sub->GetActiveSlotName(CtxTag);
		Row->bAutoSaveActive = Sub->IsAutoSaveActive(CtxTag);

		ContextRows.Add(Row);
	}

	PGX_LOG_VERBOSE(LogPGXSaveInspector, TEXT("[SaveInspector] RefreshContextList: %d contexts"), ContextRows.Num());

	if (ContextListView.IsValid())
	{
		ContextListView->RequestListRefresh();
	}
}

void SPGXSaveInspectorTab::RefreshDomainDetail()
{
	DomainDetailItems.Empty();

	UPGXSaveSubsystem* Sub = BoundSubsystem.Get();
	if (!IsValid(Sub) || !SelectedContextTag.IsValid())
	{
		if (DomainNoSelectionText.IsValid())
		{
			DomainNoSelectionText->SetVisibility(EVisibility::Visible);
		}
		if (DomainListView.IsValid())
		{
			DomainListView->SetVisibility(EVisibility::Collapsed);
		}
		return;
	}

	const UPGXSaveConfig* Config = Sub->GetContextConfig(SelectedContextTag);
	if (!Config)
	{
		if (DomainNoSelectionText.IsValid())
		{
			DomainNoSelectionText->SetText(LOCTEXT("DomainNotFound", "Context config not found"));
			DomainNoSelectionText->SetVisibility(EVisibility::Visible);
		}
		if (DomainListView.IsValid())
		{
			DomainListView->SetVisibility(EVisibility::Collapsed);
		}
		return;
	}

	if (DomainNoSelectionText.IsValid())
	{
		DomainNoSelectionText->SetVisibility(EVisibility::Collapsed);
	}
	if (DomainListView.IsValid())
	{
		DomainListView->SetVisibility(EVisibility::Visible);
	}

	for (const FPGXSaveDomainEntry& Domain : Config->SaveDomains)
	{
		auto Row = MakeShared<FPGXDomainDetailRow>();
		Row->DomainFullTag = Domain.DomainTag.ToString();
		Row->DomainLeafTag = PGXEditorUtils::TagToLeafName(Domain.DomainTag);
		Row->SaveGameClassName = Domain.SaveGameClass
			? Domain.SaveGameClass->GetName() : TEXT("(default)");
		Row->bRequired = Domain.bRequired;

		UPGXSaveGame* SG = Sub->GetSaveGame(Domain.DomainTag);
		if (SG)
		{
			Row->bHasInstance = true;
			Row->SaveFormatVersion = SG->SaveFormatVersion;

			const int32 S = SG->StringData.Num();
			const int32 I = SG->IntData.Num();
			const int32 F = SG->FloatData.Num();
			const int32 B = SG->BoolData.Num();
			const int32 V = SG->VectorData.Num();
			const int32 R = SG->RotatorData.Num();
			const int32 T = SG->TransformData.Num();
			const int32 G = SG->TagData.Num();

			Row->TotalKeys = S + I + F + B + V + R + T + G;
			Row->KeyBreakdown = FString::Printf(
				TEXT("Str:%d Int:%d Float:%d Bool:%d Vec:%d Rot:%d Trans:%d Tag:%d"),
				S, I, F, B, V, R, T, G);
		}

		DomainDetailItems.Add(Row);
	}

	PGX_LOG_VERBOSE(LogPGXSaveInspector, TEXT("[SaveInspector] RefreshDomainDetail: %d domains for %s"),
		DomainDetailItems.Num(), *SelectedContextTag.ToString());

	if (DomainListView.IsValid())
	{
		DomainListView->RequestListRefresh();
	}
}

void SPGXSaveInspectorTab::RefreshSlotBrowser()
{
	SlotRows.Empty();

	UPGXSaveSubsystem* Sub = BoundSubsystem.Get();
	if (IsValid(Sub) && SelectedContextTag.IsValid())
	{
		TArray<FPGXSaveSlotInfo> Slots = Sub->GetAllSlots(SelectedContextTag);
		for (FPGXSaveSlotInfo& Slot : Slots)
		{
			SlotRows.Add(MakeShared<FPGXSaveSlotInfo>(MoveTemp(Slot)));
		}
	}

	PGX_LOG_VERBOSE(LogPGXSaveInspector, TEXT("[SaveInspector] RefreshSlotBrowser: %d slots"), SlotRows.Num());

	if (SlotListView.IsValid())
	{
		SlotListView->RequestListRefresh();
	}
}

// ============================================================================
// EN: PIE Lifecycle
// ES: Ciclo de Vida PIE
// ============================================================================

void SPGXSaveInspectorTab::BindPIEDelegates()
{
	PIEStartedHandle = FEditorDelegates::PostPIEStarted.AddSP(
		SharedThis(this), &SPGXSaveInspectorTab::OnPIEStarted);
	PIEEndedHandle = FEditorDelegates::EndPIE.AddSP(
		SharedThis(this), &SPGXSaveInspectorTab::OnPIEEnded);
	PGX_LOG_INFO(LogPGXSaveInspector, TEXT("[SaveInspector] PIE delegates bound"));
}

void SPGXSaveInspectorTab::OnPIEStarted(bool /*bIsSimulating*/)
{
	PGX_LOG_INFO(LogPGXSaveInspector, TEXT("[SaveInspector] OnPIEStarted"));
	BindToSubsystem();
}

void SPGXSaveInspectorTab::OnPIEEnded(bool /*bIsSimulating*/)
{
	PGX_LOG_INFO(LogPGXSaveInspector, TEXT("[SaveInspector] OnPIEEnded"));
	UnbindFromSubsystem();

	if (ContentSwitcher.IsValid())
	{
		ContentSwitcher->SetActiveWidgetIndex(0);
	}
	if (FooterStatusText.IsValid())
	{
		FooterStatusText->SetText(LOCTEXT("FooterDisconnected", "Disconnected"));
		FooterStatusText->SetColorAndOpacity(FSlateColor(PGX::Text::Muted));
	}
}

void SPGXSaveInspectorTab::BindToSubsystem()
{
	// EN: Find the PIE world's GameInstance and its SaveSubsystem
	// ES: Encontrar el GameInstance del mundo PIE y su SaveSubsystem
	UPGXSaveSubsystem* Sub = nullptr;

	if (GEditor)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::PIE && Context.World())
			{
				UGameInstance* GI = Context.World()->GetGameInstance();
				if (GI)
				{
					Sub = GI->GetSubsystem<UPGXSaveSubsystem>();
					break;
				}
			}
		}
	}

	if (!Sub)
	{
		PGX_LOG_WARNING(LogPGXSaveInspector, TEXT("[SaveInspector] PIE active but SaveSubsystem not found"));
		if (FooterStatusText.IsValid())
		{
			FooterStatusText->SetText(LOCTEXT("FooterNoSubsystem", "PIE active but SaveSubsystem not found"));
			FooterStatusText->SetColorAndOpacity(FSlateColor(PGX::Semantic::Warn));
		}
		return;
	}

	BoundSubsystem = Sub;
	PGX_LOG_INFO(LogPGXSaveInspector, TEXT("[SaveInspector] BindToSubsystem — connected"));

	// EN: Bind to native delegates for live updates
	// ES: Bindear a delegates nativos para actualizaciones en vivo
	SaveCompletedHandle = Sub->OnSaveCompletedNative.AddSP(
		SharedThis(this), &SPGXSaveInspectorTab::OnSaveCompleted);
	LoadCompletedHandle = Sub->OnLoadCompletedNative.AddSP(
		SharedThis(this), &SPGXSaveInspectorTab::OnLoadCompleted);
	SlotDeletedHandle = Sub->OnSlotDeletedNative.AddSP(
		SharedThis(this), &SPGXSaveInspectorTab::OnSlotDeleted);
	AutoSaveHandle = Sub->OnAutoSaveTriggeredNative.AddSP(
		SharedThis(this), &SPGXSaveInspectorTab::OnAutoSaveTriggered);

	if (ContentSwitcher.IsValid())
	{
		ContentSwitcher->SetActiveWidgetIndex(1);
	}
	if (FooterStatusText.IsValid())
	{
		FooterStatusText->SetText(FText::Format(
			LOCTEXT("FooterConnected", "Connected to PIE | {0} context(s)"),
			FText::AsNumber(Sub->GetContextCount())));
		FooterStatusText->SetColorAndOpacity(FSlateColor(PGX::Semantic::Good));
	}

	// EN: Initial data refresh / ES: Refresh inicial de datos
	RefreshContextList();
	RefreshDomainDetail();
	RefreshSlotBrowser();
}

void SPGXSaveInspectorTab::UnbindFromSubsystem()
{
	UPGXSaveSubsystem* Sub = BoundSubsystem.Get();
	if (Sub)
	{
		if (SaveCompletedHandle.IsValid())
		{
			Sub->OnSaveCompletedNative.Remove(SaveCompletedHandle);
			SaveCompletedHandle.Reset();
		}
		if (LoadCompletedHandle.IsValid())
		{
			Sub->OnLoadCompletedNative.Remove(LoadCompletedHandle);
			LoadCompletedHandle.Reset();
		}
		if (SlotDeletedHandle.IsValid())
		{
			Sub->OnSlotDeletedNative.Remove(SlotDeletedHandle);
			SlotDeletedHandle.Reset();
		}
		if (AutoSaveHandle.IsValid())
		{
			Sub->OnAutoSaveTriggeredNative.Remove(AutoSaveHandle);
			AutoSaveHandle.Reset();
		}
		PGX_LOG_INFO(LogPGXSaveInspector, TEXT("[SaveInspector] UnbindFromSubsystem — all delegates removed"));
	}

	BoundSubsystem.Reset();
}

// ============================================================================
// EN: Delegate callbacks
// ES: Callbacks de delegates
// ============================================================================

void SPGXSaveInspectorTab::OnSaveCompleted(const FString& SlotName, EPGXSaveResult Result)
{
	PGX_LOG_VERBOSE(LogPGXSaveInspector, TEXT("[SaveInspector] OnSaveCompleted: %s -> %s"),
		*SlotName, Result == EPGXSaveResult::Success ? TEXT("OK") : TEXT("FAIL"));

	TSharedPtr<FPGXSavePipelineEntry> Entry = MakeShared<FPGXSavePipelineEntry>();
	Entry->Timestamp = FDateTime::Now();
	Entry->OperationType = TEXT("Save");
	Entry->ContextName = SelectedContextTag.IsValid() ? SelectedContextTag.ToString() : TEXT("-");
	Entry->SlotName = SlotName;
	Entry->Result = Result;
	PipelineEntries.Insert(Entry, 0);

	if (PipelineListView.IsValid())
	{
		PipelineListView->RequestListRefresh();
	}

	RefreshSlotBrowser();
	RefreshContextList();
}

void SPGXSaveInspectorTab::OnLoadCompleted(const FString& SlotName, EPGXSaveResult Result, UPGXSaveGame* /*SaveGame*/)
{
	PGX_LOG_VERBOSE(LogPGXSaveInspector, TEXT("[SaveInspector] OnLoadCompleted: %s -> %s"),
		*SlotName, Result == EPGXSaveResult::Success ? TEXT("OK") : TEXT("FAIL"));

	TSharedPtr<FPGXSavePipelineEntry> Entry = MakeShared<FPGXSavePipelineEntry>();
	Entry->Timestamp = FDateTime::Now();
	Entry->OperationType = TEXT("Load");
	Entry->ContextName = SelectedContextTag.IsValid() ? SelectedContextTag.ToString() : TEXT("-");
	Entry->SlotName = SlotName;
	Entry->Result = Result;
	PipelineEntries.Insert(Entry, 0);

	if (PipelineListView.IsValid())
	{
		PipelineListView->RequestListRefresh();
	}

	RefreshDomainDetail();
}

void SPGXSaveInspectorTab::OnSlotDeleted(const FString& SlotName)
{
	PGX_LOG_VERBOSE(LogPGXSaveInspector, TEXT("[SaveInspector] OnSlotDeleted: %s"), *SlotName);

	TSharedPtr<FPGXSavePipelineEntry> Entry = MakeShared<FPGXSavePipelineEntry>();
	Entry->Timestamp = FDateTime::Now();
	Entry->OperationType = TEXT("Delete");
	Entry->ContextName = SelectedContextTag.IsValid() ? SelectedContextTag.ToString() : TEXT("-");
	Entry->SlotName = SlotName;
	Entry->Result = EPGXSaveResult::Success;
	PipelineEntries.Insert(Entry, 0);

	if (PipelineListView.IsValid())
	{
		PipelineListView->RequestListRefresh();
	}

	RefreshSlotBrowser();
	RefreshContextList();
}

void SPGXSaveInspectorTab::OnAutoSaveTriggered(FGameplayTag ContextTag)
{
	PGX_LOG_INFO(LogPGXSaveInspector, TEXT("[SaveInspector] OnAutoSaveTriggered: %s"), *ContextTag.ToString());

	TSharedPtr<FPGXSavePipelineEntry> Entry = MakeShared<FPGXSavePipelineEntry>();
	Entry->Timestamp = FDateTime::Now();
	Entry->OperationType = TEXT("AutoSave");
	Entry->ContextName = ContextTag.ToString();
	Entry->SlotName = TEXT("(auto)");
	Entry->Result = EPGXSaveResult::Success;
	PipelineEntries.Insert(Entry, 0);

	if (PipelineListView.IsValid())
	{
		PipelineListView->RequestListRefresh();
	}
}

// ============================================================================
// EN: Actions
// ES: Acciones
// ============================================================================

FReply SPGXSaveInspectorTab::OnRefreshClicked()
{
	PGX_LOG_INFO(LogPGXSaveInspector, TEXT("[SaveInspector] Manual refresh"));
	RefreshContextList();
	RefreshDomainDetail();
	RefreshSlotBrowser();
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
