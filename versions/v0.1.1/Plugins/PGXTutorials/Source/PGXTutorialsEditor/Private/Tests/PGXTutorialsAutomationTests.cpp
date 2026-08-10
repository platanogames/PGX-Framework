// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "PGXTutorialTypes.h"

/**
 * EN: WITH_DEV_AUTOMATION_TESTS smoke tests for PGXTutorials editor types.
 *     File-level guard + IMPLEMENT_SIMPLE_AUTOMATION_TEST direct macro —
 *     no PGXCoreDeveloper dep, so PGXTutorialsEditor.Build.cs needs no
 *     mutation. Tests use the flat `PGX.Tutorials.<Name>` namespace.
 *
 *     Scope intentionally narrow at baseline:
 *     - Type construction sanity (enum default values + struct default
 *       construction + cardinality).
 *     - No PIE / editor-process fixture required.
 *
 *     Deferred until a PIE/editor fixture is available:
 *     - Module surface tests (NomadTab spawner registered, console
 *       command count post-StartupModule).
 *     - FPGXTutorialRunner state machine lifecycle.
 *     - FPGXTutorialActionExecutor end-to-end (CreateFolder / CreateAsset
 *       / OpenAsset / NavigateCB / ConfigBasePath all require an editor
 *       process with real Content Browser).
 *
 *
 * ES: Smoke tests WITH_DEV_AUTOMATION_TESTS para tipos editor de PGXTutorials.
 *     Guard file-level + macro IMPLEMENT_SIMPLE_AUTOMATION_TEST direct
 *     — sin dependencia PGXCoreDeveloper, asi que
 *     PGXTutorialsEditor.Build.cs no muta. Los tests usan el namespace
 *     `PGX.Tutorials.<Name>`.
 *
 *     Scope intencionalmente estrecho en baseline:
 *     - Sanity de construccion de tipos (defaults de enum + construccion
 *       default de struct + cardinalidad).
 *     - Sin fixture PIE / editor-process requerido.
 *
 *     DIFERIDO hasta disponer de fixture PIE/editor:
 *     - Tests de surface de modulo (NomadTab spawner registrado, count
 *       de comandos consola post-StartupModule).
 *     - Lifecycle state machine FPGXTutorialRunner.
 *     - End-to-end FPGXTutorialActionExecutor (CreateFolder / CreateAsset
 *       / OpenAsset / NavigateCB / ConfigBasePath todos requieren editor
 *       process con Content Browser real).
 *
 */

// ============================================================================
// EN: Enum default values
// ES: Defaults de enums
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXTutorialsLanguageEnumDefaults,
	"PGX.Tutorials.LanguageEnumDefaults",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXTutorialsLanguageEnumDefaults::RunTest(const FString& /*Parameters*/)
{
	// EN: Default-constructed FPGXTutorialStep does NOT carry an explicit
	//     language hint; language selection is resolved at runtime from
	//     the persisted preference. The enum itself defaults to its
	//     first declared value (English) per C++ enum class semantics.
	// ES: FPGXTutorialStep construido por defecto NO porta un hint de
	//     idioma explicito; la seleccion de idioma se resuelve en runtime
	//     desde la preferencia persistida. El enum mismo defaultea al
	//     primer valor declarado (English) per semantica enum class.
	const EPGXTutorialLanguage Default = static_cast<EPGXTutorialLanguage>(0);
	TestEqual(
		TEXT("EPGXTutorialLanguage first-declared value is English (0)"),
		static_cast<int32>(Default),
		static_cast<int32>(EPGXTutorialLanguage::English));
	TestNotEqual(
		TEXT("EPGXTutorialLanguage::Spanish differs from English"),
		static_cast<int32>(EPGXTutorialLanguage::Spanish),
		static_cast<int32>(EPGXTutorialLanguage::English));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXTutorialsActionEnumDefaults,
	"PGX.Tutorials.ActionEnumDefaults",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXTutorialsActionEnumDefaults::RunTest(const FString& /*Parameters*/)
{
	// EN: First-declared value semantics: EPGXTutorialAction::None = 0,
	//     and the canonical 6-value cardinality (None / CreateFolder /
	//     CreateAsset / OpenAsset / NavigateCB / ConfigBasePath) is
	//     locked by the public enum contract.
	// ES: Semantica de primer-valor-declarado: EPGXTutorialAction::None
	//     = 0, y la cardinalidad canonical 6-valores (None / CreateFolder
	//     / CreateAsset / OpenAsset / NavigateCB / ConfigBasePath) esta
	//     bloqueada por el contrato publico del enum.
	const EPGXTutorialAction Default = static_cast<EPGXTutorialAction>(0);
	TestEqual(
		TEXT("EPGXTutorialAction first-declared value is None (0)"),
		static_cast<int32>(Default),
		static_cast<int32>(EPGXTutorialAction::None));
	TestEqual(
		TEXT("EPGXTutorialAction::ConfigBasePath is the last canonical action (5)"),
		static_cast<int32>(EPGXTutorialAction::ConfigBasePath),
		5);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXTutorialsAgentTypeEnumDefaults,
	"PGX.Tutorials.AgentTypeEnumDefaults",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXTutorialsAgentTypeEnumDefaults::RunTest(const FString& /*Parameters*/)
{
	// EN: EPGXTutorialAgentType has 2 canonical values: Guide
	//     (display-only) and Constructor (creates folders / assets).
	//     First-declared = Guide.
	// ES: EPGXTutorialAgentType tiene 2 valores canonicos: Guide (solo
	//     display) y Constructor (crea carpetas / assets).
	//     Primero-declarado = Guide.
	const EPGXTutorialAgentType Default = static_cast<EPGXTutorialAgentType>(0);
	TestEqual(
		TEXT("EPGXTutorialAgentType first-declared value is Guide (0)"),
		static_cast<int32>(Default),
		static_cast<int32>(EPGXTutorialAgentType::Guide));
	TestNotEqual(
		TEXT("EPGXTutorialAgentType::Constructor differs from Guide"),
		static_cast<int32>(EPGXTutorialAgentType::Constructor),
		static_cast<int32>(EPGXTutorialAgentType::Guide));
	return true;
}

// ============================================================================
// EN: Struct default construction
// ES: Construccion default de structs
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXTutorialsActionResultDefaults,
	"PGX.Tutorials.ActionResultDefaults",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXTutorialsActionResultDefaults::RunTest(const FString& /*Parameters*/)
{
	// EN: Default-constructed FPGXTutorialActionResult must signal
	//     failure (bSuccess = false) so that an unset-on-purpose action
	//     result cannot be confused with a successful execution. Empty
	//     FeedbackText follows.
	// ES: FPGXTutorialActionResult construido por defecto debe señalizar
	//     fallo (bSuccess = false) para que un resultado de accion
	//     no-set-on-purpose no pueda confundirse con una ejecucion
	//     exitosa. FeedbackText vacio sigue.
	const FPGXTutorialActionResult Default;
	TestFalse(TEXT("Default FPGXTutorialActionResult bSuccess must be false"), Default.bSuccess);
	TestTrue(TEXT("Default FPGXTutorialActionResult FeedbackText must be empty"), Default.FeedbackText.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXTutorialsStepDefaults,
	"PGX.Tutorials.StepDefaults",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXTutorialsStepDefaults::RunTest(const FString& /*Parameters*/)
{
	// EN: Default-constructed FPGXTutorialStep represents a callout-
	//     centered display-only step with no target tab and no action.
	//     The accent color default is the documented "system blue"
	//     (R=0.3 / G=0.6 / B=0.9 / A=1.0) used when no per-system
	//     accent override applies.
	// ES: FPGXTutorialStep construido por defecto representa un paso
	//     callout-centrado solo-display sin tab target y sin accion.
	//     El default del accent color es el "azul sistema" documentado
	//     (R=0.3 / G=0.6 / B=0.9 / A=1.0) usado cuando no aplica
	//     override de accent per-sistema.
	const FPGXTutorialStep Default;
	TestEqual(TEXT("Default Step TargetTabId is NAME_None"), Default.TargetTabId, FName(NAME_None));
	TestTrue(TEXT("Default Step bOpenTab is true"), Default.bOpenTab);
	TestEqual(
		TEXT("Default Step Action is None"),
		static_cast<int32>(Default.Action),
		static_cast<int32>(EPGXTutorialAction::None));
	TestTrue(TEXT("Default Step ActionPath empty"), Default.ActionPath.IsEmpty());
	TestTrue(TEXT("Default Step AssetClass empty"), Default.AssetClass.IsEmpty());
	TestTrue(TEXT("Default Step AssetName empty"), Default.AssetName.IsEmpty());

	// EN: Accent color sanity — non-zero RGB so a non-set accent is
	//     not visually identical to "no highlight" in the overlay.
	//     A=1.0 ensures full opacity.
	// ES: Sanity del accent color — RGB no-cero para que un accent
	//     no-set no sea visualmente identico a "sin highlight" en el
	//     overlay. A=1.0 asegura opacidad total.
	const float TotalRGB = Default.AccentColor.R + Default.AccentColor.G + Default.AccentColor.B;
	TestTrue(TEXT("Default Step AccentColor RGB sum > 0"), TotalRGB > 0.0f);
	TestEqual(TEXT("Default Step AccentColor alpha is 1.0"), Default.AccentColor.A, 1.0f);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
