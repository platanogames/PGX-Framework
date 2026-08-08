// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: SPGXPass1InspectorBase — canonical PASS1 read-only inspector base class.
//     Shared extraction. Subclasses declare a title and two lists
//     (observable classes + deferred cards) via virtual overrides; the base owns
//     the standard ChildSlot layout (header + scroll + iterate) reachable via
//     BuildInspectorLayout(). For premium variants (Online / PSO / Multiplayer /
//     Interaction) override BuildHeader() to return a SPGXPanelHeader.
//
//     Slate inheritance pattern: this base intentionally does NOT declare its own
//     SLATE_BEGIN_ARGS / Construct. Each leaf subclass declares its own
//     SLATE_BEGIN_ARGS(SPGX<X>Inspector) + Construct(FArguments) and inside the
//     subclass Construct calls SPGXPass1InspectorBase::BuildInspectorLayout().
//     This avoids the FArguments type-mismatch issue when SLATE_BEGIN_ARGS is
//     declared on both base + leaf classes.
// ES: SPGXPass1InspectorBase — base canonical para inspectores PASS1 read-only.
//     Las subclases declaran titulo y dos listas mediante virtuales; la
//     base expone BuildInspectorLayout() que cada subclass llama desde su propio
//     Construct. Sobrescribir BuildHeader() para inyectar SPGXPanelHeader.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class UClass;

class PGXCOREEDITOR_API SPGXPass1InspectorBase : public SCompoundWidget
{
protected:
	/**
	 * EN: Builds the standard PASS1 inspector layout into ChildSlot. Subclasses MUST call
	 *     this exactly once from their own Construct(). Uses the virtual hooks below to
	 *     gather plugin-specific data; the layout itself (SScrollBox + iteration + padding
	 *     via PGX::Spacing tokens) is shared.
	 * ES: Construye el layout estandar PASS1 en ChildSlot. Las subclases DEBEN llamarlo
	 *     una sola vez desde su propio Construct(). Usa los virtual hooks de abajo para
	 *     datos plugin-specific; el layout es compartido.
	 */
	void BuildInspectorLayout();

	/**
	 * EN: Header title text used by the default BuildHeader() impl. Override to change
	 *     the title without replacing the entire header widget.
	 * ES: Texto del titulo usado por BuildHeader() default. Sobrescribir para cambiar
	 *     titulo sin reemplazar el widget.
	 */
	virtual FText GetInspectorTitle() const;

	/**
	 * EN: List of (Label, UClass*) pairs rendered as observable cards. Each pair becomes
	 *     a card via PGX::Inspector::MakeObservableCard. Default is empty.
	 * ES: Lista de pares (Label, UClass*) renderizados como observable cards.
	 */
	virtual TArray<TPair<FText, UClass*>> GetObservableClasses() const;

	/**
	 * EN: List of (Label, Detail) pairs rendered as deferred cards. Default is empty.
	 * ES: Lista de pares (Label, Detail) renderizados como deferred cards.
	 */
	virtual TArray<TPair<FText, FText>> GetDeferredCards() const;

	/**
	 * EN: Builds the header widget rendered above the scroll body. Default returns a
	 *     STextBlock with GetInspectorTitle(). Override to inject SPGXPanelHeader.
	 * ES: Construye el header. Default = STextBlock con GetInspectorTitle().
	 */
	virtual TSharedRef<SWidget> BuildHeader() const;
};
