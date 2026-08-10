// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Data/PGXConfigDataAsset.h"
#include "Trace/PGXTraceTypes.h"
#include "PGXDocsConfig.generated.h"

// ============================================================================
// EN: Support Enums
// ES: Enums de soporte
// ============================================================================

/** EN: Preferred display language for bilingual docs / ES: Idioma preferido para docs bilingues */
UENUM(BlueprintType)
enum class EPGXDocsLanguage : uint8
{
	English    UMETA(DisplayName = "English"),
	Spanish    UMETA(DisplayName = "Español"),
	Both       UMETA(DisplayName = "Both (EN/ES)")
};

/** EN: Documentation viewer theme / ES: Tema del visor de documentacion */
UENUM(BlueprintType)
enum class EPGXDocsTheme : uint8
{
	Dark     UMETA(DisplayName = "Dark"),
	Light    UMETA(DisplayName = "Light"),
	System   UMETA(DisplayName = "Match Editor")
};

// ============================================================================
// EN: Config DataAsset Dashboard — rich, data-driven configuration
//     Following the PGX dual-layer pattern: DeveloperSettings + Config DA.
//
// ES: Dashboard Config DataAsset — configuracion rica, data-driven
//     Siguiendo el patron dual PGX: DeveloperSettings + Config DA.
// ============================================================================

/**
 * EN: Rich data-driven configuration for PGX Documentation Viewer.
 *     Create via Content Browser > PGX Framework > Documentation > Docs Config.
 *     Controls doc roots, language, display, search, filtering, crossrefs,
 *     security, validation, and traceability.
 *
 * ES: Configuracion rica data-driven para el Visor de Documentacion PGX.
 *     Crear via Content Browser > PGX Framework > Documentation > Docs Config.
 *     Controla raices de docs, idioma, display, busqueda, filtrado, crossrefs,
 *     seguridad, validacion y trazabilidad.
 */
UCLASS(BlueprintType, meta = (DisplayName = "PGX Docs Config"))
class PGXDOCS_API UPGXDocsConfig : public UPGXConfigDataAsset
{
	GENERATED_BODY()

public:

	// ============================================================
	// ROOTS — Where to look for documentation
	// ============================================================

	/** EN: Include <Project>/Docs/ as documentation root / ES: Incluir <Project>/Docs/ como raiz de documentacion */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Roots",
		meta = (ToolTip = "Include <Project>/Docs/ as documentation root"))
	bool bIncludeProjectDocs = true;


	/** EN: Include plugin Docs/ folders / ES: Incluir carpetas Docs/ de plugins */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Roots",
		meta = (ToolTip = "Include plugin Docs/ folders (e.g. Plugins/PGXDocs/Docs/Sample)"))
	bool bIncludePluginDocs = true;

	/** EN: Additional documentation root directories / ES: Directorios raiz de documentacion adicionales */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Roots",
		meta = (AdvancedDisplay, ToolTip = "Additional documentation root directories"))
	TArray<FDirectoryPath> AdditionalDocRoots;

	// ============================================================
	// LANGUAGE — Display language
	// ============================================================

	/** EN: Preferred display language for bilingual docs / ES: Idioma preferido para docs bilingues */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Language",
		meta = (ToolTip = "Preferred display language for bilingual docs"))
	EPGXDocsLanguage PreferredLanguage = EPGXDocsLanguage::Both;

	// ============================================================
	// DISPLAY — Visual customization
	// ============================================================

	/** EN: Dark or Light theme / ES: Tema Dark o Light */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display",
		meta = (ToolTip = "Dark or Light theme"))
	EPGXDocsTheme Theme = EPGXDocsTheme::Dark;

	/** EN: Base font size in pixels / ES: Tamano de fuente base en pixeles */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display",
		meta = (AdvancedDisplay, ClampMin = "10", ClampMax = "24", ToolTip = "Base font size in pixels"))
	int32 BaseFontSize = 14;

	/** EN: Sidebar width percentage / ES: Porcentaje de ancho del sidebar */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display",
		meta = (AdvancedDisplay, ClampMin = "15", ClampMax = "40", ToolTip = "Sidebar width percentage"))
	int32 SidebarWidthPercent = 25;

	/** EN: Show YAML frontmatter metadata for documents / ES: Mostrar metadata frontmatter YAML de documentos */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display",
		meta = (AdvancedDisplay, ToolTip = "Show YAML frontmatter metadata for documents"))
	bool bShowFrontMatterPanel = true;

	/** EN: Show document status bar / ES: Mostrar barra de estado del documento */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display",
		meta = (AdvancedDisplay, ToolTip = "Show document status bar (doc count, watcher state, etc.)"))
	bool bShowStatusBar = true;

	// ============================================================
	// SEARCH — Search configuration
	// ============================================================

	/** EN: Max search results displayed / ES: Maximo de resultados de busqueda mostrados */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Search",
		meta = (AdvancedDisplay, ClampMin = "5", ClampMax = "100", ToolTip = "Max search results displayed"))
	int32 MaxSearchResults = 25;

	/** EN: Auto-rebuild index when docs change / ES: Auto-reconstruir indice cuando cambian los docs */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Search",
		meta = (AdvancedDisplay, ToolTip = "Auto-rebuild index when docs change (requires live reload)"))
	bool bAutoRebuildIndex = true;

	// ============================================================
	// FILTERING — Include/exclude rules
	// ============================================================

	/** EN: File patterns to exclude / ES: Patrones de archivo a excluir */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Filtering",
		meta = (AdvancedDisplay, ToolTip = "File patterns to exclude (e.g. '_Draft*', 'TEMP_*')"))
	TArray<FString> ExcludePatterns;

	/** EN: Folders to exclude from scan / ES: Carpetas a excluir del scan */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Filtering",
		meta = (AdvancedDisplay, ToolTip = "Folders to exclude from scan (relative to project root)"))
	TArray<FDirectoryPath> ExcludedFolders;

	// ============================================================
	// CROSSREFS — Cross-reference links
	// ============================================================

	/** EN: Enable [[Class:...]] links that open C++ source / ES: Habilitar links [[Class:...]] que abren fuente C++ */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CrossRefs",
		meta = (AdvancedDisplay, ToolTip = "Enable [[Class:...]] links that open C++ source"))
	bool bEnableClassLinks = true;

	/** EN: Enable [[BP:...]] links that open Blueprint assets / ES: Habilitar links [[BP:...]] que abren assets Blueprint */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CrossRefs",
		meta = (AdvancedDisplay, ToolTip = "Enable [[BP:...]] links that open Blueprint assets"))
	bool bEnableBlueprintLinks = true;

	/** EN: Enable [[Doc:...]] internal doc navigation / ES: Habilitar navegacion interna [[Doc:...]] */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CrossRefs",
		meta = (AdvancedDisplay, ToolTip = "Enable [[Doc:...]] internal doc navigation"))
	bool bEnableDocLinks = true;

	// ============================================================
	// SECURITY — Security policies
	// ============================================================

	/** EN: Allow opening external URLs from documentation / ES: Permitir abrir URLs externas desde documentacion */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Security",
		meta = (AdvancedDisplay, ToolTip = "Allow opening external URLs from documentation"))
	bool bEnableExternalLinks = false;

	/** EN: Restrict image loading to configured roots only / ES: Restringir carga de imagenes solo a raices configuradas */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Security",
		meta = (AdvancedDisplay, ToolTip = "Restrict image loading to configured roots only"))
	bool bSandboxImagePaths = true;

	// ============================================================
	// VALIDATION — Validation configuration
	// ============================================================

	/** EN: Auto-validate docs when opening viewer / ES: Auto-validar docs al abrir visor */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation",
		meta = (AdvancedDisplay, ToolTip = "Auto-validate docs when opening viewer"))
	bool bValidateOnOpen = false;

	/** EN: Path for validation report output / ES: Ruta para output del reporte de validacion */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation",
		meta = (AdvancedDisplay, ToolTip = "Path for validation report output"))
	FString ValidationReportPath = TEXT("Saved/PGXDocs/validation_report.json");

	// ============================================================
	// TRACEABILITY — Infrastructure v0.4.0
	// ============================================================

	/** EN: Tracing configuration for this system / ES: Configuracion de trazado para este sistema */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace", meta = (AdvancedDisplay))
	FPGXTraceConfig TraceConfig;
};
