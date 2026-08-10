// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Validation/PGXRegistryValidationService.h"
#include "Registry/PGXRegistryDefinition.h"
#include "Registry/PGXRegistrySettings.h"
#include "Registry/PGXDataRegistrySubsystem.h"
#include "Tables/PGXTableTypes.h"
#include "Base/PGXDataAsset.h"
#include "Engine/DataTable.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/ObjectRedirector.h"

// ═══════════════════════════════════════════════════════════════
// Public API
// ═══════════════════════════════════════════════════════════════

int32 FPGXRegistryValidationService::ValidateTable(const UDataTable* Table,
	const UPGXRegistryDefinition* Definition, TArray<FPGXRegistryValidationIssue>& OutIssues) const
{
	if (!Table)
	{
		return 0;
	}

	const int32 BeforeCount = OutIssues.Num();
	const UPGXRegistrySettings* Settings = GetDefault<UPGXRegistrySettings>();

	// EN: Schema rules / ES: Reglas de esquema
	RunRDT001_InvalidRowStruct(Table, OutIssues);

	// EN: If row struct is invalid, skip row-level checks / ES: Si el row struct es invalido, saltar checks de fila
	if (Table->GetRowStruct() && Table->GetRowStruct()->IsChildOf(FPGXRegistryCategoryRow::StaticStruct()))
	{
		RunRDT002_MissingCategoryTag(Table, OutIssues);
		RunRDT003_EmptyItemsByTag(Table, OutIssues);

		// EN: Tag integrity / ES: Integridad de tags
		RunRDT010_InvalidCategoryTag(Table, OutIssues);
		RunRDT011_InvalidItemTag(Table, OutIssues);
		RunRDT012_TagOutsideNamespace(Table, Settings, OutIssues);

		// EN: Duplicate checks / ES: Verificaciones de duplicados
		RunRDT020_DuplicateCategoryRow(Table, OutIssues);
		RunRDT021_DuplicateItemKey(Table, OutIssues);

		// EN: Asset reference checks / ES: Verificaciones de referencia de assets
		RunRDT030_NullAssetRef(Table, OutIssues);
		RunRDT031_BrokenSoftRef(Table, OutIssues);
		if (Definition)
		{
			RunRDT032_IncompatibleClass(Table, Definition, OutIssues);
		}
		RunRDT033_RedirectorDetected(Table, OutIssues);

		// EN: Runtime quality / ES: Calidad runtime
		RunRDT040_CategoryBudgetExceeded(Table, Settings, OutIssues);
		RunRDT041_TableBudgetExceeded(Table, Settings, OutIssues);
	}

	return OutIssues.Num() - BeforeCount;
}

int32 FPGXRegistryValidationService::ValidateAll(TArray<FPGXRegistryValidationIssue>& OutIssues) const
{
	const int32 BeforeCount = OutIssues.Num();

	// EN: Discover all definitions / ES: Descubrir todas las definiciones
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

	TArray<FAssetData> FoundDefinitions;
	AssetRegistry.GetAssetsByClass(UPGXRegistryDefinition::StaticClass()->GetClassPathName(), FoundDefinitions, true);

	TSet<FName> LinkedTableNames;

	for (const FAssetData& DefAssetData : FoundDefinitions)
	{
		const UPGXRegistryDefinition* Definition = Cast<UPGXRegistryDefinition>(DefAssetData.GetAsset());
		if (!Definition)
		{
			continue;
		}

		// EN: RDT050 — check policy / ES: RDT050 — verificar politica
		RunRDT050_UndefinedConflictPolicy(Definition, OutIssues);

		// EN: Collect all tables for cross-table checks / ES: Recolectar todas las tablas para checks entre tablas
		TArray<const UDataTable*> LoadedTables;

		for (const TSoftObjectPtr<UDataTable>& TableRef : Definition->DataTables)
		{
			UDataTable* Table = TableRef.LoadSynchronous();
			if (Table)
			{
				ValidateTable(Table, Definition, OutIssues);
				LoadedTables.Add(Table);
				LinkedTableNames.Add(FName(*Table->GetPathName()));
			}
		}

		// EN: RDT022 — cross-table conflict check / ES: RDT022 — verificar conflictos entre tablas
		if (LoadedTables.Num() > 1)
		{
			RunRDT022_CrossTableConflict(LoadedTables, Definition, OutIssues);
		}
	}

	// EN: RDT051 — find orphaned PGXDataTableAssets not linked to any definition
	// ES: RDT051 — encontrar PGXDataTableAssets huerfanos no vinculados a ninguna definicion
	TArray<FAssetData> AllTables;
	AssetRegistry.GetAssetsByClass(FTopLevelAssetPath(TEXT("/Script/PGXCoreRuntime"), TEXT("PGXDataTableAsset")), AllTables, true);

	for (const FAssetData& TableData : AllTables)
	{
		if (!LinkedTableNames.Contains(FName(*TableData.GetObjectPathString())))
		{
			FPGXRegistryValidationIssue Issue = MakeIssue(
				EPGXValidationRuleId::RDT051_TableNotLinked,
				EPGXRegistryValidationSeverity::Warning,
				TableData.AssetName.ToString(),
				NAME_None,
				FString::Printf(TEXT("Table [%s] is not linked to any active registry definition"),
					*TableData.AssetName.ToString()));
			OutIssues.Add(MoveTemp(Issue));
		}
	}

	return OutIssues.Num() - BeforeCount;
}

int32 FPGXRegistryValidationService::CountErrors(const TArray<FPGXRegistryValidationIssue>& Issues)
{
	int32 Count = 0;
	for (const FPGXRegistryValidationIssue& Issue : Issues)
	{
		if (Issue.Severity == EPGXRegistryValidationSeverity::Error)
		{
			++Count;
		}
	}
	return Count;
}

int32 FPGXRegistryValidationService::CountWarnings(const TArray<FPGXRegistryValidationIssue>& Issues)
{
	int32 Count = 0;
	for (const FPGXRegistryValidationIssue& Issue : Issues)
	{
		if (Issue.Severity == EPGXRegistryValidationSeverity::Warning)
		{
			++Count;
		}
	}
	return Count;
}

// ═══════════════════════════════════════════════════════════════
// Schema Rules / Reglas de Esquema
// ═══════════════════════════════════════════════════════════════

void FPGXRegistryValidationService::RunRDT001_InvalidRowStruct(const UDataTable* Table,
	TArray<FPGXRegistryValidationIssue>& OutIssues) const
{
	if (!Table->GetRowStruct() ||
		!Table->GetRowStruct()->IsChildOf(FPGXRegistryCategoryRow::StaticStruct()))
	{
		OutIssues.Add(MakeIssue(
			EPGXValidationRuleId::RDT001_InvalidRowStruct,
			EPGXRegistryValidationSeverity::Error,
			Table->GetName(), NAME_None,
			FString::Printf(TEXT("Table [%s] uses row struct [%s] instead of FPGXRegistryCategoryRow"),
				*Table->GetName(),
				Table->GetRowStruct() ? *Table->GetRowStruct()->GetName() : TEXT("null"))));
	}
}

void FPGXRegistryValidationService::RunRDT002_MissingCategoryTag(const UDataTable* Table,
	TArray<FPGXRegistryValidationIssue>& OutIssues) const
{
	const TMap<FName, uint8*>& RowMap = Table->GetRowMap();
	for (const auto& Pair : RowMap)
	{
		const FPGXRegistryCategoryRow* Row = reinterpret_cast<const FPGXRegistryCategoryRow*>(Pair.Value);
		if (!Row || !Row->CategoryTag.IsValid())
		{
			OutIssues.Add(MakeIssue(
				EPGXValidationRuleId::RDT002_MissingCategoryTag,
				EPGXRegistryValidationSeverity::Error,
				Table->GetName(), Pair.Key,
				FString::Printf(TEXT("Row [%s] has missing or invalid CategoryTag"), *Pair.Key.ToString())));
		}
	}
}

void FPGXRegistryValidationService::RunRDT003_EmptyItemsByTag(const UDataTable* Table,
	TArray<FPGXRegistryValidationIssue>& OutIssues) const
{
	const TMap<FName, uint8*>& RowMap = Table->GetRowMap();
	for (const auto& Pair : RowMap)
	{
		const FPGXRegistryCategoryRow* Row = reinterpret_cast<const FPGXRegistryCategoryRow*>(Pair.Value);
		if (Row && Row->ItemsByTag.Num() == 0)
		{
			OutIssues.Add(MakeIssue(
				EPGXValidationRuleId::RDT003_EmptyItemsByTag,
				EPGXRegistryValidationSeverity::Error,
				Table->GetName(), Pair.Key,
				FString::Printf(TEXT("Row [%s] has an empty ItemsByTag map"), *Pair.Key.ToString())));
		}
	}
}

// ═══════════════════════════════════════════════════════════════
// Tag Integrity / Integridad de Tags
// ═══════════════════════════════════════════════════════════════

void FPGXRegistryValidationService::RunRDT010_InvalidCategoryTag(const UDataTable* Table,
	TArray<FPGXRegistryValidationIssue>& OutIssues) const
{
	// EN: Already covered by RDT002 for missing, this checks structural validity
	// ES: Ya cubierto por RDT002 para faltante, esto verifica validez estructural
	// RDT002 already catches !IsValid(); this rule is the extension point for additional validation.
}

void FPGXRegistryValidationService::RunRDT011_InvalidItemTag(const UDataTable* Table,
	TArray<FPGXRegistryValidationIssue>& OutIssues) const
{
	const TMap<FName, uint8*>& RowMap = Table->GetRowMap();
	for (const auto& Pair : RowMap)
	{
		const FPGXRegistryCategoryRow* Row = reinterpret_cast<const FPGXRegistryCategoryRow*>(Pair.Value);
		if (!Row) continue;

		for (const auto& ItemPair : Row->ItemsByTag)
		{
			if (!ItemPair.Key.IsValid())
			{
				OutIssues.Add(MakeIssue(
					EPGXValidationRuleId::RDT011_InvalidItemTag,
					EPGXRegistryValidationSeverity::Error,
					Table->GetName(), Pair.Key,
					FString::Printf(TEXT("Row [%s] contains an invalid ItemTag key"), *Pair.Key.ToString())));
			}
		}
	}
}

void FPGXRegistryValidationService::RunRDT012_TagOutsideNamespace(const UDataTable* Table,
	const UPGXRegistrySettings* Settings, TArray<FPGXRegistryValidationIssue>& OutIssues) const
{
	if (!Settings || Settings->AllowedTagRoots.Num() == 0)
	{
		return; // EN: No restriction configured / ES: Sin restriccion configurada
	}

	const TMap<FName, uint8*>& RowMap = Table->GetRowMap();
	for (const auto& Pair : RowMap)
	{
		const FPGXRegistryCategoryRow* Row = reinterpret_cast<const FPGXRegistryCategoryRow*>(Pair.Value);
		if (!Row) continue;

		for (const auto& ItemPair : Row->ItemsByTag)
		{
			if (!ItemPair.Key.IsValid()) continue;

			bool bMatchesRoot = false;
			for (const FGameplayTag& Root : Settings->AllowedTagRoots)
			{
				if (ItemPair.Key.MatchesTag(Root))
				{
					bMatchesRoot = true;
					break;
				}
			}

			if (!bMatchesRoot)
			{
				OutIssues.Add(MakeIssue(
					EPGXValidationRuleId::RDT012_TagOutsideNamespace,
					EPGXRegistryValidationSeverity::Warning,
					Table->GetName(), Pair.Key,
					FString::Printf(TEXT("ItemTag [%s] is outside approved namespace roots"),
						*ItemPair.Key.ToString()),
					true /* bAutoFixable */));
			}
		}
	}
}

// ═══════════════════════════════════════════════════════════════
// Duplicate / Conflict
// ═══════════════════════════════════════════════════════════════

void FPGXRegistryValidationService::RunRDT020_DuplicateCategoryRow(const UDataTable* Table,
	TArray<FPGXRegistryValidationIssue>& OutIssues) const
{
	TSet<FGameplayTag> SeenCategories;
	const TMap<FName, uint8*>& RowMap = Table->GetRowMap();

	for (const auto& Pair : RowMap)
	{
		const FPGXRegistryCategoryRow* Row = reinterpret_cast<const FPGXRegistryCategoryRow*>(Pair.Value);
		if (!Row || !Row->CategoryTag.IsValid()) continue;

		if (SeenCategories.Contains(Row->CategoryTag))
		{
			OutIssues.Add(MakeIssue(
				EPGXValidationRuleId::RDT020_DuplicateCategoryRow,
				EPGXRegistryValidationSeverity::Error,
				Table->GetName(), Pair.Key,
				FString::Printf(TEXT("Duplicate CategoryTag [%s] in row [%s]"),
					*Row->CategoryTag.ToString(), *Pair.Key.ToString())));
		}
		else
		{
			SeenCategories.Add(Row->CategoryTag);
		}
	}
}

void FPGXRegistryValidationService::RunRDT021_DuplicateItemKey(const UDataTable* Table,
	TArray<FPGXRegistryValidationIssue>& OutIssues) const
{
	// EN: TMap naturally prevents duplicate keys per row — this rule is satisfied by
	//     the data structure itself. Included as the explicit extension point.
	// ES: TMap naturalmente previene claves duplicadas por fila — esta regla se satisface
	//     por la estructura de datos misma. Incluida como punto de extension explicito.
}

void FPGXRegistryValidationService::RunRDT022_CrossTableConflict(const TArray<const UDataTable*>& Tables,
	const UPGXRegistryDefinition* Definition, TArray<FPGXRegistryValidationIssue>& OutIssues) const
{
	if (!Definition) return;

	// EN: Build set of all composite keys across tables, detect collisions
	// ES: Construir set de todas las claves compuestas entre tablas, detectar colisiones
	TMap<FString, FString> KeyToTable; // CompositeKey -> first table name

	for (const UDataTable* Table : Tables)
	{
		if (!Table) continue;

		const TMap<FName, uint8*>& RowMap = Table->GetRowMap();
		for (const auto& Pair : RowMap)
		{
			const FPGXRegistryCategoryRow* Row = reinterpret_cast<const FPGXRegistryCategoryRow*>(Pair.Value);
			if (!Row || !Row->CategoryTag.IsValid()) continue;

			for (const auto& ItemPair : Row->ItemsByTag)
			{
				if (!ItemPair.Key.IsValid()) continue;

				const FString CompositeStr = FString::Printf(TEXT("%s::%s::%s"),
					*Definition->DatabaseTag.ToString(),
					*Row->CategoryTag.ToString(),
					*ItemPair.Key.ToString());

				if (const FString* FirstTable = KeyToTable.Find(CompositeStr))
				{
					if (Definition->ConflictPolicy == EPGXRegistryConflictPolicy::FailOnConflict)
					{
						OutIssues.Add(MakeIssue(
							EPGXValidationRuleId::RDT022_CrossTableConflict,
							EPGXRegistryValidationSeverity::Error,
							Table->GetName(), Pair.Key,
							FString::Printf(TEXT("Cross-table conflict: key [%s] also exists in [%s]"),
								*CompositeStr, **FirstTable)));
					}
				}
				else
				{
					KeyToTable.Add(CompositeStr, Table->GetName());
				}
			}
		}
	}
}

// ═══════════════════════════════════════════════════════════════
// Asset References / Referencias de Assets
// ═══════════════════════════════════════════════════════════════

void FPGXRegistryValidationService::RunRDT030_NullAssetRef(const UDataTable* Table,
	TArray<FPGXRegistryValidationIssue>& OutIssues) const
{
	const TMap<FName, uint8*>& RowMap = Table->GetRowMap();
	for (const auto& Pair : RowMap)
	{
		const FPGXRegistryCategoryRow* Row = reinterpret_cast<const FPGXRegistryCategoryRow*>(Pair.Value);
		if (!Row) continue;

		for (const auto& ItemPair : Row->ItemsByTag)
		{
			if (ItemPair.Value.IsNull())
			{
				OutIssues.Add(MakeIssue(
					EPGXValidationRuleId::RDT030_NullAssetRef,
					EPGXRegistryValidationSeverity::Error,
					Table->GetName(), Pair.Key,
					FString::Printf(TEXT("Null asset reference for ItemTag [%s] in row [%s]"),
						*ItemPair.Key.ToString(), *Pair.Key.ToString()),
					true /* bAutoFixable — can remove null entries */));
			}
		}
	}
}

void FPGXRegistryValidationService::RunRDT031_BrokenSoftRef(const UDataTable* Table,
	TArray<FPGXRegistryValidationIssue>& OutIssues) const
{
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

	const TMap<FName, uint8*>& RowMap = Table->GetRowMap();
	for (const auto& Pair : RowMap)
	{
		const FPGXRegistryCategoryRow* Row = reinterpret_cast<const FPGXRegistryCategoryRow*>(Pair.Value);
		if (!Row) continue;

		for (const auto& ItemPair : Row->ItemsByTag)
		{
			if (ItemPair.Value.IsNull()) continue;

			const FSoftObjectPath SoftPath = ItemPair.Value.ToSoftObjectPath();
			if (SoftPath.IsNull()) continue;

			FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(SoftPath);
			if (!AssetData.IsValid())
			{
				OutIssues.Add(MakeIssue(
					EPGXValidationRuleId::RDT031_BrokenSoftRef,
					EPGXRegistryValidationSeverity::Error,
					Table->GetName(), Pair.Key,
					FString::Printf(TEXT("Broken soft reference [%s] for ItemTag [%s]"),
						*SoftPath.ToString(), *ItemPair.Key.ToString())));
			}
		}
	}
}

void FPGXRegistryValidationService::RunRDT032_IncompatibleClass(const UDataTable* Table,
	const UPGXRegistryDefinition* Definition, TArray<FPGXRegistryValidationIssue>& OutIssues) const
{
	if (!Definition || !Definition->ExpectedAssetClass) return;

	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

	const TMap<FName, uint8*>& RowMap = Table->GetRowMap();
	for (const auto& Pair : RowMap)
	{
		const FPGXRegistryCategoryRow* Row = reinterpret_cast<const FPGXRegistryCategoryRow*>(Pair.Value);
		if (!Row) continue;

		for (const auto& ItemPair : Row->ItemsByTag)
		{
			if (ItemPair.Value.IsNull()) continue;

			const FSoftObjectPath SoftPath = ItemPair.Value.ToSoftObjectPath();
			FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(SoftPath);
			if (!AssetData.IsValid()) continue;

			// EN: Check if the asset class is compatible / ES: Verificar si la clase de asset es compatible
			UClass* AssetClass = AssetData.GetClass();
			if (AssetClass && !AssetClass->IsChildOf(Definition->ExpectedAssetClass))
			{
				OutIssues.Add(MakeIssue(
					EPGXValidationRuleId::RDT032_IncompatibleClass,
					EPGXRegistryValidationSeverity::Error,
					Table->GetName(), Pair.Key,
					FString::Printf(TEXT("Asset [%s] class [%s] incompatible with expected [%s]"),
						*AssetData.AssetName.ToString(),
						*AssetClass->GetName(),
						*Definition->ExpectedAssetClass->GetName())));
			}
		}
	}
}

void FPGXRegistryValidationService::RunRDT033_RedirectorDetected(const UDataTable* Table,
	TArray<FPGXRegistryValidationIssue>& OutIssues) const
{
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

	const TMap<FName, uint8*>& RowMap = Table->GetRowMap();
	for (const auto& Pair : RowMap)
	{
		const FPGXRegistryCategoryRow* Row = reinterpret_cast<const FPGXRegistryCategoryRow*>(Pair.Value);
		if (!Row) continue;

		for (const auto& ItemPair : Row->ItemsByTag)
		{
			if (ItemPair.Value.IsNull()) continue;

			const FSoftObjectPath SoftPath = ItemPair.Value.ToSoftObjectPath();
			FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(SoftPath);
			if (!AssetData.IsValid()) continue;

			if (AssetData.IsRedirector())
			{
				OutIssues.Add(MakeIssue(
					EPGXValidationRuleId::RDT033_RedirectorDetected,
					EPGXRegistryValidationSeverity::Warning,
					Table->GetName(), Pair.Key,
					FString::Printf(TEXT("Redirector detected for ItemTag [%s] at [%s]"),
						*ItemPair.Key.ToString(), *SoftPath.ToString()),
					true /* bAutoFixable */));
			}
		}
	}
}

// ═══════════════════════════════════════════════════════════════
// Runtime Quality / Calidad Runtime
// ═══════════════════════════════════════════════════════════════

void FPGXRegistryValidationService::RunRDT040_CategoryBudgetExceeded(const UDataTable* Table,
	const UPGXRegistrySettings* Settings, TArray<FPGXRegistryValidationIssue>& OutIssues) const
{
	const int32 Budget = Settings ? Settings->CategoryBudget : 1000;

	const TMap<FName, uint8*>& RowMap = Table->GetRowMap();
	for (const auto& Pair : RowMap)
	{
		const FPGXRegistryCategoryRow* Row = reinterpret_cast<const FPGXRegistryCategoryRow*>(Pair.Value);
		if (!Row) continue;

		if (Row->ItemsByTag.Num() > Budget)
		{
			OutIssues.Add(MakeIssue(
				EPGXValidationRuleId::RDT040_CategoryBudgetExceeded,
				EPGXRegistryValidationSeverity::Warning,
				Table->GetName(), Pair.Key,
				FString::Printf(TEXT("Category [%s] has %d items, exceeds budget of %d"),
					*Row->CategoryTag.ToString(), Row->ItemsByTag.Num(), Budget)));
		}
	}
}

void FPGXRegistryValidationService::RunRDT041_TableBudgetExceeded(const UDataTable* Table,
	const UPGXRegistrySettings* Settings, TArray<FPGXRegistryValidationIssue>& OutIssues) const
{
	const int32 Budget = Settings ? Settings->TableBudget : 10000;

	int32 TotalEntries = 0;
	const TMap<FName, uint8*>& RowMap = Table->GetRowMap();
	for (const auto& Pair : RowMap)
	{
		const FPGXRegistryCategoryRow* Row = reinterpret_cast<const FPGXRegistryCategoryRow*>(Pair.Value);
		if (Row)
		{
			TotalEntries += Row->ItemsByTag.Num();
		}
	}

	if (TotalEntries > Budget)
	{
		OutIssues.Add(MakeIssue(
			EPGXValidationRuleId::RDT041_TableBudgetExceeded,
			EPGXRegistryValidationSeverity::Warning,
			Table->GetName(), NAME_None,
			FString::Printf(TEXT("Table [%s] has %d total entries, exceeds budget of %d"),
				*Table->GetName(), TotalEntries, Budget)));
	}
}

// ═══════════════════════════════════════════════════════════════
// Policy / Documentation
// ═══════════════════════════════════════════════════════════════

void FPGXRegistryValidationService::RunRDT050_UndefinedConflictPolicy(const UPGXRegistryDefinition* Definition,
	TArray<FPGXRegistryValidationIssue>& /*OutIssues*/) const
{
	// EN: This rule is informational — if multiple tables exist and policy is not explicitly set,
	//     warn that the default policy will apply.
	// ES: Esta regla es informativa — si existen multiples tablas y la politica no esta explicitamente
	//     establecida, advertir que la politica default se aplicara.
	if (Definition && Definition->DataTables.Num() > 1)
	{
		// EN: Policy is always set (enum default), so this is really about awareness
		// ES: La politica siempre esta establecida (default del enum), asi que es sobre concientizacion
		// Rule satisfied by default — no issue unless we add explicit "unset" state
	}
}

void FPGXRegistryValidationService::RunRDT051_TableNotLinked(const UDataTable* Table,
	TArray<FPGXRegistryValidationIssue>& OutIssues) const
{
	// EN: Handled in ValidateAll() where we have access to all definitions
	// ES: Manejado en ValidateAll() donde tenemos acceso a todas las definiciones
}

// ═══════════════════════════════════════════════════════════════
// Helpers / Utilidades
// ═══════════════════════════════════════════════════════════════

FPGXRegistryValidationIssue FPGXRegistryValidationService::MakeIssue(EPGXValidationRuleId RuleId,
	EPGXRegistryValidationSeverity Severity, const FString& TableName, FName RowName,
	const FString& Message, bool bAutoFixable)
{
	FPGXRegistryValidationIssue Issue;
	Issue.RuleId = RuleId;
	Issue.Severity = Severity;
	Issue.TableName = TableName;
	Issue.RowName = RowName;
	Issue.Message = Message;
	Issue.bAutoFixable = bAutoFixable;
	return Issue;
}
