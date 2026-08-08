// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Helpers/PGXInspectorHelpers.h"
#include "Style/PGXVisualTokens.h"

#include "Observability/PGXObservable.h"
#include "Observability/PGXObservabilityRegistry.h"
#include "Observability/PGXSchemaDescriptor.h"

#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "PGXInspectorHelpers"

namespace PGX
{
	namespace Inspector
	{
		TSharedRef<SWidget> MakeCard(const FText& Label, const FText& Detail)
		{
			return SNew(SBorder)
				.Padding(PGX::Spacing::MD)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(Label)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, PGX::Spacing::SM, 0.0f, 0.0f)
					[
						SNew(STextBlock)
						.Text(Detail)
						.AutoWrapText(true)
					]
				];
		}

		TSharedRef<SWidget> MakeObservableCard(const FText& Label, UClass* ObservableClass)
		{
			return MakeCard(Label, DescribeObservableClass(ObservableClass));
		}

		TSharedRef<SWidget> MakeDeferredCard(const FText& Label, const FText& Detail)
		{
			return MakeCard(Label, Detail);
		}

		FText DescribeObservableClass(UClass* ObservableClass)
		{
			if (!ObservableClass)
			{
				return LOCTEXT("MissingClass", "Class missing; cannot inspect observable contract.");
			}

			UObject* DefaultObject = ObservableClass->GetDefaultObject();
			const IPGXObservable* Observable = Cast<IPGXObservable>(DefaultObject);
			if (!Observable)
			{
				return FText::FromString(FString::Printf(TEXT("%s does not implement IPGXObservable."), *ObservableClass->GetName()));
			}

			const FPGXSchemaDescriptor Descriptor = Observable->GetSchemaDescriptor();
			const bool bRegistered = FPGXObservabilityRegistry::FindClassByTypeName(ObservableClass->GetFName()) != nullptr;
			return FText::FromString(FString::Printf(
				TEXT("Class=%s | Schema=%s | Fields=%d | Registry=%s"),
				*Descriptor.TypeName.ToString(),
				*Descriptor.SchemaVersion.ToString(),
				Descriptor.Fields.Num(),
				bRegistered ? TEXT("Registered") : TEXT("Not registered yet")));
		}
	}
}

#undef LOCTEXT_NAMESPACE
