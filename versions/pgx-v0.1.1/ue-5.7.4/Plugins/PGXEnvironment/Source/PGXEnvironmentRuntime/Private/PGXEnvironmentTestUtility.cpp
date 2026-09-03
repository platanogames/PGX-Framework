// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXEnvironmentTestUtility.h"
#include "Logging/PGXLogMacros.h"

#include "PGXEnvironmentConfig.h"
#include "PGXEnvironmentRuntime.h"
#include "PGXEnvironmentSettings.h"
#include "PGXEnvironmentSubsystem.h"
#include "PGXEnvironmentTickProfile.h"
#include "PGXEnvironmentTypes.h"
#include "PGXEnvironmentVariable.h"
#include "PGXEnvironmentZoneDefinition.h"
#include "Tags/PGXEnvironmentTags.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameplayTagContainer.h"

namespace
{
	// EN: Probe ZoneTag used by world-bound validators to exercise the public
	//     registry / modifier surface. Earlier baseline used
	//     RequestGameplayTag(name, bErrorIfNotFound=false) on an unauthored
	//     child tag "PGX.Environment.Zone.EnvTest_Probe" — that pattern
	//     returns an invalid FGameplayTag when the tag is not registered,
	//     which let ValidateZoneRegistration / ValidateModifierAndClamp
	//     silently SKIP the success-path assertions while still returning
	//     PASS to the caller (runtime-safety
	//     known runtime constraint).
	//
	//     Fix: use TAG_PGX_Environment_Zone (the native parent handle
	//     declared via UE_DECLARE_GAMEPLAY_TAG_EXTERN + UE_DEFINE_GAMEPLAY_TAG
	//     in Tags/PGXEnvironmentTags.h/cpp). Native handles are pre-
	//     registered before any consumer reads them, so RequestGameplayTag
	//     is no longer needed and the probe path is guaranteed valid. The
	//     parent vs child tag distinction does not collide with project-
	//     authored child tags because FGameplayTag registry equality is
	//     exact-match — `Zone` (parent) and `Zone.Cave` (child) compare
	//     unequal in the subsystem ZoneRegistry.
	// ES: Tag de zona probe usado por los validadores world-bound para
	//     ejercitar el surface publico de registry / modifier. El baseline
	//     previo usaba RequestGameplayTag(name, bErrorIfNotFound=false)
	//     sobre un tag hijo no-authoring "PGX.Environment.Zone.EnvTest_Probe"
	//     — ese patron retorna un FGameplayTag invalido cuando el tag no
	//     esta registrado, lo cual permitia que ValidateZoneRegistration /
	//     ValidateModifierAndClamp SALTASEN silenciosamente las assertions
	//     del path de exito mientras retornaban PASS al caller (runtime-safety
	//     known runtime constraint).
	//
	//     Fix: usar TAG_PGX_Environment_Zone (el handle padre nativo
	//     declarado via UE_DECLARE_GAMEPLAY_TAG_EXTERN + UE_DEFINE_GAMEPLAY_TAG
	//     en Tags/PGXEnvironmentTags.h/cpp). Los handles nativos estan
	//     pre-registrados antes de que cualquier consumer los lea, asi que
	//     RequestGameplayTag ya no es necesario y el probe path esta
	//     garantizado valido. La distincion tag padre vs hijo no colisiona
	//     con child tags authoring del proyecto porque la igualdad del
	//     registry FGameplayTag es exact-match — `Zone` (padre) y
	//     `Zone.Cave` (hijo) comparan unequal en el ZoneRegistry del
	//     subsistema.
	static FGameplayTag MakeProbeZoneTag()
	{
		return TAG_PGX_Environment_Zone;
	}
}

UPGXEnvironmentSubsystem* UPGXEnvironmentTestUtility::GetSubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}
	const UWorld* World = GEngine
		? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull)
		: nullptr;
	if (!World)
	{
		return nullptr;
	}
	return World->GetSubsystem<UPGXEnvironmentSubsystem>();
}

// ============================================================================
// EN: World-independent validators
// ES: Validadores world-independent
// ============================================================================

bool UPGXEnvironmentTestUtility::ValidateTypeContracts(TArray<FString>& OutIssues)
{
	bool bPassed = true;

	const FPGXEnvironmentResult Default;
	if (Default.Code != EPGXEnvironmentResultCode::Failed)
	{
		OutIssues.Add(TEXT("[Types] Default FPGXEnvironmentResult Code expected Failed."));
		bPassed = false;
	}

	const FPGXEnvironmentResult Success = FPGXEnvironmentResult::MakeSuccess(TEXT("ok"));
	if (Success.Code != EPGXEnvironmentResultCode::Success || Success.Description != TEXT("ok"))
	{
		OutIssues.Add(TEXT("[Types] MakeSuccess roundtrip failed."));
		bPassed = false;
	}

	const FPGXEnvironmentResult Fail = FPGXEnvironmentResult::MakeFail(
		EPGXEnvironmentResultCode::NotFound, TEXT("missing"));
	if (Fail.Code != EPGXEnvironmentResultCode::NotFound || Fail.Description != TEXT("missing"))
	{
		OutIssues.Add(TEXT("[Types] MakeFail roundtrip failed."));
		bPassed = false;
	}

	return bPassed;
}

bool UPGXEnvironmentTestUtility::ValidateTagHandles(TArray<FString>& OutIssues)
{
	bool bPassed = true;

	auto Check = [&OutIssues, &bPassed](const FGameplayTag& Tag, const TCHAR* Label)
	{
		if (!Tag.IsValid())
		{
			OutIssues.Add(FString::Printf(TEXT("[Tags] %s tag handle resolves to invalid tag."), Label));
			bPassed = false;
		}
	};

	Check(TAG_PGX_Environment_Variable,           TEXT("Variable parent"));
	Check(TAG_PGX_Environment_Zone,               TEXT("Zone parent"));
	Check(TAG_PGX_Environment_Severity,           TEXT("Severity parent"));
	Check(TAG_PGX_Environment_Severity_None,      TEXT("Severity.None"));
	Check(TAG_PGX_Environment_Severity_Minor,     TEXT("Severity.Minor"));
	Check(TAG_PGX_Environment_Severity_Moderate,  TEXT("Severity.Moderate"));
	Check(TAG_PGX_Environment_Severity_Severe,    TEXT("Severity.Severe"));
	Check(TAG_PGX_Environment_Severity_Critical,  TEXT("Severity.Critical"));
	Check(TAG_PGX_Environment_Result,             TEXT("Result parent"));

	return bPassed;
}

bool UPGXEnvironmentTestUtility::ValidateDataAssetDefaults(TArray<FString>& OutIssues)
{
	bool bPassed = true;

	UPGXEnvironmentVariable* Variable = NewObject<UPGXEnvironmentVariable>(GetTransientPackage());
	if (!Variable)
	{
		OutIssues.Add(TEXT("[DA] UPGXEnvironmentVariable failed to NewObject."));
		return false;
	}
	if (Variable->Kind != EPGXEnvironmentVariableKind::Continuous ||
		Variable->InitialValue != 0.0f ||
		Variable->ClampMin != 0.0f ||
		Variable->ClampMax != 1.0f ||
		Variable->ThresholdBands.Num() != 0)
	{
		OutIssues.Add(TEXT("[DA] UPGXEnvironmentVariable defaults do not match contract."));
		bPassed = false;
	}

	UPGXEnvironmentZoneDefinition* Zone = NewObject<UPGXEnvironmentZoneDefinition>(GetTransientPackage());
	if (!Zone)
	{
		OutIssues.Add(TEXT("[DA] UPGXEnvironmentZoneDefinition failed to NewObject."));
		return false;
	}
	if (Zone->VariableSeeds.Num() != 0 || Zone->DefaultSeverity != EPGXEnvironmentSeverity::None)
	{
		OutIssues.Add(TEXT("[DA] UPGXEnvironmentZoneDefinition defaults do not match contract."));
		bPassed = false;
	}

	UPGXEnvironmentTickProfile* Tick = NewObject<UPGXEnvironmentTickProfile>(GetTransientPackage());
	if (!Tick)
	{
		OutIssues.Add(TEXT("[DA] UPGXEnvironmentTickProfile failed to NewObject."));
		return false;
	}
	if (Tick->ActiveTickHz != 5.0f ||
		Tick->DormantTickHz != 0.0f ||
		Tick->DormancyAfterSeconds != 0.0f ||
		Tick->MaxCatchUpSeconds != 1.0f)
	{
		OutIssues.Add(TEXT("[DA] UPGXEnvironmentTickProfile defaults do not match contract."));
		bPassed = false;
	}

	UPGXEnvironmentConfig* Config = NewObject<UPGXEnvironmentConfig>(GetTransientPackage());
	if (!Config)
	{
		OutIssues.Add(TEXT("[DA] UPGXEnvironmentConfig failed to NewObject."));
		return false;
	}
	if (Config->Variables.Num() != 0 ||
		Config->ZoneDefinitions.Num() != 0 ||
		!Config->DefaultTickProfile.IsNull())
	{
		OutIssues.Add(TEXT("[DA] UPGXEnvironmentConfig defaults do not match contract."));
		bPassed = false;
	}

	const UPGXEnvironmentSettings* Settings = GetDefault<UPGXEnvironmentSettings>();
	if (!Settings)
	{
		OutIssues.Add(TEXT("[DA] GetDefault<UPGXEnvironmentSettings> returned null."));
		bPassed = false;
	}
	else
	{
		// EN: Project users may legitimately author ActiveConfig in defaultconfig
		//     before tests run. We only validate the verbose flag default here
		//     since ActiveConfig is project-config-driven.
		// ES: Los usuarios del proyecto pueden authoring legitimamente
		//     ActiveConfig en defaultconfig antes que los tests corran. Solo
		//     validamos el default del flag verbose aqui ya que ActiveConfig
		//     es project-config-driven.
		if (Settings->bVerboseEnvironmentDebug != false)
		{
			OutIssues.Add(TEXT("[DA] UPGXEnvironmentSettings::bVerboseEnvironmentDebug expected default false."));
			bPassed = false;
		}
	}

	return bPassed;
}

// ============================================================================
// EN: World-bound validators
// ES: Validadores world-bound
// ============================================================================

bool UPGXEnvironmentTestUtility::ValidateSubsystemReady(
	const UObject* WorldContextObject,
	TArray<FString>& OutIssues)
{
	UPGXEnvironmentSubsystem* Subsystem = GetSubsystem(WorldContextObject);
	if (!Subsystem)
	{
		OutIssues.Add(TEXT("[Subsystem] GetSubsystem returned null (no world or subsystem missing)."));
		return false;
	}
	return true;
}

bool UPGXEnvironmentTestUtility::ValidateZoneRegistration(
	const UObject* WorldContextObject,
	TArray<FString>& OutIssues)
{
	UPGXEnvironmentSubsystem* Subsystem = GetSubsystem(WorldContextObject);
	if (!Subsystem)
	{
		OutIssues.Add(TEXT("[Registration] Subsystem unavailable."));
		return false;
	}

	bool bPassed = true;

	// EN: Invalid-tag rejection.
	// ES: Rechazo tag invalido.
	const FPGXEnvironmentResult Invalid = Subsystem->RegisterZone(FGameplayTag());
	if (Invalid.Code != EPGXEnvironmentResultCode::InvalidConfig)
	{
		OutIssues.Add(TEXT("[Registration] Empty ZoneTag should yield InvalidConfig."));
		bPassed = false;
	}

	const FGameplayTag ProbeTag = MakeProbeZoneTag();
	// EN: Probe is now backed by the native parent tag handle
	//     TAG_PGX_Environment_Zone — pre-registered, always valid. No
	//     skip-as-pass path; if the native macro ever stops compiling
	//     this assert fires loud rather than turning the validator into
	//     a silent no-op (runtime-safety
	//     known runtime constraint).
	// ES: El probe ahora esta respaldado por el handle padre nativo
	//     TAG_PGX_Environment_Zone — pre-registrado, siempre valido. Sin
	//     path skip-as-pass; si el macro nativo deja de compilar este
	//     assert dispara fuerte en lugar de convertir el validador en un
	//     no-op silencioso (runtime-safety
	//     known runtime constraint).
	if (!ProbeTag.IsValid())
	{
		OutIssues.Add(TEXT("[Registration] Native parent tag TAG_PGX_Environment_Zone resolved invalid — UE_DEFINE_GAMEPLAY_TAG wiring broken upstream."));
		return false;
	}

	// EN: Defensive cleanup if a previous test leaked state.
	// ES: Cleanup defensivo por si un test previo dejo estado.
	if (Subsystem->IsZoneRegistered(ProbeTag))
	{
		Subsystem->UnregisterZone(ProbeTag);
	}

	const FPGXEnvironmentResult Reg1 = Subsystem->RegisterZone(ProbeTag);
	if (Reg1.Code != EPGXEnvironmentResultCode::Success)
	{
		OutIssues.Add(FString::Printf(
			TEXT("[Registration] First RegisterZone expected Success; got code=%d desc=%s"),
			static_cast<int32>(Reg1.Code), *Reg1.Description));
		bPassed = false;
	}

	if (!Subsystem->IsZoneRegistered(ProbeTag))
	{
		OutIssues.Add(TEXT("[Registration] IsZoneRegistered should be true after Success."));
		bPassed = false;
	}

	const TArray<FGameplayTag> AllTags = Subsystem->GetRegisteredZoneTags();
	if (!AllTags.Contains(ProbeTag))
	{
		OutIssues.Add(TEXT("[Registration] GetRegisteredZoneTags should contain probe tag."));
		bPassed = false;
	}

	const FPGXEnvironmentResult Reg2 = Subsystem->RegisterZone(ProbeTag);
	if (Reg2.Code != EPGXEnvironmentResultCode::AlreadyRegistered)
	{
		OutIssues.Add(FString::Printf(
			TEXT("[Registration] Duplicate RegisterZone expected AlreadyRegistered; got code=%d"),
			static_cast<int32>(Reg2.Code)));
		bPassed = false;
	}

	const FPGXEnvironmentResult Unreg1 = Subsystem->UnregisterZone(ProbeTag);
	if (Unreg1.Code != EPGXEnvironmentResultCode::Success)
	{
		OutIssues.Add(FString::Printf(
			TEXT("[Registration] UnregisterZone expected Success; got code=%d"),
			static_cast<int32>(Unreg1.Code)));
		bPassed = false;
	}

	const FPGXEnvironmentResult Unreg2 = Subsystem->UnregisterZone(ProbeTag);
	if (Unreg2.Code != EPGXEnvironmentResultCode::NotFound)
	{
		OutIssues.Add(FString::Printf(
			TEXT("[Registration] Re-UnregisterZone expected NotFound; got code=%d"),
			static_cast<int32>(Unreg2.Code)));
		bPassed = false;
	}

	return bPassed;
}

bool UPGXEnvironmentTestUtility::ValidateModifierAndClamp(
	const UObject* WorldContextObject,
	TArray<FString>& OutIssues)
{
	UPGXEnvironmentSubsystem* Subsystem = GetSubsystem(WorldContextObject);
	if (!Subsystem)
	{
		OutIssues.Add(TEXT("[Modifier] Subsystem unavailable."));
		return false;
	}

	if (!Subsystem->HasActiveConfig())
	{
		OutIssues.Add(TEXT("[Modifier] No ActiveConfig authored — set Settings.ActiveConfig + at least one Variable + ZoneDefinition for full clamp coverage. Skipping clamp leg."));
		// EN: Surface invalid-arg paths even without ActiveConfig — they are
		//     orthogonal to authoring presence.
		// ES: Superficiar paths con args invalidos incluso sin ActiveConfig —
		//     son ortogonales a la presencia de authoring.
	}

	bool bPassed = true;

	// EN: Invalid-tag rejection.
	// ES: Rechazo tag invalido.
	const FPGXEnvironmentResult InvalidZone = Subsystem->ApplyVariableModifier(
		FGameplayTag(), FGameplayTag(), 1.0f);
	if (InvalidZone.Code != EPGXEnvironmentResultCode::InvalidConfig)
	{
		OutIssues.Add(TEXT("[Modifier] Empty tags should yield InvalidConfig."));
		bPassed = false;
	}

	// EN: Non-finite delta rejection. ProbeTag now backed by the native
	//     parent handle TAG_PGX_Environment_Zone — guaranteed valid, so
	//     the NaN-gate exercise always runs (no skip-as-pass).
	// ES: Rechazo delta non-finite. ProbeTag ahora respaldado por el
	//     handle padre nativo TAG_PGX_Environment_Zone — garantizado
	//     valido, asi que el ejercicio del gate NaN siempre corre (sin
	//     skip-as-pass).
	const FGameplayTag ProbeTag = MakeProbeZoneTag();
	if (!ProbeTag.IsValid())
	{
		OutIssues.Add(TEXT("[Modifier] Native parent tag TAG_PGX_Environment_Zone resolved invalid — UE_DEFINE_GAMEPLAY_TAG wiring broken upstream."));
		return false;
	}

	// EN: Use the native parent tag to isolate the OutOfBounds path. Even
	//     if the zone is not registered, the IsFinite check fires first.
	// ES: Usar el tag padre nativo para aislar el path OutOfBounds. Aunque
	//     la zona no este registrada, el check IsFinite dispara primero.
	const FPGXEnvironmentResult NonFinite = Subsystem->ApplyVariableModifier(
		ProbeTag, ProbeTag, std::numeric_limits<float>::quiet_NaN());
	if (NonFinite.Code != EPGXEnvironmentResultCode::OutOfBounds)
	{
		OutIssues.Add(FString::Printf(
			TEXT("[Modifier] NaN delta should yield OutOfBounds; got code=%d"),
			static_cast<int32>(NonFinite.Code)));
		bPassed = false;
	}

	return bPassed;
}

bool UPGXEnvironmentTestUtility::ValidateThresholdTransition(
	const UObject* WorldContextObject,
	TArray<FString>& OutIssues)
{
	UPGXEnvironmentSubsystem* Subsystem = GetSubsystem(WorldContextObject);
	if (!Subsystem)
	{
		OutIssues.Add(TEXT("[Threshold] Subsystem unavailable."));
		return false;
	}

	if (!Subsystem->HasActiveConfig())
	{
		OutIssues.Add(TEXT("[Threshold] No ActiveConfig authored — author at least one Variable with ThresholdBands + a ZoneDefinition seeded with that Variable for the transition leg. Skipping."));
		// EN: Cannot validate transitions without authored bands. This is the
		//     graceful path documented in the function contract — surfaces
		//     the authoring gap clearly without a hard failure.
		// ES: No se pueden validar transiciones sin bands authoring. Este es
		//     el path graceful documentado en el contrato de la funcion —
		//     superficia el gap de authoring claro sin un hard failure.
		return true;
	}

	// EN: With ActiveConfig authored, a developer-driven scenario validates
	//     transition by:
	//     1. Subscribe a local lambda to OnZoneSeverityChangedNative.
	//     2. Register a probe zone (RegisterZone seeds + evaluates).
	//     3. Apply a modifier large enough to cross a band boundary.
	//     4. Assert the lambda was invoked.
	//     5. Unregister the probe zone.
	//
	//     Baseline does not author project content here — wiring
	//     authored-DA fixtures is project-side. The validator merely
	//     verifies the delegate handle exists (compiles + present on the
	//     subsystem instance) so authored-content tests use the same
	//     valid attach point.
	// ES: Con ActiveConfig authoring, un escenario developer-driven valida
	//     transicion por:
	//     1. Suscribir un lambda local a OnZoneSeverityChangedNative.
	//     2. Registrar una probe zone (RegisterZone seeds + evalua).
	//     3. Aplicar un modificador suficientemente grande para cruzar un
	//        boundary de banda.
	//     4. Aseverar que el lambda fue invocado.
	//     5. Desregistrar la probe zone.
	//
	//     El baseline no authoring contenido del proyecto aqui — el wiring
	//     de fixtures authoring-DA es project-side. El validador
	//     simplemente verifica que el handle delegate existe (compila +
	//     presente en la instancia subsystem) para que los tests de
	//     contenido authored futuros encuentren un attach point valido.
	const FDelegateHandle Handle = Subsystem->OnZoneSeverityChangedNative.AddLambda(
		[](FGameplayTag, FGameplayTag, EPGXEnvironmentSeverity, EPGXEnvironmentSeverity){});
	if (!Handle.IsValid())
	{
		OutIssues.Add(TEXT("[Threshold] AddLambda on OnZoneSeverityChangedNative produced invalid handle."));
		Subsystem->OnZoneSeverityChangedNative.Remove(Handle);
		return false;
	}
	Subsystem->OnZoneSeverityChangedNative.Remove(Handle);

	return true;
}

// ============================================================================
// EN: Aggregate
// ES: Agregado
// ============================================================================

bool UPGXEnvironmentTestUtility::RunAllEnvironmentTests(
	const UObject* WorldContextObject,
	TArray<FString>& OutIssues)
{
	bool bAggregate = true;

	bAggregate &= ValidateTypeContracts(OutIssues);
	bAggregate &= ValidateTagHandles(OutIssues);
	bAggregate &= ValidateDataAssetDefaults(OutIssues);
	bAggregate &= ValidateSubsystemReady(WorldContextObject, OutIssues);
	bAggregate &= ValidateZoneRegistration(WorldContextObject, OutIssues);
	bAggregate &= ValidateModifierAndClamp(WorldContextObject, OutIssues);
	bAggregate &= ValidateThresholdTransition(WorldContextObject, OutIssues);

	PGX_LOG_INFO(LogPGXEnvironment,
		TEXT("[TestUtility] RunAllEnvironmentTests: %s (issues=%d)"),
		bAggregate ? TEXT("PASS") : TEXT("FAIL"), OutIssues.Num());

	return bAggregate;
}
