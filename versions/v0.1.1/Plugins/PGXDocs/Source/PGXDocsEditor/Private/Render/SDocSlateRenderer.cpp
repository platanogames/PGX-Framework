// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Render/SDocSlateRenderer.h"
#include "PGXDocsConfig.h"

#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Widgets/Text/SMultiLineEditableText.h"
#include "Widgets/SBoxPanel.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/CoreStyle.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "PGXDocs"

// ============================================================================
// EN: Private markdown-renderer palette inspired by VS Code Dark+.
//     It remains local because no other PGX plugin currently shares this renderer.
// ES: Paleta privada del renderer markdown inspirada en VS Code Dark+.
//     Permanece local porque ningun otro plugin PGX comparte este renderer.
// ============================================================================
namespace DocColors
{
	static const FLinearColor Background(0.118f, 0.118f, 0.118f);     // #1e1e1e
	static const FLinearColor Text(0.831f, 0.831f, 0.831f);           // #d4d4d4
	static const FLinearColor Link(0.306f, 0.788f, 0.690f);           // #4ec9b0
	static const FLinearColor CodeBg(0.176f, 0.176f, 0.176f);         // #2d2d2d
	static const FLinearColor Border(0.243f, 0.243f, 0.243f);         // #3e3e3e
	static const FLinearColor BlockquoteBorder(0.337f, 0.612f, 0.839f); // #569cd6
	static const FLinearColor InlineCode(0.863f, 0.863f, 0.667f);     // #dcdcaa

	// EN: Heading colors — clear visual hierarchy per level
	// ES: Colores de heading — jerarquia visual clara por nivel
	static const FLinearColor H1(0.878f, 0.918f, 0.976f);             // #e0eafa — luminous white-blue (page title)
	static const FLinearColor H2(0.400f, 0.667f, 0.878f);             // #66aae0 — bright blue (section)
	static const FLinearColor H3(0.380f, 0.710f, 0.659f);             // #61b5a8 — teal (subsection — breaks blue family)
	static const FLinearColor H4(0.620f, 0.690f, 0.749f);             // #9eb0bf — light steel (minor heading)
	static const FLinearColor H56(0.580f, 0.610f, 0.640f);            // #949ca3 — muted gray (smallest)
	static const FLinearColor H3Accent(0.380f, 0.710f, 0.659f, 0.5f); // H3 left accent bar (semi-transparent teal)

	// EN: Table colors — high contrast for accessibility
	// ES: Colores de tabla — alto contraste para accesibilidad
	static const FLinearColor TableHeaderBg(0.145f, 0.192f, 0.255f);  // #253141 — dark navy header
	static const FLinearColor TableHeaderText(0.900f, 0.930f, 0.970f); // #e6edf8 — bright white-blue text
	static const FLinearColor TableRowEven(0.137f, 0.137f, 0.145f);   // #232325 — subtle dark even
	static const FLinearColor TableRowOdd(0.169f, 0.173f, 0.184f);    // #2b2c2f — visible lighter odd
	static const FLinearColor TableBorder(0.200f, 0.216f, 0.243f);    // #33373e — table frame border

	// EN: Code block text — bright enough to read comfortably on dark bg
	// ES: Texto de code block — suficientemente brillante para leer comodamente sobre fondo oscuro
	static const FLinearColor CodeText(0.808f, 0.867f, 0.910f);       // #ceddea — soft bright for code readability
}

// ============================================================================
// Construction
// ============================================================================

void SDocSlateRenderer::Construct(const FArguments& InArgs)
{
	CachedConfig = InArgs._Config;
	InitStyleSet();

	ChildSlot
	[
		SAssignNew(ScrollBox, SScrollBox)
	];
}

void SDocSlateRenderer::InitStyleSet()
{
	static const FName StyleSetName(TEXT("PGXDocsRichText"));

	// EN: Unregister if already exists (hot-reload safety) / ES: Des-registrar si ya existe (seguridad hot-reload)
	if (FSlateStyleRegistry::FindSlateStyle(StyleSetName))
	{
		// EN: Reuse existing / ES: Reusar existente
		DocStyleSet = MakeShareable(new FSlateStyleSet(StyleSetName));
		return;
	}

	DocStyleSet = MakeShareable(new FSlateStyleSet(StyleSetName));

	const int32 FontSize = GetBaseFontSize();
	const FString FontName = TEXT("Roboto");
	const FString MonoFontName = TEXT("Courier New");

	// EN: Normal text style / ES: Estilo de texto normal
	FTextBlockStyle NormalStyle = FTextBlockStyle()
		.SetFont(FCoreStyle::GetDefaultFontStyle("Regular", static_cast<float>(FontSize)))
		.SetColorAndOpacity(FSlateColor(DocColors::Text));

	// EN: Bold / ES: Negrita
	FTextBlockStyle BoldStyle = FTextBlockStyle(NormalStyle)
		.SetFont(FCoreStyle::GetDefaultFontStyle("Bold", static_cast<float>(FontSize)));

	// EN: Italic / ES: Cursiva
	FTextBlockStyle ItalicStyle = FTextBlockStyle(NormalStyle)
		.SetFont(FCoreStyle::GetDefaultFontStyle("Italic", static_cast<float>(FontSize)));

	// EN: BoldItalic / ES: Negrita cursiva
	FTextBlockStyle BoldItalicStyle = FTextBlockStyle(NormalStyle)
		.SetFont(FCoreStyle::GetDefaultFontStyle("BoldItalic", static_cast<float>(FontSize)));

	// EN: Inline code / ES: Code inline
	FTextBlockStyle CodeStyle = FTextBlockStyle(NormalStyle)
		.SetFont(FCoreStyle::GetDefaultFontStyle("Mono", static_cast<float>(FontSize)))
		.SetColorAndOpacity(FSlateColor(DocColors::InlineCode));

	// EN: Strikethrough / ES: Tachado
	FTextBlockStyle StrikeStyle = FTextBlockStyle(NormalStyle)
		.SetStrikeBrush(*FCoreStyle::Get().GetBrush("DefaultTextUnderline"));

	// EN: Link style / ES: Estilo de link
	FTextBlockStyle LinkStyle = FTextBlockStyle(NormalStyle)
		.SetColorAndOpacity(FSlateColor(DocColors::Link));

	DocStyleSet->Set("RichText.Default", NormalStyle);
	DocStyleSet->Set("RichText.Bold", BoldStyle);
	DocStyleSet->Set("RichText.Italic", ItalicStyle);
	DocStyleSet->Set("RichText.BoldItalic", BoldItalicStyle);
	DocStyleSet->Set("RichText.Code", CodeStyle);
	DocStyleSet->Set("RichText.Strikethrough", StrikeStyle);
	DocStyleSet->Set("RichText.Link", LinkStyle);

	// EN: Code block text style — bright color for readability on dark bg
	// ES: Estilo de texto de code block — color brillante para legibilidad sobre fondo oscuro
	FTextBlockStyle CodeBlockStyle = FTextBlockStyle()
		.SetFont(FCoreStyle::GetDefaultFontStyle("Mono", static_cast<float>(FontSize)))
		.SetColorAndOpacity(FSlateColor(DocColors::CodeText));
	DocStyleSet->Set("CodeBlock.Text", CodeBlockStyle);

	// EN: Hyperlink style for clickable links / ES: Estilo de hyperlink para links clickeables
	FHyperlinkStyle HyperlinkStyle;
	HyperlinkStyle.SetUnderlineStyle(
		FButtonStyle()
		.SetNormal(FSlateNoResource())
		.SetHovered(FSlateNoResource())
		.SetPressed(FSlateNoResource())
	);
	HyperlinkStyle.SetTextStyle(LinkStyle);
	DocStyleSet->Set("RichText.Link", HyperlinkStyle);
}

// ============================================================================
// Public API
// ============================================================================

void SDocSlateRenderer::SetContent(const TArray<FDocElement>& Elements, const FString& /*DocTitle*/)
{
	Clear();
	CachedHeadings.Empty();
	BuildWidgets(Elements);
	ScrollToTop();
}

void SDocSlateRenderer::Clear()
{
	if (ScrollBox.IsValid())
	{
		ScrollBox->ClearChildren();
	}
	CachedHeadings.Empty();
}

void SDocSlateRenderer::ScrollToTop()
{
	if (ScrollBox.IsValid())
	{
		ScrollBox->SetScrollOffset(0.0f);
	}
}

float SDocSlateRenderer::GetScrollOffset() const
{
	if (ScrollBox.IsValid())
	{
		return ScrollBox->GetScrollOffset();
	}
	return 0.0f;
}

void SDocSlateRenderer::SetScrollOffset(float Offset)
{
	if (ScrollBox.IsValid())
	{
		ScrollBox->SetScrollOffset(Offset);
	}
}

TArray<TPair<FString, int32>> SDocSlateRenderer::GetTOC() const
{
	return CachedHeadings;
}

int32 SDocSlateRenderer::GetBaseFontSize() const
{
	if (CachedConfig)
	{
		return FMath::Clamp(CachedConfig->BaseFontSize, 10, 24);
	}
	return 14;
}

// ============================================================================
// Widget Building
// ============================================================================

void SDocSlateRenderer::BuildWidgets(const TArray<FDocElement>& Elements)
{
	if (!ScrollBox.IsValid())
	{
		return;
	}

	for (const FDocElement& Elem : Elements)
	{
		ScrollBox->AddSlot()
		.Padding(FMargin(40.0f, 5.0f, 40.0f, 5.0f))
		[
			BuildBlock(Elem)
		];
	}
}

TSharedRef<SWidget> SDocSlateRenderer::BuildBlock(const FDocElement& Element)
{
	switch (Element.Type)
	{
	case EDocElementType::Paragraph:     return BuildParagraph(Element);
	case EDocElementType::Heading:       return BuildHeading(Element);
	case EDocElementType::CodeBlock:     return BuildCodeBlock(Element);
	case EDocElementType::Blockquote:    return BuildBlockquote(Element);
	case EDocElementType::HorizontalRule: return BuildHorizontalRule();
	case EDocElementType::UnorderedList:
	case EDocElementType::OrderedList:   return BuildList(Element);
	case EDocElementType::Table:         return BuildTable(Element);
	case EDocElementType::ListItem:      return BuildListItem(Element, false, 0);
	case EDocElementType::TableRow:
	case EDocElementType::TableCell:
		// EN: These should be handled by their parent builders / ES: Estos deben ser manejados por sus builders padre
		return BuildParagraph(Element);
	default:
		return SNullWidget::NullWidget;
	}
}

// ============================================================================
// Paragraph
// ============================================================================

TSharedRef<SWidget> SDocSlateRenderer::BuildParagraph(const FDocElement& Element)
{
	if (Element.Runs.Num() == 0 && Element.Children.Num() == 0)
	{
		return SNullWidget::NullWidget;
	}

	// EN: If this paragraph has children (e.g. from blockquote or list), build them recursively
	// ES: Si este parrafo tiene hijos (e.g. de blockquote o lista), construirlos recursivamente
	if (Element.Runs.Num() == 0 && Element.Children.Num() > 0)
	{
		TSharedRef<SVerticalBox> VBox = SNew(SVerticalBox);
		for (const FDocElement& Child : Element.Children)
		{
			VBox->AddSlot()
			.AutoHeight()
			.Padding(0, 1)
			[
				BuildBlock(Child)
			];
		}
		return VBox;
	}

	return BuildInlineText(Element.Runs);
}

// ============================================================================
// Inline Text (SRichTextBlock)
// ============================================================================

FString SDocSlateRenderer::RunsToRichText(const TArray<FDocTextRun>& Runs) const
{
	FString Result;
	Result.Reserve(1024);

	for (const FDocTextRun& Run : Runs)
	{
		if (Run.Text.IsEmpty() && Run.ImageSrc.IsEmpty())
		{
			continue;
		}

		// EN: Escape XML special chars in text / ES: Escapear caracteres especiales XML en texto
		FString EscapedText = Run.Text;
		EscapedText.ReplaceInline(TEXT("&"), TEXT("&amp;"));
		EscapedText.ReplaceInline(TEXT("<"), TEXT("&lt;"));
		EscapedText.ReplaceInline(TEXT(">"), TEXT("&gt;"));

		// EN: Images — show as [Image] placeholder / ES: Imagenes — mostrar como placeholder [Image]
		if (!Run.ImageSrc.IsEmpty())
		{
			Result.Append(TEXT("<RichText.Code>[img: "));
			Result.Append(FPaths::GetCleanFilename(Run.ImageSrc));
			Result.Append(TEXT("]</>"));
			continue;
		}

		// EN: Links use hyperlink decorator / ES: Links usan decorador hyperlink
		if (!Run.LinkUrl.IsEmpty())
		{
			// EN: Use hyperlink run — <a> tag with id attribute for metadata
			// ES: Usar hyperlink run — tag <a> con atributo id para metadata
			FString EscapedUrl = Run.LinkUrl;
			EscapedUrl.ReplaceInline(TEXT("\""), TEXT("&quot;"));
			Result.Appendf(TEXT("<a id=\"href\" href=\"%s\" style=\"RichText.Link\">%s</>"), *EscapedUrl, *EscapedText);
			continue;
		}

		FString StyleTag;
		switch (Run.Style)
		{
		case EDocTextStyle::Bold:          StyleTag = TEXT("RichText.Bold"); break;
		case EDocTextStyle::Italic:        StyleTag = TEXT("RichText.Italic"); break;
		case EDocTextStyle::BoldItalic:    StyleTag = TEXT("RichText.BoldItalic"); break;
		case EDocTextStyle::Code:          StyleTag = TEXT("RichText.Code"); break;
		case EDocTextStyle::Strikethrough: StyleTag = TEXT("RichText.Strikethrough"); break;
		default:                           StyleTag = TEXT("RichText.Default"); break;
		}

		Result.Appendf(TEXT("<%s>%s</>"), *StyleTag, *EscapedText);
	}

	return Result;
}

TSharedRef<SWidget> SDocSlateRenderer::BuildInlineText(const TArray<FDocTextRun>& Runs) const
{
	if (Runs.Num() == 0)
	{
		return SNullWidget::NullWidget;
	}

	// EN: Check if any run has a link — if so, use SRichTextBlock with hyperlink decorator
	// ES: Verificar si algun run tiene link — si es asi, usar SRichTextBlock con decorador hyperlink
	bool bHasLinks = false;
	for (const FDocTextRun& Run : Runs)
	{
		if (!Run.LinkUrl.IsEmpty())
		{
			bHasLinks = true;
			break;
		}
	}

	// EN: Check if all text is the same simple style — use STextBlock for simplicity
	// ES: Verificar si todo el texto es el mismo estilo simple — usar STextBlock por simplicidad
	bool bAllSameStyle = !bHasLinks;
	if (bAllSameStyle)
	{
		const EDocTextStyle FirstStyle = Runs[0].Style;
		for (int32 i = 1; i < Runs.Num(); ++i)
		{
			if (Runs[i].Style != FirstStyle || !Runs[i].ImageSrc.IsEmpty())
			{
				bAllSameStyle = false;
				break;
			}
		}
	}

	if (bAllSameStyle && Runs.Num() > 0)
	{
		// EN: Build plain text / ES: Construir texto plano
		FString PlainText;
		for (const FDocTextRun& Run : Runs)
		{
			PlainText.Append(Run.Text);
		}

		FSlateFontInfo Font;
		FSlateColor Color(DocColors::Text);

		switch (Runs[0].Style)
		{
		case EDocTextStyle::Bold:
			Font = FCoreStyle::GetDefaultFontStyle("Bold", static_cast<float>(GetBaseFontSize()));
			break;
		case EDocTextStyle::Italic:
			Font = FCoreStyle::GetDefaultFontStyle("Italic", static_cast<float>(GetBaseFontSize()));
			break;
		case EDocTextStyle::BoldItalic:
			Font = FCoreStyle::GetDefaultFontStyle("BoldItalic", static_cast<float>(GetBaseFontSize()));
			break;
		case EDocTextStyle::Code:
			Font = FCoreStyle::GetDefaultFontStyle("Mono", static_cast<float>(GetBaseFontSize()));
			Color = FSlateColor(DocColors::InlineCode);
			break;
		default:
			Font = FCoreStyle::GetDefaultFontStyle("Regular", static_cast<float>(GetBaseFontSize()));
			break;
		}

		return SNew(STextBlock)
			.Text(FText::FromString(PlainText))
			.Font(Font)
			.ColorAndOpacity(Color)
			.AutoWrapText(true);
	}

	// EN: Mixed styles or links — use SRichTextBlock / ES: Estilos mixtos o links — usar SRichTextBlock
	const FString RichText = RunsToRichText(Runs);

	TSharedRef<SRichTextBlock> RichTextBlock = SNew(SRichTextBlock)
		.Text(FText::FromString(RichText))
		.AutoWrapText(true)
		.DecoratorStyleSet(DocStyleSet.Get());

	if (bHasLinks)
	{
		RichTextBlock->SetDecoratorStyleSet(DocStyleSet.Get());
	}

	return RichTextBlock;
}

// ============================================================================
// Heading
// ============================================================================

TSharedRef<SWidget> SDocSlateRenderer::BuildHeading(const FDocElement& Element)
{
	const int32 Level = FMath::Clamp(Element.HeadingLevel, 1, 6);
	const int32 BaseFontSize = GetBaseFontSize();

	// EN: Font size offsets — clear gap between each tier
	//     H1(+10)=24  H2(+7)=21  H3(+2)=16  H4(+1)=15  H5-6(+0)=14
	//     The H2→H3 gap (5pt) is the biggest jump — marks the title/subsection boundary
	// ES: Offsets de fuente — gap claro entre cada nivel
	static const int32 HeadingSizeOffsets[] = { 10, 7, 2, 1, 0, 0 }; // H1..H6
	const int32 FontSize = BaseFontSize + HeadingSizeOffsets[Level - 1];

	// EN: Build heading text from runs / ES: Construir texto del heading desde runs
	FString HeadingText;
	for (const FDocTextRun& Run : Element.Runs)
	{
		HeadingText.Append(Run.Text);
	}

	// EN: Cache heading for TOC / ES: Cachear heading para TOC
	const_cast<SDocSlateRenderer*>(this)->CachedHeadings.Add(TPair<FString, int32>(HeadingText, Level));

	// EN: Generous spacing — main titles need the most room
	// ES: Espaciado generoso — titulos principales necesitan mas espacio
	float TopPad, BottomPad;
	switch (Level)
	{
	case 1:  TopPad = 32.0f; BottomPad = 8.0f;  break;
	case 2:  TopPad = 28.0f; BottomPad = 10.0f; break;
	case 3:  TopPad = 20.0f; BottomPad = 6.0f;  break;
	case 4:  TopPad = 16.0f; BottomPad = 4.0f;  break;
	default: TopPad = 12.0f; BottomPad = 3.0f;  break;
	}

	// EN: Color per heading level — distinct families per tier
	// ES: Color por nivel de heading — familias distintas por nivel
	FLinearColor HeadingColor;
	switch (Level)
	{
	case 1:  HeadingColor = DocColors::H1; break;   // white-blue (page title)
	case 2:  HeadingColor = DocColors::H2; break;   // bright blue (section)
	case 3:  HeadingColor = DocColors::H3; break;   // teal (subsection — different family)
	case 4:  HeadingColor = DocColors::H4; break;   // steel gray
	default: HeadingColor = DocColors::H56; break;   // muted gray
	}

	// EN: H1/H2 centered (main titles), H3+ left-aligned (subsections)
	// ES: H1/H2 centrados (titulos principales), H3+ alineados a la izquierda (subsecciones)
	const ETextJustify::Type Justify = (Level <= 2) ? ETextJustify::Center : ETextJustify::Left;

	// EN: Font weight — H1/H2 bold (titles), H3 regular (subsection — lighter feel), H4+ regular
	// ES: Peso de fuente — H1/H2 bold (titulos), H3 regular (subseccion — mas ligero), H4+ regular
	const FString FontWeight = (Level <= 2) ? TEXT("Bold") : TEXT("Regular");

	// EN: Build the heading text widget / ES: Construir el widget de texto del heading
	TSharedRef<STextBlock> HeadingTextBlock = SNew(STextBlock)
		.Text(FText::FromString(HeadingText))
		.Font(FCoreStyle::GetDefaultFontStyle(*FontWeight, static_cast<float>(FontSize)))
		.ColorAndOpacity(FSlateColor(HeadingColor))
		.Justification(Justify)
		.AutoWrapText(true);

	// EN: For H3/H4 — add a colored left accent bar to visually distinguish from plain text
	// ES: Para H3/H4 — añadir barra de acento izquierda coloreada para distinguir visualmente del texto plano
	TSharedRef<SWidget> HeadingContent = (Level == 3 || Level == 4)
		? StaticCastSharedRef<SWidget>(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
				.BorderBackgroundColor(HeadingColor * FLinearColor(1, 1, 1, 0.5f))
				.Padding(FMargin(1.5f, 0.0f))
				[
					SNullWidget::NullWidget
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(8.0f, 0, 0, 0)
			[
				HeadingTextBlock
			]
		)
		: StaticCastSharedRef<SWidget>(HeadingTextBlock);

	TSharedRef<SVerticalBox> HeadingWidget = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, TopPad, 0, BottomPad)
		[
			HeadingContent
		];

	// EN: Add bottom separator for H1 and H2 — visually anchors the title
	// ES: Añadir separador inferior para H1 y H2 — ancla visualmente el titulo
	if (Level <= 2)
	{
		// EN: H1 separator thicker and more visible / ES: Separador H1 mas grueso y visible
		const float SepThickness = (Level == 1) ? 2.0f : 1.0f;
		const FLinearColor SepColor = (Level == 1)
			? FLinearColor(DocColors::H2.R, DocColors::H2.G, DocColors::H2.B, 0.4f)
			: FLinearColor(DocColors::Border.R, DocColors::Border.G, DocColors::Border.B, 0.6f);

		HeadingWidget->AddSlot()
		.AutoHeight()
		.Padding(0, 0, 0, 10.0f)
		[
			SNew(SSeparator)
			.SeparatorImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
			.ColorAndOpacity(SepColor)
			.Thickness(SepThickness)
		];
	}

	return HeadingWidget;
}

// ============================================================================
// Code Block
// ============================================================================

TSharedRef<SWidget> SDocSlateRenderer::BuildCodeBlock(const FDocElement& Element)
{
	// EN: Get persistent code block style from DocStyleSet — pointer remains valid (owned by DocStyleSet)
	// ES: Obtener estilo persistente de code block desde DocStyleSet — puntero permanece valido (propiedad de DocStyleSet)
	const FTextBlockStyle& CodeBlockStyle = DocStyleSet->GetWidgetStyle<FTextBlockStyle>("CodeBlock.Text");

	// EN: Extra vertical breathing room around code blocks / ES: Respiro vertical extra alrededor de bloques de codigo
	return SNew(SBox)
		.Padding(FMargin(0, 8.0f, 0, 8.0f))
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
			.BorderBackgroundColor(DocColors::CodeBg)
			.Padding(FMargin(16.0f, 12.0f))
			[
				SNew(SMultiLineEditableText)
				.IsReadOnly(true)
				.AutoWrapText(false)
				.Text(FText::FromString(Element.CodeText))
				.TextStyle(&CodeBlockStyle)
				.Marshaller(nullptr)
			]
		];
}

// ============================================================================
// Blockquote
// ============================================================================

TSharedRef<SWidget> SDocSlateRenderer::BuildBlockquote(const FDocElement& Element)
{
	TSharedRef<SVerticalBox> QuoteContent = SNew(SVerticalBox);

	// EN: Build children (paragraphs, nested blockquotes, etc.)
	// ES: Construir hijos (parrafos, blockquotes anidados, etc.)
	for (const FDocElement& Child : Element.Children)
	{
		QuoteContent->AddSlot()
		.AutoHeight()
		.Padding(0, 2)
		[
			BuildBlock(Child)
		];
	}

	// EN: Blockquote with left border accent / ES: Blockquote con acento de borde izquierdo
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
			.BorderBackgroundColor(DocColors::BlockquoteBorder)
			.Padding(FMargin(2.0f, 0.0f))
			[
				SNullWidget::NullWidget
			]
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
			.BorderBackgroundColor(DocColors::CodeBg)
			.Padding(FMargin(16.0f, 10.0f))
			[
				QuoteContent
			]
		];
}

// ============================================================================
// Horizontal Rule
// ============================================================================

TSharedRef<SWidget> SDocSlateRenderer::BuildHorizontalRule()
{
	return SNew(SBox)
		.Padding(FMargin(0, 16.0f))
		[
			SNew(SSeparator)
			.SeparatorImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
			.ColorAndOpacity(DocColors::Border)
			.Thickness(1.0f)
		];
}

// ============================================================================
// Lists
// ============================================================================

TSharedRef<SWidget> SDocSlateRenderer::BuildList(const FDocElement& Element)
{
	const bool bOrdered = (Element.Type == EDocElementType::OrderedList);
	int32 Index = Element.ListStart;

	TSharedRef<SVerticalBox> ListBox = SNew(SVerticalBox);

	for (const FDocElement& Child : Element.Children)
	{
		if (Child.Type == EDocElementType::ListItem)
		{
			ListBox->AddSlot()
			.AutoHeight()
			.Padding(FMargin(20.0f, 3.0f, 0.0f, 3.0f))
			[
				BuildListItem(Child, bOrdered, Index)
			];
			++Index;
		}
	}

	return ListBox;
}

TSharedRef<SWidget> SDocSlateRenderer::BuildListItem(const FDocElement& Element, bool bOrdered, int32 Index)
{
	// EN: Build bullet/number marker / ES: Construir marcador bullet/numero
	FString Marker;
	if (Element.bIsTaskItem)
	{
		Marker = Element.bIsTaskChecked ? TEXT("[x] ") : TEXT("[ ] ");
	}
	else if (bOrdered)
	{
		Marker = FString::Printf(TEXT("%d. "), Index);
	}
	else
	{
		Marker = TEXT("\u2022 "); // bullet
	}

	TSharedRef<SHorizontalBox> ItemBox = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0, 2, 4, 0)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Marker))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", static_cast<float>(GetBaseFontSize())))
			.ColorAndOpacity(FSlateColor(DocColors::Text))
		];

	// EN: Item content: might have inline runs and/or child blocks (nested lists, paragraphs)
	// ES: Contenido del item: puede tener runs inline y/o bloques hijos (listas anidadas, parrafos)
	if (Element.Runs.Num() > 0 && Element.Children.Num() == 0)
	{
		// EN: Simple inline text item / ES: Item de texto inline simple
		ItemBox->AddSlot()
		.FillWidth(1.0f)
		[
			BuildInlineText(Element.Runs)
		];
	}
	else
	{
		// EN: Complex item with children (may include paragraphs, nested lists)
		// ES: Item complejo con hijos (puede incluir parrafos, listas anidadas)
		TSharedRef<SVerticalBox> ContentBox = SNew(SVerticalBox);

		if (Element.Runs.Num() > 0)
		{
			ContentBox->AddSlot()
			.AutoHeight()
			[
				BuildInlineText(Element.Runs)
			];
		}

		for (const FDocElement& Child : Element.Children)
		{
			ContentBox->AddSlot()
			.AutoHeight()
			.Padding(0, 1)
			[
				BuildBlock(Child)
			];
		}

		ItemBox->AddSlot()
		.FillWidth(1.0f)
		[
			ContentBox
		];
	}

	return ItemBox;
}

// ============================================================================
// Table
// ============================================================================

TSharedRef<SWidget> SDocSlateRenderer::BuildTable(const FDocElement& Element)
{
	// EN: Collect all rows (from TableRow children) / ES: Recopilar todas las filas (de hijos TableRow)
	TArray<const FDocElement*> Rows;
	for (const FDocElement& Child : Element.Children)
	{
		if (Child.Type == EDocElementType::TableRow)
		{
			Rows.Add(&Child);
		}
	}

	if (Rows.Num() == 0)
	{
		return SNullWidget::NullWidget;
	}

	// EN: Determine column count from first row / ES: Determinar cantidad de columnas de la primera fila
	int32 ColCount = 0;
	if (Rows.Num() > 0)
	{
		ColCount = Rows[0]->Children.Num();
	}

	if (ColCount == 0)
	{
		return SNullWidget::NullWidget;
	}

	// EN: Build table using SGridPanel / ES: Construir tabla usando SGridPanel
	TSharedRef<SGridPanel> Grid = SNew(SGridPanel);

	for (int32 RowIdx = 0; RowIdx < Rows.Num(); ++RowIdx)
	{
		const FDocElement& Row = *Rows[RowIdx];

		for (int32 ColIdx = 0; ColIdx < Row.Children.Num() && ColIdx < ColCount; ++ColIdx)
		{
			const FDocElement& Cell = Row.Children[ColIdx];

			// EN: High-contrast cell backgrounds for accessibility
			// ES: Fondos de celda con alto contraste para accesibilidad
			FLinearColor CellBg;
			FLinearColor CellTextColor;
			FSlateFontInfo CellFont;

			if (Cell.bIsTableHeader)
			{
				CellBg = DocColors::TableHeaderBg;
				CellTextColor = DocColors::TableHeaderText;
				CellFont = FCoreStyle::GetDefaultFontStyle("Bold", static_cast<float>(GetBaseFontSize()));
			}
			else
			{
				CellBg = (RowIdx % 2 == 0) ? DocColors::TableRowEven : DocColors::TableRowOdd;
				CellTextColor = DocColors::Text;
				CellFont = FCoreStyle::GetDefaultFontStyle("Regular", static_cast<float>(GetBaseFontSize()));
			}

			// EN: Cell alignment / ES: Alineacion de celda
			ETextJustify::Type Justify = ETextJustify::Left;
			switch (Cell.CellAlign)
			{
			case EDocAlign::Center: Justify = ETextJustify::Center; break;
			case EDocAlign::Right:  Justify = ETextJustify::Right; break;
			default: break;
			}

			// EN: Build cell text / ES: Construir texto de celda
			FString CellText;
			for (const FDocTextRun& Run : Cell.Runs)
			{
				CellText.Append(Run.Text);
			}

			Grid->AddSlot(ColIdx, RowIdx)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
				.BorderBackgroundColor(CellBg)
				.Padding(FMargin(12.0f, 6.0f))
				[
					SNew(STextBlock)
					.Text(FText::FromString(CellText))
					.Font(CellFont)
					.ColorAndOpacity(FSlateColor(CellTextColor))
					.Justification(Justify)
					.AutoWrapText(true)
				]
			];
		}
	}

	// EN: Wrap grid in border with vertical breathing room
	// ES: Envolver grid en borde con respiro vertical
	return SNew(SBox)
		.Padding(FMargin(0, 8.0f, 0, 8.0f))
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
			.BorderBackgroundColor(DocColors::TableBorder)
			.Padding(1.0f)
			[
				Grid
			]
		];
}

// ============================================================================
// Hyperlink handling
// ============================================================================

void SDocSlateRenderer::HandleHyperlinkClick(const TMap<FString, FString>& Metadata) const
{
	const FString* UrlPtr = Metadata.Find(TEXT("href"));
	if (UrlPtr && OnLinkClicked.IsBound())
	{
		OnLinkClicked.Execute(*UrlPtr);
	}
}

#undef LOCTEXT_NAMESPACE
