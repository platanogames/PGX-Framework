// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Logging/PGXLogDomainRendererBase.h"
#include "Logging/PGXLogTagHelper.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "PGXLogDomainRenderer"

namespace
{
	FText VerbosityToShortLabel(EPGXLogVerbosity V)
	{
		switch (V)
		{
		case EPGXLogVerbosity::Verbose: return LOCTEXT("VRB", "VRB");
		case EPGXLogVerbosity::Debug:   return LOCTEXT("DBG", "DBG");
		case EPGXLogVerbosity::Info:    return LOCTEXT("INF", "INF");
		case EPGXLogVerbosity::Warning: return LOCTEXT("WRN", "WRN");
		case EPGXLogVerbosity::Error:   return LOCTEXT("ERR", "ERR");
		case EPGXLogVerbosity::Fatal:   return LOCTEXT("FTL", "FTL");
		default:                        return LOCTEXT("UNK", "???");
		}
	}
}

// ═══════════════════════════════════════════════════════════════
// Data Methods — Default Implementations
// ═══════════════════════════════════════════════════════════════

FLinearColor UPGXLogDomainRendererBase::GetDomainColor_Implementation() const
{
	return FLinearColor(0.5f, 0.5f, 0.5f); // Gray default
}

FText UPGXLogDomainRendererBase::GetDomainDisplayName_Implementation() const
{
	return LOCTEXT("GenericDomain", "Generic");
}

TArray<FPGXLogColumnDef> UPGXLogDomainRendererBase::GetColumnDefinitions_Implementation() const
{
	// EN: Default: no extra columns / ES: Default: sin columnas extra
	return TArray<FPGXLogColumnDef>();
}

bool UPGXLogDomainRendererBase::SupportsDetailWindow_Implementation() const
{
	return true;
}

// ═══════════════════════════════════════════════════════════════
// Slate Widget Methods — Default Row & Detail
// ═══════════════════════════════════════════════════════════════

TSharedRef<SWidget> UPGXLogDomainRendererBase::BuildRowWidget(const FPGXLogEntry& Entry) const
{
	const FLinearColor VerbColor = FPGXLogTagHelper::GetVerbosityColor(Entry.Verbosity);
	const FLinearColor DomainColor = GetDomainColor();
	const FString TimeStr = Entry.Timestamp.ToString(TEXT("%H:%M:%S"));

	// EN: Standard row: Time | Verbosity badge | Category | Message
	// ES: Fila estandar: Tiempo | Badge verbosity | Categoria | Mensaje
	TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox)

		// EN: Time column / ES: Columna de tiempo
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(70.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TimeStr))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
				.Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
			]
		]

		// EN: Verbosity badge / ES: Badge de verbosity
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(2.0f, 0.0f)
		[
			SNew(SBox)
			.WidthOverride(40.0f)
			[
				SNew(SBorder)
				.BorderBackgroundColor(VerbColor * 0.3f)
				.Padding(FMargin(4.0f, 1.0f))
				[
					SNew(STextBlock)
					.Text(VerbosityToShortLabel(Entry.Verbosity))
					.ColorAndOpacity(FSlateColor(VerbColor))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 8))
					.Justification(ETextJustify::Center)
				]
			]
		]

		// EN: Domain color dot / ES: Punto de color de dominio
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4.0f, 0.0f, 2.0f, 0.0f)
		[
			SNew(SBox)
			.WidthOverride(8.0f)
			.HeightOverride(8.0f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderBackgroundColor(DomainColor)
			]
		]

		// EN: Category column / ES: Columna de categoria
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(2.0f, 0.0f)
		[
			SNew(SBox)
			.WidthOverride(100.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromName(Entry.Category))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.85f)))
				.Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			]
		];

	// EN: Add domain-specific columns from Params / ES: Agregar columnas de dominio desde Params
	const TArray<FPGXLogColumnDef> Columns = GetColumnDefinitions();
	for (const FPGXLogColumnDef& Col : Columns)
	{
		const FString ParamVal = GetParamValue(Entry, Col.ParamKey);
		Row->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4.0f, 0.0f)
			[
				SNew(SBox)
				.WidthOverride(Col.Width)
				[
					SNew(STextBlock)
					.Text(FText::FromString(ParamVal))
					.ColorAndOpacity(FSlateColor(DomainColor * 0.8f + FLinearColor(0.2f, 0.2f, 0.2f)))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				]
			];
	}

	// EN: Message column (fills remaining width) / ES: Columna de mensaje (llena ancho restante)
	Row->AddSlot()
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		.Padding(4.0f, 0.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Entry.Message))
			.ColorAndOpacity(FSlateColor(VerbColor))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
		];

	return Row;
}

TSharedRef<SWidget> UPGXLogDomainRendererBase::BuildDetailWidget(const FPGXLogEntry& Entry) const
{
	const FLinearColor DomainColor = GetDomainColor();

	TSharedRef<SVerticalBox> Detail = SNew(SVerticalBox)

		// EN: Header with domain color accent / ES: Header con acento de color de dominio
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
			.BorderBackgroundColor(DomainColor)
			.Padding(0.0f)
			[
				SNew(SBox).HeightOverride(2.0f)
			]
		]

		// EN: Message / ES: Mensaje
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 8.0f, 8.0f, 4.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Entry.Message))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
			.AutoWrapText(true)
		]

		// EN: Timestamp + Category / ES: Timestamp + Categoria
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 0.0f, 8.0f, 8.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("%s | %s | Frame %llu"),
				*Entry.Timestamp.ToString(TEXT("%H:%M:%S.%s")),
				*Entry.Category.ToString(),
				Entry.FrameNumber)))
			.Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
		];

	// EN: Parameters section / ES: Seccion de parametros
	if (Entry.Params.Num() > 0)
	{
		Detail->AddSlot()
			.AutoHeight()
			.Padding(8.0f, 4.0f, 8.0f, 2.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("DetailParams", "Parameters"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
				.ColorAndOpacity(FSlateColor(DomainColor))
			];

		for (const FPGXLogParam& Param : Entry.Params)
		{
			Detail->AddSlot()
				.AutoHeight()
				.Padding(16.0f, 1.0f, 8.0f, 1.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(120.0f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(Param.Key))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
							.ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.6f)))
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(Param.Value))
						.Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
						.AutoWrapText(true)
					]
				];
		}
	}

	// EN: Tags section / ES: Seccion de tags
	if (!Entry.Tags.IsEmpty())
	{
		Detail->AddSlot()
			.AutoHeight()
			.Padding(8.0f, 4.0f, 8.0f, 2.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("DetailTags", "Tags"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.8f, 0.8f)))
			];

		Detail->AddSlot()
			.AutoHeight()
			.Padding(16.0f, 1.0f, 8.0f, 4.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Entry.Tags.ToStringSimple()))
				.Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
				.AutoWrapText(true)
				.ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.7f, 0.7f)))
			];
	}

	return Detail;
}

// ═══════════════════════════════════════════════════════════════
// Helpers
// ═══════════════════════════════════════════════════════════════

FString UPGXLogDomainRendererBase::GetParamValue(const FPGXLogEntry& Entry, const FString& Key)
{
	for (const FPGXLogParam& Param : Entry.Params)
	{
		if (Param.Key == Key)
		{
			return Param.Value;
		}
	}
	return FString();
}

#undef LOCTEXT_NAMESPACE
