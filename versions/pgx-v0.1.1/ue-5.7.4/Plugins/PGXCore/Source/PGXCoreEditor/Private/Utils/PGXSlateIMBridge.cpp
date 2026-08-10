// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Utils/PGXSlateIMBridge.h"
#include "SlateIM.h"
#include "Style/PGXEditorStyle.h"

void PGXSlateIM::QuickGraph(
	const TArrayView<double>& Values,
	const FLinearColor& Color,
	float Thickness,
	double MinY,
	double MaxY)
{
	SlateIM::BeginGraph();
	SlateIM::GraphLine(Values, Color, Thickness, FDoubleRange(MinY, MaxY));
	SlateIM::EndGraph();
}

void PGXSlateIM::MultiGraph(
	const TArray<FGraphLine>& Lines,
	double MinY,
	double MaxY)
{
	SlateIM::BeginGraph();
	for (const FGraphLine& Line : Lines)
	{
		SlateIM::GraphLine(Line.Values, Line.Color, Line.Thickness, FDoubleRange(MinY, MaxY));
	}
	SlateIM::EndGraph();
}

void PGXSlateIM::Title(const FStringView& InText)
{
	const FTextBlockStyle* Style = &FPGXEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>("PGXEditor.Font.PanelTitle");
	SlateIM::Text(InText, Style);
}

void PGXSlateIM::Section(const FStringView& InText)
{
	const FTextBlockStyle* Style = &FPGXEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>("PGXEditor.Font.SectionHeader");
	SlateIM::Text(InText, Style);
}

void PGXSlateIM::Label(const FStringView& InText)
{
	const FTextBlockStyle* Style = &FPGXEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>("PGXEditor.Font.Body");
	SlateIM::Text(InText, Style);
}

void PGXSlateIM::Muted(const FStringView& InText)
{
	SlateIM::Text(InText, FSlateColor(PGX::Text::Muted));
}

void PGXSlateIM::Status(const FStringView& InText, const FLinearColor& Color)
{
	SlateIM::Text(InText, FSlateColor(Color));
}

void PGXSlateIM::Separator()
{
	// EN: SlateIM has no Separator — use Spacer as visual break / ES: SlateIM no tiene Separator — usar Spacer
	SlateIM::Spacer(FVector2D(0.0, 4.0));
}
