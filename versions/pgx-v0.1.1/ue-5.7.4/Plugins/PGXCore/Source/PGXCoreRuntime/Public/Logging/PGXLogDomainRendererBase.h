// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Logging/PGXLogTypes.h"
#include "Widgets/SWidget.h"
#include "PGXLogDomainRendererBase.generated.h"

/**
 * EN: Base class for domain-specific log renderers.
 *     Each PGX subsystem can override this to provide custom column definitions,
 *     row widgets, and detail panels for its log entries in the Log Viewer.
 *
 *     Extension pattern:
 *     1. Subclass UPGXLogDomainRendererBase (C++ or Blueprint)
 *     2. Override GetDomainColor(), GetDomainDisplayName(), GetColumnDefinitions()
 *     3. For C++ custom Slate: override BuildRowWidget() / BuildDetailWidget()
 *     4. Create a UPGXLogDomainConfig DA and assign the renderer class
 *     5. The Log Viewer auto-discovers and uses it
 *
 *     The base implementation provides a generic row (time + verbosity badge +
 *     category + message) and a detail panel showing all params and tags.
 *
 * ES: Clase base para renderers de log especificos de dominio.
 *     Cada subsistema PGX puede heredar para proveer definiciones de columna custom,
 *     widgets de fila, y paneles de detalle para sus entradas en el Log Viewer.
 */
UCLASS(Blueprintable, Abstract)
class PGXCORERUNTIME_API UPGXLogDomainRendererBase : public UObject
{
	GENERATED_BODY()

public:
	// ═══════════════════════════════════════
	// Data Methods (overridable in C++ and BP)
	// Metodos de Datos (overridables en C++ y BP)
	// ═══════════════════════════════════════

	/** EN: Domain display color for badges and accents / ES: Color de dominio para badges y acentos */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|Log|Domain")
	FLinearColor GetDomainColor() const;
	virtual FLinearColor GetDomainColor_Implementation() const;

	/** EN: Human-readable domain name / ES: Nombre de dominio legible */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|Log|Domain")
	FText GetDomainDisplayName() const;
	virtual FText GetDomainDisplayName_Implementation() const;

	/** EN: Extra columns this domain adds beyond the standard (Time, Verb, Category, Message) / ES: Columnas extra que agrega este dominio */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|Log|Domain")
	TArray<FPGXLogColumnDef> GetColumnDefinitions() const;
	virtual TArray<FPGXLogColumnDef> GetColumnDefinitions_Implementation() const;

	/** EN: Whether this domain supports opening a detail window on double-click / ES: Si este dominio soporta ventana de detalle al doble-click */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|Log|Domain")
	bool SupportsDetailWindow() const;
	virtual bool SupportsDetailWindow_Implementation() const;

	// ═══════════════════════════════════════
	// Slate Widget Methods (C++ override only)
	// Metodos de Widget Slate (solo override C++)
	// ═══════════════════════════════════════

	/**
	 * EN: Build the row widget for this entry. Default: standard columns + domain-specific params.
	 *     Override in C++ subclasses for fully custom row rendering.
	 * ES: Construir el widget de fila para esta entrada. Default: columnas estandar + params de dominio.
	 */
	virtual TSharedRef<SWidget> BuildRowWidget(const FPGXLogEntry& Entry) const;

	/**
	 * EN: Build the detail window content for this entry. Default: all params + tags.
	 *     Override in C++ subclasses for fully custom detail rendering.
	 * ES: Construir el contenido de la ventana de detalle. Default: todos los params + tags.
	 */
	virtual TSharedRef<SWidget> BuildDetailWidget(const FPGXLogEntry& Entry) const;

protected:
	/**
	 * EN: Helper: extract a param value from the entry by key. Returns empty string if not found.
	 * ES: Helper: extraer valor de param de la entrada por key. Retorna string vacio si no existe.
	 */
	static FString GetParamValue(const FPGXLogEntry& Entry, const FString& Key);
};
