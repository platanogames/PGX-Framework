// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Inspector helpers — shared widget factories + observable describer for PASS1 inspectors.
//     Shared extraction that eliminates duplicated card construction (MakeCard /
//     BuildObservableCard / BuildDeferredCard / DescribeObservableClass) across 13 plugins.
// ES: Helpers de inspector — fabricas de widgets y describer de observables para PASS1.
//     Colapsa la construccion duplicada cross-plugin en una sola fuente canonica.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class SWidget;

namespace PGX
{
	namespace Inspector
	{
		/**
		 * EN: Builds a simple two-row card (Label + Detail) used by PASS1 inspectors.
		 *     Layout: SBorder with PGX::Spacing::MD padding, SVerticalBox with two
		 *     SText slots (Label primary + Detail with auto-wrap + PGX::Spacing::SM top gap).
		 * ES: Construye una card de 2 filas (Label + Detail) usada por inspectores PASS1.
		 *     Layout: SBorder con padding PGX::Spacing::MD, SVerticalBox con 2 slots SText
		 *     (Label primario + Detail con auto-wrap + gap superior PGX::Spacing::SM).
		 */
		PGXCOREEDITOR_API TSharedRef<SWidget> MakeCard(const FText& Label, const FText& Detail);

		/**
		 * EN: Builds an observable card — same layout as MakeCard but the Detail field is
		 *     auto-populated by DescribeObservableClass(ObservableClass).
		 * ES: Construye una card observable — mismo layout que MakeCard pero el campo Detail
		 *     se rellena via DescribeObservableClass(ObservableClass).
		 */
		PGXCOREEDITOR_API TSharedRef<SWidget> MakeObservableCard(const FText& Label, UClass* ObservableClass);

		/**
		 * EN: Builds a deferred card — semantic alias of MakeCard used to mark deferred
		 *     PASS2/PASS3 surfaces. Kept as separate symbol for grep-friendliness and to
		 *     allow visual differentiation (e.g. muted text color) without touching
		 *     callers.
		 * ES: Construye una card deferred — alias semantico de MakeCard para marcar
		 *     superficies diferidas a PASS2/PASS3. Simbolo separado para grep + para permitir
		 *     diferenciacion visual (e.g. color muted) sin tocar callers.
		 */
		PGXCOREEDITOR_API TSharedRef<SWidget> MakeDeferredCard(const FText& Label, const FText& Detail);

		/**
		 * EN: Returns a one-line summary of an observable class:
		 *     "Class=<TypeName> | Schema=<SchemaVersion> | Fields=<N> | Registry=<state>".
		 *     Resolves IPGXObservable + FPGXObservabilityRegistry from PGXCoreRuntime.
		 *     If ObservableClass is null, returns "Class missing; cannot inspect observable contract.".
		 *     If class does not implement IPGXObservable, returns "<ClassName> does not implement IPGXObservable.".
		 * ES: Devuelve resumen de una linea de una clase observable. Resuelve IPGXObservable +
		 *     FPGXObservabilityRegistry de PGXCoreRuntime. Maneja null + sin-interface.
		 */
		PGXCOREEDITOR_API FText DescribeObservableClass(UClass* ObservableClass);
	}
}
