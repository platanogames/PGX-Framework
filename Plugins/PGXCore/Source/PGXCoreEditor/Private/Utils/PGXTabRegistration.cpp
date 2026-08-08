// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Utils/PGXTabRegistration.h"

namespace PGX::Editor
{
	FTabSpawnerEntry& RegisterNomadTab(
		const FName& TabId,
		const FOnSpawnTab& OnSpawnTab)
	{
		return FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TabId, OnSpawnTab);
	}

	FTabSpawnerEntry& RegisterNomadTab(
		const FName& TabId,
		const FOnSpawnTab& OnSpawnTab,
		const FText& DisplayName,
		const FText& TooltipText,
		ETabSpawnerMenuType::Type MenuType,
		TSharedPtr<FWorkspaceItem> Group,
		const FSlateIcon& Icon)
	{
		FTabSpawnerEntry& Entry = RegisterNomadTab(TabId, OnSpawnTab)
			.SetDisplayName(DisplayName)
			.SetTooltipText(TooltipText)
			.SetMenuType(MenuType)
			.SetIcon(Icon);

		if (Group.IsValid())
		{
			Entry.SetGroup(Group.ToSharedRef());
		}

		return Entry;
	}

	void UnregisterNomadTab(const FName& TabId)
	{
		if (FGlobalTabmanager::Get()->HasTabSpawner(TabId))
		{
			FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabId);
		}
	}
}
