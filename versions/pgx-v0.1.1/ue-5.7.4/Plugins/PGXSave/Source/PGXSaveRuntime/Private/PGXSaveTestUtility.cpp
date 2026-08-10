// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXSaveTestUtility.h"
#include "PGXSaveSubsystem.h"
#include "PGXSaveConfig.h"
#include "PGXSaveGame.h"
#include "PGXSaveVersioning.h"
#include "Logging/PGXLogCategories.h"
#include "Logging/PGXLogMacros.h"
#include "Engine/GameInstance.h"

// EN: Test utility for the PGX Save system — validates data integrity,
//     slot operations, versioning, and performance.
// ES: Utilidad de test para el sistema PGX Save — valida integridad de datos,
//     operaciones de slots, versionado, y rendimiento.

// EN: Prefix for all test slot names to avoid colliding with user data
// ES: Prefijo para todos los nombres de slots de test para evitar colisiones con datos de usuario
static const FString TestSlotPrefix = TEXT("_PGXTest_");

// EN: Test key prefix for key-value data
// ES: Prefijo de key para datos key-value de test
static const FName TestKeyPrefix = TEXT("_PGXTest");

// ============================================================================
// EN: Internal helpers
// ES: Helpers internos
// ============================================================================

UPGXSaveSubsystem* UPGXSaveTestUtility::GetSubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return nullptr;
	}

	UGameInstance* GI = World->GetGameInstance();
	return GI ? GI->GetSubsystem<UPGXSaveSubsystem>() : nullptr;
}

void UPGXSaveTestUtility::LogTestResult(const FString& TestName, bool bPassed, const FString& Details)
{
	if (bPassed)
	{
		PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility] PASS: %s %s"),
			*TestName, Details.IsEmpty() ? TEXT("") : *FString::Printf(TEXT("(%s)"), *Details));
	}
	else
	{
		PGX_LOG_ERROR(LogPGXSave, TEXT("[TestUtility] FAIL: %s %s"),
			*TestName, Details.IsEmpty() ? TEXT("") : *FString::Printf(TEXT("(%s)"), *Details));
	}
}

FGameplayTag UPGXSaveTestUtility::GetFirstDomainTag(UPGXSaveSubsystem* Sub, FGameplayTag ContextTag)
{
	const UPGXSaveConfig* Config = Sub->GetContextConfig(ContextTag);
	if (Config && Config->SaveDomains.Num() > 0)
	{
		return Config->SaveDomains[0].DomainTag;
	}
	return FGameplayTag();
}

// ============================================================================
// EN: RunQuickTest
// ES: Test Rapido
// ============================================================================

void UPGXSaveTestUtility::RunQuickTest(const UObject* WorldContextObject)
{
	PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility] ========== RunQuickTest START =========="));

	UPGXSaveSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		LogTestResult(TEXT("RunQuickTest"), false, TEXT("SaveSubsystem not found"));
		return;
	}

	// EN: Auto-discover first context
	// ES: Auto-descubrir primer contexto
	TArray<FGameplayTag> ContextTags = Sub->GetAllContextTags();
	if (ContextTags.Num() == 0)
	{
		LogTestResult(TEXT("RunQuickTest"), false, TEXT("No contexts discovered - create a UPGXSaveConfig DA first"));
		return;
	}

	const FGameplayTag ContextTag = ContextTags[0];
	const FGameplayTag DomainTag = GetFirstDomainTag(Sub, ContextTag);
	if (!DomainTag.IsValid())
	{
		LogTestResult(TEXT("RunQuickTest"), false, TEXT("No domains in first context"));
		return;
	}

	PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility] Using context: %s, domain: %s"),
		*ContextTag.ToString(), *DomainTag.ToString());

	const FString SlotName = TestSlotPrefix + TEXT("Quick");

	// EN: Step 1 — Write test data (all 8 types)
	// ES: Paso 1 — Escribir datos de test (los 8 tipos)
	UPGXSaveGame* SG = Sub->GetSaveGame(DomainTag);
	if (!SG)
	{
		LogTestResult(TEXT("RunQuickTest"), false, TEXT("GetSaveGame returned null"));
		return;
	}

	SG->WriteString(FName("_PGXTest_String"), TEXT("HelloPGX"));
	SG->WriteInt(FName("_PGXTest_Int"), 42);
	SG->WriteFloat(FName("_PGXTest_Float"), 3.14f);
	SG->WriteBool(FName("_PGXTest_Bool"), true);
	SG->WriteVector(FName("_PGXTest_Vector"), FVector(1.0, 2.0, 3.0));
	SG->WriteRotator(FName("_PGXTest_Rotator"), FRotator(10.0f, 20.0f, 30.0f));
	SG->WriteTransform(FName("_PGXTest_Transform"), FTransform(FRotator::ZeroRotator, FVector(100, 200, 300)));
	SG->WriteTag(FName("_PGXTest_Tag"), FGameplayTag::RequestGameplayTag(FName("PGX.Save"), false));

	// EN: Step 2 — Save to disk
	// ES: Paso 2 — Guardar en disco
	const EPGXSaveResult SaveResult = Sub->SaveContext(ContextTag, SlotName);
	LogTestResult(TEXT("QuickTest.Save"), SaveResult == EPGXSaveResult::Success,
		FString::Printf(TEXT("Result=%d"), static_cast<int32>(SaveResult)));

	if (SaveResult != EPGXSaveResult::Success)
	{
		return;
	}

	// EN: Step 3 — Clear in-memory data
	// ES: Paso 3 — Limpiar datos en memoria
	Sub->ClearDomain(DomainTag);

	// EN: Step 4 — Load from disk
	// ES: Paso 4 — Cargar desde disco
	const EPGXSaveResult LoadResult = Sub->LoadContext(ContextTag, SlotName);
	LogTestResult(TEXT("QuickTest.Load"), LoadResult == EPGXSaveResult::Success,
		FString::Printf(TEXT("Result=%d"), static_cast<int32>(LoadResult)));

	if (LoadResult != EPGXSaveResult::Success)
	{
		return;
	}

	// EN: Step 5 — Verify all 8 types
	// ES: Paso 5 — Verificar los 8 tipos
	SG = Sub->GetSaveGame(DomainTag);
	if (!SG)
	{
		LogTestResult(TEXT("QuickTest.Verify"), false, TEXT("GetSaveGame returned null after load"));
		return;
	}

	int32 PassCount = 0;
	int32 FailCount = 0;

	auto Check = [&](const FString& TypeName, bool bMatch)
	{
		if (bMatch) { ++PassCount; } else { ++FailCount; }
		LogTestResult(FString::Printf(TEXT("QuickTest.%s"), *TypeName), bMatch);
	};

	Check(TEXT("String"),    SG->ReadString(FName("_PGXTest_String")) == TEXT("HelloPGX"));
	Check(TEXT("Int"),       SG->ReadInt(FName("_PGXTest_Int")) == 42);
	Check(TEXT("Float"),     FMath::IsNearlyEqual(SG->ReadFloat(FName("_PGXTest_Float")), 3.14f, 0.001f));
	Check(TEXT("Bool"),      SG->ReadBool(FName("_PGXTest_Bool")));
	Check(TEXT("Vector"),    SG->ReadVector(FName("_PGXTest_Vector")).Equals(FVector(1.0, 2.0, 3.0), 0.01));
	Check(TEXT("Rotator"),   SG->ReadRotator(FName("_PGXTest_Rotator")).Equals(FRotator(10.0f, 20.0f, 30.0f), 0.01f));
	Check(TEXT("Transform"), SG->ReadTransform(FName("_PGXTest_Transform")).GetLocation().Equals(FVector(100, 200, 300), 0.01));
	Check(TEXT("Tag"),       SG->ReadTag(FName("_PGXTest_Tag")) == FGameplayTag::RequestGameplayTag(FName("PGX.Save"), false));

	// EN: Step 6 — Cleanup test slot
	// ES: Paso 6 — Limpiar slot de test
	Sub->DeleteSlot(ContextTag, SlotName);

	PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility] RunQuickTest COMPLETE: %d/%d passed"),
		PassCount, PassCount + FailCount);
	PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility] ========== RunQuickTest END =========="));
}

// ============================================================================
// EN: TestKeyValueRoundTrip
// ES: Test de Roundtrip Key-Value
// ============================================================================

void UPGXSaveTestUtility::TestKeyValueRoundTrip(const UObject* WorldContextObject, FGameplayTag ContextTag)
{
	PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility] ========== TestKeyValueRoundTrip START =========="));

	UPGXSaveSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		LogTestResult(TEXT("KeyValueRoundTrip"), false, TEXT("SaveSubsystem not found"));
		return;
	}

	const FGameplayTag DomainTag = GetFirstDomainTag(Sub, ContextTag);
	if (!DomainTag.IsValid())
	{
		LogTestResult(TEXT("KeyValueRoundTrip"), false, TEXT("No domains in context"));
		return;
	}

	const FString SlotName = TestSlotPrefix + TEXT("KV");
	UPGXSaveGame* SG = Sub->GetSaveGame(DomainTag);
	if (!SG) { LogTestResult(TEXT("KeyValueRoundTrip"), false, TEXT("null SaveGame")); return; }

	// EN: Write diverse test data — multiple keys per type
	// ES: Escribir datos de test diversos — multiples keys por tipo
	for (int32 i = 0; i < 5; ++i)
	{
		const FName KeyStr = FName(*FString::Printf(TEXT("_PGXTest_Str_%d"), i));
		const FName KeyInt = FName(*FString::Printf(TEXT("_PGXTest_Int_%d"), i));
		const FName KeyFlt = FName(*FString::Printf(TEXT("_PGXTest_Flt_%d"), i));
		const FName KeyBool = FName(*FString::Printf(TEXT("_PGXTest_Bool_%d"), i));

		SG->WriteString(KeyStr, FString::Printf(TEXT("Value_%d"), i * 100));
		SG->WriteInt(KeyInt, i * 1000 + 7);
		SG->WriteFloat(KeyFlt, static_cast<float>(i) * 1.5f + 0.25f);
		SG->WriteBool(KeyBool, i % 2 == 0);
	}

	SG->WriteVector(FName("_PGXTest_Vec3D"), FVector(999.0, -888.0, 777.0));
	SG->WriteRotator(FName("_PGXTest_Rot3D"), FRotator(45.0f, 90.0f, 135.0f));
	SG->WriteTransform(FName("_PGXTest_Xform"), FTransform(FRotator(1, 2, 3), FVector(10, 20, 30), FVector(2, 2, 2)));
	SG->WriteTag(FName("_PGXTest_GTag"), FGameplayTag::RequestGameplayTag(FName("PGX.Save"), false));

	// EN: Save → Clear → Load
	// ES: Guardar → Limpiar → Cargar
	EPGXSaveResult Res = Sub->SaveContext(ContextTag, SlotName);
	LogTestResult(TEXT("KV.Save"), Res == EPGXSaveResult::Success);
	if (Res != EPGXSaveResult::Success) return;

	Sub->ClearDomain(DomainTag);

	Res = Sub->LoadContext(ContextTag, SlotName);
	LogTestResult(TEXT("KV.Load"), Res == EPGXSaveResult::Success);
	if (Res != EPGXSaveResult::Success) return;

	// EN: Verify
	// ES: Verificar
	SG = Sub->GetSaveGame(DomainTag);
	int32 PassCount = 0;
	int32 FailCount = 0;

	auto Check = [&](bool bMatch) { if (bMatch) ++PassCount; else ++FailCount; };

	for (int32 i = 0; i < 5; ++i)
	{
		Check(SG->ReadString(FName(*FString::Printf(TEXT("_PGXTest_Str_%d"), i))) == FString::Printf(TEXT("Value_%d"), i * 100));
		Check(SG->ReadInt(FName(*FString::Printf(TEXT("_PGXTest_Int_%d"), i))) == i * 1000 + 7);
		Check(FMath::IsNearlyEqual(SG->ReadFloat(FName(*FString::Printf(TEXT("_PGXTest_Flt_%d"), i))), static_cast<float>(i) * 1.5f + 0.25f, 0.001f));
		Check(SG->ReadBool(FName(*FString::Printf(TEXT("_PGXTest_Bool_%d"), i))) == (i % 2 == 0));
	}

	Check(SG->ReadVector(FName("_PGXTest_Vec3D")).Equals(FVector(999.0, -888.0, 777.0), 0.01));
	Check(SG->ReadRotator(FName("_PGXTest_Rot3D")).Equals(FRotator(45.0f, 90.0f, 135.0f), 0.01f));
	Check(SG->ReadTransform(FName("_PGXTest_Xform")).GetLocation().Equals(FVector(10, 20, 30), 0.01));
	Check(SG->ReadTag(FName("_PGXTest_GTag")) == FGameplayTag::RequestGameplayTag(FName("PGX.Save"), false));

	Sub->DeleteSlot(ContextTag, SlotName);

	PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility] KeyValueRoundTrip: %d/%d passed"),
		PassCount, PassCount + FailCount);
	PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility] ========== TestKeyValueRoundTrip END =========="));
}

// ============================================================================
// EN: TestSlotOperations
// ES: Test de Operaciones de Slots
// ============================================================================

void UPGXSaveTestUtility::TestSlotOperations(const UObject* WorldContextObject, FGameplayTag ContextTag)
{
	PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility] ========== TestSlotOperations START =========="));

	UPGXSaveSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub) { LogTestResult(TEXT("SlotOps"), false, TEXT("no subsystem")); return; }

	const FGameplayTag DomainTag = GetFirstDomainTag(Sub, ContextTag);
	if (!DomainTag.IsValid()) { LogTestResult(TEXT("SlotOps"), false, TEXT("no domain")); return; }

	const FString SlotA = TestSlotPrefix + TEXT("SlotA");
	const FString SlotB = TestSlotPrefix + TEXT("SlotB");

	// EN: Write data and create SlotA
	// ES: Escribir datos y crear SlotA
	UPGXSaveGame* SG = Sub->GetSaveGame(DomainTag);
	if (!SG) { LogTestResult(TEXT("SlotOps"), false, TEXT("null SaveGame")); return; }

	SG->WriteString(FName("_PGXTest_SlotData"), TEXT("SlotA_Original"));
	EPGXSaveResult Res = Sub->SaveContext(ContextTag, SlotA);
	LogTestResult(TEXT("SlotOps.CreateA"), Res == EPGXSaveResult::Success);

	// EN: Verify SlotA exists
	// ES: Verificar que SlotA existe
	LogTestResult(TEXT("SlotOps.ExistsA"), Sub->DoesSlotExist(ContextTag, SlotA));

	// EN: Copy SlotA → SlotB
	// ES: Copiar SlotA → SlotB
	Res = Sub->CopySlot(ContextTag, SlotA, SlotB);
	LogTestResult(TEXT("SlotOps.Copy"), Res == EPGXSaveResult::Success);
	LogTestResult(TEXT("SlotOps.ExistsB"), Sub->DoesSlotExist(ContextTag, SlotB));

	// EN: Load from SlotB and verify data integrity
	// ES: Cargar desde SlotB y verificar integridad de datos
	Sub->ClearDomain(DomainTag);
	Res = Sub->LoadContext(ContextTag, SlotB);
	LogTestResult(TEXT("SlotOps.LoadB"), Res == EPGXSaveResult::Success);

	SG = Sub->GetSaveGame(DomainTag);
	LogTestResult(TEXT("SlotOps.CopyIntegrity"),
		SG && SG->ReadString(FName("_PGXTest_SlotData")) == TEXT("SlotA_Original"));

	// EN: Enumerate slots
	// ES: Enumerar slots
	TArray<FPGXSaveSlotInfo> AllSlots = Sub->GetAllSlots(ContextTag);
	int32 TestSlotCount = 0;
	for (const FPGXSaveSlotInfo& Info : AllSlots)
	{
		if (Info.SlotName.StartsWith(TestSlotPrefix))
		{
			++TestSlotCount;
		}
	}
	LogTestResult(TEXT("SlotOps.Enumerate"), TestSlotCount >= 2,
		FString::Printf(TEXT("found %d test slots in %d total"), TestSlotCount, AllSlots.Num()));

	// EN: Delete SlotA
	// ES: Eliminar SlotA
	Res = Sub->DeleteSlot(ContextTag, SlotA);
	LogTestResult(TEXT("SlotOps.DeleteA"), Res == EPGXSaveResult::Success);
	LogTestResult(TEXT("SlotOps.GoneA"), !Sub->DoesSlotExist(ContextTag, SlotA));

	// EN: Cleanup SlotB
	// ES: Limpiar SlotB
	Sub->DeleteSlot(ContextTag, SlotB);

	PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility] ========== TestSlotOperations END =========="));
}

// ============================================================================
// EN: TestQuickSaveLoad
// ES: Test de QuickSave/QuickLoad
// ============================================================================

void UPGXSaveTestUtility::TestQuickSaveLoad(const UObject* WorldContextObject, FGameplayTag ContextTag)
{
	PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility] ========== TestQuickSaveLoad START =========="));

	UPGXSaveSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub) { LogTestResult(TEXT("QuickSL"), false, TEXT("no subsystem")); return; }

	const FGameplayTag DomainTag = GetFirstDomainTag(Sub, ContextTag);
	if (!DomainTag.IsValid()) { LogTestResult(TEXT("QuickSL"), false, TEXT("no domain")); return; }

	// EN: Write test data
	// ES: Escribir datos de test
	UPGXSaveGame* SG = Sub->GetSaveGame(DomainTag);
	if (!SG) { LogTestResult(TEXT("QuickSL"), false, TEXT("null SaveGame")); return; }

	SG->WriteString(FName("_PGXTest_QS"), TEXT("QuickSaveData"));
	SG->WriteInt(FName("_PGXTest_QS_Num"), 9999);

	// EN: QuickSave
	// ES: QuickSave
	EPGXSaveResult Res = Sub->QuickSave(ContextTag);
	LogTestResult(TEXT("QuickSL.Save"), Res == EPGXSaveResult::Success);
	if (Res != EPGXSaveResult::Success) return;

	// EN: Clear and QuickLoad
	// ES: Limpiar y QuickLoad
	Sub->ClearDomain(DomainTag);

	Res = Sub->QuickLoad(ContextTag);
	LogTestResult(TEXT("QuickSL.Load"), Res == EPGXSaveResult::Success);
	if (Res != EPGXSaveResult::Success) return;

	// EN: Verify
	// ES: Verificar
	SG = Sub->GetSaveGame(DomainTag);
	LogTestResult(TEXT("QuickSL.String"), SG && SG->ReadString(FName("_PGXTest_QS")) == TEXT("QuickSaveData"));
	LogTestResult(TEXT("QuickSL.Int"), SG && SG->ReadInt(FName("_PGXTest_QS_Num")) == 9999);

	PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility] ========== TestQuickSaveLoad END =========="));
}

// ============================================================================
// EN: TestVersionMigration
// ES: Test de Migracion de Version
// ============================================================================

void UPGXSaveTestUtility::TestVersionMigration(const UObject* WorldContextObject, FGameplayTag ContextTag)
{
	PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility] ========== TestVersionMigration START =========="));

	UPGXSaveSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub) { LogTestResult(TEXT("Migration"), false, TEXT("no subsystem")); return; }

	const UPGXSaveConfig* Config = Sub->GetContextConfig(ContextTag);
	if (!Config) { LogTestResult(TEXT("Migration"), false, TEXT("no config")); return; }

	const FGameplayTag DomainTag = GetFirstDomainTag(Sub, ContextTag);
	if (!DomainTag.IsValid()) { LogTestResult(TEXT("Migration"), false, TEXT("no domain")); return; }

	const int32 CurrentVersion = Config->CurrentSaveVersion;
	const int32 OldVersion = CurrentVersion - 1;

	if (OldVersion < 1)
	{
		LogTestResult(TEXT("Migration"), false, TEXT("CurrentSaveVersion must be > 1 for migration test"));
		return;
	}

	const FString SlotName = TestSlotPrefix + TEXT("Migration");

	// EN: Step 1 — Register a temporary migration from (CurrentVersion - 1) → CurrentVersion
	// ES: Paso 1 — Registrar migracion temporal de (CurrentVersion - 1) → CurrentVersion
	const FName MigrationMarkerKey(TEXT("_PGXTest_Migrated"));

	FPGXSaveVersioning::RegisterMigration(OldVersion, CurrentVersion,
		[MigrationMarkerKey](UPGXSaveGame* MigrateSG) -> bool
		{
			// EN: The migration writes a marker to prove it ran
			// ES: La migracion escribe un marcador para demostrar que se ejecuto
			MigrateSG->WriteString(MigrationMarkerKey, TEXT("MIGRATED"));
			return true;
		});

	LogTestResult(TEXT("Migration.Register"),
		FPGXSaveVersioning::HasMigrationPath(OldVersion, CurrentVersion));

	// EN: Step 2 — Write data and manually set SaveGame to old version, then save
	// ES: Paso 2 — Escribir datos y manualmente poner SaveGame en version vieja, luego guardar
	UPGXSaveGame* SG = Sub->GetSaveGame(DomainTag);
	if (!SG) { LogTestResult(TEXT("Migration"), false, TEXT("null SaveGame")); return; }

	SG->WriteString(FName("_PGXTest_MigData"), TEXT("PreMigration"));
	SG->SaveFormatVersion = OldVersion;

	EPGXSaveResult Res = Sub->SaveContext(ContextTag, SlotName);
	LogTestResult(TEXT("Migration.Save"), Res == EPGXSaveResult::Success);
	if (Res != EPGXSaveResult::Success) return;

	// EN: Step 3 — Clear and load (should trigger migration)
	// ES: Paso 3 — Limpiar y cargar (deberia disparar migracion)
	Sub->ClearDomain(DomainTag);

	Res = Sub->LoadContext(ContextTag, SlotName);
	LogTestResult(TEXT("Migration.Load"), Res == EPGXSaveResult::Success);

	// EN: Step 4 — Verify migration ran (check marker key, not lambda capture)
	// ES: Paso 4 — Verificar que la migracion se ejecuto (verificar key marcador, no captura lambda)
	SG = Sub->GetSaveGame(DomainTag);
	LogTestResult(TEXT("Migration.VersionUpdated"),
		SG && SG->SaveFormatVersion == CurrentVersion,
		FString::Printf(TEXT("expected v%d, got v%d"), CurrentVersion, SG ? SG->SaveFormatVersion : -1));
	LogTestResult(TEXT("Migration.MarkerWritten"),
		SG && SG->ReadString(MigrationMarkerKey) == TEXT("MIGRATED"));
	LogTestResult(TEXT("Migration.DataPreserved"),
		SG && SG->ReadString(FName("_PGXTest_MigData")) == TEXT("PreMigration"));

	// EN: Cleanup
	// ES: Limpieza
	Sub->DeleteSlot(ContextTag, SlotName);

	PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility] ========== TestVersionMigration END =========="));
}

// ============================================================================
// EN: RunStressTest
// ES: Stress Test
// ============================================================================

void UPGXSaveTestUtility::RunStressTest(const UObject* WorldContextObject, FGameplayTag ContextTag, int32 KeyCount)
{
	PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility] ========== RunStressTest START (KeyCount=%d) =========="), KeyCount);

	UPGXSaveSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub) { LogTestResult(TEXT("Stress"), false, TEXT("no subsystem")); return; }

	const FGameplayTag DomainTag = GetFirstDomainTag(Sub, ContextTag);
	if (!DomainTag.IsValid()) { LogTestResult(TEXT("Stress"), false, TEXT("no domain")); return; }

	const FString SlotName = TestSlotPrefix + TEXT("Stress");

	// EN: Phase 1 — Write N keys
	// ES: Fase 1 — Escribir N keys
	const double WriteStart = FPlatformTime::Seconds();
	{
		UPGXSaveGame* SG = Sub->GetSaveGame(DomainTag);
		if (!SG) { LogTestResult(TEXT("Stress"), false, TEXT("null SaveGame")); return; }

		for (int32 i = 0; i < KeyCount; ++i)
		{
			SG->WriteInt(FName(*FString::Printf(TEXT("_PGXTest_Stress_%d"), i)), i * 7 + 3);
		}
	}
	const double WriteTime = FPlatformTime::Seconds() - WriteStart;

	// EN: Phase 2 — Save to disk
	// ES: Fase 2 — Guardar en disco
	const double SaveStart = FPlatformTime::Seconds();
	EPGXSaveResult Res = Sub->SaveContext(ContextTag, SlotName);
	const double SaveTime = FPlatformTime::Seconds() - SaveStart;
	LogTestResult(TEXT("Stress.Save"), Res == EPGXSaveResult::Success);
	if (Res != EPGXSaveResult::Success) return;

	// EN: Phase 3 — Clear and load
	// ES: Fase 3 — Limpiar y cargar
	Sub->ClearDomain(DomainTag);

	const double LoadStart = FPlatformTime::Seconds();
	Res = Sub->LoadContext(ContextTag, SlotName);
	const double LoadTime = FPlatformTime::Seconds() - LoadStart;
	LogTestResult(TEXT("Stress.Load"), Res == EPGXSaveResult::Success);
	if (Res != EPGXSaveResult::Success) return;

	// EN: Phase 4 — Verify all keys
	// ES: Fase 4 — Verificar todas las keys
	const double VerifyStart = FPlatformTime::Seconds();
	int32 Mismatches = 0;
	{
		UPGXSaveGame* SG = Sub->GetSaveGame(DomainTag);
		if (!SG) { LogTestResult(TEXT("Stress"), false, TEXT("null after load")); return; }

		for (int32 i = 0; i < KeyCount; ++i)
		{
			const int32 Expected = i * 7 + 3;
			const int32 Actual = SG->ReadInt(FName(*FString::Printf(TEXT("_PGXTest_Stress_%d"), i)));
			if (Actual != Expected)
			{
				++Mismatches;
			}
		}
	}
	const double VerifyTime = FPlatformTime::Seconds() - VerifyStart;
	const double TotalTime = WriteTime + SaveTime + LoadTime + VerifyTime;

	LogTestResult(TEXT("Stress.Verify"), Mismatches == 0,
		FString::Printf(TEXT("%d/%d keys correct"), KeyCount - Mismatches, KeyCount));

	PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility] Stress Performance (%d keys):"), KeyCount);
	PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility]   Write:  %.3f ms"), WriteTime * 1000.0);
	PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility]   Save:   %.3f ms"), SaveTime * 1000.0);
	PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility]   Load:   %.3f ms"), LoadTime * 1000.0);
	PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility]   Verify: %.3f ms"), VerifyTime * 1000.0);
	PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility]   Total:  %.3f ms (%.0f keys/sec)"),
		TotalTime * 1000.0, KeyCount / FMath::Max(TotalTime, 0.0001));

	// EN: Cleanup
	// ES: Limpieza
	Sub->DeleteSlot(ContextTag, SlotName);

	PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility] ========== RunStressTest END =========="));
}

// ============================================================================
// EN: SimulateGameSession
// ES: Simulacion de Sesion de Juego
// ============================================================================

void UPGXSaveTestUtility::SimulateGameSession(const UObject* WorldContextObject, FGameplayTag ContextTag)
{
	PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility] ========== SimulateGameSession START =========="));

	UPGXSaveSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub) { LogTestResult(TEXT("GameSession"), false, TEXT("no subsystem")); return; }

	const FGameplayTag DomainTag = GetFirstDomainTag(Sub, ContextTag);
	if (!DomainTag.IsValid()) { LogTestResult(TEXT("GameSession"), false, TEXT("no domain")); return; }

	UPGXSaveGame* SG = Sub->GetSaveGame(DomainTag);
	if (!SG) { LogTestResult(TEXT("GameSession"), false, TEXT("null SaveGame")); return; }

	// EN: Phase 1 — Game startup: initial data
	// ES: Fase 1 — Inicio de juego: datos iniciales
	PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility] Phase 1: Game Startup"));
	SG->WriteString(FName("PlayerName"), TEXT("TestPlayer_PGX"));
	SG->WriteInt(FName("Level"), 1);
	SG->WriteInt(FName("HP"), 100);
	SG->WriteInt(FName("MaxHP"), 100);
	SG->WriteFloat(FName("PlayTime"), 0.0f);
	SG->WriteVector(FName("SpawnLocation"), FVector(0.0, 0.0, 100.0));
	SG->WriteBool(FName("TutorialComplete"), false);

	// EN: Phase 2 — Gameplay progression
	// ES: Fase 2 — Progresion de gameplay
	PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility] Phase 2: Gameplay Progression"));
	SG->WriteInt(FName("Level"), 5);
	SG->WriteInt(FName("HP"), 85);
	SG->WriteFloat(FName("PlayTime"), 1800.0f);
	SG->WriteVector(FName("LastCheckpoint"), FVector(5000.0, 3000.0, 200.0));
	SG->WriteBool(FName("TutorialComplete"), true);
	SG->WriteInt(FName("Gold"), 2500);
	SG->WriteInt(FName("EnemiesDefeated"), 47);
	SG->WriteString(FName("CurrentChapter"), TEXT("Chapter_03"));

	// EN: Phase 3 — Manual save
	// ES: Fase 3 — Guardado manual
	const FString ManualSlot = TestSlotPrefix + TEXT("Session_Manual");
	PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility] Phase 3: Manual Save to %s"), *ManualSlot);
	Sub->SaveContext(ContextTag, ManualSlot);

	// EN: Phase 4 — More gameplay, then auto-save trigger
	// ES: Fase 4 — Mas gameplay, luego trigger de auto-save
	PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility] Phase 4: More Gameplay + Auto-Save"));
	SG->WriteInt(FName("Level"), 7);
	SG->WriteInt(FName("HP"), 45);
	SG->WriteFloat(FName("PlayTime"), 3600.0f);
	SG->WriteInt(FName("Gold"), 5100);
	SG->WriteInt(FName("EnemiesDefeated"), 93);
	SG->WriteString(FName("CurrentChapter"), TEXT("Chapter_05"));

	Sub->TriggerAutoSave(ContextTag);

	// EN: Phase 5 — Quick save
	// ES: Fase 5 — Quick save
	PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility] Phase 5: Quick Save"));
	SG->WriteVector(FName("LastCheckpoint"), FVector(12000.0, -500.0, 400.0));
	Sub->QuickSave(ContextTag);

	// EN: Phase 6 — Simulate death: load from manual save
	// ES: Fase 6 — Simular muerte: cargar desde save manual
	PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility] Phase 6: Load from Manual Save (simulating death)"));
	Sub->LoadContext(ContextTag, ManualSlot);

	SG = Sub->GetSaveGame(DomainTag);
	if (SG)
	{
		LogTestResult(TEXT("GameSession.LoadVerify"),
			SG->ReadInt(FName("Level")) == 5 && SG->ReadString(FName("CurrentChapter")) == TEXT("Chapter_03"),
			TEXT("State should revert to Phase 2 checkpoint"));
	}

	// EN: Cleanup
	// ES: Limpieza
	Sub->DeleteSlot(ContextTag, ManualSlot);

	PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility] ========== SimulateGameSession END =========="));
}

// ============================================================================
// EN: RunAllTests — aggregate validation for Test Dashboard
// ES: RunAllTests — validacion agregada para Test Dashboard
// ============================================================================

bool UPGXSaveTestUtility::RunAllTests(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	bool bAllPassed = true;

	OutIssues.Add(TEXT("=== PGX Save Test Suite ==="));

	// EN: Test 1: Subsystem accessible / ES: Test 1: Subsistema accesible
	UPGXSaveSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		OutIssues.Add(TEXT("[FAIL] SaveSubsystem not found"));
		return false;
	}
	OutIssues.Add(TEXT("[PASS] SaveSubsystem found"));

	// EN: Test 2: Context discovery / ES: Test 2: Descubrimiento de contextos
	TArray<FGameplayTag> ContextTags = Sub->GetAllContextTags();
	if (ContextTags.Num() == 0)
	{
		OutIssues.Add(TEXT("[FAIL] No save contexts discovered"));
		bAllPassed = false;
	}
	else
	{
		OutIssues.Add(FString::Printf(TEXT("[PASS] %d save context(s) discovered"), ContextTags.Num()));
	}

	// EN: Test 3: SaveGame accessible / ES: Test 3: SaveGame accesible
	if (ContextTags.Num() > 0)
	{
		const FGameplayTag FirstCtx = ContextTags[0];
		const FGameplayTag DomainTag = GetFirstDomainTag(Sub, FirstCtx);

		if (!DomainTag.IsValid())
		{
			OutIssues.Add(TEXT("[FAIL] No domains in first context"));
			bAllPassed = false;
		}
		else
		{
			UPGXSaveGame* SG = Sub->GetSaveGame(DomainTag);
			if (!IsValid(SG))
			{
				OutIssues.Add(TEXT("[FAIL] SaveGame is null"));
				bAllPassed = false;
			}
			else
			{
				OutIssues.Add(TEXT("[PASS] SaveGame accessible"));

				// EN: Test 4: Write/Save/Load roundtrip / ES: Test 4: Roundtrip Write/Save/Load
				const FString TestSlot = TEXT("_PGXTest_Dashboard");
				SG->WriteString(FName("_PGXTest_Verify"), TEXT("DashboardTest"));

				const EPGXSaveResult SaveRes = Sub->SaveContext(FirstCtx, TestSlot);
				if (SaveRes != EPGXSaveResult::Success)
				{
					OutIssues.Add(TEXT("[FAIL] Save to disk"));
					bAllPassed = false;
				}
				else
				{
					// EN: Clear only the test key (not the whole domain) to avoid side effects in PIE
					// ES: Limpiar solo la clave de test (no todo el dominio) para evitar side effects en PIE
					SG->WriteString(FName("_PGXTest_Verify"), TEXT(""));
					const EPGXSaveResult LoadRes = Sub->LoadContext(FirstCtx, TestSlot);
					if (LoadRes != EPGXSaveResult::Success)
					{
						OutIssues.Add(TEXT("[FAIL] Load from disk"));
						bAllPassed = false;
					}
					else
					{
						SG = Sub->GetSaveGame(DomainTag);
						if (SG && SG->ReadString(FName("_PGXTest_Verify")) == TEXT("DashboardTest"))
						{
							OutIssues.Add(TEXT("[PASS] Save/Load roundtrip"));
						}
						else
						{
							OutIssues.Add(TEXT("[FAIL] Data verification after load"));
							bAllPassed = false;
						}
					}
					Sub->DeleteSlot(FirstCtx, TestSlot);
				}
			}
		}
	}

	OutIssues.Add(FString::Printf(TEXT("=== Result: %s ==="), bAllPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED")));
	PGX_LOG_INFO(LogPGXSave, TEXT("[TestUtility] RunAllTests — %s"), bAllPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED"));
	return bAllPassed;
}
