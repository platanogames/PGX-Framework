// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once

#include "CoreMinimal.h"
#include "Framework/Docking/TabManager.h"
#include "Textures/SlateIcon.h"

class FWorkspaceItem;

namespace PGX::Editor
{
	/**
	 * EN: Low-level central wrapper for PGX NomadTab registration. Keeps existing chained customisation intact.
	 * ES: Wrapper central de bajo nivel para registro de NomadTab PGX. Conserva la customizacion encadenada existente.
	 */
	PGXCOREEDITOR_API FTabSpawnerEntry& RegisterNomadTab(
		const FName& TabId,
		const FOnSpawnTab& OnSpawnTab);

	/**
	 * EN: Register a PGX editor nomad tab with the shared defaults used by PGX panels.
	 * ES: Registra un NomadTab de editor PGX con los defaults compartidos por paneles PGX.
	 */
	PGXCOREEDITOR_API FTabSpawnerEntry& RegisterNomadTab(
		const FName& TabId,
		const FOnSpawnTab& OnSpawnTab,
		const FText& DisplayName,
		const FText& TooltipText = FText::GetEmpty(),
		ETabSpawnerMenuType::Type MenuType = ETabSpawnerMenuType::Enabled,
		TSharedPtr<FWorkspaceItem> Group = nullptr,
		const FSlateIcon& Icon = FSlateIcon());

	/**
	 * EN: Safe unregister wrapper. UE tolerates missing entries poorly in some flows; guard centrally.
	 * ES: Wrapper seguro de unregister. UE tolera mal entradas ausentes en algunos flujos; guard central.
	 */
	PGXCOREEDITOR_API void UnregisterNomadTab(const FName& TabId);
}
