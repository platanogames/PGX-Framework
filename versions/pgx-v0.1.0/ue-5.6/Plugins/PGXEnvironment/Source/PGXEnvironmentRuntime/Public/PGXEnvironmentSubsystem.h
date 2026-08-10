// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXWorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "Interfaces/PGXTaggedRegistry.h"
#include "PGXEnvironmentTypes.h"
#include "PGXEnvironmentSubsystem.generated.h"

class UPGXEnvironmentConfig;
class UPGXEnvironmentZoneDefinition;
class UPGXEnvironmentVariable;

/**
 * Native multicast delegate fired when a zone variable crosses an authored
 * severity threshold. Parameters are zone tag, variable tag, previous
 * severity and new severity. PGXMessage bridging is not included.
 */
DECLARE_MULTICAST_DELEGATE_FourParams(
	FOnPGXEnvironmentSeverityChangedNative,
	FGameplayTag /*ZoneTag*/,
	FGameplayTag /*VariableTag*/,
	EPGXEnvironmentSeverity /*PrevSeverity*/,
	EPGXEnvironmentSeverity /*NewSeverity*/);

/** Runtime values and cached threshold severity for one registered zone. */
struct FPGXEnvironmentZoneState
{
	FGameplayTag ZoneTag;

	TMap<FGameplayTag, float> Values;

	TMap<FGameplayTag, EPGXEnvironmentSeverity> LastSeverityByVariable;
};

/**
 * World-scoped environment registry. It resolves the configured environment
 * definition, registers zones, applies authored variable seeds, evaluates
 * threshold bands and supports bounded variable modifiers.
 *
 * Sensor meshes, propagation graphs, time-of-day drivers, save integration,
 * PGXMessage bridging and a dedicated editor inspector are not included.
 */
UCLASS()
class PGXENVIRONMENTRUNTIME_API UPGXEnvironmentSubsystem : public UPGXWorldSubsystem, public IPGXTaggedRegistry
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End USubsystem Interface

	// ========================================================================
	// Zone registry
	// ========================================================================

	/** Register a zone and initialize its authored variable values. */
	FPGXEnvironmentResult RegisterZone(FGameplayTag ZoneTag);

	/** Unregister a zone and discard its runtime state. */
	FPGXEnvironmentResult UnregisterZone(FGameplayTag ZoneTag);

	/** Whether a zone tag is currently registered. */
	bool IsZoneRegistered(FGameplayTag ZoneTag) const;

	/** Return all registered zone tags. */
	TArray<FGameplayTag> GetRegisteredZoneTags() const;

	// ========================================================================
	// EN: IPGXTaggedRegistry adoption — ZoneRegistry facade
	// ES: Adopcion IPGXTaggedRegistry — fachada de ZoneRegistry
	// ========================================================================

	/** EN: True if ZoneRegistry contains the tag / ES: True si ZoneRegistry contiene el tag. */
	bool HasEntryByTag(FGameplayTag Tag) const override;

	/** EN: Number of registered zones / ES: Numero de zonas registradas. */
	int32 GetCount() const override;

	/** EN: Snapshot of registered zone tags / ES: Snapshot de tags de zonas registradas. */
	void GetSnapshot(TArray<FGameplayTag>& OutTags) const override;

	// ========================================================================
	// Variable modifiers
	// ========================================================================

	/**
	 * Apply a delta to one variable in one registered zone. Finite values are
	 * clamped to the authored range and threshold transitions are reevaluated.
	 * The optional Source pointer is used for diagnostics only. This operation
	 * does not propagate changes to linked zones.
	 */
	FPGXEnvironmentResult ApplyVariableModifier(
		FGameplayTag ZoneTag,
		FGameplayTag VariableTag,
		float Delta,
		const UObject* Source = nullptr);

	// ========================================================================
	// EN: Diagnostics
	// ES: Diagnostica
	// ========================================================================

	/** EN: Whether the subsystem resolved an active config. / ES: Si el subsistema resolvio un config activo. */
	bool HasActiveConfig() const { return ActiveConfig != nullptr; }

	/** EN: Verbose-log toggle resolved from Settings at Initialize. / ES: Toggle de log verboso resuelto desde Settings al Initialize. */
	bool IsVerbose() const { return bVerboseEnvironmentDebug; }

	// ========================================================================
	// Native threshold delegate
	// ========================================================================

	/** Fires when a variable enters a different authored threshold band. */
	FOnPGXEnvironmentSeverityChangedNative OnZoneSeverityChangedNative;

private:
	// EN: Helpers threshold evaluation. ResolveZoneDefinition looks up the authored DA
	//     for a given ZoneTag from ActiveConfig (synchronous load of the
	//     soft-pointer if needed). ApplyAuthoredSeeds populates Values from
	//     the ZoneDefinition's VariableSeeds. EvaluateZoneThresholds walks
	//     the zone's authored variables, computes per-variable severity
	//     from ThresholdBands, and fires OnZoneSeverityChangedNative when
	//     a transition occurs vs LastSeverityByVariable. All three return
	//     void; caller is responsible for sequencing.
	// ES: Helpers threshold evaluation. ResolveZoneDefinition busca el DA authoring para
	//     un ZoneTag dado desde ActiveConfig (load sincronico del soft-
	//     pointer si necesario). ApplyAuthoredSeeds popula Values desde los
	//     VariableSeeds del ZoneDefinition. EvaluateZoneThresholds recorre
	//     las variables authoring de la zona, computa severity per-variable
	//     desde ThresholdBands, y dispara OnZoneSeverityChangedNative cuando
	//     ocurre transicion vs LastSeverityByVariable. Los tres retornan
	//     void; el caller es responsable del sequencing.
	UPGXEnvironmentZoneDefinition* ResolveZoneDefinition(FGameplayTag ZoneTag) const;
	void ApplyAuthoredSeeds(FPGXEnvironmentZoneState& State, const UPGXEnvironmentZoneDefinition& Definition);
	void EvaluateZoneThresholds(FPGXEnvironmentZoneState& State, const UPGXEnvironmentZoneDefinition& Definition);

	// EN: Helper used by EvaluateZoneThresholds. Picks the highest-severity
	//     band whose [LowerInclusive, UpperExclusive) range contains the
	//     value. Returns Severity::None if no band matches.
	// ES: Helper usado por EvaluateZoneThresholds. Escoge la banda de
	//     severity mas alta cuyo rango [LowerInclusive, UpperExclusive)
	//     contiene el valor. Retorna Severity::None si ninguna banda hace
	//     match.
	static EPGXEnvironmentSeverity ResolveSeverityForValue(float Value, const TArray<FPGXEnvironmentThresholdBand>& Bands);


	// Resolve ActiveConfig once during Initialize. If the soft reference is
	// empty or cannot be loaded, ActiveConfig remains null and a warning is logged.
	void ResolveActiveConfig();

	// EN: Cached active config pointer; nullptr if Settings ActiveConfig
	//     was empty / failed to load.
	// ES: Pointer cacheado al config activo; nullptr si el Settings
	//     ActiveConfig estaba vacio / fallo al cargar.
	UPROPERTY()
	TObjectPtr<UPGXEnvironmentConfig> ActiveConfig;

	// EN: Verbose toggle snapshot from Settings at Initialize.
	// ES: Snapshot del toggle verboso desde Settings al Initialize.
	bool bVerboseEnvironmentDebug = false;

	// EN: Zone registry. Tag → state. Keys are unique per
	//     subsystem. RegisterZone rejects duplicates with AlreadyRegistered;
	//     UnregisterZone rejects unknown tags with NotFound.
	// ES: Zone registry. Tag → estado. Keys unicas por subsistema.
	//     RegisterZone rechaza duplicados con AlreadyRegistered;
	//     UnregisterZone rechaza tags desconocidos con NotFound.
	TMap<FGameplayTag, FPGXEnvironmentZoneState> ZoneRegistry;
};
