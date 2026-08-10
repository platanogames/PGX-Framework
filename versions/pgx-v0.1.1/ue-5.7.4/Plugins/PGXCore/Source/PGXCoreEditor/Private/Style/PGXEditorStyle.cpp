// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Style/PGXEditorStyle.h"
#include "Style/PGXVisualTokens.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"
#include "Styling/SlateColor.h"
#include "Brushes/SlateColorBrush.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Interfaces/IPluginManager.h"

#define IMAGE_BRUSH_SVG(RelativePath, ...) FSlateVectorImageBrush(StyleInstance->RootToContentDir(RelativePath, TEXT(".svg")), __VA_ARGS__)

TSharedPtr<FSlateStyleSet> FPGXEditorStyle::StyleInstance = nullptr;

static const FName StyleSetName(TEXT("PGXEditorStyle"));

const FName& FPGXEditorStyle::GetStyleSetName()
{
	return StyleSetName;
}

void FPGXEditorStyle::Initialize()
{
	if (StyleInstance.IsValid())
	{
		return;
	}

	StyleInstance = MakeShareable(new FSlateStyleSet(StyleSetName));

	// EN: Resolve the Resources directory of the PGXCore plugin
	// ES: Resolver el directorio Resources del plugin PGXCore
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("PGXCore"));
	if (Plugin.IsValid())
	{
		StyleInstance->SetContentRoot(Plugin->GetBaseDir() / TEXT("Resources"));
	}

	const FVector2D Icon20x20(20.0f, 20.0f);
	const FVector2D Icon40x40(40.0f, 40.0f);

	// EN: Register SVG icons — 22 custom icons from "Icons/N.svg"
	// ES: Registrar iconos SVG — 22 iconos personalizados de "Icons/N.svg"
	// Note: 1.svg skipped (dark fill, not usable on dark toolbar)

	// 2.svg — Dashboard with charts → Hub
	StyleInstance->Set("PGXEditor.Icon.Hub",              new IMAGE_BRUSH_SVG("Icons/2", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.Hub.Large",        new IMAGE_BRUSH_SVG("Icons/2", Icon40x40));

	// 3.svg — Circular refresh arrow → Restart
	StyleInstance->Set("PGXEditor.Icon.Restart",          new IMAGE_BRUSH_SVG("Icons/3", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.Restart.Large",    new IMAGE_BRUSH_SVG("Icons/3", Icon40x40));

	// 4.svg — Code window with </> → Log Viewer
	StyleInstance->Set("PGXEditor.Icon.LogViewer",        new IMAGE_BRUSH_SVG("Icons/4", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.LogViewer.Large",  new IMAGE_BRUSH_SVG("Icons/4", Icon40x40));

	// 5.svg — Floppy disk → Save Inspector
	StyleInstance->Set("PGXEditor.Icon.SaveInspector",        new IMAGE_BRUSH_SVG("Icons/5", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.SaveInspector.Large",  new IMAGE_BRUSH_SVG("Icons/5", Icon40x40));

	// 6.svg — Connected workflow nodes → GameFlow
	StyleInstance->Set("PGXEditor.Icon.GameFlow",         new IMAGE_BRUSH_SVG("Icons/6", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.GameFlow.Large",   new IMAGE_BRUSH_SVG("Icons/6", Icon40x40));

	// 7.svg — Monitor with eye → System Observer
	StyleInstance->Set("PGXEditor.Icon.SystemObserver",       new IMAGE_BRUSH_SVG("Icons/7", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.SystemObserver.Large", new IMAGE_BRUSH_SVG("Icons/7", Icon40x40));

	// 8.svg — Stacked documents → Docs
	StyleInstance->Set("PGXEditor.Icon.Docs",             new IMAGE_BRUSH_SVG("Icons/8", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.Docs.Large",       new IMAGE_BRUSH_SVG("Icons/8", Icon40x40));

	// 9.svg — Gear/cogwheel → Settings
	StyleInstance->Set("PGXEditor.Icon.Settings",         new IMAGE_BRUSH_SVG("Icons/9", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.Settings.Large",   new IMAGE_BRUSH_SVG("Icons/9", Icon40x40));

	// 10.svg — Checkmark in seal → Validate
	StyleInstance->Set("PGXEditor.Icon.Validate",         new IMAGE_BRUSH_SVG("Icons/10", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.Validate.Large",   new IMAGE_BRUSH_SVG("Icons/10", Icon40x40));

	// 11.svg — Circle with "i" → About
	StyleInstance->Set("PGXEditor.Icon.About",            new IMAGE_BRUSH_SVG("Icons/11", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.About.Large",      new IMAGE_BRUSH_SVG("Icons/11", Icon40x40));

	// 12.svg — Wrench + tools → Tools
	StyleInstance->Set("PGXEditor.Icon.Tools",            new IMAGE_BRUSH_SVG("Icons/12", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.Tools.Large",      new IMAGE_BRUSH_SVG("Icons/12", Icon40x40));

	// 13.svg — Circuit board / system tree → MGOS
	StyleInstance->Set("PGXEditor.Icon.MGOS",             new IMAGE_BRUSH_SVG("Icons/13", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.MGOS.Large",       new IMAGE_BRUSH_SVG("Icons/13", Icon40x40));

	// 14.svg — Person with loading → Loading
	StyleInstance->Set("PGXEditor.Icon.Loading",          new IMAGE_BRUSH_SVG("Icons/14", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.Loading.Large",    new IMAGE_BRUSH_SVG("Icons/14", Icon40x40));

	// 15.svg — Radial spinner → PSO
	StyleInstance->Set("PGXEditor.Icon.PSO",              new IMAGE_BRUSH_SVG("Icons/15", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.PSO.Large",        new IMAGE_BRUSH_SVG("Icons/15", Icon40x40));

	// 16.svg — Speaker/volume → Audio
	StyleInstance->Set("PGXEditor.Icon.Audio",            new IMAGE_BRUSH_SVG("Icons/16", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.Audio.Large",      new IMAGE_BRUSH_SVG("Icons/16", Icon40x40));

	// 17.svg — Database cylinder → Data Registry
	StyleInstance->Set("PGXEditor.Icon.DataRegistry",       new IMAGE_BRUSH_SVG("Icons/17", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.DataRegistry.Large",  new IMAGE_BRUSH_SVG("Icons/17", Icon40x40));

	// 18.svg — Grid dashboard → Config Dashboard
	StyleInstance->Set("PGXEditor.Icon.ConfigDashboard",       new IMAGE_BRUSH_SVG("Icons/18", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.ConfigDashboard.Large", new IMAGE_BRUSH_SVG("Icons/18", Icon40x40));

	// 19.svg — Git branch fork → Version Control
	StyleInstance->Set("PGXEditor.Icon.VersionControl",        new IMAGE_BRUSH_SVG("Icons/19", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.VersionControl.Large",  new IMAGE_BRUSH_SVG("Icons/19", Icon40x40));

	// 30.svg — Clipboard with checkmark + test result lines → Test Dashboard
	// Test Dashboard uses its dedicated Icons/30 asset.
	StyleInstance->Set("PGXEditor.Icon.TestDashboard",        new IMAGE_BRUSH_SVG("Icons/30", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.TestDashboard.Large",  new IMAGE_BRUSH_SVG("Icons/30", Icon40x40));

	// 20.svg — Envelope/message → Message System
	StyleInstance->Set("PGXEditor.Icon.Message",              new IMAGE_BRUSH_SVG("Icons/20", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.Message.Large",        new IMAGE_BRUSH_SVG("Icons/20", Icon40x40));

	// 21.svg — Hexagon/event → Event Handler
	StyleInstance->Set("PGXEditor.Icon.EventHandler",         new IMAGE_BRUSH_SVG("Icons/21", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.EventHandler.Large",   new IMAGE_BRUSH_SVG("Icons/21", Icon40x40));

	// 22.svg — Dashboard/health → Platform Health
	StyleInstance->Set("PGXEditor.Icon.PlatformHealth",         new IMAGE_BRUSH_SVG("Icons/22", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.PlatformHealth.Large",   new IMAGE_BRUSH_SVG("Icons/22", Icon40x40));

	// 23.svg — Beaker with play button → Sim Harness (PSPH)

	// 25.svg — Stacked layers with arrow → LevelFlow
	StyleInstance->Set("PGXEditor.Icon.LevelFlow",              new IMAGE_BRUSH_SVG("Icons/25", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.LevelFlow.Large",        new IMAGE_BRUSH_SVG("Icons/25", Icon40x40));

	// 26.svg — Badge with checkmark → Profile
	StyleInstance->Set("PGXEditor.Icon.Profile",                new IMAGE_BRUSH_SVG("Icons/26", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.Profile.Large",          new IMAGE_BRUSH_SVG("Icons/26", Icon40x40));

	// 27.svg — Grid of components → Visual Showcase
	StyleInstance->Set("PGXEditor.Icon.Showcase",               new IMAGE_BRUSH_SVG("Icons/27", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.Showcase.Large",         new IMAGE_BRUSH_SVG("Icons/27", Icon40x40));

	// 28.svg — Book with bookmark → Tutorials
	StyleInstance->Set("PGXEditor.Icon.Tutorials",              new IMAGE_BRUSH_SVG("Icons/28", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.Tutorials.Large",        new IMAGE_BRUSH_SVG("Icons/28", Icon40x40));

	// 29.svg — Building blocks → Scaffold
	StyleInstance->Set("PGXEditor.Icon.Scaffold",               new IMAGE_BRUSH_SVG("Icons/29", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.Scaffold.Large",         new IMAGE_BRUSH_SVG("Icons/29", Icon40x40));

	// Panel-entry canonical brushes (allocation table reservation).
	// Initial reuse of the Icons/<N> base brush; the SVG asset can be specialized later.
	// EN: <Plugin>Panel canonical para NomadTab/Hub/Toolbar; <Plugin> legacy mantain otros consumers.
	// ES: <Plugin>Panel canonical para NomadTab/Hub/Toolbar; <Plugin> legacy mantain otros consumers.
	StyleInstance->Set("PGXEditor.Icon.ScaffoldPanel",          new IMAGE_BRUSH_SVG("Icons/29", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.ScaffoldPanel.Large",    new IMAGE_BRUSH_SVG("Icons/29", Icon40x40));
	StyleInstance->Set("PGXEditor.Icon.TutorialsPanel",         new IMAGE_BRUSH_SVG("Icons/28", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.TutorialsPanel.Large",   new IMAGE_BRUSH_SVG("Icons/28", Icon40x40));
	StyleInstance->Set("PGXEditor.Icon.VersionControlPanel",       new IMAGE_BRUSH_SVG("Icons/19", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.VersionControlPanel.Large", new IMAGE_BRUSH_SVG("Icons/19", Icon40x40));

	// EN: Each panel uses a dedicated Icons/<N>.svg asset.
	// ES: Cada panel usa un asset Icons/<N>.svg dedicado.

	// 32.svg — Spark/lightning bolt → PGX Ability (ability activation glyph)
	StyleInstance->Set("PGXEditor.Icon.AbilityPanel",           new IMAGE_BRUSH_SVG("Icons/32", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.AbilityPanel.Large",     new IMAGE_BRUSH_SVG("Icons/32", Icon40x40));

	// 33.svg — Loading spinner + transition arrows → PGX Loading + LevelFlow
	StyleInstance->Set("PGXEditor.Icon.LoadingPanel",           new IMAGE_BRUSH_SVG("Icons/33", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.LoadingPanel.Large",     new IMAGE_BRUSH_SVG("Icons/33", Icon40x40));

	// 34.svg — Material sphere + PBR highlights → PGX Materials
	StyleInstance->Set("PGXEditor.Icon.MaterialsPanel",         new IMAGE_BRUSH_SVG("Icons/34", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.MaterialsPanel.Large",   new IMAGE_BRUSH_SVG("Icons/34", Icon40x40));

	// 23.svg — Beaker with play button -> PGX SimHarness
	StyleInstance->Set("PGXEditor.Icon.SimHarness",             new IMAGE_BRUSH_SVG("Icons/23", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.SimHarness.Large",       new IMAGE_BRUSH_SVG("Icons/23", Icon40x40));
	StyleInstance->Set("PGXEditor.Icon.SimHarnessPanel",        new IMAGE_BRUSH_SVG("Icons/23", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.SimHarnessPanel.Large",  new IMAGE_BRUSH_SVG("Icons/23", Icon40x40));

	// EN: Dedicated panel icons: motion curve, aperture, clapboard, particle burst, and backpack.
	// ES: Iconos de panel dedicados: curva de movimiento, apertura, claqueta, particulas y mochila.
	StyleInstance->Set("PGXEditor.Icon.AnimationPanel",         new IMAGE_BRUSH_SVG("Icons/36", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.AnimationPanel.Large",   new IMAGE_BRUSH_SVG("Icons/36", Icon40x40));
	StyleInstance->Set("PGXEditor.Icon.CameraPanel",            new IMAGE_BRUSH_SVG("Icons/37", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.CameraPanel.Large",      new IMAGE_BRUSH_SVG("Icons/37", Icon40x40));
	StyleInstance->Set("PGXEditor.Icon.CinematicPanel",         new IMAGE_BRUSH_SVG("Icons/38", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.CinematicPanel.Large",   new IMAGE_BRUSH_SVG("Icons/38", Icon40x40));
	StyleInstance->Set("PGXEditor.Icon.VFXPanel",               new IMAGE_BRUSH_SVG("Icons/39", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.VFXPanel.Large",         new IMAGE_BRUSH_SVG("Icons/39", Icon40x40));
	StyleInstance->Set("PGXEditor.Icon.InventoryPanel",         new IMAGE_BRUSH_SVG("Icons/40", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.InventoryPanel.Large",   new IMAGE_BRUSH_SVG("Icons/40", Icon40x40));

	// ─── PLUGIN ICON BRUSHES ───
	// EN: Stable brush keys let Hub cards, toolbar entries, and observer tabs share
	//     the same per-plugin visual identity.
	// ES: Claves de brush estables permiten compartir identidad visual entre Hub,
	//     toolbar y tabs de observacion.
	StyleInstance->Set("PGXEditor.Icon.Environment",            new IMAGE_BRUSH_SVG("Icons/31", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.Environment.Large",      new IMAGE_BRUSH_SVG("Icons/31", Icon40x40));

	StyleInstance->Set("PGXEditor.Icon.Trade",                  new IMAGE_BRUSH_SVG("Icons/50", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.Trade.Large",            new IMAGE_BRUSH_SVG("Icons/50", Icon40x40));

	StyleInstance->Set("PGXEditor.Icon.Vehicles",               new IMAGE_BRUSH_SVG("Icons/52", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.Vehicles.Large",         new IMAGE_BRUSH_SVG("Icons/52", Icon40x40));

	StyleInstance->Set("PGXEditor.Icon.Crafting",               new IMAGE_BRUSH_SVG("Icons/43", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.Crafting.Large",         new IMAGE_BRUSH_SVG("Icons/43", Icon40x40));

	StyleInstance->Set("PGXEditor.Icon.Colony",                 new IMAGE_BRUSH_SVG("Icons/42", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.Colony.Large",           new IMAGE_BRUSH_SVG("Icons/42", Icon40x40));

	StyleInstance->Set("PGXEditor.Icon.Interaction",            new IMAGE_BRUSH_SVG("Icons/45", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.Interaction.Large",      new IMAGE_BRUSH_SVG("Icons/45", Icon40x40));

	StyleInstance->Set("PGXEditor.Icon.Spawn",                  new IMAGE_BRUSH_SVG("Icons/49", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.Spawn.Large",            new IMAGE_BRUSH_SVG("Icons/49", Icon40x40));

	StyleInstance->Set("PGXEditor.Icon.Input",                  new IMAGE_BRUSH_SVG("Icons/44", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.Input.Large",            new IMAGE_BRUSH_SVG("Icons/44", Icon40x40));

	StyleInstance->Set("PGXEditor.Icon.UI",                     new IMAGE_BRUSH_SVG("Icons/51", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.UI.Large",               new IMAGE_BRUSH_SVG("Icons/51", Icon40x40));

	StyleInstance->Set("PGXEditor.Icon.AI",                     new IMAGE_BRUSH_SVG("Icons/41", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.AI.Large",               new IMAGE_BRUSH_SVG("Icons/41", Icon40x40));

	StyleInstance->Set("PGXEditor.Icon.Inventory",              new IMAGE_BRUSH_SVG("Icons/46", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.Inventory.Large",        new IMAGE_BRUSH_SVG("Icons/46", Icon40x40));

	// Note: PGXEditor.Icon.Tutorials (Icons/28), .Scaffold (Icons/29), .VersionControl (Icons/19)
	//       already registered above with their own SVG assets — no scaffold needed for those three.

	// ─── PANEL BRUSH SLOTS ───
	// EN: Distinct from SYSTEM identity brushes (PGXEditor.Icon.<Plugin>), the
	//     PANEL brushes (PGXEditor.Icon.<Plugin>Panel) are used by NomadTab /
	//     toolbar / Hub card entries for the per-plugin observability panel
	//     SCompoundWidget. Interaction and Trade share their system identity icon.
	//     PGXEditor.Icon.PSOPanel reuses Icons/15 (PSO already has dedicated
	//     SVG) — distinct slot from system identity .Icon.PSO.
	// ES: Distintas de las marcas SYSTEM identity. Brushes PANEL los usan las
	//     entradas NomadTab / toolbar / Hub card del panel observability por plugin.
	StyleInstance->Set("PGXEditor.Icon.InteractionPanel",       new IMAGE_BRUSH_SVG("Icons/45", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.InteractionPanel.Large", new IMAGE_BRUSH_SVG("Icons/45", Icon40x40));

	StyleInstance->Set("PGXEditor.Icon.MultiplayerPanel",       new IMAGE_BRUSH_SVG("Icons/47", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.MultiplayerPanel.Large", new IMAGE_BRUSH_SVG("Icons/47", Icon40x40));

	StyleInstance->Set("PGXEditor.Icon.OnlinePanel",            new IMAGE_BRUSH_SVG("Icons/48", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.OnlinePanel.Large",      new IMAGE_BRUSH_SVG("Icons/48", Icon40x40));

	StyleInstance->Set("PGXEditor.Icon.TradePanel",             new IMAGE_BRUSH_SVG("Icons/50", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.TradePanel.Large",       new IMAGE_BRUSH_SVG("Icons/50", Icon40x40));

	StyleInstance->Set("PGXEditor.Icon.PSOPanel",               new IMAGE_BRUSH_SVG("Icons/15", Icon20x20));
	StyleInstance->Set("PGXEditor.Icon.PSOPanel.Large",         new IMAGE_BRUSH_SVG("Icons/15", Icon40x40));

	// ─── PGX TEXT BLOCK STYLES ─── (derived from PGXVisualTokens)
	{
		auto MakeTextStyle = [](const FSlateFontInfo& InFont, const FLinearColor& InColor) -> FTextBlockStyle
		{
			FTextBlockStyle Style = FTextBlockStyle::GetDefault();
			Style.SetFont(InFont);
			Style.SetColorAndOpacity(FSlateColor(InColor));
			return Style;
		};

		StyleInstance->Set("PGXEditor.Font.PanelTitle",    MakeTextStyle(PGX::Font::PanelTitle(),    PGX::Text::Primary));
		StyleInstance->Set("PGXEditor.Font.SectionHeader", MakeTextStyle(PGX::Font::SectionHeader(), PGX::Text::Primary));
		StyleInstance->Set("PGXEditor.Font.SubHeader",     MakeTextStyle(PGX::Font::SubHeader(),     PGX::Text::Primary));
		StyleInstance->Set("PGXEditor.Font.Body",          MakeTextStyle(PGX::Font::Body(),          PGX::Text::Primary));
		StyleInstance->Set("PGXEditor.Font.BodySmall",     MakeTextStyle(PGX::Font::BodySmall(),     PGX::Text::Secondary));
		StyleInstance->Set("PGXEditor.Font.Badge",         MakeTextStyle(PGX::Font::Badge(),         PGX::Text::OnColor));
		StyleInstance->Set("PGXEditor.Font.Caption",       MakeTextStyle(PGX::Font::Caption(),       PGX::Text::Muted));
		StyleInstance->Set("PGXEditor.Font.CaptionBold",   MakeTextStyle(PGX::Font::CaptionBold(),   PGX::Text::Muted));
		StyleInstance->Set("PGXEditor.Font.Mono",          MakeTextStyle(PGX::Font::Mono(),          PGX::Text::Primary));
		StyleInstance->Set("PGXEditor.Font.Hint",          MakeTextStyle(PGX::Font::Hint(),          PGX::Text::Muted));
		StyleInstance->Set("PGXEditor.Font.KPIValue",      MakeTextStyle(PGX::Font::KPIValue(),      PGX::Text::Primary));
		StyleInstance->Set("PGXEditor.Font.KPILabel",      MakeTextStyle(PGX::Font::KPILabel(),      PGX::Text::Muted));
	}

	// ─── PGX SHAPE BRUSHES ─── (procedural, no PNGs)
	{
		// EN: Card background — rounded rect, Surface::Raised
		// ES: Fondo de tarjeta — rect redondeado, Surface::Raised
		StyleInstance->Set("PGXEditor.Shape.Card",
			new FSlateRoundedBoxBrush(PGX::Surface::Raised, PGX::Radius::Medium));

		// EN: Card with border
		StyleInstance->Set("PGXEditor.Shape.CardBordered",
			new FSlateRoundedBoxBrush(PGX::Surface::Raised, PGX::Radius::Medium,
				PGX::Border::Default, 1.0f));

		// EN: Pill badge shape
		StyleInstance->Set("PGXEditor.Shape.Pill",
			new FSlateRoundedBoxBrush(PGX::Surface::Elevated, PGX::Radius::Pill));

		// EN: Status dot (tinted at runtime via ColorAndOpacity)
		StyleInstance->Set("PGXEditor.Shape.StatusDot",
			new FSlateRoundedBoxBrush(FLinearColor::White, PGX::Radius::Pill));

		// EN: Flat solid for accent bars and separators
		StyleInstance->Set("PGXEditor.Shape.Flat",
			new FSlateColorBrush(FLinearColor::White));

		// EN: Separator line
		StyleInstance->Set("PGXEditor.Shape.Separator",
			new FSlateColorBrush(PGX::Border::Subtle));

		// ─── Premium Visual Upgrade v2 — Rounded containers & cards ───

		// EN: Premium shell background — covers UE chrome in docked mode
		// ES: Fondo del shell premium — cubre el chrome de UE en modo docked
		StyleInstance->Set("PGXEditor.Premium.Shell",
			new FSlateColorBrush(PGX::Surface::Void));

		// EN: Premium title bar background
		StyleInstance->Set("PGXEditor.Premium.TitleBar",
			new FSlateRoundedBoxBrush(PGX::Surface::Base, 0.0f));

		// EN: Premium card — rest state (Raised bg, Glass border)
		StyleInstance->Set("PGXEditor.Premium.Card",
			new FSlateRoundedBoxBrush(PGX::Surface::Raised, PGX::Radius::Large,
				PGX::Border::Glass, 1.0f));

		// EN: Premium card — hover state (Elevated bg, Hover border)
		StyleInstance->Set("PGXEditor.Premium.CardHover",
			new FSlateRoundedBoxBrush(PGX::Surface::Elevated, PGX::Radius::Large,
				PGX::Border::Hover, 1.0f));

		// EN: Premium section container — Base bg with Glass border
		StyleInstance->Set("PGXEditor.Premium.Section",
			new FSlateRoundedBoxBrush(PGX::Surface::Base, PGX::Radius::Medium,
				PGX::Border::Glass, 1.0f));

		// EN: Premium section header — Raised bg
		StyleInstance->Set("PGXEditor.Premium.SectionHeader",
			new FSlateRoundedBoxBrush(PGX::Surface::Raised, PGX::Radius::Medium));

		// EN: Premium input field — Void bg with Glass border
		StyleInstance->Set("PGXEditor.Premium.InputField",
			new FSlateRoundedBoxBrush(PGX::Surface::Void, PGX::Radius::Small,
				PGX::Border::Glass, 1.0f));

		// EN: Premium shadow simulation (subtle dark underlay)
		StyleInstance->Set("PGXEditor.Premium.Shadow",
			new FSlateRoundedBoxBrush(PGX::Shadow::Card, PGX::Radius::Large + 2.0f));

		// ─── Premium Button Brushes (per-state, for FButtonStyle construction) ───

		// EN: Primary button — subtle filled with accent-level border
		StyleInstance->Set("PGXEditor.Button.Primary.Normal",
			new FSlateRoundedBoxBrush(PGX::Surface::Raised, PGX::Radius::Medium,
				PGX::Border::Glass, 1.0f));
		StyleInstance->Set("PGXEditor.Button.Primary.Hovered",
			new FSlateRoundedBoxBrush(PGX::Surface::Elevated, PGX::Radius::Medium,
				PGX::Border::Hover, 1.0f));
		StyleInstance->Set("PGXEditor.Button.Primary.Pressed",
			new FSlateRoundedBoxBrush(PGX::Surface::Base, PGX::Radius::Medium,
				PGX::Border::Active, 1.0f));

		// EN: Secondary button — transparent with subtle border
		StyleInstance->Set("PGXEditor.Button.Secondary.Normal",
			new FSlateRoundedBoxBrush(FLinearColor::Transparent, PGX::Radius::Medium,
				PGX::Border::Glass, 1.0f));
		StyleInstance->Set("PGXEditor.Button.Secondary.Hovered",
			new FSlateRoundedBoxBrush(PGX::Overlay::HoverLight, PGX::Radius::Medium,
				PGX::Border::Hover, 1.0f));
		StyleInstance->Set("PGXEditor.Button.Secondary.Pressed",
			new FSlateRoundedBoxBrush(PGX::Overlay::Press, PGX::Radius::Medium,
				PGX::Border::Active, 1.0f));

		// EN: Ghost button — no border, no bg, hover only
		StyleInstance->Set("PGXEditor.Button.Ghost.Normal",
			new FSlateRoundedBoxBrush(FLinearColor::Transparent, PGX::Radius::Medium));
		StyleInstance->Set("PGXEditor.Button.Ghost.Hovered",
			new FSlateRoundedBoxBrush(PGX::Overlay::HoverLight, PGX::Radius::Medium));
		StyleInstance->Set("PGXEditor.Button.Ghost.Pressed",
			new FSlateRoundedBoxBrush(PGX::Overlay::Press, PGX::Radius::Medium));
	}

	// ─── PGX BUTTON STYLES ─── (FButtonStyle using Premium brushes)
	{
		auto MakeButtonStyle = [&](const FName& Prefix) -> FButtonStyle
		{
			FButtonStyle Style;
			Style.SetNormal(*StyleInstance->GetBrush(FName(*(Prefix.ToString() + TEXT(".Normal")))));
			Style.SetHovered(*StyleInstance->GetBrush(FName(*(Prefix.ToString() + TEXT(".Hovered")))));
			Style.SetPressed(*StyleInstance->GetBrush(FName(*(Prefix.ToString() + TEXT(".Pressed")))));
			Style.SetNormalForeground(FSlateColor(PGX::Text::Primary));
			Style.SetHoveredForeground(FSlateColor(PGX::Text::Primary));
			Style.SetPressedForeground(FSlateColor(PGX::Text::Secondary));
			Style.SetNormalPadding(FMargin(PGX::Spacing::LG, PGX::Spacing::SM));
			Style.SetPressedPadding(FMargin(PGX::Spacing::LG, PGX::Spacing::SM));
			return Style;
		};

		StyleInstance->Set("PGXEditor.Button.Primary",   MakeButtonStyle(FName(TEXT("PGXEditor.Button.Primary"))));
		StyleInstance->Set("PGXEditor.Button.Secondary",  MakeButtonStyle(FName(TEXT("PGXEditor.Button.Secondary"))));
		StyleInstance->Set("PGXEditor.Button.Ghost",      MakeButtonStyle(FName(TEXT("PGXEditor.Button.Ghost"))));
	}

	FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
}

void FPGXEditorStyle::Shutdown()
{
	if (StyleInstance.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
		StyleInstance.Reset();
	}
}

const ISlateStyle& FPGXEditorStyle::Get()
{
	return *StyleInstance;
}

#undef IMAGE_BRUSH_SVG
