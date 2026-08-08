// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Profile/PGXProfileSubsystem.h"
#include "Profile/PGXProfileTags.h"
#include "Profile/PGXProjectProfileConfig.h"
#include "Profile/PGXPlatformConfig.h"
#include "Profile/PGXProfileSettings.h"
#include "Logging/PGXLogCategories.h"

// AssetRegistry for auto-discovery
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"

// EN: Profile Subsystem — complete implementation (discovery, resolution, console, simulation)
// ES: Subsistema de Profile — implementacion completa (descubrimiento, resolucion, consola, simulacion)

// ============================================================================
// Static
// ============================================================================

TWeakObjectPtr<UPGXProfileSubsystem> UPGXProfileSubsystem::CachedInstance = nullptr;

// ============================================================================
// Lifecycle
// ============================================================================

void UPGXProfileSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CachedInstance = this;

	// EN: Step 1 — Discover all profile config assets via AssetRegistry
	// ES: Paso 1 — Descubrir todos los config assets de profile via AssetRegistry
	DiscoverProfiles();

	// EN: Step 2 — Read active profile from Project Settings
	// ES: Paso 2 — Leer profile activo desde Project Settings
	const UPGXProfileSettings* Settings = GetDefault<UPGXProfileSettings>();
	if (Settings && !Settings->ActiveProfile.IsNull())
	{
		ActiveConfig = Settings->ActiveProfile.LoadSynchronous();
	}

	// EN: Step 3 — If no active config set but we discovered exactly one, use it
	// ES: Paso 3 — Si no hay config activo pero descubrimos exactamente uno, usarlo
	if (!ActiveConfig && DiscoveredConfigs.Num() == 1)
	{
		ActiveConfig = DiscoveredConfigs[0];
		UE_LOG(LogPGX, Log, TEXT("[ProfileSubsystem] Auto-selected single discovered profile: %s"),
			*ActiveConfig->GetName());
	}

	// EN: Step 4 — Resolve the profile
	// ES: Paso 4 — Resolver el profile
	if (ActiveConfig)
	{
		ResolvedProfile = ResolveProfile(ActiveConfig);
		ResolvedProfile.State = EPGXProfileState::Resolved;
		ResolvedProfile.ResolvedTimestamp = FDateTime::UtcNow();

		UE_LOG(LogPGX, Log, TEXT("[ProfileSubsystem] Profile resolved from: %s | Mode: %d | Targets: %d | Build: %d"),
			*ActiveConfig->GetName(),
			static_cast<int32>(ResolvedProfile.Identity.ProjectMode),
			ResolvedProfile.Identity.ActiveTargets.Num(),
			static_cast<int32>(ResolvedProfile.Identity.BuildContext));
	}
	else if (Settings && Settings->bUseSafeDefaultsWhenNoProfile)
	{
		ResolvedProfile = FPGXResolvedProfile::MakeDefaultPermissive();
		UE_LOG(LogPGX, Log, TEXT("[ProfileSubsystem] No profile configured, using safe defaults (all allowed, no budgets)"));
	}
	else
	{
		ResolvedProfile.State = EPGXProfileState::Unresolved;
		UE_LOG(LogPGX, Warning, TEXT("[ProfileSubsystem] No profile configured and safe defaults disabled. Profile is UNRESOLVED."));
	}

	// EN: Step 5 — Load platform configs from active profile DA
	// ES: Paso 5 — Cargar configs de plataforma desde el DA de profile activo
	LoadPlatformConfigs();

	// EN: Step 6 — Register console commands
	// ES: Paso 6 — Registrar comandos de consola
	RegisterConsoleCommands();

	// EN: Step 7 — Broadcast resolution delegates
	// ES: Paso 7 — Broadcast delegados de resolucion
	if (ResolvedProfile.State == EPGXProfileState::Resolved)
	{
		OnProfileResolvedNative.Broadcast(ResolvedProfile);
		OnProfileResolved.Broadcast(ResolvedProfile);
	}

	// EN: Step 8 — Lock in Shipping builds
	// ES: Paso 8 — Bloquear en builds Shipping
#if UE_BUILD_SHIPPING
	LockProfile();
#endif

	UE_LOG(LogPGX, Log, TEXT("[ProfileSubsystem] Initialized. Discovered: %d configs. State: %d"),
		DiscoveredConfigs.Num(), static_cast<int32>(ResolvedProfile.State));
}

void UPGXProfileSubsystem::Deinitialize()
{
	UnregisterConsoleCommands();

	CachedInstance = nullptr;
	ActivePlatformConfig = nullptr;
	LoadedPlatformConfigs.Empty();
	ActiveConfig = nullptr;
	DiscoveredConfigs.Empty();
	ResolvedProfile = FPGXResolvedProfile();

	Super::Deinitialize();
}

// ============================================================================
// Query API
// ============================================================================

bool UPGXProfileSubsystem::IsCapabilityEnabled(FName CapabilityName) const
{
	return ResolvedProfile.IsCapabilityAllowed(CapabilityName);
}

bool UPGXProfileSubsystem::IsFeatureAllowed(FName FeatureName) const
{
	return ResolvedProfile.IsFeatureAllowed(FeatureName);
}

int64 UPGXProfileSubsystem::GetBudget(FName BudgetName) const
{
	return ResolvedProfile.GetBudget(BudgetName);
}

EPGXPersistenceBackend UPGXProfileSubsystem::GetPersistenceBackend(FName SystemName) const
{
	return ResolvedProfile.GetPersistenceBackend(SystemName);
}

bool UPGXProfileSubsystem::IsProfileResolved() const
{
	return ResolvedProfile.State == EPGXProfileState::Resolved
		|| ResolvedProfile.State == EPGXProfileState::Locked;
}

// ============================================================================
// Platform Config API (v2.0)
// ============================================================================

const UPGXPlatformConfig* UPGXProfileSubsystem::GetActivePlatformConfig() const
{
	return ActivePlatformConfig;
}

const UPGXPlatformConfig* UPGXProfileSubsystem::GetPlatformConfigFor(EPGXTargetPlatform Platform) const
{
	const TObjectPtr<UPGXPlatformConfig>* Found = LoadedPlatformConfigs.Find(Platform);
	return Found ? Found->Get() : nullptr;
}

void UPGXProfileSubsystem::LoadPlatformConfigs()
{
	if (!IsValid(ActiveConfig))
	{
		return;
	}

	// EN: Load all platform configs referenced by the active profile
	// ES: Cargar todas las configs de plataforma referenciadas por el profile activo
	for (const auto& Pair : ActiveConfig->PlatformConfigs)
	{
		if (!Pair.Value.IsNull())
		{
			UPGXPlatformConfig* Loaded = Pair.Value.LoadSynchronous();
			if (IsValid(Loaded))
			{
				LoadedPlatformConfigs.Add(Pair.Key, Loaded);
				UE_LOG(LogPGX, Log, TEXT("[ProfileSubsystem] Loaded PlatformConfig for %d: %s"),
					static_cast<int32>(Pair.Key), *Loaded->GetName());
			}
			else
			{
				UE_LOG(LogPGX, Warning, TEXT("[ProfileSubsystem] Failed to load PlatformConfig for platform %d"),
					static_cast<int32>(Pair.Key));
			}
		}
	}

	// EN: Determine active platform config — use first active target that has a config
	// ES: Determinar config de plataforma activa — usar primer target activo que tenga config
	for (const EPGXTargetPlatform& Target : ResolvedProfile.Identity.ActiveTargets)
	{
		if (const TObjectPtr<UPGXPlatformConfig>* Found = LoadedPlatformConfigs.Find(Target))
		{
			ActivePlatformConfig = *Found;
			UE_LOG(LogPGX, Log, TEXT("[ProfileSubsystem] Active PlatformConfig set for platform %d: %s"),
				static_cast<int32>(Target), *ActivePlatformConfig->GetName());
			break;
		}
	}

	if (!IsValid(ActivePlatformConfig) && LoadedPlatformConfigs.Num() > 0)
	{
		UE_LOG(LogPGX, Log, TEXT("[ProfileSubsystem] No PlatformConfig matches active targets. %d configs loaded but none active."),
			LoadedPlatformConfigs.Num());
	}
}

// ============================================================================
// Discovery
// ============================================================================

void UPGXProfileSubsystem::DiscoverProfiles()
{
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();

	FARFilter Filter;
	Filter.ClassPaths.Add(UPGXProjectProfileConfig::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true;

	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssets(Filter, AssetDataList);

	for (const FAssetData& AssetData : AssetDataList)
	{
		if (UPGXProjectProfileConfig* Config = Cast<UPGXProjectProfileConfig>(AssetData.GetAsset()))
		{
			DiscoveredConfigs.Add(Config);
		}
	}

	const UPGXProfileSettings* Settings = GetDefault<UPGXProfileSettings>();
	if (Settings && Settings->bVerboseProfileResolution)
	{
		UE_LOG(LogPGX, Log, TEXT("[ProfileSubsystem] Discovery: Found %d profile configs"), DiscoveredConfigs.Num());
		for (const auto& Config : DiscoveredConfigs)
		{
			UE_LOG(LogPGX, Log, TEXT("  - %s"), *Config->GetPathName());
		}
	}
}

// ============================================================================
// Resolution Engine
// ============================================================================

FPGXResolvedProfile UPGXProfileSubsystem::ResolveProfile(const UPGXProjectProfileConfig* Config) const
{
	check(Config);

	FPGXResolvedProfile Profile;

	// EN: Copy base layers from the config DA
	// ES: Copiar capas base del config DA
	Profile.Identity = Config->Identity;
	Profile.Capabilities = Config->BaseCapabilities;
	Profile.Policies = Config->BasePolicies;
	Profile.Budgets = Config->BaseBudgets;
	Profile.Features = Config->BaseFeatures;

	const UPGXProfileSettings* Settings = GetDefault<UPGXProfileSettings>();
	const bool bVerbose = Settings && Settings->bVerboseProfileResolution;

	// EN: Apply platform overrides for active targets
	// ES: Aplicar overrides de plataforma para targets activos
	ApplyPlatformOverrides(Config, Profile);

	// EN: Apply multi-platform resolution if multiple targets
	// ES: Aplicar resolucion multi-plataforma si hay multiples targets
	if (Profile.Identity.ActiveTargets.Num() > 1)
	{
		ApplyMultiPlatformResolution(Profile);
		if (bVerbose)
		{
			UE_LOG(LogPGX, Log, TEXT("[ProfileSubsystem] Multi-platform resolution applied for %d targets"),
				Profile.Identity.ActiveTargets.Num());
		}
	}

	// EN: Apply build context overrides
	// ES: Aplicar overrides de contexto de build
	ApplyBuildOverrides(Config, Profile);

	return Profile;
}

void UPGXProfileSubsystem::ApplyPlatformOverrides(const UPGXProjectProfileConfig* Config, FPGXResolvedProfile& OutProfile) const
{
	const UPGXProfileSettings* Settings = GetDefault<UPGXProfileSettings>();
	const bool bVerbose = Settings && Settings->bVerboseProfileResolution;

	for (const EPGXTargetPlatform& Target : OutProfile.Identity.ActiveTargets)
	{
		const FPGXPlatformOverride* Override = Config->PlatformOverrides.Find(Target);
		if (!Override) continue;

		if (Override->bOverrideCapabilities)
		{
			MergeCapabilitiesAND(OutProfile.Capabilities, Override->Capabilities);
			if (bVerbose) UE_LOG(LogPGX, Log, TEXT("[ProfileSubsystem] Platform %d: Capabilities override applied"), static_cast<int32>(Target));
		}

		if (Override->bOverrideBudgets)
		{
			MergeBudgetsMIN(OutProfile.Budgets, Override->Budgets);
			if (bVerbose) UE_LOG(LogPGX, Log, TEXT("[ProfileSubsystem] Platform %d: Budgets override applied"), static_cast<int32>(Target));
		}

		if (Override->bOverrideFeatures)
		{
			MergeFeaturesMostRestrictive(OutProfile.Features, Override->Features);
			if (bVerbose) UE_LOG(LogPGX, Log, TEXT("[ProfileSubsystem] Platform %d: Features override applied"), static_cast<int32>(Target));
		}
	}
}

void UPGXProfileSubsystem::ApplyBuildOverrides(const UPGXProjectProfileConfig* Config, FPGXResolvedProfile& OutProfile) const
{
	const FPGXBuildOverride* Override = Config->BuildOverrides.Find(OutProfile.Identity.BuildContext);
	if (!Override) return;

	const UPGXProfileSettings* Settings = GetDefault<UPGXProfileSettings>();
	const bool bVerbose = Settings && Settings->bVerboseProfileResolution;

	if (Override->bOverrideCapabilities)
	{
		MergeCapabilitiesAND(OutProfile.Capabilities, Override->Capabilities);
		if (bVerbose) UE_LOG(LogPGX, Log, TEXT("[ProfileSubsystem] Build %d: Capabilities override applied"), static_cast<int32>(OutProfile.Identity.BuildContext));
	}

	if (Override->bOverridePolicies)
	{
		// EN: Policies use "most restrictive" — for bools, OR means more restrictive
		// ES: Politicas usan "mas restrictivo" — para bools, OR significa mas restrictivo
		FPGXProfilePolicies& Base = OutProfile.Policies;
		const FPGXProfilePolicies& Ovr = Override->Policies;

		Base.Security.bRequireEncryption     = Base.Security.bRequireEncryption     || Ovr.Security.bRequireEncryption;
		Base.Security.bRequireCompression    = Base.Security.bRequireCompression    || Ovr.Security.bRequireCompression;
		Base.Security.bSensitiveDataHandling = Base.Security.bSensitiveDataHandling || Ovr.Security.bSensitiveDataHandling;
		Base.Security.bStripDebugStrings     = Base.Security.bStripDebugStrings     || Ovr.Security.bStripDebugStrings;
		Base.bRequiresRestart                = Base.bRequiresRestart                || Ovr.bRequiresRestart;

		// EN: Persistence backends — override replaces base
		// ES: Backends de persistencia — override reemplaza base
		Base.Persistence = Ovr.Persistence;

		if (bVerbose) UE_LOG(LogPGX, Log, TEXT("[ProfileSubsystem] Build %d: Policies override applied"), static_cast<int32>(OutProfile.Identity.BuildContext));
	}
}

void UPGXProfileSubsystem::ApplyMultiPlatformResolution(FPGXResolvedProfile& OutProfile) const
{
	// EN: Multi-platform resolution is already handled incrementally via
	//     MergeCapabilitiesAND / MergeBudgetsMIN / MergeFeaturesMostRestrictive
	//     in ApplyPlatformOverrides(). Each platform override merges against
	//     the accumulating profile, producing the intersection naturally.
	//
	// ES: La resolucion multi-plataforma ya se maneja incrementalmente via
	//     MergeCapabilitiesAND / MergeBudgetsMIN / MergeFeaturesMostRestrictive
	//     en ApplyPlatformOverrides(). Cada override de plataforma se fusiona
	//     contra el profile acumulado, produciendo la interseccion naturalmente.
}

void UPGXProfileSubsystem::LockProfile()
{
	ResolvedProfile.State = EPGXProfileState::Locked;
	UE_LOG(LogPGX, Log, TEXT("[ProfileSubsystem] Profile LOCKED (Shipping build). No further changes allowed."));
}

// ============================================================================
// Resolution Merge Helpers
// ============================================================================

void UPGXProfileSubsystem::MergeCapabilitiesAND(FPGXProfileCapabilities& Base, const FPGXProfileCapabilities& Override)
{
	// EN: AND merge — if either side says false, result is false
	// ES: Merge AND — si cualquier lado dice false, resultado es false
	Base.bAllowINIPersistence    = Base.bAllowINIPersistence    && Override.bAllowINIPersistence;
	Base.bAllowSaveData          = Base.bAllowSaveData          && Override.bAllowSaveData;
	Base.bAllowExternalWrites    = Base.bAllowExternalWrites    && Override.bAllowExternalWrites;
	Base.bAllowExports           = Base.bAllowExports           && Override.bAllowExports;
	Base.bAllowConsoleCommands   = Base.bAllowConsoleCommands   && Override.bAllowConsoleCommands;
	Base.bAllowDangerousCommands = Base.bAllowDangerousCommands && Override.bAllowDangerousCommands;
	Base.bAllowProfiling         = Base.bAllowProfiling         && Override.bAllowProfiling;
	Base.bAllowTelemetry         = Base.bAllowTelemetry         && Override.bAllowTelemetry;
	Base.bAllowPersistentLogs    = Base.bAllowPersistentLogs    && Override.bAllowPersistentLogs;
	Base.bAllowHotReload         = Base.bAllowHotReload         && Override.bAllowHotReload;
}

void UPGXProfileSubsystem::MergeBudgetsMIN(FPGXProfileBudgets& Base, const FPGXProfileBudgets& Override)
{
	// EN: MIN merge — take the smaller non-zero value (0 = unlimited is special: non-zero wins)
	// ES: Merge MIN — tomar el valor menor no-cero (0 = ilimitado es especial: no-cero gana)
	auto MinBudget = [](int64 A, int64 B) -> int64
	{
		if (A == 0) return B;
		if (B == 0) return A;
		return FMath::Min(A, B);
	};

	auto MinBudget32 = [](int32 A, int32 B) -> int32
	{
		if (A == 0) return B;
		if (B == 0) return A;
		return FMath::Min(A, B);
	};

	Base.RAM_MB           = MinBudget(Base.RAM_MB,           Override.RAM_MB);
	Base.VRAM_MB          = MinBudget(Base.VRAM_MB,          Override.VRAM_MB);
	Base.Disk_MB          = MinBudget(Base.Disk_MB,          Override.Disk_MB);
	Base.StreamingPool_MB = MinBudget(Base.StreamingPool_MB, Override.StreamingPool_MB);
	Base.TextureMaxDim    = MinBudget32(Base.TextureMaxDim,  Override.TextureMaxDim);
	Base.MeshMaxTris      = MinBudget32(Base.MeshMaxTris,    Override.MeshMaxTris);
	Base.ShaderBudget     = MinBudget32(Base.ShaderBudget,   Override.ShaderBudget);
	Base.PSOBudget        = MinBudget32(Base.PSOBudget,      Override.PSOBudget);
	Base.AudioMemory_MB   = MinBudget32(Base.AudioMemory_MB, Override.AudioMemory_MB);
}

void UPGXProfileSubsystem::MergeFeaturesMostRestrictive(FPGXProfileFeatureMatrix& Base, const FPGXProfileFeatureMatrix& Override)
{
	// EN: Most restrictive merge — Disallowed > FallbackRequired > Allowed
	// ES: Merge mas restrictivo — Disallowed > FallbackRequired > Allowed
	Base.Nanite.Policy         = MostRestrictivePolicy(Base.Nanite.Policy,         Override.Nanite.Policy);
	Base.Lumen.Policy          = MostRestrictivePolicy(Base.Lumen.Policy,          Override.Lumen.Policy);
	Base.RayTracing.Policy     = MostRestrictivePolicy(Base.RayTracing.Policy,     Override.RayTracing.Policy);
	Base.VSM.Policy            = MostRestrictivePolicy(Base.VSM.Policy,            Override.VSM.Policy);
	Base.ForwardShading.Policy = MostRestrictivePolicy(Base.ForwardShading.Policy, Override.ForwardShading.Policy);
	Base.MobileRenderer.Policy = MostRestrictivePolicy(Base.MobileRenderer.Policy, Override.MobileRenderer.Policy);
	Base.VirtualTextures.Policy= MostRestrictivePolicy(Base.VirtualTextures.Policy,Override.VirtualTextures.Policy);

	// EN: bRequiresRestart — OR (if either requires, result requires)
	// ES: bRequiresRestart — OR (si cualquiera requiere, el resultado requiere)
	Base.Nanite.bRequiresRestart         = Base.Nanite.bRequiresRestart         || Override.Nanite.bRequiresRestart;
	Base.Lumen.bRequiresRestart          = Base.Lumen.bRequiresRestart          || Override.Lumen.bRequiresRestart;
	Base.RayTracing.bRequiresRestart     = Base.RayTracing.bRequiresRestart     || Override.RayTracing.bRequiresRestart;
	Base.VSM.bRequiresRestart            = Base.VSM.bRequiresRestart            || Override.VSM.bRequiresRestart;
	Base.ForwardShading.bRequiresRestart = Base.ForwardShading.bRequiresRestart || Override.ForwardShading.bRequiresRestart;
	Base.MobileRenderer.bRequiresRestart = Base.MobileRenderer.bRequiresRestart || Override.MobileRenderer.bRequiresRestart;
	Base.VirtualTextures.bRequiresRestart= Base.VirtualTextures.bRequiresRestart|| Override.VirtualTextures.bRequiresRestart;
}

EPGXFeaturePolicy UPGXProfileSubsystem::MostRestrictivePolicy(EPGXFeaturePolicy A, EPGXFeaturePolicy B)
{
	// EN: Disallowed(1) > FallbackRequired(2) > Allowed(0)
	// ES: Disallowed(1) > FallbackRequired(2) > Allowed(0)
	if (A == EPGXFeaturePolicy::Disallowed || B == EPGXFeaturePolicy::Disallowed)
		return EPGXFeaturePolicy::Disallowed;
	if (A == EPGXFeaturePolicy::FallbackRequired || B == EPGXFeaturePolicy::FallbackRequired)
		return EPGXFeaturePolicy::FallbackRequired;
	return EPGXFeaturePolicy::Allowed;
}

// ============================================================================
// Console Commands
// ============================================================================

void UPGXProfileSubsystem::RegisterConsoleCommands()
{
	RegisterQueryConsoleCommands();
#if WITH_EDITOR
	RegisterSimulationConsoleCommands();
#endif
}

void UPGXProfileSubsystem::RegisterQueryConsoleCommands()
{
	RegisteredCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.profile.status"),
		TEXT("Print current profile status summary"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			UE_LOG(LogPGX, Log, TEXT(""));
			UE_LOG(LogPGX, Log, TEXT("========== PGX Profile Status =========="));
			UE_LOG(LogPGX, Log, TEXT("  State: %d"), static_cast<int32>(ResolvedProfile.State));
			UE_LOG(LogPGX, Log, TEXT("  Config: %s"), ActiveConfig ? *ActiveConfig->GetName() : TEXT("(none — using defaults)"));
			UE_LOG(LogPGX, Log, TEXT("  Mode: %d"), static_cast<int32>(ResolvedProfile.Identity.ProjectMode));
			UE_LOG(LogPGX, Log, TEXT("  Targets: %d"), ResolvedProfile.Identity.ActiveTargets.Num());
			UE_LOG(LogPGX, Log, TEXT("  Build: %d"), static_cast<int32>(ResolvedProfile.Identity.BuildContext));
			UE_LOG(LogPGX, Log, TEXT("  Restriction: %d"), static_cast<int32>(ResolvedProfile.Identity.RestrictionLevel));
			UE_LOG(LogPGX, Log, TEXT("========================================"));
			UE_LOG(LogPGX, Log, TEXT(""));
		}),
		ECVF_Default
	));

	RegisteredCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.profile.dump"),
		TEXT("Dump full resolved profile details"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			UE_LOG(LogPGX, Log, TEXT(""));
			UE_LOG(LogPGX, Log, TEXT("========== PGX Profile Dump =========="));

			// Identity
			UE_LOG(LogPGX, Log, TEXT("--- Identity ---"));
			UE_LOG(LogPGX, Log, TEXT("  Mode: %d | Build: %d | Restriction: %d"),
				static_cast<int32>(ResolvedProfile.Identity.ProjectMode),
				static_cast<int32>(ResolvedProfile.Identity.BuildContext),
				static_cast<int32>(ResolvedProfile.Identity.RestrictionLevel));
			for (const auto& T : ResolvedProfile.Identity.ActiveTargets)
			{
				UE_LOG(LogPGX, Log, TEXT("  Target: %d"), static_cast<int32>(T));
			}

			// Capabilities
			UE_LOG(LogPGX, Log, TEXT("--- Capabilities ---"));
			UE_LOG(LogPGX, Log, TEXT("  INI=%d SaveData=%d ExtWrites=%d Exports=%d"),
				ResolvedProfile.Capabilities.bAllowINIPersistence,
				ResolvedProfile.Capabilities.bAllowSaveData,
				ResolvedProfile.Capabilities.bAllowExternalWrites,
				ResolvedProfile.Capabilities.bAllowExports);
			UE_LOG(LogPGX, Log, TEXT("  Console=%d Dangerous=%d Profiling=%d"),
				ResolvedProfile.Capabilities.bAllowConsoleCommands,
				ResolvedProfile.Capabilities.bAllowDangerousCommands,
				ResolvedProfile.Capabilities.bAllowProfiling);
			UE_LOG(LogPGX, Log, TEXT("  Telemetry=%d PersistentLogs=%d HotReload=%d"),
				ResolvedProfile.Capabilities.bAllowTelemetry,
				ResolvedProfile.Capabilities.bAllowPersistentLogs,
				ResolvedProfile.Capabilities.bAllowHotReload);

			// Budgets
			UE_LOG(LogPGX, Log, TEXT("--- Budgets ---"));
			UE_LOG(LogPGX, Log, TEXT("  RAM=%lld VRAM=%lld Disk=%lld StreamPool=%lld"),
				ResolvedProfile.Budgets.RAM_MB,
				ResolvedProfile.Budgets.VRAM_MB,
				ResolvedProfile.Budgets.Disk_MB,
				ResolvedProfile.Budgets.StreamingPool_MB);
			UE_LOG(LogPGX, Log, TEXT("  TexMax=%d MeshTris=%d Shaders=%d PSO=%d Audio=%d"),
				ResolvedProfile.Budgets.TextureMaxDim,
				ResolvedProfile.Budgets.MeshMaxTris,
				ResolvedProfile.Budgets.ShaderBudget,
				ResolvedProfile.Budgets.PSOBudget,
				ResolvedProfile.Budgets.AudioMemory_MB);

			// Features
			UE_LOG(LogPGX, Log, TEXT("--- Features ---"));
			UE_LOG(LogPGX, Log, TEXT("  Nanite=%d Lumen=%d RT=%d VSM=%d FwdShading=%d Mobile=%d VTex=%d"),
				static_cast<int32>(ResolvedProfile.Features.Nanite.Policy),
				static_cast<int32>(ResolvedProfile.Features.Lumen.Policy),
				static_cast<int32>(ResolvedProfile.Features.RayTracing.Policy),
				static_cast<int32>(ResolvedProfile.Features.VSM.Policy),
				static_cast<int32>(ResolvedProfile.Features.ForwardShading.Policy),
				static_cast<int32>(ResolvedProfile.Features.MobileRenderer.Policy),
				static_cast<int32>(ResolvedProfile.Features.VirtualTextures.Policy));

			// Policies
			UE_LOG(LogPGX, Log, TEXT("--- Policies ---"));
			UE_LOG(LogPGX, Log, TEXT("  Config→%d Log→%d Save→%d"),
				static_cast<int32>(ResolvedProfile.Policies.Persistence.ConfigBackend),
				static_cast<int32>(ResolvedProfile.Policies.Persistence.LogBackend),
				static_cast<int32>(ResolvedProfile.Policies.Persistence.SaveBackend));
			UE_LOG(LogPGX, Log, TEXT("  Encrypt=%d Compress=%d StripDebug=%d"),
				ResolvedProfile.Policies.Security.bRequireEncryption,
				ResolvedProfile.Policies.Security.bRequireCompression,
				ResolvedProfile.Policies.Security.bStripDebugStrings);

			UE_LOG(LogPGX, Log, TEXT("========================================"));
			UE_LOG(LogPGX, Log, TEXT(""));
		}),
		ECVF_Default
	));

	RegisteredCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.profile.capability"),
		TEXT("Query a capability: pgx.profile.capability <name>"),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& Args)
		{
			if (Args.Num() < 1)
			{
				UE_LOG(LogPGX, Warning, TEXT("Usage: pgx.profile.capability <CapabilityName>"));
				return;
			}
			const FName Name(*Args[0]);
			const bool bAllowed = ResolvedProfile.IsCapabilityAllowed(Name);
			UE_LOG(LogPGX, Log, TEXT("[Profile] Capability '%s' = %s"), *Args[0], bAllowed ? TEXT("ALLOWED") : TEXT("DENIED"));
		}),
		ECVF_Default
	));

	RegisteredCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.profile.budget"),
		TEXT("Query a budget: pgx.profile.budget <name>"),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& Args)
		{
			if (Args.Num() < 1)
			{
				UE_LOG(LogPGX, Warning, TEXT("Usage: pgx.profile.budget <BudgetName>"));
				return;
			}
			const FName Name(*Args[0]);
			const int64 Value = ResolvedProfile.GetBudget(Name);
			UE_LOG(LogPGX, Log, TEXT("[Profile] Budget '%s' = %lld %s"), *Args[0], Value, Value == 0 ? TEXT("(unlimited)") : TEXT(""));
		}),
		ECVF_Default
	));

	RegisteredCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.profile.feature"),
		TEXT("Query a feature: pgx.profile.feature <name>"),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& Args)
		{
			if (Args.Num() < 1)
			{
				UE_LOG(LogPGX, Warning, TEXT("Usage: pgx.profile.feature <FeatureName>"));
				return;
			}
			const FName Name(*Args[0]);
			const EPGXFeaturePolicy Policy = ResolvedProfile.GetFeaturePolicy(Name);
			const TCHAR* PolicyStr = TEXT("UNKNOWN");
			switch (Policy)
			{
			case EPGXFeaturePolicy::Allowed:          PolicyStr = TEXT("ALLOWED"); break;
			case EPGXFeaturePolicy::Disallowed:       PolicyStr = TEXT("DISALLOWED"); break;
			case EPGXFeaturePolicy::FallbackRequired: PolicyStr = TEXT("FALLBACK_REQUIRED"); break;
			}
			UE_LOG(LogPGX, Log, TEXT("[Profile] Feature '%s' = %s"), *Args[0], PolicyStr);
		}),
		ECVF_Default
	));

}

// ============================================================================
// Simulation Console Commands (Editor only)
// ============================================================================

#if WITH_EDITOR
void UPGXProfileSubsystem::RegisterSimulationConsoleCommands()
{
	RegisteredCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.profile.simulate.platform"),
		TEXT("Simulate a target platform: pgx.profile.simulate.platform <PC|Console_PS|Console_Xbox|Console_Switch|Mobile_iOS|Mobile_Android|XR>"),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& Args)
		{
			if (Args.Num() < 1)
			{
				UE_LOG(LogPGX, Warning, TEXT("Usage: pgx.profile.simulate.platform <Platform>"));
				UE_LOG(LogPGX, Warning, TEXT("  Platforms: PC, Console_PS, Console_Xbox, Console_Switch, Mobile_iOS, Mobile_Android, XR"));
				return;
			}

			const FString& PlatStr = Args[0];
			EPGXTargetPlatform Platform = EPGXTargetPlatform::PC;

			if      (PlatStr == TEXT("PC"))              Platform = EPGXTargetPlatform::PC;
			else if (PlatStr == TEXT("Console_PS"))      Platform = EPGXTargetPlatform::Console_PS;
			else if (PlatStr == TEXT("Console_Xbox"))    Platform = EPGXTargetPlatform::Console_Xbox;
			else if (PlatStr == TEXT("Console_Switch"))  Platform = EPGXTargetPlatform::Console_Switch;
			else if (PlatStr == TEXT("Mobile_iOS"))      Platform = EPGXTargetPlatform::Mobile_iOS;
			else if (PlatStr == TEXT("Mobile_Android"))  Platform = EPGXTargetPlatform::Mobile_Android;
			else if (PlatStr == TEXT("XR"))              Platform = EPGXTargetPlatform::XR;
			else
			{
				UE_LOG(LogPGX, Warning, TEXT("Unknown platform: %s"), *PlatStr);
				return;
			}

			SimulatePlatform(Platform);
		}),
		ECVF_Default
	));

	RegisteredCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.profile.simulate.build"),
		TEXT("Simulate a build context: pgx.profile.simulate.build <Development|Test|Shipping>"),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& Args)
		{
			if (Args.Num() < 1)
			{
				UE_LOG(LogPGX, Warning, TEXT("Usage: pgx.profile.simulate.build <BuildContext>"));
				UE_LOG(LogPGX, Warning, TEXT("  Contexts: Development, Test, Shipping"));
				return;
			}

			const FString& CtxStr = Args[0];
			EPGXBuildContext Context = EPGXBuildContext::Development;

			if      (CtxStr == TEXT("Development")) Context = EPGXBuildContext::Development;
			else if (CtxStr == TEXT("Test"))         Context = EPGXBuildContext::Test;
			else if (CtxStr == TEXT("Shipping"))     Context = EPGXBuildContext::Shipping;
			else
			{
				UE_LOG(LogPGX, Warning, TEXT("Unknown build context: %s"), *CtxStr);
				return;
			}

			SimulateBuildContext(Context);
		}),
		ECVF_Default
	));

	RegisteredCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.profile.simulate.clear"),
		TEXT("Clear all simulation overrides and restore original profile"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			ClearSimulationOverrides();
		}),
		ECVF_Default
	));

	RegisteredCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.profile.simulate.status"),
		TEXT("Show current simulation status"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			UE_LOG(LogPGX, Log, TEXT(""));
			UE_LOG(LogPGX, Log, TEXT("========== PGX Profile Simulation Status =========="));
			UE_LOG(LogPGX, Log, TEXT("  Simulation Active: %s"), bSimulationActive ? TEXT("YES") : TEXT("NO"));
			if (bSimulationActive)
			{
				UE_LOG(LogPGX, Log, TEXT("  Current Platform(s): %d target(s)"), ResolvedProfile.Identity.ActiveTargets.Num());
				for (const auto& T : ResolvedProfile.Identity.ActiveTargets)
				{
					UE_LOG(LogPGX, Log, TEXT("    - %d"), static_cast<int32>(T));
				}
				UE_LOG(LogPGX, Log, TEXT("  Current Build: %d"), static_cast<int32>(ResolvedProfile.Identity.BuildContext));
				UE_LOG(LogPGX, Log, TEXT("  Original Platform(s): %d target(s)"), PreSimulationProfile.Identity.ActiveTargets.Num());
				UE_LOG(LogPGX, Log, TEXT("  Original Build: %d"), static_cast<int32>(PreSimulationProfile.Identity.BuildContext));
			}
			UE_LOG(LogPGX, Log, TEXT("===================================================="));
			UE_LOG(LogPGX, Log, TEXT(""));
		}),
		ECVF_Default
	));
}
#endif // WITH_EDITOR

void UPGXProfileSubsystem::UnregisterConsoleCommands()
{
	for (IConsoleObject* Cmd : RegisteredCommands)
	{
		IConsoleManager::Get().UnregisterConsoleObject(Cmd);
	}
	RegisteredCommands.Empty();
}

// ============================================================================
// Editor Simulation
// ============================================================================

#if WITH_EDITOR

void UPGXProfileSubsystem::SimulatePlatform(EPGXTargetPlatform Platform)
{
	if (!IsValid(ActiveConfig))
	{
		UE_LOG(LogPGX, Warning, TEXT("[ProfileSubsystem] Cannot simulate: no active config. Initialize the profile first."));
		return;
	}

	if (ResolvedProfile.State == EPGXProfileState::Locked)
	{
		UE_LOG(LogPGX, Warning, TEXT("[ProfileSubsystem] Cannot simulate: profile is locked"));
		return;
	}

	if (!bSimulationActive)
	{
		PreSimulationProfile = ResolvedProfile;
		bSimulationActive = true;
	}

	// EN: Override active targets with the simulated platform
	// ES: Sobreescribir targets activos con la plataforma simulada
	ResolvedProfile.Identity.ActiveTargets.Empty();
	ResolvedProfile.Identity.ActiveTargets.Add(Platform);

	ReResolveWithSimulation();

	// EN: Swap active platform config to the simulated platform
	// ES: Cambiar config de plataforma activa a la plataforma simulada
	if (const TObjectPtr<UPGXPlatformConfig>* Found = LoadedPlatformConfigs.Find(Platform))
	{
		ActivePlatformConfig = *Found;
	}
	else
	{
		ActivePlatformConfig = nullptr;
	}

	UE_LOG(LogPGX, Log, TEXT("[ProfileSubsystem] Simulation: Platform set to %d"), static_cast<int32>(Platform));
}

void UPGXProfileSubsystem::SimulateBuildContext(EPGXBuildContext BuildContext)
{
	if (!IsValid(ActiveConfig))
	{
		UE_LOG(LogPGX, Warning, TEXT("[ProfileSubsystem] Cannot simulate: no active config. Initialize the profile first."));
		return;
	}

	if (ResolvedProfile.State == EPGXProfileState::Locked)
	{
		UE_LOG(LogPGX, Warning, TEXT("[ProfileSubsystem] Cannot simulate: profile is locked"));
		return;
	}

	if (!bSimulationActive)
	{
		PreSimulationProfile = ResolvedProfile;
		bSimulationActive = true;
	}

	ResolvedProfile.Identity.BuildContext = BuildContext;

	ReResolveWithSimulation();

	UE_LOG(LogPGX, Log, TEXT("[ProfileSubsystem] Simulation: Build context set to %d"), static_cast<int32>(BuildContext));
}

void UPGXProfileSubsystem::ClearSimulationOverrides()
{
	if (!bSimulationActive) return;

	const FPGXResolvedProfile OldProfile = ResolvedProfile;

	OnProfilePreChangeNative.Broadcast(ResolvedProfile);

	ResolvedProfile = PreSimulationProfile;
	bSimulationActive = false;

	OnProfileChangedNative.Broadcast(OldProfile, ResolvedProfile);
	OnProfileChanged.Broadcast(OldProfile, ResolvedProfile);

	UE_LOG(LogPGX, Log, TEXT("[ProfileSubsystem] Simulation cleared. Original profile restored."));
}

void UPGXProfileSubsystem::ReResolveWithSimulation()
{
	if (!ActiveConfig) return;

	const FPGXResolvedProfile OldProfile = ResolvedProfile;

	OnProfilePreChangeNative.Broadcast(OldProfile);

	// EN: Re-resolve from config with current (simulated) identity
	// ES: Re-resolver desde config con identidad actual (simulada)
	FPGXResolvedProfile NewProfile;
	NewProfile.Identity = ResolvedProfile.Identity;
	NewProfile.Capabilities = ActiveConfig->BaseCapabilities;
	NewProfile.Policies = ActiveConfig->BasePolicies;
	NewProfile.Budgets = ActiveConfig->BaseBudgets;
	NewProfile.Features = ActiveConfig->BaseFeatures;

	ApplyPlatformOverrides(ActiveConfig, NewProfile);
	if (NewProfile.Identity.ActiveTargets.Num() > 1)
	{
		ApplyMultiPlatformResolution(NewProfile);
	}
	ApplyBuildOverrides(ActiveConfig, NewProfile);

	NewProfile.State = EPGXProfileState::Resolved;
	NewProfile.ResolvedTimestamp = FDateTime::UtcNow();

	ResolvedProfile = NewProfile;

	OnProfileChangedNative.Broadcast(OldProfile, ResolvedProfile);
	OnProfileChanged.Broadcast(OldProfile, ResolvedProfile);
}

#endif // WITH_EDITOR
