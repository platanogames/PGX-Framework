// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXCommitTagger.h"
#include "PGXVersionControlSettings.h"

FPGXCommitTagger::FPGXCommitTagger()
{
	InitializeSystemMap();
}

void FPGXCommitTagger::InitializeSystemMap()
{
	// EN: Plugin directory fragments mapped to display names
	// ES: Fragmentos de directorio de plugins mapeados a nombres para mostrar
	SystemMap.Add(TEXT("PGXAudio"),          TEXT("Audio"));
	SystemMap.Add(TEXT("PGXSave"),           TEXT("Save"));
	SystemMap.Add(TEXT("PGXGameFlow"),       TEXT("GameFlow"));
	SystemMap.Add(TEXT("PGXPSO"),            TEXT("PSO"));
	SystemMap.Add(TEXT("PGXLoading"),        TEXT("Loading"));
	SystemMap.Add(TEXT("PGXLevelFlow"),      TEXT("LevelFlow"));
	SystemMap.Add(TEXT("PGXInput"),          TEXT("Input"));
	SystemMap.Add(TEXT("PGXCamera"),         TEXT("Camera"));
	SystemMap.Add(TEXT("PGXUI"),             TEXT("UI"));
	SystemMap.Add(TEXT("PGXInteraction"),    TEXT("Interaction"));
	SystemMap.Add(TEXT("PGXInventory"),      TEXT("Inventory"));
	SystemMap.Add(TEXT("PGXAbility"),        TEXT("Ability"));
	SystemMap.Add(TEXT("PGXSpawn"),          TEXT("Spawn"));
	SystemMap.Add(TEXT("PGXAI"),             TEXT("AI"));
	SystemMap.Add(TEXT("PGXOnline"),         TEXT("Online"));
	SystemMap.Add(TEXT("PGXMaterials"),      TEXT("Materials"));
	SystemMap.Add(TEXT("PGXVFX"),            TEXT("VFX"));
	SystemMap.Add(TEXT("PGXAnimation"),      TEXT("Animation"));
	SystemMap.Add(TEXT("PGXMultiplayer"),    TEXT("Multiplayer"));
	SystemMap.Add(TEXT("PGXCinematic"),      TEXT("Cinematic"));
	SystemMap.Add(TEXT("PGXMGOS"),           TEXT("MGOS"));
	SystemMap.Add(TEXT("PGXDocs"),           TEXT("Docs"));
	SystemMap.Add(TEXT("PGXEditorTools"),    TEXT("EditorTools"));
	SystemMap.Add(TEXT("PGXVersionControl"), TEXT("VersionControl"));
	SystemMap.Add(TEXT("PGXCore"),           TEXT("Core"));

	// EN: Public project directories / ES: Directorios publicos del proyecto
	SystemMap.Add(TEXT("Docs/"),             TEXT("Docs"));
}

TArray<FString> FPGXCommitTagger::DetectSystems(const TArray<FString>& InFilePaths) const
{
	TSet<FString> DetectedSet;

	for (const FString& Path : InFilePaths)
	{
		for (const auto& Pair : SystemMap)
		{
			if (Path.Contains(Pair.Key))
			{
				DetectedSet.Add(Pair.Value);
			}
		}
	}

	TArray<FString> Result = DetectedSet.Array();
	Result.Sort();
	return Result;
}

FString FPGXCommitTagger::BuildCommitPrefix(const TArray<FString>& InTags) const
{
	if (InTags.IsEmpty()) return FString();

	FString Prefix;
	for (const FString& Tag : InTags)
	{
		Prefix += FString::Printf(TEXT("[%s]"), *Tag);
	}
	return Prefix;
}

FString FPGXCommitTagger::GetCommitTemplate(const FString& InChangelistName, const TArray<FString>& InTags) const
{
	const UPGXVersionControlSettings* Settings = GetDefault<UPGXVersionControlSettings>();
	FString Template = Settings->CommitMessageTemplate;

	const FString Prefix = BuildCommitPrefix(InTags);
	Template.ReplaceInline(TEXT("{PREFIX}"), *Prefix);
	Template.ReplaceInline(TEXT("{NAME}"), *InChangelistName);
	Template.ReplaceInline(TEXT("{DESC}"), TEXT(""));

	return Template;
}
