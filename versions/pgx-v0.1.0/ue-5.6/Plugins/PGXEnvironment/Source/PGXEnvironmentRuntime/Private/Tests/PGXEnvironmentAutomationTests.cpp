// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "PGXEnvironmentConfig.h"
#include "PGXEnvironmentSettings.h"
#include "PGXEnvironmentSubsystem.h"
#include "PGXEnvironmentTickProfile.h"
#include "PGXEnvironmentTypes.h"
#include "PGXEnvironmentVariable.h"
#include "PGXEnvironmentZoneDefinition.h"
#include "Tags/PGXEnvironmentTags.h"

#include "GameplayTagContainer.h"
#include "Observability/PGXObservable.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

#include <limits>

/**
 * EN: Path B WITH_DEV_AUTOMATION_TESTS smoke tests for Behavior PGXEnvironment
 *     baseline. File-level guard + IMPLEMENT_SIMPLE_AUTOMATION_TEST direct
 *     macro — no PGXCoreDeveloper dependency, so PGXEnvironmentRuntime.Build.cs
 *     does not mutate. Naming `PGX.Environment.<Name>` flat per current product boundary
 *     brief.
 *
 *     Scope intentionally narrow at baseline:
 *     - Type construction (FPGXEnvironmentResult MakeSuccess / MakeFail).
 *     - Tag handle validity (caught the RequestGameplayTag-false-returns-
 *       invalid regression class at startup, not at first runtime probe).
 *     - Object DA construction defaults (Variable / Zone / TickProfile /
 *       Config).
 *     - Subsystem class direct NewObject in transient package — validates
 *       UCLASS metadata + the public surface that does NOT require
 *       Initialize (RegisterZone / IsZoneRegistered / GetRegisteredZoneTags
 *       / UnregisterZone / ApplyVariableModifier gate paths).
 *
 *     OUTSIDE THIS TEST SURFACE (no test seam is exposed):
 *     - Subsystem Initialize / Deinitialize lifecycle (requires PIE world
 *       fixture per Loading Configuration world-backed fixture precedent).
 *     - ApplyVariableModifier value-mutation success path with seeded
 *       authored Variables (requires injecting an ActiveConfig — would
 *       need an editor-only test seam that is not exposed, keeping
 *       the production surface clean).
 *     - Threshold-transition emit on OnZoneSeverityChangedNative (same
 *       authored-DA fixture dependency).
 *
 *     The TestUtility BPL (UPGXEnvironmentTestUtility) covers the same
 *     mutation paths from PIE where authored-DA fixtures are reachable —
 *     this file covers the headless / no-PIE surface only.
 *
 * ES: Smoke tests Path B WITH_DEV_AUTOMATION_TESTS para baseline Behavior
 *     PGXEnvironment. Guard file-level + macro IMPLEMENT_SIMPLE_AUTOMATION_TEST
 *     direct — sin dependencia PGXCoreDeveloper, asi que
 *     PGXEnvironmentRuntime.Build.cs no muta. Naming `PGX.Environment.<Name>`
 *     flat per brief current product boundary.
 *
 *     Scope intencionalmente estrecho en baseline:
 *     - Construccion de tipos (FPGXEnvironmentResult MakeSuccess / MakeFail).
 *     - Validez de tag handles (atrapa la clase de regresion
 *       RequestGameplayTag-false-returns-invalid en startup, no en el
 *       primer probe runtime).
 *     - Defaults de construccion Object DA (Variable / Zone / TickProfile
 *       / Config).
 *     - NewObject directo del subsystem class en transient package —
 *       valida metadata UCLASS + el surface publico que NO requiere
 *       Initialize (RegisterZone / IsZoneRegistered / GetRegisteredZoneTags
 *       / UnregisterZone / ApplyVariableModifier gate paths).
 *
 *     DIFERIDO a version futura (sin test seam introducido):
 *     - Lifecycle Initialize / Deinitialize del subsystem (requiere PIE
 *       world fixture per precedente Loading Configuration world-backed fixture).
 *     - Path de exito mutacion-valor de ApplyVariableModifier con
 *       Variables authoring seeded (requiere inyectar un ActiveConfig —
 *       necesitaria un test seam editor-only en el subsystem; diferido
 *       para mantener el surface productivo limpio).
 *     - Emit transicion-threshold en OnZoneSeverityChangedNative (misma
 *       dependencia de fixture authoring-DA).
 *
 *     El BPL TestUtility (UPGXEnvironmentTestUtility) cubre los mismos
 *     paths de mutacion desde PIE donde los fixtures authoring-DA son
 *     alcanzables — este archivo cubre solo el surface headless / no-PIE.
 */

// ============================================================================
// EN: Type contracts
// ES: Contratos de tipo
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXEnvironmentResultDefaultIsFailed,
	"PGX.Environment.ResultDefaultIsFailed",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXEnvironmentResultDefaultIsFailed::RunTest(const FString& /*Parameters*/)
{
	const FPGXEnvironmentResult Default;
	TestEqual(
		TEXT("Default FPGXEnvironmentResult Code must be Failed"),
		static_cast<int32>(Default.Code),
		static_cast<int32>(EPGXEnvironmentResultCode::Failed));
	TestTrue(
		TEXT("Default FPGXEnvironmentResult Description must be empty"),
		Default.Description.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXEnvironmentResultMakeSuccessRoundtrip,
	"PGX.Environment.ResultMakeSuccessRoundtrip",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXEnvironmentResultMakeSuccessRoundtrip::RunTest(const FString& /*Parameters*/)
{
	const FPGXEnvironmentResult R = FPGXEnvironmentResult::MakeSuccess(TEXT("ok"));
	TestEqual(
		TEXT("MakeSuccess Code must be Success"),
		static_cast<int32>(R.Code),
		static_cast<int32>(EPGXEnvironmentResultCode::Success));
	TestEqual(TEXT("MakeSuccess Description roundtrip"), R.Description, FString(TEXT("ok")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXEnvironmentResultMakeFailRoundtrip,
	"PGX.Environment.ResultMakeFailRoundtrip",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXEnvironmentResultMakeFailRoundtrip::RunTest(const FString& /*Parameters*/)
{
	const FPGXEnvironmentResult R = FPGXEnvironmentResult::MakeFail(
		EPGXEnvironmentResultCode::NotFound, TEXT("missing"));
	TestEqual(
		TEXT("MakeFail Code roundtrip"),
		static_cast<int32>(R.Code),
		static_cast<int32>(EPGXEnvironmentResultCode::NotFound));
	TestEqual(TEXT("MakeFail Description roundtrip"), R.Description, FString(TEXT("missing")));
	return true;
}

// ============================================================================
// EN: Tag handles
// ES: Handles de tag
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXEnvironmentTagHandlesValid,
	"PGX.Environment.TagHandlesValid",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXEnvironmentTagHandlesValid::RunTest(const FString& /*Parameters*/)
{
	// EN: UE_DEFINE_GAMEPLAY_TAG produces FNativeGameplayTag wrappers that do
	//     NOT expose IsValid() directly — call .GetTag() to obtain the
	//     underlying FGameplayTag and then IsValid(). UE 5.6 API discipline
	//     surfaced by Validation compile gate iter 4 (runtime-safety
	//     known engine constraint).
	// ES: UE_DEFINE_GAMEPLAY_TAG produce wrappers FNativeGameplayTag que NO
	//     exponen IsValid() directamente — llamar .GetTag() para obtener el
	//     FGameplayTag subyacente y luego IsValid(). Disciplina API UE 5.6
	//     superficiada por Validation compile gate iter 4 (runtime-safety
	//     known engine constraint).
	TestTrue(TEXT("TAG_PGX_Environment_Variable valid"),  TAG_PGX_Environment_Variable.GetTag().IsValid());
	TestTrue(TEXT("TAG_PGX_Environment_Zone valid"),      TAG_PGX_Environment_Zone.GetTag().IsValid());
	TestTrue(TEXT("TAG_PGX_Environment_Severity valid"),  TAG_PGX_Environment_Severity.GetTag().IsValid());
	TestTrue(TEXT("TAG_PGX_Environment_Severity_None valid"),     TAG_PGX_Environment_Severity_None.GetTag().IsValid());
	TestTrue(TEXT("TAG_PGX_Environment_Severity_Minor valid"),    TAG_PGX_Environment_Severity_Minor.GetTag().IsValid());
	TestTrue(TEXT("TAG_PGX_Environment_Severity_Moderate valid"), TAG_PGX_Environment_Severity_Moderate.GetTag().IsValid());
	TestTrue(TEXT("TAG_PGX_Environment_Severity_Severe valid"),   TAG_PGX_Environment_Severity_Severe.GetTag().IsValid());
	TestTrue(TEXT("TAG_PGX_Environment_Severity_Critical valid"), TAG_PGX_Environment_Severity_Critical.GetTag().IsValid());
	TestTrue(TEXT("TAG_PGX_Environment_Result valid"),            TAG_PGX_Environment_Result.GetTag().IsValid());
	return true;
}

// ============================================================================
// EN: Data Asset defaults
// ES: Defaults de Data Asset
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXEnvironmentVariableDefaults,
	"PGX.Environment.VariableDefaults",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXEnvironmentVariableDefaults::RunTest(const FString& /*Parameters*/)
{
	UPGXEnvironmentVariable* Variable = NewObject<UPGXEnvironmentVariable>(
		GetTransientPackage(),
		UPGXEnvironmentVariable::StaticClass(),
		NAME_None,
		RF_Transient);
	if (!TestNotNull(TEXT("UPGXEnvironmentVariable instance"), Variable))
	{
		return false;
	}
	TestEqual(TEXT("Variable Kind default"),         static_cast<int32>(Variable->Kind), static_cast<int32>(EPGXEnvironmentVariableKind::Continuous));
	TestEqual(TEXT("Variable InitialValue default"), Variable->InitialValue, 0.0f);
	TestEqual(TEXT("Variable ClampMin default"),     Variable->ClampMin, 0.0f);
	TestEqual(TEXT("Variable ClampMax default"),     Variable->ClampMax, 1.0f);
	TestEqual(TEXT("Variable ThresholdBands default empty"), Variable->ThresholdBands.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXEnvironmentZoneDefinitionDefaults,
	"PGX.Environment.ZoneDefinitionDefaults",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXEnvironmentZoneDefinitionDefaults::RunTest(const FString& /*Parameters*/)
{
	UPGXEnvironmentZoneDefinition* Zone = NewObject<UPGXEnvironmentZoneDefinition>(
		GetTransientPackage(),
		UPGXEnvironmentZoneDefinition::StaticClass(),
		NAME_None,
		RF_Transient);
	if (!TestNotNull(TEXT("UPGXEnvironmentZoneDefinition instance"), Zone))
	{
		return false;
	}
	TestEqual(TEXT("Zone VariableSeeds default empty"), Zone->VariableSeeds.Num(), 0);
	TestEqual(
		TEXT("Zone DefaultSeverity default"),
		static_cast<int32>(Zone->DefaultSeverity),
		static_cast<int32>(EPGXEnvironmentSeverity::None));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXEnvironmentTickProfileDefaults,
	"PGX.Environment.TickProfileDefaults",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXEnvironmentTickProfileDefaults::RunTest(const FString& /*Parameters*/)
{
	UPGXEnvironmentTickProfile* Tick = NewObject<UPGXEnvironmentTickProfile>(
		GetTransientPackage(),
		UPGXEnvironmentTickProfile::StaticClass(),
		NAME_None,
		RF_Transient);
	if (!TestNotNull(TEXT("UPGXEnvironmentTickProfile instance"), Tick))
	{
		return false;
	}
	TestEqual(TEXT("ActiveTickHz default"),          Tick->ActiveTickHz, 5.0f);
	TestEqual(TEXT("DormantTickHz default"),         Tick->DormantTickHz, 0.0f);
	TestEqual(TEXT("DormancyAfterSeconds default"),  Tick->DormancyAfterSeconds, 0.0f);
	TestEqual(TEXT("MaxCatchUpSeconds default"),     Tick->MaxCatchUpSeconds, 1.0f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXEnvironmentConfigDefaults,
	"PGX.Environment.ConfigDefaults",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXEnvironmentConfigDefaults::RunTest(const FString& /*Parameters*/)
{
	UPGXEnvironmentConfig* Config = NewObject<UPGXEnvironmentConfig>(
		GetTransientPackage(),
		UPGXEnvironmentConfig::StaticClass(),
		NAME_None,
		RF_Transient);
	if (!TestNotNull(TEXT("UPGXEnvironmentConfig instance"), Config))
	{
		return false;
	}
	TestEqual(TEXT("Config Variables default empty"),       Config->Variables.Num(), 0);
	TestEqual(TEXT("Config ZoneDefinitions default empty"), Config->ZoneDefinitions.Num(), 0);
	TestTrue(TEXT("Config DefaultTickProfile default null"), Config->DefaultTickProfile.IsNull());
	return true;
}

// ============================================================================
// EN: Subsystem direct surface (no PIE / no Initialize)
// ES: Surface directo del subsystem (sin PIE / sin Initialize)
// ============================================================================

namespace
{
	UPGXEnvironmentSubsystem* MakeTransientSubsystem()
	{
		// EN: Direct NewObject of the subsystem class is acceptable here for
		//     surface-only smoke testing — Initialize is NOT called, so the
		//     state-dependent paths (ActiveConfig resolution, Settings read)
		//     are NOT exercised. Only the Initialize-independent surface is
		//     covered: registry mutation + tag validity gates + non-finite
		//     delta gate. Lifecycle tests are outside this file-level test surface
		//     contract.
		// ES: NewObject directo de la clase subsistema es aceptable aqui
		//     para smoke testing solo-surface — Initialize NO se llama, asi
		//     que los paths state-dependent (resolucion ActiveConfig,
		//     lectura Settings) NO se ejercitan. Solo el surface
		//     Initialize-independent se cubre: mutacion del registry +
		//     gates de tag validity + gate de delta non-finite. Tests de
		//     lifecycle diferidos per contrato en el header file-level.
		return NewObject<UPGXEnvironmentSubsystem>(
			GetTransientPackage(),
			UPGXEnvironmentSubsystem::StaticClass(),
			NAME_None,
			RF_Transient);
	}

	FGameplayTag MakeTestZoneTag()
	{
		// EN: Use the parent Zone tag to guarantee validity even if the
		//     project does not author probe child tags. Functional for
		//     gate / register-by-tag exercise.
		// ES: Usar el tag padre Zone para garantizar validez incluso si el
		//     proyecto no authoring tags hijo de probe. Funcional para
		//     ejercicio de gate / register-por-tag.
		return TAG_PGX_Environment_Zone;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXEnvironmentSubsystemNewObject,
	"PGX.Environment.SubsystemNewObject",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXEnvironmentSubsystemNewObject::RunTest(const FString& /*Parameters*/)
{
	UPGXEnvironmentSubsystem* Subsystem = MakeTransientSubsystem();
	if (!TestNotNull(TEXT("UPGXEnvironmentSubsystem instance"), Subsystem))
	{
		return false;
	}
	TestFalse(
		TEXT("Fresh subsystem has no ActiveConfig (Initialize not called)"),
		Subsystem->HasActiveConfig());
	TestFalse(
		TEXT("Fresh subsystem IsVerbose() default false"),
		Subsystem->IsVerbose());
	TestEqual(
		TEXT("Fresh subsystem GetRegisteredZoneTags() empty"),
		Subsystem->GetRegisteredZoneTags().Num(),
		0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXEnvironmentSubsystemRegisterRejectsInvalidTag,
	"PGX.Environment.SubsystemRegisterRejectsInvalidTag",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXEnvironmentSubsystemRegisterRejectsInvalidTag::RunTest(const FString& /*Parameters*/)
{
	UPGXEnvironmentSubsystem* Subsystem = MakeTransientSubsystem();
	if (!TestNotNull(TEXT("UPGXEnvironmentSubsystem instance"), Subsystem))
	{
		return false;
	}
	const FPGXEnvironmentResult R = Subsystem->RegisterZone(FGameplayTag());
	TestEqual(
		TEXT("Empty ZoneTag yields InvalidConfig"),
		static_cast<int32>(R.Code),
		static_cast<int32>(EPGXEnvironmentResultCode::InvalidConfig));
	TestFalse(TEXT("InvalidConfig path returns non-empty Description"), R.Description.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXEnvironmentSubsystemRegisterAndQuery,
	"PGX.Environment.SubsystemRegisterAndQuery",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXEnvironmentSubsystemRegisterAndQuery::RunTest(const FString& /*Parameters*/)
{
	UPGXEnvironmentSubsystem* Subsystem = MakeTransientSubsystem();
	if (!TestNotNull(TEXT("UPGXEnvironmentSubsystem instance"), Subsystem))
	{
		return false;
	}
	const FGameplayTag Tag = MakeTestZoneTag();
	if (!TestTrue(TEXT("Test tag valid"), Tag.IsValid()))
	{
		return false;
	}

	const FPGXEnvironmentResult Reg = Subsystem->RegisterZone(Tag);
	TestEqual(
		TEXT("RegisterZone Success"),
		static_cast<int32>(Reg.Code),
		static_cast<int32>(EPGXEnvironmentResultCode::Success));

	TestTrue(TEXT("IsZoneRegistered after Success"), Subsystem->IsZoneRegistered(Tag));
	TestEqual(TEXT("GetRegisteredZoneTags has 1 entry"), Subsystem->GetRegisteredZoneTags().Num(), 1);
	TestTrue(
		TEXT("GetRegisteredZoneTags contains test tag"),
		Subsystem->GetRegisteredZoneTags().Contains(Tag));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXEnvironmentSubsystemDuplicateRegisterRejected,
	"PGX.Environment.SubsystemDuplicateRegisterRejected",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXEnvironmentSubsystemDuplicateRegisterRejected::RunTest(const FString& /*Parameters*/)
{
	UPGXEnvironmentSubsystem* Subsystem = MakeTransientSubsystem();
	if (!TestNotNull(TEXT("UPGXEnvironmentSubsystem instance"), Subsystem))
	{
		return false;
	}
	const FGameplayTag Tag = MakeTestZoneTag();
	Subsystem->RegisterZone(Tag);
	const FPGXEnvironmentResult R = Subsystem->RegisterZone(Tag);
	TestEqual(
		TEXT("Duplicate RegisterZone yields AlreadyRegistered"),
		static_cast<int32>(R.Code),
		static_cast<int32>(EPGXEnvironmentResultCode::AlreadyRegistered));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXEnvironmentSubsystemUnregisterRoundtrip,
	"PGX.Environment.SubsystemUnregisterRoundtrip",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXEnvironmentSubsystemUnregisterRoundtrip::RunTest(const FString& /*Parameters*/)
{
	UPGXEnvironmentSubsystem* Subsystem = MakeTransientSubsystem();
	if (!TestNotNull(TEXT("UPGXEnvironmentSubsystem instance"), Subsystem))
	{
		return false;
	}
	const FGameplayTag Tag = MakeTestZoneTag();

	const FPGXEnvironmentResult InvalidUnreg = Subsystem->UnregisterZone(FGameplayTag());
	TestEqual(
		TEXT("Empty UnregisterZone yields InvalidConfig"),
		static_cast<int32>(InvalidUnreg.Code),
		static_cast<int32>(EPGXEnvironmentResultCode::InvalidConfig));

	const FPGXEnvironmentResult NotFoundUnreg = Subsystem->UnregisterZone(Tag);
	TestEqual(
		TEXT("Unknown UnregisterZone yields NotFound"),
		static_cast<int32>(NotFoundUnreg.Code),
		static_cast<int32>(EPGXEnvironmentResultCode::NotFound));

	Subsystem->RegisterZone(Tag);
	const FPGXEnvironmentResult Unreg = Subsystem->UnregisterZone(Tag);
	TestEqual(
		TEXT("Known UnregisterZone yields Success"),
		static_cast<int32>(Unreg.Code),
		static_cast<int32>(EPGXEnvironmentResultCode::Success));

	TestFalse(
		TEXT("IsZoneRegistered after Unregister returns false"),
		Subsystem->IsZoneRegistered(Tag));
	TestEqual(
		TEXT("GetRegisteredZoneTags empty after Unregister"),
		Subsystem->GetRegisteredZoneTags().Num(),
		0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXEnvironmentSubsystemModifierRejectsInvalidTags,
	"PGX.Environment.SubsystemModifierRejectsInvalidTags",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXEnvironmentSubsystemModifierRejectsInvalidTags::RunTest(const FString& /*Parameters*/)
{
	UPGXEnvironmentSubsystem* Subsystem = MakeTransientSubsystem();
	if (!TestNotNull(TEXT("UPGXEnvironmentSubsystem instance"), Subsystem))
	{
		return false;
	}
	const FPGXEnvironmentResult R = Subsystem->ApplyVariableModifier(
		FGameplayTag(), FGameplayTag(), 1.0f);
	TestEqual(
		TEXT("Empty tags yield InvalidConfig"),
		static_cast<int32>(R.Code),
		static_cast<int32>(EPGXEnvironmentResultCode::InvalidConfig));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXEnvironmentSubsystemModifierRejectsNonFiniteDelta,
	"PGX.Environment.SubsystemModifierRejectsNonFiniteDelta",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXEnvironmentSubsystemModifierRejectsNonFiniteDelta::RunTest(const FString& /*Parameters*/)
{
	UPGXEnvironmentSubsystem* Subsystem = MakeTransientSubsystem();
	if (!TestNotNull(TEXT("UPGXEnvironmentSubsystem instance"), Subsystem))
	{
		return false;
	}
	const FGameplayTag Tag = MakeTestZoneTag();

	const FPGXEnvironmentResult Nan = Subsystem->ApplyVariableModifier(
		Tag, Tag, std::numeric_limits<float>::quiet_NaN());
	TestEqual(
		TEXT("NaN delta yields OutOfBounds"),
		static_cast<int32>(Nan.Code),
		static_cast<int32>(EPGXEnvironmentResultCode::OutOfBounds));

	const FPGXEnvironmentResult PosInf = Subsystem->ApplyVariableModifier(
		Tag, Tag, std::numeric_limits<float>::infinity());
	TestEqual(
		TEXT("+Inf delta yields OutOfBounds"),
		static_cast<int32>(PosInf.Code),
		static_cast<int32>(EPGXEnvironmentResultCode::OutOfBounds));

	const FPGXEnvironmentResult NegInf = Subsystem->ApplyVariableModifier(
		Tag, Tag, -std::numeric_limits<float>::infinity());
	TestEqual(
		TEXT("-Inf delta yields OutOfBounds"),
		static_cast<int32>(NegInf.Code),
		static_cast<int32>(EPGXEnvironmentResultCode::OutOfBounds));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXEnvironmentSubsystemModifierUnregisteredZone,
	"PGX.Environment.SubsystemModifierUnregisteredZone",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXEnvironmentSubsystemModifierUnregisteredZone::RunTest(const FString& /*Parameters*/)
{
	UPGXEnvironmentSubsystem* Subsystem = MakeTransientSubsystem();
	if (!TestNotNull(TEXT("UPGXEnvironmentSubsystem instance"), Subsystem))
	{
		return false;
	}
	const FGameplayTag Tag = MakeTestZoneTag();
	const FPGXEnvironmentResult R = Subsystem->ApplyVariableModifier(Tag, Tag, 0.5f);
	TestEqual(
		TEXT("Unregistered zone yields NotFound"),
		static_cast<int32>(R.Code),
		static_cast<int32>(EPGXEnvironmentResultCode::NotFound));
	return true;
}

// ============================================================================
// EN: Observability 8.3.B alpha pilot
// ES: Piloto Observability 8.3.B
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXEnvironmentObservableDataAssetContracts,
	"PGX.Environment.ObservableDataAssetContracts",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXEnvironmentObservableDataAssetContracts::RunTest(const FString& /*Parameters*/)
{
	TestTrue(
		TEXT("UPGXEnvironmentConfig implements IPGXObservable"),
		UPGXEnvironmentConfig::StaticClass()->ImplementsInterface(UPGXObservable::StaticClass()));
	TestTrue(
		TEXT("UPGXEnvironmentVariable implements IPGXObservable"),
		UPGXEnvironmentVariable::StaticClass()->ImplementsInterface(UPGXObservable::StaticClass()));
	TestTrue(
		TEXT("UPGXEnvironmentZoneDefinition implements IPGXObservable"),
		UPGXEnvironmentZoneDefinition::StaticClass()->ImplementsInterface(UPGXObservable::StaticClass()));
	TestTrue(
		TEXT("UPGXEnvironmentTickProfile implements IPGXObservable"),
		UPGXEnvironmentTickProfile::StaticClass()->ImplementsInterface(UPGXObservable::StaticClass()));

	UPGXEnvironmentVariable* Variable = NewObject<UPGXEnvironmentVariable>(
		GetTransientPackage(),
		UPGXEnvironmentVariable::StaticClass(),
		NAME_None,
		RF_Transient);
	if (!TestNotNull(TEXT("Observable variable instance"), Variable))
	{
		return false;
	}

	const FPGXJsonValue Json = Variable->ToJson();
	const FString ExpectedType = FString::Printf(TEXT("\"type\":\"%s\""), *Variable->GetClass()->GetName());
	TestFalse(TEXT("Observable ToJson returns non-empty envelope"), Json.IsEmpty());
	TestTrue(
		TEXT("Observable envelope includes class name"),
		Json.JsonString.Contains(ExpectedType));
	TestEqual(
		TEXT("Observable schema version baseline"),
		Variable->GetSchemaVersion(),
		FName(TEXT("1.0")));

	const FPGXSchemaDescriptor Descriptor = Variable->GetSchemaDescriptor();
	TestEqual(TEXT("Descriptor type name"), Descriptor.TypeName, UPGXEnvironmentVariable::StaticClass()->GetFName());
	TestTrue(TEXT("Descriptor exposes reflected fields"), Descriptor.Fields.Num() > 0);

	const FPGXValidationResult EmptyPayload = Variable->FromJson(FPGXJsonValue());
	TestFalse(TEXT("FromJson empty payload fails visibly"), EmptyPayload.bValid);
	TestTrue(TEXT("FromJson empty payload reports errors"), EmptyPayload.Errors.Num() > 0);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
