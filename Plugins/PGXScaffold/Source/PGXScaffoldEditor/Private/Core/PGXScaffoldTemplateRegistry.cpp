// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Core/PGXScaffoldTemplateRegistry.h"
#include "PGXScaffoldEditor.h"
#include "Logging/PGXLogMacros.h"

TUniquePtr<FPGXScaffoldTemplateRegistry> FPGXScaffoldTemplateRegistry::Instance;

FPGXScaffoldTemplateRegistry& FPGXScaffoldTemplateRegistry::Get()
{
	if (!Instance.IsValid())
	{
		Instance = MakeUnique<FPGXScaffoldTemplateRegistry>();
	}
	return *Instance;
}

void FPGXScaffoldTemplateRegistry::RegisterTemplate(const FPGXScaffoldTemplate& Template)
{
	// EN: Prevent duplicate registrations / ES: Prevenir registros duplicados
	for (const auto& Existing : Templates)
	{
		if (Existing.TemplateId == Template.TemplateId)
		{
			PGX_LOG_WARNING(LogPGXScaffold, TEXT("FPGXScaffoldTemplateRegistry: Template '%s' already registered, skipping"),
				*Template.TemplateId.ToString());
			return;
		}
	}

	Templates.Add(Template);
	PGX_LOG_INFO(LogPGXScaffold, TEXT("FPGXScaffoldTemplateRegistry: Registered template '%s' (%d items)"),
		*Template.TemplateId.ToString(), Template.Items.Num());
}

const FPGXScaffoldTemplate* FPGXScaffoldTemplateRegistry::FindTemplate(FName TemplateId) const
{
	for (const auto& Template : Templates)
	{
		if (Template.TemplateId == TemplateId)
		{
			return &Template;
		}
	}
	return nullptr;
}

TArray<FName> FPGXScaffoldTemplateRegistry::GetCategories() const
{
	TArray<FName> Categories;
	for (const auto& Template : Templates)
	{
		Categories.AddUnique(Template.Category);
	}
	return Categories;
}

TArray<const FPGXScaffoldTemplate*> FPGXScaffoldTemplateRegistry::GetTemplatesByCategory(FName Category) const
{
	TArray<const FPGXScaffoldTemplate*> Result;
	for (const auto& Template : Templates)
	{
		if (Template.Category == Category)
		{
			Result.Add(&Template);
		}
	}
	return Result;
}
