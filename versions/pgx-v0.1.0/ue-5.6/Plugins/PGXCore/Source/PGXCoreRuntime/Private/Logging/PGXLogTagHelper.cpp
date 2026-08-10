// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Logging/PGXLogTagHelper.h"
#include "Logging/PGXLogTags.h"

// EN: Static helper implementation for Log↔Tag↔Color mappings
// ES: Implementacion del helper estatico para mapeos Log↔Tag↔Color

FGameplayTag FPGXLogTagHelper::GetVerbosityTag(EPGXLogVerbosity Verbosity)
{
	switch (Verbosity)
	{
	case EPGXLogVerbosity::Verbose: return TAG_PGX_Log_Verbose;
	case EPGXLogVerbosity::Debug:   return TAG_PGX_Log_Debug;
	case EPGXLogVerbosity::Info:    return TAG_PGX_Log_Info;
	case EPGXLogVerbosity::Warning: return TAG_PGX_Log_Warning;
	case EPGXLogVerbosity::Error:   return TAG_PGX_Log_Error;
	case EPGXLogVerbosity::Fatal:   return TAG_PGX_Log_Fatal;
	default:                        return FGameplayTag();
	}
}

FGameplayTag FPGXLogTagHelper::GetContextTag(FName Category)
{
	// EN: Map known categories to context tags. New categories default to Core.
	// ES: Mapear categorias conocidas a tags de contexto. Categorias nuevas default a Core.
	const FString CatStr = Category.ToString();

	if (CatStr.Contains(TEXT("Save")) || CatStr.Contains(TEXT("Data")))
	{
		return TAG_PGX_Log_Context_IO;
	}
	if (CatStr.Contains(TEXT("Editor")))
	{
		return TAG_PGX_Log_Context_Editor;
	}
	if (CatStr.Contains(TEXT("Network")) || CatStr.Contains(TEXT("Net")))
	{
		return TAG_PGX_Log_Context_Network;
	}
	if (CatStr.Contains(TEXT("Gameplay")) || CatStr.Contains(TEXT("StateMachine"))
		|| CatStr.Contains(TEXT("Pool")) || CatStr.Contains(TEXT("Spawn")))
	{
		return TAG_PGX_Log_Context_Gameplay;
	}

	// EN: Default: Core context / ES: Default: contexto Core
	return TAG_PGX_Log_Context_Core;
}

FLinearColor FPGXLogTagHelper::GetVerbosityColor(EPGXLogVerbosity Verbosity)
{
	switch (Verbosity)
	{
	case EPGXLogVerbosity::Verbose: return FLinearColor(0.5f, 0.5f, 0.5f);     // Gray
	case EPGXLogVerbosity::Debug:   return FLinearColor(0.0f, 0.784f, 0.784f);  // Cyan
	case EPGXLogVerbosity::Info:    return FLinearColor::White;                   // White
	case EPGXLogVerbosity::Warning: return FLinearColor::Yellow;                  // Yellow
	case EPGXLogVerbosity::Error:   return FLinearColor(1.0f, 0.314f, 0.314f);  // Red
	case EPGXLogVerbosity::Fatal:   return FLinearColor(1.0f, 0.0f, 1.0f);      // Magenta
	default:                        return FLinearColor::White;
	}
}

void FPGXLogTagHelper::AutoPopulateTags(FPGXLogEntry& Entry)
{
	// EN: Add verbosity tag / ES: Agregar tag de verbosity
	const FGameplayTag VerbTag = GetVerbosityTag(Entry.Verbosity);
	if (VerbTag.IsValid())
	{
		Entry.Tags.AddTag(VerbTag);
	}

	// EN: Add context tag based on category / ES: Agregar tag de contexto basado en categoria
	const FGameplayTag CtxTag = GetContextTag(Entry.Category);
	if (CtxTag.IsValid())
	{
		Entry.Tags.AddTag(CtxTag);
	}
}

FGameplayTag FPGXLogTagHelper::AutoInferDomainTag(FName Category)
{
	// EN: Built-in category→domain map. Matches category name substrings.
	// ES: Mapa built-in de categoria→dominio. Coincide substrings de nombre de categoria.
	const FString CatStr = Category.ToString();

	struct FDomainMapping { const TCHAR* Substring; const TCHAR* TagStr; };
	static const FDomainMapping Mappings[] =
	{
		{ TEXT("Audio"),        TEXT("PGX.Log.Domain.Audio") },
		{ TEXT("Save"),         TEXT("PGX.Log.Domain.Save") },
		{ TEXT("GameFlow"),     TEXT("PGX.Log.Domain.GameFlow") },
		{ TEXT("PSO"),          TEXT("PGX.Log.Domain.PSO") },
		{ TEXT("LevelFlow"),    TEXT("PGX.Log.Domain.LevelFlow") },
		{ TEXT("Loading"),      TEXT("PGX.Log.Domain.Loading") },
		{ TEXT("Profile"),      TEXT("PGX.Log.Domain.Profile") },
		{ TEXT("MGOS"),         TEXT("PGX.Log.Domain.MGOS") },
		{ TEXT("Registry"),     TEXT("PGX.Log.Domain.DataRegistry") },
		{ TEXT("Construction"), TEXT("PGX.Log.Domain.Construction") },
		{ TEXT("Message"),      TEXT("PGX.Log.Domain.Message") },
		{ TEXT("EventHandler"), TEXT("PGX.Log.Domain.EventHandler") },
	};

	for (const FDomainMapping& M : Mappings)
	{
		if (CatStr.Contains(M.Substring))
		{
			return FGameplayTag::RequestGameplayTag(FName(M.TagStr), false);
		}
	}

	return FGameplayTag();
}

FLinearColor FPGXLogTagHelper::GetDomainColor(const FGameplayTag& DomainTag)
{
	if (!DomainTag.IsValid())
	{
		return FLinearColor(0.5f, 0.5f, 0.5f);
	}

	const FString TagStr = DomainTag.ToString();

	if (TagStr.Contains(TEXT("Audio")))        return FLinearColor(1.0f, 0.596f, 0.0f);
	if (TagStr.Contains(TEXT("Save")))         return FLinearColor(0.298f, 0.686f, 0.314f);
	if (TagStr.Contains(TEXT("GameFlow")))     return FLinearColor(1.0f, 0.596f, 0.0f);
	if (TagStr.Contains(TEXT("PSO")))          return FLinearColor(0.0f, 0.737f, 0.831f);
	if (TagStr.Contains(TEXT("LevelFlow")))    return FLinearColor(0.129f, 0.588f, 0.953f);
	if (TagStr.Contains(TEXT("Loading")))      return FLinearColor(0.914f, 0.118f, 0.388f);
	if (TagStr.Contains(TEXT("Profile")))      return FLinearColor(1.0f, 0.757f, 0.027f);
	if (TagStr.Contains(TEXT("MGOS")))         return FLinearColor(0.498f, 0.0f, 1.0f);
	if (TagStr.Contains(TEXT("DataRegistry"))) return FLinearColor(0.0f, 0.8f, 1.0f);
	if (TagStr.Contains(TEXT("Construction"))) return FLinearColor(0.0f, 0.502f, 0.502f);
	if (TagStr.Contains(TEXT("Message")))      return FLinearColor(0.706f, 0.314f, 0.863f);
	if (TagStr.Contains(TEXT("EventHandler"))) return FLinearColor(0.863f, 0.235f, 0.235f);

	return FLinearColor(0.5f, 0.5f, 0.5f);
}
