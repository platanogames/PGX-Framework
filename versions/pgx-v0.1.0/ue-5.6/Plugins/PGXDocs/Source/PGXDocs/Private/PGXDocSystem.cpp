// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXDocSystem.h"
#include "Logging/PGXLogMacros.h"
#include "PGXDocsModule.h"
#include "PGXDocsConfig.h"
#include "PGXDocsSettings.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <windows.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

// EN: md4c C library (extern "C" for C++ linkage) / ES: Libreria C md4c (extern "C" para linkaje C++)
extern "C"
{
#include "md4c.h"
#include "md4c-html.h"
}

// EN: Callback for md4c HTML output / ES: Callback para output HTML de md4c
namespace
{
	struct FMd4cContext
	{
		FString* OutputString;
	};

	void Md4cCallback(const MD_CHAR* Text, MD_SIZE Size, void* UserData)
	{
		FMd4cContext* Ctx = static_cast<FMd4cContext*>(UserData);
		if (Ctx && Ctx->OutputString && Size > 0)
		{
			// EN: Direct UTF8→TCHAR conversion with explicit size (no intermediate TArray, minimal stack usage)
			// ES: Conversion directa UTF8→TCHAR con tamano explicito (sin TArray intermedio, uso minimo de stack)
			const FUTF8ToTCHAR Converter(Text, static_cast<int32>(Size));
			Ctx->OutputString->Append(Converter.Get(), Converter.Length());
		}
	}

#if PLATFORM_WINDOWS
	// EN: Separate function for SEH guard — MSVC forbids __try in functions with C++ destructors
	// ES: Funcion separada para guard SEH — MSVC prohibe __try en funciones con destructores C++
	int MdHtmlWithStackGuard(const char* Data, MD_SIZE Size, void (*Callback)(const MD_CHAR*, MD_SIZE, void*), void* UserData, unsigned ParseFlags, unsigned RenderFlags)
	{
		int Result = -1;
		__try
		{
			Result = md_html(Data, Size, Callback, UserData, ParseFlags, RenderFlags);
		}
		__except (GetExceptionCode() == EXCEPTION_STACK_OVERFLOW ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
		{
			_resetstkoflw();
			Result = -2; // EN: Sentinel for stack overflow / ES: Sentinel para stack overflow
		}
		return Result;
	}
#endif
}

FDocSystem& FDocSystem::Get()
{
	static FDocSystem Instance;
	return Instance;
}

void FDocSystem::Initialize(const UPGXDocsConfig* Config)
{
	if (bIsInitialized)
	{
		return;
	}

	CachedConfig = Config;

	PGX_LOG_INFO(LogPGXDocs, TEXT("PGXDocs: System initializing..."));

	// EN: Initialize registry — scans all doc roots / ES: Inicializar registry — escanea todas las raices de docs
	Registry.Initialize(Config);

	// EN: Cache the index path now while UObjects are alive — Shutdown() runs after UObject teardown
	// ES: Cachear el path del indice ahora mientras los UObjects estan vivos — Shutdown() corre despues del teardown de UObjects
	const UPGXDocsSettings* Settings = UPGXDocsSettings::Get();
	CachedIndexCachePath = FPaths::Combine(FPaths::ProjectDir(), Settings->IndexCachePath);

	if (!SearchIndex.LoadCache(CachedIndexCachePath))
	{
		// EN: No cache — build from scratch / ES: Sin cache — construir desde cero
		RebuildSearchIndex();
	}

	bIsInitialized = true;
	PGX_LOG_INFO(LogPGXDocs, TEXT("PGXDocs: System initialized — %d docs, %d indexed"), Registry.GetDocCount(), SearchIndex.GetIndexedCount());
}

void FDocSystem::Shutdown()
{
	if (!bIsInitialized)
	{
		return;
	}

	// EN: Save search index cache — using path cached at Initialize() (UObjects are already destroyed at this point)
	// ES: Guardar cache de indice de busqueda — usando path cacheado en Initialize() (UObjects ya estan destruidos en este punto)
	if (!CachedIndexCachePath.IsEmpty())
	{
		IFileManager::Get().MakeDirectory(*FPaths::GetPath(CachedIndexCachePath), true);
		SearchIndex.SaveCache(CachedIndexCachePath);
	}

	DocumentCache.Empty();
	SearchIndex.Clear();
	bIsInitialized = false;

	PGX_LOG_INFO(LogPGXDocs, TEXT("PGXDocs: System shut down"));
}

void FDocSystem::RebuildRegistry()
{
	Registry.Rebuild();
	RebuildSearchIndex();
	OnDocsChanged.Broadcast();
}

TSharedPtr<FDocDocument> FDocSystem::LoadDocument(const FString& DocId)
{
	// EN: Check cache / ES: Verificar cache
	TSharedPtr<FDocDocument>* Cached = DocumentCache.Find(DocId);
	if (Cached && Cached->IsValid())
	{
		return *Cached;
	}

	// EN: Resolve path / ES: Resolver ruta
	const FString AbsPath = Registry.ResolveDocId(DocId);
	if (AbsPath.IsEmpty())
	{
		PGX_LOG_WARNING(LogPGXDocs, TEXT("PGXDocs: Cannot resolve DocId: %s"), *DocId);
		return nullptr;
	}

	// EN: Read file / ES: Leer archivo
	FString RawContent;
	if (!FFileHelper::LoadFileToString(RawContent, *AbsPath))
	{
		PGX_LOG_WARNING(LogPGXDocs, TEXT("PGXDocs: Cannot read file: %s"), *AbsPath);
		return nullptr;
	}

	TSharedPtr<FDocDocument> Doc = MakeShared<FDocDocument>();
	Doc->DocId = DocId;
	Doc->AbsolutePath = AbsPath;
	Doc->RawMarkdown = RawContent;

	// EN: Parse module and chapter from DocId / ES: Parsear modulo y capitulo del DocId
	FString ModulePart, ChapterPart;
	if (DocId.Split(TEXT("/"), &ModulePart, &ChapterPart))
	{
		Doc->ModuleId = ModulePart;
		Doc->ChapterId = ChapterPart;
	}

	// EN: Parse frontmatter / ES: Parsear frontmatter
	ParseFrontMatter(*Doc);

	// EN: Extract title / ES: Extraer titulo
	Doc->Title = ExtractTitle(Doc->RawMarkdown);
	if (Doc->Title.IsEmpty())
	{
		Doc->Title = FPaths::GetBaseFilename(AbsPath).Replace(TEXT("_"), TEXT(" "));
	}

	// EN: Get file modification time / ES: Obtener tiempo de modificacion del archivo
	Doc->ModifiedTimeUtc = IFileManager::Get().GetTimeStamp(*AbsPath).ToUnixTimestamp();

	// EN: Manage cache size / ES: Gestionar tamano de cache
	if (DocumentCache.Num() >= MaxCacheSize)
	{
		// EN: Simple eviction — remove first entry / ES: Eviccion simple — remover primera entrada
		auto It = DocumentCache.CreateIterator();
		if (It)
		{
			DocumentCache.Remove(It.Key());
		}
	}

	DocumentCache.Add(DocId, Doc);
	return Doc;
}

TArray<FDocSearchResult> FDocSystem::Search(const FString& Query, const FDocSearchFilters& Filters) const
{
	int32 MaxResults = 25;
	if (CachedConfig)
	{
		MaxResults = CachedConfig->MaxSearchResults;
	}
	return SearchIndex.Search(Query, Filters, MaxResults);
}

void FDocSystem::RebuildSearchIndex()
{
	SearchIndex.Clear();

	TArray<FString> AllDocIds = Registry.GetAllDocIds();
	for (const FString& DocId : AllDocIds)
	{
		TSharedPtr<FDocDocument> Doc = LoadDocument(DocId);
		if (Doc.IsValid())
		{
			SearchIndex.IndexDocument(*Doc);
		}
	}

	PGX_LOG_INFO(LogPGXDocs, TEXT("PGXDocs: Search index rebuilt — %d documents indexed"), SearchIndex.GetIndexedCount());
}

FDocValidationReport FDocSystem::ValidateAll() const
{
	return Validator.ValidateAll(Registry);
}

void FDocSystem::OnFilesChanged(const TArray<FString>& ChangedPaths)
{
	bool bNeedsRebuild = false;

	for (const FString& Path : ChangedPaths)
	{
		if (!Path.EndsWith(TEXT(".md")))
		{
			continue;
		}

		const FString NormPath = FPaths::ConvertRelativePathToFull(Path);
		const FString DocId = Registry.GetDocIdForPath(NormPath);

		if (DocId.IsEmpty())
		{
			// EN: New file — need full rebuild / ES: Archivo nuevo — necesita rebuild completo
			bNeedsRebuild = true;
			continue;
		}

		// EN: Invalidate cache / ES: Invalidar cache
		DocumentCache.Remove(DocId);

		// EN: Reload and re-index / ES: Recargar y re-indexar
		TSharedPtr<FDocDocument> Doc = LoadDocument(DocId);
		if (Doc.IsValid())
		{
			SearchIndex.IndexDocument(*Doc);
		}
	}

	if (bNeedsRebuild)
	{
		RebuildRegistry();
	}
	else
	{
		OnDocsChanged.Broadcast();
	}
}

void FDocSystem::ParseFrontMatter(FDocDocument& Document) const
{
	const FString& Content = Document.RawMarkdown;

	// EN: Check for YAML frontmatter (starts with ---) / ES: Verificar frontmatter YAML (empieza con ---)
	if (!Content.StartsWith(TEXT("---")))
	{
		return;
	}

	// EN: Find closing --- / ES: Encontrar --- de cierre
	int32 EndPos = Content.Find(TEXT("---"), ESearchCase::IgnoreCase, ESearchDir::FromStart, 3);
	if (EndPos == INDEX_NONE)
	{
		return;
	}

	const FString FrontMatterBlock = Content.Mid(3, EndPos - 3).TrimStartAndEnd();

	// EN: Simple YAML parsing (key: value) / ES: Parseo YAML simple (key: value)
	TArray<FString> Lines;
	FrontMatterBlock.ParseIntoArrayLines(Lines);

	for (const FString& Line : Lines)
	{
		FString Key, Value;
		if (Line.Split(TEXT(":"), &Key, &Value))
		{
			Key = Key.TrimStartAndEnd();
			Value = Value.TrimStartAndEnd();

			// EN: Remove surrounding quotes / ES: Remover comillas envolventes
			if (Value.StartsWith(TEXT("\"")) && Value.EndsWith(TEXT("\"")))
			{
				Value = Value.Mid(1, Value.Len() - 2);
			}

			Document.FrontMatter.Add(Key, Value);

			// EN: Extract tags from frontmatter / ES: Extraer tags del frontmatter
			if (Key.Equals(TEXT("tags"), ESearchCase::IgnoreCase))
			{
				// EN: Handle comma-separated or YAML array / ES: Manejar separados por coma o array YAML
				Value.ReplaceInline(TEXT("["), TEXT(""));
				Value.ReplaceInline(TEXT("]"), TEXT(""));
				TArray<FString> TagParts;
				Value.ParseIntoArray(TagParts, TEXT(","));
				for (FString& Tag : TagParts)
				{
					Tag = Tag.TrimStartAndEnd();
					if (!Tag.IsEmpty())
					{
						Document.Tags.Add(Tag);
					}
				}
			}
		}
	}

	// EN: Strip frontmatter from raw content for rendering
	// ES: Quitar frontmatter del contenido crudo para renderizado
	Document.RawMarkdown = Content.Mid(EndPos + 3).TrimStart();
}

FString FDocSystem::ExtractTitle(const FString& Markdown) const
{
	TArray<FString> Lines;
	Markdown.ParseIntoArrayLines(Lines);

	for (const FString& Line : Lines)
	{
		FString Trimmed = Line.TrimStart();
		if (Trimmed.StartsWith(TEXT("# ")) && !Trimmed.StartsWith(TEXT("## ")))
		{
			return Trimmed.Mid(2).TrimStartAndEnd();
		}
	}

	return FString();
}

// ============================================================================
// EN: ParseMarkdownToTree — md_parse() callback API → FDocElement tree
// ES: ParseMarkdownToTree — API de callbacks md_parse() → arbol FDocElement
// ============================================================================

namespace
{
	// EN: Helper to extract FString from MD_ATTRIBUTE (may contain entities)
	// ES: Helper para extraer FString de MD_ATTRIBUTE (puede contener entidades)
	FString Md4cAttrToString(const MD_ATTRIBUTE& Attr)
	{
		if (!Attr.text || Attr.size == 0)
		{
			return FString();
		}
		const FUTF8ToTCHAR Converter(Attr.text, static_cast<int32>(Attr.size));
		return FString(Converter.Length(), Converter.Get());
	}

	// EN: Context for stack-based md_parse tree building
	// ES: Contexto para construccion de arbol md_parse basada en stack
	struct FMd4cTreeContext
	{
		TArray<FDocElement> RootElements;           // EN: Final result / ES: Resultado final
		TArray<FDocElement*> BlockStack;              // EN: Stack of nested blocks / ES: Stack de bloques anidados
		TArray<EDocTextStyle> SpanStyleStack;         // EN: Stack of inline styles / ES: Stack de estilos inline
		FString CurrentLinkUrl;                       // EN: URL of active link / ES: URL del link activo
		FString CurrentImageSrc;                      // EN: Src of active image / ES: Src de la imagen activa
		bool bInCodeSpan = false;                     // EN: Inside inline code / ES: Dentro de code inline
		bool bInImage = false;                        // EN: Inside image span / ES: Dentro de span imagen

		// EN: Get the current effective text style by combining the style stack
		// ES: Obtener el estilo de texto efectivo actual combinando el stack de estilos
		EDocTextStyle GetCurrentStyle() const
		{
			if (bInCodeSpan)
			{
				return EDocTextStyle::Code;
			}

			bool bBold = false;
			bool bItalic = false;
			bool bStrike = false;
			bool bUnderline = false;

			for (EDocTextStyle S : SpanStyleStack)
			{
				switch (S)
				{
				case EDocTextStyle::Bold: bBold = true; break;
				case EDocTextStyle::Italic: bItalic = true; break;
				case EDocTextStyle::Strikethrough: bStrike = true; break;
				case EDocTextStyle::Underline: bUnderline = true; break;
				default: break;
				}
			}

			if (bBold && bItalic) return EDocTextStyle::BoldItalic;
			if (bBold) return EDocTextStyle::Bold;
			if (bItalic) return EDocTextStyle::Italic;
			if (bStrike) return EDocTextStyle::Strikethrough;
			if (bUnderline) return EDocTextStyle::Underline;
			return EDocTextStyle::Normal;
		}

		// EN: Get the element currently at the top of the block stack
		// ES: Obtener el elemento actualmente en la cima del stack de bloques
		FDocElement* CurrentBlock()
		{
			return BlockStack.Num() > 0 ? BlockStack.Last() : nullptr;
		}

		// EN: Push a new element onto the appropriate parent
		// ES: Pushear un nuevo elemento al padre apropiado
		FDocElement* PushElement(EDocElementType Type)
		{
			FDocElement NewElem;
			NewElem.Type = Type;

			if (BlockStack.Num() > 0)
			{
				FDocElement* Parent = BlockStack.Last();
				Parent->Children.Add(MoveTemp(NewElem));
				FDocElement* Ptr = &Parent->Children.Last();
				BlockStack.Push(Ptr);
				return Ptr;
			}

			RootElements.Add(MoveTemp(NewElem));
			FDocElement* Ptr2 = &RootElements.Last();
			BlockStack.Push(Ptr2);
			return Ptr2;
		}

		void PopBlock()
		{
			if (BlockStack.Num() > 0)
			{
				BlockStack.Pop();
			}
		}
	};

	// EN: md_parse enter_block callback / ES: Callback enter_block de md_parse
	int TreeEnterBlock(MD_BLOCKTYPE Type, void* Detail, void* UserData)
	{
		FMd4cTreeContext* Ctx = static_cast<FMd4cTreeContext*>(UserData);

		switch (Type)
		{
		case MD_BLOCK_DOC:
			// EN: Document root — no element needed / ES: Raiz del documento — no necesita elemento
			break;

		case MD_BLOCK_P:
			Ctx->PushElement(EDocElementType::Paragraph);
			break;

		case MD_BLOCK_H:
		{
			FDocElement* Elem = Ctx->PushElement(EDocElementType::Heading);
			if (Detail)
			{
				const MD_BLOCK_H_DETAIL* HD = static_cast<const MD_BLOCK_H_DETAIL*>(Detail);
				Elem->HeadingLevel = static_cast<int32>(HD->level);
			}
			break;
		}

		case MD_BLOCK_CODE:
		{
			FDocElement* Elem = Ctx->PushElement(EDocElementType::CodeBlock);
			if (Detail)
			{
				const MD_BLOCK_CODE_DETAIL* CD = static_cast<const MD_BLOCK_CODE_DETAIL*>(Detail);
				Elem->CodeLanguage = Md4cAttrToString(CD->lang);
			}
			break;
		}

		case MD_BLOCK_QUOTE:
			Ctx->PushElement(EDocElementType::Blockquote);
			break;

		case MD_BLOCK_HR:
			Ctx->PushElement(EDocElementType::HorizontalRule);
			break;

		case MD_BLOCK_UL:
			Ctx->PushElement(EDocElementType::UnorderedList);
			break;

		case MD_BLOCK_OL:
		{
			FDocElement* Elem = Ctx->PushElement(EDocElementType::OrderedList);
			if (Detail)
			{
				const MD_BLOCK_OL_DETAIL* OD = static_cast<const MD_BLOCK_OL_DETAIL*>(Detail);
				Elem->ListStart = static_cast<int32>(OD->start);
			}
			break;
		}

		case MD_BLOCK_LI:
		{
			FDocElement* Elem = Ctx->PushElement(EDocElementType::ListItem);
			if (Detail)
			{
				const MD_BLOCK_LI_DETAIL* LD = static_cast<const MD_BLOCK_LI_DETAIL*>(Detail);
				Elem->bIsTaskItem = (LD->is_task != 0);
				Elem->bIsTaskChecked = (LD->is_task != 0 && (LD->task_mark == 'x' || LD->task_mark == 'X'));
			}
			break;
		}

		case MD_BLOCK_TABLE:
			Ctx->PushElement(EDocElementType::Table);
			break;

		case MD_BLOCK_THEAD:
		case MD_BLOCK_TBODY:
			// EN: No dedicated element — rows go directly into Table / ES: Sin elemento dedicado — filas van directamente a Table
			break;

		case MD_BLOCK_TR:
			Ctx->PushElement(EDocElementType::TableRow);
			break;

		case MD_BLOCK_TH:
		{
			FDocElement* Elem = Ctx->PushElement(EDocElementType::TableCell);
			Elem->bIsTableHeader = true;
			if (Detail)
			{
				const MD_BLOCK_TD_DETAIL* TD = static_cast<const MD_BLOCK_TD_DETAIL*>(Detail);
				switch (TD->align)
				{
				case MD_ALIGN_CENTER: Elem->CellAlign = EDocAlign::Center; break;
				case MD_ALIGN_RIGHT: Elem->CellAlign = EDocAlign::Right; break;
				default: Elem->CellAlign = EDocAlign::Left; break;
				}
			}
			break;
		}

		case MD_BLOCK_TD:
		{
			FDocElement* Elem = Ctx->PushElement(EDocElementType::TableCell);
			Elem->bIsTableHeader = false;
			if (Detail)
			{
				const MD_BLOCK_TD_DETAIL* TD = static_cast<const MD_BLOCK_TD_DETAIL*>(Detail);
				switch (TD->align)
				{
				case MD_ALIGN_CENTER: Elem->CellAlign = EDocAlign::Center; break;
				case MD_ALIGN_RIGHT: Elem->CellAlign = EDocAlign::Right; break;
				default: Elem->CellAlign = EDocAlign::Left; break;
				}
			}
			break;
		}

		case MD_BLOCK_HTML:
			// EN: Raw HTML block — treat as paragraph with raw text / ES: Bloque HTML crudo — tratar como párrafo con texto crudo
			Ctx->PushElement(EDocElementType::Paragraph);
			break;

		default:
			break;
		}

		return 0;
	}

	// EN: md_parse leave_block callback / ES: Callback leave_block de md_parse
	int TreeLeaveBlock(MD_BLOCKTYPE Type, void* /*Detail*/, void* UserData)
	{
		FMd4cTreeContext* Ctx = static_cast<FMd4cTreeContext*>(UserData);

		switch (Type)
		{
		case MD_BLOCK_DOC:
		case MD_BLOCK_THEAD:
		case MD_BLOCK_TBODY:
			// EN: No matching push / ES: Sin push correspondiente
			break;

		default:
			Ctx->PopBlock();
			break;
		}

		return 0;
	}

	// EN: md_parse enter_span callback / ES: Callback enter_span de md_parse
	int TreeEnterSpan(MD_SPANTYPE Type, void* Detail, void* UserData)
	{
		FMd4cTreeContext* Ctx = static_cast<FMd4cTreeContext*>(UserData);

		switch (Type)
		{
		case MD_SPAN_STRONG:
			Ctx->SpanStyleStack.Push(EDocTextStyle::Bold);
			break;

		case MD_SPAN_EM:
			Ctx->SpanStyleStack.Push(EDocTextStyle::Italic);
			break;

		case MD_SPAN_CODE:
			Ctx->bInCodeSpan = true;
			break;

		case MD_SPAN_DEL:
			Ctx->SpanStyleStack.Push(EDocTextStyle::Strikethrough);
			break;

		case MD_SPAN_U:
			Ctx->SpanStyleStack.Push(EDocTextStyle::Underline);
			break;

		case MD_SPAN_A:
			if (Detail)
			{
				const MD_SPAN_A_DETAIL* AD = static_cast<const MD_SPAN_A_DETAIL*>(Detail);
				Ctx->CurrentLinkUrl = Md4cAttrToString(AD->href);
			}
			break;

		case MD_SPAN_IMG:
			if (Detail)
			{
				const MD_SPAN_IMG_DETAIL* ID = static_cast<const MD_SPAN_IMG_DETAIL*>(Detail);
				Ctx->CurrentImageSrc = Md4cAttrToString(ID->src);
				Ctx->bInImage = true;
			}
			break;

		default:
			break;
		}

		return 0;
	}

	// EN: md_parse leave_span callback / ES: Callback leave_span de md_parse
	int TreeLeaveSpan(MD_SPANTYPE Type, void* /*Detail*/, void* UserData)
	{
		FMd4cTreeContext* Ctx = static_cast<FMd4cTreeContext*>(UserData);

		switch (Type)
		{
		case MD_SPAN_STRONG:
		case MD_SPAN_EM:
		case MD_SPAN_DEL:
		case MD_SPAN_U:
			if (Ctx->SpanStyleStack.Num() > 0)
			{
				Ctx->SpanStyleStack.Pop();
			}
			break;

		case MD_SPAN_CODE:
			Ctx->bInCodeSpan = false;
			break;

		case MD_SPAN_A:
			Ctx->CurrentLinkUrl.Empty();
			break;

		case MD_SPAN_IMG:
		{
			// EN: Emit an image run in the current block / ES: Emitir un run de imagen en el bloque actual
			FDocElement* Block = Ctx->CurrentBlock();
			if (Block && !Ctx->CurrentImageSrc.IsEmpty())
			{
				FDocTextRun ImgRun;
				ImgRun.Style = EDocTextStyle::Normal;
				ImgRun.ImageSrc = Ctx->CurrentImageSrc;
				Block->Runs.Add(MoveTemp(ImgRun));
			}
			Ctx->CurrentImageSrc.Empty();
			Ctx->bInImage = false;
			break;
		}

		default:
			break;
		}

		return 0;
	}

	// EN: md_parse text callback / ES: Callback text de md_parse
	int TreeText(MD_TEXTTYPE Type, const MD_CHAR* Text, MD_SIZE Size, void* UserData)
	{
		FMd4cTreeContext* Ctx = static_cast<FMd4cTreeContext*>(UserData);
		FDocElement* Block = Ctx->CurrentBlock();
		if (!Block)
		{
			return 0;
		}

		// EN: For image spans, text is the alt text — store but don't create a visible run
		// ES: Para spans de imagen, el texto es el alt text — almacenar pero no crear run visible
		if (Ctx->bInImage)
		{
			return 0;
		}

		switch (Type)
		{
		case MD_TEXT_NORMAL:
		case MD_TEXT_ENTITY:
		case MD_TEXT_CODE:
		{
			FString TextStr;
			if (Size > 0)
			{
				const FUTF8ToTCHAR Converter(Text, static_cast<int32>(Size));
				TextStr = FString(Converter.Length(), Converter.Get());
			}

			// EN: For entities, do basic HTML entity decode / ES: Para entidades, hacer decode basico de entidades HTML
			if (Type == MD_TEXT_ENTITY)
			{
				TextStr.ReplaceInline(TEXT("&amp;"), TEXT("&"));
				TextStr.ReplaceInline(TEXT("&lt;"), TEXT("<"));
				TextStr.ReplaceInline(TEXT("&gt;"), TEXT(">"));
				TextStr.ReplaceInline(TEXT("&quot;"), TEXT("\""));
				TextStr.ReplaceInline(TEXT("&#39;"), TEXT("'"));
				TextStr.ReplaceInline(TEXT("&nbsp;"), TEXT(" "));
			}

			// EN: For code blocks, accumulate into CodeText / ES: Para code blocks, acumular en CodeText
			if (Block->Type == EDocElementType::CodeBlock)
			{
				Block->CodeText.Append(TextStr);
				return 0;
			}

			// EN: Try to merge with last run if same style and no special attrs
			// ES: Intentar merge con el ultimo run si mismo estilo y sin attrs especiales
			const EDocTextStyle CurrentStyle = Ctx->GetCurrentStyle();
			if (Block->Runs.Num() > 0)
			{
				FDocTextRun& LastRun = Block->Runs.Last();
				if (LastRun.Style == CurrentStyle && LastRun.LinkUrl == Ctx->CurrentLinkUrl
					&& LastRun.ImageSrc.IsEmpty())
				{
					LastRun.Text.Append(TextStr);
					return 0;
				}
			}

			FDocTextRun Run;
			Run.Text = MoveTemp(TextStr);
			Run.Style = CurrentStyle;
			Run.LinkUrl = Ctx->CurrentLinkUrl;
			Block->Runs.Add(MoveTemp(Run));
			break;
		}

		case MD_TEXT_BR:
		{
			// EN: Hard line break → newline run / ES: Salto de linea duro → run de newline
			FDocTextRun Run;
			Run.Text = TEXT("\n");
			Run.Style = Ctx->GetCurrentStyle();
			Block->Runs.Add(MoveTemp(Run));
			break;
		}

		case MD_TEXT_SOFTBR:
		{
			// EN: Soft break → space / ES: Soft break → espacio
			FDocTextRun Run;
			Run.Text = TEXT(" ");
			Run.Style = Ctx->GetCurrentStyle();
			Block->Runs.Add(MoveTemp(Run));
			break;
		}

		case MD_TEXT_HTML:
		{
			// EN: Raw inline HTML — insert as normal text / ES: HTML inline crudo — insertar como texto normal
			if (Size > 0)
			{
				const FUTF8ToTCHAR Converter(Text, static_cast<int32>(Size));
				FDocTextRun Run;
				Run.Text = FString(Converter.Length(), Converter.Get());
				Run.Style = Ctx->GetCurrentStyle();
				Block->Runs.Add(MoveTemp(Run));
			}
			break;
		}

		default:
			break;
		}

		return 0;
	}

#if PLATFORM_WINDOWS
	// EN: SEH wrapper for md_parse (same pattern as MdHtmlWithStackGuard)
	// ES: Wrapper SEH para md_parse (mismo patron que MdHtmlWithStackGuard)
	int MdParseWithStackGuard(const char* Data, MD_SIZE Size, const MD_PARSER* Parser, void* UserData)
	{
		int Result = -1;
		__try
		{
			Result = md_parse(Data, Size, Parser, UserData);
		}
		__except (GetExceptionCode() == EXCEPTION_STACK_OVERFLOW ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
		{
			_resetstkoflw();
			Result = -2;
		}
		return Result;
	}
#endif
}

TArray<FDocElement> FDocSystem::ParseMarkdownToTree(const FString& Markdown)
{
	// EN: Guard: documents beyond 512 KB get empty result to avoid stack overflow
	// ES: Guard: documentos de mas de 512 KB retornan resultado vacio para evitar stack overflow
	static constexpr int32 MaxMarkdownSize = 512 * 1024;
	if (Markdown.Len() > MaxMarkdownSize)
	{
		PGX_LOG_WARNING(LogPGXDocs, TEXT("PGXDocs: Document too large for ParseMarkdownToTree (%d chars, max %d)"), Markdown.Len(), MaxMarkdownSize);
		FDocElement FallbackElem;
		FallbackElem.Type = EDocElementType::CodeBlock;
		FallbackElem.CodeText = Markdown.Left(MaxMarkdownSize);
		TArray<FDocElement> Result;
		Result.Add(MoveTemp(FallbackElem));
		return Result;
	}

	// EN: Convert to UTF8 / ES: Convertir a UTF8
	const FTCHARToUTF8 Utf8Converter(*Markdown);
	const char* Utf8Data = Utf8Converter.Get();
	const MD_SIZE Utf8Size = static_cast<MD_SIZE>(Utf8Converter.Length());

	// EN: Setup parser callbacks / ES: Configurar callbacks del parser
	MD_PARSER Parser;
	FMemory::Memzero(&Parser, sizeof(Parser));
	Parser.abi_version = 0;
	Parser.flags = MD_FLAG_TABLES | MD_FLAG_STRIKETHROUGH | MD_FLAG_TASKLISTS | MD_FLAG_PERMISSIVEAUTOLINKS;
	Parser.enter_block = TreeEnterBlock;
	Parser.leave_block = TreeLeaveBlock;
	Parser.enter_span = TreeEnterSpan;
	Parser.leave_span = TreeLeaveSpan;
	Parser.text = TreeText;
	Parser.debug_log = nullptr;
	Parser.syntax = nullptr;

	FMd4cTreeContext Ctx;

#if PLATFORM_WINDOWS
	const int Result = MdParseWithStackGuard(Utf8Data, Utf8Size, &Parser, &Ctx);
#else
	const int Result = md_parse(Utf8Data, Utf8Size, &Parser, &Ctx);
#endif

	if (Result != 0)
	{
		PGX_LOG_WARNING(LogPGXDocs, TEXT("PGXDocs: ParseMarkdownToTree failed (code %d)"), Result);
	}

	return MoveTemp(Ctx.RootElements);
}

FString FDocSystem::RenderMarkdownToHtml(const FString& Markdown)
{
	// EN: Guard: documents beyond 512 KB of raw markdown get fallback rendering to avoid stack overflow in md4c's recursive parser
	// ES: Guard: documentos de mas de 512 KB de markdown crudo usan renderizado fallback para evitar stack overflow en el parser recursivo de md4c
	static constexpr int32 MaxMarkdownSize = 512 * 1024;
	if (Markdown.Len() > MaxMarkdownSize)
	{
		PGX_LOG_WARNING(LogPGXDocs, TEXT("PGXDocs: Document too large for md4c (%d chars, max %d) — using fallback"), Markdown.Len(), MaxMarkdownSize);
		FString Escaped = Markdown;
		Escaped.ReplaceInline(TEXT("&"), TEXT("&amp;"));
		Escaped.ReplaceInline(TEXT("<"), TEXT("&lt;"));
		Escaped.ReplaceInline(TEXT(">"), TEXT("&gt;"));
		return FString::Printf(TEXT("<pre>%s</pre>"), *Escaped);
	}

	// EN: Convert FString to UTF8 for md4c / ES: Convertir FString a UTF8 para md4c
	const FTCHARToUTF8 Utf8Converter(*Markdown);
	const char* Utf8Data = Utf8Converter.Get();
	const MD_SIZE Utf8Size = static_cast<MD_SIZE>(Utf8Converter.Length());

	// EN: Pre-reserve output — HTML is typically ~1.5x the markdown size
	// ES: Pre-reservar output — HTML es tipicamente ~1.5x el tamano del markdown
	FString HtmlOutput;
	HtmlOutput.Reserve(Markdown.Len() * 3 / 2);

	FMd4cContext OutputCtx;
	OutputCtx.OutputString = &HtmlOutput;

	// EN: md4c flags: CommonMark + GFM extensions / ES: Flags md4c: CommonMark + extensiones GFM
	const unsigned ParseFlags = MD_FLAG_TABLES | MD_FLAG_STRIKETHROUGH | MD_FLAG_TASKLISTS | MD_FLAG_PERMISSIVEAUTOLINKS;
	const unsigned RenderFlags = 0;

	// EN: Call md4c with stack overflow protection on Windows
	// ES: Llamar md4c con proteccion contra stack overflow en Windows
#if PLATFORM_WINDOWS
	const int Result = MdHtmlWithStackGuard(Utf8Data, Utf8Size, Md4cCallback, &OutputCtx, ParseFlags, RenderFlags);
#else
	const int Result = md_html(Utf8Data, Utf8Size, Md4cCallback, &OutputCtx, ParseFlags, RenderFlags);
#endif

	if (Result == -2)
	{
		PGX_LOG_ERROR(LogPGXDocs, TEXT("PGXDocs: Stack overflow in md4c parser — document too complex, using fallback"));
		FString Escaped = Markdown;
		Escaped.ReplaceInline(TEXT("&"), TEXT("&amp;"));
		Escaped.ReplaceInline(TEXT("<"), TEXT("&lt;"));
		Escaped.ReplaceInline(TEXT(">"), TEXT("&gt;"));
		return FString::Printf(TEXT("<pre>%s</pre>"), *Escaped);
	}

	if (Result != 0)
	{
		PGX_LOG_WARNING(LogPGXDocs, TEXT("PGXDocs: md4c parse error (code %d)"), Result);
		return FString::Printf(TEXT("<pre>%s</pre>"), *Markdown);
	}

	return HtmlOutput;
}
