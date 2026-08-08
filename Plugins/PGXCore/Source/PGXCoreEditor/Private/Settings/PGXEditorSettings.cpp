// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Settings/PGXEditorSettings.h"

#define LOCTEXT_NAMESPACE "PGXEditorSettings"

UPGXEditorSettings::UPGXEditorSettings()
{
	DefaultBlueprintPath.Path = TEXT("/Game/Blueprints/PGX");
}

FText UPGXEditorSettings::GetSectionText() const
{
	return LOCTEXT("SectionText", "Editor");
}

FText UPGXEditorSettings::GetSectionDescription() const
{
	return LOCTEXT("SectionDesc", "Configure PGX editor toolbar pins and asset creation defaults.");
}

#undef LOCTEXT_NAMESPACE
