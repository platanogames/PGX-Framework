// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
//
// EN: Visual design tokens — single-file canonical source of truth for all PGX
//     editor visuals (colors, fonts, spacing, motion, geometry). Every PGX
//     plugin should pull tokens from here instead of using raw literals
//     (FLinearColor(...), FMargin(N.0f), inline pixel sizes).
//
//     Canonical namespaces (single file — there is NO separate PGXSemanticTokens.h;
//     the "semantic" status palette lives below as PGX::Semantic):
//
//       Color & surface (FLinearColor):
//         PGX::Surface     — elevation levels (Void/Base/Raised/Elevated/Active)
//         PGX::Border      — white-alpha border variants (Glass/Hover/Focus/Active/Subtle/Default)
//         PGX::Text        — text hierarchy (Primary/Secondary/Muted/OnColor)
//         PGX::Shadow      — card and glow shadows
//         PGX::Overlay     — hover/press modifier overlays
//         PGX::Gradient    — endpoint colors for programmatic gradients
//         PGX::Semantic    — status palette: Good/Warn/Error/Info/Neutral (+ Bg 12% / Border 30% variants)
//         PGX::System      — per-plugin identity colors (Save/GameFlow/PSO/Loading/Profile/MGOS/…)
//
//       Geometry & layout (float):
//         PGX::Spacing     — XS/SM/MD/LG/XL/XXL (base-4px scale)
//         PGX::Height      — canonical widget heights (PanelHeader/StatusDot/TableRow/…)
//         PGX::Radius      — corner radius scale (Small/Medium/Large/XLarge/XXLarge/Pill)
//         PGX::Width       — common fixed widths (accent stripes)
//
//       Type & motion:
//         PGX::Font        — FSlateFontInfo inline factories (PanelTitle/SectionHeader/…)
//         PGX::Motion      — animation durations (Fast/Standard/Emphasis/AlertPulse)
//
// ES: Tokens de diseno visual — fuente unica de verdad en un solo archivo para
//     toda la visualidad del editor PGX (colores, fonts, spacing, motion,
//     geometria). Cada plugin PGX debe consumir tokens desde aqui en lugar
//     de usar literales raw (FLinearColor(...), FMargin(N.0f), tamanos pixel
//     inline).
//
//     Namespaces canonical (archivo unico — NO existe PGXSemanticTokens.h por
//     separado; la paleta "semantic" de estatus vive abajo como PGX::Semantic).
//     Ver enumeracion en bloque EN superior.
//
#pragma once

#include "CoreMinimal.h"
#include "Styling/CoreStyle.h"

namespace PGX
{
	// === SURFACE (elevation levels — neutral grays, elegant dark theme) ===
	// EN: Dark theme elevation scale — neutral grays that don't tire the eyes.
	//     Color accents come from System colors on stripes/chips/text, not backgrounds.
	// ES: Escala de elevacion tema oscuro — grises neutros que no cansan la vista.
	//     Acentos de color vienen de System colors en stripes/chips/texto, no en fondos.
	namespace Surface
	{
		inline constexpr FLinearColor Void     = FLinearColor(0.012f, 0.012f, 0.012f, 1.0f); // ~#030303 deepest
		inline constexpr FLinearColor Base     = FLinearColor(0.022f, 0.022f, 0.022f, 1.0f); // ~#080808 panel bg
		inline constexpr FLinearColor Raised   = FLinearColor(0.040f, 0.040f, 0.040f, 1.0f); // ~#121212 cards/sections
		inline constexpr FLinearColor Elevated = FLinearColor(0.060f, 0.060f, 0.060f, 1.0f); // ~#1C1C1C hover state
		inline constexpr FLinearColor Active   = FLinearColor(0.085f, 0.085f, 0.085f, 1.0f); // ~#262626 selected/active
	}

	// === BORDER (alpha variants — web-aligned naming: Glass, Hover, Focus, Active) ===
	// EN: White-alpha borders matching PGX web: --border-glass, --border-hover
	// ES: Bordes white-alpha alineados con PGX web: --border-glass, --border-hover
	namespace Border
	{
		inline constexpr FLinearColor Glass  = FLinearColor(1.0f, 1.0f, 1.0f, 0.08f); // --border-glass
		inline constexpr FLinearColor Hover  = FLinearColor(1.0f, 1.0f, 1.0f, 0.15f); // --border-hover
		inline constexpr FLinearColor Focus  = FLinearColor(1.0f, 1.0f, 1.0f, 0.20f);
		inline constexpr FLinearColor Active = FLinearColor(1.0f, 1.0f, 1.0f, 0.30f);
		// EN: Legacy aliases — kept for backward compat in existing widgets
		// ES: Aliases legacy — mantenidos para compat con widgets existentes
		inline constexpr FLinearColor Subtle  = FLinearColor(1.0f, 1.0f, 1.0f, 0.04f); // thinner than Glass
		inline constexpr FLinearColor Default = Glass;
	}

	// === TEXT (hierarchy — neutral grays for readability) ===
	// EN: Neutral text colors — clean and readable on dark backgrounds.
	// ES: Colores de texto neutros — limpios y legibles sobre fondos oscuros.
	namespace Text
	{
		inline constexpr FLinearColor Primary   = FLinearColor(0.85f, 0.85f, 0.85f, 1.0f); // ~#E8E8E8 high contrast
		inline constexpr FLinearColor Secondary = FLinearColor(0.55f, 0.55f, 0.55f, 1.0f); // ~#9A9A9A medium
		inline constexpr FLinearColor Muted     = FLinearColor(0.33f, 0.33f, 0.33f, 1.0f); // ~#606060 low emphasis
		inline constexpr FLinearColor OnColor   = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);    // white on accent
	}

	// === SHADOW (card/glow effects) ===
	namespace Shadow
	{
		inline constexpr FLinearColor Card = FLinearColor(0.0f, 0.0f, 0.0f, 0.30f);         // --shadow-card
		inline constexpr FLinearColor Glow = FLinearColor(0.498f, 0.0f, 1.0f, 0.15f);       // --shadow-glow (purple)
	}

	// === OVERLAY (hover/press state modifiers) ===
	namespace Overlay
	{
		inline constexpr FLinearColor HoverLight  = FLinearColor(1.0f, 1.0f, 1.0f, 0.03f);
		inline constexpr FLinearColor HoverMedium = FLinearColor(1.0f, 1.0f, 1.0f, 0.06f);
		inline constexpr FLinearColor Press       = FLinearColor(0.0f, 0.0f, 0.0f, 0.10f);
	}

	// === GRADIENT (endpoint colors — no native gradient in Slate, used programmatically) ===
	namespace Gradient
	{
		inline constexpr FLinearColor PrimaryStart = FLinearColor(0.498f, 0.0f, 1.0f, 1.0f);  // #7f00ff (MGOS purple)
		inline constexpr FLinearColor PrimaryEnd   = FLinearColor(0.0f, 0.737f, 0.831f, 1.0f); // #00bcd4 (PSO cyan)
	}

	// === SEMANTIC (status colors — solid, background 12%, border 30%) ===
	namespace Semantic
	{
		inline constexpr FLinearColor Good    = FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);
		inline constexpr FLinearColor Warn    = FLinearColor(0.85f, 0.75f, 0.1f, 1.0f);
		inline constexpr FLinearColor Error   = FLinearColor(0.8f, 0.2f, 0.2f, 1.0f);
		inline constexpr FLinearColor Info    = FLinearColor(0.3f, 0.6f, 0.9f, 1.0f);
		inline constexpr FLinearColor Neutral = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

		inline constexpr FLinearColor GoodBg    = FLinearColor(0.2f, 0.8f, 0.2f, 0.12f);
		inline constexpr FLinearColor WarnBg    = FLinearColor(0.85f, 0.75f, 0.1f, 0.12f);
		inline constexpr FLinearColor ErrorBg   = FLinearColor(0.8f, 0.2f, 0.2f, 0.12f);
		inline constexpr FLinearColor InfoBg    = FLinearColor(0.3f, 0.6f, 0.9f, 0.12f);
		inline constexpr FLinearColor NeutralBg = FLinearColor(0.5f, 0.5f, 0.5f, 0.12f);

		inline constexpr FLinearColor GoodBorder    = FLinearColor(0.2f, 0.8f, 0.2f, 0.3f);
		inline constexpr FLinearColor WarnBorder    = FLinearColor(0.85f, 0.75f, 0.1f, 0.3f);
		inline constexpr FLinearColor ErrorBorder   = FLinearColor(0.8f, 0.2f, 0.2f, 0.3f);
		inline constexpr FLinearColor InfoBorder    = FLinearColor(0.3f, 0.6f, 0.9f, 0.3f);
		inline constexpr FLinearColor NeutralBorder = FLinearColor(0.5f, 0.5f, 0.5f, 0.3f);
	}

	// === STATE PALETTE (runtime/editor state colors — canonical inspector vocabulary) ===
	// EN: Shared colors for state machines shown in editor inspectors.
	// ES: Colores compartidos para maquinas de estado mostradas en inspectores editor.
	namespace StatePalette
	{
		inline constexpr FLinearColor Idle          = Semantic::Neutral;
		inline constexpr FLinearColor Preparing     = FLinearColor(0.2f, 0.6f, 1.0f, 1.0f); // Blue
		inline constexpr FLinearColor Loading       = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f); // Yellow
		inline constexpr FLinearColor Active        = Loading;
		inline constexpr FLinearColor FadingIn      = FLinearColor(0.0f, 0.8f, 0.9f, 1.0f); // Cyan
		inline constexpr FLinearColor Waiting       = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange
		inline constexpr FLinearColor Transitioning = Waiting;
		inline constexpr FLinearColor Paused        = Waiting;
		inline constexpr FLinearColor PostLoad      = FLinearColor(0.6f, 0.4f, 0.9f, 1.0f); // Purple
		inline constexpr FLinearColor Complete      = Semantic::Good;
		inline constexpr FLinearColor Failed        = Semantic::Error;
		inline constexpr FLinearColor Unknown       = Semantic::Neutral;
	}

	// === SYSTEM (canonical identity colors) ===
namespace System
{
	inline constexpr FLinearColor Simulation    = FLinearColor(0.302f, 0.8f, 0.502f, 1.0f);
		inline constexpr FLinearColor Save          = FLinearColor(0.298f, 0.686f, 0.314f, 1.0f);
		inline constexpr FLinearColor GameFlow      = FLinearColor(1.0f, 0.596f, 0.0f, 1.0f);
		inline constexpr FLinearColor PSO           = FLinearColor(0.0f, 0.737f, 0.831f, 1.0f);
		inline constexpr FLinearColor LevelFlow     = FLinearColor(0.129f, 0.588f, 0.953f, 1.0f);
		inline constexpr FLinearColor Loading       = FLinearColor(0.914f, 0.118f, 0.388f, 1.0f);
		inline constexpr FLinearColor Profile       = FLinearColor(1.0f, 0.757f, 0.027f, 1.0f);
		inline constexpr FLinearColor MGOS          = FLinearColor(0.498f, 0.0f, 1.0f, 1.0f);
		inline constexpr FLinearColor Documentation = FLinearColor(0.0f, 0.545f, 0.545f, 1.0f);
		inline constexpr FLinearColor Audio         = FLinearColor(1.0f, 0.596f, 0.0f, 1.0f);
		inline constexpr FLinearColor Construction  = FLinearColor(0.0f, 0.502f, 0.502f, 1.0f);
		inline constexpr FLinearColor DataRegistry  = FLinearColor(0.0f, 0.8f, 1.0f, 1.0f);
		inline constexpr FLinearColor Config        = FLinearColor(0.4f, 0.902f, 0.302f, 1.0f);
		inline constexpr FLinearColor Message       = FLinearColor(0.612f, 0.153f, 0.690f, 1.0f);
		inline constexpr FLinearColor EventHandler  = FLinearColor(0.957f, 0.263f, 0.212f, 1.0f);
		inline constexpr FLinearColor Log           = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);
		inline constexpr FLinearColor Scaffold      = FLinearColor(1.0f, 0.749f, 0.0f, 1.0f); // Amber (255, 191, 0)
		inline constexpr FLinearColor Tutorials     = FLinearColor(0.95f, 0.55f, 0.15f, 1.0f); // Warm orange-amber (242, 140, 38) — educational onboarding motif distinct from Scaffold Amber
		inline constexpr FLinearColor VersionControl = FLinearColor(0.6f, 0.7f, 0.2f, 1.0f);  // Olive (153, 178, 51) — VCS/source-control motif (was GVCSColor anonymous namespace en SPGXVersionControlInspectorTab.cpp:34)

		// EN: Distinct identity colors for Multiplayer, Online, and Trade panels.
		// ES: Colores de identidad distintos para paneles Multiplayer, Online y Trade.
		inline constexpr FLinearColor Multiplayer    = FLinearColor(0.0f, 0.592f, 0.655f, 1.0f);   // Deep-teal (0, 151, 167) — multiplayer/network motif
		inline constexpr FLinearColor Online         = FLinearColor(0.161f, 0.502f, 0.725f, 1.0f); // Vibrant-blue (41, 128, 185) — online services/cloud motif
		inline constexpr FLinearColor Trade          = FLinearColor(0.855f, 0.647f, 0.125f, 1.0f); // Golden-amber (218, 165, 32) — trade/economy motif

		// EN: Neutral placeholders remain until the corresponding visual identities are finalized.
		// ES: Los placeholders neutrales permanecen hasta finalizar sus identidades visuales.
		inline constexpr FLinearColor Ability        = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
		inline constexpr FLinearColor Materials      = FLinearColor(0.6f, 0.45f, 0.85f, 1.0f); // PBR violet — material introspection domain
		inline constexpr FLinearColor Animation      = FLinearColor(0.451f, 0.851f, 0.659f, 1.0f); // Motion-teal (115, 217, 168) — keyframe curve / motion interpolation semantic
		inline constexpr FLinearColor Camera         = FLinearColor(0.953f, 0.749f, 0.353f, 1.0f); // Aperture-amber (243, 191, 90) — lens / iris glint semantic
		inline constexpr FLinearColor Cinematic      = FLinearColor(0.851f, 0.251f, 0.553f, 1.0f); // Clapboard-magenta (217, 64, 141) — narrative / sequencer cue semantic
		inline constexpr FLinearColor VFX            = FLinearColor(1.0f, 0.451f, 0.149f, 1.0f);  // Particle-burst-orange (255, 115, 38) — flame / spark semantic
		inline constexpr FLinearColor Inventory      = FLinearColor(0.651f, 0.502f, 0.302f, 1.0f); // Backpack-leather (166, 128, 77) — carrying capacity semantic
		inline constexpr FLinearColor Interaction   = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

		// EN: Dynamic lookup for Observer/Hub/Dashboard panels
		// ES: Lookup dinamico para paneles Observer/Hub/Dashboard
		PGXCOREEDITOR_API FLinearColor GetColorByName(FName SystemName);
	}

	// === SPACING (base-4px scale) ===
	namespace Spacing
	{
		inline constexpr float XS  = 2.0f;
		inline constexpr float SM  = 4.0f;
		inline constexpr float MD  = 8.0f;
		inline constexpr float LG  = 12.0f;
		inline constexpr float XL  = 16.0f;
		inline constexpr float XXL = 24.0f;
	}

	// === HEIGHT (canonical widget heights) ===
	namespace Height
	{
		inline constexpr float AccentBar   = 2.0f;
		inline constexpr float StatusDot   = 6.0f;
		inline constexpr float HealthDot   = 8.0f;
		inline constexpr float Badge       = 16.0f;
		inline constexpr float ProgressBar = 20.0f;
		inline constexpr float BudgetBar   = 4.0f;
		inline constexpr float TableRow    = 26.0f;
		inline constexpr float TableRowCompact = 22.0f;
		inline constexpr float PanelHeader = 36.0f;
		inline constexpr float TitleBar    = 44.0f;  // Premium title bar height
		inline constexpr float Footer      = 24.0f;
		inline constexpr float IconLarge   = 40.0f;
		inline constexpr float GraphDefault     = 80.0f;
		inline constexpr float GraphCompact     = 60.0f;
		inline constexpr float SparklineDefault = 12.0f;
	}

	// === FONT (inline functions returning FSlateFontInfo — gotcha #11 safe) ===
	namespace Font
	{
		inline FSlateFontInfo PanelTitle()    { return FCoreStyle::GetDefaultFontStyle("Bold", 13); }
		inline FSlateFontInfo SectionHeader() { return FCoreStyle::GetDefaultFontStyle("Bold", 11); }
		inline FSlateFontInfo SubHeader()     { return FCoreStyle::GetDefaultFontStyle("Bold", 10); }
		inline FSlateFontInfo Body()          { return FCoreStyle::GetDefaultFontStyle("Regular", 10); }
		inline FSlateFontInfo BodySmall()     { return FCoreStyle::GetDefaultFontStyle("Regular", 9); }
		inline FSlateFontInfo Badge()         { return FCoreStyle::GetDefaultFontStyle("Bold", 9); }
		inline FSlateFontInfo Caption()       { return FCoreStyle::GetDefaultFontStyle("Regular", 8); }
		inline FSlateFontInfo CaptionBold()   { return FCoreStyle::GetDefaultFontStyle("Bold", 8); }
		inline FSlateFontInfo Mono()          { return FCoreStyle::GetDefaultFontStyle("Mono", 9); }
		inline FSlateFontInfo Hint()          { return FCoreStyle::GetDefaultFontStyle("Italic", 8); }
		inline FSlateFontInfo KPIValue()      { return FCoreStyle::GetDefaultFontStyle("Bold", 18); }
		inline FSlateFontInfo KPILabel()      { return FCoreStyle::GetDefaultFontStyle("Regular", 8); }
	}

	// === MOTION (animation durations in seconds) ===
	namespace Motion
	{
		inline constexpr float Fast       = 0.12f;
		inline constexpr float Standard   = 0.18f;
		inline constexpr float Emphasis   = 0.26f;
		inline constexpr float AlertPulse = 0.6f;
	}

	// === RADIUS (corner radius values — aligned with web --radius-* tokens) ===
	namespace Radius
	{
		inline constexpr float Small   = 4.0f;   // --radius-xs
		inline constexpr float Medium  = 8.0f;   // --radius-sm
		inline constexpr float Large   = 12.0f;  // --radius-md
		inline constexpr float XLarge  = 16.0f;  // --radius-lg
		inline constexpr float XXLarge = 24.0f;  // --radius-xl
		inline constexpr float Pill    = 99.0f;
	}

	// === WIDTH (common fixed widths) ===
	namespace Width
	{
		inline constexpr float AccentStripe    = 4.0f;
		inline constexpr float AccentStripeHover = 2.0f;
		inline constexpr float AccentStripeSelected = 3.0f;
	}
}
