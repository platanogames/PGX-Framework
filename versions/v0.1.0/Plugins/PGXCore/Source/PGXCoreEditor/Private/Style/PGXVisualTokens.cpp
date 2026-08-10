// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Dynamic system color lookup — checks user settings override first, then falls back to compile-time defaults.
// ES: Lookup dinamico de color de sistema — consulta override de settings primero, luego cae a defaults compile-time.
#include "Style/PGXVisualTokens.h"
#include "Settings/PGXEditorSettings.h"

FLinearColor PGX::System::GetColorByName(FName SystemName)
{
	// EN: Check user settings override first / ES: Consultar override de settings primero
	if (const UPGXEditorSettings* Settings = GetDefault<UPGXEditorSettings>())
	{
		// EN: Member-pointer map to settings fields / ES: Mapa de member-pointers a campos de settings
		static const TMap<FName, FLinearColor UPGXEditorSettings::*> SettingsMap = {
			{ FName(TEXT("Save")),          &UPGXEditorSettings::ColorSave },
			{ FName(TEXT("GameFlow")),      &UPGXEditorSettings::ColorGameFlow },
			{ FName(TEXT("PSO")),           &UPGXEditorSettings::ColorPSO },
			{ FName(TEXT("LevelFlow")),     &UPGXEditorSettings::ColorLevelFlow },
			{ FName(TEXT("Loading")),       &UPGXEditorSettings::ColorLoading },
			{ FName(TEXT("Profile")),       &UPGXEditorSettings::ColorProfile },
			{ FName(TEXT("MGOS")),          &UPGXEditorSettings::ColorMGOS },
			{ FName(TEXT("Documentation")), &UPGXEditorSettings::ColorDocumentation },
			{ FName(TEXT("Audio")),         &UPGXEditorSettings::ColorAudio },
			{ FName(TEXT("Construction")),  &UPGXEditorSettings::ColorConstruction },
			{ FName(TEXT("DataRegistry")),  &UPGXEditorSettings::ColorDataRegistry },
			{ FName(TEXT("Config")),        &UPGXEditorSettings::ColorConfig },
			{ FName(TEXT("Message")),       &UPGXEditorSettings::ColorMessage },
			{ FName(TEXT("EventHandler")),  &UPGXEditorSettings::ColorEventHandler },
			{ FName(TEXT("Log")),           &UPGXEditorSettings::ColorLog },
			{ FName(TEXT("Scaffold")),      &UPGXEditorSettings::ColorScaffold },
		};

		if (const auto* MemberPtr = SettingsMap.Find(SystemName))
		{
			return Settings->**MemberPtr;
		}
	}

	// EN: Fallback to compile-time defaults / ES: Fallback a defaults compile-time
	static const TMap<FName, FLinearColor> Defaults = {
		{ FName(TEXT("Save")),          PGX::System::Save },
		{ FName(TEXT("GameFlow")),      PGX::System::GameFlow },
		{ FName(TEXT("PSO")),           PGX::System::PSO },
		{ FName(TEXT("LevelFlow")),     PGX::System::LevelFlow },
		{ FName(TEXT("Loading")),       PGX::System::Loading },
		{ FName(TEXT("Profile")),       PGX::System::Profile },
		{ FName(TEXT("MGOS")),          PGX::System::MGOS },
		{ FName(TEXT("Documentation")), PGX::System::Documentation },
		{ FName(TEXT("Audio")),         PGX::System::Audio },
		{ FName(TEXT("Construction")),  PGX::System::Construction },
		{ FName(TEXT("DataRegistry")),  PGX::System::DataRegistry },
		{ FName(TEXT("Config")),        PGX::System::Config },
		{ FName(TEXT("Message")),       PGX::System::Message },
		{ FName(TEXT("EventHandler")),  PGX::System::EventHandler },
		{ FName(TEXT("Log")),           PGX::System::Log },
		{ FName(TEXT("Scaffold")),      PGX::System::Scaffold },
	};

	if (const FLinearColor* Found = Defaults.Find(SystemName))
	{
		return *Found;
	}
	return PGX::Semantic::Neutral;
}
