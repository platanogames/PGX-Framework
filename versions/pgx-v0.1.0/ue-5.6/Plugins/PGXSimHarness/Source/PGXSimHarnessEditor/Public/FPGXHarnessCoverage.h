// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NameTypes.h"
#include "FPGXHarnessCoverage.generated.h"

class UWorld;

/**
 * EN: Depth of plugin presence verification (ordered: NotPresent → Verified).
 *
 *     NotPresent:       Module/plugin not loaded
 *     PluginLoaded:     .uplugin enabled + module loaded
 *     SubsystemExists:  Subsystem class is reflectable + Get() returns non-null
 *     Initialized:      Subsystem Initialize() was called without crash (world-level)
 *     Verified:         Reserved; the current harness does not emit this depth.
 *
 * ES: Profundidad de la verificacion de presencia del plugin (ordenada).
 */
UENUM(BlueprintType)
enum class EPGXVerificationDepth : uint8
{
	NotPresent       UMETA(DisplayName = "Not Present"),
	PluginLoaded     UMETA(DisplayName = "Plugin Loaded"),
	SubsystemExists  UMETA(DisplayName = "Subsystem Exists"),
	// PresenceOnly means the subsystem class is reflectable AND Get() returns non-null,
	// but the body is empty/trivial (extended presence-only plugins: Animation, Cinematic,
	// Materials, VFX). Between SubsystemExists and Initialized in the depth
	// progression. Used by FPGXHarnessCoverage::VerifyPluginPresence when a
	// plugin is in the presence-only set — makes the absence of runtime deep
	// coverage explicit rather than collapsing to Initialized (which would
	// falsely imply working initialization code).
	PresenceOnly     UMETA(DisplayName = "Presence Only (Presence-onlys)"),
	Initialized      UMETA(DisplayName = "Initialized"),
	Verified         UMETA(DisplayName = "Verified (Reserved)")
};

/**
 * EN: Result of a single presence check or aggregated depth verdict.
 *
 *     Pending:        Not yet evaluated (default for newly-added entries)
 *     Pass:           Check passed
 *     Fail:           Check failed (e.g., .uplugin not enabled)
 *     Skipped:        Check intentionally skipped (e.g., subsystem exists but
 *                     world not available during the check window)
 *     NotApplicable:  Check does not apply to this plugin (e.g., tools-only
 *                     plugins without runtime subsystem class)
 *
 * ES: Resultado de un check individual o veredicto agregado.
 */
UENUM(BlueprintType)
enum class EPGXVerificationResult : uint8
{
	Pending        UMETA(DisplayName = "Pending"),
	Pass           UMETA(DisplayName = "Pass"),
	Fail           UMETA(DisplayName = "Fail"),
	Skipped        UMETA(DisplayName = "Skipped"),
	NotApplicable  UMETA(DisplayName = "Not Applicable")
};

/**
 * EN: Single plugin's coverage entry.
 *
 *     PluginName:        Plugin identifier (e.g., "PGXSpawn")
 *     SubsystemClassNames: C++ class name(s) for the plugin's runtime
 *                          subsystem(s); empty when plugin has no runtime class
 *     Depth:             Highest verification depth achieved
 *     Result:            Aggregated verdict across CheckResults
 *     Detail:            Human-readable explanation (e.g., ".uplugin disabled",
 *                        "UWorldSubsystem not found in world")
 *     CheckResults:      Per-check pass/fail array. Index 0 = .uplugin enabled,
 *                        Index 1 = subsystem class reflectable + Get() non-null,
 *                        Index 2 = subsystem Initialize() safe
 *     PassCount:         Number of CheckResults equal to Pass
 *     TotalChecks:       Length of CheckResults (or count of applicable checks)
 *
 * ES: Entrada de cobertura de un plugin individual.
 */
USTRUCT(BlueprintType)
struct PGXSIMHARNESSEDITOR_API FPGXPluginCoverageEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Coverage")
	FName PluginName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Coverage")
	TArray<FName> SubsystemClassNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Coverage")
	EPGXVerificationDepth Depth = EPGXVerificationDepth::NotPresent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Coverage")
	EPGXVerificationResult Result = EPGXVerificationResult::Pending;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Coverage")
	FString Detail;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Coverage")
	TArray<EPGXVerificationResult> CheckResults;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Coverage")
	int32 PassCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Coverage")
	int32 TotalChecks = 0;
};

/**
 * EN: Catalog-level coverage matrix and presence check helper.
 *
 *     Provides a static API for verifying the presence of PGX plugins in the
 *     loaded editor session. Used by:
 *       - FPGXVisualHarness::GetCoverageMatrix() — 33-plugin panorama
 *       - PGX inspector widgets and NomadTab panels
 *       - SimHarness setup verification pass
 *
 *     This class is read-only / non-mutating runtime: it does not load
 *     plugins or subsystems (subsystems that the harness needs to run are
 *     loaded by FPGXVisualHarness::Inject*). It observes what is already
 *     available in the editor.
 *
 *     Scope boundaries (baseline support, for the coverage model):
 *       IN scope:  enums, struct, presence check, coverage matrix get
 *       OUT scope: Inject*, Teardown*, VerifyAllAPIs — owned by tool-specific
 *
 * ES: Helper para catalogar cobertura y verificar presencia de plugins.
 */
class PGXSIMHARNESSEDITOR_API FPGXHarnessCoverage
{
public:
	/**
	 * EN: Get coverage matrix for all known PGX plugins. Returns one
	 *     FPGXPluginCoverageEntry per plugin. Entries are evaluated against
	 *     the supplied World (if non-null); plugins with no World context get
	 *     shallower Depth verdicts (PluginLoaded is the maximum without
	 *     World resolution).
	 *
	 *     EN: World — optional editor world for subsystem resolution.
	 *         Pass nullptr to do plugin-loaded-only verification.
	 */
	static TArray<FPGXPluginCoverageEntry> GetCoverageMatrix(UWorld* World = nullptr);

	/**
	 * EN: Verify presence for a single plugin (by FName, e.g., "PGXSpawn").
	 *
	 *     EN: PluginName — case-sensitive plugin FName matching IPluginManager.
	 *         World — optional editor world for subsystem resolution.
	 */
	static FPGXPluginCoverageEntry VerifyPluginPresence(FName PluginName, UWorld* World = nullptr);

	/**
	 * EN: Number of plugins in the canonical coverage scope (coverage model).
	 *
	 *     Used by panels and automation tests to verify coverage completeness.
	 */
	static int32 GetCanonicalPluginCount();

private:
	/**
	 * EN: Catalog entry for a single subsystem — class path + container type.
	 *
	 *     Used internally to look up a UWorldSubsystem vs UGameInstanceSubsystem
	 *     for resolution under the supplied World context.
	 */
	struct FPGXSubsystemRef
	{
		FString ClassPath;
		bool bIsWorldSubsystem;
	};

	/**
	 * EN: Internal map of plugin name → list of subsystem refs.
	 *
	 *     Built once via static initialization; covers all 23 plugins with
	 *     subsystem classes among the 33 canonical plugins. The remaining
	 *     10 plugins have no runtime subsystem class because they are tools-only,
	 *     component-only, or do not expose a runtime subsystem.
	 */
	static const TMap<FName, TArray<FPGXSubsystemRef>>& GetPluginSubsystemRefs();

	/**
	 * EN: Canonical ordered list of 33 plugin names for coverage enumeration.
	 *
	 *     Order is curated to match the coverage model gap analysis grouping
	 *     (covered core → implemented → partial → presence-only → manifest-missing
	 *     → tools/docs → internal/defer).
	 */
	static const TArray<FName>& GetCanonicalPlugins();
};
