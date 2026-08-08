// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Inspector/SPGXMessageInspectorTab.h"

#include "Messages/PGXMessageSubsystem.h"
#include "Messages/PGXMessageSettings.h"
#include "Messages/PGXMessageConfig.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Editor.h"
#include "EditorStyleSet.h"
#include "Style/PGXEditorStyle.h"
#include "Style/PGXVisualTokens.h"

#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

#define LOCTEXT_NAMESPACE "PGXMessageInspectorTab"

namespace
{
	constexpr double GPGXMessageInspectorRefreshSeconds = 1.0;
	constexpr int32  GPGXMessageInspectorLifecycleCap   = 50;
	constexpr int32  GPGXMessageInspectorHistoryMax     = 25;
}

void SPGXMessageInspectorTab::Construct(const FArguments& /*InArgs*/)
{
	PIEStartedHandle = FEditorDelegates::PostPIEStarted.AddSP(
		SharedThis(this), &SPGXMessageInspectorTab::BindToSubsystem);
	PIEEndedHandle = FEditorDelegates::EndPIE.AddSP(
		SharedThis(this), &SPGXMessageInspectorTab::UnbindFromSubsystem);

	ChildSlot
	[
		SNew(SVerticalBox)

		// --- Overview ---
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f)
		[
			SNew(SBorder)
			.BorderImage(FPGXEditorStyle::Get().GetBrush("PGXEditor.Premium.Card"))
			.Padding(12.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("OverviewHeader", "OVERVIEW"))
					.Font(PGX::Font::SectionHeader())
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text_Lambda([this]() { return GetOverviewSummaryText(); })
				]
			]
		]

		// --- Channels (top-half of remaining space) ---
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(8.0f, 0.0f, 8.0f, 4.0f)
		[
			SNew(SBorder)
			.BorderImage(FPGXEditorStyle::Get().GetBrush("PGXEditor.Premium.Card"))
			.Padding(12.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ChannelsHeader", "CHANNELS (select to filter History)"))
					.Font(PGX::Font::SectionHeader())
				]
				+ SVerticalBox::Slot().FillHeight(1.0f).Padding(0.0f, 6.0f, 0.0f, 0.0f)
				[
					SAssignNew(ChannelListView, SListView<TSharedPtr<FPGXMessageChannelEntry>>)
					.ListItemsSource(&ChannelEntries)
					.OnGenerateRow(this, &SPGXMessageInspectorTab::OnGenerateChannelRow)
					.OnSelectionChanged(this, &SPGXMessageInspectorTab::OnChannelSelectionChanged)
					.SelectionMode(ESelectionMode::Single)
				]
			]
		]

		// --- History (mid panel) ---
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(8.0f, 0.0f, 8.0f, 4.0f)
		[
			SNew(SBorder)
			.BorderImage(FPGXEditorStyle::Get().GetBrush("PGXEditor.Premium.Card"))
			.Padding(12.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text_Lambda([this]() { return GetHistoryHeaderText(); })
					.Font(PGX::Font::SectionHeader())
				]
				+ SVerticalBox::Slot().FillHeight(1.0f).Padding(0.0f, 6.0f, 0.0f, 0.0f)
				[
					SAssignNew(HistoryListView, SListView<TSharedPtr<FPGXMessageRecord>>)
					.ListItemsSource(&HistoryEntries)
					.OnGenerateRow(this, &SPGXMessageInspectorTab::OnGenerateHistoryRow)
					.SelectionMode(ESelectionMode::None)
				]
			]
		]

		// --- Listener Lifecycle (bottom of list-view stack) ---
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(8.0f, 0.0f, 8.0f, 4.0f)
		[
			SNew(SBorder)
			.BorderImage(FPGXEditorStyle::Get().GetBrush("PGXEditor.Premium.Card"))
			.Padding(12.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT(
						"LifecycleHeader",
						"LISTENER LIFECYCLE (delta-inferred — OnListenerRegistered is DYNAMIC-only)"))
					.Font(PGX::Font::SectionHeader())
				]
				+ SVerticalBox::Slot().FillHeight(1.0f).Padding(0.0f, 6.0f, 0.0f, 0.0f)
				[
					SAssignNew(LifecycleListView, SListView<TSharedPtr<FPGXMessageLifecycleEvent>>)
					.ListItemsSource(&LifecycleEvents)
					.OnGenerateRow(this, &SPGXMessageInspectorTab::OnGenerateLifecycleRow)
					.SelectionMode(ESelectionMode::None)
				]
			]
		]

		// --- Test Broadcast (stub) ---
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 0.0f, 8.0f, 4.0f)
		[
			SNew(SBorder)
			.BorderImage(FPGXEditorStyle::Get().GetBrush("PGXEditor.Premium.Card"))
			.Padding(12.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("TestBroadcastHeader", "TEST BROADCAST"))
					.Font(PGX::Font::SectionHeader())
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 0.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 8.0f, 0.0f)
					[
						SNew(SButton)
						.IsEnabled(false)
						.Text(LOCTEXT("TestBroadcastBtn", "Broadcast Test Message"))
						.ToolTipText(LOCTEXT(
							"TestBroadcastBtnTooltip",
							"Test Broadcast requires a gameplay tag picker and payload serialization, which are not available in this preview."))
					]
					+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT(
							"TestBroadcastNote",
							"Stub — broadcast authoring requires an FGameplayTag picker and payload serialization. Console: pgx.message.broadcast (dev/editor only)."))
					]
				]
			]
		]

		// --- Config (read-only snapshot) ---
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 0.0f, 8.0f, 4.0f)
		[
			SNew(SBorder)
			.BorderImage(FPGXEditorStyle::Get().GetBrush("PGXEditor.Premium.Card"))
			.Padding(12.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ConfigHeader", "CONFIG (read-only snapshot)"))
					.Font(PGX::Font::SectionHeader())
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text_Lambda([this]() { return GetConfigSummaryText(); })
				]
			]
		]

		// --- Warnings (derived diagnostics) ---
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 0.0f, 8.0f, 8.0f)
		[
			SNew(SBorder)
			.BorderImage(FPGXEditorStyle::Get().GetBrush("PGXEditor.Premium.Card"))
			.Padding(12.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("WarningsHeader", "WARNINGS"))
					.Font(PGX::Font::SectionHeader())
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text_Lambda([this]() { return GetWarningsSummaryText(); })
				]
			]
		]
	];

	if (GEditor && GEditor->PlayWorld)
	{
		BindToSubsystem();
	}
}

SPGXMessageInspectorTab::~SPGXMessageInspectorTab()
{
	UnbindFromSubsystem();

	if (PIEStartedHandle.IsValid())
	{
		FEditorDelegates::PostPIEStarted.Remove(PIEStartedHandle);
		PIEStartedHandle.Reset();
	}
	if (PIEEndedHandle.IsValid())
	{
		FEditorDelegates::EndPIE.Remove(PIEEndedHandle);
		PIEEndedHandle.Reset();
	}
}

void SPGXMessageInspectorTab::BindToSubsystem(bool /*bIsSimulating*/)
{
	UnbindFromSubsystem();

	UPGXMessageSubsystem* Subsystem = ResolvePIESubsystem();
	if (!Subsystem)
	{
		return;
	}
	WeakSubsystem = Subsystem;

	BroadcastNativeHandle = Subsystem->OnMessageBroadcastNative.AddSP(
		SharedThis(this), &SPGXMessageInspectorTab::HandleBroadcast);

	// EN: Reset delta baselines so a new PIE session does not generate spurious
	//     "Unregistered" rows for channels from the previous run.
	// ES: Reset baselines de delta para que una nueva sesion PIE no genere filas
	//     "Unregistered" espurias por canales de la corrida previa.
	LastSeenListenerCounts.Reset();
	LifecycleEvents.Reset();
	if (LifecycleListView.IsValid())
	{
		LifecycleListView->RequestListRefresh();
	}

	LastRefreshSeconds = 0.0;
	RefreshSnapshot();
}

void SPGXMessageInspectorTab::UnbindFromSubsystem(bool /*bIsSimulating*/)
{
	if (UPGXMessageSubsystem* Subsystem = WeakSubsystem.Get())
	{
		if (BroadcastNativeHandle.IsValid())
		{
			Subsystem->OnMessageBroadcastNative.Remove(BroadcastNativeHandle);
		}
	}
	BroadcastNativeHandle.Reset();
	WeakSubsystem.Reset();
}

UPGXMessageSubsystem* SPGXMessageInspectorTab::ResolvePIESubsystem() const
{
	if (!GEngine)
	{
		return nullptr;
	}

	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::PIE && Context.World())
		{
			if (UGameInstance* GI = Context.World()->GetGameInstance())
			{
				if (UPGXMessageSubsystem* Sub = GI->GetSubsystem<UPGXMessageSubsystem>())
				{
					return Sub;
				}
			}
		}
	}
	return nullptr;
}

void SPGXMessageInspectorTab::HandleBroadcast(
	FGameplayTag Channel,
	const UScriptStruct* /*StructType*/,
	double Timestamp)
{
	// EN: The message bus runs on GameThread. Background workers must marshal
	//     before invoking UObject-bound callbacks,
	//     therefore this handler is reached only from GameThread. Assert the
	//     invariant; do not rely on the writes themselves being safe across
	//     threads — FString / double / int32 are not atomic in the C++ memory
	//     model. Keep the GameThread-only invariant explicit and asserted.
	// ES: El bus de Message corre en GameThread.
	//     Los workers en background deben marshal antes de invocar callbacks
	//     bound a UObject, por tanto este handler se alcanza solo
	//     desde GameThread. Assertear el invariante; no fiarse de que los
	//     writes en si sean seguros entre threads — FString / double / int32
	//     no son atomic en el modelo de memoria C++. Mantener el invariante
	//     GameThread-only explicito y comprobado.
	check(IsInGameThread());

	LastBroadcastChannelName = Channel.IsValid() ? Channel.ToString() : TEXT("(invalid)");
	LastBroadcastTimestamp = Timestamp;
	++ObservedBroadcastCount;
}

void SPGXMessageInspectorTab::RefreshSnapshot()
{
	UPGXMessageSubsystem* Subsystem = WeakSubsystem.Get();
	if (!Subsystem)
	{
		CachedStats = FPGXMessageStats();
		ChannelEntries.Reset();
		HistoryEntries.Reset();
		LifecycleEvents.Reset();
		LastSeenListenerCounts.Reset();
		if (ChannelListView.IsValid())   ChannelListView->RequestListRefresh();
		if (HistoryListView.IsValid())   HistoryListView->RequestListRefresh();
		if (LifecycleListView.IsValid()) LifecycleListView->RequestListRefresh();
		return;
	}

	// EN: Phase 1 — stats + channel list.
	// ES: Phase 1 — stats + lista de canales.
	CachedStats = Subsystem->GetStats();

	const TArray<FGameplayTag> ActiveChannels = Subsystem->GetAllActiveChannels();
	ChannelEntries.Reset(ActiveChannels.Num());

	const double NowSeconds = FPlatformTime::Seconds();

	// EN: Phase 2 — diff per-channel listener counts → lifecycle events.
	// ES: Phase 2 — diff de counts por canal → eventos lifecycle.
	TMap<FGameplayTag, int32> CurrentCounts;
	CurrentCounts.Reserve(ActiveChannels.Num());

	for (const FGameplayTag& ChannelTag : ActiveChannels)
	{
		const int32 NewCount = Subsystem->GetListenerCount(ChannelTag);
		CurrentCounts.Add(ChannelTag, NewCount);

		TSharedPtr<FPGXMessageChannelEntry> Entry = MakeShared<FPGXMessageChannelEntry>();
		Entry->Channel = ChannelTag;
		Entry->ListenerCount = NewCount;
		ChannelEntries.Add(MoveTemp(Entry));

		const int32* PrevPtr = LastSeenListenerCounts.Find(ChannelTag);
		const int32 PrevCount = PrevPtr ? *PrevPtr : 0;
		if (NewCount != PrevCount)
		{
			TSharedPtr<FPGXMessageLifecycleEvent> Ev = MakeShared<FPGXMessageLifecycleEvent>();
			Ev->Channel   = ChannelTag;
			Ev->Delta     = NewCount - PrevCount;
			Ev->NewCount  = NewCount;
			Ev->Type      = (Ev->Delta > 0)
				? EPGXMessageLifecycleEventType::Registered
				: EPGXMessageLifecycleEventType::Unregistered;
			Ev->Timestamp = NowSeconds;
			LifecycleEvents.Insert(MoveTemp(Ev), 0);
		}
	}

	// EN: Detect channels that disappeared (went to 0 listeners and were dropped
	//     from GetAllActiveChannels). Each previously-seen-but-now-absent channel
	//     gets an Unregistered event for its remaining count.
	// ES: Detectar canales que desaparecieron (bajaron a 0 listeners y fueron
	//     droppeados de GetAllActiveChannels). Cada canal previamente visto pero
	//     ahora ausente obtiene un evento Unregistered por su count remanente.
	for (const TPair<FGameplayTag, int32>& Prev : LastSeenListenerCounts)
	{
		if (Prev.Value > 0 && !CurrentCounts.Contains(Prev.Key))
		{
			TSharedPtr<FPGXMessageLifecycleEvent> Ev = MakeShared<FPGXMessageLifecycleEvent>();
			Ev->Channel   = Prev.Key;
			Ev->Delta     = -Prev.Value;
			Ev->NewCount  = 0;
			Ev->Type      = EPGXMessageLifecycleEventType::Unregistered;
			Ev->Timestamp = NowSeconds;
			LifecycleEvents.Insert(MoveTemp(Ev), 0);
		}
	}

	LastSeenListenerCounts = MoveTemp(CurrentCounts);

	// EN: Cap lifecycle log so memory does not unbound during long PIE sessions.
	// ES: Cap del log lifecycle para que la memoria no crezca sin limite durante
	//     sesiones PIE largas.
	if (LifecycleEvents.Num() > GPGXMessageInspectorLifecycleCap)
	{
		LifecycleEvents.SetNum(GPGXMessageInspectorLifecycleCap);
	}

	// EN: Phase 3 — history filtered by SelectedChannel (or empty if none selected).
	// ES: Phase 3 — historia filtrada por SelectedChannel (o vacio si ninguno seleccionado).
	HistoryEntries.Reset();
	if (SelectedChannel.IsValid())
	{
		const TArray<FPGXMessageRecord> Records =
			Subsystem->GetMessageHistory(SelectedChannel, GPGXMessageInspectorHistoryMax);
		HistoryEntries.Reserve(Records.Num());
		for (const FPGXMessageRecord& Rec : Records)
		{
			HistoryEntries.Add(MakeShared<FPGXMessageRecord>(Rec));
		}
	}

	// EN: Refresh all three SListViews. RequestListRefresh is required after
	//     mutating the backing arrays.
	// ES: Refresh de los tres SListViews. RequestListRefresh es necesario tras
	//     mutar los arrays de datos.
	if (ChannelListView.IsValid())   ChannelListView->RequestListRefresh();
	if (HistoryListView.IsValid())   HistoryListView->RequestListRefresh();
	if (LifecycleListView.IsValid()) LifecycleListView->RequestListRefresh();
}

void SPGXMessageInspectorTab::Tick(
	const FGeometry& AllottedGeometry,
	const double InCurrentTime,
	const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (InCurrentTime - LastRefreshSeconds >= GPGXMessageInspectorRefreshSeconds)
	{
		LastRefreshSeconds = InCurrentTime;
		RefreshSnapshot();
	}
}

TSharedRef<ITableRow> SPGXMessageInspectorTab::OnGenerateChannelRow(
	TSharedPtr<FPGXMessageChannelEntry> Entry,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	const FString ChannelText = (Entry.IsValid() && Entry->Channel.IsValid())
		? Entry->Channel.ToString()
		: TEXT("(invalid channel)");
	const int32 Listeners = Entry.IsValid() ? Entry->ListenerCount : 0;

	return SNew(STableRow<TSharedPtr<FPGXMessageChannelEntry>>, OwnerTable)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(4.0f, 2.0f)
		[
			SNew(STextBlock).Text(FText::FromString(ChannelText))
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 2.0f)
		[
			SNew(STextBlock).Text(FText::AsNumber(Listeners))
		]
	];
}

TSharedRef<ITableRow> SPGXMessageInspectorTab::OnGenerateHistoryRow(
	TSharedPtr<FPGXMessageRecord> Entry,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	const FString ChannelText = (Entry.IsValid() && Entry->Channel.IsValid())
		? Entry->Channel.ToString()
		: TEXT("(invalid)");
	const FString PayloadText = Entry.IsValid() ? Entry->PayloadTypeName : FString();
	const double  Ts          = Entry.IsValid() ? Entry->Timestamp : 0.0;
	const int32   Listeners   = Entry.IsValid() ? Entry->ListenersNotified : 0;

	return SNew(STableRow<TSharedPtr<FPGXMessageRecord>>, OwnerTable)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 2.0f)
		[
			SNew(STextBlock).Text(FText::AsNumber(Ts))
		]
		+ SHorizontalBox::Slot().FillWidth(0.4f).Padding(8.0f, 2.0f)
		[
			SNew(STextBlock).Text(FText::FromString(ChannelText))
		]
		+ SHorizontalBox::Slot().FillWidth(0.4f).Padding(8.0f, 2.0f)
		[
			SNew(STextBlock).Text(FText::FromString(PayloadText))
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 2.0f)
		[
			SNew(STextBlock).Text(FText::AsNumber(Listeners))
		]
	];
}

TSharedRef<ITableRow> SPGXMessageInspectorTab::OnGenerateLifecycleRow(
	TSharedPtr<FPGXMessageLifecycleEvent> Entry,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	const FString ChannelText = (Entry.IsValid() && Entry->Channel.IsValid())
		? Entry->Channel.ToString()
		: TEXT("(invalid)");
	const FString TypeText = (Entry.IsValid()
		&& Entry->Type == EPGXMessageLifecycleEventType::Registered)
		? TEXT("REGISTERED")
		: TEXT("UNREGISTERED");
	const int32  Delta = Entry.IsValid() ? Entry->Delta : 0;
	const int32  NewCount = Entry.IsValid() ? Entry->NewCount : 0;
	const double Ts = Entry.IsValid() ? Entry->Timestamp : 0.0;

	return SNew(STableRow<TSharedPtr<FPGXMessageLifecycleEvent>>, OwnerTable)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 2.0f)
		[
			SNew(STextBlock).Text(FText::AsNumber(Ts))
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 2.0f)
		[
			SNew(STextBlock).Text(FText::FromString(TypeText))
		]
		+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(8.0f, 2.0f)
		[
			SNew(STextBlock).Text(FText::FromString(ChannelText))
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 2.0f)
		[
			SNew(STextBlock).Text(FText::Format(
				LOCTEXT("DeltaFmt", "{0} (=> {1})"),
				FText::AsNumber(Delta),
				FText::AsNumber(NewCount)))
		]
	];
}

void SPGXMessageInspectorTab::OnChannelSelectionChanged(
	TSharedPtr<FPGXMessageChannelEntry> NewSelection,
	ESelectInfo::Type /*SelectInfo*/)
{
	if (NewSelection.IsValid() && NewSelection->Channel.IsValid())
	{
		SelectedChannel = NewSelection->Channel;
	}
	else
	{
		SelectedChannel = FGameplayTag();
	}

	// EN: Force an immediate refresh of just the History panel so selection
	//     feels responsive instead of waiting up to 1 s for the next Tick.
	// ES: Forzar refresh inmediato solo del panel History para que la seleccion
	//     se sienta responsive en vez de esperar hasta 1 s al siguiente Tick.
	UPGXMessageSubsystem* Subsystem = WeakSubsystem.Get();
	HistoryEntries.Reset();
	if (Subsystem && SelectedChannel.IsValid())
	{
		const TArray<FPGXMessageRecord> Records =
			Subsystem->GetMessageHistory(SelectedChannel, GPGXMessageInspectorHistoryMax);
		HistoryEntries.Reserve(Records.Num());
		for (const FPGXMessageRecord& Rec : Records)
		{
			HistoryEntries.Add(MakeShared<FPGXMessageRecord>(Rec));
		}
	}
	if (HistoryListView.IsValid())
	{
		HistoryListView->RequestListRefresh();
	}
}

FText SPGXMessageInspectorTab::GetOverviewSummaryText() const
{
	if (!WeakSubsystem.IsValid())
	{
		return GetEmptyStateText();
	}

	return FText::Format(
		LOCTEXT(
			"OverviewSummaryFormat",
			"TotalBroadcasts: {0}\nTotalListenersNotified: {1}\nActiveChannels: {2}\nActiveListeners: {3}\nHistorySize: {4}\nMaxFanOutOnSingleBroadcast: {5}\nObserved broadcasts (since open): {6}\nLast broadcast channel: {7}"),
		FText::AsNumber(CachedStats.TotalBroadcasts),
		FText::AsNumber(CachedStats.TotalListenersNotified),
		FText::AsNumber(CachedStats.ActiveChannels),
		FText::AsNumber(CachedStats.ActiveListeners),
		FText::AsNumber(CachedStats.HistorySize),
		FText::AsNumber(CachedStats.MaxFanOutOnSingleBroadcast),
		FText::AsNumber(ObservedBroadcastCount),
		FText::FromString(LastBroadcastChannelName.IsEmpty()
			? FString(TEXT("(none)"))
			: LastBroadcastChannelName));
}

FText SPGXMessageInspectorTab::GetEmptyStateText() const
{
	return LOCTEXT(
		"EmptyState",
		"PGX Message Subsystem not available — start PIE to attach the inspector.");
}

FText SPGXMessageInspectorTab::GetHistoryHeaderText() const
{
	if (SelectedChannel.IsValid())
	{
		return FText::Format(
			LOCTEXT("HistoryHeaderFormat", "HISTORY — {0}"),
			FText::FromString(SelectedChannel.ToString()));
	}
	return LOCTEXT("HistoryHeaderEmpty", "HISTORY — (select a channel above)");
}

FText SPGXMessageInspectorTab::GetConfigSummaryText() const
{
	const UPGXMessageSettings* Settings = GetDefault<UPGXMessageSettings>();
	if (!Settings)
	{
		return LOCTEXT(
			"ConfigSettingsMissing",
			"UPGXMessageSettings unavailable — Project Settings not registered.");
	}

	const TSoftObjectPtr<UPGXMessageConfig>& ActiveConfigSoft = Settings->ActiveConfig;
	const FString ActiveConfigPath = ActiveConfigSoft.IsNull()
		? FString(TEXT("(none — AssetRegistry fallback)"))
		: ActiveConfigSoft.ToString();

	// EN: LoadSynchronous is acceptable in editor inspector context (low frequency,
	//     1 Hz refresh). The DA is small. If null after load, surface settings-only.
	// ES: LoadSynchronous es aceptable en contexto de inspector editor (baja
	//     frecuencia, refresh 1 Hz). El DA es pequeño. Si null tras load,
	//     mostrar solo settings.
	UPGXMessageConfig* Config = ActiveConfigSoft.IsNull()
		? nullptr
		: const_cast<TSoftObjectPtr<UPGXMessageConfig>&>(ActiveConfigSoft).LoadSynchronous();

	if (!Config)
	{
		return FText::Format(
			LOCTEXT(
				"ConfigSettingsOnlyFormat",
				"Settings:\n  ActiveConfig: {0}\n  EmergencyHistoryFallback: (see Project Settings)\nConfig DA: not loaded."),
			FText::FromString(ActiveConfigPath));
	}

	return FText::Format(
		LOCTEXT(
			"ConfigSummaryFormat",
			"Settings:\n  ActiveConfig: {0}\nConfig DA ({1}):\n  MaxMessageHistory: {2}\n  bLogBroadcasts: {3}\n  bLogRegistrations: {4}\n  bEnablePartialMatching: {5}\n  bAllowTestBroadcasts: {6}"),
		FText::FromString(ActiveConfigPath),
		FText::FromString(Config->GetName()),
		FText::AsNumber(Config->MaxMessageHistory),
		FText::FromString(Config->bLogBroadcasts ? TEXT("true") : TEXT("false")),
		FText::FromString(Config->bLogRegistrations ? TEXT("true") : TEXT("false")),
		FText::FromString(Config->bEnablePartialMatching ? TEXT("true") : TEXT("false")),
		FText::FromString(Config->bAllowTestBroadcasts ? TEXT("true") : TEXT("false")));
}

FText SPGXMessageInspectorTab::GetWarningsSummaryText() const
{
	if (!WeakSubsystem.IsValid())
	{
		return LOCTEXT(
			"WarningsEmpty",
			"(no diagnostics — subsystem not attached)");
	}

	// EN: Lightweight thresholds — adjustable as production data accumulates.
	//     the configuration-source invariant forbids hardcoded values in production code; these
	//     are inspector-only heuristics, not gameplay/runtime defaults, so the
	//     literal threshold lives here pending a future Inspector settings DA
	//     if these need project-level tuning.
	// ES: Thresholds livianos — ajustables conforme se acumule data productiva.
	//     El invariante de fuente de configuracion prohibe valores hardcoded en codigo productivo;
	//     estos son heuristicos solo-inspector, no defaults de runtime/gameplay,
	//     asi que el literal threshold vive aqui pendiente de un futuro
	//     Inspector settings DA si necesitan tuning a nivel proyecto.
	constexpr int32 HighFanOutThreshold = 32;

	TArray<FText> Lines;

	if (CachedStats.MaxFanOutOnSingleBroadcast > HighFanOutThreshold)
	{
		Lines.Add(FText::Format(
			LOCTEXT(
				"WarnHighFanOut",
				"  - HIGH FAN-OUT: a single broadcast notified {0} listeners (threshold {1}). Consider channel split or partial-match audit."),
			FText::AsNumber(CachedStats.MaxFanOutOnSingleBroadcast),
			FText::AsNumber(HighFanOutThreshold)));
	}

	if (CachedStats.MaxBroadcastDepth > 1)
	{
		Lines.Add(FText::Format(
			LOCTEXT(
				"WarnNestingDepth",
				"  - BROADCAST NESTING DEPTH > 1 ({0}): cascade chain detected via non-orchestrator path. Inspect call graph."),
			FText::AsNumber(CachedStats.MaxBroadcastDepth)));
	}

	if (CachedStats.MaxQueueDepth > 0)
	{
		Lines.Add(FText::Format(
			LOCTEXT(
				"WarnQueueDepth",
				"  - DEFERRED QUEUE DEPTH > 0 ({0}): broadcasts queued during parent dispatch — same-frame deferred-dispatch ordering active."),
			FText::AsNumber(CachedStats.MaxQueueDepth)));
	}

	// EN: Broad partial-match listener heuristic deferred — would require
	//     subsystem to expose per-channel listener match-mode counts. Today
	//     not in the public API; hook would mean PGXCoreRuntime mutation
	//     and is intentionally outside this inspector's scope. Documented gap.
	// ES: Heuristico de listener broad partial-match diferido — requeriria
	//     que el subsistema expusiera counts por canal de match-mode. Hoy
	//     no esta en el API publico; el hook implicaria mutar PGXCoreRuntime
	//     y queda intencionadamente fuera del alcance de este inspector. Gap documentado.

	if (Lines.Num() == 0)
	{
		return LOCTEXT(
			"WarningsClean",
			"All clean. No high fan-out, no elevated nesting/queue depth.");
	}

	FText Joined = FText::FromString(FString());
	for (int32 Idx = 0; Idx < Lines.Num(); ++Idx)
	{
		if (Idx > 0)
		{
			Joined = FText::Format(
				LOCTEXT("WarnLineJoin", "{0}\n{1}"),
				Joined,
				Lines[Idx]);
		}
		else
		{
			Joined = Lines[Idx];
		}
	}
	return Joined;
}

#undef LOCTEXT_NAMESPACE
