// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Inspector/PGXAudioInspector.h"
#include "PGXAudioSubsystem.h"
#include "Logging/PGXLogMacros.h"
#include "Mix/PGXAudioMixSubsystem.h"
#include "Manager/PGXMusicManager.h"
#include "Manager/PGXDialogueManager.h"
#include "Manager/PGXSoundPool.h"
#include "PGXAudioLog.h"
#include "PGXAudioTypes.h"
#include "Utils/PGXEditorUtils.h"
#include "Style/PGXVisualTokens.h"
#include "Widgets/SPGXPremiumShell.h"
#include "Widgets/PGXButtonHelpers.h"
#include "Widgets/SPGXSectionDivider.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Workspace/PGXWorkspaceMenu.h"
#include "Style/PGXEditorStyle.h"
#include "Editor.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Utils/PGXTabRegistration.h"

#define LOCTEXT_NAMESPACE "PGXAudioInspector"

// EN: Audio system color — Orange per PGX Visual Tokens / ES: Color del sistema Audio — Naranja via PGX Visual Tokens
static const FLinearColor GAudioColor = PGX::System::Audio;

const FName SPGXAudioInspector::TabId(TEXT("PGXAudioInspector"));

// ── Tab Spawner ──────────────────────────────────────────────────────────

void SPGXAudioInspector::RegisterTabSpawner()
{
	PGX::Editor::RegisterNomadTab(TabId, FOnSpawnTab::CreateStatic(&SPGXAudioInspector::SpawnTab))
		.SetDisplayName(FText::FromString(TEXT("Audio")))
		.SetTooltipText(FText::FromString(TEXT("PGX Audio system inspector and debugger")))
		.SetGroup(FPGXWorkspaceMenu::GetInspectorsGroup())
		.SetIcon(FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.Audio"));
}

void SPGXAudioInspector::UnregisterTabSpawner()
{
	PGX::Editor::UnregisterNomadTab(TabId);
}

TSharedRef<SDockTab> SPGXAudioInspector::SpawnTab(const FSpawnTabArgs& /*Args*/)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SPGXAudioInspector)
		];
}

// ── Construct ────────────────────────────────────────────────────────────

void SPGXAudioInspector::Construct(const FArguments& /*InArgs*/)
{
	PGX_LOG_INFO(LogPGXAudio, TEXT("[AudioInspector] Construct"));

	ChildSlot
	[
		// EN: Premium Shell — covers UE chrome, provides title bar + footer
		// ES: Shell Premium — cubre chrome de UE, provee title bar + footer
		SNew(SPGXPremiumShell)
		.SystemColor(PGX::System::Audio)
		.Title(LOCTEXT("PanelTitle", "Audio Inspector"))
		.Subtitle(LOCTEXT("PanelSubtitle", "Dual-backend audio debugger"))
		.Icon(FPGXEditorStyle::Get().GetBrush("PGXEditor.Icon.Audio"))
		.bShowFooter(true)
		.StatusText_Lambda([this]() -> FText
		{
			if (StatusFooterText.IsValid())
			{
				return StatusFooterText->GetText();
			}
			return LOCTEXT("FooterWaiting", "Waiting for PIE...");
		})
		.Content()
		[
			SNew(SVerticalBox)

			// EN: Toolbar: KPIs + Actions / ES: Toolbar: KPIs + Acciones
			+ SVerticalBox::Slot().AutoHeight().Padding(8, 4)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0).VAlign(VAlign_Center)
				[
					SAssignNew(ActiveSoundsKPI, STextBlock)
					.Text(FText::FromString(TEXT("Sounds: 0")))
					.Font(PGX::Font::Badge())
					.ColorAndOpacity(FSlateColor(GAudioColor))
				]

				+ SHorizontalBox::Slot().AutoWidth().Padding(8, 0).VAlign(VAlign_Center)
				[
					SAssignNew(PoolUsageKPI, STextBlock)
					.Text(FText::FromString(TEXT("Pool: 0/0")))
					.Font(PGX::Font::BodySmall())
				]

				+ SHorizontalBox::Slot().AutoWidth().Padding(8, 0).VAlign(VAlign_Center)
				[
					SAssignNew(MemoryKPI, STextBlock)
					.Text(FText::FromString(TEXT("Mem: 0 MB")))
					.Font(PGX::Font::BodySmall())
				]

				+ SHorizontalBox::Slot().FillWidth(1.0f) [ SNullWidget::NullWidget ]

				// EN: Stop All button / ES: Boton Stop All
				+ SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Stop All")))
					.OnClicked(this, &SPGXAudioInspector::OnStopAllClicked)
					.ToolTipText(FText::FromString(TEXT("Stop all currently playing sounds")))
				]

				// EN: Mute All toggle / ES: Toggle Mute All
				+ SHorizontalBox::Slot().AutoWidth().Padding(8, 0).VAlign(VAlign_Center)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SNew(SCheckBox)
						.OnCheckStateChanged(this, &SPGXAudioInspector::OnMuteAllToggled)
						.ToolTipText(FText::FromString(TEXT("Toggle global audio mute")))
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(2, 0).VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Mute All")))
						.Font(PGX::Font::BodySmall())
					]
				]
			]

			// EN: Scrollable body with 12 sections / ES: Cuerpo scrollable con 12 secciones
			+ SVerticalBox::Slot().FillHeight(1.0f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot().Padding(8)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight() [ BuildSystemStatusSection() ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 2) [ BuildBackendSection() ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 2) [ BuildChannelMixerSection() ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 2) [ BuildActiveSoundsSection() ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 2) [ BuildMusicSection() ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 2) [ BuildDialogueSection() ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 2) [ BuildMixLayersSection() ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 2) [ BuildDuckingSection() ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 2) [ BuildPoolSection() ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 2) [ BuildMemorySection() ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 2) [ BuildEventHistorySection() ]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 2) [ BuildProfileSection() ]
				]
			]
		]
	];

	// EN: Hidden text block for footer status (read by StatusText lambda)
	// ES: Text block oculto para estado del footer (leido por lambda StatusText)
	SAssignNew(StatusFooterText, STextBlock)
		.Text(FText::FromString(TEXT("Waiting for PIE...")));
}

SPGXAudioInspector::~SPGXAudioInspector()
{
	PGX_LOG_INFO(LogPGXAudio, TEXT("[AudioInspector] Destructor — cleanup"));
}

// ── Tick ─────────────────────────────────────────────────────────────────

void SPGXAudioInspector::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	RefreshAccumulator += InDeltaTime;
	if (RefreshAccumulator < RefreshInterval)
	{
		return;
	}
	RefreshAccumulator = 0.0f;

	UPGXAudioSubsystem* AudioSub = GetAudioSubsystem();
	const UPGXAudioMixSubsystem* MixSub = GetMixSubsystem();

	// EN: Update footer PIE state / ES: Actualizar estado PIE del footer
	if (StatusFooterText.IsValid())
	{
		if (AudioSub)
		{
			StatusFooterText->SetText(FText::FromString(TEXT("PIE Active — Audio system connected")));
			StatusFooterText->SetColorAndOpacity(FSlateColor(PGX::Semantic::Good));
		}
		else
		{
			StatusFooterText->SetText(FText::FromString(TEXT("Waiting for PIE...")));
			StatusFooterText->SetColorAndOpacity(FSlateColor(PGX::Semantic::Neutral));
		}
	}

	// EN: Update system status / ES: Actualizar estado del sistema
	if (SystemStatusText.IsValid())
	{
		if (AudioSub)
		{
			SystemStatusText->SetText(FText::FromString(FString::Printf(
				TEXT("State: %s | Active Sounds: %d | Global Mute: %s"),
				*UEnum::GetValueAsString(AudioSub->GetAudioState()),
				AudioSub->GetActiveSoundCount(),
				AudioSub->IsMuteAll() ? TEXT("ON") : TEXT("OFF")
			)));
		}
		else
		{
			SystemStatusText->SetText(FText::FromString(TEXT("AudioSubsystem not available (no PIE world?)")));
		}
	}

	// EN: Update KPI chips / ES: Actualizar KPI chips
	if (AudioSub)
	{
		if (ActiveSoundsKPI.IsValid())
		{
			ActiveSoundsKPI->SetText(FText::FromString(FString::Printf(
				TEXT("Sounds: %d"), AudioSub->GetActiveSoundCount())));
		}
		if (PoolUsageKPI.IsValid())
		{
			const FPGXSoundPoolStats PStats = AudioSub->GetPoolStatistics();
			PoolUsageKPI->SetText(FText::FromString(FString::Printf(
				TEXT("Pool: %d/%d"), PStats.InUse, PStats.TotalCapacity)));
		}
		if (MemoryKPI.IsValid())
		{
			const FPGXAudioMemoryInfo Mem = AudioSub->GetMemoryEstimate();
			MemoryKPI->SetText(FText::FromString(FString::Printf(
				TEXT("Mem: %.1f MB"), static_cast<double>(Mem.EstimatedMemoryBytes) / (1024.0 * 1024.0))));
		}
	}

	// EN: Update backend info / ES: Actualizar info del backend
	if (BackendText.IsValid() && AudioSub)
	{
		BackendText->SetText(FText::FromString(FString::Printf(
			TEXT("Type: %s | Status: %s"),
			*UEnum::GetValueAsString(AudioSub->GetActiveBackendType()),
			*AudioSub->GetBackendStatusText()
		)));
	}

	// ── Channel Mixer → SListView refresh ────────────────────────────────
	ChannelData.Reset();
	if (AudioSub)
	{
		const TArray<FPGXAudioChannelSnapshot> Channels = AudioSub->GetAllChannelStates();
		for (const FPGXAudioChannelSnapshot& Ch : Channels)
		{
			ChannelData.Add(MakeShared<FPGXAudioChannelSnapshot>(Ch));
		}
	}
	if (ChannelListView.IsValid())
	{
		ChannelListView->RequestListRefresh();
	}

	// ── Active Sounds → SListView refresh ────────────────────────────────
	SoundData.Reset();
	if (AudioSub)
	{
		const TArray<FPGXActiveSoundInfo> Sounds = AudioSub->GetActiveSounds();
		for (const FPGXActiveSoundInfo& Snd : Sounds)
		{
			SoundData.Add(MakeShared<FPGXActiveSoundInfo>(Snd));
		}
	}
	if (SoundListView.IsValid())
	{
		SoundListView->RequestListRefresh();
	}

	// ── Event History → SListView refresh (25 entries, newest first) ─────
	EventData.Reset();
	if (AudioSub)
	{
		const TArray<FPGXAudioEventRecord> Events = AudioSub->GetEventHistory(25);
		for (int32 i = Events.Num() - 1; i >= 0; --i)
		{
			EventData.Add(MakeShared<FPGXAudioEventRecord>(Events[i]));
		}
	}
	if (EventListView.IsValid())
	{
		EventListView->RequestListRefresh();
	}

	// EN: Update music / ES: Actualizar musica
	if (MusicText.IsValid() && AudioSub)
	{
		MusicText->SetText(FText::FromString(FString::Printf(
			TEXT("State: %s | Track: %s | MusicState: %s"),
			*UEnum::GetValueAsString(AudioSub->GetMusicPlaybackState()),
			*AudioSub->GetCurrentTrackName(),
			*AudioSub->GetMusicState().ToString()
		)));
	}

	// EN: Update dialogue / ES: Actualizar dialogo
	if (DialogueText.IsValid() && AudioSub)
	{
		DialogueText->SetText(FText::FromString(FString::Printf(
			TEXT("Playing: %s | Queue: %d"),
			AudioSub->IsDialoguePlaying() ? TEXT("YES") : TEXT("NO"),
			AudioSub->GetDialogueQueueCount()
		)));
	}

	// EN: Update mix layers / ES: Actualizar capas de mezcla
	if (MixLayersText.IsValid() && MixSub)
	{
		FString MixInfo;
		const TArray<FPGXMixLayerState> Layers = MixSub->GetAllMixLayerStates();
		for (const FPGXMixLayerState& L : Layers)
		{
			MixInfo += FString::Printf(TEXT("  %s: %s (%d channels) %s\n"),
				*UEnum::GetValueAsString(L.Layer),
				L.bActive ? TEXT("ACTIVE") : TEXT("inactive"),
				L.ChannelValues.Num(),
				L.SourceConfigName.IsEmpty() ? TEXT("") : *FString::Printf(TEXT("[%s]"), *L.SourceConfigName));
		}
		if (MixInfo.IsEmpty()) MixInfo = TEXT("  No mix data (no world?)");
		MixLayersText->SetText(FText::FromString(MixInfo));
	}

	// EN: Update ducking / ES: Actualizar ducking
	if (DuckingText.IsValid() && MixSub)
	{
		FString DuckInfo;
		const TArray<FPGXDuckingRule> Rules = MixSub->GetActiveDuckingRules();
		for (const FPGXDuckingRule& R : Rules)
		{
			DuckInfo += FString::Printf(TEXT("  %s: %s -> %s (%.0f%% | P%d)\n"),
				*R.RuleName.ToString(), *R.TriggerChannelTag.ToString(),
				*R.TargetChannelTag.ToString(), R.DuckVolume * 100.0f, R.Priority);
		}
		if (DuckInfo.IsEmpty()) DuckInfo = TEXT("  No active ducking rules");
		DuckingText->SetText(FText::FromString(DuckInfo));
	}

	// EN: Update pool stats / ES: Actualizar stats del pool
	if (PoolText.IsValid() && AudioSub)
	{
		const FPGXSoundPoolStats Stats = AudioSub->GetPoolStatistics();
		PoolText->SetText(FText::FromString(FString::Printf(
			TEXT("Capacity: %d | InUse: %d | Available: %d | Peak: %d | Misses: %d | Growths: %d"),
			Stats.TotalCapacity, Stats.InUse, Stats.Available, Stats.PeakUsage, Stats.MissCount, Stats.GrowthEvents
		)));
	}

	// EN: Update memory / ES: Actualizar memoria
	if (MemoryText.IsValid() && AudioSub)
	{
		const FPGXAudioMemoryInfo Mem = AudioSub->GetMemoryEstimate();
		MemoryText->SetText(FText::FromString(FString::Printf(
			TEXT("Loaded: %d | Memory: %.1f MB | Pool: %d/%d | Peak Concurrent: %d"),
			Mem.LoadedSoundCount, static_cast<double>(Mem.EstimatedMemoryBytes) / (1024.0 * 1024.0),
			Mem.PoolInUse, Mem.PoolAllocated, Mem.PeakConcurrent
		)));
	}

	// EN: Update profile / ES: Actualizar perfil
	if (ProfileText.IsValid())
	{
		ProfileText->SetText(FText::FromString(TEXT("  Profile constraints: (bind to Profile system for live data)")));
	}
}

// ── Section Builders ─────────────────────────────────────────────────────

TSharedRef<SWidget> SPGXAudioInspector::BuildSystemStatusSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[ SNew(SPGXSectionDivider).Title(LOCTEXT("SectStatus", "SYSTEM STATUS")).AccentColor(PGX::System::Audio) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(4, 2)
		[
			SAssignNew(SystemStatusText, STextBlock)
			.Text(FText::FromString(TEXT("Waiting for PIE...")))
		];
}

TSharedRef<SWidget> SPGXAudioInspector::BuildBackendSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[ SNew(SPGXSectionDivider).Title(LOCTEXT("SectBackend", "BACKEND DETAILS")).AccentColor(PGX::System::Audio) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(4, 2)
		[
			SAssignNew(BackendText, STextBlock)
			.Text(FText::FromString(TEXT("...")))
		];
}

TSharedRef<SWidget> SPGXAudioInspector::BuildChannelMixerSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[ SNew(SPGXSectionDivider).Title(LOCTEXT("SectChannels", "CHANNEL MIXER")).AccentColor(PGX::System::Audio) ]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SBox)
			.MaxDesiredHeight(200.0f)
			[
				SAssignNew(ChannelListView, SListView<TSharedPtr<FPGXAudioChannelSnapshot>>)
				.ListItemsSource(&ChannelData)
				.OnGenerateRow(this, &SPGXAudioInspector::OnGenerateChannelRow)
				.HeaderRow(
					SNew(SHeaderRow)
					+ SHeaderRow::Column("Name").DefaultLabel(FText::FromString(TEXT("Channel"))).FillWidth(0.3f)
					+ SHeaderRow::Column("Volume").DefaultLabel(FText::FromString(TEXT("Volume"))).FillWidth(0.15f)
					+ SHeaderRow::Column("Muted").DefaultLabel(FText::FromString(TEXT("Muted"))).FillWidth(0.15f)
					+ SHeaderRow::Column("Sounds").DefaultLabel(FText::FromString(TEXT("Active"))).FillWidth(0.15f)
					+ SHeaderRow::Column("CtxTag").DefaultLabel(FText::FromString(TEXT("Tag"))).FillWidth(0.25f)
				)
			]
		];
}

TSharedRef<SWidget> SPGXAudioInspector::BuildActiveSoundsSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[ SNew(SPGXSectionDivider).Title(LOCTEXT("SectSounds", "ACTIVE SOUNDS")).AccentColor(PGX::System::Audio) ]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SBox)
			.MaxDesiredHeight(250.0f)
			[
				SAssignNew(SoundListView, SListView<TSharedPtr<FPGXActiveSoundInfo>>)
				.ListItemsSource(&SoundData)
				.OnGenerateRow(this, &SPGXAudioInspector::OnGenerateActiveSoundRow)
				.HeaderRow(
					SNew(SHeaderRow)
					+ SHeaderRow::Column("Handle").DefaultLabel(FText::FromString(TEXT("ID"))).FillWidth(0.1f)
					+ SHeaderRow::Column("Name").DefaultLabel(FText::FromString(TEXT("Sound"))).FillWidth(0.25f)
					+ SHeaderRow::Column("Channel").DefaultLabel(FText::FromString(TEXT("Channel"))).FillWidth(0.2f)
					+ SHeaderRow::Column("Age").DefaultLabel(FText::FromString(TEXT("Age"))).FillWidth(0.15f)
					+ SHeaderRow::Column("Priority").DefaultLabel(FText::FromString(TEXT("Pri"))).FillWidth(0.15f)
					+ SHeaderRow::Column("Spatial").DefaultLabel(FText::FromString(TEXT("3D"))).FillWidth(0.15f)
				)
			]
		];
}

TSharedRef<SWidget> SPGXAudioInspector::BuildMusicSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[ SNew(SPGXSectionDivider).Title(LOCTEXT("SectMusic", "MUSIC MANAGER")).AccentColor(PGX::System::Audio) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(4, 2)
		[
			SAssignNew(MusicText, STextBlock)
			.Text(FText::FromString(TEXT("...")))
		];
}

TSharedRef<SWidget> SPGXAudioInspector::BuildDialogueSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[ SNew(SPGXSectionDivider).Title(LOCTEXT("SectDialogue", "DIALOGUE MANAGER")).AccentColor(PGX::System::Audio) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(4, 2)
		[
			SAssignNew(DialogueText, STextBlock)
			.Text(FText::FromString(TEXT("...")))
		];
}

TSharedRef<SWidget> SPGXAudioInspector::BuildMixLayersSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[ SNew(SPGXSectionDivider).Title(LOCTEXT("SectMixLayers", "MIX LAYERS (5-LAYER MODEL)")).AccentColor(PGX::System::Audio) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(4, 2)
		[
			SAssignNew(MixLayersText, STextBlock)
			.Text(FText::FromString(TEXT("...")))
		];
}

TSharedRef<SWidget> SPGXAudioInspector::BuildDuckingSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[ SNew(SPGXSectionDivider).Title(LOCTEXT("SectDucking", "DUCKING RULES")).AccentColor(PGX::System::Audio) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(4, 2)
		[
			SAssignNew(DuckingText, STextBlock)
			.Text(FText::FromString(TEXT("...")))
		];
}

TSharedRef<SWidget> SPGXAudioInspector::BuildPoolSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[ SNew(SPGXSectionDivider).Title(LOCTEXT("SectPool", "SOUND POOL")).AccentColor(PGX::System::Audio) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(4, 2)
		[
			SAssignNew(PoolText, STextBlock)
			.Text(FText::FromString(TEXT("...")))
		];
}

TSharedRef<SWidget> SPGXAudioInspector::BuildMemorySection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[ SNew(SPGXSectionDivider).Title(LOCTEXT("SectMemory", "MEMORY")).AccentColor(PGX::System::Audio) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(4, 2)
		[
			SAssignNew(MemoryText, STextBlock)
			.Text(FText::FromString(TEXT("...")))
		];
}

TSharedRef<SWidget> SPGXAudioInspector::BuildEventHistorySection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[ SNew(SPGXSectionDivider).Title(LOCTEXT("SectEvents", "EVENT HISTORY (LAST 25)")).AccentColor(PGX::System::Audio) ]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SBox)
			.MaxDesiredHeight(300.0f)
			[
				SAssignNew(EventListView, SListView<TSharedPtr<FPGXAudioEventRecord>>)
				.ListItemsSource(&EventData)
				.OnGenerateRow(this, &SPGXAudioInspector::OnGenerateEventRow)
				.HeaderRow(
					SNew(SHeaderRow)
					+ SHeaderRow::Column("Time").DefaultLabel(FText::FromString(TEXT("Time"))).FillWidth(0.12f)
					+ SHeaderRow::Column("Event").DefaultLabel(FText::FromString(TEXT("Event"))).FillWidth(0.25f)
					+ SHeaderRow::Column("Sound").DefaultLabel(FText::FromString(TEXT("Sound"))).FillWidth(0.2f)
					+ SHeaderRow::Column("Channel").DefaultLabel(FText::FromString(TEXT("Channel"))).FillWidth(0.2f)
					+ SHeaderRow::Column("Duration").DefaultLabel(FText::FromString(TEXT("Dur"))).FillWidth(0.1f)
					+ SHeaderRow::Column("Status").DefaultLabel(FText::FromString(TEXT("Status"))).FillWidth(0.13f)
				)
			]
		];
}

TSharedRef<SWidget> SPGXAudioInspector::BuildProfileSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[ SNew(SPGXSectionDivider).Title(LOCTEXT("SectProfile", "PROFILE CONSTRAINTS")).AccentColor(PGX::System::Audio) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(4, 2)
		[
			SAssignNew(ProfileText, STextBlock)
			.Text(FText::FromString(TEXT("  (requires Profile system binding)")))
		];
}

// ── Row Generators ───────────────────────────────────────────────────────

TSharedRef<ITableRow> SPGXAudioInspector::OnGenerateChannelRow(
	TSharedPtr<FPGXAudioChannelSnapshot> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FPGXAudioChannelSnapshot>>, OwnerTable)
	[
		SNew(SHorizontalBox)

		// EN: Channel name / ES: Nombre del canal
		+ SHorizontalBox::Slot().FillWidth(0.3f).Padding(4, 2).VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Item->DisplayName))
			.Font(PGX::Font::Badge())
		]

		// EN: Volume percentage / ES: Porcentaje de volumen
		+ SHorizontalBox::Slot().FillWidth(0.15f).Padding(4, 2).VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("%.0f%%"), Item->Volume * 100.0f)))
			.Font(PGX::Font::Mono())
		]

		// EN: Mute state / ES: Estado de mute
		+ SHorizontalBox::Slot().FillWidth(0.15f).Padding(4, 2).VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Item->bMuted ? TEXT("MUTED") : TEXT("-")))
			.ColorAndOpacity(Item->bMuted ? FSlateColor(PGX::Semantic::Error) : FSlateColor(FLinearColor::Gray))
		]

		// EN: Active sound count / ES: Cantidad de sonidos activos
		+ SHorizontalBox::Slot().FillWidth(0.15f).Padding(4, 2).VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("%d"), Item->ActiveSoundCount)))
		]

		// EN: Channel tag (leaf name) / ES: Tag del canal (nombre hoja)
		+ SHorizontalBox::Slot().FillWidth(0.25f).Padding(4, 2).VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(PGXEditorUtils::TagToLeafName(Item->ChannelTag)))
			.ToolTipText(FText::FromString(Item->ChannelTag.ToString()))
			.Font(PGX::Font::Caption())
			.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
		]
	];
}

TSharedRef<ITableRow> SPGXAudioInspector::OnGenerateActiveSoundRow(
	TSharedPtr<FPGXActiveSoundInfo> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FPGXActiveSoundInfo>>, OwnerTable)
	[
		SNew(SHorizontalBox)

		// EN: Handle ID / ES: ID del handle
		+ SHorizontalBox::Slot().FillWidth(0.1f).Padding(4, 2).VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("#%d"), Item->Handle.Id)))
			.Font(PGX::Font::Mono())
			.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
		]

		// EN: Sound name / ES: Nombre del sonido
		+ SHorizontalBox::Slot().FillWidth(0.25f).Padding(4, 2).VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Item->SoundName))
		]

		// EN: Channel (leaf name) / ES: Canal (nombre hoja)
		+ SHorizontalBox::Slot().FillWidth(0.2f).Padding(4, 2).VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(PGXEditorUtils::TagToLeafName(Item->ChannelTag)))
			.ToolTipText(FText::FromString(Item->ChannelTag.ToString()))
		]

		// EN: Age in seconds / ES: Edad en segundos
		+ SHorizontalBox::Slot().FillWidth(0.15f).Padding(4, 2).VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("%.1fs"), Item->Age)))
			.Font(PGX::Font::Mono())
		]

		// EN: Priority / ES: Prioridad
		+ SHorizontalBox::Slot().FillWidth(0.15f).Padding(4, 2).VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("P%d"), Item->Priority)))
		]

		// EN: 3D/2D indicator / ES: Indicador 3D/2D
		+ SHorizontalBox::Slot().FillWidth(0.15f).Padding(4, 2).VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Item->bIs3D ? TEXT("3D") : TEXT("2D")))
			.ColorAndOpacity(Item->bIs3D ? FSlateColor(GAudioColor) : FSlateColor(FLinearColor::Gray))
			.Font(PGX::Font::Badge())
		]
	];
}

TSharedRef<ITableRow> SPGXAudioInspector::OnGenerateEventRow(
	TSharedPtr<FPGXAudioEventRecord> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FPGXAudioEventRecord>>, OwnerTable)
	[
		SNew(SHorizontalBox)

		// EN: Timestamp / ES: Marca de tiempo
		+ SHorizontalBox::Slot().FillWidth(0.12f).Padding(4, 2).VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("%.1fs"), Item->Timestamp)))
			.Font(PGX::Font::Mono())
			.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
		]

		// EN: Event tag (leaf) / ES: Tag del evento (hoja)
		+ SHorizontalBox::Slot().FillWidth(0.25f).Padding(4, 2).VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(PGXEditorUtils::TagToLeafName(Item->EventTag)))
			.ToolTipText(FText::FromString(Item->EventTag.ToString()))
		]

		// EN: Sound name / ES: Nombre del sonido
		+ SHorizontalBox::Slot().FillWidth(0.2f).Padding(4, 2).VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Item->SoundName.IsEmpty() ? TEXT("-") : Item->SoundName))
		]

		// EN: Channel (leaf) / ES: Canal (hoja)
		+ SHorizontalBox::Slot().FillWidth(0.2f).Padding(4, 2).VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(PGXEditorUtils::TagToLeafName(Item->ChannelTag)))
			.ToolTipText(FText::FromString(Item->ChannelTag.ToString()))
		]

		// EN: Duration / ES: Duracion
		+ SHorizontalBox::Slot().FillWidth(0.1f).Padding(4, 2).VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("%.1fs"), Item->Duration)))
			.Font(PGX::Font::Mono())
		]

		// EN: Success/Fail status / ES: Estado exito/fallo
		+ SHorizontalBox::Slot().FillWidth(0.13f).Padding(4, 2).VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Item->bSuccess ? TEXT("OK") : TEXT("FAIL")))
			.ColorAndOpacity(Item->bSuccess ? FSlateColor(PGX::Semantic::Good) : FSlateColor(PGX::Semantic::Error))
			.Font(PGX::Font::Badge())
		]
	];
}

// ── Actions ──────────────────────────────────────────────────────────────

FReply SPGXAudioInspector::OnStopAllClicked()
{
	PGX_LOG_INFO(LogPGXAudio, TEXT("[AudioInspector] Stop All clicked"));

	if (UPGXAudioSubsystem* AudioSub = GetAudioSubsystem())
	{
		AudioSub->StopAllSounds(0.5f);
		PGX_LOG_INFO(LogPGXAudio, TEXT("[AudioInspector] StopAllSounds(0.5f) executed"));
	}
	else
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("[AudioInspector] Stop All — no audio subsystem available"));
	}

	return FReply::Handled();
}

void SPGXAudioInspector::OnMuteAllToggled(ECheckBoxState NewState)
{
	const bool bMuted = (NewState == ECheckBoxState::Checked);
	PGX_LOG_INFO(LogPGXAudio, TEXT("[AudioInspector] Mute All toggled: %s"), bMuted ? TEXT("ON") : TEXT("OFF"));

	if (UPGXAudioSubsystem* AudioSub = GetAudioSubsystem())
	{
		AudioSub->SetMuteAll(bMuted);
		PGX_LOG_INFO(LogPGXAudio, TEXT("[AudioInspector] SetMuteAll(%s) executed"), bMuted ? TEXT("true") : TEXT("false"));
	}
	else
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("[AudioInspector] Mute All — no audio subsystem available"));
	}
}

// ── Helpers ──────────────────────────────────────────────────────────────

UPGXAudioSubsystem* SPGXAudioInspector::GetAudioSubsystem() const
{
	if (!GEditor)
	{
		return nullptr;
	}

	// EN: Try PIE world first / ES: Intentar mundo PIE primero
	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	for (const FWorldContext& Context : WorldContexts)
	{
		if (Context.WorldType == EWorldType::PIE && Context.World())
		{
			if (const UGameInstance* GI = Context.World()->GetGameInstance())
			{
				if (UPGXAudioSubsystem* Sub = GI->GetSubsystem<UPGXAudioSubsystem>())
				{
					return Sub;
				}
			}
		}
	}

	return nullptr;
}

UPGXAudioMixSubsystem* SPGXAudioInspector::GetMixSubsystem() const
{
	if (!GEditor)
	{
		return nullptr;
	}

	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	for (const FWorldContext& Context : WorldContexts)
	{
		if (Context.WorldType == EWorldType::PIE && Context.World())
		{
			if (UPGXAudioMixSubsystem* Mix = Context.World()->GetSubsystem<UPGXAudioMixSubsystem>())
			{
				return Mix;
			}
		}
	}

	return nullptr;
}

#undef LOCTEXT_NAMESPACE
