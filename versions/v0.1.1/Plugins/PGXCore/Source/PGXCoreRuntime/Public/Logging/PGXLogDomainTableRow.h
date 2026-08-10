// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Construction/PGXConstructionTypes.h"
#include "PGXLogDomainTableRow.generated.h"

class UPGXLogDomainRendererBase;

/**
 * EN: DataTable row struct for bulk domain configuration.
 *     Each row defines a log domain: tag, color, display name, renderer class, and auto-infer categories.
 *     Attach a DataTable using this row struct to UPGXLogDomainConfig::DomainDataTable for bulk setup.
 *     The subsystem ingests rows during DiscoverDomainConfigs() — each row becomes a virtual domain.
 *
 * ES: Struct de fila DataTable para configuracion masiva de dominios.
 *     Cada fila define un dominio de log: tag, color, nombre, clase renderer y categorias auto-infer.
 *     Adjuntar un DataTable usando esta fila a UPGXLogDomainConfig::DomainDataTable para setup masivo.
 *     El subsistema ingesta filas durante DiscoverDomainConfigs() — cada fila se convierte en un dominio virtual.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXLogDomainTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** EN: Domain tag that entries must match / ES: Tag de dominio que las entradas deben coincidir */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Domain", meta = (Categories = "PGX.Log.Domain"))
	FGameplayTag DomainTag;

	/** EN: Display color for this domain in the Log Viewer / ES: Color de display para este dominio */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Domain")
	FLinearColor DomainColor = FLinearColor(0.5f, 0.5f, 0.5f);

	/** EN: Human-readable name shown in filter bar and detail windows / ES: Nombre legible mostrado en barra de filtro y ventanas de detalle */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Domain")
	FText DomainDisplayName;

	/** EN: Optional icon name from PGXEditorStyle / ES: Nombre opcional de icono de PGXEditorStyle */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Domain")
	FName DomainIcon;

	/** EN: How to resolve the renderer class / ES: Como resolver la clase del renderer */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Renderer")
	EPGXClassSourceMode RendererSourceMode = EPGXClassSourceMode::Default;

	/** EN: C++ renderer class (when RendererSourceMode == CppClass) / ES: Clase renderer C++ */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Renderer",
		meta = (EditCondition = "RendererSourceMode == EPGXClassSourceMode::CppClass"))
	TSubclassOf<UPGXLogDomainRendererBase> CppRendererClass;

	/** EN: Blueprint renderer class (when RendererSourceMode == Blueprint) / ES: Clase renderer Blueprint */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Renderer",
		meta = (EditCondition = "RendererSourceMode == EPGXClassSourceMode::Blueprint"))
	TSoftClassPtr<UPGXLogDomainRendererBase> BlueprintRendererClass;

	/** EN: Log categories that auto-map to this domain (backward compat with v2.2) / ES: Categorias de log que auto-mapean a este dominio */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AutoInfer")
	TArray<FName> AutoInferCategories;
};
