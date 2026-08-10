// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXDocTypes.generated.h"

// ============================================================================
// EN: Core data types for PGX Documentation System.
//     Shared between PGXDocs (services) and PGXDocsEditor (UI).
//     FDocElement tree is the cross-DLL data bridge between md4c parsing
//     (PGXDocs module) and Slate rendering (PGXDocsEditor module).
//
// ES: Tipos de datos core para el Sistema de Documentacion PGX.
//     Compartidos entre PGXDocs (servicios) y PGXDocsEditor (UI).
//     El arbol FDocElement es el data bridge cross-DLL entre parsing md4c
//     (modulo PGXDocs) y rendering Slate (modulo PGXDocsEditor).
// ============================================================================

// ============================================================================
// EN: Document Element Tree — intermediate representation for Slate rendering
// ES: Arbol de Elementos de Documento — representacion intermedia para rendering Slate
// ============================================================================

/** EN: Text alignment for table cells / ES: Alineacion de texto para celdas de tabla */
enum class EDocAlign : uint8
{
	Left,
	Center,
	Right
};

/** EN: Type of document block element / ES: Tipo de elemento bloque del documento */
enum class EDocElementType : uint8
{
	Paragraph,
	Heading,
	CodeBlock,
	Blockquote,
	HorizontalRule,
	UnorderedList,
	OrderedList,
	ListItem,
	Table,
	TableRow,
	TableCell,
};

/** EN: Inline text style / ES: Estilo inline de texto */
enum class EDocTextStyle : uint8
{
	Normal,
	Bold,
	Italic,
	BoldItalic,
	Code,
	Strikethrough,
	Underline
};

/**
 * EN: A run of text with uniform style. Multiple runs compose inline content.
 * ES: Un fragmento de texto con estilo uniforme. Multiples runs componen contenido inline.
 */
struct PGXDOCS_API FDocTextRun
{
	FString Text;
	EDocTextStyle Style = EDocTextStyle::Normal;
	FString LinkUrl;        // EN: Non-empty if hyperlink / ES: No-vacio si es hyperlink
	FString ImageSrc;       // EN: Non-empty if inline image / ES: No-vacio si es imagen inline
};

/**
 * EN: A document element (block or container). Forms a tree structure.
 *     Blocks contain either inline text (Runs) or child blocks (Children), or both.
 * ES: Un elemento del documento (bloque o contenedor). Forma una estructura de arbol.
 *     Los bloques contienen texto inline (Runs) o bloques hijos (Children), o ambos.
 */
struct PGXDOCS_API FDocElement
{
	EDocElementType Type = EDocElementType::Paragraph;
	TArray<FDocTextRun> Runs;        // EN: Inline text (for P, H, LI, TD) / ES: Texto inline (para P, H, LI, TD)
	TArray<FDocElement> Children;     // EN: Sub-elements (for List, Table, Blockquote) / ES: Sub-elementos (para List, Table, Blockquote)

	// EN: Metadata per type / ES: Metadata por tipo
	int32 HeadingLevel = 0;          // 1-6 for Heading
	FString CodeLanguage;            // For CodeBlock
	FString CodeText;                // Full text of CodeBlock
	int32 ListStart = 1;             // Start of OL
	bool bIsTaskItem = false;        // Task list item
	bool bIsTaskChecked = false;     // Task list checked
	bool bIsTableHeader = false;     // TableCell is header (TH)
	EDocAlign CellAlign = EDocAlign::Left;  // Cell alignment
};

/**
 * EN: Represents a single parsed documentation file with metadata and cached renders.
 * ES: Representa un archivo de documentacion parseado con metadata y renders cacheados.
 */
USTRUCT(BlueprintType)
struct PGXDOCS_API FDocDocument
{
	GENERATED_BODY()

	/** EN: Unique identifier — "module_id/chapter_id" or relative path / ES: Identificador unico */
	UPROPERTY(BlueprintReadOnly, Category = "PGXDocs")
	FString DocId;

	/** EN: Module this doc belongs to / ES: Modulo al que pertenece este doc */
	UPROPERTY(BlueprintReadOnly, Category = "PGXDocs")
	FString ModuleId;

	/** EN: Chapter identifier within the module / ES: Identificador de capitulo dentro del modulo */
	UPROPERTY(BlueprintReadOnly, Category = "PGXDocs")
	FString ChapterId;

	/** EN: Display title (from H1 or manifest) / ES: Titulo para mostrar (de H1 o manifest) */
	UPROPERTY(BlueprintReadOnly, Category = "PGXDocs")
	FString Title;

	/** EN: Absolute file path on disk / ES: Ruta absoluta en disco */
	UPROPERTY(BlueprintReadOnly, Category = "PGXDocs")
	FString AbsolutePath;

	/** EN: Raw .md file content / ES: Contenido crudo del archivo .md */
	FString RawMarkdown;

	/** EN: Rendered HTML (cached after md4c parse) / ES: HTML renderizado (cacheado despues del parse md4c) */
	FString Html;

	/** EN: Stripped plain text for search indexing / ES: Texto plano para indice de busqueda */
	FString PlainText;

	/** EN: Extracted headings: {text, anchorId} / ES: Headings extraidos: {texto, anchorId} */
	TArray<TPair<FString, FString>> Headings;

	/** EN: Parsed YAML frontmatter key-value pairs / ES: Pares clave-valor de frontmatter YAML parseados */
	TMap<FString, FString> FrontMatter;

	/** EN: Tags from frontmatter or manifest / ES: Tags del frontmatter o manifest */
	UPROPERTY(BlueprintReadOnly, Category = "PGXDocs")
	TArray<FString> Tags;

	/** EN: Last modified time in UTC ticks / ES: Ultimo tiempo de modificacion en ticks UTC */
	int64 ModifiedTimeUtc = 0;

	/** EN: Hash of content for incremental rebuild / ES: Hash del contenido para rebuild incremental */
	uint64 ContentHash = 0;
};

/**
 * EN: Tree node for documentation navigation hierarchy.
 * ES: Nodo de arbol para la jerarquia de navegacion de documentacion.
 */
struct PGXDOCS_API FDocNode
{
	/** EN: Node identifier (unique within tree) / ES: Identificador de nodo */
	FString Id;

	/** EN: Display title / ES: Titulo para mostrar */
	FString Title;

	/** EN: DocId if this is a leaf (empty if branch/folder) / ES: DocId si es hoja (vacio si rama/carpeta) */
	FString DocId;

	/** EN: Child nodes / ES: Nodos hijos */
	TArray<TSharedPtr<FDocNode>> Children;

	/** EN: Sort order from manifest or alphabetical / ES: Orden de manifest o alfabetico */
	int32 Order = 0;

	/** EN: Tags for this node / ES: Tags para este nodo */
	TArray<FString> Tags;

	/** EN: UI expansion state / ES: Estado de expansion en UI */
	bool bIsExpanded = false;
};

/**
 * EN: A single search result with match context and score.
 * ES: Un resultado de busqueda individual con contexto y puntuacion.
 */
USTRUCT(BlueprintType)
struct PGXDOCS_API FDocSearchResult
{
	GENERATED_BODY()

	/** EN: Document identifier / ES: Identificador del documento */
	UPROPERTY(BlueprintReadOnly, Category = "PGXDocs")
	FString DocId;

	/** EN: Document title / ES: Titulo del documento */
	UPROPERTY(BlueprintReadOnly, Category = "PGXDocs")
	FString Title;

	/** EN: Module the doc belongs to / ES: Modulo del documento */
	UPROPERTY(BlueprintReadOnly, Category = "PGXDocs")
	FString ModuleId;

	/** EN: Snippet around the match / ES: Extracto alrededor del match */
	UPROPERTY(BlueprintReadOnly, Category = "PGXDocs")
	FString MatchContext;

	/** EN: Relevance score (higher = better) / ES: Puntuacion de relevancia (mayor = mejor) */
	UPROPERTY(BlueprintReadOnly, Category = "PGXDocs")
	int32 Score = 0;
};

/**
 * EN: Filters for narrowing search results.
 * ES: Filtros para acotar resultados de busqueda.
 */
USTRUCT(BlueprintType)
struct PGXDOCS_API FDocSearchFilters
{
	GENERATED_BODY()

	/** EN: Filter by module ID / ES: Filtrar por ID de modulo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGXDocs")
	FString ModuleFilter;

	/** EN: Filter by tags / ES: Filtrar por tags */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGXDocs")
	TArray<FString> TagFilters;
};

/**
 * EN: A single validation entry (error or warning).
 * ES: Una entrada de validacion individual (error o warning).
 */
USTRUCT(BlueprintType)
struct PGXDOCS_API FDocValidationEntry
{
	GENERATED_BODY()

	/** EN: Document where the issue was found / ES: Documento donde se encontro el problema */
	UPROPERTY(BlueprintReadOnly, Category = "PGXDocs")
	FString DocId;

	/** EN: Human-readable message / ES: Mensaje legible */
	UPROPERTY(BlueprintReadOnly, Category = "PGXDocs")
	FString Message;

	/** EN: Type: "BrokenLink", "MissingImage", "OrphanDoc" / ES: Tipo de problema */
	UPROPERTY(BlueprintReadOnly, Category = "PGXDocs")
	FString Type;
};

/**
 * EN: Complete validation report for all documentation.
 * ES: Reporte de validacion completo para toda la documentacion.
 */
USTRUCT(BlueprintType)
struct PGXDOCS_API FDocValidationReport
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGXDocs")
	TArray<FDocValidationEntry> Errors;

	UPROPERTY(BlueprintReadOnly, Category = "PGXDocs")
	TArray<FDocValidationEntry> Warnings;

	UPROPERTY(BlueprintReadOnly, Category = "PGXDocs")
	int32 TotalDocs = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PGXDocs")
	int32 DocsWithErrors = 0;
};
