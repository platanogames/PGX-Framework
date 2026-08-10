// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Inspector/SPGXVersionControlInspectorTab.h"
#include "PGXSourceControlSubsystem.h"
#include "Logging/PGXLogMacros.h"
#include "PGXVersionControlEditor.h"

#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Views/SListView.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "Framework/Docking/TabManager.h"
#include "ISettingsModule.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Editor.h"
#include "Utils/PGXEditorUtils.h"
#include "Style/PGXVisualTokens.h"
#include "Style/PGXEditorStyle.h"
#include "Widgets/SPGXPremiumShell.h"
#include "Widgets/SPGXSectionDivider.h"
#include "Widgets/SPGXFooterBar.h"
#include "Widgets/SPGXStatusBadge.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/SWindow.h"

#define LOCTEXT_NAMESPACE "PGXVersionControlInspector"

// EN: VCS system color uses the shared PGX::System::VersionControl token.
// ES: El color del sistema VCS usa el token compartido PGX::System::VersionControl.

// ============================================================================
// EN: Construct / Destructor / ES: Construccion / Destructor
// ============================================================================

void SPGXVersionControlInspectorTab::Construct(const FArguments& /*InArgs*/)
{
	PGX_LOG_INFO(LogPGXVersionControl, TEXT("[VCS] Construct"));

	// EN: S1: Bind changelists-changed delegate / ES: S1: Vincular delegate de changelists-changed
	if (UPGXSourceControlSubsystem* Sub = GetSubsystem())
	{
		Sub->GetOnChangelistsChanged().AddSP(SharedThis(this), &SPGXVersionControlInspectorTab::OnChangelistsChanged);
	}

	ChildSlot
	[
		SNew(SPGXPremiumShell)
		.SystemColor(PGX::System::VersionControl)
		.Title(LOCTEXT("VCSTitle", "VERSION CONTROL"))
		// Use the dedicated panel icon while preserving the general icon for other consumers.
		.Icon(FPGXEditorStyle::Get().GetBrush("PGXEditor.Icon.VersionControlPanel"))
		.bShowFooter(true)
		.TitleRightContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, PGX::Spacing::SM, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("Refresh", "Refresh"))
				.ToolTipText(LOCTEXT("RefreshTip", "Refresh provider status and changelists"))
				.OnClicked_Lambda([this]() { RefreshData(); return FReply::Handled(); })
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, PGX::Spacing::SM, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("ValidateAll", "Validate All"))
				.ToolTipText(LOCTEXT("ValidateAllTip", "Run PGX pre-commit validation on all pending files"))
				.OnClicked_Lambda([this]() { OnValidateClicked(); return FReply::Handled(); })
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("OpenUESC", "Open UE Source Control"))
				.ToolTipText(LOCTEXT("OpenUESCTip", "Open Unreal Engine Source Control settings"))
				.OnClicked_Lambda([]() { FGlobalTabmanager::Get()->TryInvokeTab(FName("SourceControl")); return FReply::Handled(); })
			]
		]
		.FooterLeftContent()
		[
			SAssignNew(FooterText, STextBlock)
			.Text(LOCTEXT("FooterReady", "Ready"))
			.Font(PGX::Font::Hint())
			.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
		]
		.Content()
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			.Padding(PGX::Spacing::MD)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 0.0f, 0.0f, 4.0f)
				[
					BuildProviderStatusSection()
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 0.0f, 0.0f, 4.0f)
				[
					BuildChangelistSection()
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 0.0f, 0.0f, 4.0f)
				[
					BuildWorkflowSection()
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					BuildQuickActionsSection()
				]
			]
		]
	];

	RefreshData();
}

SPGXVersionControlInspectorTab::~SPGXVersionControlInspectorTab()
{
	PGX_LOG_INFO(LogPGXVersionControl, TEXT("[VCS] Destructor — cleanup"));
	// EN: S1: Unbind changelists-changed delegate / ES: S1: Desvincular delegate de changelists-changed
	if (UPGXSourceControlSubsystem* Sub = GetSubsystem())
	{
		Sub->GetOnChangelistsChanged().RemoveAll(this);
	}
}

void SPGXVersionControlInspectorTab::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	RefreshAccumulator += InDeltaTime;
	if (RefreshAccumulator >= RefreshInterval)
	{
		RefreshAccumulator = 0.0f;
		RefreshData();
	}
}

UPGXSourceControlSubsystem* SPGXVersionControlInspectorTab::GetSubsystem() const
{
	if (GEditor)
	{
		return GEditor->GetEditorSubsystem<UPGXSourceControlSubsystem>();
	}
	return nullptr;
}

// ============================================================================
// EN: Section Builders / ES: Constructores de Seccion
// ============================================================================

// EN: BuildToolbar() — Removed: integrated into SPGXPremiumShell.TitleRightContent in Construct()
// ES: BuildToolbar() — Eliminado: integrado en SPGXPremiumShell.TitleRightContent en Construct()

TSharedRef<SWidget> SPGXVersionControlInspectorTab::BuildProviderStatusSection()
{
	return SNew(SBorder)
		.BorderImage(FPGXEditorStyle::Get().GetBrush("PGXEditor.Premium.Card"))
		.Padding(6.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[ SNew(SPGXSectionDivider).Title(LOCTEXT("SectionProvider", "PROVIDER STATUS")).AccentColor(PGX::System::VersionControl) ]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f, 0.0f, 0.0f)
			[
				SNew(SHorizontalBox)

				// EN: Provider KPI / ES: KPI de proveedor
				+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 16, 0).VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock).Text(LOCTEXT("ProviderLabel2", "PROVIDER"))
						.Font(PGX::Font::CaptionBold())
						.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						SAssignNew(ProviderNameText, STextBlock).Text(LOCTEXT("ProviderNone", "\x2014"))
						.Font(PGX::Font::SubHeader())
					]
				]

				// EN: Branch KPI / ES: KPI de rama
				+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 16, 0).VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock).Text(LOCTEXT("BranchLabel2", "BRANCH"))
						.Font(PGX::Font::CaptionBold())
						.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						SAssignNew(BranchText, STextBlock).Text(LOCTEXT("BranchUnknown", "\x2014"))
						.Font(PGX::Font::Mono())
					]
				]

				// EN: Status KPI / ES: KPI de estado
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock).Text(LOCTEXT("StatusLabel2", "STATUS"))
						.Font(PGX::Font::CaptionBold())
						.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						SAssignNew(ConnectionStatusText, STextBlock).Text(LOCTEXT("StatusUnknown", "\x2014"))
						.Font(PGX::Font::SubHeader())
					]
				]
			]
		];
}

TSharedRef<SWidget> SPGXVersionControlInspectorTab::BuildChangelistSection()
{
	return SNew(SBorder)
		.BorderImage(FPGXEditorStyle::Get().GetBrush("PGXEditor.Premium.Card"))
		.Padding(6.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SPGXSectionDivider)
				.Title(LOCTEXT("SectionCL", "CHANGELISTS"))
				.AccentColor(PGX::System::VersionControl)
				.RightContent()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, PGX::Spacing::SM, 0.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("NewCL", "+ New"))
						.ToolTipText(LOCTEXT("NewCLTip", "Create a new changelist"))
						.OnClicked_Lambda([this]() { OnCreateChangelistClicked(); return FReply::Handled(); })
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, PGX::Spacing::SM, 0.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("DeleteCL", "Delete"))
						.ToolTipText(LOCTEXT("DeleteCLTip", "Delete selected changelist (moves files to Default)"))
						.OnClicked_Lambda([this]() { OnDeleteChangelistClicked(); return FReply::Handled(); })
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(SButton)
						.Text(LOCTEXT("RenameCL", "Rename"))
						.ToolTipText(LOCTEXT("RenameCLTip", "Rename selected changelist"))
						.OnClicked_Lambda([this]() { OnRenameChangelistClicked(); return FReply::Handled(); })
					]
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.MinDesiredHeight(120.0f)
				.MaxDesiredHeight(300.0f)
				[
					SAssignNew(ChangelistListView, SListView<TSharedPtr<FPGXChangelist>>)
					.ListItemsSource(&ChangelistItems)
					.OnGenerateRow(this, &SPGXVersionControlInspectorTab::GenerateChangelistRow)
					.OnSelectionChanged_Lambda([this](TSharedPtr<FPGXChangelist> InItem, ESelectInfo::Type)
					{
						SelectedChangelist = InItem;
					})
				]
			]
		];
}

TSharedRef<SWidget> SPGXVersionControlInspectorTab::BuildWorkflowSection()
{
	return SNew(SBorder)
		.BorderImage(FPGXEditorStyle::Get().GetBrush("PGXEditor.Premium.Card"))
		.Padding(6.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[ SNew(SPGXSectionDivider).Title(LOCTEXT("SectionWorkflow", "PGX WORKFLOW")).AccentColor(PGX::System::VersionControl) ]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f, 0.0f, 2.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f).VAlign(VAlign_Center)
				[
					SNew(STextBlock).Text(LOCTEXT("SystemsDetected", "Systems:"))
					.Font(PGX::Font::Badge())
				]
				+ SHorizontalBox::Slot().FillWidth(1.0f)
				[
					SAssignNew(SystemTagsBox, SWrapBox)
					.UseAllottedSize(true)
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 2.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)
				[
					SNew(STextBlock).Text(LOCTEXT("SuggestedPrefix", "Suggested prefix:"))
				]
				+ SHorizontalBox::Slot().FillWidth(1.0f)
				[
					SAssignNew(SuggestedPrefixText, STextBlock).Text(LOCTEXT("NoPrefix", "(none)"))
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 4.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)
				[
					SNew(STextBlock).Text(LOCTEXT("Template", "Template:"))
				]
				+ SHorizontalBox::Slot().FillWidth(1.0f)
				[
					SAssignNew(TemplateText, STextBlock)
					.Text(LOCTEXT("NoTemplate", "(none)"))
					.AutoWrapText(true)
					.ColorAndOpacity(FSlateColor(PGX::Semantic::Info))
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 2.0f)
			[
				SNew(SSeparator)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, PGX::Spacing::SM, 0.0f, PGX::Spacing::SM)
			[
				SNew(SPGXSectionDivider)
				.Title(LOCTEXT("SectionValidation", "PRE-COMMIT VALIDATION"))
				.AccentColor(PGX::System::VersionControl)
				.RightContent()
				[
					SNew(SButton)
					.Text(LOCTEXT("Validate", "Validate"))
					.OnClicked_Lambda([this]() { OnValidateClicked(); return FReply::Handled(); })
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(ValidationResultsBox, SVerticalBox)
			]
		];
}

TSharedRef<SWidget> SPGXVersionControlInspectorTab::BuildQuickActionsSection()
{
	return SNew(SBorder)
		.BorderImage(FPGXEditorStyle::Get().GetBrush("PGXEditor.Premium.Card"))
		.Padding(6.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[ SNew(SPGXSectionDivider).Title(LOCTEXT("SectionQuick", "QUICK ACTIONS")).AccentColor(PGX::System::VersionControl) ]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f, 0.0f, 0.0f)
			[
				SNew(SWrapBox)
				.UseAllottedSize(true)

				+ SWrapBox::Slot().Padding(0.0f, 0.0f, 4.0f, 4.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("OpenSettings", "Open Settings"))
					.ToolTipText(LOCTEXT("OpenSettingsTip", "Open PGX Version Control project settings"))
					.OnClicked_Lambda([]()
					{
						FModuleManager::LoadModuleChecked<ISettingsModule>("Settings")
							.ShowViewer("Project", "PGX", "VersionControl");
						return FReply::Handled();
					})
				]

				+ SWrapBox::Slot().Padding(0.0f, 0.0f, 4.0f, 4.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("CopyTemplate", "Copy Template"))
					.ToolTipText(LOCTEXT("CopyTemplateTip", "Copy commit message template to clipboard"))
					.OnClicked_Lambda([this]()
					{
						if (UPGXSourceControlSubsystem* Sub = GetSubsystem())
						{
							const TArray<FPGXChangelist>& CLs = Sub->GetChangelists();
							if (CLs.Num() > 0)
							{
								const FString Template = Sub->GetCommitTemplate(CLs[0].Guid);
								FPlatformApplicationMisc::ClipboardCopy(*Template);
							}
						}
						return FReply::Handled();
					})
				]
			]
		];
}

// ============================================================================
// EN: List View / ES: Vista de Lista
// ============================================================================

TSharedRef<ITableRow> SPGXVersionControlInspectorTab::GenerateChangelistRow(
	TSharedPtr<FPGXChangelist> InItem,
	const TSharedRef<STableViewBase>& InOwnerTable)
{
	return SNew(STableRow<TSharedPtr<FPGXChangelist>>, InOwnerTable)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4.0f, 2.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("%s%s (%d files)"),
					*InItem->DisplayName,
					InItem->bIsDefault ? TEXT(" [Default]") : TEXT(""),
					InItem->FilePaths.Num())))
				.Font(InItem->bIsDefault
					? PGX::Font::SubHeader()
					: PGX::Font::Body())
			]
		];
}

// ============================================================================
// EN: Refresh & Actions / ES: Refresco & Acciones
// ============================================================================

void SPGXVersionControlInspectorTab::RefreshData()
{
	UPGXSourceControlSubsystem* Sub = GetSubsystem();
	if (!Sub) return;

	// EN: Provider status / ES: Estado del provider
	if (ProviderNameText.IsValid())
	{
		ProviderNameText->SetText(FText::FromString(Sub->GetProviderName()));
	}
	if (BranchText.IsValid())
	{
		BranchText->SetText(FText::FromString(Sub->GetCurrentBranch()));
	}
	if (ConnectionStatusText.IsValid())
	{
		const bool bConnected = Sub->IsProviderConnected();
		ConnectionStatusText->SetText(bConnected
			? LOCTEXT("Connected", "Connected")
			: LOCTEXT("Disconnected", "Disconnected"));
		ConnectionStatusText->SetColorAndOpacity(bConnected
			? FSlateColor(PGX::Semantic::Good)
			: FSlateColor(PGX::Semantic::Error));
	}

	// EN: Changelists / ES: Changelists
	ChangelistItems.Empty();
	for (const FPGXChangelist& CL : Sub->GetChangelists())
	{
		ChangelistItems.Add(MakeShared<FPGXChangelist>(CL));
	}
	if (ChangelistListView.IsValid())
	{
		ChangelistListView->RequestListRefresh();
	}

	// EN: Workflow tags as colored badges / ES: Tags de workflow como badges de color
	const TArray<FString> Tags = Sub->GetAutoDetectedSystemTags();
	if (SystemTagsBox.IsValid())
	{
		SystemTagsBox->ClearChildren();
		if (Tags.IsEmpty())
		{
			SystemTagsBox->AddSlot().Padding(0, 0, 4, 2)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("NoTagsFound", "(none)"))
				.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
			];
		}
		else
		{
			for (const FString& DetectedTag : Tags)
			{
				SystemTagsBox->AddSlot().Padding(0, 0, 4, 2)
				[
					SNew(SBorder)
					.BorderBackgroundColor([]() { FLinearColor C = PGX::System::VersionControl; C.A = 0.3f; return C; }())
					.Padding(FMargin(6.0f, 2.0f))
					[
						SNew(STextBlock)
						.Text(FText::FromString(DetectedTag))
						.Font(PGX::Font::CaptionBold())
						.ColorAndOpacity(FSlateColor(PGX::System::VersionControl))
					]
				];
			}
		}
	}

	if (SuggestedPrefixText.IsValid())
	{
		FString Prefix;
		for (const FString& TagName : Tags)  // EN: Renamed to avoid shadowing SWidget::Tag / ES: Renombrado para evitar ocultar SWidget::Tag
		{
			Prefix += FString::Printf(TEXT("[%s]"), *TagName);
		}
		SuggestedPrefixText->SetText(Prefix.IsEmpty()
			? LOCTEXT("NoPrefixCalc", "(none)")
			: FText::FromString(Prefix));
	}

	if (TemplateText.IsValid())
	{
		const TArray<FPGXChangelist>& CLs = Sub->GetChangelists();
		if (CLs.Num() > 0)
		{
			TemplateText->SetText(FText::FromString(Sub->GetCommitTemplate(CLs[0].Guid)));
		}
	}

	// EN: Update footer / ES: Actualizar footer
	if (FooterText.IsValid())
	{
		const bool bConn = Sub->IsProviderConnected();
		FString FooterStr = bConn
			? FString::Printf(TEXT("%s — %s — %d changelists — %d tags"),
				*Sub->GetProviderName(), *Sub->GetCurrentBranch(),
				ChangelistItems.Num(), Tags.Num())
			: TEXT("Disconnected");
		FooterText->SetText(FText::FromString(FooterStr));
	}
}

void SPGXVersionControlInspectorTab::OnChangelistsChanged()
{
	RefreshData();
}

void SPGXVersionControlInspectorTab::OnValidateClicked()
{
	PGX_LOG_INFO(LogPGXVersionControl, TEXT("[VCS] Validate clicked"));
	UPGXSourceControlSubsystem* Sub = GetSubsystem();
	if (!Sub || !ValidationResultsBox.IsValid()) return;

	CachedValidationIssues = Sub->ValidatePending();
	ValidationResultsBox->ClearChildren();

	if (CachedValidationIssues.IsEmpty())
	{
		ValidationResultsBox->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 2.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("AllPassed", "All checks passed."))
				.ColorAndOpacity(FSlateColor(PGX::Semantic::Good))
			];
		return;
	}

	for (const FPGXValidationIssue& Issue : CachedValidationIssues)
	{
		FLinearColor Color;
		FString Icon;
		switch (Issue.Severity)
		{
		case EPGXValidationSeverity::Error:   Color = PGX::Semantic::Error; Icon = TEXT("X"); break;
		case EPGXValidationSeverity::Warning: Color = PGX::Semantic::Warn; Icon = TEXT("!"); break;
		default:                              Color = PGX::Semantic::Info; Icon = TEXT("i"); break;
		}

		ValidationResultsBox->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 1.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Icon))
					.ColorAndOpacity(FSlateColor(Color))
				]
				+ SHorizontalBox::Slot().FillWidth(1.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(FString::Printf(TEXT("%s — %s (%s)"),
						*Issue.RuleId, *Issue.Message, *FPaths::GetCleanFilename(Issue.FilePath))))
					.AutoWrapText(true)
					.ColorAndOpacity(FSlateColor(Color))
				]
			];
	}
}

void SPGXVersionControlInspectorTab::OnCreateChangelistClicked()
{
	UPGXSourceControlSubsystem* Sub = GetSubsystem();
	if (!Sub) return;

	// EN: Create with auto-generated name / ES: Crear con nombre auto-generado
	static int32 Counter = 1;
	const FString Name = FString::Printf(TEXT("Changelist %d"), Counter++);
	PGX_LOG_INFO(LogPGXVersionControl, TEXT("[VCS] Create changelist '%s'"), *Name);
	Sub->CreateChangelist(Name, TEXT(""));
	RefreshData();
}

void SPGXVersionControlInspectorTab::OnDeleteChangelistClicked()
{
	UPGXSourceControlSubsystem* Sub = GetSubsystem();
	if (!Sub || !SelectedChangelist.IsValid()) return;

	if (SelectedChangelist->bIsDefault)
	{
		PGX_LOG_WARNING(LogPGXVersionControl, TEXT("Cannot delete the default changelist."));
		return;
	}

	Sub->DeleteChangelist(SelectedChangelist->Guid);
	SelectedChangelist.Reset();
	RefreshData();
}

void SPGXVersionControlInspectorTab::OnRenameChangelistClicked()
{
	UPGXSourceControlSubsystem* Sub = GetSubsystem();
	if (!Sub || !SelectedChangelist.IsValid()) return;

	if (SelectedChangelist->bIsDefault)
	{
		PGX_LOG_WARNING(LogPGXVersionControl, TEXT("[VCS] Cannot rename the default changelist."));
		return;
	}

	// EN: Modal dialog for rename / ES: Dialogo modal para renombrar
	TSharedRef<SWindow> RenameWindow = SNew(SWindow)
		.Title(LOCTEXT("RenameCLTitle", "Rename Changelist"))
		.SizingRule(ESizingRule::Autosized)
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		.IsTopmostWindow(true);

	TSharedPtr<SEditableTextBox> NameInput;
	FGuid CapturedGuid = SelectedChangelist->Guid;
	bool bConfirmed = false;
	FString NewName;

	RenameWindow->SetContent(
		SNew(SBox)
		.MinDesiredWidth(300.0f)
		.Padding(16.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("RenameCLPrompt", "Enter new name:"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)
			[
				SAssignNew(NameInput, SEditableTextBox)
				.Text(FText::FromString(SelectedChangelist->DisplayName))
				.SelectAllTextWhenFocused(true)
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f)
				[ SNullWidget::NullWidget ]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)
				[
					SNew(SButton)
					.Text(LOCTEXT("RenameOK", "Rename"))
					.OnClicked_Lambda([&bConfirmed, &NewName, &NameInput, &RenameWindow]()
					{
						if (NameInput.IsValid())
						{
							NewName = NameInput->GetText().ToString().TrimStartAndEnd();
						}
						bConfirmed = true;
						RenameWindow->RequestDestroyWindow();
						return FReply::Handled();
					})
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("RenameCancel", "Cancel"))
					.OnClicked_Lambda([&RenameWindow]()
					{
						RenameWindow->RequestDestroyWindow();
						return FReply::Handled();
					})
				]
			]
		]
	);

	GEditor->EditorAddModalWindow(RenameWindow);

	if (bConfirmed && !NewName.IsEmpty())
	{
		PGX_LOG_INFO(LogPGXVersionControl, TEXT("[VCS] Rename changelist -> '%s'"), *NewName);
		Sub->RenameChangelist(CapturedGuid, NewName);
		RefreshData();
	}
}

#undef LOCTEXT_NAMESPACE
