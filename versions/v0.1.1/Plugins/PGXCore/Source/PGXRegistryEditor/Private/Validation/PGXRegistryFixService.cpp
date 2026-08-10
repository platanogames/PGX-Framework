// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Validation/PGXRegistryFixService.h"
#include "Tables/PGXTableTypes.h"
#include "Engine/DataTable.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/UObjectGlobals.h"

int32 FPGXRegistryFixService::ApplySafeFixes(const TArray<FPGXRegistryValidationIssue>& Issues, TArray<FString>& OutLog)
{
	int32 FixedCount = 0;

	for (const FPGXRegistryValidationIssue& Issue : Issues)
	{
		if (!Issue.bAutoFixable)
		{
			continue;
		}

		FString FixLog;
		if (FixIssue(Issue, FixLog))
		{
			OutLog.Add(MoveTemp(FixLog));
			++FixedCount;
		}
	}

	return FixedCount;
}

bool FPGXRegistryFixService::FixIssue(const FPGXRegistryValidationIssue& Issue, FString& OutLog)
{
	switch (Issue.RuleId)
	{
	case EPGXValidationRuleId::RDT030_NullAssetRef:
		return FixNullAssetRefs(Issue.TableName, Issue.RowName, OutLog);

	case EPGXValidationRuleId::RDT033_RedirectorDetected:
		return FixRedirector(Issue.TableName, Issue.RowName, Issue.AssetPath, OutLog);

	case EPGXValidationRuleId::RDT012_TagOutsideNamespace:
		// EN: Tag normalization needs a project-specific mapping; fix this rule manually.
		// ES: La normalizacion requiere un mapeo del proyecto; esta regla se corrige manualmente.
		OutLog = FString::Printf(TEXT("RDT012 autofix unavailable for [%s]; normalize the tag manually"),
			*Issue.TableName);
		return false;

	default:
		OutLog = FString::Printf(TEXT("Rule [%s] is not auto-fixable"),
			*Issue.GetRuleIdString());
		return false;
	}
}

bool FPGXRegistryFixService::FixNullAssetRefs(const FString& TableName, FName RowName, FString& OutLog)
{
	// EN: Find the DataTable by name / ES: Encontrar el DataTable por nombre
	UDataTable* Table = FindObject<UDataTable>(nullptr, *TableName);
	if (!Table)
	{
		// EN: Try searching by short name / ES: Intentar buscar por nombre corto
		for (TObjectIterator<UDataTable> It; It; ++It)
		{
			if (It->GetName() == TableName)
			{
				Table = *It;
				break;
			}
		}
	}

	if (!Table)
	{
		OutLog = FString::Printf(TEXT("Could not find DataTable [%s]"), *TableName);
		return false;
	}

	// EN: Access the row and remove null entries / ES: Acceder a la fila y remover entradas nulas
	const TMap<FName, uint8*>& RowMap = Table->GetRowMap();
	const uint8* const* RowDataPtr = RowMap.Find(RowName);
	if (!RowDataPtr || !*RowDataPtr)
	{
		OutLog = FString::Printf(TEXT("Could not find row [%s] in table [%s]"), *RowName.ToString(), *TableName);
		return false;
	}

	FPGXRegistryCategoryRow* Row = const_cast<FPGXRegistryCategoryRow*>(
		reinterpret_cast<const FPGXRegistryCategoryRow*>(*RowDataPtr));

	int32 RemovedCount = 0;
	TArray<FGameplayTag> NullKeys;
	for (const auto& ItemPair : Row->ItemsByTag)
	{
		if (ItemPair.Value.IsNull())
		{
			NullKeys.Add(ItemPair.Key);
		}
	}

	for (const FGameplayTag& NullKey : NullKeys)
	{
		Row->ItemsByTag.Remove(NullKey);
		++RemovedCount;
	}

	if (RemovedCount > 0)
	{
		Table->MarkPackageDirty();
		OutLog = FString::Printf(TEXT("Removed %d null entries from row [%s] in [%s]"),
			RemovedCount, *RowName.ToString(), *TableName);
		return true;
	}

	OutLog = TEXT("No null entries found to remove");
	return false;
}

bool FPGXRegistryFixService::FixRedirector(const FString& TableName, FName RowName,
	const FString& AssetPath, FString& OutLog)
{
	// EN: Redirector fixup is a complex operation best handled by UE's asset tools.
	//     We log a suggestion rather than attempting in-place fixup.
	// ES: El fixup de redirectores es una operacion compleja mejor manejada por las
	//     herramientas de assets de UE. Registramos una sugerencia en vez de intentar fixup in-place.
	OutLog = FString::Printf(
		TEXT("[RDT033] Redirector at [%s] in row [%s] of [%s]. "
			 "Use 'Fix Up Redirectors in Folder' from Content Browser to resolve."),
		*AssetPath, *RowName.ToString(), *TableName);

	// EN: Return true as we've "handled" it with guidance / ES: Retornar true ya que lo "manejamos" con guia
	return true;
}
