// Copyright PGX Framework. All Rights Reserved.

#include "PGXEnvironmentSubsystem.h"
#include "Logging/PGXLogMacros.h"

#include "PGXEnvironmentRuntime.h"
#include "PGXEnvironmentSettings.h"
#include "PGXEnvironmentConfig.h"
#include "PGXEnvironmentZoneDefinition.h"
#include "PGXEnvironmentVariable.h"
#include "Utils/PGXConfigResolution.h"

void UPGXEnvironmentSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// EN: Settings-first read per the current product scope.
	//     Snapshot the verbose toggle so query/diagnostic paths can read it
	//     cheaply without bouncing through GetDefault each call.
	// ES: Read Settings-first per the current product scope. Snapshot
	//     del toggle verboso para que los paths de query/diagnostica lo
	//     lean barato sin rebotar por GetDefault en cada call.
	if (const UPGXEnvironmentSettings* Settings = GetDefault<UPGXEnvironmentSettings>())
	{
		bVerboseEnvironmentDebug = Settings->bVerboseEnvironmentDebug;
	}

	ResolveActiveConfig();

	PGX_LOG_INFO(LogPGXEnvironment, TEXT("[EnvironmentSubsystem] Initialized — verbose=%s, ActiveConfig=%s"),
		bVerboseEnvironmentDebug ? TEXT("true") : TEXT("false"),
		ActiveConfig ? *ActiveConfig->GetName() : TEXT("(none)"));
}

void UPGXEnvironmentSubsystem::Deinitialize()
{
	PGX_LOG_INFO(LogPGXEnvironment, TEXT("[EnvironmentSubsystem] Deinitialized"));

	ActiveConfig = nullptr;

	Super::Deinitialize();
}

void UPGXEnvironmentSubsystem::ResolveActiveConfig()
{
	// EN: PGX::ResolveSingleConfig<T>() centralizes Settings-first resolution,
	//     synchronous loading, the deprecated AssetRegistry fallback and logging.
	// ES: PGX::ResolveSingleConfig<T>() centraliza la resolucion Settings-first,
	//     la carga sincrona, el fallback AssetRegistry deprecado y el logging.
	ActiveConfig = nullptr;

	const UPGXEnvironmentSettings* Settings = GetDefault<UPGXEnvironmentSettings>();
	if (!Settings)
	{
		PGX_LOG_WARNING(LogPGXEnvironment, TEXT("[EnvironmentSubsystem] GetDefault<UPGXEnvironmentSettings> returned null"));
		return;
	}

	ActiveConfig = PGX::ResolveSingleConfig<UPGXEnvironmentConfig>(
		Settings->ActiveConfig, TEXT("Environment"));
}

// ============================================================================
// EN: Zone registry — zone registry implementation
// ES: Zone registry — implementacion zone registry
// ============================================================================

FPGXEnvironmentResult UPGXEnvironmentSubsystem::RegisterZone(FGameplayTag ZoneTag)
{
	if (!ZoneTag.IsValid())
	{
		const FString Reason = TEXT(
			"UPGXEnvironmentSubsystem::RegisterZone called with invalid (empty) ZoneTag.");
		PGX_LOG_WARNING(LogPGXEnvironment, TEXT("[EnvironmentSubsystem] %s"), *Reason);
		return FPGXEnvironmentResult::MakeFail(EPGXEnvironmentResultCode::InvalidConfig, Reason);
	}

	if (ZoneRegistry.Contains(ZoneTag))
	{
		const FString Reason = FString::Printf(
			TEXT("Zone '%s' is already registered. Unregister it first or pick a unique tag."),
			*ZoneTag.ToString());
		PGX_LOG_WARNING(LogPGXEnvironment, TEXT("[EnvironmentSubsystem] %s"), *Reason);
		return FPGXEnvironmentResult::MakeFail(EPGXEnvironmentResultCode::AlreadyRegistered, Reason);
	}

	// EN: Build a fresh state entry. threshold evaluation wires authored seed read +
	//     threshold evaluation. If ActiveConfig has a matching
	//     ZoneDefinition, ApplyAuthoredSeeds populates Values and
	//     EvaluateZoneThresholds fires OnZoneSeverityChangedNative for
	//     each band that already resolves to a non-None severity at
	//     register time (transition None → real).
	// ES: Construir entry de estado fresca. threshold evaluation wire el read authoring
	//     del seed + evaluacion de threshold. Si ActiveConfig tiene un
	//     ZoneDefinition matching, ApplyAuthoredSeeds popula Values y
	//     EvaluateZoneThresholds dispara OnZoneSeverityChangedNative para
	//     cada banda que ya resuelve a severity non-None al register
	//     time (transicion None → real).
	FPGXEnvironmentZoneState State;
	State.ZoneTag = ZoneTag;

	UPGXEnvironmentZoneDefinition* Definition = ResolveZoneDefinition(ZoneTag);
	if (Definition)
	{
		ApplyAuthoredSeeds(State, *Definition);
		EvaluateZoneThresholds(State, *Definition);
	}

	ZoneRegistry.Add(ZoneTag, MoveTemp(State));

	if (bVerboseEnvironmentDebug)
	{
		PGX_LOG_INFO(LogPGXEnvironment,
			TEXT("[EnvironmentSubsystem] Zone registered: %s (registry size %d, authored=%s)"),
			*ZoneTag.ToString(), ZoneRegistry.Num(),
			Definition ? TEXT("yes") : TEXT("no"));
	}

	return FPGXEnvironmentResult::MakeSuccess(
		FString::Printf(TEXT("Zone '%s' registered."), *ZoneTag.ToString()));
}

FPGXEnvironmentResult UPGXEnvironmentSubsystem::UnregisterZone(FGameplayTag ZoneTag)
{
	if (!ZoneTag.IsValid())
	{
		const FString Reason = TEXT(
			"UPGXEnvironmentSubsystem::UnregisterZone called with invalid (empty) ZoneTag.");
		PGX_LOG_WARNING(LogPGXEnvironment, TEXT("[EnvironmentSubsystem] %s"), *Reason);
		return FPGXEnvironmentResult::MakeFail(EPGXEnvironmentResultCode::InvalidConfig, Reason);
	}

	const int32 Removed = ZoneRegistry.Remove(ZoneTag);
	if (Removed == 0)
	{
		const FString Reason = FString::Printf(
			TEXT("Zone '%s' is not in the registry."), *ZoneTag.ToString());
		// EN: NotFound is a typed result, not a Warning — callers may legitimately
		//     probe by tag without being sure of registration. Log at Verbose only.
		// ES: NotFound es un resultado tipado, no un Warning — los callers pueden
		//     legitimamente probar por tag sin estar seguros del registro. Loguear
		//     solo en Verbose.
		PGX_LOG_VERBOSE(LogPGXEnvironment, TEXT("[EnvironmentSubsystem] %s"), *Reason);
		return FPGXEnvironmentResult::MakeFail(EPGXEnvironmentResultCode::NotFound, Reason);
	}

	if (bVerboseEnvironmentDebug)
	{
		PGX_LOG_INFO(LogPGXEnvironment, TEXT("[EnvironmentSubsystem] Zone unregistered: %s (registry size %d)"),
			*ZoneTag.ToString(), ZoneRegistry.Num());
	}

	return FPGXEnvironmentResult::MakeSuccess(
		FString::Printf(TEXT("Zone '%s' unregistered."), *ZoneTag.ToString()));
}

bool UPGXEnvironmentSubsystem::IsZoneRegistered(FGameplayTag ZoneTag) const
{
	return ZoneTag.IsValid() && ZoneRegistry.Contains(ZoneTag);
}

TArray<FGameplayTag> UPGXEnvironmentSubsystem::GetRegisteredZoneTags() const
{
	TArray<FGameplayTag> Tags;
	ZoneRegistry.GetKeys(Tags);
	return Tags;
}

bool UPGXEnvironmentSubsystem::HasEntryByTag(FGameplayTag Tag) const
{
	return IsZoneRegistered(Tag);
}

int32 UPGXEnvironmentSubsystem::GetCount() const
{
	return ZoneRegistry.Num();
}

void UPGXEnvironmentSubsystem::GetSnapshot(TArray<FGameplayTag>& OutTags) const
{
	ZoneRegistry.GetKeys(OutTags);
}

// ============================================================================
// EN: threshold evaluation helpers — authored seed read + threshold evaluation
// ES: Helpers threshold evaluation — read authoring de seed + evaluacion de threshold
// ============================================================================

UPGXEnvironmentZoneDefinition* UPGXEnvironmentSubsystem::ResolveZoneDefinition(FGameplayTag ZoneTag) const
{
	if (!ActiveConfig || !ZoneTag.IsValid())
	{
		return nullptr;
	}

	for (const TSoftObjectPtr<UPGXEnvironmentZoneDefinition>& Soft : ActiveConfig->ZoneDefinitions)
	{
		if (Soft.IsNull())
		{
			continue;
		}

		// EN: LoadSynchronous acceptable here — RegisterZone is a low-frequency
		//     world-subsystem entry point, the DA is small. Cached on
		//     subsequent calls by the engine's loaded-asset table.
		// ES: LoadSynchronous aceptable aqui — RegisterZone es un entry point
		//     de world-subsystem de baja frecuencia, el DA es pequeño. Cached
		//     en calls subsecuentes por la tabla de loaded-asset del engine.
		UPGXEnvironmentZoneDefinition* Candidate =
			const_cast<TSoftObjectPtr<UPGXEnvironmentZoneDefinition>&>(Soft).LoadSynchronous();
		if (Candidate && Candidate->ZoneTag.IsValid() && Candidate->ZoneTag == ZoneTag)
		{
			return Candidate;
		}
	}

	return nullptr;
}

void UPGXEnvironmentSubsystem::ApplyAuthoredSeeds(
	FPGXEnvironmentZoneState& State,
	const UPGXEnvironmentZoneDefinition& Definition)
{
	for (const FPGXEnvironmentVariableSeed& Seed : Definition.VariableSeeds)
	{
		if (Seed.Variable.IsNull())
		{
			continue;
		}

		UPGXEnvironmentVariable* Variable =
			const_cast<TSoftObjectPtr<UPGXEnvironmentVariable>&>(Seed.Variable).LoadSynchronous();
		if (!Variable || !Variable->VariableTag.IsValid())
		{
			continue;
		}

		const float SeedValue = Seed.bUseInitialValueOverride
			? Seed.InitialValueOverride
			: Variable->InitialValue;

		// EN: Clamp seed against the variable's authored bounds (Authoring
		//     Guide authoring invariant 1 — bounded by data, never by hardcoded
		//     literals here).
		// ES: Clamp del seed contra los bounds authoring de la variable
		//     (Authoring authoring invariante 1 — bounded por data, nunca por
		//     literals hardcoded aqui).
		const float Clamped = FMath::Clamp(SeedValue, Variable->ClampMin, Variable->ClampMax);
		State.Values.Add(Variable->VariableTag, Clamped);

		// EN: Initialise LastSeverityByVariable to None so the first
		//     EvaluateZoneThresholds call detects any non-None band as a
		//     transition (None → real).
		// ES: Inicializar LastSeverityByVariable a None para que la primera
		//     llamada de EvaluateZoneThresholds detecte cualquier banda
		//     non-None como transicion (None → real).
		State.LastSeverityByVariable.Add(Variable->VariableTag, EPGXEnvironmentSeverity::None);
	}
}

void UPGXEnvironmentSubsystem::EvaluateZoneThresholds(
	FPGXEnvironmentZoneState& State,
	const UPGXEnvironmentZoneDefinition& /*Definition*/)
{
	// EN: Walk the loaded variables stored in State.Values. For each one,
	//     resolve the Variable DA from the ActiveConfig variable taxonomy
	//     to read its ThresholdBands, compute the new severity, and emit
	//     a transition delegate when it differs from the cached value.
	// ES: Recorrer las variables loaded en State.Values. Por cada una,
	//     resolver el Variable DA desde la taxonomia de variables del
	//     ActiveConfig para leer sus ThresholdBands, computar la nueva
	//     severity, y emitir el delegate de transicion cuando difiera del
	//     valor cacheado.
	if (!ActiveConfig)
	{
		return;
	}

	// EN: Build a quick map VariableTag → Variable DA from the ActiveConfig
	//     taxonomy. Cheap because ActiveConfig is already loaded and
	//     Variables is a small TArray of soft-pointers; LoadSynchronous on
	//     each is acceptable at register time.
	// ES: Construir un map rapido VariableTag → Variable DA desde la
	//     taxonomia del ActiveConfig. Barato porque ActiveConfig ya esta
	//     loaded y Variables es un TArray pequeño de soft-pointers;
	//     LoadSynchronous en cada uno es aceptable al register time.
	TMap<FGameplayTag, UPGXEnvironmentVariable*> VarByTag;
	for (const TSoftObjectPtr<UPGXEnvironmentVariable>& Soft : ActiveConfig->Variables)
	{
		if (Soft.IsNull())
		{
			continue;
		}
		UPGXEnvironmentVariable* Var =
			const_cast<TSoftObjectPtr<UPGXEnvironmentVariable>&>(Soft).LoadSynchronous();
		if (Var && Var->VariableTag.IsValid())
		{
			VarByTag.Add(Var->VariableTag, Var);
		}
	}

	for (TPair<FGameplayTag, float>& Entry : State.Values)
	{
		UPGXEnvironmentVariable* const* VarPtr = VarByTag.Find(Entry.Key);
		if (!VarPtr || !*VarPtr)
		{
			continue;
		}
		const UPGXEnvironmentVariable* Var = *VarPtr;
		if (Var->ThresholdBands.Num() == 0)
		{
			continue;
		}

		const EPGXEnvironmentSeverity NewSeverity =
			ResolveSeverityForValue(Entry.Value, Var->ThresholdBands);
		EPGXEnvironmentSeverity& CachedSeverity =
			State.LastSeverityByVariable.FindOrAdd(Entry.Key, EPGXEnvironmentSeverity::None);

		if (NewSeverity != CachedSeverity)
		{
			const EPGXEnvironmentSeverity Prev = CachedSeverity;
			CachedSeverity = NewSeverity;

			if (bVerboseEnvironmentDebug)
			{
				PGX_LOG_INFO(LogPGXEnvironment,
					TEXT("[EnvironmentSubsystem] Severity transition: zone=%s var=%s %d → %d"),
					*State.ZoneTag.ToString(), *Entry.Key.ToString(),
					static_cast<int32>(Prev), static_cast<int32>(NewSeverity));
			}

			OnZoneSeverityChangedNative.Broadcast(State.ZoneTag, Entry.Key, Prev, NewSeverity);
		}
	}
}

EPGXEnvironmentSeverity UPGXEnvironmentSubsystem::ResolveSeverityForValue(
	float Value,
	const TArray<FPGXEnvironmentThresholdBand>& Bands)
{
	EPGXEnvironmentSeverity Best = EPGXEnvironmentSeverity::None;
	for (const FPGXEnvironmentThresholdBand& Band : Bands)
	{
		// EN: [LowerInclusive, UpperExclusive) interval per the current product scope
		//     authoring convention. Multiple matching bands → pick the
		//     highest severity (uint8 ordering).
		// ES: Intervalo [LowerInclusive, UpperExclusive) per convencion
		//     authoring the public API the public API. Multiples bands matching → escoger
		//     la severity mas alta (orden uint8).
		if (Value >= Band.LowerBoundInclusive && Value < Band.UpperBoundExclusive)
		{
			if (static_cast<uint8>(Band.Severity) > static_cast<uint8>(Best))
			{
				Best = Band.Severity;
			}
		}
	}
	return Best;
}

// ============================================================================
// EN: Modifier API — modifier API
// ES: API de modificador — modifier API
// ============================================================================

FPGXEnvironmentResult UPGXEnvironmentSubsystem::ApplyVariableModifier(
	FGameplayTag ZoneTag,
	FGameplayTag VariableTag,
	float Delta,
	const UObject* Source)
{
	if (!ZoneTag.IsValid() || !VariableTag.IsValid())
	{
		const FString Reason = FString::Printf(
			TEXT("ApplyVariableModifier called with invalid tags: zone=%s var=%s"),
			ZoneTag.IsValid() ? *ZoneTag.ToString() : TEXT("(invalid)"),
			VariableTag.IsValid() ? *VariableTag.ToString() : TEXT("(invalid)"));
		PGX_LOG_WARNING(LogPGXEnvironment, TEXT("[EnvironmentSubsystem] %s"), *Reason);
		return FPGXEnvironmentResult::MakeFail(EPGXEnvironmentResultCode::InvalidConfig, Reason);
	}

	if (!FMath::IsFinite(Delta))
	{
		const FString Reason = FString::Printf(
			TEXT("ApplyVariableModifier rejected non-finite Delta on zone=%s var=%s"),
			*ZoneTag.ToString(), *VariableTag.ToString());
		PGX_LOG_WARNING(LogPGXEnvironment, TEXT("[EnvironmentSubsystem] %s"), *Reason);
		return FPGXEnvironmentResult::MakeFail(EPGXEnvironmentResultCode::OutOfBounds, Reason);
	}

	FPGXEnvironmentZoneState* StatePtr = ZoneRegistry.Find(ZoneTag);
	if (!StatePtr)
	{
		const FString Reason = FString::Printf(
			TEXT("ApplyVariableModifier called for unregistered zone '%s'."),
			*ZoneTag.ToString());
		PGX_LOG_VERBOSE(LogPGXEnvironment, TEXT("[EnvironmentSubsystem] %s"), *Reason);
		return FPGXEnvironmentResult::MakeFail(EPGXEnvironmentResultCode::NotFound, Reason);
	}

	// EN: Resolve the Variable DA from ActiveConfig taxonomy so we can
	//     read ClampMin / ClampMax + ThresholdBands. NotFound when the
	//     variable is not part of the project's authored taxonomy or
	//     ActiveConfig is absent (developer / authoring mistake).
	// ES: Resolver el Variable DA desde la taxonomia del ActiveConfig para
	//     poder leer ClampMin / ClampMax + ThresholdBands. NotFound cuando
	//     la variable no es parte de la taxonomia authoring del proyecto
	//     o ActiveConfig esta ausente (error developer / authoring).
	UPGXEnvironmentVariable* Variable = nullptr;
	if (ActiveConfig)
	{
		for (const TSoftObjectPtr<UPGXEnvironmentVariable>& Soft : ActiveConfig->Variables)
		{
			if (Soft.IsNull())
			{
				continue;
			}
			UPGXEnvironmentVariable* Candidate =
				const_cast<TSoftObjectPtr<UPGXEnvironmentVariable>&>(Soft).LoadSynchronous();
			if (Candidate && Candidate->VariableTag == VariableTag)
			{
				Variable = Candidate;
				break;
			}
		}
	}

	if (!Variable)
	{
		const FString Reason = FString::Printf(
			TEXT("ApplyVariableModifier could not resolve variable '%s' from ActiveConfig taxonomy."),
			*VariableTag.ToString());
		PGX_LOG_WARNING(LogPGXEnvironment, TEXT("[EnvironmentSubsystem] %s"), *Reason);
		return FPGXEnvironmentResult::MakeFail(EPGXEnvironmentResultCode::NotFound, Reason);
	}

	// EN: Use FindOrAdd with the variable's authored InitialValue so a
	//     modifier call before any seed seeds the entry deterministically.
	// ES: Usar FindOrAdd con el InitialValue authoring de la variable para
	//     que una llamada de modificador antes de cualquier seed seedee la
	//     entry deterministicamente.
	float& CurrentValue = StatePtr->Values.FindOrAdd(VariableTag, Variable->InitialValue);
	const float Before = CurrentValue;
	CurrentValue = FMath::Clamp(Before + Delta, Variable->ClampMin, Variable->ClampMax);

	// EN: Re-trigger threshold evaluation so any band crossing fires the
	//     transition delegate. Definition pointer for the helper is the
	//     same authored DA we used at register time; resolve it again here
	//     to keep the helper API uniform (single-zone bounded — no graph
	//     walk).
	// ES: Re-triggerear evaluacion de threshold para que cualquier cruce
	//     de banda dispare el delegate de transicion. El pointer Definition
	//     para el helper es el mismo DA authoring que usamos al register
	//     time; resolverlo de nuevo aqui mantiene el API helper uniforme
	//     (single-zone bounded — sin walk de grafo).
	if (UPGXEnvironmentZoneDefinition* Definition = ResolveZoneDefinition(ZoneTag))
	{
		EvaluateZoneThresholds(*StatePtr, *Definition);
	}

	if (bVerboseEnvironmentDebug)
	{
		PGX_LOG_INFO(LogPGXEnvironment,
			TEXT("[EnvironmentSubsystem] Modifier applied: zone=%s var=%s delta=%.4f %.4f → %.4f%s"),
			*ZoneTag.ToString(), *VariableTag.ToString(), Delta, Before, CurrentValue,
			Source ? *FString::Printf(TEXT(" source=%s"), *Source->GetName()) : TEXT(""));
	}

	return FPGXEnvironmentResult::MakeSuccess(
		FString::Printf(TEXT("zone=%s var=%s %.4f → %.4f"),
			*ZoneTag.ToString(), *VariableTag.ToString(), Before, CurrentValue));
}
