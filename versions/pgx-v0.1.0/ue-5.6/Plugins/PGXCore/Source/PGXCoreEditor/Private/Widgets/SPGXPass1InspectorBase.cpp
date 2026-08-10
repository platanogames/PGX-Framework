// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Widgets/SPGXPass1InspectorBase.h"
#include "Helpers/PGXInspectorHelpers.h"
#include "Style/PGXVisualTokens.h"

#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "PGXPass1InspectorBase"

void SPGXPass1InspectorBase::BuildInspectorLayout()
{
	const TArray<TPair<FText, UClass*>> Observables = GetObservableClasses();
	const TArray<TPair<FText, FText>>   Deferred    = GetDeferredCards();

	TSharedRef<SVerticalBox> Body = SNew(SVerticalBox);

	// EN: Header row.
	// ES: Fila de cabecera.
	Body->AddSlot().AutoHeight().Padding(PGX::Spacing::MD, PGX::Spacing::SM)
	[
		BuildHeader()
	];

	// EN: Observable cards.
	// ES: Observable cards.
	for (const TPair<FText, UClass*>& Pair : Observables)
	{
		Body->AddSlot().AutoHeight().Padding(PGX::Spacing::MD, PGX::Spacing::SM)
		[
			PGX::Inspector::MakeObservableCard(Pair.Key, Pair.Value)
		];
	}

	// EN: Deferred cards.
	// ES: Deferred cards.
	for (const TPair<FText, FText>& Pair : Deferred)
	{
		Body->AddSlot().AutoHeight().Padding(PGX::Spacing::MD, PGX::Spacing::SM)
		[
			PGX::Inspector::MakeDeferredCard(Pair.Key, Pair.Value)
		];
	}

	ChildSlot
	[
		SNew(SScrollBox)
		+ SScrollBox::Slot().Padding(PGX::Spacing::MD)
		[
			Body
		]
	];
}

FText SPGXPass1InspectorBase::GetInspectorTitle() const
{
	return LOCTEXT("DefaultHeader", "PASS1 Inspector");
}

TArray<TPair<FText, UClass*>> SPGXPass1InspectorBase::GetObservableClasses() const
{
	return {};
}

TArray<TPair<FText, FText>> SPGXPass1InspectorBase::GetDeferredCards() const
{
	return {};
}

TSharedRef<SWidget> SPGXPass1InspectorBase::BuildHeader() const
{
	return SNew(STextBlock).Text(GetInspectorTitle());
}

#undef LOCTEXT_NAMESPACE
