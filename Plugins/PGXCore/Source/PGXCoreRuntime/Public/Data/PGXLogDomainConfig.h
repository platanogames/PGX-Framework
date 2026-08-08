// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Construction/PGXConstructionTypes.h"
#include "Observability/PGXObservable.h"
#include "PGXLogDomainConfig.generated.h"

class UPGXLogDomainRendererBase;
class UDataTable;

/**
 * EN: Configuration Data Asset for a log domain.
 *     Each domain (Audio, Save, GameFlow...) has one of these.
 *     Auto-discovered by UPGXLogSubsystem at initialization via AssetRegistry.
 *
 *     UX-First: The DA is the box. The renderer is the shape.
 *     - Create in Content Browser: PGX Data Assets > Log Domain Config
 *     - Assign a domain tag, color, display name
 *     - Choose renderer: PGX default (C++) or custom (C++ or Blueprint)
 *     - The Log Viewer picks it up automatically
 *
 * ES: Data Asset de configuracion para un dominio de log.
 *     Cada dominio (Audio, Save, GameFlow...) tiene uno de estos.
 *     Auto-descubierto por UPGXLogSubsystem en inicializacion via AssetRegistry.
 */
UCLASS(BlueprintType)
class PGXCORERUNTIME_API UPGXLogDomainConfig : public UPrimaryDataAsset, public IPGXObservable
{
	GENERATED_BODY()

public:
	UPGXLogDomainConfig();

	//~ Begin IPGXObservable (delegates to PGXCoreObservability helpers)
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;
	//~ End IPGXObservable

	// ═══════════════════════════════════════
	// Domain Identity / Identidad de Dominio
	// ═══════════════════════════════════════

	/** EN: Domain tag that entries must match (e.g. PGX.Log.Domain.Audio) / ES: Tag de dominio que las entradas deben coincidir */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Domain", meta = (Categories = "PGX.Log.Domain"))
	FGameplayTag DomainTag;

	/** EN: Display color for this domain in the Log Viewer / ES: Color de display para este dominio */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Domain")
	FLinearColor DomainColor = FLinearColor(0.5f, 0.5f, 0.5f);

	/** EN: Human-readable name shown in filter bar and detail windows / ES: Nombre legible mostrado en barra de filtro y ventanas de detalle */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Domain")
	FText DomainDisplayName;

	/** EN: Optional icon name from PGXEditorStyle / ES: Nombre opcional de icono de PGXEditorStyle */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Domain", meta = (AdvancedDisplay))
	FName DomainIcon;

	// ═══════════════════════════════════════
	// Renderer Class / Clase de Renderer
	// ═══════════════════════════════════════

	/**
	 * EN: How to resolve the renderer class.
	 *     Default: uses the PGX built-in renderer for this domain.
	 *     CppClass: uses a custom C++ subclass.
	 *     Blueprint: uses a Blueprint subclass.
	 * ES: Como resolver la clase del renderer.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Renderer", meta = (AdvancedDisplay))
	EPGXClassSourceMode RendererSourceMode = EPGXClassSourceMode::Default;

	/** EN: C++ renderer class (when RendererSourceMode == CppClass) / ES: Clase renderer C++ */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Renderer",
		meta = (AdvancedDisplay, EditCondition = "RendererSourceMode == EPGXClassSourceMode::CppClass"))
	TSubclassOf<UPGXLogDomainRendererBase> CppRendererClass;

	/** EN: Blueprint renderer class (when RendererSourceMode == Blueprint) / ES: Clase renderer Blueprint */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Renderer",
		meta = (AdvancedDisplay, EditCondition = "RendererSourceMode == EPGXClassSourceMode::Blueprint"))
	TSoftClassPtr<UPGXLogDomainRendererBase> BlueprintRendererClass;

	// ═══════════════════════════════════════
	// Bulk Config / Config Masiva
	// ═══════════════════════════════════════

	/** EN: Optional DataTable for bulk domain configuration (row struct: FPGXLogDomainTableRow) / ES: DataTable opcional para config masiva */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bulk",
		meta = (AdvancedDisplay, RequiredAssetDataTags = "RowStructure=/Script/PGXCoreRuntime.PGXLogDomainTableRow"))
	TObjectPtr<UDataTable> DomainDataTable;

	// ═══════════════════════════════════════
	// Auto-Infer Categories / Categorias Auto-Inferidas
	// ═══════════════════════════════════════

	/**
	 * EN: Log categories that auto-map to this domain (for backward compat with v2.2).
	 *     Entries with these categories get this domain tag auto-assigned if they have no DomainTag.
	 *     e.g. {"LogPGXAudio", "LogPGXAudioGI"} → PGX.Log.Domain.Audio
	 * ES: Categorias de log que auto-mapean a este dominio (para backward compat con v2.2).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AutoInfer", meta = (AdvancedDisplay))
	TArray<FName> AutoInferCategories;

	// ═══════════════════════════════════════
	// UPrimaryDataAsset Interface
	// ═══════════════════════════════════════

	FPrimaryAssetId GetPrimaryAssetId() const override;
};
