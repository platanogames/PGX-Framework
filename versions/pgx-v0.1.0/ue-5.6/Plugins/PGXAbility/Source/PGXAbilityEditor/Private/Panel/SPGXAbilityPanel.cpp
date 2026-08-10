// Copyright PGX Framework. All Rights Reserved.

#include "Panel/SPGXAbilityPanel.h"

#include "PGXAbilitySubsystem.h"
#include "PGXAbilityComponent.h"

#include "Style/PGXVisualTokens.h"
#include "Widgets/SPGXPanelHeader.h"

#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "PGXAbilityPanel"

void SPGXAbilityPanel::Construct(const FArguments& /*InArgs*/)
{
	ChildSlot
	[
		SNew(SVerticalBox)

		// EN: PGX PanelHeader (canonical reusable) — accent color PGX::System::Ability.
		// ES: PGX PanelHeader (reusable canonical) — color accent PGX::System::Ability.
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SPGXPanelHeader)
			.SystemColor(PGX::System::Ability)
			.Title(LOCTEXT("PGXAbilityHeaderTitle", "PGX Ability"))
			.Subtitle(LOCTEXT(
				"PGXAbilityHeaderSubtitle",
				"snapshot preview — one-shot snapshot (live reactive refresh pending)"))
		]

		// EN: Body — one-shot status snapshot + manual refresh. Honest snapshot preview: the blocking
		//     accessors are live, but this view does not yet subscribe to the native delegates
		//     for automatic refresh; this panel currently refreshes only when rebuilt.
		// ES: Cuerpo — snapshot de estado one-shot + refresh manual.
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(PGX::Spacing::XL, PGX::Spacing::LG, PGX::Spacing::XL, PGX::Spacing::LG)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, PGX::Spacing::MD)
			[
				SAssignNew(StatusTextBlock, STextBlock)
				.Text(this, &SPGXAbilityPanel::GetStatusText)
				.Font(PGX::Font::Body())
				.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
				.AutoWrapText(true)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				.Text(LOCTEXT("PGXAbilityRefresh", "Refresh"))
				.OnClicked(this, &SPGXAbilityPanel::OnRefreshClicked)
			]
		]
	];
}

FReply SPGXAbilityPanel::OnRefreshClicked()
{
	if (StatusTextBlock.IsValid())
	{
		StatusTextBlock->SetText(GetStatusText());
	}
	return FReply::Handled();
}

FText SPGXAbilityPanel::GetStatusText() const
{
	if (!GEngine)
	{
		return LOCTEXT("PGXAbilityNoEngine", "Engine unavailable.");
	}

	// EN: Editor-context world (PIE if running, else editor world) — this panel is editor-only
	//     observability, not a runtime gameplay surface.
	// ES: Mundo de contexto editor (PIE si corre, si no mundo de editor).
	const UWorld* World = nullptr;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game)
		{
			World = Context.World();
			break;
		}
	}
	if (!World)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::Editor)
			{
				World = Context.World();
				break;
			}
		}
	}

	if (!World)
	{
		return LOCTEXT("PGXAbilityNoWorld", "No active world — open or play a level to inspect PGXAbility state.");
	}

	const UGameInstance* GameInstance = World->GetGameInstance();
	const UPGXAbilitySubsystem* Subsystem = GameInstance ? GameInstance->GetSubsystem<UPGXAbilitySubsystem>() : nullptr;
	if (!Subsystem)
	{
		return LOCTEXT("PGXAbilityNoSubsystem", "UPGXAbilitySubsystem not available for this world (requires a running GameInstance, e.g. PIE).");
	}

	const int32 ComponentCount = Subsystem->GetRegisteredComponentCount();
	const int32 ActiveAbilityCount = Subsystem->GetActiveAbilityCount();

	if (ComponentCount == 0)
	{
		return FText::Format(
			LOCTEXT("PGXAbilityEmptyState", "Subsystem ready. 0 UPGXAbilityComponent instances registered.\n\nAdd a PGXAbilityComponent to an actor and enter PIE to see live data here."),
			FText::AsNumber(ComponentCount));
	}

	return FText::Format(
		LOCTEXT("PGXAbilitySnapshot", "Subsystem ready.\nRegistered components: {0}\nAggregate active abilities: {1}\n\n(snapshot — press Refresh for current state; GetAttributeWatcherCount not implemented, no watcher concept exists yet)"),
		FText::AsNumber(ComponentCount),
		FText::AsNumber(ActiveAbilityCount));
}

#undef LOCTEXT_NAMESPACE
