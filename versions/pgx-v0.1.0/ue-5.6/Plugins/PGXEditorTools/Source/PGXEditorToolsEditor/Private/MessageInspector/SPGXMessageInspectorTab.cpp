// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "MessageInspector/SPGXMessageInspectorTab.h"
#include "Messages/PGXMessageSubsystem.h"
#include "Messages/PGXMessage.h"
#include "Messages/PGXMessageDelegates.h"
#include "Messages/PGXMessageLog.h"
#include "Logging/PGXLogMacros.h"
#include "Utils/PGXEditorUtils.h"
#include "Widgets/SPGXEmptyStateV2.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/STableRow.h"
#include "Styling/CoreStyle.h"
#include "Styling/AppStyle.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Editor.h"
#include "Style/PGXVisualTokens.h"
#include "Style/PGXEditorStyle.h"
#include "Widgets/SPGXPremiumShell.h"
#include "Widgets/SPGXSectionDivider.h"
#include "Widgets/SPGXKPIChip.h"
#include "Widgets/PGXButtonHelpers.h"

#define LOCTEXT_NAMESPACE "PGXMessageInspector"

// EN: System color: Purple (156, 39, 176) / ES: Color del sistema: Morado
namespace { static const FLinearColor GMessageInspectorColor = PGX::System::Message; }

// ========================================================================
// Row Widgets for SListView tables
// ========================================================================

class SPGXChannelTableRow : public SMultiColumnTableRow<TSharedPtr<FPGXChannelRowData>>
{
public:
	SLATE_BEGIN_ARGS(SPGXChannelTableRow) {}
		SLATE_ARGUMENT(TSharedPtr<FPGXChannelRowData>, Item)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable)
	{
		Item = InArgs._Item;
		SMultiColumnTableRow::Construct(FSuperRowType::FArguments(), InOwnerTable);
	}

	TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
	{
		if (!Item.IsValid()) return SNullWidget::NullWidget;

		if (ColumnName == "Channel")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->LeafTag))
				.ToolTipText(FText::FromString(Item->FullTag))
			];
		}
		if (ColumnName == "Listeners")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock).Text(FText::AsNumber(Item->ListenerCount))
				.ColorAndOpacity(Item->ListenerCount > 0
					? FSlateColor(PGX::Semantic::Good)
					: FSlateColor(PGX::Text::Muted))
			];
		}
		if (ColumnName == "Broadcasts")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock).Text(FText::AsNumber(Item->BroadcastCount))
				.ColorAndOpacity(Item->BroadcastCount > 0
					? FSlateColor(PGX::Semantic::Info)
					: FSlateColor(PGX::Text::Muted))
			];
		}
		return SNullWidget::NullWidget;
	}

private:
	TSharedPtr<FPGXChannelRowData> Item;
};

class SPGXMessageHistoryTableRow : public SMultiColumnTableRow<TSharedPtr<FPGXMessageHistoryRowData>>
{
public:
	SLATE_BEGIN_ARGS(SPGXMessageHistoryTableRow) {}
		SLATE_ARGUMENT(TSharedPtr<FPGXMessageHistoryRowData>, Item)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable)
	{
		Item = InArgs._Item;
		SMultiColumnTableRow::Construct(FSuperRowType::FArguments(), InOwnerTable);
	}

	TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
	{
		if (!Item.IsValid()) return SNullWidget::NullWidget;

		if (ColumnName == "Time")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock)
				.Text(PGXEditorUtils::FormatRelativeTime(Item->Timestamp))
				.ToolTipText(FText::FromString(FString::Printf(TEXT("%.3f s"), Item->Timestamp)))
				.Font(PGX::Font::BodySmall())
			];
		}
		if (ColumnName == "Channel")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->LeafChannel))
				.ToolTipText(FText::FromString(Item->FullChannel))
			];
		}
		if (ColumnName == "Payload")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->PayloadType))
				.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
			];
		}
		if (ColumnName == "Notified")
		{
			return SNew(SBox).Padding(FMargin(4, 2))
			[
				SNew(STextBlock).Text(FText::AsNumber(Item->ListenersNotified))
			];
		}
		return SNullWidget::NullWidget;
	}

private:
	TSharedPtr<FPGXMessageHistoryRowData> Item;
};

// ========================================================================
// SPGXMessageInspectorTab Implementation
// ========================================================================

void SPGXMessageInspectorTab::Construct(const FArguments& /*InArgs*/)
{
	PGX_LOG_INFO(LogPGXMessage, TEXT("[MessageInspector] Construct — building panel with universal shell"));

	ChildSlot
	[
		// EN: Premium Shell — covers UE chrome, provides title bar + footer
		// ES: Shell Premium — cubre chrome de UE, provee title bar + footer
		SNew(SPGXPremiumShell)
		.SystemColor(PGX::System::Message)
		.Title(LOCTEXT("PanelTitle", "Message Inspector"))
		.Subtitle(LOCTEXT("PanelSubtitle", "Pub/Sub message bus live monitor"))
		.Icon(FPGXEditorStyle::Get().GetBrush("PGXEditor.Icon.Message"))
		.bShowFooter(true)
		.FooterLeftContent()
		[
			SAssignNew(FooterStatusText, STextBlock)
			.Text(LOCTEXT("FooterWaiting", "Waiting for PIE..."))
			.Font(PGX::Font::BodySmall())
			.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
		]
		.Content()
		[
			SAssignNew(ContentSwitcher, SWidgetSwitcher)
			.WidgetIndex(0)

			// EN: Slot 0 — Empty state (no PIE) / ES: Slot 0 — Estado vacio (sin PIE)
			+ SWidgetSwitcher::Slot()
			[
				SNew(SPGXEmptyStateV2)
				.Icon(FAppStyle::GetBrush("Icons.Warning"))
				.Message(LOCTEXT("EmptyReason", "No PIE session active"))
				.Hint(LOCTEXT("EmptySuggestion", "Start a Play-In-Editor session to see live message data"))
			]

			// EN: Slot 1 — Live content / ES: Slot 1 — Contenido live
			+ SWidgetSwitcher::Slot()
			[
				SNew(SVerticalBox)

				// EN: Status line / ES: Linea de estado
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 4)
				[
					SAssignNew(StatusText, STextBlock)
					.Text(LOCTEXT("StatusLoading", "Status: Waiting for PIE..."))
					.Font(PGX::Font::Body())
				]

				// EN: KPI chips row / ES: Fila de chips KPI
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 4)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
					[
						SNew(SPGXKPIChip)
						.Label(LOCTEXT("ChannelsKPI", "CHANNELS"))
						.AccentColor(PGX::System::Message)
						.ValueWidget()
						[
							SAssignNew(ChannelsText, STextBlock)
							.Text(LOCTEXT("ChannelsDefault", "0"))
							.Font(PGX::Font::KPIValue())
						]
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
					[
						SNew(SPGXKPIChip)
						.Label(LOCTEXT("ListenersKPI", "LISTENERS"))
						.AccentColor(PGX::System::Message)
						.ValueWidget()
						[
							SAssignNew(ListenersText, STextBlock)
							.Text(LOCTEXT("ListenersDefault", "0"))
							.Font(PGX::Font::KPIValue())
						]
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(SPGXKPIChip)
						.Label(LOCTEXT("BroadcastsKPI", "BROADCASTS"))
						.AccentColor(PGX::System::Message)
						.ValueWidget()
						[
							SAssignNew(BroadcastsText, STextBlock)
							.Text(LOCTEXT("BroadcastsDefault", "0"))
							.Font(PGX::Font::KPIValue())
						]
					]
				]

				// EN: Body (scrollable sections) / ES: Cuerpo (secciones scrolleables)
				+ SVerticalBox::Slot().FillHeight(1.0f)
				[
					SNew(SScrollBox)
					+ SScrollBox::Slot().Padding(0, 4)
					[
						BuildChannelsSection()
					]
					+ SScrollBox::Slot().Padding(0, 0)
					[
						SNew(SSeparator)
					]
					+ SScrollBox::Slot().Padding(0, 4)
					[
						BuildHistorySection()
					]
					+ SScrollBox::Slot().Padding(0, 0)
					[
						SNew(SSeparator)
					]
					+ SScrollBox::Slot().Padding(0, 4)
					[
						BuildActionsSection()
					]
				]
			]
		]
	];

	// EN: Bind PIE lifecycle delegates (S1) / ES: Bindear delegates de ciclo de vida PIE (S1)
	BindPIEDelegates();

	// EN: If PIE is already running, bind immediately / ES: Si PIE ya esta corriendo, bindear inmediatamente
	if (GEditor && GEditor->PlayWorld)
	{
		PGX_LOG_INFO(LogPGXMessage, TEXT("[MessageInspector] PIE already active at Construct time — binding to subsystem"));
		BindToSubsystem();
		if (ContentSwitcher.IsValid())
		{
			ContentSwitcher->SetActiveWidgetIndex(1);
		}
	}
}

SPGXMessageInspectorTab::~SPGXMessageInspectorTab()
{
	PGX_LOG_INFO(LogPGXMessage, TEXT("[MessageInspector] Destructor — unbinding all delegates"));

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

void SPGXMessageInspectorTab::BindPIEDelegates()
{
	PIEStartedHandle = FEditorDelegates::PostPIEStarted.AddSP(SharedThis(this), &SPGXMessageInspectorTab::OnPIEStarted);
	PIEEndedHandle = FEditorDelegates::EndPIE.AddSP(SharedThis(this), &SPGXMessageInspectorTab::OnPIEEnded);
	PGX_LOG_INFO(LogPGXMessage, TEXT("[MessageInspector] PIE delegates bound (PostPIEStarted + EndPIE)"));
}

void SPGXMessageInspectorTab::OnPIEStarted(bool /*bIsSimulating*/)
{
	PGX_LOG_INFO(LogPGXMessage, TEXT("[MessageInspector] OnPIEStarted — switching to live view"));

	BindToSubsystem();

	if (ContentSwitcher.IsValid())
	{
		ContentSwitcher->SetActiveWidgetIndex(1);
	}
	if (FooterStatusText.IsValid())
	{
		FooterStatusText->SetText(LOCTEXT("FooterConnected", "Connected to PIE"));
		FooterStatusText->SetColorAndOpacity(FSlateColor(PGX::Semantic::Good));
	}
}

void SPGXMessageInspectorTab::OnPIEEnded(bool /*bIsSimulating*/)
{
	PGX_LOG_INFO(LogPGXMessage, TEXT("[MessageInspector] OnPIEEnded — switching to empty state"));

	UnbindFromSubsystem();

	if (ContentSwitcher.IsValid())
	{
		ContentSwitcher->SetActiveWidgetIndex(0);
	}
}

void SPGXMessageInspectorTab::BindToSubsystem()
{
	UPGXMessageSubsystem* MsgSub = GetMessageSubsystem();
	if (!IsValid(MsgSub))
	{
		PGX_LOG_WARNING(LogPGXMessage, TEXT("[MessageInspector] BindToSubsystem — subsystem not available"));
		return;
	}

	BoundSubsystem = MsgSub;

	if (!BroadcastHandle.IsValid())
	{
		BroadcastHandle = MsgSub->OnMessageBroadcastNative.AddSP(
			SharedThis(this), &SPGXMessageInspectorTab::OnMessageBroadcast);
		PGX_LOG_INFO(LogPGXMessage, TEXT("[MessageInspector] Bound to OnMessageBroadcastNative"));
	}
}

void SPGXMessageInspectorTab::UnbindFromSubsystem()
{
	UPGXMessageSubsystem* MsgSub = BoundSubsystem.Get();
	if (IsValid(MsgSub) && BroadcastHandle.IsValid())
	{
		MsgSub->OnMessageBroadcastNative.Remove(BroadcastHandle);
		PGX_LOG_INFO(LogPGXMessage, TEXT("[MessageInspector] Unbound from OnMessageBroadcastNative"));
	}
	BroadcastHandle.Reset();
	BoundSubsystem.Reset();
}

void SPGXMessageInspectorTab::OnMessageBroadcast(
	FGameplayTag Channel, const UScriptStruct* StructType, double Timestamp)
{
	// EN: Apply channel filter — skip if not matching selected channel / ES: Aplicar filtro de canal
	if (SelectedChannelFilter.IsValid() && Channel != SelectedChannelFilter)
	{
		return;
	}

	auto Row = MakeShared<FPGXMessageHistoryRowData>();
	Row->Timestamp = Timestamp;
	Row->FullChannel = Channel.ToString();
	Row->LeafChannel = PGXEditorUtils::TagToLeafName(Channel);
	Row->PayloadType = StructType ? StructType->GetName() : TEXT("(unknown)");
	Row->ListenersNotified = BoundSubsystem.IsValid()
		? BoundSubsystem->GetListenerCount(Channel) : 0;

	HistoryItems.Insert(Row, 0);
	if (HistoryItems.Num() > 50)
	{
		HistoryItems.SetNum(50);
	}

	if (HistoryListView.IsValid())
	{
		HistoryListView->RequestListRefresh();
	}

	PGX_LOG_VERBOSE(LogPGXMessage, TEXT("[MessageInspector] OnMessageBroadcast — %s (%s) -> %d listeners"),
		*Channel.ToString(), StructType ? *StructType->GetName() : TEXT("null"), Row->ListenersNotified);
}

void SPGXMessageInspectorTab::OnChannelSelected(
	TSharedPtr<FPGXChannelRowData> Item, ESelectInfo::Type /*SelectInfo*/)
{
	if (Item.IsValid())
	{
		SelectedChannelFilter = Item->ChannelTag;
		PGX_LOG_INFO(LogPGXMessage, TEXT("[MessageInspector] Channel filter set: %s"), *Item->FullTag);
	}
	else
	{
		SelectedChannelFilter = FGameplayTag();
		PGX_LOG_INFO(LogPGXMessage, TEXT("[MessageInspector] Channel filter cleared (showing all)"));
	}
	RefreshHistorySection();
}

void SPGXMessageInspectorTab::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	RefreshAccumulator += InDeltaTime;
	if (RefreshAccumulator >= RefreshInterval)
	{
		RefreshAccumulator = 0.0f;
		RefreshData();
	}
}

// EN: BuildOverviewSection — now inlined into Construct via SPGXPremiumShell + SPGXKPIChip
// ES: BuildOverviewSection — ahora inline en Construct via SPGXPremiumShell + SPGXKPIChip

TSharedRef<SWidget> SPGXMessageInspectorTab::BuildChannelsSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("ChannelsTitle", "ACTIVE CHANNELS"))
			.AccentColor(PGX::System::Message)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 4)
		[
			SNew(SBox)
			.MaxDesiredHeight(150.0f)
			[
				SAssignNew(ChannelListView, SListView<TSharedPtr<FPGXChannelRowData>>)
				.ListItemsSource(&ChannelItems)
				.OnGenerateRow(this, &SPGXMessageInspectorTab::OnGenerateChannelRow)
				.OnSelectionChanged(this, &SPGXMessageInspectorTab::OnChannelSelected)
				.SelectionMode(ESelectionMode::SingleToggle)
				.HeaderRow
				(
					SNew(SHeaderRow)
					+ SHeaderRow::Column("Channel")
						.DefaultLabel(LOCTEXT("ChColChannel", "Channel"))
						.FillWidth(1.0f)
					+ SHeaderRow::Column("Listeners")
						.DefaultLabel(LOCTEXT("ChColListeners", "Listeners"))
						.FillWidth(0.4f)
					+ SHeaderRow::Column("Broadcasts")
						.DefaultLabel(LOCTEXT("ChColBroadcasts", "Broadcasts"))
						.FillWidth(0.4f)
				)
			]
		];
}

TSharedRef<SWidget> SPGXMessageInspectorTab::BuildHistorySection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("HistoryTitle", "RECENT MESSAGES"))
			.AccentColor(PGX::System::Message)
			.RightContent()
			[
				SAssignNew(ChannelFilterLabel, STextBlock)
				.Text(LOCTEXT("FilterAll", "All channels"))
				.Font(PGX::Font::Hint())
				.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 4)
		[
			SNew(SBox)
			.MaxDesiredHeight(200.0f)
			[
				SAssignNew(HistoryListView, SListView<TSharedPtr<FPGXMessageHistoryRowData>>)
				.ListItemsSource(&HistoryItems)
				.OnGenerateRow(this, &SPGXMessageInspectorTab::OnGenerateHistoryRow)
				.HeaderRow
				(
					SNew(SHeaderRow)
					+ SHeaderRow::Column("Time")
						.DefaultLabel(LOCTEXT("HiColTime", "Time"))
						.FillWidth(0.4f)
					+ SHeaderRow::Column("Channel")
						.DefaultLabel(LOCTEXT("HiColChannel", "Channel"))
						.FillWidth(1.0f)
					+ SHeaderRow::Column("Payload")
						.DefaultLabel(LOCTEXT("HiColPayload", "Payload"))
						.FillWidth(0.7f)
					+ SHeaderRow::Column("Notified")
						.DefaultLabel(LOCTEXT("HiColNotified", "Notified"))
						.FillWidth(0.3f)
				)
			]
		];
}

TSharedRef<SWidget> SPGXMessageInspectorTab::BuildActionsSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SPGXSectionDivider)
			.Title(LOCTEXT("ActionsTitle", "ACTIONS"))
			.AccentColor(PGX::System::Message)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 4)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("TestBroadcast", "Send Test Message"))
				.ToolTipText(LOCTEXT("TestBroadcastTip", "Broadcast a test message on PGX.Message.System channel"))
				.OnClicked_Lambda([this]() -> FReply
				{
					UPGXMessageSubsystem* MsgSub = BoundSubsystem.Get();
					if (IsValid(MsgSub))
					{
						FPGXMessage TestMsg;
						TestMsg.MessageTag = FGameplayTag::RequestGameplayTag(
							FName(TEXT("PGX.Message.System")), false);
						TestMsg.Timestamp = FPlatformTime::Seconds();
						if (TestMsg.MessageTag.IsValid())
						{
							MsgSub->BroadcastMessage<FPGXMessage>(TestMsg.MessageTag, TestMsg);
							PGX_LOG_INFO(LogPGXMessage, TEXT("[MessageInspector] Action: Send Test Message executed"));
						}
						else
						{
							PGX_LOG_WARNING(LogPGXMessage, TEXT("[MessageInspector] Action: PGX.Message.System tag not found"));
						}
					}
					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("ClearHistory", "Clear History"))
				.ToolTipText(LOCTEXT("ClearHistoryTip", "Clear the message history buffer. Does not affect listeners."))
				.OnClicked_Lambda([this]() -> FReply
				{
					UPGXMessageSubsystem* MsgSub = BoundSubsystem.Get();
					if (IsValid(MsgSub))
					{
						MsgSub->ClearHistory();
						HistoryItems.Empty();
						if (HistoryListView.IsValid())
						{
							HistoryListView->RequestListRefresh();
						}
						PGX_LOG_INFO(LogPGXMessage, TEXT("[MessageInspector] Action: Clear History executed"));
					}
					return FReply::Handled();
				})
			]
		];
}

TSharedRef<ITableRow> SPGXMessageInspectorTab::OnGenerateChannelRow(
	TSharedPtr<FPGXChannelRowData> Item, const TSharedRef<STableViewBase>& Owner)
{
	return SNew(SPGXChannelTableRow, Owner)
		.Item(Item);
}

TSharedRef<ITableRow> SPGXMessageInspectorTab::OnGenerateHistoryRow(
	TSharedPtr<FPGXMessageHistoryRowData> Item, const TSharedRef<STableViewBase>& Owner)
{
	return SNew(SPGXMessageHistoryTableRow, Owner)
		.Item(Item);
}

UPGXMessageSubsystem* SPGXMessageInspectorTab::GetMessageSubsystem() const
{
	if (GEditor)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::PIE && IsValid(Context.World()))
			{
				UGameInstance* GI = Context.World()->GetGameInstance();
				if (IsValid(GI))
				{
					return GI->GetSubsystem<UPGXMessageSubsystem>();
				}
			}
		}
	}
	return nullptr;
}

void SPGXMessageInspectorTab::RefreshData()
{
	UPGXMessageSubsystem* MsgSub = BoundSubsystem.Get();
	if (!IsValid(MsgSub))
	{
		// EN: Try to get it via GetMessageSubsystem (might become available between ticks)
		MsgSub = GetMessageSubsystem();
		if (IsValid(MsgSub))
		{
			BindToSubsystem();
		}
		else
		{
			if (ContentSwitcher.IsValid() && ContentSwitcher->GetActiveWidgetIndex() != 0)
			{
				ContentSwitcher->SetActiveWidgetIndex(0);
			}
			return;
		}
	}

	// EN: Ensure live view is active / ES: Asegurar que la vista live esta activa
	if (ContentSwitcher.IsValid() && ContentSwitcher->GetActiveWidgetIndex() != 1)
	{
		ContentSwitcher->SetActiveWidgetIndex(1);
	}

	const FPGXMessageStats MsgStats = MsgSub->GetStats();

	if (StatusText.IsValid())
	{
		StatusText->SetText(FText::FromString(FString::Printf(TEXT("Status: Active | History: %d"), MsgStats.HistorySize)));
	}
	if (ChannelsText.IsValid()) ChannelsText->SetText(FText::AsNumber(MsgStats.ActiveChannels));
	if (ListenersText.IsValid()) ListenersText->SetText(FText::AsNumber(MsgStats.ActiveListeners));
	if (BroadcastsText.IsValid()) BroadcastsText->SetText(FText::AsNumber(MsgStats.TotalBroadcasts));

	// EN: Build channel table with broadcast counts / ES: Construir tabla de canales con conteos de broadcasts
	{
		const TArray<FGameplayTag> Channels = MsgSub->GetAllActiveChannels();

		// EN: Count broadcasts per channel from full history / ES: Contar broadcasts por canal del historial completo
		const TArray<FPGXMessageRecord> AllHistory = MsgSub->GetMessageHistory(FGameplayTag(), 100);
		TMap<FGameplayTag, int32> BroadcastCounts;
		for (const FPGXMessageRecord& Record : AllHistory)
		{
			BroadcastCounts.FindOrAdd(Record.Channel)++;
		}

		ChannelItems.Empty(Channels.Num());
		for (const FGameplayTag& Ch : Channels)
		{
			auto Row = MakeShared<FPGXChannelRowData>();
			Row->ChannelTag = Ch;
			Row->FullTag = Ch.ToString();
			Row->LeafTag = PGXEditorUtils::TagToLeafName(Ch);
			Row->ListenerCount = MsgSub->GetListenerCount(Ch);
			Row->BroadcastCount = BroadcastCounts.Contains(Ch) ? BroadcastCounts[Ch] : 0;
			ChannelItems.Add(Row);
		}
		if (ChannelListView.IsValid()) ChannelListView->RequestListRefresh();
	}

	// EN: Refresh history section / ES: Refrescar seccion de historial
	RefreshHistorySection();

	// EN: Update footer / ES: Actualizar footer
	if (FooterStatusText.IsValid())
	{
		FooterStatusText->SetText(FText::Format(
			LOCTEXT("FooterLive", "Connected to PIE | {0} channels | {1} history entries"),
			FText::AsNumber(MsgSub->GetStats().ActiveChannels),
			FText::AsNumber(HistoryItems.Num())));
		FooterStatusText->SetColorAndOpacity(FSlateColor(PGX::Semantic::Good));
	}
}

void SPGXMessageInspectorTab::RefreshHistorySection()
{
	UPGXMessageSubsystem* MsgSub = BoundSubsystem.Get();
	if (!IsValid(MsgSub)) return;

	const TArray<FPGXMessageRecord> History = MsgSub->GetMessageHistory(SelectedChannelFilter, 50);
	HistoryItems.Empty(History.Num());
	for (const FPGXMessageRecord& Record : History)
	{
		auto Row = MakeShared<FPGXMessageHistoryRowData>();
		Row->Timestamp = Record.Timestamp;
		Row->FullChannel = Record.Channel.ToString();
		Row->LeafChannel = PGXEditorUtils::TagToLeafName(Record.Channel);
		Row->PayloadType = Record.PayloadTypeName;
		Row->ListenersNotified = Record.ListenersNotified;
		HistoryItems.Add(Row);
	}
	if (HistoryListView.IsValid()) HistoryListView->RequestListRefresh();

	UpdateChannelFilterLabel();
}

void SPGXMessageInspectorTab::UpdateChannelFilterLabel()
{
	if (!ChannelFilterLabel.IsValid()) return;

	if (SelectedChannelFilter.IsValid())
	{
		ChannelFilterLabel->SetText(FText::Format(
			LOCTEXT("FilterChannel", "Filtered: {0}"),
			FText::FromString(PGXEditorUtils::TagToLeafName(SelectedChannelFilter))));
		ChannelFilterLabel->SetColorAndOpacity(FSlateColor(GMessageInspectorColor));
	}
	else
	{
		ChannelFilterLabel->SetText(LOCTEXT("FilterAll", "All channels"));
		ChannelFilterLabel->SetColorAndOpacity(FSlateColor(PGX::Text::Muted));
	}
}

#undef LOCTEXT_NAMESPACE
