// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Widgets/SPGXLiveInspectorBase.h"

#include "Widgets/SPGXPremiumShell.h"
#include "Style/PGXVisualTokens.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Editor.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

#define LOCTEXT_NAMESPACE "PGXLiveInspectorBase"
DEFINE_LOG_CATEGORY_STATIC(LogPGXLiveInspector, Log, All);

SPGXLiveInspectorBase::~SPGXLiveInspectorBase()
{
	if (GEditor)
	{
		FEditorDelegates::PostPIEStarted.Remove(PIEStartedHandle);
		FEditorDelegates::EndPIE.Remove(PIEEndedHandle);
	}
	// EN: Use the tracked flag, NOT the virtual GetBindMode() — during destruction the
	//     vtable is the base's, so the derived override would not run.
	// ES: Usar el flag rastreado, NO el virtual GetBindMode() (vtable es del base en dtor).
	if (bDelegatesBound)
	{
		UnbindSubsystemDelegates();
		bDelegatesBound = false;
	}
}

void SPGXLiveInspectorBase::BuildLiveLayout()
{
	// EN: Status bar is referenced by the shell footer — initialize it first.
	// ES: El status bar va en el footer del shell — inicializarlo primero.
	SAssignNew(StatusBarWidget, STextBlock)
		.Font(PGX::Font::BodySmall())
		.ColorAndOpacity(FSlateColor(PGX::Text::Muted));
	SetStatusBarIdle();

	ChildSlot
	[
		SNew(SPGXPremiumShell)
		.SystemColor(GetSystemColor())
		.Title(GetInspectorTitle())
		.TitleRightContent()
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0, 0, 8, 0)
			[
				BuildKPIChips()
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("Refresh", "Refresh"))
				.ToolTipText(LOCTEXT("RefreshTooltip", "Refresh all panels from subsystem state"))
				.OnClicked(this, &SPGXLiveInspectorBase::OnRefreshClicked)
			]
		]
		.FooterLeftContent()
		[
			StatusBarWidget.ToSharedRef()
		]
		.Content()
		[
			BuildBody()
		]
	];
}

void SPGXLiveInspectorBase::BindPIEDelegates()
{
	PIEStartedHandle = FEditorDelegates::PostPIEStarted.AddSP(
		SharedThis(this), &SPGXLiveInspectorBase::OnPIEStarted);
	PIEEndedHandle = FEditorDelegates::EndPIE.AddSP(
		SharedThis(this), &SPGXLiveInspectorBase::OnPIEEnded);
}

void SPGXLiveInspectorBase::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	// EN: Poll-mode only: re-pull on cadence while PIE is active.
	// ES: Solo poll-mode: re-pull por cadencia mientras PIE esta activo.
	if (!bIsPIEActive || GetBindMode() != EPGXLiveBindMode::Poll)
	{
		return;
	}

	if (InCurrentTime - LastPollTime >= GetPollIntervalSeconds())
	{
		LastPollTime = InCurrentTime;
		RefreshAll();
	}
}

void SPGXLiveInspectorBase::OnPIEStarted(bool /*bIsSimulating*/)
{
	bIsPIEActive = true;
	LastPollTime = 0.0;
	SetStatusBarLive();

	if (GetBindMode() == EPGXLiveBindMode::Delegate)
	{
		BindSubsystemDelegates();
		bDelegatesBound = true;
	}
	RefreshAll();
}

void SPGXLiveInspectorBase::OnPIEEnded(bool /*bIsSimulating*/)
{
	bIsPIEActive = false;

	if (bDelegatesBound)
	{
		UnbindSubsystemDelegates();
		bDelegatesBound = false;
	}

	// EN: POST-PIE DATA RETENTION (doctrine): do NOT clear the panels — keep the last
	//     frame's data so the user can inspect it after PIE ends. Only the status bar
	//     changes to "PIE Ended (data retained)".
	// ES: RETENCION POST-PIE (doctrina): NO limpiar los paneles — conservar los datos
	//     del ultimo frame. Solo el status bar cambia.
	SetStatusBarRetained();
}

FReply SPGXLiveInspectorBase::OnRefreshClicked()
{
	RefreshAll();
	return FReply::Handled();
}

void SPGXLiveInspectorBase::RefreshAll()
{
	// EN: Only re-pull while PIE is active. Outside PIE
	//     (e.g. clicking Refresh after PIE ended) RebuildPanels would reset the rows before
	//     it can read a live subsystem, wiping the retained data the base promises. Skip the
	//     rebuild so the last frame's data is preserved.
	// ES: GUARD DE RETENCION: solo re-pull con PIE activo. Fuera de PIE el rebuild borraria
	//     los datos retenidos antes de poder leer el subsystem.
	if (!bIsPIEActive)
	{
		return;
	}
	RebuildPanels();
}

UGameInstance* SPGXLiveInspectorBase::GetPIEGameInstance() const
{
	if (!GEditor)
	{
		return nullptr;
	}
	for (const FWorldContext& Context : GEditor->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::PIE && Context.World())
		{
			if (UGameInstance* GI = Context.World()->GetGameInstance())
			{
				return GI;
			}
		}
	}
	return nullptr;
}

void SPGXLiveInspectorBase::SetStatusBarLive()
{
	if (StatusBarWidget.IsValid())
	{
		StatusBarWidget->SetText(LOCTEXT("StatusLive", "LIVE — Polling"));
		StatusBarWidget->SetColorAndOpacity(FSlateColor(PGX::Semantic::Good));
	}
}

void SPGXLiveInspectorBase::SetStatusBarIdle()
{
	if (StatusBarWidget.IsValid())
	{
		StatusBarWidget->SetText(LOCTEXT("StatusIdle", "Waiting for PIE"));
		StatusBarWidget->SetColorAndOpacity(FSlateColor(PGX::Text::Muted));
	}
}

void SPGXLiveInspectorBase::SetStatusBarRetained()
{
	if (StatusBarWidget.IsValid())
	{
		StatusBarWidget->SetText(LOCTEXT("StatusRetained", "PIE Ended (data retained)"));
		StatusBarWidget->SetColorAndOpacity(FSlateColor(PGX::Semantic::Warn));
	}
}

#undef LOCTEXT_NAMESPACE
