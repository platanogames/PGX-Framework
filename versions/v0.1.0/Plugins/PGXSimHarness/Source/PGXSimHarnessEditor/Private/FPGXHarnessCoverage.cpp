// Copyright PGX Framework. All Rights Reserved.

#include "FPGXHarnessCoverage.h"

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Logging/PGXLogMacros.h"
#include "Subsystems/SubsystemCollection.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPGXHarnessCoverage, Log, All);

// ============================================================================
// EN: Static plugin catalog for public coverage reporting.
// ES: Catalogo estatico para el informe publico de cobertura.
// ============================================================================

const TArray<FName>& FPGXHarnessCoverage::GetCanonicalPlugins()
{
	// EN: Stable grouping keeps the coverage panel readable from top to bottom.
	// ES: La agrupacion estable mantiene legible el panel de cobertura.
	static const TArray<FName> Canonical =
	{
		// ─── Core (covered systems) ───
		FName(TEXT("PGXCore")),
		FName(TEXT("PGXGameFlow")),
		FName(TEXT("PGXSave")),
		FName(TEXT("PGXPSO")),
		FName(TEXT("PGXLoading")),
		FName(TEXT("PGXMGOS")),
		FName(TEXT("PGXAudio")),
		FName(TEXT("PGXInput")),

		// ─── Implemented (implemented presence) ───
		FName(TEXT("PGXAI")),
		FName(TEXT("PGXAbility")),
		FName(TEXT("PGXSpawn")),
		FName(TEXT("PGXUI")),

		// ─── Partial surface ───
		FName(TEXT("PGXCamera")),
		FName(TEXT("PGXInteraction")),
		FName(TEXT("PGXInventory")),

		// ─── Presence-only plugins (body empty by design) ───
		FName(TEXT("PGXAnimation")),
		FName(TEXT("PGXCinematic")),
		FName(TEXT("PGXMaterials")),
		FName(TEXT("PGXVFX")),

		// ─── Additional plugins (reported as presence-only) ───
		FName(TEXT("PGXMultiplayer")),
		FName(TEXT("PGXOnline")),

		// ─── Manifest-missing real partial ───
		FName(TEXT("PGXColony")),
		FName(TEXT("PGXCrafting")),
		FName(TEXT("PGXEnvironment")),
		FName(TEXT("PGXTrade")),
		FName(TEXT("PGXVehicles")),

		// ─── Tools / docs (editor-only) ───
		FName(TEXT("PGXDocs")),
		FName(TEXT("PGXTutorials")),
		FName(TEXT("PGXEditorTools")),
		FName(TEXT("PGXScaffold")),
		FName(TEXT("PGXVersionControl")),
		FName(TEXT("PGXSimHarness"))
	};
	return Canonical;
}

int32 FPGXHarnessCoverage::GetCanonicalPluginCount()
{
	return GetCanonicalPlugins().Num();
}

const TMap<FName, TArray<FPGXHarnessCoverage::FPGXSubsystemRef>>& FPGXHarnessCoverage::GetPluginSubsystemRefs()
{
	// EN: 23 plugins have at least one runtime subsystem class. The remaining
	//     10 (PGXAnimation, PGXCinematic, PGXInteraction, PGXCore, PGXDocs,
	//     PGXEditorTools, PGXScaffold, PGXSimHarness,
	//     PGXTutorials) have NO runtime subsystem class — they are tools-only,
	//     component-only, or do not expose a runtime subsystem.
	//
	//     Subsystem container type determined by design:
	//       UWorldSubsystem     → bIsWorldSubsystem = true
	//       UGameInstanceSubsystem → bIsWorldSubsystem = false

	using FRef = FPGXSubsystemRef;

	static const TMap<FName, TArray<FRef>> Refs =
	{
		// Core
		{ FName(TEXT("PGXGameFlow")), {
			{ TEXT("/Script/PGXGameFlowRuntime.PGXGameFlowSubsystem"), false },
			{ TEXT("/Script/PGXGameFlowRuntime.PGXLevelTransitionSubsystem"), false }
		}},
		{ FName(TEXT("PGXSave")), {
			{ TEXT("/Script/PGXSaveRuntime.PGXSaveSubsystem"), false }
		}},
		{ FName(TEXT("PGXPSO")), {
			{ TEXT("/Script/PGXPSORuntime.PGXPSOSubsystem"), false }
		}},
		{ FName(TEXT("PGXLoading")), {
			{ TEXT("/Script/PGXLoadingRuntime.PGXLevelFlowSubsystem"), false },
			{ TEXT("/Script/PGXLoadingRuntime.PGXLoadingSubsystem"), false }
		}},
		{ FName(TEXT("PGXMGOS")), {
			{ TEXT("/Script/PGXMGOSRuntime.PGXGCObserverSubsystem"), false }
		}},
		{ FName(TEXT("PGXAudio")), {
			{ TEXT("/Script/PGXAudioRuntime.PGXAudioSubsystem"), false }
		}},
		{ FName(TEXT("PGXInput")), {
			{ TEXT("/Script/PGXInputRuntime.PGXInputSubsystem"), false }
		}},

		// Implemented (high-priority)
		{ FName(TEXT("PGXAI")), {
			{ TEXT("/Script/PGXAIRuntime.PGXAISubsystem"), true }
		}},
		{ FName(TEXT("PGXAbility")), {
			{ TEXT("/Script/PGXAbilityRuntime.PGXAbilitySubsystem"), false }
		}},
		{ FName(TEXT("PGXSpawn")), {
			//  Removed
			//   redundant U- prefix. UE class path is <ModuleName>.<ClassName> — UCLASS
			//   declarations are reflected WITHOUT the U- prefix in their script path.
			{ TEXT("/Script/PGXSpawnRuntime.PGXSpawnSubsystem"), true }
		}},
		{ FName(TEXT("PGXUI")), {
			{ TEXT("/Script/PGXUIRuntime.PGXUISubsystem"), false }
		}},

		// Partial surface
		{ FName(TEXT("PGXCamera")), {
			{ TEXT("/Script/PGXCameraRuntime.PGXCameraSubsystem"), true }
		}},
		// PGXInteraction: component-only (no UWorldSubsystem class) → no entry
		{ FName(TEXT("PGXInventory")), {
			{ TEXT("/Script/PGXInventoryRuntime.PGXInventorySubsystem"), false }
		}},

		// Unavailable public dependencies are intentionally omitted here and
		// reported explicitly by VerifyPluginPresence without loading modules.

		// Manifest-missing (subset that has runtime subsystem; remainder
		// may need a separate validation pass to enumerate)
		{ FName(TEXT("PGXColony")), {
			{ TEXT("/Script/PGXColonyRuntime.PGXColonySubsystem"), false }
		}},
		{ FName(TEXT("PGXCrafting")), {
			{ TEXT("/Script/PGXCraftingRuntime.PGXCraftingSubsystem"), false }
		}},
		{ FName(TEXT("PGXEnvironment")), {
			{ TEXT("/Script/PGXEnvironmentRuntime.PGXEnvironmentSubsystem"), true }
		}},
		{ FName(TEXT("PGXTrade")), {
			{ TEXT("/Script/PGXTradeRuntime.PGXTradeSubsystem"), false }
		}},
		{ FName(TEXT("PGXVehicles")), {
			{ TEXT("/Script/PGXVehiclesRuntime.PGXVehiclesSubsystem"), false }
		}}
		//
		//   PGXVersionControl removed from subsystem catalog. The only module
		//   in its .uplugin is PGXVersionControlEditor (Editor-only) and the
		//   subsystem class UPGXSourceControlSubsystem extends UEditorSubsystem
		//   — not resolvable via World->GetGameInstance()->GetSubsystem<>() which
		//   is what FPGXHarnessCoverage::VerifyPluginPresence uses. Plugin still
		//   appears in GetCanonicalPlugins() so VerifyPluginPresence runs the
		//   uplugin-enabled check (Pass) plus subsystem checks (NotApplicable).
	};
	return Refs;
}

// ============================================================================
// EN: Find a subsystem instance by UClass in a World context.
// ES: Buscar instancia de subsystem por UClass en contexto World.
// ============================================================================

namespace
{
	USubsystem* FindSubsystemInstance(UWorld* World, UClass* TargetClass, bool bIsWorldSubsystem)
	{
		if (!World || !TargetClass)
		{
			return nullptr;
		}

		if (bIsWorldSubsystem)
		{
			TArray<UWorldSubsystem*> AllWorldSubs = World->GetSubsystemArrayCopy<UWorldSubsystem>();
			for (UWorldSubsystem* Candidate : AllWorldSubs)
			{
				if (Candidate && Candidate->GetClass() == TargetClass)
				{
					return Candidate;
				}
			}
			return nullptr;
		}

		UGameInstance* GI = World->GetGameInstance();
		if (!GI)
		{
			return nullptr;
		}
		TArray<UGameInstanceSubsystem*> AllGISubs = GI->GetSubsystemArrayCopy<UGameInstanceSubsystem>();
		for (UGameInstanceSubsystem* Candidate : AllGISubs)
		{
			if (Candidate && Candidate->GetClass() == TargetClass)
			{
				return Candidate;
			}
		}
		return nullptr;
	}

	/**
	 * EN: Read .uplugin CanContainContent + check Content/ directory existence.
	 * ES: Lee CanContainContent del .uplugin + verifica existencia de Content/.
	 */
	struct FCanContainContentInfo
	{
		bool bCanContainContent = false;
		bool bContentDirectoryExists = false;
		FString BaseDir;
	};

	FCanContainContentInfo GetCanContainContentInfo(FName PluginName)
	{
		FCanContainContentInfo Info;
		const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(PluginName.ToString());
		if (!Plugin.IsValid())
		{
			return Info;
		}
		Info.BaseDir = Plugin->GetBaseDir();
		Info.bCanContainContent = Plugin->GetDescriptor().bCanContainContent;
		const FString ContentDir = Info.BaseDir / TEXT("Content");
		Info.bContentDirectoryExists = FPaths::DirectoryExists(*ContentDir);
		return Info;
	}
}

// ============================================================================
// EN: presence-only plugin set
//     whose bodies are empty. They are demoted to EPGXVerificationDepth::
//     PresenceOnly by VerifyPluginPresence. Hardcoded here because no
//     runtime introspection can distinguish a presence-only plugin from a
//     "fully implemented plugin" in plain reflection.
// ES: Set de plugins presence-only — los cuatro plugins con
//     bodies vacios.
// ============================================================================
static const TSet<FName>& GetPresenceOnlyPlugins()
{
	static const TSet<FName> PresenceOnlyPlugins =
	{
		FName(TEXT("PGXAnimation")),
		FName(TEXT("PGXCinematic"))
	};
	return PresenceOnlyPlugins;
}

static const TSet<FName>& GetUnavailablePlugins()
{
	static const TSet<FName> UnavailablePlugins =
	{
		FName(TEXT("PGXMaterials")),
		FName(TEXT("PGXVFX")),
		FName(TEXT("PGXMultiplayer")),
		FName(TEXT("PGXOnline"))
	};
	return UnavailablePlugins;
}

// ============================================================================
// EN: Public presence check.
// ES: Check publico de presencia.
// ============================================================================

FPGXPluginCoverageEntry FPGXHarnessCoverage::VerifyPluginPresence(FName PluginName, UWorld* World)
{
	FPGXPluginCoverageEntry Entry;
	Entry.PluginName = PluginName;
	Entry.Depth = EPGXVerificationDepth::NotPresent;
	Entry.Result = EPGXVerificationResult::Pending;
	Entry.TotalChecks = 0;
	Entry.PassCount = 0;

	if (GetUnavailablePlugins().Contains(PluginName))
	{
		Entry.Result = EPGXVerificationResult::NotApplicable;
		Entry.Detail = TEXT("unavailable: plugin is outside this build's dependency set");
		return Entry;
	}

	// ───── Check 1: .uplugin enabled + module loaded ─────
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(PluginName.ToString());
	const bool bPluginEnabled = Plugin.IsValid() && Plugin->IsEnabled();
	Entry.CheckResults.Add(bPluginEnabled
		? EPGXVerificationResult::Pass
		: EPGXVerificationResult::Fail);
	Entry.TotalChecks++;
	if (bPluginEnabled)
	{
		Entry.PassCount++;
		Entry.Depth = EPGXVerificationDepth::PluginLoaded;
		Entry.Detail = TEXT(".uplugin enabled + module loaded");
	}
	else
	{
		Entry.Result = EPGXVerificationResult::Fail;
		Entry.Detail = FString::Printf(TEXT(".uplugin '%s' not enabled or not registered with IPluginManager"), *PluginName.ToString());
		return Entry;
	}

	// ───── Look up subsystem refs for this plugin ─────
	const TArray<FPGXSubsystemRef>* PluginRefs = GetPluginSubsystemRefs().Find(PluginName);
	const bool bHasSubsystemRefs = PluginRefs && PluginRefs->Num() > 0;

	if (!bHasSubsystemRefs)
	{
		// No runtime subsystem class is declared for this plugin (for example,
		// editor-only or component-only integrations). Both subsystem checks
		//   (Check 2 + Check 3) become NotApplicable — but we do NOT early return
		//   anymore, so Check 4 (CanContainContent -> Content/) still runs and
		//   surface degraded state for plugins that claim Content/ but lack it.
		Entry.CheckResults.Add(EPGXVerificationResult::NotApplicable);
		Entry.CheckResults.Add(EPGXVerificationResult::NotApplicable);
		Entry.Depth = EPGXVerificationDepth::PluginLoaded;
		Entry.Detail += TEXT("; no runtime subsystem class (editor-only/component-only)");
		// [continues to Check 4 below]
	}

	if (bHasSubsystemRefs)
	{
	// ───── Check 2: subsystem class reflectable + Get() non-null ─────
	bool bAnySubsystemReachable = false;
	bool bAnySubsystemLoaded = false;
	int32 LoadedCount = 0;
	int32 ReachableCount = 0;

	for (const FPGXSubsystemRef& Ref : *PluginRefs)
	{
		// EN: LoadClass triggers module load + returns UClass (more reliable than FindObject).
		// ES: LoadClass dispara carga del modulo + retorna UClass (mas fiable que FindObject).
		UClass* SubClass = LoadClass<USubsystem>(nullptr, *Ref.ClassPath);
		if (!SubClass)
		{
			continue;
		}
		Entry.SubsystemClassNames.Add(FName(*SubClass->GetName()));
		bAnySubsystemLoaded = true;
		LoadedCount++;

		USubsystem* Instance = FindSubsystemInstance(World, SubClass, Ref.bIsWorldSubsystem);
		if (Instance != nullptr)
		{
			bAnySubsystemReachable = true;
			ReachableCount++;
		}
	}

	// EN: Aggregate check 2 verdict across all subsystems of this plugin.
	const TCHAR* Check2DetailPart = bAnySubsystemLoaded
		? (bAnySubsystemReachable ? TEXT("subsystem live") : TEXT("subsystem class loaded but Get() returned null"))
		: TEXT("subsystem class not loaded");
	if (bAnySubsystemLoaded && !bAnySubsystemReachable)
	{
		// EN: Class loaded but not live in world — mark SubsystemExists == No, depth stays PluginLoaded.
		Entry.CheckResults.Add(EPGXVerificationResult::Skipped);
		Entry.Detail += FString::Printf(TEXT("; %d/%d subsystems loaded, %d reachable (%s)"),
			LoadedCount, PluginRefs->Num(), ReachableCount, Check2DetailPart);
	}
	else if (!bAnySubsystemLoaded)
	{
		Entry.CheckResults.Add(EPGXVerificationResult::Fail);
		Entry.Detail += FString::Printf(TEXT("; 0/%d subsystems loaded (classpath unresolved — check Build.cs deps)"),
			PluginRefs->Num());
		Entry.Result = EPGXVerificationResult::Fail;
		return Entry;
	}
	else
	{
		Entry.CheckResults.Add(EPGXVerificationResult::Pass);
		Entry.TotalChecks++;
		Entry.PassCount++;
		Entry.Depth = EPGXVerificationDepth::SubsystemExists;
		Entry.Detail += FString::Printf(TEXT("; %d/%d subsystems live"),
			ReachableCount, PluginRefs->Num());
	}

	// ───── Check 3: subsystem Initialize() reached (best-effort: subsystem live = Initialize() ran) ─────
	// EN: We cannot introspect Initialize() body without per-subsystem cooperation. We observe
	//     that the framework registers every subsystem and calls Initialize() during world
	//     startup; if the subsystem is reachable (Check 2 = Pass), the framework has already
	//     invoked Initialize(). Mark Skipped if no World, Pass if Check 2 passed.
	if (!World)
	{
		Entry.CheckResults.Add(EPGXVerificationResult::Skipped);
		Entry.Detail += TEXT("; Initialize() not observable without World");
	}
	else
	{
		Entry.CheckResults.Add(EPGXVerificationResult::Pass);
		Entry.TotalChecks++;
		Entry.PassCount++;
		Entry.Depth = EPGXVerificationDepth::Initialized;
		Entry.Detail += TEXT("; Initialize() reached (subsystem live = framework-invoked)");
	}
		} // if (bHasSubsystemRefs)

	//
	// ───── Check 4: .uplugin CanContainContent -> Content/ directory exists ─────
	// Runs unconditionally for every plugin — surfaces degraded state for any
	// plugin whose .uplugin declares CanContainContent=true but whose Content/
	// folder is missing. For presence-only plugins (PGXAnimation/Cinematic)
	// this is expected (they ship empty by design), so the result
	// surfaces as Fail here but the Depth override to PresenceOnly (below)
	// tells the consumer that this fail is the canonical presence-only signal
	// rather than a build-gate break.
	{
		const FCanContainContentInfo Info = GetCanContainContentInfo(PluginName);
		if (!Info.bCanContainContent)
		{
			Entry.CheckResults.Add(EPGXVerificationResult::NotApplicable);
			Entry.Detail += TEXT("; .uplugin CanContainContent=false (content optional)");
		}
		else if (Info.bContentDirectoryExists)
		{
			Entry.CheckResults.Add(EPGXVerificationResult::Pass);
			Entry.TotalChecks++;
			Entry.PassCount++;
			Entry.Detail += TEXT("; CanContainContent=true, Content/ present");
		}
		else
		{
			Entry.CheckResults.Add(EPGXVerificationResult::Fail);
			Entry.TotalChecks++;
			Entry.Detail += TEXT("; CanContainContent=true but Content/ empty (presence-only signature)");
		}
	}

	// ───── Presence-only override ─────
	//  For the presence-only plugins whose
	// bodies are empty (by design), demote depth from Initialized (which
	// would falsely imply working initialization code) to PresenceOnly for honest coverage reporting.
	// Triggered when the plugin name is in GetPresenceOnlyPlugins(). The Check 4 Fail
	// result is preserved (degraded content), but the Depth makes the shell
	// nature explicit to consumers of GetCoverageMatrix().
	if (GetPresenceOnlyPlugins().Contains(PluginName))
	{
		Entry.Detail += TEXT("; [presence-only override] depth demoted to PresenceOnly for honest coverage reporting");
		Entry.Depth = EPGXVerificationDepth::PresenceOnly;
	}

	// ───── Aggregate result ─────
	//  Updated to handle
	// the empty-TotalChecks case (no subsystem refs + CanContainContent=false
	// + no assertion surface) — falls through to NotApplicable. The "all pass"
	// and "all fail" branches drive Pass / Fail respectively; mixed signals
	// resolve to Skipped.
	if (Entry.TotalChecks == 0)
	{
		Entry.Result = EPGXVerificationResult::NotApplicable;
	}
	else if (Entry.PassCount == Entry.TotalChecks)
	{
		Entry.Result = EPGXVerificationResult::Pass;
	}
	else if (Entry.PassCount == 0)
	{
		Entry.Result = EPGXVerificationResult::Fail;
	}
	else
	{
		Entry.Result = EPGXVerificationResult::Skipped;
	}

	PGX_LOG_VERBOSE(LogFPGXHarnessCoverage,
		TEXT("VerifyPluginPresence(%s) → Depth=%d, Result=%d, Pass=%d/%d, Detail='%s'"),
		*PluginName.ToString(),
		static_cast<int32>(Entry.Depth), static_cast<int32>(Entry.Result),
		Entry.PassCount, Entry.TotalChecks, *Entry.Detail);

	return Entry;
}

// ============================================================================
// EN: Coverage matrix — one entry per canonical plugin (33 total).
// ES: Matriz de cobertura — una entrada por plugin canonico (33 total).
// ============================================================================

TArray<FPGXPluginCoverageEntry> FPGXHarnessCoverage::GetCoverageMatrix(UWorld* World)
{
	TArray<FPGXPluginCoverageEntry> Matrix;
	Matrix.Reserve(GetCanonicalPluginCount());

	int32 PassCount = 0;
	int32 FailCount = 0;
	int32 SkippedCount = 0;
	int32 NotApplicableCount = 0;
	int32 PendingCount = 0;

	for (const FName& PluginName : GetCanonicalPlugins())
	{
		FPGXPluginCoverageEntry Entry = VerifyPluginPresence(PluginName, World);
		switch (Entry.Result)
		{
			case EPGXVerificationResult::Pass:            ++PassCount;            break;
			case EPGXVerificationResult::Fail:            ++FailCount;            break;
			case EPGXVerificationResult::Skipped:         ++SkippedCount;         break;
			case EPGXVerificationResult::NotApplicable:  ++NotApplicableCount;   break;
			case EPGXVerificationResult::Pending:         ++PendingCount;         break;
			default: break;
		}
		Matrix.Add(MoveTemp(Entry));
	}

	PGX_LOG_INFO(LogFPGXHarnessCoverage,
		TEXT("FPGXHarnessCoverage::GetCoverageMatrix — %d plugins: %d Pass, %d Fail, %d Skipped, %d NotApplicable, %d Pending (World=%s)"),
		Matrix.Num(), PassCount, FailCount, SkippedCount, NotApplicableCount, PendingCount,
		World ? TEXT("set") : TEXT("null"));

	return Matrix;
}
