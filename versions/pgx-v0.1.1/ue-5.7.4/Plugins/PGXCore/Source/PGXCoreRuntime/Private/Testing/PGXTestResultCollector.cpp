// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Testing/PGXTestResultCollector.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "PGXCoreRuntime.h"

// ============================================================================
// EN: Singleton accessor
// ES: Acceso al singleton
// ============================================================================

FPGXTestResultCollector& FPGXTestResultCollector::Get()
{
	static FPGXTestResultCollector Instance;
	return Instance;
}

// ============================================================================
// EN: Record / Clear
// ES: Registrar / Limpiar
// ============================================================================

void FPGXTestResultCollector::RecordResult(const FPGXTestResult& Result)
{
	{
		FScopeLock ScopeLock(&Lock);
		Results.Add(Result);
	}
	OnResultsChanged.Broadcast();
}

void FPGXTestResultCollector::ClearAll()
{
	{
		FScopeLock ScopeLock(&Lock);
		Results.Empty();
	}
	OnResultsChanged.Broadcast();
}

void FPGXTestResultCollector::ClearSystem(const FString& SystemName)
{
	{
		FScopeLock ScopeLock(&Lock);
		Results.RemoveAll([&SystemName](const FPGXTestResult& R) { return R.SystemName == SystemName; });
	}
	OnResultsChanged.Broadcast();
}

// ============================================================================
// EN: Queries
// ES: Consultas
// ============================================================================

TArray<FPGXSystemTestSummary> FPGXTestResultCollector::GetSystemSummaries() const
{
	FScopeLock ScopeLock(&Lock);

	// EN: Group results by system name / ES: Agrupar resultados por nombre de sistema
	TMap<FString, FPGXSystemTestSummary> SummaryMap;

	for (const FPGXTestResult& R : Results)
	{
		FPGXSystemTestSummary& Summary = SummaryMap.FindOrAdd(R.SystemName);
		if (Summary.SystemName.IsEmpty())
		{
			Summary.SystemName = R.SystemName;
			Summary.SystemVersion = R.SystemVersion;
		}

		Summary.Results.Add(R);

		if (R.bPassed)
		{
			Summary.PassedCount++;
		}
		else
		{
			Summary.FailedCount++;
		}

		if (R.Timestamp > Summary.LastRunTime)
		{
			Summary.LastRunTime = R.Timestamp;
		}
	}

	TArray<FPGXSystemTestSummary> Out;
	SummaryMap.GenerateValueArray(Out);

	// EN: Sort by system name for consistent display / ES: Ordenar por nombre de sistema para display consistente
	Out.Sort([](const FPGXSystemTestSummary& A, const FPGXSystemTestSummary& B)
	{
		return A.SystemName < B.SystemName;
	});

	return Out;
}

int32 FPGXTestResultCollector::GetTotalPassed() const
{
	FScopeLock ScopeLock(&Lock);
	int32 Count = 0;
	for (const FPGXTestResult& R : Results)
	{
		if (R.bPassed) ++Count;
	}
	return Count;
}

int32 FPGXTestResultCollector::GetTotalFailed() const
{
	FScopeLock ScopeLock(&Lock);
	int32 Count = 0;
	for (const FPGXTestResult& R : Results)
	{
		if (!R.bPassed) ++Count;
	}
	return Count;
}

int32 FPGXTestResultCollector::GetTotalTests() const
{
	FScopeLock ScopeLock(&Lock);
	return Results.Num();
}

// ============================================================================
// EN: JSON Export
// ES: Exportar JSON
// ============================================================================

FString FPGXTestResultCollector::ExportToJson() const
{
	FScopeLock ScopeLock(&Lock);

	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("framework"), TEXT("PGX"));
	Root->SetStringField(TEXT("version"), PGXVersion::String);
	Root->SetStringField(TEXT("exportTime"), FDateTime::UtcNow().ToIso8601());

	// EN: Summary / ES: Resumen
	TSharedRef<FJsonObject> SummaryObj = MakeShared<FJsonObject>();
	int32 TotalPassed = 0;
	int32 TotalFailed = 0;
	double TotalDuration = 0.0;
	for (const FPGXTestResult& R : Results)
	{
		if (R.bPassed) ++TotalPassed; else ++TotalFailed;
		TotalDuration += R.DurationMs;
	}
	SummaryObj->SetNumberField(TEXT("totalTests"), Results.Num());
	SummaryObj->SetNumberField(TEXT("passed"), TotalPassed);
	SummaryObj->SetNumberField(TEXT("failed"), TotalFailed);
	SummaryObj->SetNumberField(TEXT("duration_ms"), TotalDuration);
	Root->SetObjectField(TEXT("summary"), SummaryObj);

	// EN: Group by system / ES: Agrupar por sistema
	TMap<FString, TArray<const FPGXTestResult*>> SystemGroups;
	for (const FPGXTestResult& R : Results)
	{
		SystemGroups.FindOrAdd(R.SystemName).Add(&R);
	}

	TArray<TSharedPtr<FJsonValue>> SystemsArray;
	for (const auto& Pair : SystemGroups)
	{
		TSharedRef<FJsonObject> SysObj = MakeShared<FJsonObject>();
		SysObj->SetStringField(TEXT("name"), Pair.Key);
		if (Pair.Value.Num() > 0 && !Pair.Value[0]->SystemVersion.IsEmpty())
		{
			SysObj->SetStringField(TEXT("version"), Pair.Value[0]->SystemVersion);
		}

		int32 SysPassed = 0;
		int32 SysFailed = 0;
		TArray<TSharedPtr<FJsonValue>> ResultsArray;

		for (const FPGXTestResult* R : Pair.Value)
		{
			if (R->bPassed) ++SysPassed; else ++SysFailed;

			TSharedRef<FJsonObject> ResObj = MakeShared<FJsonObject>();
			ResObj->SetStringField(TEXT("test"), R->TestName);
			ResObj->SetBoolField(TEXT("passed"), R->bPassed);
			ResObj->SetNumberField(TEXT("duration_ms"), R->DurationMs);
			ResObj->SetStringField(TEXT("timestamp"), R->Timestamp.ToIso8601());

			TArray<TSharedPtr<FJsonValue>> IssuesArr;
			for (const FString& Issue : R->Issues)
			{
				IssuesArr.Add(MakeShared<FJsonValueString>(Issue));
			}
			ResObj->SetArrayField(TEXT("issues"), IssuesArr);

			ResultsArray.Add(MakeShared<FJsonValueObject>(ResObj));
		}

		SysObj->SetNumberField(TEXT("passed"), SysPassed);
		SysObj->SetNumberField(TEXT("failed"), SysFailed);
		SysObj->SetArrayField(TEXT("results"), ResultsArray);

		SystemsArray.Add(MakeShared<FJsonValueObject>(SysObj));
	}

	Root->SetArrayField(TEXT("systems"), SystemsArray);

	// EN: Serialize to string / ES: Serializar a string
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(Root, Writer);

	return OutputString;
}

// ============================================================================
// EN: Markdown Export
// ES: Exportar Markdown
// ============================================================================

FString FPGXTestResultCollector::ExportToMarkdown(EPGXReportLanguage Language) const
{
	FScopeLock ScopeLock(&Lock);

	// EN: Bilingual string helper / ES: Helper de strings bilingues
	auto L = [Language](const TCHAR* En, const TCHAR* Es) -> const TCHAR*
	{
		return (Language == EPGXReportLanguage::English) ? En : Es;
	};

	FString MD;

	// EN: Header / ES: Encabezado
	MD += FString::Printf(TEXT("# PGX Framework — %s\n"), L(TEXT("Test Report"), TEXT("Informe de Test")));
	MD += FString::Printf(TEXT("> Framework: PGX %s | Engine: UE 5.6.1 | %s: %s\n\n"),
		PGXVersion::String,
		L(TEXT("Date"), TEXT("Fecha")),
		*FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S")));

	// EN: Summary / ES: Resumen
	int32 TotalPassed = 0;
	int32 TotalFailed = 0;
	double TotalDuration = 0.0;
	bool bHarnessDetected = false;

	for (const FPGXTestResult& R : Results)
	{
		if (R.bPassed) ++TotalPassed; else ++TotalFailed;
		TotalDuration += R.DurationMs;
		if (R.TestName.Contains(TEXT("[HARNESS]"))) bHarnessDetected = true;
	}

	const int32 TotalTests = Results.Num();
	const float PassRate = TotalTests > 0 ? (static_cast<float>(TotalPassed) / static_cast<float>(TotalTests)) * 100.0f : 0.0f;

	MD += FString::Printf(TEXT("## %s\n"), L(TEXT("Summary"), TEXT("Resumen")));
	MD += FString::Printf(TEXT("| %s | %s |\n"), L(TEXT("Metric"), TEXT("Metrica")), L(TEXT("Value"), TEXT("Valor")));
	MD += TEXT("|--------|-------|\n");
	MD += FString::Printf(TEXT("| %s | %d |\n"), L(TEXT("Total Tests"), TEXT("Tests Totales")), TotalTests);
	MD += FString::Printf(TEXT("| %s | %d |\n"), L(TEXT("Passed"), TEXT("Aprobados")), TotalPassed);
	MD += FString::Printf(TEXT("| %s | %d |\n"), L(TEXT("Failed"), TEXT("Fallidos")), TotalFailed);
	MD += FString::Printf(TEXT("| %s | %.1f%% |\n"), L(TEXT("Pass Rate"), TEXT("Tasa de Aprobacion")), PassRate);
	MD += FString::Printf(TEXT("| %s | %.1f ms |\n"), L(TEXT("Total Duration"), TEXT("Duracion Total")), TotalDuration);
	MD += FString::Printf(TEXT("| %s | %s |\n\n"),
		L(TEXT("Harness Mode"), TEXT("Modo Harness")),
		bHarnessDetected ? L(TEXT("Active"), TEXT("Activo")) : L(TEXT("Inactive"), TEXT("Inactivo")));

	// EN: Results by system / ES: Resultados por sistema
	MD += FString::Printf(TEXT("## %s\n\n"), L(TEXT("Results by System"), TEXT("Resultados por Sistema")));

	// EN: Group by system preserving insertion order / ES: Agrupar por sistema preservando orden de insercion
	TArray<FString> SystemOrder;
	TMap<FString, TArray<const FPGXTestResult*>> SystemGroups;
	TMap<FString, FString> SystemVersions;
	for (const FPGXTestResult& R : Results)
	{
		if (!SystemGroups.Contains(R.SystemName))
		{
			SystemOrder.Add(R.SystemName);
		}
		SystemGroups.FindOrAdd(R.SystemName).Add(&R);
		SystemVersions.FindOrAdd(R.SystemName) = R.SystemVersion;
	}

	for (const FString& SysName : SystemOrder)
	{
		const TArray<const FPGXTestResult*>& SysResults = SystemGroups.FindChecked(SysName);
		const FString& SysVersion = SystemVersions.FindChecked(SysName);

		int32 SysPassed = 0;
		int32 SysFailed = 0;
		double SysDuration = 0.0;
		bool bSysHarness = false;

		for (const FPGXTestResult* R : SysResults)
		{
			if (R->bPassed) ++SysPassed; else ++SysFailed;
			SysDuration += R->DurationMs;
			if (R->TestName.Contains(TEXT("[HARNESS]"))) bSysHarness = true;
		}

		const int32 SysTotal = SysPassed + SysFailed;
		const TCHAR* StatusStr = (SysFailed == 0)
			? L(TEXT("PASSED"), TEXT("APROBADO"))
			: L(TEXT("FAILED"), TEXT("FALLIDO"));

		MD += FString::Printf(TEXT("### %s %s — %s (%d/%d)\n\n"),
			*SysName, *SysVersion, StatusStr, SysPassed, SysTotal);

		MD += FString::Printf(TEXT("| # | %s | %s | %s | %s |\n"),
			L(TEXT("Test"), TEXT("Test")),
			L(TEXT("Result"), TEXT("Resultado")),
			L(TEXT("Duration"), TEXT("Duracion")),
			L(TEXT("Issues"), TEXT("Problemas")));
		MD += TEXT("|---|------|--------|----------|--------|\n");

		int32 TestNum = 0;
		TArray<FString> AllIssues;
		for (const FPGXTestResult* R : SysResults)
		{
			++TestNum;
			const TCHAR* ResultStr = R->bPassed ? TEXT("PASS") : TEXT("FAIL");
			FString IssueStr = TEXT("-");
			if (R->Issues.Num() > 0)
			{
				IssueStr = FString::Join(R->Issues, TEXT("; "));
				AllIssues.Append(R->Issues);
			}

			MD += FString::Printf(TEXT("| %d | %s | %s | %.1f ms | %s |\n"),
				TestNum, *R->TestName, ResultStr, R->DurationMs, *IssueStr);
		}

		// EN: Details block — evidence from test execution / ES: Bloque de detalles — evidencia de ejecucion
		MD += FString::Printf(TEXT("\n**%s:**\n"), L(TEXT("Details"), TEXT("Detalles")));
		MD += FString::Printf(TEXT("- %s: %.1f ms\n"),
			L(TEXT("Total duration"), TEXT("Duracion total")), SysDuration);
		if (bSysHarness)
		{
			MD += FString::Printf(TEXT("- %s\n"),
				L(TEXT("Executed with injected test config (Harness Mode)"),
				  TEXT("Ejecutado con config de test inyectada (Modo Harness)")));
		}
		if (AllIssues.Num() > 0)
		{
			MD += FString::Printf(TEXT("- %s\n"), L(TEXT("Issues found"), TEXT("Problemas encontrados")));
			for (const FString& Issue : AllIssues)
			{
				MD += FString::Printf(TEXT("  - %s\n"), *Issue);
			}
		}
		else
		{
			MD += FString::Printf(TEXT("- %s\n"),
				L(TEXT("No issues detected"), TEXT("Sin problemas detectados")));
		}

		MD += TEXT("\n");
	}

	// EN: Test environment / ES: Entorno de test
	MD += FString::Printf(TEXT("## %s\n"), L(TEXT("Test Environment"), TEXT("Entorno de Test")));
	MD += FString::Printf(TEXT("- Framework: PGX Core %s\n"), PGXVersion::String);
	MD += TEXT("- Engine: Unreal Engine 5.6.1\n");
	MD += TEXT("- Platform: Windows\n");
	MD += TEXT("- Build: Development Editor\n");
	MD += FString::Printf(TEXT("- PIE World: %s\n"),
		TotalTests > 0
			? L(TEXT("Active (tests executed in PIE session)"), TEXT("Activo (tests ejecutados en sesion PIE)"))
			: L(TEXT("Unknown (no test results)"), TEXT("Desconocido (sin resultados de test)")));

	if (bHarnessDetected)
	{
		MD += FString::Printf(TEXT("- Harness: %s\n"),
			L(TEXT("6 systems injected (Audio, PSO, Save, Construction, Message, EventHandler)"),
			  TEXT("6 sistemas inyectados (Audio, PSO, Save, Construction, Message, EventHandler)")));
	}

	MD += TEXT("\n---\n");
	MD += FString::Printf(TEXT("*%s PGX Test Dashboard — %s*\n"),
		L(TEXT("Generated by"), TEXT("Generado por")),
		*FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S")));

	return MD;
}
