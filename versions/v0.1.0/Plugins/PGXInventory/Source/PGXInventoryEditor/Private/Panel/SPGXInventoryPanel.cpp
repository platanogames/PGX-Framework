// Copyright PGX Framework. All Rights Reserved.

#include "Panel/SPGXInventoryPanel.h"

#include "PGXItemDefinition.h"
#include "PGXInventoryComponent.h"
#include "Style/PGXVisualTokens.h"

#include "Observability/PGXObservable.h"
#include "Observability/PGXSchemaDescriptor.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "UObject/UObjectIterator.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Brushes/SlateColorBrush.h"

#define LOCTEXT_NAMESPACE "PGXInventory"

void SPGXInventoryPanel::Construct(const FArguments& InArgs)
{
	// EN: Widget-owned brushes — leak prevention per the panel ownership policy.
	SurfaceBaseBrush   = MakeShared<FSlateColorBrush>(PGX::Surface::Base);
	SurfaceRaisedBrush = MakeShared<FSlateColorBrush>(PGX::Surface::Raised);
	AccentBrush        = MakeShared<FSlateColorBrush>(PGX::System::Inventory);

	ChildSlot
	[
		SNew(SBorder).BorderImage(SurfaceBaseBrush.Get()).Padding(FMargin(PGX::Spacing::MD))
		[
			SNew(SVerticalBox)

			// ─── Header ───
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 0.0f, 0.0f, PGX::Spacing::MD))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SBorder).BorderImage(AccentBrush.Get()).Padding(FMargin(0.0f))
					[
						SNew(SBox).WidthOverride(PGX::Width::AccentStripe).HeightOverride(PGX::Height::PanelHeader)
					]
				]
				+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(FMargin(PGX::Spacing::MD, 0.0f, 0.0f, 0.0f)).VAlign(VAlign_Center)
				[
					SNew(STextBlock).Text(LOCTEXT("PanelTitle", "PGX Inventory")).Font(PGX::Font::PanelTitle()).ColorAndOpacity(FSlateColor(PGX::Text::Primary))
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(SButton).OnClicked(this, &SPGXInventoryPanel::OnRefreshClicked)
					[
						SNew(STextBlock).Text(LOCTEXT("RefreshButton", "Refresh")).Font(PGX::Font::Body())
					]
				]
			]

			// ─── Body ───
			+ SVerticalBox::Slot().FillHeight(1.0f)
			[
				SNew(SScrollBox)

				// Section A: ItemDefinition IPGXObservable observable
				+ SScrollBox::Slot().Padding(FMargin(0.0f, 0.0f, 0.0f, PGX::Spacing::SM))
				[
					SNew(SBorder).BorderImage(SurfaceRaisedBrush.Get()).Padding(FMargin(PGX::Spacing::MD))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(STextBlock).Text(LOCTEXT("ItemDefHeader", "UPGXItemDefinition assets — IPGXObservable observable")).Font(PGX::Font::SectionHeader()).ColorAndOpacity(FSlateColor(PGX::Text::Primary))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, PGX::Spacing::XS, 0.0f, 0.0f))
						[
							SAssignNew(ItemDefCountText, STextBlock).Font(PGX::Font::KPIValue()).ColorAndOpacity(FSlateColor(PGX::System::Inventory))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, PGX::Spacing::XS, 0.0f, 0.0f))
						[
							SNew(STextBlock).Text(LOCTEXT("ItemDefHint", "UPGXItemDefinition : UPGXDataAsset, IPGXObservable — observable schema support. Schema canonical export ToJson() + GetSchemaDescriptor() available.")).Font(PGX::Font::Hint()).ColorAndOpacity(FSlateColor(PGX::Text::Secondary)).AutoWrapText(true)
						]
					]
				]

				// Section B: Component live state (runtime-derived)
				+ SScrollBox::Slot().Padding(FMargin(0.0f, 0.0f, 0.0f, PGX::Spacing::SM))
				[
					SNew(SBorder).BorderImage(SurfaceRaisedBrush.Get()).Padding(FMargin(PGX::Spacing::MD))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(STextBlock).Text(LOCTEXT("ComponentHeader", "UPGXInventoryComponent loaded subclass count (NOT live instance count)")).Font(PGX::Font::SectionHeader()).ColorAndOpacity(FSlateColor(PGX::Text::Primary))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, PGX::Spacing::XS, 0.0f, 0.0f))
						[
							SAssignNew(ComponentCountText, STextBlock).Font(PGX::Font::KPIValue()).ColorAndOpacity(FSlateColor(PGX::System::Inventory))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, PGX::Spacing::XS, 0.0f, 0.0f))
						[
							SNew(STextBlock).Text(LOCTEXT("ComponentHint", "Counter shows LOADED subclasses (native + BP-generated) of UPGXInventoryComponent via TObjectIterator IsChildOf — NOT runtime live instance count. Live instance enumeration is not provided; it requires a per-world component traversal during PIE. Component API real: AddItem/RemoveItem/TransferItemTo/GetItemQuantity/GetUsedSlotCount/GetCurrentWeight/GetItemsSnapshot.")).Font(PGX::Font::Hint()).ColorAndOpacity(FSlateColor(PGX::Text::Secondary)).AutoWrapText(true)
						]
					]
				]

				// Section C: Subsystem GAP + Schema demo button
				+ SScrollBox::Slot().Padding(FMargin(0.0f, 0.0f, 0.0f, PGX::Spacing::SM))
				[
					SNew(SBorder).BorderImage(SurfaceRaisedBrush.Get()).Padding(FMargin(PGX::Spacing::MD))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(STextBlock).Text(LOCTEXT("SubsystemHeader", "Subsystem GAP + ItemDefinition schema demo")).Font(PGX::Font::SectionHeader()).ColorAndOpacity(FSlateColor(PGX::Text::Primary))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, PGX::Spacing::XS, 0.0f, 0.0f))
						[
							SNew(STextBlock).Text(LOCTEXT("SubsystemGap", "UPGXInventorySubsystem — global inventory coordinator (API surface in active development).")).Font(PGX::Font::Body()).ColorAndOpacity(FSlateColor(PGX::Text::Secondary)).AutoWrapText(true)
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, PGX::Spacing::SM, 0.0f, 0.0f))
						[
							SNew(SButton).OnClicked(this, &SPGXInventoryPanel::OnShowSchemaClicked)
							[
								SNew(STextBlock).Text(LOCTEXT("SchemaButton", "Show ItemDefinition schema descriptor (preview)")).Font(PGX::Font::Body())
							]
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, PGX::Spacing::SM, 0.0f, 0.0f))
						[
							SAssignNew(SchemaText, STextBlock).Font(PGX::Font::Mono()).ColorAndOpacity(FSlateColor(PGX::Text::Secondary)).AutoWrapText(true)
						]
					]
				]
			]
		]
	];

	RefreshState();
}

FReply SPGXInventoryPanel::OnRefreshClicked()
{
	RefreshState();
	return FReply::Handled();
}

void SPGXInventoryPanel::RefreshState()
{
	// EN: ItemDefinition asset inventory + Component derived classes loaded
	// ES: Inventario ItemDefinition + clases Component derivadas cargadas
	int32 ItemDefCount = 0;
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		TArray<FAssetData> Found;
		AssetRegistryModule.Get().GetAssetsByClass(UPGXItemDefinition::StaticClass()->GetClassPathName(), Found, /*bSearchSubClasses=*/true);
		ItemDefCount = Found.Num();
	}

	int32 ComponentDerived = 0;
	for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
	{
		UClass* Class = *ClassIt;
		if (!Class || Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists))
		{
			continue;
		}
		if (Class->IsChildOf(UPGXInventoryComponent::StaticClass()))
		{
			++ComponentDerived;
		}
	}

	if (ItemDefCountText.IsValid())
	{
		ItemDefCountText->SetText(FText::AsNumber(ItemDefCount));
	}
	if (ComponentCountText.IsValid())
	{
		ComponentCountText->SetText(FText::AsNumber(ComponentDerived));
	}
}

FReply SPGXInventoryPanel::OnShowSchemaClicked()
{
	// EN: Demo schema descriptor via ItemDefinition CDO (IPGXObservable observable).
	// ES: Demo schema descriptor via CDO ItemDefinition (IPGXObservable observable).
	const UPGXItemDefinition* DefaultCDO = GetDefault<UPGXItemDefinition>();
	FString Summary;
	if (DefaultCDO)
	{
		const FPGXSchemaDescriptor Descriptor = DefaultCDO->GetSchemaDescriptor();
		Summary = FString::Printf(TEXT("{\n  \"schema\": {\n    \"type\": \"%s\",\n    \"version\": \"%s\",\n    \"plugin\": \"%s\"\n  },\n  \"fields\": [\n"),
			*Descriptor.TypeName.ToString(),
			*Descriptor.SchemaVersion.ToString(),
			*Descriptor.OwningPlugin.ToString());
		for (const FPGXSchemaField& Field : Descriptor.Fields)
		{
			Summary += FString::Printf(TEXT("    { \"name\": \"%s\", \"type\": \"%s\", \"required\": %s },\n"),
				*Field.FieldName.ToString(),
				*Field.FieldType.ToString(),
				Field.bRequired ? TEXT("true") : TEXT("false"));
		}
		Summary += TEXT("  ]\n}");
	}
	else
	{
		Summary = TEXT("UPGXItemDefinition::GetDefault() returned null — IPGXObservable demo unavailable.");
	}

	if (SchemaText.IsValid())
	{
		SchemaText->SetText(FText::FromString(Summary));
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
