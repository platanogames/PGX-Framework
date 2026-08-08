// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Logging/PGXLogCategories.h"

// EN: Config resolution helpers — Settings-first with AssetRegistry fallback.
//     Used by ALL PGX subsystems to resolve their Config DAs deterministically.
//     The user assigns DAs in Project Settings. If not assigned, the system falls
//     back to AssetRegistry scan with a deprecation warning.
//
// ES: Helpers de resolucion de config — Settings primero con fallback a AssetRegistry.
//     Usado por TODOS los subsistemas PGX para resolver sus Config DAs deterministicamente.
//     El usuario asigna DAs en Project Settings. Si no estan asignados, el sistema hace
//     fallback a escaneo de AssetRegistry con warning de deprecacion.

namespace PGX
{
	/**
	 * EN: Resolve a single config DA. Priority: Settings field > AssetRegistry fallback.
	 *     If Settings field is set, loads and returns that DA.
	 *     If not set, scans AssetRegistry:
	 *       - 1 found: auto-resolves with deprecation warning
	 *       - >1 found: error log, uses first found
	 *       - 0 found: returns nullptr
	 *
	 * ES: Resuelve un DA de config unico. Prioridad: campo Settings > fallback AssetRegistry.
	 *     Si el campo de Settings esta asignado, carga y retorna ese DA.
	 *     Si no esta asignado, escanea AssetRegistry:
	 *       - 1 encontrado: auto-resuelve con warning de deprecacion
	 *       - >1 encontrados: error log, usa el primero
	 *       - 0 encontrados: retorna nullptr
	 *
	 * @param ConfigRef      TSoftObjectPtr from the Settings class (may be null/unset)
	 * @param SystemName     Human-readable system name for log messages (e.g. "Loading", "Message")
	 * @return               Loaded config DA, or nullptr if not found
	 */
	template<typename TConfig>
	TConfig* ResolveSingleConfig(const TSoftObjectPtr<TConfig>& ConfigRef, const FString& SystemName)
	{
		// ── Phase 1: Settings-first ──
		if (!ConfigRef.IsNull())
		{
			TConfig* Loaded = ConfigRef.LoadSynchronous();
			if (IsValid(Loaded))
			{
				UE_LOG(LogPGXSettings, Log, TEXT("[%s] Config resolved from Project Settings: %s"),
					*SystemName, *Loaded->GetName());
				return Loaded;
			}
			UE_LOG(LogPGXSettings, Warning, TEXT("[%s] Config DA assigned in Project Settings but failed to load: %s"),
				*SystemName, *ConfigRef.ToString());
		}

		// ── Phase 2: AssetRegistry fallback (deprecated) ──
		IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		TArray<FAssetData> FoundAssets;
		AssetRegistry.GetAssetsByClass(TConfig::StaticClass()->GetClassPathName(), FoundAssets);

		if (FoundAssets.Num() == 0)
		{
			UE_LOG(LogPGXSettings, Log, TEXT("[%s] No config DA found. Using defaults."), *SystemName);
			return nullptr;
		}

		TConfig* Resolved = Cast<TConfig>(FoundAssets[0].GetAsset());

		if (FoundAssets.Num() == 1)
		{
			UE_LOG(LogPGXSettings, Warning, TEXT("[%s] Config DA auto-discovered from AssetRegistry: %s. "
				"Assign it in Project Settings > PGX > %s to remove this warning. "
				"Auto-discovery is deprecated and will be removed in v0.6.0."),
				*SystemName, *FoundAssets[0].AssetName.ToString(), *SystemName);
		}
		else
		{
			UE_LOG(LogPGXSettings, Error, TEXT("[%s] Multiple config DAs found (%d). Using: %s. "
				"Assign the correct one in Project Settings > PGX > %s for deterministic resolution."),
				*SystemName, FoundAssets.Num(), *FoundAssets[0].AssetName.ToString(), *SystemName);
		}

		return Resolved;
	}

	/**
	 * EN: Resolve multiple configs from a DataTable. Priority: DataTable > AssetRegistry fallback.
	 *     TRow must be a FTableRowBase with a Tag key and a TSoftObjectPtr<TConfig> value.
	 *     The caller provides lambdas to extract the key and value from each row.
	 *
	 * ES: Resuelve multiples configs desde un DataTable. Prioridad: DataTable > fallback AssetRegistry.
	 *     TRow debe ser un FTableRowBase con una key de Tag y un TSoftObjectPtr<TConfig> value.
	 *     El caller provee lambdas para extraer key y value de cada row.
	 *
	 * @param DataTableRef   TSoftObjectPtr to the DataTable from Settings (may be null/unset)
	 * @param OutMap          Output map from key to loaded config
	 * @param SystemName      Human-readable system name for log messages
	 * @param GetKey          Lambda: (const TRow&) -> FGameplayTag
	 * @param GetConfig       Lambda: (const TRow&) -> TSoftObjectPtr<TConfig>
	 * @return                Number of configs resolved
	 */
	template<typename TRow, typename TConfig, typename FGetKey, typename FGetConfig>
	int32 ResolveMultiConfig(
		const TSoftObjectPtr<UDataTable>& DataTableRef,
		TMap<FGameplayTag, TObjectPtr<TConfig>>& OutMap,
		const FString& SystemName,
		FGetKey GetKey,
		FGetConfig GetConfig)
	{
		OutMap.Empty();

		// ── Phase 1: DataTable-first ──
		if (!DataTableRef.IsNull())
		{
			UDataTable* Table = DataTableRef.LoadSynchronous();
			if (IsValid(Table))
			{
				TArray<TRow*> Rows;
				Table->GetAllRows<TRow>(TEXT("PGXConfigResolution"), Rows);

				for (const TRow* Row : Rows)
				{
					if (!Row) { continue; }

					const FGameplayTag Key = GetKey(*Row);
					const TSoftObjectPtr<TConfig> ConfigPtr = GetConfig(*Row);

					if (!Key.IsValid())
					{
						UE_LOG(LogPGXSettings, Warning, TEXT("[%s] DataTable row with invalid tag key — skipped."), *SystemName);
						continue;
					}

					if (ConfigPtr.IsNull())
					{
						UE_LOG(LogPGXSettings, Warning, TEXT("[%s] DataTable row '%s' has no config assigned — skipped."),
							*SystemName, *Key.ToString());
						continue;
					}

					TConfig* Loaded = ConfigPtr.LoadSynchronous();
					if (IsValid(Loaded))
					{
						OutMap.Add(Key, Loaded);
					}
					else
					{
						UE_LOG(LogPGXSettings, Warning, TEXT("[%s] Failed to load config for tag '%s': %s"),
							*SystemName, *Key.ToString(), *ConfigPtr.ToString());
					}
				}

				UE_LOG(LogPGXSettings, Log, TEXT("[%s] Resolved %d configs from DataTable: %s"),
					*SystemName, OutMap.Num(), *Table->GetName());
				return OutMap.Num();
			}
			UE_LOG(LogPGXSettings, Warning, TEXT("[%s] DataTable assigned in Settings but failed to load: %s"),
				*SystemName, *DataTableRef.ToString());
		}

		// ── Phase 2: AssetRegistry fallback (deprecated) ──
		IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		TArray<FAssetData> FoundAssets;
		AssetRegistry.GetAssetsByClass(TConfig::StaticClass()->GetClassPathName(), FoundAssets);

		if (FoundAssets.Num() > 0)
		{
			UE_LOG(LogPGXSettings, Warning, TEXT("[%s] %d config DAs auto-discovered from AssetRegistry. "
				"Configure a DataTable in Project Settings > PGX > %s to remove this warning. "
				"Auto-discovery is deprecated and will be removed in v0.6.0."),
				*SystemName, FoundAssets.Num(), *SystemName);
		}

		return OutMap.Num();
	}
}
