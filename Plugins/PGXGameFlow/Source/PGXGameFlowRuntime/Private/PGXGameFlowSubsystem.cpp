// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXGameFlowSubsystem.h"
#include "PGXGameFlowConfig.h"
#include "PGXFlowRulesConfig.h"
#include "PGXGameFlowSettings.h"
#include "Utils/PGXConfigResolution.h"
#include "Logging/PGXLogCategories.h"
#include "Logging/PGXLogMacros.h"
#include "Messages/PGXBridgeMessages.h"
#include "Messages/PGXMessageSubsystem.h"
#include "Messages/Tags/PGXBridgeTags.h"
#include "Tags/PGXGameFlowTags.h"

// Profile dependency
#include "Profile/PGXProfileSubsystem.h"
#include "Profile/PGXPlatformConfig.h"

// AssetRegistry for auto-discovery
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Trace/PGXTraceHelper.h"
#include "Trace/PGXTraceTags.h"

// EN: GameFlow subsystem — complete implementation (discovery, validation, mutation, delegates, console)
// ES: Subsistema GameFlow — implementacion completa (descubrimiento, validacion, mutacion, delegados, consola)

// ============================================================================
// Static
// ============================================================================

TWeakObjectPtr<UPGXGameFlowSubsystem> UPGXGameFlowSubsystem::CachedInstance = nullptr;

// ============================================================================
// Channel Name Lookup
// ============================================================================

static const TCHAR* GChannelNames[] =
{
	TEXT("Global"),
	TEXT("UI"),
	TEXT("Characters"),
	TEXT("AI"),
	TEXT("Cameras"),
	TEXT("Systems"),
	TEXT("LevelLogic"),
	TEXT("Actors")
};
static_assert(UE_ARRAY_COUNT(GChannelNames) == PGX_FLOW_CHANNEL_COUNT, "GChannelNames must match channel count");

FString UPGXGameFlowSubsystem::GetChannelName(EPGXFlowChannel Channel)
{
	const int32 Index = static_cast<int32>(Channel);
	if (Index >= 0 && Index < PGX_FLOW_CHANNEL_COUNT)
	{
		return GChannelNames[Index];
	}
	return TEXT("INVALID");
}

bool UPGXGameFlowSubsystem::DoesTagMatchBranchForTesting(const FGameplayTag& Destination, const FGameplayTag& Branch)
{
	return IsInBranch(Destination, Branch);
}

// ============================================================================
// Lifecycle
// ============================================================================

void UPGXGameFlowSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// EN: Ensure Profile subsystem initializes before GameFlow
	// ES: Asegurar que el subsistema Profile se inicialice antes que GameFlow
	Collection.InitializeDependency<UPGXProfileSubsystem>();
	Collection.InitializeDependency<UPGXMessageSubsystem>();

	Super::Initialize(Collection);

	CachedInstance = this;

	DiscoverConfigs();
	BuildRulesCache();
	ResolveRuntimeBudgets();
	ApplyInitialStates();
	RegisterConsoleCommands();
	RegisterLoadingBridgeListeners();

	bIsInitialized = true;

	// EN: Apply project profile constraints if available
	// ES: Aplicar restricciones del profile de proyecto si esta disponible
	if (auto* ProfileSS = GetGameInstance()->GetSubsystem<UPGXProfileSubsystem>())
	{
		if (ProfileSS->IsProfileResolved())
		{
			ApplyProfileConstraints(ProfileSS->GetResolvedProfile());
		}
		ProfileSS->OnProfileChangedNative.AddUObject(this, &ThisClass::HandleProfileChanged);
	}

	// EN: Register default trace config for GameFlow system
	// ES: Registrar config de traza por defecto para sistema GameFlow
	FPGXTraceHelper::RegisterSystemTraceConfig(TAG_PGX_System_GameFlow, FPGXTraceConfig());

	PGX_LOG_INFO(LogPGXGameFlow, TEXT("[GameFlowSubsystem] Initialized: %d global configs, %d rules configs"),
		DiscoveredConfigs.Num(),
		DiscoveredRulesConfigs.Num());

	// Log initial channel states
	for (int32 i = 0; i < PGX_FLOW_CHANNEL_COUNT; ++i)
	{
		const FGameplayTag& Tag = ChannelStates[i].CurrentTag;
		PGX_LOG_INFO(LogPGXGameFlow, TEXT("  [%s] = %s"),
			GChannelNames[i],
			Tag.IsValid() ? *Tag.ToString() : TEXT("(none)"));
	}
}

void UPGXGameFlowSubsystem::Deinitialize()
{
	// EN: Cleanup Profile delegate subscription / ES: Limpiar suscripcion a delegate de Profile
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UPGXProfileSubsystem* Profile = GI->GetSubsystem<UPGXProfileSubsystem>())
		{
			Profile->OnProfileChangedNative.RemoveAll(this);
		}
	}

	UnregisterLoadingBridgeListeners();
	UnregisterConsoleCommands();
	FPGXTraceHelper::UnregisterSystemTraceConfig(TAG_PGX_System_GameFlow);

	CachedInstance = nullptr;
	bIsInitialized = false;

	// Clear state
	for (int32 i = 0; i < PGX_FLOW_CHANNEL_COUNT; ++i)
	{
		ChannelStates[i].Reset();
	}

	ChannelRulesMap.Empty();
	ActiveConfig = nullptr;
	DiscoveredConfigs.Empty();
	DiscoveredRulesConfigs.Empty();

	Super::Deinitialize();
}

// ============================================================================
// Discovery & Cache
// ============================================================================

void UPGXGameFlowSubsystem::DiscoverConfigs()
{
	const UPGXGameFlowSettings* Settings = GetDefault<UPGXGameFlowSettings>();

	// --- Phase 1: GameFlowConfig (single config — Settings first) ---
	// EN: Settings-first resolution with AssetRegistry fallback (deprecated)
	// ES: Resolucion Settings-first con fallback a AssetRegistry (deprecated)
	ActiveConfig = PGX::ResolveSingleConfig<UPGXGameFlowConfig>(Settings->ActiveConfig, TEXT("GameFlow"));

	if (IsValid(ActiveConfig))
	{
		DiscoveredConfigs.Add(ActiveConfig);
		PGX_LOG_INFO(LogPGXGameFlow, TEXT("[Discovery] Active config: '%s' (ContextTag: %s)"),
			*ActiveConfig->GetName(),
			ActiveConfig->ContextTag.IsValid() ? *ActiveConfig->ContextTag.ToString() : TEXT("(none)"));
	}

	// --- Phase 2: FlowRulesConfig (multi-config — Settings DataTable or AssetRegistry) ---
	if (!Settings->FlowRulesTable.IsNull())
	{
		// EN: Load rules from DataTable (deterministic)
		// ES: Cargar reglas desde DataTable (deterministico)
		UDataTable* Table = Settings->FlowRulesTable.LoadSynchronous();
		if (IsValid(Table))
		{
			TArray<FPGXFlowRulesRow*> Rows;
			Table->GetAllRows<FPGXFlowRulesRow>(TEXT("GameFlowDiscovery"), Rows);

			for (const FPGXFlowRulesRow* Row : Rows)
			{
				if (!Row || Row->RulesRef.IsNull()) { continue; }
				UPGXFlowRulesConfig* RulesConfig = Row->RulesRef.LoadSynchronous();
				if (IsValid(RulesConfig))
				{
					DiscoveredRulesConfigs.Add(RulesConfig);
					PGX_LOG_INFO(LogPGXGameFlow, TEXT("[Discovery] Rules config from DataTable: '%s' — channel: %s, %d rules"),
						*RulesConfig->GetName(),
						*GetChannelName(RulesConfig->Channel),
						RulesConfig->FlowRules.Num());
				}
			}
			PGX_LOG_INFO(LogPGXGameFlow, TEXT("[GameFlow] %d rules configs resolved from DataTable."), DiscoveredRulesConfigs.Num());
		}
	}
	else
	{
		// EN: AssetRegistry fallback (deprecated)
		// ES: Fallback AssetRegistry (deprecated)
		const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		const IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

		FARFilter Filter;
		Filter.ClassPaths.Add(UPGXFlowRulesConfig::StaticClass()->GetClassPathName());
		Filter.bRecursiveClasses = true;

		TArray<FAssetData> AssetDataList;
		AssetRegistry.GetAssets(Filter, AssetDataList);

		for (const FAssetData& AssetData : AssetDataList)
		{
			UPGXFlowRulesConfig* RulesConfig = Cast<UPGXFlowRulesConfig>(AssetData.GetAsset());
			if (!RulesConfig)
			{
				PGX_LOG_WARNING(LogPGXGameFlow, TEXT("[Discovery] Failed to load FlowRulesConfig: %s"), *AssetData.GetObjectPathString());
				continue;
			}

			DiscoveredRulesConfigs.Add(RulesConfig);

			PGX_LOG_INFO(LogPGXGameFlow, TEXT("[Discovery] Rules config: '%s' — channel: %s, %d rules"),
				*RulesConfig->GetName(),
				*GetChannelName(RulesConfig->Channel),
				RulesConfig->FlowRules.Num());
		}

		if (DiscoveredRulesConfigs.Num() > 0)
		{
			PGX_LOG_WARNING(LogPGXGameFlow, TEXT("[GameFlow] %d rules configs auto-discovered from AssetRegistry. "
				"Configure a DataTable in Project Settings > PGX > Game Flow to remove this warning."),
				DiscoveredRulesConfigs.Num());
		}
	}
}

void UPGXGameFlowSubsystem::BuildRulesCache()
{
	ChannelRulesMap.Empty();

	DiscoveredRulesConfigs.Sort([](const TObjectPtr<UPGXFlowRulesConfig>& A, const TObjectPtr<UPGXFlowRulesConfig>& B)
	{
		if (!A) { return false; }
		if (!B) { return true; }

		const int32 ChannelA = static_cast<int32>(A->Channel);
		const int32 ChannelB = static_cast<int32>(B->Channel);
		if (ChannelA != ChannelB)
		{
			return ChannelA < ChannelB;
		}

		if (A->ConflictPriority != B->ConflictPriority)
		{
			return A->ConflictPriority > B->ConflictPriority;
		}

		return A->GetPathName() < B->GetPathName();
	});

	const EPGXFlowDuplicateRulesPolicy Policy = ActiveConfig
		? ActiveConfig->DuplicateRulesPolicy
		: GetDefault<UPGXGameFlowConfig>()->DuplicateRulesPolicy;

	for (UPGXFlowRulesConfig* RulesConfig : DiscoveredRulesConfigs)
	{
		if (!RulesConfig) continue;

		const EPGXFlowChannel Ch = RulesConfig->Channel;
		if (!IsValidChannel(Ch))
		{
			PGX_LOG_WARNING(LogPGXGameFlow, TEXT("[BuildCache] Invalid channel in '%s' — skipped"), *RulesConfig->GetName());
			continue;
		}

		if (ChannelRulesMap.Contains(Ch))
		{
			TObjectPtr<UPGXFlowRulesConfig>& Existing = ChannelRulesMap.FindChecked(Ch);
			const FString ExistingName = IsValid(Existing.Get()) ? Existing->GetName() : TEXT("(none)");
			bool bReplaceExisting = false;

			switch (Policy)
			{
			case EPGXFlowDuplicateRulesPolicy::LastWins:
				bReplaceExisting = true;
				break;
			case EPGXFlowDuplicateRulesPolicy::HighestPriorityWins:
				if (!IsValid(Existing.Get())
					|| RulesConfig->ConflictPriority > Existing->ConflictPriority
					|| (RulesConfig->ConflictPriority == Existing->ConflictPriority
						&& RulesConfig->GetPathName() < Existing->GetPathName()))
				{
					bReplaceExisting = true;
				}
				break;
			case EPGXFlowDuplicateRulesPolicy::FirstWins:
			default:
				bReplaceExisting = false;
				break;
			}

			PGX_LOG_WARNING(LogPGXGameFlow,
				TEXT("[BuildCache] Duplicate rules for channel %s — existing='%s' candidate='%s' policy=%d action=%s"),
				*GetChannelName(Ch),
				*ExistingName,
				*RulesConfig->GetName(),
				static_cast<int32>(Policy),
				bReplaceExisting ? TEXT("replace") : TEXT("ignore"));

			if (bReplaceExisting)
			{
				Existing = RulesConfig;
			}
			continue;
		}

		ChannelRulesMap.Add(Ch, RulesConfig);
	}
}

void UPGXGameFlowSubsystem::ApplyInitialStates()
{
	if (!ActiveConfig) return;

	for (const auto& Pair : ActiveConfig->InitialChannelStates)
	{
		const EPGXFlowChannel Ch = Pair.Key;
		const FGameplayTag& InitTag = Pair.Value;

		if (!IsValidChannel(Ch)) continue;
		if (!InitTag.IsValid()) continue;

		const int32 Index = static_cast<int32>(Ch);
		ChannelStates[Index].CurrentTag = InitTag;
		ChannelStates[Index].History.Add(FPGXFlowHistoryEntry(InitTag));

		PGX_LOG_INFO(LogPGXGameFlow, TEXT("[Init] Channel %s → %s"),
			*GetChannelName(Ch), *InitTag.ToString());
	}
}

void UPGXGameFlowSubsystem::ResolveRuntimeBudgets()
{
	const UPGXGameFlowConfig* ConfigSource = ActiveConfig ? ActiveConfig.Get() : GetDefault<UPGXGameFlowConfig>();
	ResolvedMaxHistoryDepth = FMath::Max(1, ConfigSource->MaxHistoryDepth);

	if (!ActiveConfig)
	{
		PGX_LOG_WARNING(LogPGXGameFlow, TEXT("[GameFlowSubsystem] No active config; using UPGXGameFlowConfig class defaults for runtime budgets."));
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UPGXProfileSubsystem* ProfileSS = GI->GetSubsystem<UPGXProfileSubsystem>())
		{
			if (const UPGXPlatformConfig* PlatformCfg = ProfileSS->GetActivePlatformConfig())
			{
				const int32 ProfileMaxHistory = PlatformCfg->GameFlowBudgets.MaxTransitionHistory;
				if (ProfileMaxHistory > 0)
				{
					ResolvedMaxHistoryDepth = FMath::Min(ResolvedMaxHistoryDepth, ProfileMaxHistory);
				}
			}
		}
	}

	PGX_LOG_INFO(LogPGXGameFlow, TEXT("[GameFlowSubsystem] Runtime budgets resolved — MaxHistoryDepth=%d"),
		ResolvedMaxHistoryDepth);
}

// ============================================================================
// Console Commands
// ============================================================================

void UPGXGameFlowSubsystem::RegisterConsoleCommands()
{
	IConsoleManager& CM = IConsoleManager::Get();

	// --- pgx.gameflow.status ---
	if (IConsoleObject* Cmd = CM.RegisterConsoleCommand(
		TEXT("pgx.gameflow.status"),
		TEXT("Show current state of all 8 GameFlow channels"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			PGX_LOG_INFO(LogPGXGameFlow, TEXT("=== GameFlow Status ==="));
			for (int32 i = 0; i < PGX_FLOW_CHANNEL_COUNT; ++i)
			{
				const FGameplayTag& Tag = ChannelStates[i].CurrentTag;
				const FGameplayTag& Last = ChannelStates[i].LastTag;
				PGX_LOG_INFO(LogPGXGameFlow, TEXT("  [%s] Current: %s | Last: %s | History: %d"),
					GChannelNames[i],
					Tag.IsValid() ? *Tag.ToString() : TEXT("(none)"),
					Last.IsValid() ? *Last.ToString() : TEXT("(none)"),
					ChannelStates[i].History.Num());
			}
		}), ECVF_Default))
	{
		RegisteredCommands.Add(Cmd);
	}

	// --- pgx.gameflow.set <ChannelIndex> <TagString> ---
	if (IConsoleObject* Cmd = CM.RegisterConsoleCommand(
		TEXT("pgx.gameflow.set"),
		TEXT("Set channel state (dev/editor gated): pgx.gameflow.set <0-7> <Tag>"),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& Args)
		{
			if (!IsConsoleMutationAllowed())
			{
				PGX_LOG_WARNING(LogPGXGameFlow, TEXT("pgx.gameflow.set blocked: console mutation disabled by GameFlow config or Shipping build."));
				return;
			}
			if (Args.Num() < 2)
			{
				PGX_LOG_WARNING(LogPGXGameFlow, TEXT("Usage: pgx.gameflow.set <ChannelIndex 0-7> <GameplayTag>"));
				return;
			}
			const int32 ChIdx = FCString::Atoi(*Args[0]);
			if (ChIdx < 0 || ChIdx >= PGX_FLOW_CHANNEL_COUNT)
			{
				PGX_LOG_WARNING(LogPGXGameFlow, TEXT("Invalid channel index: %d (must be 0-%d)"), ChIdx, PGX_FLOW_CHANNEL_COUNT - 1);
				return;
			}
			const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*Args[1]), false);
			if (!Tag.IsValid())
			{
				PGX_LOG_WARNING(LogPGXGameFlow, TEXT("Invalid tag: %s"), *Args[1]);
				return;
			}
			const FPGXFlowResult Result = SetStateByTag(static_cast<EPGXFlowChannel>(ChIdx), Tag, nullptr);
			PGX_LOG_INFO(LogPGXGameFlow, TEXT("SetState [%s] → %s: %s (%s)"),
				GChannelNames[ChIdx], *Tag.ToString(),
				Result.bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"),
				*Result.Description);
		}), ECVF_Default))
	{
		RegisteredCommands.Add(Cmd);
	}

	// --- pgx.gameflow.canchange <ChannelIndex> <TagString> ---
	if (IConsoleObject* Cmd = CM.RegisterConsoleCommand(
		TEXT("pgx.gameflow.canchange"),
		TEXT("Check if transition is valid: pgx.gameflow.canchange <0-7> <Tag>"),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& Args)
		{
			if (Args.Num() < 2)
			{
				PGX_LOG_WARNING(LogPGXGameFlow, TEXT("Usage: pgx.gameflow.canchange <ChannelIndex 0-7> <GameplayTag>"));
				return;
			}
			const int32 ChIdx = FCString::Atoi(*Args[0]);
			if (ChIdx < 0 || ChIdx >= PGX_FLOW_CHANNEL_COUNT)
			{
				PGX_LOG_WARNING(LogPGXGameFlow, TEXT("Invalid channel index: %d"), ChIdx);
				return;
			}
			const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*Args[1]), false);
			if (!Tag.IsValid())
			{
				PGX_LOG_WARNING(LogPGXGameFlow, TEXT("Invalid tag: %s"), *Args[1]);
				return;
			}
			const FPGXFlowResult Result = CanChangeByTag(static_cast<EPGXFlowChannel>(ChIdx), Tag);
			PGX_LOG_INFO(LogPGXGameFlow, TEXT("CanChange [%s] → %s: %s (%s)"),
				GChannelNames[ChIdx], *Tag.ToString(),
				Result.bSuccess ? TEXT("ALLOWED") : TEXT("DENIED"),
				*Result.Description);
		}), ECVF_Default))
	{
		RegisteredCommands.Add(Cmd);
	}

	// --- pgx.gameflow.history <ChannelIndex> ---
	if (IConsoleObject* Cmd = CM.RegisterConsoleCommand(
		TEXT("pgx.gameflow.history"),
		TEXT("Show transition history: pgx.gameflow.history <0-7>"),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& Args)
		{
			if (Args.Num() < 1)
			{
				PGX_LOG_WARNING(LogPGXGameFlow, TEXT("Usage: pgx.gameflow.history <ChannelIndex 0-7>"));
				return;
			}
			const int32 ChIdx = FCString::Atoi(*Args[0]);
			if (ChIdx < 0 || ChIdx >= PGX_FLOW_CHANNEL_COUNT)
			{
				PGX_LOG_WARNING(LogPGXGameFlow, TEXT("Invalid channel index: %d"), ChIdx);
				return;
			}
			const FPGXFlowChannelState& State = ChannelStates[ChIdx];
			PGX_LOG_INFO(LogPGXGameFlow, TEXT("=== History [%s] (%d entries) ==="), GChannelNames[ChIdx], State.History.Num());
			for (int32 j = 0; j < State.History.Num(); ++j)
			{
				const FPGXFlowHistoryEntry& Entry = State.History[j];
				PGX_LOG_INFO(LogPGXGameFlow, TEXT("  [%d] %s @ %s"),
					j, *Entry.FlowTag.ToString(), *Entry.Timestamp.ToString());
			}
		}), ECVF_Default))
	{
		RegisteredCommands.Add(Cmd);
	}

	// --- pgx.gameflow.revert <ChannelIndex> ---
	if (IConsoleObject* Cmd = CM.RegisterConsoleCommand(
		TEXT("pgx.gameflow.revert"),
		TEXT("Revert channel to previous state (dev/editor gated): pgx.gameflow.revert <0-7>"),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& Args)
		{
			if (!IsConsoleMutationAllowed())
			{
				PGX_LOG_WARNING(LogPGXGameFlow, TEXT("pgx.gameflow.revert blocked: console mutation disabled by GameFlow config or Shipping build."));
				return;
			}
			if (Args.Num() < 1)
			{
				PGX_LOG_WARNING(LogPGXGameFlow, TEXT("Usage: pgx.gameflow.revert <ChannelIndex 0-7>"));
				return;
			}
			const int32 ChIdx = FCString::Atoi(*Args[0]);
			if (ChIdx < 0 || ChIdx >= PGX_FLOW_CHANNEL_COUNT)
			{
				PGX_LOG_WARNING(LogPGXGameFlow, TEXT("Invalid channel index: %d"), ChIdx);
				return;
			}
			const FPGXFlowResult Result = RevertToPreviousFlow(static_cast<EPGXFlowChannel>(ChIdx), nullptr);
			PGX_LOG_INFO(LogPGXGameFlow, TEXT("Revert [%s]: %s (%s)"),
				GChannelNames[ChIdx],
				Result.bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"),
				*Result.Description);
		}), ECVF_Default))
	{
		RegisteredCommands.Add(Cmd);
	}

	// --- pgx.gameflow.rules <ChannelIndex> ---
	if (IConsoleObject* Cmd = CM.RegisterConsoleCommand(
		TEXT("pgx.gameflow.rules"),
		TEXT("Show rules for a channel: pgx.gameflow.rules <0-7>"),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& Args)
		{
			if (Args.Num() < 1)
			{
				PGX_LOG_WARNING(LogPGXGameFlow, TEXT("Usage: pgx.gameflow.rules <ChannelIndex 0-7>"));
				return;
			}
			const int32 ChIdx = FCString::Atoi(*Args[0]);
			if (ChIdx < 0 || ChIdx >= PGX_FLOW_CHANNEL_COUNT)
			{
				PGX_LOG_WARNING(LogPGXGameFlow, TEXT("Invalid channel index: %d"), ChIdx);
				return;
			}
			const EPGXFlowChannel Ch = static_cast<EPGXFlowChannel>(ChIdx);
			const TObjectPtr<UPGXFlowRulesConfig>* FoundRules = ChannelRulesMap.Find(Ch);
			if (!FoundRules || !(*FoundRules))
			{
				PGX_LOG_INFO(LogPGXGameFlow, TEXT("[%s] No rules config loaded (permissive)"), GChannelNames[ChIdx]);
				return;
			}
			const UPGXFlowRulesConfig* RC = *FoundRules;
			PGX_LOG_INFO(LogPGXGameFlow, TEXT("=== Rules [%s] (%d rules) ==="), GChannelNames[ChIdx], RC->FlowRules.Num());
			for (const auto& Pair : RC->FlowRules)
			{
				const FPGXFlowRule& Rule = Pair.Value;
				PGX_LOG_INFO(LogPGXGameFlow, TEXT("  Origin: %s | Allowed: %d | Disallowed: %d | Revert: %s"),
					*Pair.Key.ToString(),
					Rule.AllowedDestinations.Num(),
					Rule.DisallowedTagQueries.Num(),
					Rule.bAllowRevert ? TEXT("YES") : TEXT("NO"));
			}
		}), ECVF_Default))
	{
		RegisteredCommands.Add(Cmd);
	}
}

void UPGXGameFlowSubsystem::UnregisterConsoleCommands()
{
	IConsoleManager& CM = IConsoleManager::Get();

	for (IConsoleObject* Cmd : RegisteredCommands)
	{
		if (Cmd)
		{
			CM.UnregisterConsoleObject(Cmd);
		}
	}
	RegisteredCommands.Empty();
}

int32 UPGXGameFlowSubsystem::GetResolvedMaxHistoryDepth() const
{
	if (ResolvedMaxHistoryDepth > 0)
	{
		return ResolvedMaxHistoryDepth;
	}

	const UPGXGameFlowConfig* ConfigSource = ActiveConfig ? ActiveConfig.Get() : GetDefault<UPGXGameFlowConfig>();
	return FMath::Max(1, ConfigSource->MaxHistoryDepth);
}

bool UPGXGameFlowSubsystem::IsConsoleMutationAllowed() const
{
#if UE_BUILD_SHIPPING
	return false;
#else
	const UPGXGameFlowConfig* ConfigSource = ActiveConfig ? ActiveConfig.Get() : GetDefault<UPGXGameFlowConfig>();
	return ConfigSource->bAllowConsoleMutations;
#endif
}

// ============================================================================
// Validation Engine
// ============================================================================

bool UPGXGameFlowSubsystem::IsInBranch(const FGameplayTag& Destination, const FGameplayTag& Rule)
{
	// EN: String-based branch check — works for ALL tags regardless of registration.
	//     Exact match OR destination is a descendant (starts with rule + ".").
	// ES: Verificacion de rama basada en strings — funciona para TODOS los tags sin importar registro.
	//     Match exacto O el destino es descendiente (empieza con rule + ".").
	const FString DestStr = Destination.ToString();
	const FString RuleStr = Rule.ToString();
	return (DestStr == RuleStr) || DestStr.StartsWith(RuleStr + TEXT("."));
}

const FPGXFlowRule* UPGXGameFlowSubsystem::FindRuleForCurrentState(EPGXFlowChannel Channel) const
{
	if (!IsValidChannel(Channel)) return nullptr;

	const int32 Index = static_cast<int32>(Channel);
	return FindRuleForTag(Channel, ChannelStates[Index].CurrentTag);
}

const FPGXFlowRule* UPGXGameFlowSubsystem::FindRuleForTag(EPGXFlowChannel Channel, const FGameplayTag& Tag) const
{
	if (!Tag.IsValid()) return nullptr;

	const TObjectPtr<UPGXFlowRulesConfig>* FoundRules = ChannelRulesMap.Find(Channel);
	if (!FoundRules || !(*FoundRules)) return nullptr;

	// EN: Exact match lookup on current state tag (no parent fallback — matches BP behavior)
	// ES: Busqueda de match exacto en tag de estado actual (sin fallback a padres — coincide con comportamiento BP)
	return (*FoundRules)->FlowRules.Find(Tag);
}

bool UPGXGameFlowSubsystem::RunAllowedCheck(const FGameplayTag& Destination, const TArray<FGameplayTag>& AllowedDestinations)
{
	// EN: Any IsInBranch match → PASS (destination is in an allowed branch)
	// ES: Cualquier match IsInBranch → PASA (destino esta en una rama permitida)
	for (const FGameplayTag& AllowedTag : AllowedDestinations)
	{
		if (IsInBranch(Destination, AllowedTag))
		{
			return true;
		}
	}
	return false;
}

bool UPGXGameFlowSubsystem::RunDisallowedCheck(const FGameplayTag& Destination, const TArray<FGameplayTag>& DisallowedTagQueries)
{
	// EN: Returns true if destination is NOT blocked (passes veto check)
	//     Returns false if destination IS blocked (veto applied)
	// ES: Retorna true si el destino NO esta bloqueado (pasa check de veto)
	//     Retorna false si el destino SI esta bloqueado (veto aplicado)
	for (const FGameplayTag& BlockedTag : DisallowedTagQueries)
	{
		if (IsInBranch(Destination, BlockedTag))
		{
			return false; // Blocked
		}
	}
	return true; // Not blocked
}

FPGXFlowResult UPGXGameFlowSubsystem::ValidateTransitionFromState(EPGXFlowChannel Channel, const FGameplayTag& CurrentState, const FGameplayTag& Destination) const
{
	// EN: Full orchestrator — implements the CanChange{Channel}ByTag truth table from BP prototype
	// ES: Orquestador completo — implementa la tabla de verdad CanChange{Channel}ByTag del prototipo BP

	// 1. Find rules for current/simulated state (not destination!)
	const FPGXFlowRule* Rule = FindRuleForTag(Channel, CurrentState);

	// No rules → ALLOW (permissive by default)
	if (!Rule)
	{
		return FPGXFlowResult::MakeSuccess(TEXT("No rules for current state — open"));
	}

	const bool bAllowedEmpty = Rule->AllowedDestinations.IsEmpty();
	const bool bDisallowedEmpty = Rule->DisallowedTagQueries.IsEmpty();

	// Both empty → ALLOW
	if (bAllowedEmpty && bDisallowedEmpty)
	{
		return FPGXFlowResult::MakeSuccess(TEXT("Rule has no restrictions — open"));
	}

	// Determine AllowedTagFlow
	bool bAllowedPass = false;

	if (bAllowedEmpty)
	{
		// No whitelist = all allowed, check blacklist only
		bAllowedPass = true;
	}
	else
	{
		// Run AllowedCheck
		bAllowedPass = RunAllowedCheck(Destination, Rule->AllowedDestinations);
	}

	// Short-circuit: if allowed check fails, deny immediately
	if (!bAllowedPass)
	{
		return FPGXFlowResult::MakeFail(EPGXFlowResultCode::ValidationError,
			FString::Printf(TEXT("Destination '%s' not in allowed branches from '%s'"),
				*Destination.ToString(),
				CurrentState.IsValid() ? *CurrentState.ToString() : TEXT("(none)")));
	}

	// Run DisallowedCheck (veto layer)
	if (!bDisallowedEmpty)
	{
		const bool bPassesVeto = RunDisallowedCheck(Destination, Rule->DisallowedTagQueries);
		if (!bPassesVeto)
		{
			return FPGXFlowResult::MakeFail(EPGXFlowResultCode::ValidationError,
				FString::Printf(TEXT("Destination '%s' blocked by disallowed rule"),
					*Destination.ToString()));
		}
	}

	// Both passed
	return FPGXFlowResult::MakeSuccess(TEXT("Transition validated"));
}

FPGXFlowResult UPGXGameFlowSubsystem::ValidateTransition(EPGXFlowChannel Channel, const FGameplayTag& Destination) const
{
	const int32 Index = static_cast<int32>(Channel);
	return ValidateTransitionFromState(Channel, ChannelStates[Index].CurrentTag, Destination);
}

// ============================================================================
// State Mutation
// ============================================================================

void UPGXGameFlowSubsystem::ApplyTransition(EPGXFlowChannel Channel, const FGameplayTag& NewTag)
{
	const int32 Index = static_cast<int32>(Channel);
	FPGXFlowChannelState& State = ChannelStates[Index];

	// EN: Save current as last
	// ES: Guardar actual como anterior
	const FGameplayTag OldTag = State.CurrentTag;
	State.LastTag = OldTag;
	State.CurrentTag = NewTag;

	// EN: Push to history arrays
	// ES: Agregar a arrays de historial
	State.History.Add(FPGXFlowHistoryEntry(NewTag));
	if (OldTag.IsValid())
	{
		State.LastTagHistory.Add(FPGXFlowHistoryEntry(OldTag));
	}

	// EN: Trim history to MaxHistoryDepth
	// ES: Recortar historial a MaxHistoryDepth
	const int32 MaxDepth = GetResolvedMaxHistoryDepth();
	while (State.History.Num() > MaxDepth)
	{
		State.History.RemoveAt(0);
	}
	while (State.LastTagHistory.Num() > MaxDepth)
	{
		State.LastTagHistory.RemoveAt(0);
	}
}

void UPGXGameFlowSubsystem::BroadcastDelegates(EPGXFlowChannel Channel, const FGameplayTag& OldTag, const FGameplayTag& FlowTag, UObject* Source)
{
	// EN: Fire channel-specific dynamic delegate (Blueprint)
	// ES: Disparar delegado dinamico especifico del canal (Blueprint)
	switch (Channel)
	{
	case EPGXFlowChannel::Global:      OnFlowGlobalChanged.Broadcast(FlowTag, Source);     break;
	case EPGXFlowChannel::UI:          OnFlowUIChanged.Broadcast(FlowTag, Source);         break;
	case EPGXFlowChannel::Characters:  OnFlowCharactersChanged.Broadcast(FlowTag, Source); break;
	case EPGXFlowChannel::AI:          OnFlowAIChanged.Broadcast(FlowTag, Source);         break;
	case EPGXFlowChannel::Cameras:     OnFlowCamerasChanged.Broadcast(FlowTag, Source);    break;
	case EPGXFlowChannel::Systems:     OnFlowSystemsChanged.Broadcast(FlowTag, Source);    break;
	case EPGXFlowChannel::LevelLogic:  OnFlowLevelLogicChanged.Broadcast(FlowTag, Source); break;
	case EPGXFlowChannel::Actors:      OnFlowActorsChanged.Broadcast(FlowTag, Source);     break;
	default: break;
	}

	// EN: Fire native generic delegate (C++)
	// ES: Disparar delegado generico nativo (C++)
	OnFlowStateChangedNative.Broadcast(Channel, FlowTag, Source);

	PublishBridgeMessage(Channel, OldTag, FlowTag, Source);
}

void UPGXGameFlowSubsystem::PublishBridgeMessage(EPGXFlowChannel Channel, const FGameplayTag& OldTag, const FGameplayTag& NewTag, UObject* Source)
{
	UGameInstance* GI = GetGameInstance();
	if (!IsValid(GI))
	{
		return;
	}

	UPGXMessageSubsystem* MessageSubsystem = GI->GetSubsystem<UPGXMessageSubsystem>();
	if (!IsValid(MessageSubsystem))
	{
		PGX_LOG_WARNING(LogPGXGameFlow, TEXT("[GameFlowSubsystem] Bridge message skipped: PGXMessageSubsystem unavailable."));
		return;
	}

	FPGXBridgeGameFlowChanged Message;
	Message.OldState = OldTag;
	Message.NewState = NewTag;
	Message.Timestamp = GI->GetWorld() ? GI->GetWorld()->GetTimeSeconds() : 0.0;

	MessageSubsystem->BroadcastMessage<FPGXBridgeGameFlowChanged>(TAG_PGX_Bridge_GameFlow_StateChanged, Message);

	if (ActiveConfig && ActiveConfig->bVerboseDebug)
	{
		PGX_LOG_VERBOSE(LogPGXGameFlow, TEXT("[GameFlowSubsystem] Bridge published channel=%s old=%s new=%s source=%s"),
			*GetChannelName(Channel),
			OldTag.IsValid() ? *OldTag.ToString() : TEXT("(none)"),
			NewTag.IsValid() ? *NewTag.ToString() : TEXT("(none)"),
			IsValid(Source) ? *Source->GetName() : TEXT("(none)"));
	}
}

void UPGXGameFlowSubsystem::RegisterLoadingBridgeListeners()
{
	UGameInstance* GI = GetGameInstance();
	if (!IsValid(GI))
	{
		PGX_LOG_WARNING(LogPGXGameFlow, TEXT("[GameFlowSubsystem] Loading bridge listener registration skipped: GameInstance unavailable."));
		return;
	}

	UPGXMessageSubsystem* MessageSubsystem = GI->GetSubsystem<UPGXMessageSubsystem>();
	if (!IsValid(MessageSubsystem))
	{
		PGX_LOG_WARNING(LogPGXGameFlow, TEXT("[GameFlowSubsystem] Loading bridge listener registration skipped: PGXMessageSubsystem unavailable."));
		return;
	}

	LoadingSetStateListenerHandle = MessageSubsystem->RegisterListener<FPGXMessage>(
		TAG_PGX_Loading_GameFlow_SetState,
		this,
		&UPGXGameFlowSubsystem::HandleLoadingSetStateMessage);
	LoadingRevertListenerHandle = MessageSubsystem->RegisterListener<FPGXMessage>(
		TAG_PGX_Loading_GameFlow_Revert,
		this,
		&UPGXGameFlowSubsystem::HandleLoadingRevertMessage);
}

void UPGXGameFlowSubsystem::UnregisterLoadingBridgeListeners()
{
	if (LoadingSetStateListenerHandle.IsValid())
	{
		LoadingSetStateListenerHandle.Unregister();
	}

	if (LoadingRevertListenerHandle.IsValid())
	{
		LoadingRevertListenerHandle.Unregister();
	}
}

void UPGXGameFlowSubsystem::HandleLoadingSetStateMessage(FGameplayTag /*Channel*/, const FPGXMessage& Message)
{
	if (!Message.MessageTag.IsValid())
	{
		PGX_LOG_WARNING(LogPGXGameFlow, TEXT("[GameFlowSubsystem] Loading set-state request ignored: invalid target state tag."));
		return;
	}

	SetStateByTag(EPGXFlowChannel::Global, Message.MessageTag, Message.Owner.Get());
}

void UPGXGameFlowSubsystem::HandleLoadingRevertMessage(FGameplayTag /*Channel*/, const FPGXMessage& Message)
{
	RevertToPreviousFlow(EPGXFlowChannel::Global, Message.Owner.Get());
}

// ============================================================================
// Public API — Set
// ============================================================================

FPGXFlowResult UPGXGameFlowSubsystem::SetStateByTag(EPGXFlowChannel Channel, FGameplayTag FlowTag, UObject* Source)
{
	// Guard: valid channel
	if (!IsValidChannel(Channel))
	{
		return FPGXFlowResult::MakeFail(EPGXFlowResultCode::InvalidContext, TEXT("Invalid channel"));
	}

	// Guard: valid tag
	if (!FlowTag.IsValid())
	{
		return FPGXFlowResult::MakeFail(EPGXFlowResultCode::InvalidContext, TEXT("Invalid flow tag"));
	}

	// Guard: redundant
	const int32 Index = static_cast<int32>(Channel);
	if (ChannelStates[Index].CurrentTag == FlowTag)
	{
		return FPGXFlowResult::MakeFail(EPGXFlowResultCode::RedundantState,
			FString::Printf(TEXT("Channel %s already in state %s"), *GetChannelName(Channel), *FlowTag.ToString()));
	}

	// Validate
	FPGXFlowResult ValidationResult = ValidateTransition(Channel, FlowTag);
	if (!ValidationResult.bSuccess)
	{
		if (ActiveConfig && ActiveConfig->bLogTransitions)
		{
			PGX_LOG_WARNING(LogPGXGameFlow, TEXT("[%s] Transition DENIED → %s: %s"),
				*GetChannelName(Channel), *FlowTag.ToString(), *ValidationResult.Description);
		}
		return ValidationResult;
	}

	// Apply
	const FGameplayTag OldTag = ChannelStates[Index].CurrentTag;
	ApplyTransition(Channel, FlowTag);

	// Broadcast
	BroadcastDelegates(Channel, OldTag, FlowTag, Source);

	// Log
	if (ActiveConfig && ActiveConfig->bLogTransitions)
	{
		PGX_LOG_INFO(LogPGXGameFlow, TEXT("[%s] %s → %s"),
			*GetChannelName(Channel),
			OldTag.IsValid() ? *OldTag.ToString() : TEXT("(none)"),
			*FlowTag.ToString());
	}

	return FPGXFlowResult::MakeSuccess(TEXT("State changed"));
}

FPGXFlowResult UPGXGameFlowSubsystem::SetBatchSequentialStateByTag(EPGXFlowChannel Channel, const TArray<FGameplayTag>& FlowTags, UObject* Source)
{
	if (!IsValidChannel(Channel))
	{
		return FPGXFlowResult::MakeFail(EPGXFlowResultCode::InvalidContext, TEXT("Invalid channel"));
	}

	if (FlowTags.IsEmpty())
	{
		return FPGXFlowResult::MakeFail(EPGXFlowResultCode::InvalidContext, TEXT("Empty tag array"));
	}

	// EN: Validate ALL first (atomic validation)
	// ES: Validar TODOS primero (validacion atomica)
	FPGXFlowResult BatchValidation = CanBatchChangeByTag(Channel, FlowTags);
	if (!BatchValidation.bSuccess)
	{
		return BatchValidation;
	}

	// EN: Apply sequentially — each transition fires its own delegate
	// ES: Aplicar secuencialmente — cada transicion dispara su propio delegado
	for (const FGameplayTag& Tag : FlowTags)
	{
		const FGameplayTag OldTag = ChannelStates[static_cast<int32>(Channel)].CurrentTag;
		ApplyTransition(Channel, Tag);
		BroadcastDelegates(Channel, OldTag, Tag, Source);

		if (ActiveConfig && ActiveConfig->bLogTransitions)
		{
			PGX_LOG_INFO(LogPGXGameFlow, TEXT("[%s] Batch → %s"), *GetChannelName(Channel), *Tag.ToString());
		}
	}

	return FPGXFlowResult::MakeSuccess(TEXT("Batch sequential applied"));
}

FPGXFlowResult UPGXGameFlowSubsystem::SetBatchStateByTag(EPGXFlowChannel Channel, const FGameplayTagContainer& FlowTags, UObject* Source)
{
	// EN: Break container into array, then delegate to sequential
	// ES: Romper contenedor en array, luego delegar a secuencial
	TArray<FGameplayTag> TagArray;
	FlowTags.GetGameplayTagArray(TagArray);

	return SetBatchSequentialStateByTag(Channel, TagArray, Source);
}

FPGXFlowResult UPGXGameFlowSubsystem::RevertToPreviousFlow(EPGXFlowChannel Channel, UObject* Source)
{
	if (!IsValidChannel(Channel))
	{
		return FPGXFlowResult::MakeFail(EPGXFlowResultCode::InvalidContext, TEXT("Invalid channel"));
	}

	const int32 Index = static_cast<int32>(Channel);
	const FPGXFlowChannelState& State = ChannelStates[Index];

	// Guard: has previous state
	if (!State.LastTag.IsValid())
	{
		return FPGXFlowResult::MakeFail(EPGXFlowResultCode::InvalidContext,
			FString::Printf(TEXT("Channel %s has no previous state to revert to"), *GetChannelName(Channel)));
	}

	// Check bAllowRevert in current rule
	const FPGXFlowRule* Rule = FindRuleForCurrentState(Channel);
	if (Rule && !Rule->bAllowRevert)
	{
		return FPGXFlowResult::MakeFail(EPGXFlowResultCode::ValidationError,
			FString::Printf(TEXT("Revert not allowed from state %s (bAllowRevert=false)"),
				*State.CurrentTag.ToString()));
	}

	// EN: Swap current and last (revert)
	// ES: Intercambiar actual y anterior (revertir)
	const FGameplayTag OldTag = State.CurrentTag;
	const FGameplayTag RevertTag = State.LastTag;
	ApplyTransition(Channel, RevertTag);
	BroadcastDelegates(Channel, OldTag, RevertTag, Source);

	if (ActiveConfig && ActiveConfig->bLogTransitions)
	{
		PGX_LOG_INFO(LogPGXGameFlow, TEXT("[%s] Reverted → %s"), *GetChannelName(Channel), *RevertTag.ToString());
	}

	return FPGXFlowResult::MakeSuccess(TEXT("Reverted to previous state"));
}

// ============================================================================
// Public API — Validation
// ============================================================================

FPGXFlowResult UPGXGameFlowSubsystem::CanChangeByTag(EPGXFlowChannel Channel, FGameplayTag FlowTag) const
{
	if (!IsValidChannel(Channel))
	{
		return FPGXFlowResult::MakeFail(EPGXFlowResultCode::InvalidContext, TEXT("Invalid channel"));
	}
	if (!FlowTag.IsValid())
	{
		return FPGXFlowResult::MakeFail(EPGXFlowResultCode::InvalidContext, TEXT("Invalid flow tag"));
	}

	// Redundant check
	const int32 Index = static_cast<int32>(Channel);
	if (ChannelStates[Index].CurrentTag == FlowTag)
	{
		return FPGXFlowResult::MakeFail(EPGXFlowResultCode::RedundantState,
			FString::Printf(TEXT("Already in state %s"), *FlowTag.ToString()));
	}

	return ValidateTransition(Channel, FlowTag);
}

FPGXFlowResult UPGXGameFlowSubsystem::CanBatchChangeByTag(EPGXFlowChannel Channel, const TArray<FGameplayTag>& FlowTags) const
{
	if (!IsValidChannel(Channel))
	{
		return FPGXFlowResult::MakeFail(EPGXFlowResultCode::InvalidContext, TEXT("Invalid channel"));
	}

	if (FlowTags.IsEmpty())
	{
		return FPGXFlowResult::MakeFail(EPGXFlowResultCode::InvalidContext, TEXT("Empty tag array"));
	}

	// EN: Simulate sequential transitions to validate the full chain
	// ES: Simular transiciones secuenciales para validar la cadena completa
	FGameplayTag SimulatedCurrent = ChannelStates[static_cast<int32>(Channel)].CurrentTag;

	for (int32 i = 0; i < FlowTags.Num(); ++i)
	{
		const FGameplayTag& Tag = FlowTags[i];

		if (!Tag.IsValid())
		{
			return FPGXFlowResult::MakeFail(EPGXFlowResultCode::InvalidContext,
				FString::Printf(TEXT("Invalid tag at index %d"), i));
		}

		if (SimulatedCurrent == Tag)
		{
			return FPGXFlowResult::MakeFail(EPGXFlowResultCode::RedundantState,
				FString::Printf(TEXT("Redundant state at index %d: %s"), i, *Tag.ToString()));
		}

		const FPGXFlowResult StepValidation = ValidateTransitionFromState(Channel, SimulatedCurrent, Tag);
		if (!StepValidation.bSuccess)
		{
			return FPGXFlowResult::MakeFail(StepValidation.Code,
				FString::Printf(TEXT("Batch index %d: %s"), i, *StepValidation.Description));
		}

		SimulatedCurrent = Tag;
	}

	return FPGXFlowResult::MakeSuccess(TEXT("Batch validation passed"));
}

bool UPGXGameFlowSubsystem::IsCurrentFlowTag(EPGXFlowChannel Channel, FGameplayTag FlowTag) const
{
	if (!IsValidChannel(Channel)) return false;
	return ChannelStates[static_cast<int32>(Channel)].CurrentTag == FlowTag;
}

bool UPGXGameFlowSubsystem::CheckCanRevert(EPGXFlowChannel Channel) const
{
	if (!IsValidChannel(Channel)) return false;

	const int32 Index = static_cast<int32>(Channel);
	if (!ChannelStates[Index].LastTag.IsValid()) return false;

	const FPGXFlowRule* Rule = FindRuleForCurrentState(Channel);
	return !Rule || Rule->bAllowRevert;
}

// ============================================================================
// Public API — Query
// ============================================================================

FGameplayTag UPGXGameFlowSubsystem::GetCurrentFlowTag(EPGXFlowChannel Channel) const
{
	if (!IsValidChannel(Channel)) return FGameplayTag();
	return ChannelStates[static_cast<int32>(Channel)].CurrentTag;
}

FGameplayTag UPGXGameFlowSubsystem::GetLastFlowTag(EPGXFlowChannel Channel) const
{
	if (!IsValidChannel(Channel)) return FGameplayTag();
	return ChannelStates[static_cast<int32>(Channel)].LastTag;
}

TArray<FPGXFlowHistoryEntry> UPGXGameFlowSubsystem::GetChannelHistory(EPGXFlowChannel Channel) const
{
	if (!IsValidChannel(Channel)) return {};
	return ChannelStates[static_cast<int32>(Channel)].History;
}

bool UPGXGameFlowSubsystem::GetAllowedTransitionByTag(EPGXFlowChannel Channel, FGameplayTag FlowTag, FPGXFlowRule& OutRule) const
{
	const FPGXFlowRule* Rule = FindRuleForTag(Channel, FlowTag);
	if (Rule)
	{
		OutRule = *Rule;
		return true;
	}
	return false;
}

bool UPGXGameFlowSubsystem::GetAllowedTransitionByCurrentFlowTag(EPGXFlowChannel Channel, FPGXFlowRule& OutRule) const
{
	const FPGXFlowRule* Rule = FindRuleForCurrentState(Channel);
	if (Rule)
	{
		OutRule = *Rule;
		return true;
	}
	return false;
}

// ============================================================================
// Profile Integration
// ============================================================================

void UPGXGameFlowSubsystem::ApplyProfileConstraints(const FPGXResolvedProfile& Profile)
{
	// EN: Enforce GameFlow platform budgets from active PlatformConfig
	// ES: Aplicar presupuestos de plataforma GameFlow desde PlatformConfig activa
	int32 EnforcedMaxChannels = 0;
	int32 EnforcedMaxHistory = 0;

	if (auto* ProfileSS = UPGXProfileSubsystem::GetCachedInstance())
	{
		if (const UPGXPlatformConfig* PlatformCfg = ProfileSS->GetActivePlatformConfig())
		{
			const auto& B = PlatformCfg->GameFlowBudgets;
			EnforcedMaxChannels = B.MaxFlowChannels;
			EnforcedMaxHistory = B.MaxTransitionHistory;
		}
	}

	ResolveRuntimeBudgets();
	const int32 MaxDepth = GetResolvedMaxHistoryDepth();
	for (int32 i = 0; i < PGX_FLOW_CHANNEL_COUNT; ++i)
	{
		FPGXFlowChannelState& State = ChannelStates[i];
		while (State.History.Num() > MaxDepth)
		{
			State.History.RemoveAt(0);
		}
		while (State.LastTagHistory.Num() > MaxDepth)
		{
			State.LastTagHistory.RemoveAt(0);
		}
	}

	PGX_LOG_INFO(LogPGXGameFlow, TEXT("[GameFlowSubsystem] Profile constraints enforced — MaxChannels=%d, MaxHistory=%d, Mode=%d, HotReload=%d"),
		EnforcedMaxChannels, EnforcedMaxHistory,
		static_cast<int32>(Profile.Identity.ProjectMode),
		Profile.Capabilities.bAllowHotReload);
}

void UPGXGameFlowSubsystem::HandleProfileChanged(const FPGXResolvedProfile& /*OldProfile*/, const FPGXResolvedProfile& NewProfile)
{
	ApplyProfileConstraints(NewProfile);
}

#if WITH_DEV_AUTOMATION_TESTS
void UPGXGameFlowSubsystem::InjectTestConfig(UPGXGameFlowConfig* TestConfig)
{
	ActiveConfig = TestConfig;
	DiscoveredConfigs.Reset();
	if (IsValid(TestConfig))
	{
		DiscoveredConfigs.Add(TestConfig);
	}
	ResolveRuntimeBudgets();
}

void UPGXGameFlowSubsystem::InjectTestRulesConfigs(const TArray<UPGXFlowRulesConfig*>& TestRulesConfigs)
{
	DiscoveredRulesConfigs.Reset();
	for (UPGXFlowRulesConfig* RulesConfig : TestRulesConfigs)
	{
		if (IsValid(RulesConfig))
		{
			DiscoveredRulesConfigs.Add(RulesConfig);
		}
	}
	BuildRulesCache();
}

void UPGXGameFlowSubsystem::RebuildRulesCacheForTesting()
{
	BuildRulesCache();
}

void UPGXGameFlowSubsystem::ResetChannelStatesForTesting()
{
	for (int32 i = 0; i < PGX_FLOW_CHANNEL_COUNT; ++i)
	{
		ChannelStates[i].Reset();
	}
}

const UPGXFlowRulesConfig* UPGXGameFlowSubsystem::GetRulesConfigForTesting(EPGXFlowChannel Channel) const
{
	const TObjectPtr<UPGXFlowRulesConfig>* FoundRules = ChannelRulesMap.Find(Channel);
	return FoundRules ? FoundRules->Get() : nullptr;
}

int32 UPGXGameFlowSubsystem::GetResolvedMaxHistoryDepthForTesting() const
{
	return GetResolvedMaxHistoryDepth();
}

bool UPGXGameFlowSubsystem::IsConsoleMutationAllowedForTesting() const
{
	return IsConsoleMutationAllowed();
}

void UPGXGameFlowSubsystem::OverrideResolvedMaxHistoryDepthForTesting(int32 MaxHistoryDepth)
{
	ResolvedMaxHistoryDepth = FMath::Max(1, MaxHistoryDepth);
	const int32 MaxDepth = GetResolvedMaxHistoryDepth();
	for (int32 i = 0; i < PGX_FLOW_CHANNEL_COUNT; ++i)
	{
		FPGXFlowChannelState& State = ChannelStates[i];
		while (State.History.Num() > MaxDepth)
		{
			State.History.RemoveAt(0);
		}
		while (State.LastTagHistory.Num() > MaxDepth)
		{
			State.LastTagHistory.RemoveAt(0);
		}
	}
}
#endif
